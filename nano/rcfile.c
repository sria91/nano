/* $Id$ */
/**************************************************************************
 *   rcfile.c                                                             *
 *                                                                        *
 *   Copyright (C) 1999-2002 Chris Allegretta                             *
 *   This program is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by *
 *   the Free Software Foundation; either version 2, or (at your option)  *
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

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "config.h"
#include "proto.h"
#include "nano.h"

#ifdef ENABLE_NANORC

#ifdef ENABLE_NLS
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

/* Static stuff for the nanorc file */
rcoption rcopts[] = {
#ifndef NANO_SMALL
    {"autoindent", AUTOINDENT},
    {"backup", BACKUP_FILE},
#endif
    {"const", CONSTUPDATE},
#ifndef NANO_SMALL
    {"cut", CUT_TO_END},
#endif
    {"fill", 0},
    {"keypad", ALT_KEYPAD},
#ifndef DISABLE_MOUSE
    {"mouse", USE_MOUSE},
#endif
#ifdef ENABLE_MULTIBUFFER
    {"multibuffer", MULTIBUFFER},
#endif
#ifndef NANO_SMALL
    {"noconvert", NO_CONVERT},
#endif
    {"nofollow", FOLLOW_SYMLINKS},
    {"nohelp", NO_HELP},
    {"nowrap", NO_WRAP},
#ifndef DISABLE_OPERATINGDIR
    {"operatingdir", 0},
#endif
    {"pico", PICO_MODE},
#ifndef NANO_SMALL
    {"quotestr", 0},
#endif
#ifdef HAVE_REGEX_H
    {"regexp", USE_REGEXP},
#endif
#ifndef NANO_SMALL
    {"smooth", SMOOTHSCROLL},
#endif
#ifndef DISABLE_SPELLER
    {"speller", 0},
#endif
    {"suspend", SUSPEND},
    {"tabsize", 0},
    {"tempfile", TEMP_OPT},
    {"view", VIEW_MODE},
    {"", 0}
};

static int errors = 0;
static int lineno = 0;
static char *nanorc;

/* We have an error in some part of the rcfile; put it on stderr and
  make the user hit return to continue starting up nano */
void rcfile_error(char *msg, ...)
{
    va_list ap;

    fprintf(stderr, "\n");
    if (lineno > 0)
	fprintf(stderr, _("Error in %s on line %d: "), nanorc, lineno);

    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fprintf(stderr, _("\nPress return to continue starting nano\n"));

    while (getchar() != '\n');

}

/* Just print the error (one of many, perhaps) but don't abort, yet */
void rcfile_msg(char *msg, ...)
{
    va_list ap;

    if (!errors) {
	errors = 1;
	fprintf(stderr, "\n");
    }
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fprintf(stderr, "\n");

}

/* Parse the next word from the string.  Returns NULL if we hit EOL */
char *parse_next_word(char *ptr)
{
    while (*ptr != ' ' && *ptr != '\t' && *ptr != '\n' && *ptr != '\0')
	ptr++;

    if (*ptr == '\0')
	return NULL;

    /* Null terminate and advance ptr */
    *ptr++ = 0;

    while ((*ptr == ' ' || *ptr == '\t') && *ptr != '\0')
	ptr++;

    return ptr;
}

char *parse_next_regex(char *ptr)
{
    while ((*ptr != '"' || (*(ptr+1) != ' ' && *(ptr+1) != '\n')) 
	   && *ptr != '\n' && *ptr != '\0')
	ptr++;

    if (*ptr == '\0')
	return NULL;

    /* Null terminate and advance ptr */
    *ptr++ = 0;

    while ((*ptr == ' ' || *ptr == '\t') && *ptr != '\0')
	ptr++;

    return ptr;

}

#ifdef ENABLE_COLOR

int colortoint(char *colorname, int *bright)
{
    int mcolor = 0;

    if (colorname == NULL)
	return -1;

    if (stristr(colorname, "bright")) {
	*bright = 1;
	colorname += 6;
    }

    if (!strcasecmp(colorname, "green"))
	mcolor += COLOR_GREEN;
    else if (!strcasecmp(colorname, "red"))
	mcolor += COLOR_RED;
    else if (!strcasecmp(colorname, "blue"))
	mcolor += COLOR_BLUE;
    else if (!strcasecmp(colorname, "white"))
	mcolor += COLOR_WHITE;
    else if (!strcasecmp(colorname, "yellow"))
	mcolor += COLOR_YELLOW;
    else if (!strcasecmp(colorname, "cyan"))
	mcolor += COLOR_CYAN;
    else if (!strcasecmp(colorname, "magenta"))
	mcolor += COLOR_MAGENTA;
    else if (!strcasecmp(colorname, "black"))
	mcolor += COLOR_BLACK;
    else {
	rcfile_error(_("color %s not understood.\n"
		       "Valid colors are \"green\", \"red\", \"blue\", \n"
		       "\"white\", \"yellow\", \"cyan\", \"magenta\" and \n"
		       "\"black\", with the optional prefix \"bright\".\n"));
	exit(1);
    }

    return mcolor;
}

