/* $Id$ */
/**************************************************************************
 *   global.c                                                             *
 *                                                                        *
 *   Copyright (C) 1999 Chris Allegretta                                  *
 *   This program is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by *
 *   the Free Software Foundation; either version 1, or (at your option)  *
 *   any later version.                                                   *
 *                                                                        *
 *   This program is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *   GNU General Public License for more details.                         *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program; if not, write to the Free Software          *
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
 *                                                                        *
 **************************************************************************/

#include "config.h"

#include <sys/stat.h>
#include "nano.h"
#include "proto.h"

#ifndef NANO_SMALL
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

/*
 * Global variables
 */
int flags = 0;			/* Our new flag containing many options */
WINDOW *edit;			/* The file portion of the editor  */
WINDOW *topwin;			/* Top line of screen */
WINDOW *bottomwin;		/* Bottom buffer */
char *filename = NULL;		/* Name of the file */
int editwinrows = 0;		/* How many rows long is the edit
				   window? */
filestruct *current;		/* Current buffer pointer */
int current_x = 0, current_y = 0;	/* Current position of X and Y in
					   the editor - relative to edit
					   window (0,0) */
filestruct *fileage = NULL;	/* Our file buffer */
filestruct *edittop = NULL;	/* Pointer to the top of the edit
				   buffer with respect to the
				   file struct */
filestruct *editbot = NULL;	/* Same for the bottom */
filestruct *filebot = NULL;	/* Last node in the file struct */
filestruct *cutbuffer = NULL;	/* A place to store cut text */

#ifdef ENABLE_MULTIBUFFER
filestruct *open_files = NULL;	/* The list of open files */
#endif

char *answer = NULL;			/* Answer str to many questions */
int totlines = 0;		/* Total number of lines in the file */
int totsize = 0;		/* Total number of bytes in the file */
int placewewant = 0;		/* The column we'd like the cursor
				   to jump to when we go to the
				   next or previous line */

int tabsize = 8;		/* Our internal tabsize variable */

char *hblank;			/* A horizontal blank line */
char *help_text;		/* The text in the help window */

/* More stuff for the marker select */

filestruct *mark_beginbuf;	/* the begin marker buffer */
int mark_beginx;		/* X value in the string to start */

#ifndef DISABLE_OPERATINGDIR
char *operating_dir = NULL;	/* Operating directory, which we can't go
				   higher than */
#endif

#ifndef DISABLE_SPELLER
char *alt_speller;		/* Alternative spell command */
#endif

shortcut main_list[MAIN_LIST_LEN];
shortcut whereis_list[WHEREIS_LIST_LEN];
shortcut replace_list[REPLACE_LIST_LEN];
shortcut replace_list_2[REPLACE_LIST_LEN]; /* 2nd half of replace dialog */
shortcut goto_list[GOTO_LIST_LEN];
shortcut gotodir_list[GOTODIR_LIST_LEN];
shortcut writefile_list[WRITEFILE_LIST_LEN];
shortcut insertfile_list[INSERTFILE_LIST_LEN];
shortcut help_list[HELP_LIST_LEN];
shortcut spell_list[SPELL_LIST_LEN];
#ifndef DISABLE_BROWSER
shortcut browser_list[BROWSER_LIST_LEN];
#endif

#ifdef ENABLE_COLOR
    colorstruct colors[NUM_NCOLORS];
#endif

#ifndef DISABLE_MOUSE
shortcut *currshortcut = main_list;	/* Current shortcut list we're using */
int currslen = MAIN_VISIBLE;		/* Length of current shortcut list */
#endif

#ifndef NANO_SMALL
toggle toggles[TOGGLE_LEN];
#endif

/* Regular expressions */

#ifdef HAVE_REGEX_H
regex_t search_regexp;		/* Global to store compiled search regexp */
regmatch_t regmatches[10];	/* Match positions for parenthetical
				   subexpressions, max of 10 */
