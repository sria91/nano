/* $Id$ */
/**************************************************************************
 *   proto.h                                                              *
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

/* Externs */

#include <sys/stat.h>

#ifdef HAVE_REGEX_H
#include <regex.h>
#endif

#include "nano.h"

extern int editwinrows;
extern int current_x, current_y, posible_max, totlines;
extern int placewewant;
extern int mark_beginx, samelinewrap;
extern int totsize, temp_opt;
extern int fill, wrap_at, flags,tabsize;
extern int search_last_line;
extern int currslen;

extern WINDOW *edit, *topwin, *bottomwin;
extern char *filename;
extern char *answer;
extern char *hblank, *help_text;
extern char *last_search;
extern char *last_replace;
#ifndef DISABLE_OPERATINGDIR
extern char *operating_dir;
#endif
#ifndef DISABLE_SPELLER
extern  char *alt_speller;
#endif
extern struct stat fileinfo;
extern filestruct *current, *fileage, *edittop, *editbot, *filebot; 
extern filestruct *cutbuffer, *mark_beginbuf;

#ifdef ENABLE_MULTIBUFFER
extern filestruct *open_files;
#endif

extern shortcut *shortcut_list;
extern shortcut main_list[MAIN_LIST_LEN], whereis_list[WHEREIS_LIST_LEN];
extern shortcut replace_list[REPLACE_LIST_LEN], goto_list[GOTO_LIST_LEN];
extern shortcut writefile_list[WRITEFILE_LIST_LEN], insertfile_list[INSERTFILE_LIST_LEN];
extern shortcut spell_list[SPELL_LIST_LEN], replace_list_2[REPLACE_LIST_LEN];
extern shortcut help_list[HELP_LIST_LEN];
#ifndef DISABLE_BROWSER
extern shortcut browser_list[BROWSER_LIST_LEN], gotodir_list[GOTODIR_LIST_LEN];
#endif
extern shortcut *currshortcut;

#ifdef HAVE_REGEX_H
extern int use_regexp, regexp_compiled;
extern regex_t search_regexp;
extern regmatch_t regmatches[10];  
#endif

extern toggle toggles[TOGGLE_LEN];

/* Programs we want available */

char *revstrstr(char *haystack, char *needle, char *rev_start);
char *strcasestr(char *haystack, char *needle);
char *revstrcasestr(char *haystack, char *needle, char *rev_start);
char *strstrwrapper(char *haystack, char *needle, char *rev_start);
int search_init(int replacing);
int renumber(filestruct * fileptr);
int free_filestruct(filestruct * src);
int xplustabs(void);
int do_yesno(int all, int leavecursor, char *msg, ...);
int actual_x(filestruct * fileptr, int xplus);
int strlenpt(char *buf);
int statusq(int allowtabs, shortcut s[], int slen, char *def, char *msg, ...);
int write_file(char *name, int tmpfile, int append, int nonamechange);
int do_cut_text(void);
int do_uncut_text(void);
int no_help(void);
int renumber_all(void);
int open_file(char *filename, int insert, int quiet);
int do_insertfile(int loading_file);

#ifdef ENABLE_MULTIBUFFER
int add_open_file(int update, int dup_fix);
#endif

#ifndef DISABLE_OPERATINGDIR
int check_operating_dir(char *currpath, int allow_tabcomp);
#endif

int do_writeout(char *path, int exiting, int append);
int do_gotoline(long line, int save_pos);
int do_replace_loop(char *prevanswer, filestruct *begin, int *beginx,
			int wholewords, int *i);
int do_find_bracket(void);

#ifdef ENABLE_MULTIBUFFER
void do_gotopos(long line, int pos_x, int pos_y, int pos_placewewant);
#endif

/* Now in move.c */
int do_up(void);
int do_down(void);
int do_left(void);
int do_right(void);
int check_wildcard_match(const char *text, const char *pattern);

char *input_tab(char *buf, int place, int *lastWasTab, int *newplace);
char *real_dir_from_tilde(char *buf);

