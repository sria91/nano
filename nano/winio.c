/* $Id$ */
/**************************************************************************
 *   winio.c                                                              *
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

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "proto.h"
#include "nano.h"

#ifndef NANO_SMALL
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif


/* winio.c statics */
static int statblank = 0;	/* Number of keystrokes left after
				   we call statubar() before we
				   actually blank the statusbar */

/* Local Function Prototypes for only winio.c */
inline int get_page_from_virtual(int virtual);
inline int get_page_start_virtual(int page);
inline int get_page_end_virtual(int page);

/* Window I/O */

int do_first_line(void)
{
    current = fileage;
    placewewant = 0;
    current_x = 0;
    edit_update(current, CENTER);
    return 1;
}

int do_last_line(void)
{
    current = filebot;
    placewewant = 0;
    current_x = 0;
    edit_update(current, CENTER);
    return 1;
}

/* Like xplustabs, but for a specifc index of a speficific filestruct */
int xpt(filestruct * fileptr, int index)
{
    int i, tabs = 0;

    if (fileptr == NULL || fileptr->data == NULL)
	return 0;

    for (i = 0; i < index && fileptr->data[i] != 0; i++) {
	tabs++;

	if (fileptr->data[i] == NANO_CONTROL_I) {
	    if (tabs % tabsize == 0);
	    else
		tabs += tabsize - (tabs % tabsize);
	} else if (fileptr->data[i] & 0x80)
	    /* Make 8 bit chars only 1 collumn! */
	    ;
	else if (fileptr->data[i] < 32)
	    tabs++;
    }

    return tabs;
}


/* Return the actual place on the screen of current->data[current_x], which 
   should always be > current_x */
int xplustabs(void)
{
    return xpt(current, current_x);
}


/* Return what current_x should be, given xplustabs() for the line, 
 * given a start position in the filestruct's data */
int actual_x_from_start(filestruct * fileptr, int xplus, int start)
{
    int i, tot = 1;

    if (fileptr == NULL || fileptr->data == NULL)
	return 0;

    for (i = start; tot <= xplus && fileptr->data[i] != 0; i++, tot++)
	if (fileptr->data[i] == NANO_CONTROL_I) {
	    if (tot % tabsize == 0)
		tot++;
	    else
		tot += tabsize - (tot % tabsize);
	} else if (fileptr->data[i] & 0x80)
	    tot++;		/* Make 8 bit chars only 1 column (again) */
	else if (fileptr->data[i] < 32)
	    tot += 2;

#ifdef DEBUG
    fprintf(stderr, _("actual_x_from_start for xplus=%d returned %d\n"),
	    xplus, i);
#endif
    return i - start;
}

/* Opposite of xplustabs */
int actual_x(filestruct * fileptr, int xplus)
{
    return actual_x_from_start(fileptr, xplus, 0);
}

/* a strlen with tabs factored in, similar to xplustabs() */
int strlenpt(char *buf)
{
    int i, tabs = 0;

    if (buf == NULL)
	return 0;

    for (i = 0; buf[i] != 0; i++) {
	tabs++;

	if (buf[i] == NANO_CONTROL_I) {
	    if (tabs % tabsize == 0);
	    else
		tabs += tabsize - (tabs % tabsize);
	} else if (buf[i] & 0x80)
	    /* Make 8 bit chars only 1 collumn! */
	    ;
	else if (buf[i] < 32)
	    tabs++;
    }

    return tabs;
}


/* resets current_y based on the position of current and puts the cursor at 
   (current_y, current_x) */
void reset_cursor(void)
{
    filestruct *ptr = edittop;
    int x;

    current_y = 0;

    while (ptr != current && ptr != editbot && ptr->next != NULL) {
	ptr = ptr->next;
	current_y++;
    }

    x = xplustabs();
    if (x <= COLS - 2)
	wmove(edit, current_y, x);
    else
	wmove(edit, current_y, x -
	      get_page_start_virtual(get_page_from_virtual(x)));

}

void blank_bottombars(void)
{
    int i = no_help()? 3 : 1;

    for (; i <= 2; i++)
	mvwaddstr(bottomwin, i, 0, hblank);

}

void blank_edit(void)
{
    int i;
    for (i = 0; i <= editwinrows - 1; i++)
	mvwaddstr(edit, i, 0, hblank);
}


void blank_statusbar(void)
{
    mvwaddstr(bottomwin, 0, 0, hblank);
}

void blank_statusbar_refresh(void)
{
    blank_statusbar();
    wrefresh(bottomwin);
}

void check_statblank(void)
{

    if (statblank > 1)
	statblank--;
    else if (statblank == 1 && !ISSET(CONSTUPDATE)) {
	statblank--;
	blank_statusbar_refresh();
    }
}

/* Repaint the statusbar when getting a character in nanogetstr */
void nanoget_repaint(char *buf, char *inputbuf, int x)
{
    int len = strlen(buf);
    int wid = COLS - len;

#ifdef ENABLE_COLOR
    color_on(bottomwin, COLOR_STATUSBAR);
#endif
    blank_statusbar();

    if (x <= COLS - 1) {
	/* Black magic */
	buf[len - 1] = ' ';

	mvwaddstr(bottomwin, 0, 0, buf);
	waddnstr(bottomwin, inputbuf, wid);
	wmove(bottomwin, 0, (x % COLS));
    }
    else {
	/* Black magic */
	buf[len - 1] = '$';

	mvwaddstr(bottomwin, 0, 0, buf);
	waddnstr(bottomwin, &inputbuf[wid * ((x - len) / (wid))], wid);
	wmove(bottomwin, 0, ((x - len) % wid) + len);
    }

#ifdef ENABLE_COLOR
    color_off(bottomwin, COLOR_STATUSBAR);
#endif
}