#endif

/* Initialize a struct *without* our lovely braces =( */
void sc_init_one(shortcut * s, int key, char *desc, char *help, int alt,
		 int misc1, int misc2, int view, int (*func) (void))
{
    s->val = key;
    s->desc = desc;
    s->help = help;
    s->altval = alt;
    s->misc1 = misc1;
    s->misc2 = misc2;
    s->viewok = view;
    s->func = func;
}

#ifndef NANO_SMALL
/* Initialize the toggles in the same manner */
void toggle_init_one(toggle * t, int val, char *desc, int flag,
		     char override_ch)
{
    t->val = val;
    t->desc = desc;
    t->flag = flag;
    t->override_ch = override_ch;
}
#endif

void toggle_init(void)
{
#ifndef NANO_SMALL
    char *toggle_const_msg, *toggle_autoindent_msg, *toggle_suspend_msg,
	*toggle_nohelp_msg, *toggle_picomode_msg, *toggle_mouse_msg,
	*toggle_cuttoend_msg, *toggle_wrap_msg, *toggle_case_msg, 
	*toggle_backwards_msg, *toggle_dos_msg, *toggle_mac_msg,
	*toggle_smooth_msg;

#ifdef ENABLE_MULTIBUFFER
    char *toggle_load_msg, *nano_openprev_msg, *nano_opennext_msg;
#endif

#ifdef HAVE_REGEX_H
    char *toggle_regexp_msg;
#endif


    toggle_const_msg = _("Constant cursor position");
    toggle_autoindent_msg = _("Auto indent");
    toggle_suspend_msg = _("Suspend");
    toggle_nohelp_msg = _("Help mode");
    toggle_picomode_msg = _("Pico mode");
    toggle_mouse_msg = _("Mouse support");
    toggle_cuttoend_msg = _("Cut to end");
    toggle_backwards_msg = _("Backwards search");
    toggle_case_msg = _("Case sensitive search");
    toggle_dos_msg = _("Writing file in DOS format");
    toggle_mac_msg = _("Writing file in Mac format");
    toggle_smooth_msg = _("Smooth scrolling");

#ifdef HAVE_REGEX_H
    toggle_regexp_msg = _("Regular expressions");
#endif
    toggle_wrap_msg = _("Auto wrap");

#ifdef ENABLE_MULTIBUFFER
    toggle_load_msg = _("Multiple file buffers");
    nano_openprev_msg = _("Open previously loaded file");
    nano_opennext_msg = _("Open next loaded file");
#endif

    toggle_init_one(&toggles[0], TOGGLE_CONST_KEY, toggle_const_msg,
		    CONSTUPDATE, 0);
    toggle_init_one(&toggles[1], TOGGLE_AUTOINDENT_KEY,
		    toggle_autoindent_msg, AUTOINDENT, 0);
    toggle_init_one(&toggles[2], TOGGLE_SUSPEND_KEY, toggle_suspend_msg,
		    SUSPEND, 0);
    toggle_init_one(&toggles[3], TOGGLE_NOHELP_KEY, toggle_nohelp_msg,
		    NO_HELP, 0);
    toggle_init_one(&toggles[4], TOGGLE_PICOMODE_KEY, toggle_picomode_msg,
		    PICO_MODE, 0);
    toggle_init_one(&toggles[5], TOGGLE_WRAP_KEY, toggle_wrap_msg,
		    NO_WRAP, 0);
    toggle_init_one(&toggles[6], TOGGLE_MOUSE_KEY, toggle_mouse_msg,
		    USE_MOUSE, 0);
    toggle_init_one(&toggles[7], TOGGLE_CUTTOEND_KEY, toggle_cuttoend_msg,
		    CUT_TO_END, 0);
    toggle_init_one(&toggles[8], TOGGLE_BACKWARDS_KEY, toggle_backwards_msg,
		    REVERSE_SEARCH, 0);
    toggle_init_one(&toggles[9], TOGGLE_CASE_KEY, toggle_case_msg,
		    CASE_SENSITIVE, 0);
    toggle_init_one(&toggles[10], TOGGLE_DOS_KEY, toggle_dos_msg,
		    DOS_FILE, 0);
    toggle_init_one(&toggles[11], TOGGLE_MAC_KEY, toggle_mac_msg,
		    MAC_FILE, 0);
    toggle_init_one(&toggles[12], TOGGLE_SMOOTH_KEY, toggle_smooth_msg,
		    SMOOTHSCROLL, 0);

#ifdef ENABLE_MULTIBUFFER
    toggle_init_one(&toggles[13], TOGGLE_LOAD_KEY, toggle_load_msg,
		    MULTIBUFFER, 0);
    toggle_init_one(&toggles[14], NANO_OPENPREV_KEY, nano_openprev_msg,
		    0, '<');
    toggle_init_one(&toggles[15], NANO_OPENNEXT_KEY, nano_opennext_msg,
		    0, '>');
#endif

#ifdef HAVE_REGEX_H
    toggle_init_one(&toggles[TOGGLE_LEN - 1], TOGGLE_REGEXP_KEY,
		    toggle_regexp_msg, USE_REGEXP, 0);
#endif
#endif
}