void parse_syntax(FILE * rcstream, char *buf, char *ptr)
{
    syntaxtype *tmpsyntax = NULL;
    char *fileregptr = NULL, *nameptr = NULL;
    exttype *exttmp = NULL;

    while (*ptr == ' ')
	ptr++;

    if (*ptr == '\n' || *ptr == '\0')
	return;

    if (*ptr != '"') {
	rcfile_error(_("regex strings must begin and end with a \" character\n"));
	return;
    }
    ptr++;

    nameptr = ptr;
    ptr = parse_next_regex(ptr);

    if (ptr == NULL) {
	rcfile_error(_("Missing syntax name"));
	return;
    }

	if (syntaxes == NULL) {
	    syntaxes = nmalloc(sizeof(syntaxtype));
	    syntaxes->desc = NULL;
	    syntaxes->desc = mallocstrcpy(syntaxes->desc, nameptr);
	    syntaxes->color = NULL;
	    syntaxes->extensions = NULL;
	    syntaxes->next = NULL;
	    tmpsyntax = syntaxes;
#ifdef DEBUG
	    fprintf(stderr,
		    _("Starting a new syntax type\n"));
	    fprintf(stderr, "string val=%s\n", nameptr);
#endif

	} else {
	    for (tmpsyntax = syntaxes;
		 tmpsyntax->next != NULL; tmpsyntax = tmpsyntax->next);
#ifdef DEBUG
	    fprintf(stderr, _("Adding new syntax after 1st\n"));
#endif

	    tmpsyntax->next = nmalloc(sizeof(syntaxtype));
	    tmpsyntax->next->desc = NULL;
	    tmpsyntax->next->desc = mallocstrcpy(tmpsyntax->next->desc, nameptr);
	    tmpsyntax->next->color = NULL;
	    tmpsyntax->next->extensions = NULL;
	    tmpsyntax->next->next = NULL;
	    tmpsyntax = tmpsyntax->next;
	}

    /* Now load in the extensions to their part of the struct */
    while (*ptr != '\n' && *ptr != '\0') {

	while (*ptr != '"' && *ptr != '\n' && *ptr != '\0')
	    ptr++;

	if (*ptr == '\n' || *ptr == '\0')
	    return;
	ptr++;

	fileregptr = ptr;
	ptr = parse_next_regex(ptr);

	if (tmpsyntax->extensions == NULL) {
	    tmpsyntax->extensions = nmalloc(sizeof(exttype));
	    tmpsyntax->extensions->val = NULL;
	    tmpsyntax->extensions->val = mallocstrcpy(tmpsyntax->extensions->val, fileregptr);
	    tmpsyntax->extensions->next = NULL;
	}
	else {
	    for (exttmp = tmpsyntax->extensions; exttmp->next != NULL;
		 exttmp = exttmp->next);
	    exttmp->next = nmalloc(sizeof(exttype));
	    exttmp->next->val = NULL;
	    exttmp->next->val = mallocstrcpy(exttmp->next->val, fileregptr);
	    exttmp->next->next = NULL;
	}
    }
}