/* Get the input from the kb, this should only be called from statusq */
int nanogetstr(int allowtabs, char *buf, char *def, shortcut s[], int slen, 
	       int start_x)
{
    int kbinput = 0, j = 0, x = 0, xend;
    int x_left = 0, inputlen, tabbed = 0;
    char *inputbuf;
#ifndef DISABLE_TABCOMP
    int shift = 0;
#endif
    
    inputbuf = nmalloc(strlen(def) + 1);
    inputbuf[0] = 0;

    x_left = strlen(buf);
    x = strlen(def) + x_left;

    currshortcut = s;
    currslen = slen;
    /* Get the input! */
    if (strlen(def) > 0)
	strcpy(inputbuf, def);

    nanoget_repaint(buf, inputbuf, x);

    /* Make sure any editor screen updates are displayed before getting input */
    wrefresh(edit);

    while ((kbinput = wgetch(bottomwin)) != 13) {
	for (j = 0; j <= slen - 1; j++) {
#ifdef DEBUG
	    fprintf(stderr, _("Aha! \'%c\' (%d)\n"), kbinput, kbinput);
#endif

	    if (kbinput == s[j].val) {

		/* We shouldn't discard the answer it gave, just because
		   we hit a keystroke, GEEZ! */
		answer = mallocstrcpy(answer, inputbuf);
		free(inputbuf);
		return s[j].val;
	    }
	}
	xend = strlen(buf) + strlen(inputbuf);

	if (kbinput != '\t')
	    tabbed = 0;

	switch (kbinput) {

	/* Stuff we want to equate with <enter>, ASCII 13 */
	case 343:
	    ungetch(13);	/* Enter on iris-ansi $TERM, sometimes */
	    break;
	/* Stuff we want to ignore */
#ifdef PDCURSES
	case 541:
	case 542:
	case 543:			/* Right ctrl again */
	case 544:
	case 545: 			/* Right alt again */
	    break;
#endif
#ifndef DISABLE_MOUSE
#ifdef NCURSES_MOUSE_VERSION
	case KEY_MOUSE:
	    do_mouse();
	    break;
#endif
#endif
	case KEY_HOME:
	    x = x_left;
	    nanoget_repaint(buf, inputbuf, x);
	    break;
	case KEY_END:
	    x = x_left + strlen(inputbuf);
	    nanoget_repaint(buf, inputbuf, x);
	    break;
	case KEY_RIGHT:
	case NANO_FORWARD_KEY:

	    if (x < xend)
		x++;
	    wmove(bottomwin, 0, x);
	    break;
	case NANO_CONTROL_D:
	    if (strlen(inputbuf) > 0 && (x - x_left) != strlen(inputbuf)) {
		memmove(inputbuf + (x - x_left),
			inputbuf + (x - x_left) + 1,
			strlen(inputbuf) - (x - x_left) - 1);
		inputbuf[strlen(inputbuf) - 1] = 0;
	    }
	    nanoget_repaint(buf, inputbuf, x);
	    break;
	case NANO_CONTROL_K:
	case NANO_CONTROL_U:
	    *inputbuf = 0;
	    x = x_left;
	    nanoget_repaint(buf, inputbuf, x);
	    break;
	case KEY_BACKSPACE:
	case KEY_DC:
	case 127:
	case NANO_CONTROL_H:
	    if (strlen(inputbuf) > 0) {
		if (x == (x_left + strlen(inputbuf)))
		    inputbuf[strlen(inputbuf) - 1] = 0;
		else if (x - x_left) {
		    memmove(inputbuf + (x - x_left) - 1,
			    inputbuf + (x - x_left),
			    strlen(inputbuf) - (x - x_left));
		    inputbuf[strlen(inputbuf) - 1] = 0;
		}
	    }
	    if (x > strlen(buf))
		x--;
	    nanoget_repaint(buf, inputbuf, x);
	    break;
#ifndef DISABLE_TABCOMP
	case NANO_CONTROL_I:
	    if (allowtabs) {
		shift = 0;
		inputbuf = input_tab(inputbuf, (x - x_left), 
				&tabbed, &shift);
		x += shift;
		if (x - x_left > strlen(inputbuf))
		    x = strlen(inputbuf) + x_left;
		nanoget_repaint(buf, inputbuf, x);
	    }
	    break;
#endif
	case KEY_LEFT:
	case NANO_BACK_KEY:
	    if (x > strlen(buf))
		x--;
	    wmove(bottomwin, 0, x);
	    break;
	case KEY_UP:
	case KEY_DOWN:
	    break;

	case 27:
	    switch (kbinput = wgetch(edit)) {
	    case 79:
		switch (kbinput = wgetch(edit)) {
		case 70:
		    x = x_left + strlen(inputbuf);
		    nanoget_repaint(buf, inputbuf, x);
		    break;
		case 72:
		    x = x_left;
		    nanoget_repaint(buf, inputbuf, x);
		    break;
		}
		break;
	    case 91:
		switch (kbinput = wgetch(edit)) {
		case 'C':
		    if (x < xend)
			x++;
		    wmove(bottomwin, 0, x);
		    break;
		case 'D':
		    if (x > strlen(buf))
			x--;
		    wmove(bottomwin, 0, x);
		    break;
		case 49:
		    x = x_left;
		    nanoget_repaint(buf, inputbuf, x);
		    goto skip_126;
		case 51:
		    if (strlen(inputbuf) > 0
			&& (x - x_left) != strlen(inputbuf)) {
			memmove(inputbuf + (x - x_left),
				inputbuf + (x - x_left) + 1,
				strlen(inputbuf) - (x - x_left) - 1);
			inputbuf[strlen(inputbuf) - 1] = 0;
		    }
		    nanoget_repaint(buf, inputbuf, x);
		    goto skip_126;
		case 52:
		    x = x_left + strlen(inputbuf);
		    nanoget_repaint(buf, inputbuf, x);
		    goto skip_126;
		  skip_126:
		    nodelay(edit, TRUE);
		    kbinput = wgetch(edit);
		    if (kbinput == 126 || kbinput == ERR)
			kbinput = -1;
		    nodelay(edit, FALSE);
		    break;
		}
	    }
	    nanoget_repaint(buf, inputbuf, x);
	    break;

	default:
	    if (kbinput < 32)
		break;

	    inputlen = strlen(inputbuf);
	    inputbuf = nrealloc(inputbuf, inputlen + 2);

	    memmove(&inputbuf[x - x_left + 1], 
			&inputbuf[x - x_left],
                        inputlen - (x - x_left) + 1);
	    inputbuf[x - x_left] = kbinput;

	    x++;

	    nanoget_repaint(buf, inputbuf, x);
#ifdef DEBUG
	    fprintf(stderr, _("input \'%c\' (%d)\n"), kbinput, kbinput);
#endif
	}
	wrefresh(bottomwin);
    }

    answer = mallocstrcpy(answer, inputbuf);
    free(inputbuf);

    /* Now that the text is editable instead of bracketed, we have to 
       check for answer == def, instead of answer == "" */
    if  (((ISSET(PICO_MODE)) && !strcmp(answer, "")) || 
	((!ISSET(PICO_MODE)) && !strcmp(answer, def)))
	return -2;
    else
	return 0;
}

