/* $Id$ */
/**************************************************************************
 *   nano.h                                                               *
 *                                                                        *
 *   Copyright (C) 1999-2004 Chris Allegretta                             *
 *   Copyright (C) 2005-2006 David Lawrence Ramsey                        *
 *   This program is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by *
 *   the Free Software Foundation; either version 2, or (at your option)  *
 *   any later version.                                                   *
 *                                                                        *
 *   This program is distributed in the hope that it will be useful, but  *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    *
 *   General Public License for more details.                             *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program; if not, write to the Free Software          *
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA            *
 *   02110-1301, USA.                                                     *
 *                                                                        *
 **************************************************************************/

#ifndef NANO_H
#define NANO_H 1

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef __TANDEM
/* Tandem NonStop Kernel. */
#include <floss.h>
#define NANO_ROOT_UID 65535
#else
#define NANO_ROOT_UID 0
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

/* Macros for flags. */
#define SET(bit) flags |= bit
#define UNSET(bit) flags &= ~bit
#define ISSET(bit) ((flags & bit) != 0)
#define TOGGLE(bit) flags ^= bit

/* Macros for character allocation, etc. */
#define charalloc(howmuch) (char *)nmalloc((howmuch) * sizeof(char))
#define charealloc(ptr, howmuch) (char *)nrealloc(ptr, (howmuch) * sizeof(char))
#define charmove(dest, src, n) memmove(dest, src, (n) * sizeof(char))
#define charset(dest, src, n) memset(dest, src, (n) * sizeof(char))

/* Other macros. */
#ifdef BROKEN_REGEXEC
#undef regexec
#define regexec(preg, string, nmatch, pmatch, eflags) safe_regexec(preg, string, nmatch, pmatch, eflags)
#endif

/* Set a default value for PATH_MAX if there isn't one. */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifdef USE_SLANG
/* Slang support. */
#include <slcurses.h>
/* Slang curses emulation brain damage, part 2: Slang doesn't define the
 * curses equivalents of the Insert or Delete keys. */
#define KEY_DC SL_KEY_DELETE
#define KEY_IC SL_KEY_IC
/* Ncurses support. */
#elif defined(HAVE_NCURSES_H)
#include <ncurses.h>
#else
/* Curses support. */
#include <curses.h>
#endif /* CURSES_H */

#ifdef ENABLE_NLS
#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#endif
#define _(string) gettext(string)
#define P_(singular, plural, number) ngettext(singular, plural, number)
#else
#define _(string) (string)
#define P_(singular, plural, number) (number == 1 ? singular : plural)
#endif
#define gettext_noop(string) (string)
#define N_(string) gettext_noop(string)
	/* Mark a string that will be sent to gettext() later. */

#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#ifdef HAVE_REGEX_H
#include <regex.h>
#endif
#include <assert.h>

/* If no vsnprintf(), use the version from glib 2.x. */
#ifndef HAVE_VSNPRINTF
#include <glib.h>
#define vsnprintf g_vsnprintf
#endif

/* If no isblank(), iswblank(), strcasecmp(), strncasecmp(),
 * strcasestr(), strnlen(), getdelim(), or getline(), use the versions
 * we have. */
#ifndef HAVE_ISBLANK
#define isblank nisblank
#endif
#ifndef HAVE_ISWBLANK
#define iswblank niswblank
#endif
#ifndef HAVE_STRCASECMP
#define strcasecmp nstrcasecmp
#endif
#ifndef HAVE_STRNCASECMP
#define strncasecmp nstrncasecmp
#endif
#ifndef HAVE_STRCASESTR
#define strcasestr nstrcasestr
#endif
#ifndef HAVE_STRNLEN
#define strnlen nstrnlen
#endif
#ifndef HAVE_GETDELIM
#define getdelim ngetdelim
#endif
#ifndef HAVE_GETLINE
#define getline ngetline
#endif

/* If we don't have regex support, turn the color support off, as it
 * depends on the use of regexes. */
#ifndef HAVE_REGEX_H
#undef ENABLE_COLOR 
#endif

/* If we aren't using ncurses, turn the mouse support off, as it's
 * ncurses-specific. */
#ifndef NCURSES_MOUSE_VERSION
#define DISABLE_MOUSE 1
#endif