/* Parse the color stuff into the colorstrings array */
void parse_colors(FILE * rcstream, char *buf, char *ptr)
{
    int fg, bg, bright = 0;
    int expectend = 0;		/* Do we expect an end= line? */
    char *tmp = NULL, *beginning, *fgstr, *bgstr;
    colortype *tmpcolor = NULL;
    syntaxtype *tmpsyntax = NULL;

    fgstr = ptr;
    ptr = parse_next_word(ptr);

    if (ptr == NULL) {
	rcfile_error(_("Missing color name"));
	return;
    }

    if (strstr(fgstr, ",")) {
	strtok(fgstr, ",");
	bgstr = strtok(NULL, ",");
    } else
	bgstr = NULL;

    fg = colortoint(fgstr, &bright);
    bg = colortoint(bgstr, &bright);

    if (syntaxes == NULL) {
	rcfile_error(_("Cannot add a color directive without a syntax line"));
	return;
    }

    for (tmpsyntax = syntaxes; tmpsyntax->next != NULL;
	 tmpsyntax = tmpsyntax->next)
	;

    /* Now the fun part, start adding regexps to individual strings
       in the colorstrings array, woo! */

    while (*ptr != '\0') {

	while (*ptr == ' ')
	    ptr++;

	if (*ptr == '\n' || *ptr == '\0')
	    break;

	if (!strncasecmp(ptr, "start=", 6)) {
	    ptr += 6;
	    expectend = 1;
	}

	if (*ptr != '"') {
	    rcfile_error(_("regex strings must begin and end with a \" character\n"));
	    continue;
	}
	ptr++;

	beginning = ptr;
	ptr = parse_next_regex(ptr);

	tmp = NULL;
	tmp = mallocstrcpy(tmp, beginning);

	if (tmpsyntax->color == NULL) {
	    tmpsyntax->color = nmalloc(sizeof(colortype));
	    tmpsyntax->color->fg = fg;
	    tmpsyntax->color->bg = bg;
	    tmpsyntax->color->bright = bright;
	    tmpsyntax->color->start = tmp;
	    tmpsyntax->color->next = NULL;
	    tmpcolor = tmpsyntax->color;
#ifdef DEBUG
	    fprintf(stderr,
		    _("Starting a new colorstring for fg %d bg %d\n"),
		    fg, bg);
	    fprintf(stderr, _("string val=%s\n"), tmp);
#endif

	} else {
	    for (tmpcolor = tmpsyntax->color;
		 tmpcolor->next != NULL; tmpcolor = tmpcolor->next);
#ifdef DEBUG
	    fprintf(stderr, _("Adding new entry for fg %d bg %d\n"), fg, bg);
	    fprintf(stderr, _("string val=%s\n"), tmp);
#endif

	    tmpcolor->next = nmalloc(sizeof(colortype));
	    tmpcolor->next->fg = fg;
	    tmpcolor->next->bg = bg;
	    tmpcolor->next->bright = bright;
	    tmpcolor->next->start = tmp;
	    tmpcolor->next->next = NULL;
	    tmpcolor = tmpcolor->next;
	}

	if (expectend) {
	    if (ptr == NULL || strncasecmp(ptr, "end=", 4)) {
		rcfile_error(_
			     ("\n\t\"start=\" requires a corresponding \"end=\""));
		return;
	    }

	    ptr += 4;

	    if (*ptr != '"') {
		rcfile_error(_
			     ("regex strings must begin and end with a \" character\n"));
		continue;
	    }
	    ptr++;


	    beginning = ptr;
	    ptr = parse_next_regex(ptr);
#ifdef DEBUG
	    fprintf(stderr, _("For end part, beginning = \"%s\"\n"),
		    beginning);
#endif
	    tmp = NULL;
	    tmp = mallocstrcpy(tmp, beginning);
	    tmpcolor->end = tmp;

	} else
	    tmpcolor->end = NULL;

    }
}

#endif /* ENABLE_COLOR */