void horizbar(WINDOW * win, int y)
{
    wattron(win, A_REVERSE);
    mvwaddstr(win, 0, 0, hblank);
    wattroff(win, A_REVERSE);
}

void titlebar(char *path)
{
    int namelen, space;
    char *what = path;

    if (path == NULL)
	what = filename;

#ifdef ENABLE_COLOR
    color_on(topwin, COLOR_TITLEBAR);
    mvwaddstr(topwin, 0, 0, hblank);
#else
    horizbar(topwin, 0);
    wattron(topwin, A_REVERSE);
#endif


    mvwaddstr(topwin, 0, 3, VERMSG);

    space = COLS - strlen(VERMSG) - strlen(VERSION) - 21;

    namelen = strlen(what);

    if (!strcmp(what, ""))
	mvwaddstr(topwin, 0, COLS / 2 - 6, _("New Buffer"));
    else {
	if (namelen > space) {
	    if (path == NULL)
		waddstr(topwin, _("  File: ..."));
	    else
		waddstr(topwin, _("   DIR: ..."));
	    waddstr(topwin, &what[namelen - space]);
	} else {
	    if (path == NULL)
		mvwaddstr(topwin, 0, COLS / 2 - (namelen / 2 + 1), "File: ");
	    else
	 	mvwaddstr(topwin, 0, COLS / 2 - (namelen / 2 + 1), " DIR: ");
	    waddstr(topwin, what);
	}
    }
    if (ISSET(MODIFIED))
	mvwaddstr(topwin, 0, COLS - 10, _("Modified"));


#ifdef ENABLE_COLOR
    color_off(topwin, COLOR_TITLEBAR);
#else
    wattroff(topwin, A_REVERSE);
#endif

    wrefresh(topwin);
    reset_cursor();
}

void onekey(char *keystroke, char *desc)
{
    char description[80];

    snprintf(description, 12, " %-10s", desc);
    wattron(bottomwin, A_REVERSE);
    waddstr(bottomwin, keystroke);
    wattroff(bottomwin, A_REVERSE);
    waddstr(bottomwin, description);
}

void clear_bottomwin(void)
{
    if (ISSET(NO_HELP))
	return;

    mvwaddstr(bottomwin, 1, 0, hblank);
    mvwaddstr(bottomwin, 2, 0, hblank);
}

void bottombars(shortcut s[], int slen)
{
    int i, j, k;
    char keystr[10];

    if (ISSET(NO_HELP))
	return;

#ifdef ENABLE_COLOR    
    color_on(bottomwin, COLOR_BOTTOMBARS);
#endif

    /* Determine how many extra spaces are needed to fill the bottom of the screen */
    k = COLS / 6 - 13;

    clear_bottomwin();
    wmove(bottomwin, 1, 0);
    for (i = 0; i <= slen - 1; i += 2) {
	snprintf(keystr, 10, "^%c", s[i].val + 64);
	onekey(keystr, s[i].desc);

	for (j = 0; j < k; j++)
	    waddch(bottomwin, ' ');
    }

    wmove(bottomwin, 2, 0);
    for (i = 1; i <= slen - 1; i += 2) {
	snprintf(keystr, 10, "^%c", s[i].val + 64);
	onekey(keystr, s[i].desc);

	for (j = 0; j < k; j++)
	    waddch(bottomwin, ' ');
    }

#ifdef ENABLE_COLOR    
    color_off(bottomwin, COLOR_BOTTOMBARS);
#endif

    wrefresh(bottomwin);

}

/* If modified is not already set, set it and update titlebar */
void set_modified(void)
{
    if (!ISSET(MODIFIED)) {
	SET(MODIFIED);
	titlebar(NULL);
	wrefresh(topwin);
    }
}

/* And so start the display update routines */
/* Given a column, this returns the "page" it is on  */
/* "page" in the case of the display columns, means which set of 80 */
/* characters is viewable (ie: page 1 shows from 1 to COLS) */
inline int get_page_from_virtual(int virtual)
{
    int page = 2;

    if (virtual <= COLS - 2)
	return 1;
    virtual -= (COLS - 2);

    while (virtual > COLS - 2 - 7) {
	virtual -= (COLS - 2 - 7);
	page++;
    }

    return page;
}

/* The inverse of the above function */
inline int get_page_start_virtual(int page)
{
    int virtual;
    virtual = --page * (COLS - 7);
    if (page)
	virtual -= 2 * page - 1;
    return virtual;
}

inline int get_page_end_virtual(int page)
{
    return get_page_start_virtual(page) + COLS - 1;
}

#ifndef NANO_SMALL
/* This takes care of the case where there is a mark that covers only */
/* the current line. */

