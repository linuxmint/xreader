/* ev-previewer.c:
 *
 * Copyright (C) 2009 Carlos Garcia Campos <carlosgc@gnome.org>
 *
 * Xreader is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Xreader is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <xreader-document.h>
#include <xreader-view.h>

#include "ev-previewer-window.h"

static gboolean      unlink_temp_file = FALSE;
static const gchar  *print_settings;
static const gchar **filenames;

static const GOptionEntry goption_options[] = {
	{ "unlink-tempfile", 'u', 0, G_OPTION_ARG_NONE, &unlink_temp_file, N_("Delete the temporary file"), NULL },
	{ "print-settings", 'p', 0, G_OPTION_ARG_FILENAME, &print_settings, N_("Print settings file"), N_("FILE") },
	{ G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &filenames, NULL, N_("FILE") },
	{ NULL }
};

static void
ev_previewer_unlink_tempfile (const gchar *filename)
{
	GFile *file, *tempdir;

	file = g_file_new_for_path (filename);
	tempdir = g_file_new_for_path (g_get_tmp_dir ());

	if (g_file_has_prefix (file, tempdir)) {
		g_file_delete (file, NULL, NULL);
	}

	g_object_unref (file);
	g_object_unref (tempdir);
}

static void
ev_previewer_load_job_finished (EvJob           *job,
				EvDocumentModel *model)
{
	if (ev_job_is_failed (job)) {
		g_warning ("%s", job->error->message);
		g_object_unref (job);

		return;
	}
	ev_document_model_set_document (model, job->document);
	g_object_unref (job);
}

static void
ev_previewer_load_document (const gchar     *filename,
			    EvDocumentModel *model)
{
	EvJob *job;
	gchar *uri;
	GFile  *file;

	file = g_file_new_for_commandline_arg (filename);
	uri = g_file_get_uri (file);
	g_object_unref (file);

	job = ev_job_load_new (uri);
	g_signal_connect (job, "finished",
			  G_CALLBACK (ev_previewer_load_job_finished),
			  model);
	ev_job_scheduler_push_job (job, EV_JOB_PRIORITY_NONE);
	g_free (uri);
}

gint
main (gint argc, gchar **argv)
{
	GtkWidget       *window;
	GOptionContext  *context;
	const gchar     *filename;
	EvDocumentModel *model;
	GError          *error = NULL;

#ifdef ENABLE_NLS
	/* Initialize the i18n stuff */
	bindtextdomain (GETTEXT_PACKAGE, XREADER_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	context = g_option_context_new (_("Print Preview"));
	g_option_context_set_translation_domain (context, GETTEXT_PACKAGE);
	g_option_context_add_main_entries (context, goption_options, GETTEXT_PACKAGE);

	g_option_context_add_group (context, gtk_get_option_group (TRUE));

	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_warning ("Error parsing command line arguments: %s", error->message);
		g_error_free (error);
		g_option_context_free (context);
		return 1;
	}
	g_option_context_free (context);

	if (!filenames) {
		g_warning ("File argument is required");
		return 1;
	}

	filename = filenames[0];

	if (!g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {
		g_warning ("Filename \"%s\" does not exist or is not a regular file", filename);
		return 1;
	}

	if (!ev_init ())
		return 1;

	ev_stock_icons_init ();

	g_set_application_name (_("Print Preview"));
	gtk_window_set_default_icon_name ("xreader");

	model = ev_document_model_new ();
	window = ev_previewer_window_new (model);
	ev_previewer_window_set_source_file (EV_PREVIEWER_WINDOW (window), filename);
	ev_previewer_window_set_print_settings (EV_PREVIEWER_WINDOW (window), print_settings);
	g_signal_connect (window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
	gtk_widget_show (window);

	ev_previewer_load_document (filename, model);

	gtk_main ();

	if (unlink_temp_file)
		ev_previewer_unlink_tempfile (filename);
	if (print_settings)
		ev_previewer_unlink_tempfile (print_settings);

	ev_shutdown ();
	ev_stock_icons_shutdown ();
	g_object_unref (model);

	return 0;
}