#if defined(DISABLE_WRAPPING) && defined(DISABLE_JUSTIFY)
#define DISABLE_WRAPJUSTIFY 1
#endif

/* Enumeration types. */
typedef enum {
    NIX_FILE, DOS_FILE, MAC_FILE
} file_format;

typedef enum {
    OVERWRITE, APPEND, PREPEND
} append_type;

typedef enum {
    UP, DOWN
} scroll_dir;

typedef enum {
    CENTER, NONE
} update_type;

/* Structure types. */
typedef struct filestruct {
    char *data;
	/* The text of this line. */
    ssize_t lineno;
	/* The number of this line. */
    struct filestruct *next;
	/* Next node. */
    struct filestruct *prev;
	/* Previous node. */
} filestruct;

typedef struct partition {
    filestruct *fileage;
	/* The top line of this portion of the file. */
    filestruct *top_prev;
	/* The line before the top line of this portion of the file. */
    char *top_data;
	/* The text before the beginning of the top line of this portion
	 * of the file. */
    filestruct *filebot;
	/* The bottom line of this portion of the file. */
    filestruct *bot_next;
	/* The line after the bottom line of this portion of the
	 * file. */
    char *bot_data;
	/* The text after the end of the bottom line of this portion of
	 * the file. */
} partition;

#ifdef ENABLE_COLOR
typedef struct colortype {
    short fg;
	/* This syntax's foreground color. */
    short bg;
	/* This syntax's background color. */
    bool bright;
	/* Is this color A_BOLD? */
    bool icase;
	/* Is this regex string case insensitive? */
    int pairnum;
	/* The color pair number used for this foreground color and
	 * background color. */
    char *start_regex;
	/* The start (or all) of the regex string. */
    regex_t *start;
	/* The compiled start (or all) of the regex string. */
    char *end_regex;
	/* The end (if any) of the regex string. */
    regex_t *end;
	/* The compiled end (if any) of the regex string. */
    struct colortype *next;
	/* Next set of colors. */
} colortype;

typedef struct exttype {
    char *ext_regex;
	/* The extensions that match this syntax. */
    regex_t *ext;
	/* The compiled extensions that match this syntax. */
    struct exttype *next;
	/* Next set of extensions. */
} exttype;

typedef struct syntaxtype {
    char *desc;
	/* The name of this syntax. */
    exttype *extensions;
	/* The list of extensions that this syntax applies to. */
    colortype *color;
	/* The colors used in this syntax. */
    struct syntaxtype *next;
	/* Next syntax. */
} syntaxtype;
#endif /* ENABLE_COLOR */

typedef struct openfilestruct {
    char *filename;
	/* The current file's name. */
    filestruct *fileage;
	/* The current file's first line. */
    filestruct *filebot;
	/* The current file's last line. */
    filestruct *edittop;
	/* The current top of the edit window. */
    filestruct *current;
	/* The current file's current line. */
    size_t totsize;
	/* The current file's total number of characters. */
    size_t current_x;
	/* The current file's x-coordinate position. */
    size_t placewewant;
	/* The current file's place we want. */
    ssize_t current_y;
	/* The current file's y-coordinate position. */
    bool modified;
	/* Whether the current file has been modified. */
#ifndef NANO_TINY
    bool mark_set;
	/* Whether the mark is on in the current file. */
    filestruct *mark_begin;
	/* The current file's beginning marked line, if any. */
    size_t mark_begin_x;
	/* The current file's beginning marked line's x-coordinate
	 * position, if any. */
    file_format fmt;
	/* The current file's format. */
    struct stat *current_stat;
	/* The current file's stat. */
#endif
#ifdef ENABLE_COLOR
    colortype *colorstrings;
	/* The current file's associated colors. */
#endif
    struct openfilestruct *next;
	/* Next node. */
    struct openfilestruct *prev;
	/* Previous node. */
} openfilestruct;

typedef struct shortcut {
    const char *desc;
	/* The function's description, e.g. "Page Up". */
#ifndef DISABLE_HELP
    const char *help;
	/* The help file entry text for this function. */
#endif
    bool blank_after;
	/* Whether there should be a blank line after the help entry
	 * text for this function. */
    /* Key values that aren't used should be set to NANO_NO_KEY. */
    int ctrlval;
	/* The special sentinel key or control key we want bound, if
	 * any. */
    int metaval;
	/* The meta key we want bound, if any. */
    int funcval;
	/* The function key we want bound, if any. */
    int miscval;
	/* The other meta key we want bound, if any. */
    bool viewok;
	/* Is this function allowed when in view mode? */
    void (*func)(void);
	/* The function to call when we get this key. */
    struct shortcut *next;
	/* Next shortcut. */
} shortcut;