/* It expects a line with no tab characers (ie: the type that edit_add */
/* deals with */
void add_marked_sameline(int begin, int end, filestruct * fileptr, int y,
			 int virt_cur_x, int this_page)
{
    /*
     * The general idea is to break the line up into 3 sections: before
     * the mark, the mark, and after the mark.  We then paint each in
     * turn (for those that are currently visible, of course
     *
     * 3 start points: 0 -> begin, begin->end, end->strlen(data)
     *    in data    :    pre          sel           post        
     */
    int this_page_start = get_page_start_virtual(this_page),
	this_page_end = get_page_end_virtual(this_page);

    /* likewise, 3 data lengths */
    int pre_data_len = begin, sel_data_len = end - begin, post_data_len = 0;	/* Determined from the other two */

    /* now fix the start locations & lengths according to the cursor's 
     * position (ie: our page) */
    if (pre_data_len < this_page_start)
	pre_data_len = 0;
    else
	pre_data_len -= this_page_start;

    if (begin < this_page_start)
	begin = this_page_start;

    if (end < this_page_start)
	end = this_page_start;

    if (begin > this_page_end)
	begin = this_page_end;

    if (end > this_page_end)
	end = this_page_end;

    /* Now calculate the lengths */
    sel_data_len = end - begin;
    post_data_len = this_page_end - end;

    /* Paint this line! */
    mvwaddnstr(edit, y, 0, &fileptr->data[this_page_start], pre_data_len);

#ifdef ENABLE_COLOR
    color_on(edit, COLOR_MARKER);
#else
    wattron(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

    mvwaddnstr(edit, y, begin - this_page_start,
	       &fileptr->data[begin], sel_data_len);

#ifdef ENABLE_COLOR
    color_off(edit, COLOR_MARKER);
#else
    wattroff(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

    mvwaddnstr(edit, y, end - this_page_start,
	       &fileptr->data[end], post_data_len);
}
#endif

/* edit_add takes care of the job of actually painting a line into the
 * edit window.
 * 
 * Called only from update_line.  Expects a converted-to-not-have-tabs
 * line */
void edit_add(filestruct * fileptr, int yval, int start, int virt_cur_x,
	      int virt_mark_beginx, int this_page)
{
#ifndef NANO_SMALL
    /* There are quite a few cases that could take place, we'll deal
     * with them each in turn */
    if (ISSET(MARK_ISSET)
	&& !((fileptr->lineno > mark_beginbuf->lineno
	      && fileptr->lineno > current->lineno)
	     || (fileptr->lineno < mark_beginbuf->lineno
		 && fileptr->lineno < current->lineno))) {
	/* If we get here we are on a line that is atleast
	 * partially selected.  The lineno checks above determined
	 * that */
	if (fileptr != mark_beginbuf && fileptr != current) {
	    /* We are on a completely marked line, paint it all
	     * inverse */
#ifdef ENABLE_COLOR
	    color_on(edit, COLOR_MARKER);
#else
	    wattron(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

	    mvwaddnstr(edit, yval, 0, fileptr->data, COLS);

#ifdef ENABLE_COLOR
	    color_off(edit, COLOR_MARKER);
#else
	    wattroff(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

	} else if (fileptr == mark_beginbuf && fileptr == current) {
	    /* Special case, we're still on the same line we started
	     * marking -- so we call our helper function */
	    if (virt_cur_x < virt_mark_beginx) {
		/* To the right of us is marked */
		add_marked_sameline(virt_cur_x, virt_mark_beginx,
				    fileptr, yval, virt_cur_x, this_page);
	    } else {
		/* To the left of us is marked */
		add_marked_sameline(virt_mark_beginx, virt_cur_x,
				    fileptr, yval, virt_cur_x, this_page);
	    }
	} else if (fileptr == mark_beginbuf) {
	    /*
	     * we're updating the line that was first marked
	     * but we're not currently on it.  So we want to
	     * figure out which half to invert based on our
	     * relative line numbers.
	     *
	     * i.e. If we're above the "beginbuf" line, we want to
	     * mark the left side.  Otherwise we're below, so we
	     * mark the right
	     */
	    int target;

	    if (mark_beginbuf->lineno > current->lineno) {
#ifdef ENABLE_COLOR
		color_on(edit, COLOR_MARKER);
#else
		wattron(edit, A_REVERSE);
#endif /* ENABLE_COLOR */
	    }

	    target =
		(virt_mark_beginx <
		 COLS - 1) ? virt_mark_beginx : COLS - 1;

	    mvwaddnstr(edit, yval, 0, fileptr->data, target);

	    if (mark_beginbuf->lineno < current->lineno) {

#ifdef ENABLE_COLOR
		color_on(edit, COLOR_MARKER);
#else
		wattron(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

	    } else {

#ifdef ENABLE_COLOR
		color_off(edit, COLOR_MARKER);
#else
		wattroff(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

	    }

	    target = (COLS - 1) - virt_mark_beginx;
	    if (target < 0)
		target = 0;

	    mvwaddnstr(edit, yval, virt_mark_beginx,
		       &fileptr->data[virt_mark_beginx], target);

	    if (mark_beginbuf->lineno < current->lineno) {

#ifdef ENABLE_COLOR
		color_off(edit, COLOR_MARKER);
#else
		wattroff(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

	    }

	} else if (fileptr == current) {
	    /* we're on the cursor's line, but it's not the first
	     * one we marked.  Similar to the previous logic. */
	    int this_page_start = get_page_start_virtual(this_page),
		this_page_end = get_page_end_virtual(this_page);

	    if (mark_beginbuf->lineno < current->lineno) {

#ifdef ENABLE_COLOR
		color_on(edit, COLOR_MARKER);
#else
		wattron(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

	    }

	    if (virt_cur_x > COLS - 2) {
		mvwaddnstr(edit, yval, 0,
			   &fileptr->data[this_page_start],
			   virt_cur_x - this_page_start);
	    } else {
		mvwaddnstr(edit, yval, 0, fileptr->data, virt_cur_x);
	    }

	    if (mark_beginbuf->lineno > current->lineno) {

#ifdef ENABLE_COLOR
		color_on(edit, COLOR_MARKER);
#else
		wattron(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

	    } else {

#ifdef ENABLE_COLOR
		color_off(edit, COLOR_MARKER);
#else
		wattroff(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

	    }

	    if (virt_cur_x > COLS - 2)
		mvwaddnstr(edit, yval, virt_cur_x - this_page_start,
			   &fileptr->data[virt_cur_x],
			   this_page_end - virt_cur_x);
	    else
		mvwaddnstr(edit, yval, virt_cur_x,
			   &fileptr->data[virt_cur_x], COLS - virt_cur_x);

	    if (mark_beginbuf->lineno > current->lineno) {

#ifdef ENABLE_COLOR
		color_off(edit, COLOR_MARKER);
#else
		wattroff(edit, A_REVERSE);
#endif /* ENABLE_COLOR */

	    }
	}

    } else
#endif
	/* Just paint the string (no mark on this line) */
	mvwaddnstr(edit, yval, 0, &fileptr->data[start],
		   get_page_end_virtual(this_page) - start + 1);
}

/*
 * Just update one line in the edit buffer.  Basically a wrapper for
 * edit_add
 *
 * index gives is a place in the string to update starting from.
 * Likely args are current_x or 0.
 */
void update_line(filestruct * fileptr, int index)
{
    filestruct *filetmp;
    int line = 0, col = 0;
    int virt_cur_x = current_x, virt_mark_beginx = mark_beginx;
    char *realdata, *tmp;
    int i, pos, len, page;

    if (!fileptr)
	return;

    /* First, blank out the line (at a minimum) */
    for (filetmp = edittop; filetmp != fileptr && filetmp != editbot;
	 filetmp = filetmp->next)
	line++;

    mvwaddstr(edit, line, 0, hblank);

    /* Next, convert all the tabs to spaces so everything else is easy */
    index = xpt(fileptr, index);

    realdata = fileptr->data;
    len = strlen(realdata);
    fileptr->data = nmalloc(xpt(fileptr, len) + 1);

    pos = 0;
    for (i = 0; i < len; i++) {
	if (realdata[i] == '\t') {
	    do {
		fileptr->data[pos++] = ' ';
		if (i < current_x)
		    virt_cur_x++;
		if (i < mark_beginx)
		    virt_mark_beginx++;
	    } while (pos % tabsize);
	    /* must decrement once to account for tab-is-one-character */
	    if (i < current_x)
		virt_cur_x--;
	    if (i < mark_beginx)
		virt_mark_beginx--;
	} else if (realdata[i] >= 1 && realdata[i] <= 26) {
	    /* Treat control characters as ^letter */
	    fileptr->data[pos++] = '^';
	    fileptr->data[pos++] = realdata[i] + 64;
	} else {
	    fileptr->data[pos++] = realdata[i];
	}
    }

    fileptr->data[pos] = '\0';

    /* Now, Paint the line */
    if (current == fileptr && index > COLS - 2) {
	/* This handles when the current line is beyond COLS */
	/* It requires figureing out what page we're at      */
	page = get_page_from_virtual(index);
	col = get_page_start_virtual(page);

	edit_add(filetmp, line, col, virt_cur_x, virt_mark_beginx, page);
	mvwaddch(edit, line, 0, '$');

	if (strlenpt(fileptr->data) > get_page_end_virtual(page) + 1)
	    mvwaddch(edit, line, COLS - 1, '$');
    } else {
	/* It's not the current line means that it's at x=0 and page=1 */
	/* If it is the current line, then we're in the same boat      */
	edit_add(filetmp, line, 0, virt_cur_x, virt_mark_beginx, 1);

	if (strlenpt(&filetmp->data[col]) > COLS)
	    mvwaddch(edit, line, COLS - 1, '$');
    }

    /* Clean up our mess */
    tmp = fileptr->data;
    fileptr->data = realdata;
    free(tmp);
}

void center_cursor(void)
{
    current_y = editwinrows / 2;
    wmove(edit, current_y, current_x);
}

/* Refresh the screen without changing the position of lines */
void edit_refresh(void)
{
    static int noloop = 0;
    int lines = 0, i = 0, currentcheck = 0;
    filestruct *temp, *hold = current;

    if (current == NULL)
	return;

    temp = edittop;

    while (lines <= editwinrows - 1 && lines <= totlines && temp != NULL) {
	hold = temp;
	update_line(temp, current_x);
	if (temp == current)
	    currentcheck = 1;

	temp = temp->next;
	lines++;
    }
    /* If noloop == 1, then we already did an edit_update without finishing
       this function.  So we don't run edit_update again */
    if (!currentcheck && !noloop) {	/* Then current has run off the screen... */
	edit_update(current, CENTER);
	noloop = 1;
    } else if (noloop)
	noloop = 0;

    if (lines <= editwinrows - 1)
	while (lines <= editwinrows - 1) {
	    mvwaddstr(edit, lines, i, hblank);
	    lines++;
	}
    if (temp == NULL)
	editbot = hold;
    else
	editbot = temp;
}

/*
 * Same as above, but touch the window first so everything is redrawn.
 */
void edit_refresh_clearok(void)
{
    clearok(edit, TRUE);
    edit_refresh();
    clearok(edit, FALSE);
}

/*
 * Nice generic routine to update the edit buffer given a pointer to the
 * file struct =) 
 */
void edit_update(filestruct * fileptr, int topmidbot)
{
    int i = 0;
    filestruct *temp;

    if (fileptr == NULL)
	return;

    temp = fileptr;
    if (topmidbot == 2);
    else if (topmidbot == 0)
	for (i = 0; i <= editwinrows - 1 && temp->prev != NULL; i++)
	    temp = temp->prev;
    else
	for (i = 0; i <= editwinrows / 2 && temp->prev != NULL; i++)
	    temp = temp->prev;

    edittop = temp;
    fix_editbot();

    edit_refresh();
}

/* This function updates current based on where current_y is, reset_cursor 
   does the opposite */
void update_cursor(void)
{
    int i = 0;

#ifdef DEBUG
    fprintf(stderr, _("Moved to (%d, %d) in edit buffer\n"), current_y,
	    current_x);
#endif

    current = edittop;
    while (i <= current_y - 1 && current->next != NULL) {
	current = current->next;
	i++;
    }

#ifdef DEBUG
    fprintf(stderr, _("current->data = \"%s\"\n"), current->data);
#endif

}

/*
 * Ask a question on the statusbar.  Answer will be stored in answer
 * global.  Returns -1 on aborted enter, -2 on a blank string, and 0
 * otherwise, the valid shortcut key caught, Def is any editable text we
 * want to put up by default.
 *
 * New arg tabs tells whether or not to allow tab completion.
 */
int statusq(int tabs, shortcut s[], int slen, char *def, char *msg, ...)
{
    va_list ap;
    char foo[133];
    int ret;

    bottombars(s, slen);

    va_start(ap, msg);
    vsnprintf(foo, 132, msg, ap);
    va_end(ap);
    strncat(foo, ": ", 132);

#ifdef ENABLE_COLOR
    color_on(bottomwin, COLOR_STATUSBAR);
#else
    wattron(bottomwin, A_REVERSE);
#endif


    ret = nanogetstr(tabs, foo, def, s, slen, (strlen(foo) + 3));

#ifdef ENABLE_COLOR
    color_off(bottomwin, COLOR_STATUSBAR);
#else
    wattroff(bottomwin, A_REVERSE);
#endif


    switch (ret) {

    case NANO_FIRSTLINE_KEY:
	do_first_line();
	break;
    case NANO_LASTLINE_KEY:
	do_last_line();
	break;
    case NANO_CANCEL_KEY:
	return -1;
    default:
	blank_statusbar();
    }

#ifdef DEBUG
    fprintf(stderr, _("I got \"%s\"\n"), answer);
#endif

    return ret;
}

/*
 * Ask a simple yes/no question on the statusbar.  Returns 1 for Y, 0 for
 * N, 2 for All (if all is non-zero when passed in) and -1 for abort (^C)
 */
int do_yesno(int all, int leavecursor, char *msg, ...)
{
    va_list ap;
    char foo[133];
    int kbinput, ok = -1, i;
    char *yesstr;		/* String of yes characters accepted */
    char *nostr;		/* Same for no */
    char *allstr;		/* And all, surprise! */
    char shortstr[5];		/* Temp string for above */
#ifndef DISABLE_MOUSE
#ifdef NCURSES_MOUSE_VERSION
    MEVENT mevent;
#endif
#endif


    /* Yes, no and all are strings of any length.  Each string consists of
	all characters accepted as a valid character for that value.
	The first value will be the one displayed in the shortcuts. */
    yesstr = _("Yy");
    nostr = _("Nn");
    allstr = _("Aa");

    /* Write the bottom of the screen */
    clear_bottomwin();

#ifdef ENABLE_COLOR
    color_on(bottomwin, COLOR_BOTTOMBARS);
#endif

    /* Remove gettext call for keybindings until we clear the thing up */
    if (!ISSET(NO_HELP)) {
	wmove(bottomwin, 1, 0);

	snprintf(shortstr, 3, " %c", yesstr[0]);
	onekey(shortstr, _("Yes"));

	if (all) {
	    snprintf(shortstr, 3, " %c", allstr[0]);
	    onekey(shortstr, _("All"));
	}
	wmove(bottomwin, 2, 0);

	snprintf(shortstr, 3, " %c", nostr[0]);
	onekey(shortstr, _("No"));

	onekey("^C", _("Cancel"));
    }
    va_start(ap, msg);
    vsnprintf(foo, 132, msg, ap);
    va_end(ap);

#ifdef ENABLE_COLOR
    color_off(bottomwin, COLOR_BOTTOMBARS);
    color_on(bottomwin, COLOR_STATUSBAR);
#else
    wattron(bottomwin, A_REVERSE);
#endif /* ENABLE_COLOR */

    blank_statusbar();
    mvwaddstr(bottomwin, 0, 0, foo);

#ifdef ENABLE_COLOR
    color_off(bottomwin, COLOR_STATUSBAR);
#else
    wattroff(bottomwin, A_REVERSE);
#endif

    wrefresh(bottomwin);

    if (leavecursor == 1)
	reset_cursor();

    while (ok == -1) {
	kbinput = wgetch(edit);

	switch (kbinput) {
#ifndef DISABLE_MOUSE
#ifdef NCURSES_MOUSE_VERSION
	case KEY_MOUSE:

	    /* Look ma!  We get to duplicate lots of code from do_mouse!! */
	    if (getmouse(&mevent) == ERR)
		break;
	    if (!wenclose(bottomwin, mevent.y, mevent.x) || ISSET(NO_HELP))
		break;
	    mevent.y -= editwinrows + 3;
	    if (mevent.y < 0)
		break;
	    else {

		/* Rather than a bunch of if statements, set up a matrix
		   of possible return keystrokes based on the x and y values */
		if (all) {
		    char yesnosquare[2][2] = {
			{yesstr[0], allstr[0]}, 
			{nostr[0], NANO_CONTROL_C }};

		    ungetch(yesnosquare[mevent.y][mevent.x/(COLS/6)]);
		} else {
		    char yesnosquare[2][2] = {
			{yesstr[0], '\0'},
			{nostr[0], NANO_CONTROL_C }};

		    ungetch(yesnosquare[mevent.y][mevent.x/(COLS/6)]);
		}
	    }
	    break;
#endif
#endif
	case NANO_CONTROL_C:
	    ok = -2;
	    break;
	default:

	    /* Look for the kbinput in the yes, no and (optinally) all str */
	    for (i = 0; yesstr[i] != 0 && yesstr[i] != kbinput; i++)
		;
	    if (yesstr[i] != 0) {
		ok = 1;
	 	break;
	    }

	    for (i = 0; nostr[i] != 0 && nostr[i] != kbinput; i++)
		;
	    if (nostr[i] != 0) {
		ok = 0;
	 	break;
	    }

	    if (all) {
	        for (i = 0; allstr[i] != 0 && allstr[i] != kbinput; i++)
		    ;
		if (allstr[i] != 0) {
		    ok = 2;
	 	    break;
		}
	    }
	}
    }

    /* Then blank the screen */
    blank_statusbar_refresh();

    if (ok == -2)
	return -1;
    else
	return ok;
}

void statusbar(char *msg, ...)
{
    va_list ap;
    char foo[133];
    int start_x = 0;

    va_start(ap, msg);
    vsnprintf(foo, 132, msg, ap);
    va_end(ap);

    start_x = COLS / 2 - strlen(foo) / 2 - 1;

    /* Blank out line */
    blank_statusbar();

    wmove(bottomwin, 0, start_x);

#ifdef ENABLE_COLOR
    color_on(bottomwin, COLOR_STATUSBAR);
#else
    wattron(bottomwin, A_REVERSE);
#endif

    waddstr(bottomwin, "[ ");
    waddstr(bottomwin, foo);
    waddstr(bottomwin, " ]");

#ifdef ENABLE_COLOR
    color_off(bottomwin, COLOR_STATUSBAR);
#else
    wattroff(bottomwin, A_REVERSE);
#endif

    wrefresh(bottomwin);

    if (ISSET(CONSTUPDATE))
	statblank = 1;
    else
	statblank = 25;
}

void display_main_list(void)
{
    bottombars(main_list, MAIN_VISIBLE);
}

int total_refresh(void)
{
    clearok(edit, TRUE);
    clearok(topwin, TRUE);
    clearok(bottomwin, TRUE);
    wnoutrefresh(edit);
    wnoutrefresh(topwin);
    wnoutrefresh(bottomwin);
    doupdate();
    clearok(edit, FALSE);
    clearok(topwin, FALSE);
    clearok(bottomwin, FALSE);
    edit_refresh();
    titlebar(NULL);
    return 1;
}

void previous_line(void)
{
    if (current_y > 0)
	current_y--;
}

int do_cursorpos(void)
{
    filestruct *fileptr;
    float linepct = 0.0, bytepct = 0.0;
    int i = 0;

    if (current == NULL || fileage == NULL)
	return 0;

    for (fileptr = fileage; fileptr != current && fileptr != NULL;
	 fileptr = fileptr->next)
	i += strlen(fileptr->data) + 1;

    if (fileptr == NULL)
	return -1;

    i += current_x;

    if (totlines > 0)
	linepct = 100 * current->lineno / totlines;

    if (totsize > 0)
	bytepct = 100 * i / totsize;

#ifdef DEBUG
    fprintf(stderr, _("do_cursorpos: linepct = %f, bytepct = %f\n"),
	    linepct, bytepct);
#endif

    statusbar(_("line %d of %d (%.0f%%), character %d of %d (%.0f%%)"),
	      current->lineno, totlines, linepct, i, totsize, bytepct);
    reset_cursor();
    return 1;
}

/* Our broken, non-shortcut list compliant help function.
   But hey, it's better than nothing, and it's dynamic! */
int do_help(void)
{
#ifndef DISABLE_HELP
    char *ptr = help_text, *end;
    int i, j, row = 0, page = 1, kbinput = 0, no_more = 0, kp, kp2;
    int no_help_flag = 0;

    blank_edit();
    curs_set(0);
    blank_statusbar();

    currshortcut = help_list;
    currslen = HELP_LIST_LEN;
    kp = keypad_on(edit, 1);
    kp2 = keypad_on(bottomwin, 1);

    if (ISSET(NO_HELP)) {

	/* Well if we're going to do this, we should at least
	   do it the right way */
	no_help_flag = 1;
	UNSET(NO_HELP);
	window_init();
	bottombars(help_list, HELP_LIST_LEN);

    } else
	bottombars(help_list, HELP_LIST_LEN);

    do {
	ptr = help_text;
	switch (kbinput) {
#ifndef DISABLE_MOUSE
#ifdef NCURSES_MOUSE_VERSION
        case KEY_MOUSE:
            do_mouse();
            break;
#endif
#endif
	case NANO_NEXTPAGE_KEY:
	case NANO_NEXTPAGE_FKEY:
	case KEY_NPAGE:
	    if (!no_more) {
		blank_edit();
		page++;
	    }
	    break;
	case NANO_PREVPAGE_KEY:
	case NANO_PREVPAGE_FKEY:
	case KEY_PPAGE:
	    if (page > 1) {
		no_more = 0;
		blank_edit();
		page--;
	    }
	    break;
	}

	/* Calculate where in the text we should be based on the page */
	for (i = 1; i < page; i++) {
	    row = 0;
	    j = 0;

	    while (row < editwinrows - 2 && *ptr != '\0') {
		if (*ptr == '\n' || j == COLS - 5) {
		    j = 0;
		    row++;
		}
		ptr++;
		j++;
	    }
	}

	if (i > 1) {

	}

	i = 0;
	j = 0;
	while (i < editwinrows && *ptr != '\0') {
	    end = ptr;
	    while (*end != '\n' && *end != '\0' && j != COLS - 5) {
		end++;
		j++;
	    }
	    if (j == COLS - 5) {

		/* Don't print half a word if we've run of of space */
		while (*end != ' ' && *end != '\0') {
		    end--;
		    j--;
		}
	    }
	    mvwaddnstr(edit, i, 0, ptr, j);
	    j = 0;
	    i++;
	    if (*end == '\n')
		end++;
	    ptr = end;
	}
	if (*ptr == '\0') {
	    no_more = 1;
	    continue;
	}
    } while ((kbinput = wgetch(edit)) != NANO_EXIT_KEY && 
      kbinput != NANO_EXIT_FKEY);

    if (no_help_flag) {
	blank_bottombars();
	wrefresh(bottomwin);
	SET(NO_HELP);
	window_init();
    }
    display_main_list();

    curs_set(1);
    edit_refresh();
    kp = keypad_on(edit, kp);
    kp2 = keypad_on(bottomwin, kp2);

#elif defined(DISABLE_HELP)
    nano_disabled_msg();
#endif

    return 1;
}

/* Dump the current file structure to stderr */
void dump_buffer(filestruct * inptr)
{
#ifdef DEBUG
    filestruct *fileptr;

    if (inptr == fileage)
	fprintf(stderr, _("Dumping file buffer to stderr...\n"));
    else if (inptr == cutbuffer)
	fprintf(stderr, _("Dumping cutbuffer to stderr...\n"));
    else
	fprintf(stderr, _("Dumping a buffer to stderr...\n"));

    fileptr = inptr;
    while (fileptr != NULL) {
	fprintf(stderr, "(%ld) %s\n", fileptr->lineno, fileptr->data);
	fflush(stderr);
	fileptr = fileptr->next;
    }
#endif				/* DEBUG */
}

void dump_buffer_reverse(filestruct * inptr)
{
#ifdef DEBUG
    filestruct *fileptr;

    fileptr = filebot;
    while (fileptr != NULL) {
	fprintf(stderr, "(%ld) %s\n", fileptr->lineno, fileptr->data);
	fflush(stderr);
	fileptr = fileptr->prev;
    }
#endif				/* DEBUG */
}

/* Fix editbot based on the assumption that edittop is correct */
void fix_editbot(void)
{
    int i;
    editbot = edittop;
    for (i = 0; (i <= editwinrows - 1) && (editbot->next != NULL)
	 && (editbot != filebot); i++, editbot = editbot->next);
}

/* highlight the current word being replaced or spell checked */
void do_replace_highlight(int highlight_flag, char *word)
{
    char *highlight_word = NULL;
    int x, y;

    highlight_word = mallocstrcpy(highlight_word, &current->data[current_x]);
    highlight_word[strlen(word)] = '\0';

    /* adjust output when word extends beyond screen*/

    x = xplustabs();
    y = get_page_end_virtual(get_page_from_virtual(x)) + 1;

    if ((COLS - (y - x) + strlen(word)) > COLS) {
	highlight_word[y - x - 1] = '$';
	highlight_word[y - x] = '\0';
    }

    /* OK display the output */

    reset_cursor();
    
    if (highlight_flag)
	wattron(edit, A_REVERSE);

    waddstr(edit, highlight_word);

    if (highlight_flag)
	wattroff(edit, A_REVERSE);

    free(highlight_word);
}

#ifdef NANO_EXTRA
#define CREDIT_LEN 47
void do_credits(void)
{
    int i, j = 0, k, place = 0, start_x;
    char *what;

    char *nanotext = _("The nano text editor");
    char *version = _("version ");
    char *brought = _("Brought to you by:");
    char *specialthx = _("Special thanks to:");
    char *fsf = _("The Free Software Foundation");
    char *ncurses = _("Pavel Curtis, Zeyd Ben-Halim and Eric S. Raymond for ncurses");
    char *anyonelse = _("and anyone else we forgot...");
    char *thankyou = _("Thank you for using nano!\n");

    char *credits[CREDIT_LEN] = {nanotext, 
			version, 
			VERSION, 
			"",
			brought,
			"Chris Allegretta",
			"Jordi Mallach",
			"Adam Rogoyski",
			"Rob Siemborski",
			"Rocco Corsi",
			"Ken Tyler",
			"Sven Guckes",
			"Florian K�nig",
			"Pauli Virtanen",
			"Daniele Medri",
			"Clement Laforet",
			"Tedi Heriyanto",
			"Bill Soudan",
			"Christian Weisgerber",
			"Erik Andersen",
			"Big Gaute",
			"Joshua Jensen",
			"Ryan Krebs",
			"Albert Chin",
			"",
			specialthx,
			"Plattsburgh State University",
			"Benet Laboratories",
			"Amy Allegretta",
			"Linda Young",
			"Jeremy Robichaud",
			"Richard Kolb II",
			fsf,
			"Linus Torvalds",
			ncurses,
			anyonelse,
			thankyou,
			"", "", "", "",
			"(c) 2000 Chris Allegretta",
			"", "", "", "",
			"www.nano-editor.org"
    };

    curs_set(0);
    nodelay(edit, TRUE);
    blank_bottombars();
    mvwaddstr(topwin, 0, 0, hblank);
    blank_edit();
    wrefresh(edit);
    wrefresh(bottomwin);
    wrefresh(topwin);

    while (wgetch(edit) == ERR) {
	for (k = 0; k <= 1; k++) {
	    blank_edit();
	    for (i = editwinrows / 2 - 1; i >= (editwinrows / 2 - 1 - j); i--) {
		mvwaddstr(edit, i * 2 - k, 0, hblank);

		if (place - (editwinrows / 2 - 1 - i) < CREDIT_LEN)
		    what = credits[place - (editwinrows / 2 - 1 - i)];
		else
		    what = "";

		start_x = COLS / 2 - strlen(what) / 2 - 1;
		mvwaddstr(edit, i * 2 - k, start_x, what);
	    }
	    usleep(700000);
	    wrefresh(edit);
	}
	if (j < editwinrows / 2 - 1)
	    j++;

	place++;

	if (place >= CREDIT_LEN + editwinrows / 2)
	    break;
    }

    nodelay(edit, FALSE);
    curs_set(1);
    display_main_list();
    total_refresh();
 }
#endif

int             keypad_on(WINDOW * win, int newval)
{

/* This is taken right from aumix.  Don't sue me */
#ifdef HAVE_USEKEYPAD
    int             old;

    old = win->_use_keypad;
    keypad(win, newval);
    return old;
#else
    keypad(win, newval);
    return 1;
#endif                          /* HAVE_USEKEYPAD */

}