void shortcut_init(int unjustify)
{
    char *nano_help_msg = "", *nano_writeout_msg = "", *nano_exit_msg = "",
	*nano_goto_msg = "", *nano_justify_msg = "", *nano_replace_msg =
	"", *nano_insert_msg = "", *nano_whereis_msg =
	"", *nano_prevpage_msg = "", *nano_nextpage_msg =
	"", *nano_cut_msg = "", *nano_uncut_msg = "", *nano_cursorpos_msg =
	"", *nano_spell_msg = "", *nano_up_msg = "", *nano_down_msg =
	"", *nano_forward_msg = "", *nano_back_msg = "", *nano_home_msg =
	"", *nano_end_msg = "", *nano_firstline_msg =
	"", *nano_lastline_msg = "", *nano_refresh_msg =
	"", *nano_mark_msg = "", *nano_delete_msg =
	"", *nano_backspace_msg = "", *nano_tab_msg =
	"", *nano_enter_msg = "", *nano_cancel_msg = 
	"", *nano_unjustify_msg = "", *nano_append_msg = ""; 

#ifndef NANO_SMALL
    char *nano_tofiles_msg = "", *nano_gotodir_msg = "", *nano_case_msg =
	"", *nano_reverse_msg = "", *nano_regexp_msg = "",
	*nano_bracket_msg = "";

    nano_help_msg = _("Invoke the help menu");
    nano_writeout_msg = _("Write the current file to disk");

#ifdef ENABLE_MULTIBUFFER
    nano_exit_msg = _("Close currently loaded file/Exit from nano");
#else
    nano_exit_msg = _("Exit from nano");
#endif

    nano_goto_msg = _("Goto a specific line number");
    nano_justify_msg = _("Justify the current paragraph");
    nano_unjustify_msg = _("Unjustify after a justify");
    nano_replace_msg = _("Replace text within the editor");
    nano_insert_msg = _("Insert another file into the current one");
    nano_whereis_msg = _("Search for text within the editor");
    nano_prevpage_msg = _("Move to the previous screen");
    nano_nextpage_msg = _("Move to the next screen");
    nano_cut_msg = _("Cut the current line and store it in the cutbuffer");
    nano_uncut_msg = _("Uncut from the cutbuffer into the current line");
    nano_cursorpos_msg = _("Show the position of the cursor");
    nano_spell_msg = _("Invoke the spell checker (if available)");
    nano_up_msg = _("Move up one line");
    nano_down_msg = _("Move down one line");
    nano_forward_msg = _("Move forward one character");
    nano_back_msg = _("Move back one character");
    nano_home_msg = _("Move to the beginning of the current line");
    nano_end_msg = _("Move to the end of the current line");
    nano_firstline_msg = _("Go to the first line of the file");
    nano_lastline_msg = _("Go to the last line of the file");
    nano_refresh_msg = _("Refresh (redraw) the current screen");
    nano_mark_msg = _("Mark text at the current cursor location");
    nano_delete_msg = _("Delete the character under the cursor");
    nano_backspace_msg =
	_("Delete the character to the left of the cursor");
    nano_tab_msg = _("Insert a tab character");
    nano_enter_msg = _("Insert a carriage return at the cursor position");
    nano_case_msg =
	_("Make the current search or replace case (in)sensitive");
    nano_tofiles_msg = _("Go to file browser");
    nano_gotodir_msg = _("Goto Directory");
    nano_cancel_msg = _("Cancel the current function");
    nano_append_msg = _("Append to the current file");
    nano_reverse_msg = _("Search backwards");
    nano_regexp_msg = _("Use Regular expressions");
    nano_bracket_msg = _("Find other bracket");
#endif

	sc_init_one(&main_list[0], NANO_HELP_KEY, _("Get Help"),
		    nano_help_msg, 0, NANO_HELP_FKEY, 0, VIEW, do_help);

#ifdef ENABLE_MULTIBUFFER
    if (open_files != NULL && (open_files->prev || open_files->next))
	sc_init_one(&main_list[1], NANO_EXIT_KEY, _("Close"),
		nano_exit_msg, 0, NANO_EXIT_FKEY, 0, VIEW, do_exit);
    else
#endif
	sc_init_one(&main_list[1], NANO_EXIT_KEY, _("Exit"),
		nano_exit_msg, 0, NANO_EXIT_FKEY, 0, VIEW, do_exit);

    sc_init_one(&main_list[2], NANO_WRITEOUT_KEY, _("WriteOut"),
		    nano_writeout_msg,
		    0, NANO_WRITEOUT_FKEY, 0, NOVIEW, do_writeout_void);

    if (ISSET(PICO_MODE))
	sc_init_one(&main_list[3], NANO_JUSTIFY_KEY, _("Justify"),
		    nano_justify_msg, 0, NANO_JUSTIFY_FKEY, 0,
		    NOVIEW, do_justify);
    else
	sc_init_one(&main_list[3], NANO_INSERTFILE_KEY, _("Read File"),
		nano_insert_msg,
		0, NANO_INSERTFILE_FKEY, 0, NOVIEW, do_insertfile_void);


    if (ISSET(PICO_MODE))
	sc_init_one(&main_list[4], NANO_INSERTFILE_KEY, _("Read File"),
		nano_insert_msg,
		0, NANO_INSERTFILE_FKEY, 0, NOVIEW, do_insertfile_void);
    else
	sc_init_one(&main_list[4], NANO_REPLACE_KEY, _("Replace"),
		    nano_replace_msg,
		    NANO_ALT_R, NANO_REPLACE_FKEY, 0, NOVIEW, do_replace);

    sc_init_one(&main_list[5], NANO_WHEREIS_KEY, _("Where Is"),
		nano_whereis_msg,
		0, NANO_WHEREIS_FKEY, 0, VIEW, do_search);

    sc_init_one(&main_list[6], NANO_PREVPAGE_KEY, _("Prev Page"),
		nano_prevpage_msg,
		0, NANO_PREVPAGE_FKEY, KEY_PPAGE, VIEW, do_page_up);

    sc_init_one(&main_list[7], NANO_NEXTPAGE_KEY, _("Next Page"),
		nano_nextpage_msg,
		0, NANO_NEXTPAGE_FKEY, KEY_NPAGE, VIEW, do_page_down);

    sc_init_one(&main_list[8], NANO_CUT_KEY, _("Cut Text"),
		nano_cut_msg, 0, NANO_CUT_FKEY, 0, NOVIEW, do_cut_text);

    if (unjustify)
	sc_init_one(&main_list[9], NANO_UNJUSTIFY_KEY, _("UnJustify"),
		nano_unjustify_msg, 0, 0, 0, NOVIEW, do_uncut_text);
    else
	sc_init_one(&main_list[9], NANO_UNCUT_KEY, _("UnCut Txt"),
		nano_uncut_msg,
		0, NANO_UNCUT_FKEY, 0, NOVIEW, do_uncut_text);

    sc_init_one(&main_list[10], NANO_CURSORPOS_KEY, _("Cur Pos"),
		nano_cursorpos_msg,
		0, NANO_CURSORPOS_FKEY, 0, VIEW, do_cursorpos);

    sc_init_one(&main_list[11], NANO_SPELL_KEY, _("To Spell"),
		nano_spell_msg, 0, NANO_SPELL_FKEY, 0, NOVIEW, do_spell);


    sc_init_one(&main_list[12], NANO_UP_KEY, _("Up"),
		nano_up_msg, 0, KEY_UP, 0, VIEW, do_up);

    sc_init_one(&main_list[13], NANO_DOWN_KEY, _("Down"),
		nano_down_msg, 0, KEY_DOWN, 0, VIEW, do_down);

    sc_init_one(&main_list[14], NANO_FORWARD_KEY, _("Forward"),
		nano_forward_msg, 0, KEY_RIGHT, 0, VIEW, do_right);

    sc_init_one(&main_list[15], NANO_BACK_KEY, _("Back"),
		nano_back_msg, 0, KEY_LEFT, 0, VIEW, do_left);

    sc_init_one(&main_list[16], NANO_HOME_KEY, _("Home"),
		nano_home_msg, 0, KEY_HOME, 362, VIEW, do_home);

    sc_init_one(&main_list[17], NANO_END_KEY, _("End"),
		nano_end_msg, 0, KEY_END, 385, VIEW, do_end);

    sc_init_one(&main_list[18], NANO_REFRESH_KEY, _("Refresh"),
		nano_refresh_msg, 0, 0, 0, VIEW, total_refresh);

    sc_init_one(&main_list[19], NANO_MARK_KEY, _("Mark Text"),
		nano_mark_msg, 0, 0, 0, NOVIEW, do_mark);

    sc_init_one(&main_list[20], NANO_DELETE_KEY, _("Delete"),
		nano_delete_msg, 0, KEY_DC,
		NANO_CONTROL_D, NOVIEW, do_delete);

    sc_init_one(&main_list[21], NANO_BACKSPACE_KEY, _("Backspace"),
		nano_backspace_msg, 0,
		KEY_BACKSPACE, 127, NOVIEW, do_backspace);

    sc_init_one(&main_list[22], NANO_TAB_KEY, _("Tab"),
		nano_tab_msg, 0, 0, 0, NOVIEW, do_tab);

    if (ISSET(PICO_MODE))
	sc_init_one(&main_list[23], NANO_REPLACE_KEY, _("Replace"),
		    nano_replace_msg,
		    NANO_ALT_R, NANO_REPLACE_FKEY, 0, NOVIEW, do_replace);
    else
	sc_init_one(&main_list[23], NANO_JUSTIFY_KEY, _("Justify"),
		    nano_justify_msg, 0, NANO_JUSTIFY_FKEY, 0,
		    NOVIEW, do_justify);

    sc_init_one(&main_list[24], NANO_ENTER_KEY, _("Enter"),
		nano_enter_msg,
		0, KEY_ENTER, NANO_CONTROL_M, NOVIEW, do_enter_void);

    sc_init_one(&main_list[25], NANO_GOTO_KEY, _("Goto Line"),
		    nano_goto_msg,
		    NANO_ALT_G, NANO_GOTO_FKEY, 0, VIEW, do_gotoline_void);

#if (!defined NANO_SMALL) && (defined HAVE_REGEX_H)
    sc_init_one(&main_list[26], 0, _("Find Other Bracket"),
		    nano_bracket_msg,
		    NANO_BRACKET_KEY, 0, 0, VIEW, do_find_bracket);
#endif

    sc_init_one(&whereis_list[0], NANO_FIRSTLINE_KEY, _("First Line"),
		nano_firstline_msg, 0, 0, 0, VIEW, do_first_line);

    sc_init_one(&whereis_list[1], NANO_LASTLINE_KEY, _("Last Line"),
		nano_lastline_msg, 0, 0, 0, VIEW, do_last_line);

    sc_init_one(&whereis_list[2], NANO_OTHERSEARCH_KEY, _("Replace"),
		nano_replace_msg, 0, 0, 0, VIEW, do_replace);

    sc_init_one(&whereis_list[3], NANO_FROMSEARCHTOGOTO_KEY,
		_("Goto Line"), nano_goto_msg, 0, 0, 0, VIEW,
		do_gotoline_void);

#ifndef NANO_SMALL
    sc_init_one(&whereis_list[4], TOGGLE_CASE_KEY, _("Case Sens"),
		nano_case_msg, 0, 0, 0, VIEW, 0);

    sc_init_one(&whereis_list[5], TOGGLE_BACKWARDS_KEY, _("Direction"),
		nano_reverse_msg, 0, 0, 0, VIEW, 0);

#ifdef HAVE_REGEX_H
    sc_init_one(&whereis_list[REPLACE_LIST_LEN - 2], TOGGLE_REGEXP_KEY,
		_("Regexp"), nano_regexp_msg, 0, 0, 0, VIEW, 0);
#endif
#endif /* NANO_SMALL */

    sc_init_one(&whereis_list[REPLACE_LIST_LEN - 1], NANO_CANCEL_KEY,
		_("Cancel"), nano_cancel_msg, 0, 0, 0, VIEW, 0);


    sc_init_one(&replace_list[0], NANO_FIRSTLINE_KEY, _("First Line"),
		nano_firstline_msg, 0, 0, 0, VIEW, do_first_line);

    sc_init_one(&replace_list[1], NANO_LASTLINE_KEY, _("Last Line"),
		nano_lastline_msg, 0, 0, 0, VIEW, do_last_line);

    sc_init_one(&replace_list[2], NANO_OTHERSEARCH_KEY, _("No Replace"),
		nano_whereis_msg, 0, 0, 0, VIEW, do_search);

    sc_init_one(&replace_list[3], NANO_FROMSEARCHTOGOTO_KEY,
		_("Goto Line"), nano_goto_msg, 0, 0, 0, VIEW,
		do_gotoline_void);

#ifndef NANO_SMALL
    sc_init_one(&replace_list[4], TOGGLE_CASE_KEY, _("Case Sens"),
		nano_case_msg, 0, 0, 0, VIEW, 0);

    sc_init_one(&replace_list[5], TOGGLE_BACKWARDS_KEY, _("Direction"),
		nano_reverse_msg, 0, 0, 0, VIEW, 0);

#ifdef HAVE_REGEX_H
    sc_init_one(&replace_list[REPLACE_LIST_LEN - 2], TOGGLE_REGEXP_KEY, 
		_("Regexp"), nano_regexp_msg, 0, 0, 0, VIEW, 0);
#endif
#endif /* NANO_SMALL */

    sc_init_one(&replace_list[REPLACE_LIST_LEN - 1], NANO_CANCEL_KEY,
		_("Cancel"), nano_cancel_msg, 0, 0, 0, VIEW, 0);


    sc_init_one(&replace_list_2[0], NANO_FIRSTLINE_KEY, _("First Line"),
		nano_firstline_msg, 0, 0, 0, VIEW, do_first_line);

    sc_init_one(&replace_list_2[1], NANO_LASTLINE_KEY, _("Last Line"),
		nano_lastline_msg, 0, 0, 0, VIEW, do_last_line);

    sc_init_one(&replace_list_2[2], NANO_CANCEL_KEY, _("Cancel"),
		nano_cancel_msg, 0, 0, 0, VIEW, 0);


    sc_init_one(&goto_list[0], NANO_FIRSTLINE_KEY, _("First Line"),
		nano_firstline_msg, 0, 0, 0, VIEW, &do_first_line);

    sc_init_one(&goto_list[1], NANO_LASTLINE_KEY, _("Last Line"),
		nano_lastline_msg, 0, 0, 0, VIEW, &do_last_line);

    sc_init_one(&goto_list[2], NANO_CANCEL_KEY, _("Cancel"),
		nano_cancel_msg, 0, 0, 0, VIEW, 0);


    sc_init_one(&help_list[0], NANO_PREVPAGE_KEY, _("Prev Page"),
		nano_prevpage_msg,
		0, NANO_PREVPAGE_FKEY, KEY_PPAGE, VIEW, do_page_up);

    sc_init_one(&help_list[1], NANO_NEXTPAGE_KEY, _("Next Page"),
		nano_nextpage_msg,
		0, NANO_NEXTPAGE_FKEY, KEY_NPAGE, VIEW, do_page_down);

    sc_init_one(&help_list[2], NANO_EXIT_KEY, _("Exit"),
		nano_exit_msg, 0, NANO_EXIT_FKEY, 0, VIEW, do_exit);


#ifndef DISABLE_BROWSER
    sc_init_one(&writefile_list[0], NANO_TOFILES_KEY, _("To Files"),
		nano_tofiles_msg, 0, 0, 0, NOVIEW, 0);
#endif

    sc_init_one(&writefile_list[WRITEFILE_LIST_LEN - 2], NANO_APPEND_KEY, _("Append"),
		nano_append_msg, 0, 0, 0, NOVIEW, 0);

    sc_init_one(&writefile_list[WRITEFILE_LIST_LEN - 1], NANO_CANCEL_KEY, _("Cancel"),
		nano_cancel_msg, 0, 0, 0, VIEW, 0);

#ifndef DISABLE_BROWSER
    sc_init_one(&insertfile_list[0], NANO_TOFILES_KEY, _("To Files"),
		nano_tofiles_msg, 0, 0, 0, NOVIEW, 0);
#endif

    sc_init_one(&insertfile_list[INSERTFILE_LIST_LEN - 1], NANO_CANCEL_KEY, _("Cancel"),
		nano_cancel_msg, 0, 0, 0, VIEW, 0);

    sc_init_one(&spell_list[0], NANO_CANCEL_KEY, _("Cancel"),
		nano_cancel_msg, 0, 0, 0, VIEW, 0);

#ifndef DISABLE_BROWSER
    sc_init_one(&browser_list[0], NANO_PREVPAGE_KEY, _("Prev Page"),
		nano_prevpage_msg,
		0, NANO_PREVPAGE_FKEY, KEY_PPAGE, VIEW, 0);

    sc_init_one(&browser_list[1], NANO_NEXTPAGE_KEY, _("Next Page"),
		nano_nextpage_msg,
		0, NANO_NEXTPAGE_FKEY, KEY_NPAGE, VIEW, 0);

    sc_init_one(&browser_list[2], NANO_GOTO_KEY, _("Goto"),
		nano_gotodir_msg, 0, NANO_GOTO_FKEY, 0, VIEW, 0);

    sc_init_one(&browser_list[3], NANO_EXIT_KEY, _("Exit"),
		nano_exit_msg, 0, NANO_EXIT_FKEY, 0, VIEW, 0);

    sc_init_one(&gotodir_list[0], NANO_CANCEL_KEY, _("Cancel"),
		nano_cancel_msg, 0, 0, 0, VIEW, 0);

#endif


    toggle_init();
}