#ifndef NANO_TINY
typedef struct toggle {
   int val;
	/* The sequence to toggle the key.  We should only need one. */
   const char *desc;
	/* The description of the toggle, e.g. "Cut to end"; we'll
	 * append Enabled or Disabled to it. */
   bool blank_after;
	/* Whether there should be a blank line after the description of
	 * the toggle. */
   long flag;
	/* Which flag actually gets toggled. */
   struct toggle *next;
	/* Next toggle. */
} toggle;
#endif

#ifdef ENABLE_NANORC
typedef struct rcoption {
   const char *name;
	/* The name of the rcfile option. */
   long flag;
	/* The flag associated with it, if any. */
} rcoption;
#endif

/* Bitwise flags so that we can save space (or, more correctly, not
 * waste it). */
#define CASE_SENSITIVE			(1<<0)
#define CONST_UPDATE			(1<<1)
#define NO_HELP				(1<<2)
#define NOFOLLOW_SYMLINKS		(1<<3)
#define SUSPEND				(1<<4)
#define NO_WRAP				(1<<5)
#define AUTOINDENT			(1<<6)
#define VIEW_MODE			(1<<7)
#define USE_MOUSE			(1<<8)
#define USE_REGEXP			(1<<9)
#define TEMP_FILE			(1<<10)
#define CUT_TO_END			(1<<11)
#define BACKWARDS_SEARCH		(1<<12)
#define MULTIBUFFER			(1<<13)
#define SMOOTH_SCROLL			(1<<14)
#define REBIND_DELETE			(1<<15)
#define REBIND_KEYPAD			(1<<16)
#define NO_CONVERT			(1<<17)
#define BACKUP_FILE			(1<<18)
#define NO_COLOR_SYNTAX			(1<<19)
#define PRESERVE			(1<<20)
#define HISTORYLOG			(1<<21)
#define RESTRICTED			(1<<22)
#define SMART_HOME			(1<<23)
#define WHITESPACE_DISPLAY		(1<<24)
#define MORE_SPACE			(1<<25)
#define TABS_TO_SPACES			(1<<26)
#define QUICK_BLANK			(1<<27)
#define WORD_BOUNDS			(1<<28)
#define NO_NEWLINES			(1<<29)
#define BOLD_TEXT			(1<<30)

/* Control key sequences.  Changing these would be very, very bad. */
#define NANO_CONTROL_SPACE 0
#define NANO_CONTROL_A 1
#define NANO_CONTROL_B 2
#define NANO_CONTROL_C 3
#define NANO_CONTROL_D 4
#define NANO_CONTROL_E 5
#define NANO_CONTROL_F 6
#define NANO_CONTROL_G 7
#define NANO_CONTROL_H 8
#define NANO_CONTROL_I 9
#define NANO_CONTROL_J 10
#define NANO_CONTROL_K 11
#define NANO_CONTROL_L 12
#define NANO_CONTROL_M 13
#define NANO_CONTROL_N 14
#define NANO_CONTROL_O 15
#define NANO_CONTROL_P 16
#define NANO_CONTROL_Q 17
#define NANO_CONTROL_R 18
#define NANO_CONTROL_S 19
#define NANO_CONTROL_T 20
#define NANO_CONTROL_U 21
#define NANO_CONTROL_V 22
#define NANO_CONTROL_W 23
#define NANO_CONTROL_X 24
#define NANO_CONTROL_Y 25
#define NANO_CONTROL_Z 26
#define NANO_CONTROL_3 27
#define NANO_CONTROL_4 28
#define NANO_CONTROL_5 29
#define NANO_CONTROL_6 30
#define NANO_CONTROL_7 31
#define NANO_CONTROL_8 127