void shortcut_init(int unjustify);
void lowercase(char *src);
void blank_bottombars(void);
void check_wrap(filestruct * inptr, char ch);
void dump_buffer(filestruct * inptr);
void align(char **strp);
void edit_refresh(void), edit_refresh_clearok(void);
void edit_update(filestruct * fileptr, int topmidbotnone);
void update_cursor(void);
void delete_node(filestruct * fileptr);
void set_modified(void);
void dump_buffer_reverse(filestruct * inptr);
void reset_cursor(void);
void check_statblank(void);
void update_line(filestruct * fileptr, int index);
void fix_editbot(void);
void statusbar(char *msg, ...);
void blank_statusbar(void);
void titlebar(char *path);
void previous_line(void);
void center_cursor(void);
void bottombars(shortcut s[], int slen);
void blank_statusbar_refresh(void);
void *nmalloc (size_t howmuch);
void *mallocstrcpy(char *dest, char *src);
void wrap_reset(void);
void display_main_list(void);
void nano_small_msg(void);
void nano_disable_msg(void);
void do_early_abort(void);
void *nmalloc(size_t howmuch);
void *nrealloc(void *ptr, size_t howmuch);
void die(char *msg, ...);
void die_save_file(char *die_filename);
void new_file(void);
void new_magicline(void);
void splice_node(filestruct *begin, filestruct *newnode, filestruct *end);
void null_at(char *data, int index);
void page_up(void);
void blank_edit(void);
void search_init_globals(void);
void replace_abort(void);
void add_to_cutbuffer(filestruct * inptr);
void do_replace_highlight(int highlight_flag, char *word);
void nano_disabled_msg(void);
void window_init(void);
void do_mouse(void);
void print_view_warning(void);
void unlink_node(filestruct * fileptr);
void cut_marked_segment(filestruct * top, int top_x, filestruct * bot,
                        int bot_x, int destructive);

#ifdef ENABLE_NANORC
void do_rcfile(void);
#endif

#ifdef NANO_EXTRA
void do_credits(void);
#endif


int do_writeout_void(void), do_exit(void), do_gotoline_void(void);
int do_insertfile_void(void), do_search(void);

#ifdef ENABLE_MULTIBUFFER
int load_open_file(void), close_open_file(void);
#endif

int do_page_up(void), do_page_down(void);
int do_cursorpos(void), do_spell(void);
int do_up(void), do_down (void), do_right(void), do_left (void);
int do_home(void), do_end(void), total_refresh(void), do_mark(void);
int do_delete(void), do_backspace(void), do_tab(void), do_justify(void);
int do_first_line(void), do_last_line(void);
int do_replace(void), do_help(void), do_enter_void(void);
int keypad_on(WINDOW * win, int newval);

#ifdef ENABLE_MULTIBUFFER
int open_file_dup_fix(int update);
int open_prevfile(int closing_file), open_nextfile(int closing_file);
#endif

char *charalloc (size_t howmuch);

#if defined (ENABLE_MULTIBUFFER) || !defined (ENABLE_OPERATINGDIR)
char *get_full_path(char *origpath);
#endif

#ifndef DISABLE_BROWSER
char *do_browser(char *path);
struct stat filestat(const char *path);
char *do_browse_from(char *inpath);
#endif

#ifdef ENABLE_COLOR
int do_colorinit(void);
void color_on(WINDOW *win, int whatever);
void color_off(WINDOW *win, int whatever);

extern colorstruct colors[NUM_NCOLORS];
#endif /* ENABLE_COLOR */

RETSIGTYPE main_loop (int junk);

filestruct *copy_node(filestruct * src);
filestruct *copy_filestruct(filestruct * src);
filestruct *make_new_node(filestruct * prevnode);
filestruct *findnextstr(int quiet, int bracket_mode, filestruct * begin,
			int beginx, char *needle);

#ifdef ENABLE_MULTIBUFFER
filestruct *open_file_dup_search(int update);
#endif
