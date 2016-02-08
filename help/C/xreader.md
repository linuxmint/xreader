% Atril
% Oz Nahum (2012); Nickolay V. Shmyrev; Niels Giesen; Claude Paroz (2004)
% 18 September 2012


Introduction
============

Atril enables you to view documents of various formats
like Portable Document Format (PDF) files and PostScript files. Atril 
follows Freedesktop.org and MATE standards to provide integration with
Desktop Environment.

Getting Started
===============

To Start atril
----------------

atril starts when you open a document such as a PDF or PostScript file.

Alternatively, you can start atril from the command line, with the
command: `atril`.

When You Start atril
----------------------

When you start atril, the following window is displayed.

![ Shows atril main window. Contains titlebar, menubar, toolbar and
display area. Menubar contains File, Edit, View, Go and Help menus.
](figures/atril_start_window.png)

Atril window contains the following elements:

Menubar
:   The menus on the menubar contain all of the commands that you need
    to work with documents in APP.

Toolbar
:   The toolbar contains a subset of the commands that you can access
    from the menubar.

Display area
:   The display area displays the document.

In atril, you can perform the same action in several ways. For example,
you can open a document in the following ways:

  -------------------------------------------------------------------------
  UI Component                         Action
  ------------------------------------ ------------------------------------
  Window                               -   Drag a file into the APP window
                                           from another application such as
                                           a file manager.
                                       
                                       -   Double-click on the file name in
                                           the file manager
                                       
                                       

  Menubar                              Choose FileOpen.

  Shortcut keys                        Press CtrlO.
  -------------------------------------------------------------------------

This manual documents functionality from the menubar.

Usage
=====

To Open A Document
------------------

To open a document, perform the following steps:

1.  Choose FileOpen.

2.  In the Open Document dialog, select the file you want to open.

3.  Click Open. atril displays the name of the document in the titlebar of
    the window.

To open another document, choose FileOpen again. atril opens each file in
a new window.

If you try to open a document with format that atril does not recognize,
the application displays an error message.

To Navigate Through a Document
------------------------------

You can navigate through a file as follows:

-   To view the next page, choose Go Next Page.

-   To view the previous page, choose Go Previous Page.

-   To view the first page in the document, choose Go First Page.

-   To view the last page in the document, choose Go Last Page.

-   To view a particular page, enter the page number or page label in
    the text box on the toolbar, then press Return.

To Scroll a Page
----------------

To display the page contents that are not currently displayed in the
display area, use the following methods:

-   Use the arrow keys or space key on the keyboard.

-   Drag the display area by clicking with the middle mouse button
    somewhere in the document and then moving the mouse. For example, to
    scroll down the page, drag the display area upwards in the window.

-   Use the scrollbars on the window.

To Change the Page Size
-----------------------

You can use the following methods to resize a page in the atril display
area:

-   To increase the page size, choose View Zoom In.

-   To decrease the page size, choose View Zoom Out.

-   To resize a page to have the same width as the atril display area,
    choose View Fit page width.

-   To resize a page to fit within the atril display area, choose View
    Best Fit.

-   To resize the atril window to have the same width and height as the
    screen, choose View Full Screen. To resize the atril window to the
    original size, click on the Exit Full Screen button.

To View Pages or Document Structure
-----------------------------------

To view bookmarks or pages, perform the following steps:

1.  Choose View Sidebar or press F9.

2.  Use the drop-down list in the side-pane header to select whether to
    display document structure or pages in the side pane.

3.  Use the side-pane scrollbars to display the required item or page in
    the side pane.

4.  Click on an entry to navigate to that location in the document.
    Click on a page to navigate to that page in the document.

To View the Properties of a Document
------------------------------------

To view the properties of a document, choose File Properties.

The Properties dialog displays all information available

To Print a Document
-------------------

To print a Document, choose File Print.

> **Note**
>
> If you cannot choose the Print menu item, the author of the document
> has disabled the print option for this document. To enable the print
> option, you must enter the master password when you open the document.

The Print dialog has the following tabbed sections:

-   [Job](#print-dialog-job)

-   [Printer](#print-dialog-printer)

-   [Paper](#print-dialog-paper)

### Job

Print range
:   Select one of the following options to determine how many pages to
    print:

    -   All

        Select this option to print all of the pages in the document.

    -   Pages From

        Select this option to print the selected range of pages in the
        document. Use the spin boxes to specify the first page and last
        page of the range.

### Printer

Printer
:   Use this drop-down list to select the printer to which you want to
    print the document.

    > **Note**
    >
    > The Create a PDF document option is not supported in this version
    > of APP.

Settings
:   Use this drop-down list to select the printer settings.

    To configure the printer, click Configure. For example, you can
    enable or disable duplex printing, or schedule delayed printing, if
    this functionality is supported by the printer.

Location
:   Use this drop-down list to select one of the following print
    destinations:

    CUPS
    :   Print the document to a CUPS printer.

        > **Note**
        >
        > If the selected printer is a CUPS printer, CUPS is the only
        > entry in this drop-down list.

    lpr
    :   Print the document to a printer.

    File
    :   Print the document to a PostScript file.

        Click Save As to display a dialog where you specify the name and
        location of the PostScript file.

    Custom
    :   Use the specified command to print the document.

        Type the name of the command in the text box. Include all
        command-line arguments.

State
:   This functionality is not supported in this version of atril.

Type
:   This functionality is not supported in this version of atril.

Comment
:   This functionality is not supported in this version of atril.

### Paper

Paper size
:   Use this drop-down list to select the size of the paper to which you
    want to print the document.

Width
:   Use this spin box to specify the width of the paper. Use the
    adjacent drop-down list to change the measurement unit.

Height
:   Use this spin box to specify the height of the paper.

Feed orientation
:   Use this drop-down list to select the orientation of the paper in
    the printer.

Page orientation
:   Use this drop-down list to select the page orientation.

Layout
:   Use this drop-down list to select the page layout. A preview of each
    layout that you select is displayed in the Preview area.

Paper Tray
:   Use this drop-down list to select the paper tray.

To Copy a Document
------------------

To copy a file, perform the following steps:

1.  Choose File Save a Copy.

2.  Type the new filename in the Filename text box in the Save a Copy
    dialog.

    If necessary, specify the location of the copied document. By
    default, copies are saved in your home directory.

3.  Click Save.

To Work With Password-Protected Documents
-----------------------------------------

An author can use the following password levels to protect a document:

-   User password that allows others only to read the document.

-   Master password that allows others to perform additional actions,
    such as print the document.

When you try to open a password-protected document, atril displays a
security dialog. Type either the user password or the master password in
the Enter document password text box, then click Open Document.

To Close a Document
-------------------

To close a document, choose File Close.

If the window is the last atril window open, the application exits.

List of Keyboard Shortcuts
==========================

Below is a table of all shortcuts present in atril:


+-----------------------+---------------------------+
|  Shortcut             |         Action            |
+=======================+===========================+
| Ctrl-O                | Open an existing document |
+-----------------------+---------------------------+
|  Ctrl-S               | Save a copy of the current| 
|                       |  document                 |
+-----------------------+---------------------------+
|  Ctrl-P               | Print document            |
+-----------------------+---------------------------+
| Ctrl-W                | Close window                 |
+-----------------------+------------------------------+
|  Ctrl-C               |    Copy selection            |
+-----------------------+------------------------------+
|  Ctrl-A               |   Select All                 |
+-----------------------+------------------------------+
| Ctrl-F ,              | Find a word or phrase in the |
|                       | document                     |
|  / (slash)            |                              |    
+-----------------------+------------------------------+
|  Ctrl-G               |  Find next                   |
|  F3                   |                              |
+-----------------------+------------------------------+
|  Ctrl + (plus sign)   |  Zoom in                  | 
|                       |                           | 
|   +                   |                           |
|                       |                           | 
|   =                   |                           |  
+-----------------------+---------------------------+
|  Ctrl -(minus sign),- |  Zoom out                 |
+-----------------------+---------------------------+
|  Ctrl-R               |  Reload the document      |
+-----------------------+---------------------------+
|  Page Up              |  Go to the previous page  |
+-----------------------+---------------------------+
|  Page Down            |  Go to the next page      |
+-----------------------+---------------------------+
|  Space, j             |  Scroll forward           |
|                       |                           |
|  Shift Backspace      |                           |
|                       |                           |
|  Return               |                           |
+-----------------------+---------------------------+
|  Shift Space, k       | Scroll backward           |
|  Backspace            |                           |
+-----------------------+---------------------------+
|  Shift Return         |                           |
|  Shift Page Up        | Go a bunch of pages up    |
+-----------------------+---------------------------+
|  Shift Page Down      | Go a bunch of pages down  |
+-----------------------+---------------------------+
|  Home                 | Go to the first page      |  
+-----------------------+---------------------------+
|  End                  | Go to the last page       | 
+-----------------------+---------------------------+
| Ctrl-L                | Go to page by number or label|
+-----------------------+---------------------------+
|  F1                   | Help                      |  
+-----------------------+---------------------------+
| F5                    | Run document as presentation|
+-----------------------+---------------------------+
| F9                    | Show or hide the side pane|
+-----------------------+---------------------------+
| F11                   | Toggle fullscreen mode    |
+-----------------------+---------------------------+