#define NANO_ALT_SPACE ' '
#define NANO_ALT_LPARENTHESIS '('
#define NANO_ALT_RPARENTHESIS ')'
#define NANO_ALT_PLUS '+'
#define NANO_ALT_COMMA ','
#define NANO_ALT_MINUS '-'
#define NANO_ALT_PERIOD '.'
#define NANO_ALT_SLASH '/'
#define NANO_ALT_0 '0'
#define NANO_ALT_6 '6'
#define NANO_ALT_9 '9'
#define NANO_ALT_LCARET '<'
#define NANO_ALT_EQUALS '='
#define NANO_ALT_RCARET '>'
#define NANO_ALT_QUESTION '?'
#define NANO_ALT_LBRACKET '['
#define NANO_ALT_BACKSLASH '\\'
#define NANO_ALT_RBRACKET ']'
#define NANO_ALT_CARET '^'
#define NANO_ALT_UNDERSCORE '_'
#define NANO_ALT_A 'a'
#define NANO_ALT_B 'b'
#define NANO_ALT_C 'c'
#define NANO_ALT_D 'd'
#define NANO_ALT_E 'e'
#define NANO_ALT_F 'f'
#define NANO_ALT_G 'g'
#define NANO_ALT_H 'h'
#define NANO_ALT_I 'i'
#define NANO_ALT_J 'j'
#define NANO_ALT_K 'k'
#define NANO_ALT_L 'l'
#define NANO_ALT_M 'm'
#define NANO_ALT_N 'n'
#define NANO_ALT_O 'o'
#define NANO_ALT_P 'p'
#define NANO_ALT_Q 'q'
#define NANO_ALT_R 'r'
#define NANO_ALT_S 's'
#define NANO_ALT_T 't'
#define NANO_ALT_U 'u'
#define NANO_ALT_V 'v'
#define NANO_ALT_W 'w'
#define NANO_ALT_X 'x'
#define NANO_ALT_Y 'y'
#define NANO_ALT_Z 'z'
#define NANO_ALT_LCURLYBRACKET '{'
#define NANO_ALT_PIPE '|'
#define NANO_ALT_RCURLYBRACKET '}'

/* Some semi-changeable keybindings; don't play with these unless you're
 * sure you know what you're doing.  Assume ERR is defined as -1. */

/* No key at all. */
#define NANO_NO_KEY			-2