/* Parse the RC file, once it has been opened successfully */
void parse_rcfile(FILE * rcstream)
{
    char *buf, *ptr, *keyword, *option;
    int set = 0, i, j;

    buf = charalloc(1024);
    while (fgets(buf, 1023, rcstream) > 0) {
	lineno++;
	ptr = buf;
	while ((*ptr == ' ' || *ptr == '\t') &&
	       (*ptr != '\n' && *ptr != '\0'))
	    ptr++;

	if (*ptr == '\n' || *ptr == '\0')
	    continue;

	if (*ptr == '#') {
#ifdef DEBUG
	    fprintf(stderr, _("parse_rcfile: Read a comment\n"));
#endif
	    continue;		/* Skip past commented lines */
	}

	/* Else skip to the next space */
	keyword = ptr;
	ptr = parse_next_word(ptr);
	if (!ptr)
	    continue;

	/* Else try to parse the keyword */
	if (!strcasecmp(keyword, "set"))
	    set = 1;
	else if (!strcasecmp(keyword, "unset"))
	    set = -1;
#ifdef ENABLE_COLOR
	else if (!strcasecmp(keyword, "syntax"))
	    parse_syntax(rcstream, buf, ptr);
	else if (!strcasecmp(keyword, "color"))
	    parse_colors(rcstream, buf, ptr);
#endif				/* ENABLE_COLOR */
	else {
	    rcfile_msg(_("command %s not understood"), keyword);
	    continue;
	}

	option = ptr;
	ptr = parse_next_word(ptr);
	/* We don't care if ptr == NULL, as it should if using proper syntax */

	if (set != 0) {
	    for (i = 0; rcopts[i].name != ""; i++) {
		if (!strcasecmp(option, rcopts[i].name)) {
#ifdef DEBUG
		    fprintf(stderr, _("parse_rcfile: Parsing option %s\n"),
			    rcopts[i].name);
#endif
		    if (set == 1 || rcopts[i].flag == FOLLOW_SYMLINKS) {
			if (!strcasecmp(rcopts[i].name, "operatingdir") ||
			    !strcasecmp(rcopts[i].name, "tabsize") ||
#ifndef DISABLE_WRAPJUSTIFY
			    !strcasecmp(rcopts[i].name, "fill") ||
#endif
#ifndef DISABLE_JUSTIFY
			    !strcasecmp(rcopts[i].name, "quotestr") ||
#endif
#ifndef DISABLE_SPELLER
			    !strcasecmp(rcopts[i].name, "speller")
#else
			    0
#endif
			    ) {

			    if (*ptr == '\n' || *ptr == '\0') {
				rcfile_error(_
					     ("option %s requires an argument"),
					     rcopts[i].name);
				continue;
			    }
			    option = ptr;
			    ptr = parse_next_word(ptr);
			    if (!strcasecmp(rcopts[i].name, "fill")) {
#ifndef DISABLE_WRAPJUSTIFY

				if ((j = atoi(option)) < MIN_FILL_LENGTH) {
				    rcfile_error(_
						 ("requested fill size %d too small"),
						 j);
				} else
				    fill = j;
#endif
			    } else
				if (!strcasecmp(rcopts[i].name, "tabsize"))
			    {
				if ((j = atoi(option)) <= 0) {
				    rcfile_error(_
						 ("requested tab size %d too small"),
						 j);
				} else {
				    tabsize = j;
				}
#ifndef DISABLE_JUSTIFY
			    } else
				if (!strcasecmp(rcopts[i].name, "quotestr"))
			    {
				quotestr = NULL;
				quotestr =
				    charalloc(strlen(option) + 1);
				strcpy(quotestr, option);
#endif
			    } else {
#ifndef DISABLE_SPELLER
				alt_speller =
				    charalloc(strlen(option) + 1);
				strcpy(alt_speller, option);
#endif
			    }
			} else
			    SET(rcopts[i].flag);
#ifdef DEBUG
			fprintf(stderr, _("set flag %d!\n"),
				rcopts[i].flag);
#endif
		    } else {
			UNSET(rcopts[i].flag);
#ifdef DEBUG
			fprintf(stderr, _("unset flag %d!\n"),
				rcopts[i].flag);
#endif
		    }
		}
	    }
	}

    }
    if (errors)
	rcfile_error(_("Errors found in .nanorc file"));

    return;
}

/* The main rc file function, tries to open the rc file */
void do_rcfile(void)
{
    char *unable = _("Unable to open ~/.nanorc file, %s");
    struct stat fileinfo;
    FILE *rcstream;
    struct passwd *userage;

    nanorc = charalloc(strlen(SYSCONFDIR) + 10);
    sprintf(nanorc, "%s/nanorc", SYSCONFDIR);

    /* Try to open system nanorc */
    if (stat(nanorc, &fileinfo) != -1)
	if ((rcstream = fopen(nanorc, "r")) != NULL) {

	   /* Parse it! */
	    parse_rcfile(rcstream);
	    fclose(rcstream);
	}

    lineno = 0;

    /* Determine home directory using getpwent(), don't rely on $HOME */
    for (userage = getpwent(); userage != NULL
	 && userage->pw_uid != geteuid(); userage = getpwent())
	;

    if (userage == NULL) {
	rcfile_error(_("I can't find my home directory!  Wah!"));
	return;
    }

    nanorc = charalloc(strlen(userage->pw_dir) + 10);
    sprintf(nanorc, "%s/.nanorc", userage->pw_dir);

    if (stat(nanorc, &fileinfo) == -1) {

	/* Abort if the file doesn't exist and there's some other kind
	   of error stat()ing it */
	if (errno != ENOENT)
	    rcfile_error(unable, strerror(errno));
	return;
    }

    if ((rcstream = fopen(nanorc, "r")) == NULL) {
	rcfile_error(unable, strerror(errno));
	return;
    }


    parse_rcfile(rcstream);
    fclose(rcstream);

}

#endif /* ENABLE_NANORC */
