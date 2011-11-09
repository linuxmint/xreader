/* ev-keyring.c
 *  this file is part of evince, a mate document viewer
 *
 * Copyright (C) 2008 Carlos Garcia Campos <carlosgc@gnome.org>
 *
 * Evince is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Evince is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "config.h"

#include <glib/gi18n.h>

#include "ev-keyring.h"

#ifdef WITH_KEYRING
#include <mate-keyring.h>

static const MateKeyringPasswordSchema doc_password_schema = {
	MATE_KEYRING_ITEM_GENERIC_SECRET,
	{
		{ "type", MATE_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ "uri",  MATE_KEYRING_ATTRIBUTE_TYPE_STRING },
		{ NULL, 0 }
	}
};
const MateKeyringPasswordSchema *EV_DOCUMENT_PASSWORD_SCHEMA = &doc_password_schema;
#endif /* WITH_KEYRING */

gboolean
ev_keyring_is_available (void)
{
#ifdef WITH_KEYRING
	return mate_keyring_is_available ();
#else
	return FALSE;
#endif
}

gchar *
ev_keyring_lookup_password (const gchar *uri)
{
	gchar             *retval = NULL;
#ifdef WITH_KEYRING
	MateKeyringResult result;
	gchar             *password = NULL;
	
	g_return_val_if_fail (uri != NULL, NULL);

	if (!mate_keyring_is_available ())
		return NULL;
	
	result = mate_keyring_find_password_sync (EV_DOCUMENT_PASSWORD_SCHEMA,
						   &password,
						   "type", "document_password",
						   "uri", uri,
						   NULL);
	if (result != MATE_KEYRING_RESULT_OK || !password) {
		if (password)
			mate_keyring_free_password (password);
		return NULL;
	}

	retval = g_strdup (password);
	mate_keyring_free_password (password);
#endif /* WITH_KEYRING */
	return retval;
}

gboolean
ev_keyring_save_password (const gchar  *uri,
			  const gchar  *password,
			  GPasswordSave flags)
{
#ifdef WITH_KEYRING
	MateKeyringResult result;
	const gchar       *keyring;
	gchar             *name;
	gchar             *unescaped_uri;

	g_return_val_if_fail (uri != NULL, FALSE);

	if (!mate_keyring_is_available ())
		return FALSE;
	
	if (flags == G_PASSWORD_SAVE_NEVER)
		return FALSE;

	keyring = (flags == G_PASSWORD_SAVE_FOR_SESSION) ? "session" : NULL;
	unescaped_uri = g_uri_unescape_string (uri, NULL);
	name = g_strdup_printf (_("Password for document %s"), unescaped_uri);
	g_free (unescaped_uri);
	
	result = mate_keyring_store_password_sync (EV_DOCUMENT_PASSWORD_SCHEMA,
						    keyring, name, password,
						    "type", "document_password",
						    "uri", uri,
						    NULL);
	g_free (name);

	return (result == MATE_KEYRING_RESULT_OK);
#else
	return FALSE;
#endif /* WITH_KEYRING */
}