/* Normal keys. */
#define NANO_XON_KEY			NANO_CONTROL_Q
#define NANO_XOFF_KEY			NANO_CONTROL_S
#define NANO_CANCEL_KEY			NANO_CONTROL_C
#define NANO_EXIT_KEY			NANO_CONTROL_X
#define NANO_EXIT_FKEY			KEY_F(2)
#define NANO_INSERTFILE_KEY		NANO_CONTROL_R
#define NANO_INSERTFILE_FKEY		KEY_F(5)
#define NANO_TOOTHERINSERT_KEY		NANO_CONTROL_X
#define NANO_WRITEOUT_KEY		NANO_CONTROL_O
#define NANO_WRITEOUT_FKEY		KEY_F(3)
#define NANO_GOTOLINE_KEY		NANO_CONTROL_7
#define NANO_GOTOLINE_FKEY		KEY_F(13)
#define NANO_GOTOLINE_ALTKEY		NANO_ALT_G
#define NANO_TOGOTOLINE_KEY		NANO_CONTROL_T
#define NANO_HELP_KEY			NANO_CONTROL_G
#define NANO_HELP_FKEY			KEY_F(1)
#define NANO_WHEREIS_KEY		NANO_CONTROL_W
#define NANO_WHEREIS_FKEY		KEY_F(6)
#define NANO_WHEREIS_NEXT_KEY		NANO_ALT_W
#define NANO_WHEREIS_NEXT_FKEY		KEY_F(16)
#define NANO_TOOTHERWHEREIS_KEY		NANO_CONTROL_T
#define NANO_REGEXP_KEY			NANO_ALT_R
#define NANO_REPLACE_KEY		NANO_CONTROL_4
#define NANO_REPLACE_FKEY		KEY_F(14)
#define NANO_ALT_REPLACE_KEY		NANO_ALT_R
#define NANO_TOOTHERSEARCH_KEY		NANO_CONTROL_R
#define NANO_PREVPAGE_KEY		NANO_CONTROL_Y
#define NANO_PREVPAGE_FKEY		KEY_F(7)
#define NANO_NEXTPAGE_KEY		NANO_CONTROL_V
#define NANO_NEXTPAGE_FKEY		KEY_F(8)
#define NANO_CUT_KEY			NANO_CONTROL_K
#define NANO_CUT_FKEY			KEY_F(9)
#define NANO_COPY_KEY			NANO_ALT_CARET
#define NANO_COPY_ALTKEY		NANO_ALT_6
#define NANO_UNCUT_KEY			NANO_CONTROL_U
#define NANO_UNCUT_FKEY			KEY_F(10)
#define NANO_CURSORPOS_KEY		NANO_CONTROL_C
#define NANO_CURSORPOS_FKEY		KEY_F(11)
#define NANO_SPELL_KEY			NANO_CONTROL_T
#define NANO_SPELL_FKEY			KEY_F(12)
#define NANO_FIRSTLINE_KEY		NANO_PREVPAGE_KEY
#define NANO_FIRSTLINE_FKEY		NANO_PREVPAGE_FKEY
#define NANO_FIRSTLINE_ALTKEY		NANO_ALT_BACKSLASH
#define NANO_FIRSTLINE_ALTKEY2		NANO_ALT_PIPE
#define NANO_FIRSTFILE_KEY		NANO_FIRSTLINE_KEY
#define NANO_FIRSTFILE_FKEY		NANO_FIRSTLINE_FKEY
#define NANO_FIRSTFILE_ALTKEY		NANO_FIRSTLINE_ALTKEY
#define NANO_FIRSTFILE_ALTKEY2		NANO_FIRSTLINE_ALTKEY2
#define NANO_LASTLINE_KEY		NANO_NEXTPAGE_KEY
#define NANO_LASTLINE_FKEY		NANO_NEXTPAGE_FKEY
#define NANO_LASTLINE_ALTKEY		NANO_ALT_SLASH
#define NANO_LASTLINE_ALTKEY2		NANO_ALT_QUESTION
#define NANO_LASTFILE_KEY		NANO_LASTLINE_KEY
#define NANO_LASTFILE_FKEY		NANO_LASTLINE_FKEY
#define NANO_LASTFILE_ALTKEY		NANO_LASTLINE_ALTKEY
#define NANO_LASTFILE_ALTKEY2		NANO_LASTLINE_ALTKEY2
#define NANO_REFRESH_KEY		NANO_CONTROL_L
#define NANO_JUSTIFY_KEY		NANO_CONTROL_J
#define NANO_JUSTIFY_FKEY		KEY_F(4)
#define NANO_UNJUSTIFY_KEY		NANO_UNCUT_KEY
#define NANO_UNJUSTIFY_FKEY		NANO_UNCUT_FKEY
#define NANO_PREVLINE_KEY		NANO_CONTROL_P
#define NANO_NEXTLINE_KEY		NANO_CONTROL_N
#define NANO_FORWARD_KEY		NANO_CONTROL_F
#define NANO_BACK_KEY			NANO_CONTROL_B
#define NANO_MARK_KEY			NANO_CONTROL_6
#define NANO_MARK_ALTKEY		NANO_ALT_A
#define NANO_MARK_FKEY			KEY_F(15)
#define NANO_HOME_KEY			NANO_CONTROL_A
#define NANO_END_KEY			NANO_CONTROL_E
#define NANO_DELETE_KEY			NANO_CONTROL_D
#define NANO_BACKSPACE_KEY		NANO_CONTROL_H
#define NANO_TAB_KEY			NANO_CONTROL_I
#define NANO_INDENTMARKED_KEY		NANO_ALT_RCURLYBRACKET
#define NANO_INDENTMARKED_ALTKEY	NANO_ALT_RBRACKET
#define NANO_UNINDENTMARKED_KEY		NANO_ALT_LCURLYBRACKET
#define NANO_UNINDENTMARKED_ALTKEY	NANO_ALT_LBRACKET
#define NANO_SUSPEND_KEY		NANO_CONTROL_Z
#define NANO_ENTER_KEY			NANO_CONTROL_M
#define NANO_TOFILES_KEY		NANO_CONTROL_T
#define NANO_APPEND_KEY			NANO_ALT_A
#define NANO_PREPEND_KEY		NANO_ALT_P
#define NANO_PREVFILE_KEY		NANO_ALT_LCARET
#define NANO_NEXTFILE_KEY		NANO_ALT_RCARET
#define NANO_PREVFILE_ALTKEY		NANO_ALT_COMMA
#define NANO_NEXTFILE_ALTKEY		NANO_ALT_PERIOD
#define NANO_BRACKET_KEY		NANO_CONTROL_5
#define NANO_NEXTWORD_KEY		NANO_CONTROL_SPACE
#define NANO_PREVWORD_KEY		NANO_ALT_SPACE
#define NANO_WORDCOUNT_KEY		NANO_ALT_D
#define NANO_SCROLLUP_KEY		NANO_ALT_MINUS
#define NANO_SCROLLDOWN_KEY		NANO_ALT_PLUS
#define NANO_SCROLLUP_ALTKEY		NANO_ALT_UNDERSCORE
#define NANO_SCROLLDOWN_ALTKEY		NANO_ALT_EQUALS
#define NANO_CUTTILLEND_KEY		NANO_CONTROL_X
#define NANO_CUTTILLEND_ALTKEY		NANO_ALT_T
#define NANO_PARABEGIN_KEY		NANO_CONTROL_W
#define NANO_PARABEGIN_ALTKEY		NANO_ALT_LPARENTHESIS
#define NANO_PARABEGIN_ALTKEY2		NANO_ALT_9
#define NANO_PARAEND_KEY		NANO_CONTROL_O
#define NANO_PARAEND_ALTKEY		NANO_ALT_RPARENTHESIS
#define NANO_PARAEND_ALTKEY2		NANO_ALT_0
#define NANO_FULLJUSTIFY_KEY		NANO_CONTROL_U
#define NANO_FULLJUSTIFY_ALTKEY		NANO_ALT_J
#define NANO_VERBATIM_KEY		NANO_ALT_V

/* Toggles do not exist if NANO_TINY is defined. */
#ifndef NANO_TINY

/* No toggle at all. */
#define TOGGLE_NO_KEY			-2

/* Normal toggles. */
#define TOGGLE_NOHELP_KEY		NANO_ALT_X
#define TOGGLE_CONST_KEY		NANO_ALT_C
#define TOGGLE_MORESPACE_KEY		NANO_ALT_O
#define TOGGLE_SMOOTH_KEY		NANO_ALT_S
#define TOGGLE_WHITESPACE_KEY		NANO_ALT_P
#define TOGGLE_SYNTAX_KEY		NANO_ALT_Y
#define TOGGLE_SMARTHOME_KEY		NANO_ALT_H
#define TOGGLE_AUTOINDENT_KEY		NANO_ALT_I
#define TOGGLE_CUTTOEND_KEY		NANO_ALT_K
#define TOGGLE_WRAP_KEY			NANO_ALT_L
#define TOGGLE_TABSTOSPACES_KEY		NANO_ALT_Q
#define TOGGLE_BACKUP_KEY		NANO_ALT_B
#define TOGGLE_MULTIBUFFER_KEY		NANO_ALT_F
#define TOGGLE_MOUSE_KEY		NANO_ALT_M
#define TOGGLE_NOCONVERT_KEY		NANO_ALT_N
#define TOGGLE_SUSPEND_KEY		NANO_ALT_Z
#define TOGGLE_CASE_KEY			NANO_ALT_C
#define TOGGLE_BACKWARDS_KEY		NANO_ALT_B
#define TOGGLE_DOS_KEY			NANO_ALT_D
#define TOGGLE_MAC_KEY			NANO_ALT_M
#endif /* !NANO_TINY */

#define MAIN_VISIBLE 12

#define VIEW TRUE
#define NOVIEW FALSE

/* The minimum editor window columns and rows required for nano to work
 * correctly. */
#define MIN_EDITOR_COLS 4
#define MIN_EDITOR_ROWS 1

/* The default number of characters from the end of the line where
 * wrapping occurs. */
#define CHARS_FROM_EOL 8

/* The default width of a tab in spaces. */
#define WIDTH_OF_TAB 8

/* The maximum number of search/replace history strings saved, not
 * counting the blank lines at their ends. */
#define MAX_SEARCH_HISTORY 100

/* The maximum number of bytes buffered at one time. */
#define MAX_BUF_SIZE 128

#endif /* !NANO_H */
