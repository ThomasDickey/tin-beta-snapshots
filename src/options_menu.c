/*
 *  Project   : tin - a Usenet reader
 *  Module    : options_menu.c
 *  Author    : Michael Bienia <michael@vorlon.ping.de>
 *  Created   : 2004-09-05
 *  Updated   : 2013-10-01
 *  Notes     : Split from config.c
 *
 * Copyright (c) 2004-2015 Michael Bienia <michael@vorlon.ping.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef TINCFG_H
#	include "tincfg.h"
#endif /* !TINCFG_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */


#define option_lines_per_page (cLINES - INDEX_TOP - 3)

#define UPDATE_INT_ATTRIBUTES(option) do { \
		scopes[0].attribute->option = tinrc.option; \
		changed |= MISC_OPTS; \
	} while (0)

#define CAO(A, O) A ## O
#define SET_NUM_ATTRIBUTE(option) do { \
		curr_scope->attribute->option = CAO(tinrc.attrib_, option); \
		curr_scope->state->option = TRUE; \
		changed |= MISC_OPTS; \
	} while (0)
#define SET_STRING_ATTRIBUTE(opt) do { \
		if (!strlen(CAO(tinrc.attrib_, opt))) { \
			reset_state(option); \
			redraw_screen(option); \
		} else { \
			FreeIfNeeded(curr_scope->attribute->opt); \
			curr_scope->state->opt = TRUE; \
			curr_scope->attribute->opt = my_strdup(CAO(tinrc.attrib_, opt)); \
		} \
		changed |= MISC_OPTS; \
	} while (0)

static enum option_enum first_option_on_screen, last_option_on_screen, last_opt;

/*
 * local prototypes
 */
static enum option_enum get_first_opt(void);
static enum option_enum move_cursor(enum option_enum cur_option, t_bool down);
static enum option_enum next_option(enum option_enum option, t_bool incl_titles);
static enum option_enum opt_scroll_down(enum option_enum option);
static enum option_enum opt_scroll_up(enum option_enum option);
static enum option_enum prev_option(enum option_enum option, t_bool incl_titles);
static enum option_enum set_option_num(int num);
static int add_new_scope(void);
static int find_scope(const char *scope);
static int get_option_num(enum option_enum option);
static int move_scope(int curr_pos);
static t_bool check_state(enum option_enum option);
static t_bool delete_scope(int curr_pos);
static t_bool option_is_title(enum option_enum option);
static t_bool option_on_page(enum option_enum option);
static t_bool rename_scope(struct t_scope *scope);
static t_bool scope_is_empty(void);
static t_function option_left(void);
static t_function option_right(void);
static t_function scope_left(void);
static t_function scope_right(void);
static void build_scope_line(int i);
static void do_delete_scope(int curr_pos);
static void do_move_scope(int from, int to);
static void draw_scope_arrow(void);
static void free_scopes_and_attributes(void);
static void highlight_option(enum option_enum option);
static void initialize_attributes(void);
static void print_any_option(enum option_enum option);
static void redraw_screen(enum option_enum option);
static void repaint_option(enum option_enum option);
static void reset_state(enum option_enum option);
static void scope_page(enum context level);
static void set_first_option_on_screen(enum option_enum last_option);
static void set_last_opt(void);
static void set_last_option_on_screen(enum option_enum first_option);
static void show_config_page(void);
static void show_scope_page(void);
static void unhighlight_option(enum option_enum option);
#ifdef USE_CURSES
	static void do_scroll(int jump);
#endif /* USE_CURSES */

static t_menu scopemenu = { 0, 0, 0, show_scope_page, draw_scope_arrow, build_scope_line };
static struct t_scope *curr_scope = NULL;

/*
 * returns the row on the screen of an option
 * note: option should be on this page
 */
int
option_row(
	enum option_enum option)
{
	int i = 0;
	enum option_enum j = first_option_on_screen;

	while (j < option) {
		if (option_is_visible(j))
			i++;
		j++;
	}

	return INDEX_TOP + i;
}


/*
 * returns the number of an option
 */
static int
get_option_num(
	enum option_enum option)
{
	enum option_enum i;
	int result = 0;

	for (i = 0; i < option && result < (int) last_opt; i = next_option(i, FALSE))
		result++;

	return result;
}


/*
 * returns the option with the given number
 */
static enum option_enum
set_option_num(
	int num)
{
	enum option_enum result = 0;

	while (num > 0 && result < last_opt) {
		result = next_option(result, FALSE);
		num--;
	}
	return result;
}


/*
 * returns TRUE if an option is visible in the menu
 */
t_bool
option_is_visible(
	enum option_enum option)
{
	switch (option) {
#ifdef HAVE_COLOR
		case OPT_COL_BACK:
		case OPT_COL_FROM:
		case OPT_COL_HEAD:
		case OPT_COL_HELP:
		case OPT_COL_INVERS_BG:
		case OPT_COL_INVERS_FG:
		case OPT_COL_MESSAGE:
		case OPT_COL_MINIHELP:
		case OPT_COL_NEWSHEADERS:
		case OPT_COL_NORMAL:
		case OPT_COL_QUOTE:
		case OPT_COL_QUOTE2:
		case OPT_COL_QUOTE3:
		case OPT_COL_RESPONSE:
		case OPT_COL_SIGNATURE:
		case OPT_COL_SUBJECT:
		case OPT_COL_TEXT:
		case OPT_COL_TITLE:
		case OPT_COL_URLS:
		case OPT_QUOTE_REGEX:
		case OPT_QUOTE_REGEX2:
		case OPT_QUOTE_REGEX3:
			return curr_scope ? FALSE : tinrc.use_color;

		case OPT_COL_MARKSTAR:
		case OPT_COL_MARKDASH:
		case OPT_COL_MARKSLASH:
		case OPT_COL_MARKSTROKE:
			return curr_scope ? FALSE : (tinrc.word_highlight && tinrc.use_color);

		case OPT_COL_VERBATIM:
			return curr_scope ? FALSE : (tinrc.verbatim_handling && tinrc.use_color);
#endif /* HAVE_COLOR */

		case OPT_WORD_H_DISPLAY_MARKS:
		case OPT_SLASHES_REGEX:
		case OPT_STARS_REGEX:
		case OPT_STROKES_REGEX:
		case OPT_UNDERSCORES_REGEX:
			return curr_scope ? FALSE : tinrc.word_highlight;

		case OPT_MONO_MARKSTAR:
		case OPT_MONO_MARKDASH:
		case OPT_MONO_MARKSLASH:
		case OPT_MONO_MARKSTROKE:
#ifdef HAVE_COLOR
			return curr_scope ? FALSE : (tinrc.word_highlight && !tinrc.use_color);
#else
			return curr_scope ? FALSE : tinrc.word_highlight;
#endif /* HAVE_COLOR */

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		case OPT_UTF8_GRAPHICS:
			return curr_scope ? FALSE : IS_LOCAL_CHARSET("UTF-8");
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

		case OPT_VERBATIM_BEGIN_REGEX:
		case OPT_VERBATIM_END_REGEX:
			return curr_scope ? FALSE : tinrc.verbatim_handling;

		case OPT_GETART_LIMIT_OPTIONS:
#ifdef HAVE_COLOR
		case OPT_COLOR_OPTIONS:
#endif /* HAVE_COLOR */
			return curr_scope ? FALSE : TRUE;

		case OPT_DISPLAY_OPTIONS:
		case OPT_FILTERING_OPTIONS:
		case OPT_SAVING_OPTIONS:
		case OPT_POSTING_OPTIONS:
		case OPT_EXPERT_OPTIONS:
			return TRUE;

		case OPT_ATTRIB_ADD_POSTED_TO_FILTER:
		case OPT_ATTRIB_ADVERTISING:
		case OPT_ATTRIB_ALTERNATIVE_HANDLING:
		case OPT_ATTRIB_ASK_FOR_METAMAIL:
		case OPT_ATTRIB_AUTO_CC_BCC:
		case OPT_ATTRIB_AUTO_LIST_THREAD:
		case OPT_ATTRIB_AUTO_SAVE:
		case OPT_ATTRIB_AUTO_SELECT:
		case OPT_ATTRIB_BATCH_SAVE:
		case OPT_ATTRIB_DATE_FORMAT:
		case OPT_ATTRIB_DELETE_TMP_FILES:
		case OPT_ATTRIB_EDITOR_FORMAT:
		case OPT_ATTRIB_FCC:
		case OPT_ATTRIB_FOLLOWUP_TO:
		case OPT_ATTRIB_FROM:
		case OPT_ATTRIB_GROUP_CATCHUP_ON_EXIT:
		case OPT_ATTRIB_GROUP_FORMAT:
#ifdef HAVE_ISPELL
		case OPT_ATTRIB_ISPELL:
#endif /* HAVE_ISPELL */
		case OPT_ATTRIB_MAILDIR:
		case OPT_ATTRIB_MAIL_8BIT_HEADER:
		case OPT_ATTRIB_MAIL_MIME_ENCODING:
		case OPT_ATTRIB_MAILING_LIST:
		case OPT_ATTRIB_MARK_IGNORE_TAGS:
		case OPT_ATTRIB_MARK_SAVED_READ:
		case OPT_ATTRIB_MIME_FORWARD:
		case OPT_ATTRIB_MIME_TYPES_TO_SAVE:
		case OPT_ATTRIB_NEWS_HEADERS_TO_DISPLAY:
		case OPT_ATTRIB_NEWS_HEADERS_TO_NOT_DISPLAY:
		case OPT_ATTRIB_NEWS_QUOTE_FORMAT:
		case OPT_ATTRIB_ORGANIZATION:
		case OPT_ATTRIB_POST_8BIT_HEADER:
		case OPT_ATTRIB_POST_MIME_ENCODING:
		case OPT_ATTRIB_POST_PROCESS_VIEW:
		case OPT_ATTRIB_POS_FIRST_UNREAD:
		case OPT_ATTRIB_QUICK_KILL_HEADER:
		case OPT_ATTRIB_QUICK_KILL_SCOPE:
		case OPT_ATTRIB_QUICK_KILL_EXPIRE:
		case OPT_ATTRIB_QUICK_KILL_CASE:
		case OPT_ATTRIB_QUICK_SELECT_HEADER:
		case OPT_ATTRIB_QUICK_SELECT_SCOPE:
		case OPT_ATTRIB_QUICK_SELECT_EXPIRE:
		case OPT_ATTRIB_QUICK_SELECT_CASE:
#ifndef DISABLE_PRINTING
		case OPT_ATTRIB_PRINT_HEADER:
#endif /* !DISABLE_PRINTING */
		case OPT_ATTRIB_PROCESS_ONLY_UNREAD:
		case OPT_ATTRIB_PROMPT_FOLLOWUPTO:
		case OPT_ATTRIB_QUOTE_CHARS:
		case OPT_ATTRIB_SAVEDIR:
		case OPT_ATTRIB_SAVEFILE:
		case OPT_ATTRIB_SHOW_AUTHOR:
		case OPT_ATTRIB_SHOW_ONLY_UNREAD_ARTS:
		case OPT_ATTRIB_SHOW_SIGNATURES:
		case OPT_ATTRIB_SIGDASHES:
		case OPT_ATTRIB_SIGFILE:
		case OPT_ATTRIB_SIGNATURE_REPOST:
		case OPT_ATTRIB_START_EDITOR_OFFSET:
		case OPT_ATTRIB_THREAD_ARTICLES:
		case OPT_ATTRIB_THREAD_CATCHUP_ON_EXIT:
		case OPT_ATTRIB_THREAD_FORMAT:
		case OPT_ATTRIB_THREAD_PERC:
		case OPT_ATTRIB_TRIM_ARTICLE_BODY:
		case OPT_ATTRIB_TEX2ISO_CONV:
		case OPT_ATTRIB_SORT_THREADS_TYPE:
#ifdef CHARSET_CONVERSION
		case OPT_ATTRIB_MM_NETWORK_CHARSET:
		case OPT_ATTRIB_UNDECLARED_CHARSET:
#endif /* CHARSET_CONVERSION */
		case OPT_ATTRIB_VERBATIM_HANDLING:
		case OPT_ATTRIB_WRAP_ON_NEXT_UNREAD:
		case OPT_ATTRIB_SORT_ARTICLE_TYPE:
		case OPT_ATTRIB_POST_PROCESS_TYPE:
		case OPT_ATTRIB_X_BODY:
		case OPT_ATTRIB_X_COMMENT_TO:
		case OPT_ATTRIB_X_HEADERS:
			return curr_scope ? TRUE : FALSE;

		default:
			return curr_scope ? FALSE : TRUE;
	}
}


/*
 * returns TRUE if option is OPT_TITLE else FALSE
 */
static t_bool
option_is_title(
	enum option_enum option)
{
	return option_table[option].var_type == OPT_TITLE;
}


/*
 * returns TRUE if option is on the current page else FALSE
 */
static t_bool
option_on_page(
	enum option_enum option)
{
	return ((option >= first_option_on_screen) && (option <= last_option_on_screen));
}


char *
fmt_option_prompt(
	char *dst,
	size_t len,
	t_bool editing,
	enum option_enum option)
{
	char *buf;
	size_t option_width = MAX(35, cCOLS / 2 - 9);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wbuf, *wbuf2;

	/* convert the option text to wchar_t */
	wbuf = char2wchar_t(_(option_table[option].txt->opt));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (!option_is_title(option)) {
		char flag;
		int num = get_option_num(option);

		flag = (curr_scope && check_state(option)) ? '+' : ' ';
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (wbuf != NULL) {
			wbuf2 = wstrunc(wbuf, option_width);
			if ((buf = wchar_t2char(wbuf2)) == NULL) {
				/* conversion failed, truncate original string */
				buf = strunc(_(option_table[option].txt->opt), option_width);
				snprintf(dst, len, "%s %c%3d %-*.*s: ", editing ? "->" : "  ", flag, num, (int) option_width, (int) option_width, buf);
			} else
				snprintf(dst, len, "%s %c%3d %-*.*s: ", editing ? "->" : "  ", flag, num,
					(int) (strlen(buf) + option_width - wcswidth(wbuf2, option_width + 1)),
					(int) (strlen(buf) + option_width - wcswidth(wbuf2, option_width + 1)), buf);
			free(wbuf2);
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		{
			/* truncate original string */
			buf = strunc(_(option_table[option].txt->opt), option_width);
			snprintf(dst, len, "%s %c%3d %-*.*s: ", editing ? "->" : "  ", flag, num, (int) option_width, (int) option_width, buf);
		}
	} else {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (wbuf != NULL) {
			wbuf2 = wstrunc(wbuf, cCOLS - 3);
			if ((buf = wchar_t2char(wbuf2)) == NULL)	/* conversion failed, truncate original string */
				buf = strunc(_(option_table[option].txt->opt), cCOLS - 3);
			free(wbuf2);
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			buf = strunc(_(option_table[option].txt->opt), cCOLS - 3);	/* truncate original string */
		snprintf(dst, len, "  %s", buf);
	}

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	FreeIfNeeded(wbuf);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	FreeIfNeeded(buf);
	return dst;
}


static void
print_any_option(
	enum option_enum option)
{
	constext **list;
	char temp[LEN], *ptr, *ptr2;
	int row = option_row(option);
	size_t len = sizeof(temp) - 1;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE) && defined(USE_CURSES)
	char *buf;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE && USE_CURSES */

	MoveCursor(row, 0);

	ptr = fmt_option_prompt(temp, len, FALSE, option);
	ptr += strlen(temp);
	len -= strlen(temp);

	switch (option_table[option].var_type) {
		case OPT_ON_OFF:
			/* tailing space to overwrite any left over F from OFF */
			snprintf(ptr, len, "%s ", print_boolean(*OPT_ON_OFF_list[option_table[option].var_index]));
			break;

		case OPT_LIST:
			list = option_table[option].opt_list;
			ptr2 = my_strdup(list[*(option_table[option].variable) + ((strcasecmp(_(list[0]), _(txt_default)) == 0) ? 1 : 0)]);
			strncpy(ptr, _(ptr2), len);
			free(ptr2);
			break;

		case OPT_STRING:
			strncpy(ptr, OPT_STRING_list[option_table[option].var_index], len);
			break;

		case OPT_NUM:
			snprintf(ptr, len, "%d", *(option_table[option].variable));
			break;

		case OPT_CHAR:
			snprintf(ptr, len, "%c", *OPT_CHAR_list[option_table[option].var_index]);
			break;

		default:
			break;
	}
#ifdef USE_CURSES
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	if ((buf = spart(temp, cCOLS - 1, FALSE)) != NULL) {
		my_printf("%s", buf);
		free(buf);
	} else
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		my_printf("%.*s", cCOLS - 1, temp);
	{
		int y, x;

		getyx(stdscr, y, x);
		if (x < cCOLS)
			clrtoeol();
	}
#else
	my_printf("%.*s", cCOLS - 1, temp);
	/* draw_arrow_mark() will read this back for repainting */
	strncpy(screen[row - INDEX_TOP].col, temp, cCOLS);
#endif /* USE_CURSES */
}


static void
repaint_option(
	enum option_enum option)
{
	if (option_on_page(option))
		print_any_option(option);
}


#ifdef USE_CURSES
static void
do_scroll(
	int jump)
{
	scrollok(stdscr, TRUE);
	MoveCursor(INDEX_TOP, 0);
	SetScrollRegion(INDEX_TOP, INDEX_TOP + option_lines_per_page - 1);
	ScrollScreen(jump);
	SetScrollRegion(0, LINES - 1);
	scrollok(stdscr, FALSE);
}
#endif /* USE_CURSES */


/*
 * returns the option after moving 'move' positions up or down
 * updates also first_option_on_screen and last_option_on screen accordingly
 */
static enum option_enum
move_cursor(
	enum option_enum cur_option,
	t_bool down)
{
	enum option_enum old_option = cur_option;

	if (down) {		/* move down */
		do {
			cur_option = next_option(cur_option, TRUE);
			if (cur_option > last_option_on_screen) {
				/* move the markers one option down */
				last_option_on_screen = cur_option;
				first_option_on_screen = next_option(first_option_on_screen, TRUE);
#ifdef USE_CURSES
				do_scroll(1);
				print_any_option(cur_option);
#else
				show_config_page();
#endif /* USE_CURSES */
			} else if (cur_option < first_option_on_screen) {
				/* wrap around: set to begin of option list */
				first_option_on_screen = cur_option;
				set_last_option_on_screen(cur_option);
				show_config_page();
			}
		} while (option_is_title(cur_option) && old_option != cur_option);
	} else {		/* move up */
		do {
			cur_option = prev_option(cur_option, TRUE);
			if (cur_option < first_option_on_screen) {
				/* move the markers one option up */
				first_option_on_screen = cur_option;
				set_last_option_on_screen(cur_option);
#ifdef USE_CURSES
				do_scroll(-1);
				print_any_option(cur_option);
#else
				show_config_page();
#endif /* USE_CURSES */
			} else if (cur_option > last_option_on_screen) {
				/* wrap around: set to end of option list */
				last_option_on_screen = cur_option;
				set_first_option_on_screen(cur_option);
				show_config_page();
			}
		} while (option_is_title(cur_option) && old_option != cur_option);
	}
	return cur_option;
}


/*
 * scroll the screen one line down
 * the selected option is only moved if it is scrolled off the screen
 */
static enum option_enum
opt_scroll_down(
	enum option_enum option)
{
	if (last_option_on_screen < last_opt) {
		first_option_on_screen = next_option(first_option_on_screen, TRUE);
		set_last_option_on_screen(first_option_on_screen);
#ifdef USE_CURSES
		do_scroll(1);
		print_any_option(last_option_on_screen);
		stow_cursor();
#else
		show_config_page();
#endif /* USE_CURSES */
		if (option < first_option_on_screen) {
			option = first_option_on_screen;
			if (option_is_title(option))
				option = next_option(option, FALSE);
#ifdef USE_CURSES
			highlight_option(option);
#endif /* USE_CURSES */
		}
#ifndef USE_CURSES
		/* in the !USE_CURSES case we must always highlight the option */
		highlight_option(option);
#endif /* !USE_CURSES */
	}
	return option;
}


/*
 * scroll the screen one line up
 * the selected option is only moved if it is scrolled off the screen
 */
static enum option_enum
opt_scroll_up(
	enum option_enum option)
{
	if (first_option_on_screen > 0) {
		first_option_on_screen = prev_option(first_option_on_screen, TRUE);
		set_last_option_on_screen(first_option_on_screen);
#ifdef USE_CURSES
		do_scroll(-1);
		print_any_option(first_option_on_screen);
		stow_cursor();
#else
		show_config_page();
#endif /* USE_CURSES */
		if (option > last_option_on_screen) {
			option = last_option_on_screen;
			if (option_is_title(option))
				option = prev_option(option, FALSE);
#ifdef USE_CURSES
			highlight_option(option);
#endif /* USE_CURSES */
		}
#ifndef USE_CURSES
		/* in the !USE_CURSES case we must always highlight the option */
		highlight_option(option);
#endif /* !USE_CURSES */
	}
	return option;
}


/*
 * returns the next visible option
 * if 'incl_titles' is TRUE titles are also returned else they are skipped
 */
static enum option_enum
next_option(
	enum option_enum option,
	t_bool incl_titles)
{
	do {
		option++;
		if (option > last_opt)
			option = 0;
	} while (!(option_is_visible(option) && (incl_titles || !option_is_title(option))));

	return option;
}


/*
 * returns the previous visible option
 * if 'incl_titles' is TRUE titles are also returned else they are skipped
 */
static enum option_enum
prev_option(
	enum option_enum option,
	t_bool incl_titles)
{
	do {
		if (option == 0)
			option = last_opt;
		else
			option--;
	} while (!(option_is_visible(option) && (incl_titles || !option_is_title(option))));

	return option;
}


/*
 * set first_option_on_screen in such way that 'last_option' will be
 * the last option on the screen
 */
static void
set_first_option_on_screen(
	enum option_enum last_option)
{
	int i;

	first_option_on_screen = last_option;
	for (i = 1; i < option_lines_per_page && first_option_on_screen > 0; i++)
		first_option_on_screen = prev_option(first_option_on_screen, TRUE);

	/*
	 * make sure that the first page is used completely
	 */
	if (first_option_on_screen == 0)
		set_last_option_on_screen(0);
}


/*
 * set last_option_on_screen in such way that 'first_option' will be
 * the first option on the screen
 */
static void
set_last_option_on_screen(
	enum option_enum first_option)
{
	int i;

	last_option_on_screen = first_option;
	/*
	 * on last page, there need not be option_lines_per_page options
	 */
	for (i = 1; i < option_lines_per_page && last_option_on_screen < last_opt; i++)
		last_option_on_screen = next_option(last_option_on_screen, TRUE);
}


static void
highlight_option(
	enum option_enum option)
{
	refresh_config_page(option); /* to keep refresh_config_page():last_option up-to-date */
	draw_arrow_mark(option_row(option));
	info_message("%s", _(option_table[option].txt->opt));
}


static void
unhighlight_option(
	enum option_enum option)
{
	/* Astonishing hack */
	t_menu *savemenu = currmenu;
	t_menu cfgmenu = { 0, 1, 0, NULL, NULL, NULL };

	currmenu = &cfgmenu;
	currmenu->curr = option_row(option) - INDEX_TOP;
	erase_arrow();
	currmenu = savemenu;
	clear_message();
}


/*
 * Refresh the config page which holds the actual option. If act_option is
 * smaller zero fall back on the last given option (first option if there was
 * no last option) and refresh the screen.
 */
void
refresh_config_page(
	enum option_enum act_option)
{
	static enum option_enum last_option = 0;
	/* t_bool force_redraw = FALSE; */

	if (act_option == SIGNAL_HANDLER) {	/* called by signal handler */
		/* force_redraw = TRUE; */
		act_option = last_option;
		set_last_option_on_screen(first_option_on_screen); /* terminal size may have changed */
		if (!option_on_page(last_option)) {
			last_option_on_screen = last_option;
			set_first_option_on_screen(last_option);
		}
		redraw_screen(last_option);
	}
	last_option = act_option;
}


static void
redraw_screen(
	enum option_enum option)
{
	show_config_page();
	highlight_option(option);
}


/*
 * show_menu_help
 */
void
show_menu_help(
	const char *help_message)
{
	MoveCursor(cLINES - 2, 0);
	CleartoEOLN();
	center_line(cLINES - 2, FALSE, _(help_message));
}


/*
 * display current configuration page
 */
static void
show_config_page(
	void)
{
	enum option_enum i;

	signal_context = curr_scope ? cAttrib : cConfig;
	mark_offset = 0;

	ClearScreen();
	center_line(0, TRUE, curr_scope ? curr_scope->scope : _(txt_options_menu));

	for (i = first_option_on_screen; i <= last_option_on_screen; i++) {
		while (!option_is_visible(i))
			i++;
		if (i > last_opt)
			break;
		print_any_option(i);
	}

	show_menu_help(txt_select_config_file_option);
	my_flush();
	stow_cursor();
}


/*
 * Check if score_kill is <= score_limit_kill and if score_select >= score_limit_select
 */
void
check_score_defaults(
	void)
{
	if (tinrc.score_kill > tinrc.score_limit_kill)
		tinrc.score_kill = tinrc.score_limit_kill;

	if (tinrc.score_select < tinrc.score_limit_select)
		tinrc.score_select = tinrc.score_limit_select;
}


static t_function
option_left(
	void)
{
	return GLOBAL_QUIT;
}


static t_function
option_right(
	void)
{
	return CONFIG_SELECT;
}


/*
 * set last_opt to the last visible option
 */
static void
set_last_opt(
	void)
{
	enum option_enum i;

	for (i = 0; i <= LAST_OPT; i++) {
		if (option_is_visible(i))
			last_opt = i;
	}
}


/*
 * returns the first visible option
 */
static enum option_enum
get_first_opt(
	void)
{
	enum option_enum i;

	for (i = 0; i <= last_opt; i++) {
		if (option_is_visible(i) && !option_is_title(i))
			break;
	}
	return i;
}


/*
 * options menu so that the user can dynamically change parameters
 */
void
config_page(
	const char *grpname,
	enum context level)
{
	char key[MAXKEYLEN];
	enum option_enum option, old_option;
	enum {
		NOT_CHANGED			= 0,
		MISC_OPTS			= 1 << 0,
		DISPLAY_OPTS		= 1 << 1,
		SCORE_OPTS			= 1 << 2,
		SHOW_AUTHOR			= 1 << 3,
		SHOW_ONLY_UNREAD	= 1 << 4,
		SORT_OPTS			= 1 << 5,
		THREAD_ARTS			= 1 << 6,
		THREAD_SCORE		= 1 << 7
	} changed = NOT_CHANGED;
	int i, scope_idx = 0;
	t_bool change_option = FALSE;
	t_function func;
#ifdef CHARSET_CONVERSION
	t_bool is_7bit;
#endif /* CHARSET_CONVERSION */
	unsigned old_show_author = 0, old_show_unread = 0, old_thread_arts = 0;

	if (curr_scope)
		initialize_attributes();
	if (grpname && curr_group) {
		/*
		 * These things can be toggled by the user,
		 * keep a copy of the current value to restore
		 * the state if necessary
		 */
		old_show_author = curr_group->attribute->show_author;
		old_show_unread = curr_group->attribute->show_only_unread_arts;
		old_thread_arts = curr_group->attribute->thread_articles;
	}
	set_last_opt();
	option = get_first_opt();
	first_option_on_screen = 0;
	set_last_option_on_screen(0);

	redraw_screen(option);
	set_xclick_off();

	forever {
		switch ((func = handle_keypad(option_left, option_right, NULL, option_menu_keys))) {
			case GLOBAL_QUIT:
				if (grpname) {
					if (curr_scope && scope_is_empty()) {
						/*
						 * Called via TAB from Config 'M'enu and all attributes
						 * have default values -> delete scope
						 */
						do_delete_scope(scope_idx);
						curr_scope = NULL;
					}
					if (changed) {
						/*
						 * At least one option or attribute has changed,
						 * write config files
						 */
						write_config_file(local_config_file);
						write_attributes_file(local_attributes_file);
					}
				}
				/* FALLTHROUGH */
			case CONFIG_NO_SAVE:
				if (grpname && curr_scope) {
					/*
					 * Called via TAB from Config 'M'enu,
					 * delete scope if all attributes have default values
					 */
					if (scope_is_empty())
						do_delete_scope(scope_idx);
					curr_scope = NULL;
				}
				assign_attributes_to_groups();
				if (grpname && curr_group) {
					/*
					 * These things can be toggled by the user,
					 * restore the cached state if no changes were made
					 */
					if (!(changed & SHOW_AUTHOR))
						curr_group->attribute->show_author = old_show_author;
					if (!(changed & SHOW_ONLY_UNREAD))
						curr_group->attribute->show_only_unread_arts = old_show_unread;
					if (!(changed & THREAD_ARTS))
						curr_group->attribute->thread_articles = old_thread_arts;

					if (changed) {
						t_bool filtered = FALSE;
						t_bool old_keep_in_base = TRUE;

						/*
						 * recook if an article is open
						 */
						if (changed & DISPLAY_OPTS) {
							if (pgart.raw)
								resize_article(TRUE, &pgart);
						}
						/*
						 * Clear art->keep_in_base if switching to !show_only_unread_arts
						 */
						if ((changed & SHOW_ONLY_UNREAD) && !curr_group->attribute->show_only_unread_arts) {
							for_each_art(i)
								arts[i].keep_in_base = FALSE;
						}

						if (changed & SCORE_OPTS) {
							unfilter_articles(curr_group);
							read_filter_file(filter_file);
							filtered = filter_articles(curr_group);
						}
						/*
						 * If the sorting/threading strategy of threads or filter options have
						 * changed, fix things so that resorting will occur
						 *
						 * If show_only_unread_arts or the scoring of a thread has changed,
						 * resort base[] (find_base() is called inside make_threads() too, so
						 * do this only if make_threads() was not called before)
						 *
						 * If we were called from page level, keep the current article in
						 * base[]. This prevents that find_base() removes the current article
						 * after switching to show_only_unread.
						 */
						if (level == cPage) {
							old_keep_in_base = arts[this_resp].keep_in_base;
							arts[this_resp].keep_in_base = TRUE;
						}
						if (changed & (THREAD_ARTS | SORT_OPTS))
							make_threads(curr_group, TRUE);
						else if (filtered)
							make_threads(curr_group, FALSE);
						else if (changed & (SHOW_ONLY_UNREAD | THREAD_SCORE))
							find_base(curr_group);

						if (level == cPage)
							arts[this_resp].keep_in_base = old_keep_in_base;
					}
				}
				clear_note_area();
				return;

			case GLOBAL_HELP:
				if (curr_scope)
					show_help_page(ATTRIB_LEVEL, _(txt_attrib_menu_com));
				else
					show_help_page(CONFIG_LEVEL, _(txt_options_menu_com));
				redraw_screen(option);
				break;

			case GLOBAL_LINE_UP:
				unhighlight_option(option);
				option = move_cursor(option, FALSE);
				highlight_option(option);
				break;

			case GLOBAL_LINE_DOWN:
				unhighlight_option(option);
				option = move_cursor(option, TRUE);
				highlight_option(option);
				break;

			case GLOBAL_FIRST_PAGE:
				unhighlight_option(option);
				option = get_first_opt();
				first_option_on_screen = 0;
				set_last_option_on_screen(0);
				redraw_screen(option);
				/* highlight_option(option); is already done by redraw_screen() */
				break;

			case GLOBAL_LAST_PAGE:
				unhighlight_option(option);
				option = last_opt;
				last_option_on_screen = last_opt;
				set_first_option_on_screen(last_opt);
				redraw_screen(option);
				/* highlight_option(option); is already done by redraw_screen() */
				break;

			case GLOBAL_PAGE_UP:
				unhighlight_option(option);
				if (option != first_option_on_screen && !(option_is_title(first_option_on_screen) && option == next_option(first_option_on_screen, FALSE))) {
					option = first_option_on_screen;
					if (option_is_title(option))
						option = next_option(option, FALSE);
					highlight_option(option);
					break;
				} else if (tinrc.scroll_lines == -2 && first_option_on_screen != 0) {
					i = option_lines_per_page / 2;

					for (; i > 0; i--) {
						last_option_on_screen = prev_option(last_option_on_screen, TRUE);
						if (last_option_on_screen == last_opt)	/* end on wrap around */
							break;
					}
				} else
					last_option_on_screen = prev_option(first_option_on_screen, TRUE);

				set_first_option_on_screen(last_option_on_screen);
				if (last_option_on_screen == last_opt)
					option = last_option_on_screen;
				else
					option = first_option_on_screen;
				if (option_is_title(option))
					option = next_option(option, FALSE);
				redraw_screen(option);
				/* highlight_option(option); is already done by redraw_screen() */
				break;

			case GLOBAL_PAGE_DOWN:
				unhighlight_option(option);
				if (option == last_opt) {
					/* wrap around */
					first_option_on_screen = 0;
					option = 0;
				} else {
					enum option_enum old_first = first_option_on_screen;

					if (tinrc.scroll_lines == -2) {
						i = option_lines_per_page / 2;

						for (; i > 0; i--) {
							first_option_on_screen = next_option(first_option_on_screen, TRUE);
							if (first_option_on_screen == 0)	/* end on wrap_around */
								break;
						}
					} else
						first_option_on_screen = next_option(last_option_on_screen, TRUE);

					if (first_option_on_screen == 0) {
						first_option_on_screen = old_first;
						option = last_opt;
						highlight_option(option);
						break;
					} else
						option = first_option_on_screen;
				}

				set_last_option_on_screen(first_option_on_screen);
				if (option_is_title(option))
					option = next_option(option, FALSE);
				redraw_screen(option);
				/* highlight_option(option); is already done by redraw_screen() */
				break;

			case GLOBAL_SCROLL_UP:
				option = opt_scroll_up(option);
				break;

			case GLOBAL_SCROLL_DOWN:
				option = opt_scroll_down(option);
				break;

			case DIGIT_1:
			case DIGIT_2:
			case DIGIT_3:
			case DIGIT_4:
			case DIGIT_5:
			case DIGIT_6:
			case DIGIT_7:
			case DIGIT_8:
			case DIGIT_9:
				unhighlight_option(option);
				option = set_option_num(prompt_num(func_to_key(func, option_menu_keys), _(txt_enter_option_num)));
				if (!option_on_page(option)) {
					first_option_on_screen = option;
					set_last_option_on_screen(option);
					redraw_screen(option);
				} else
					highlight_option(option);
				break;

#ifndef NO_SHELL_ESCAPE
			case GLOBAL_SHELL_ESCAPE:
				shell_escape();
				redraw_screen(option);
				break;
#endif /* !NO_SHELL_ESCAPE */

			case GLOBAL_SEARCH_SUBJECT_FORWARD:
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
			case GLOBAL_SEARCH_REPEAT:
				if (func == GLOBAL_SEARCH_REPEAT && last_search != GLOBAL_SEARCH_SUBJECT_FORWARD && last_search != GLOBAL_SEARCH_SUBJECT_BACKWARD)
					info_message(_(txt_no_prev_search));
				else {
					old_option = option;
					option = search_config((func == GLOBAL_SEARCH_SUBJECT_FORWARD), (func == GLOBAL_SEARCH_REPEAT), option, last_opt);
					if (option != old_option) {
						unhighlight_option(old_option);
						if (!option_on_page(option)) {
							first_option_on_screen = option;
							set_last_option_on_screen(option);
							redraw_screen(option);
						} else
							highlight_option(option);
					}
				}
				break;

			case CONFIG_SCOPE_MENU:
				if (!curr_scope) {
					scope_page(level);
					set_last_opt();
					option = get_first_opt();
					first_option_on_screen = 0;
					set_last_option_on_screen(0);
					redraw_screen(option);
				}
				break;

			case CONFIG_RESET_ATTRIB:
				if (curr_scope) {
					if (curr_scope->global)
						info_message(_(txt_scope_operation_not_allowed));
					else if (check_state(option)) {
						reset_state(option);
						changed |= MISC_OPTS;
						redraw_screen(option);
					}
				}
				break;

			case CONFIG_SELECT:
				if (curr_scope && curr_scope->global)
					info_message(_(txt_scope_operation_not_allowed));
				else
					change_option = TRUE;
				break;

			case CONFIG_TOGGLE_ATTRIB:
				if (grpname) {
					if (curr_scope) {
						if (scope_is_empty()) {
							do_delete_scope(scope_idx);
							scope_idx = 0;
						}
						curr_scope = NULL;
					} else {
						if (!(scope_idx = find_scope(grpname)))
							scope_idx = add_scope(grpname);
						if (scope_idx) {
							curr_scope = &scopes[scope_idx];
							initialize_attributes();
						}
					}
					set_last_opt();
					option = get_first_opt();
					first_option_on_screen = 0;
					set_last_option_on_screen(0);
					redraw_screen(option);
				}
				break;

			case GLOBAL_REDRAW_SCREEN:
				my_retouch();
				set_xclick_off();
				set_last_option_on_screen(first_option_on_screen);
				redraw_screen(option);
				break;

			case GLOBAL_VERSION:
				info_message(cvers);
				break;

			default:
				info_message(_(txt_bad_command), printascii(key, func_to_key(GLOBAL_HELP, option_menu_keys)));
				break;
		} /* switch (ch) */

		if (change_option) {
			switch (option_table[option].var_type) {
				case OPT_ON_OFF:
					switch (option) {
						case OPT_ABBREVIATE_GROUPNAME:
						case OPT_AUTO_RECONNECT:
						case OPT_CACHE_OVERVIEW_FILES:
						case OPT_CATCHUP_READ_GROUPS:
						case OPT_FORCE_SCREEN_REDRAW:
						case OPT_KEEP_DEAD_ARTICLES:
						case OPT_SHOW_ONLY_UNREAD_GROUPS:
						case OPT_STRIP_BLANKS:
						case OPT_STRIP_NEWSRC:
#if defined(HAVE_ICONV_OPEN_TRANSLIT) && defined(CHARSET_CONVERSION)
						case OPT_TRANSLIT:
#endif /* HAVE_ICONV_OPEN_TRANSLIT && CHARSET_CONVERSION */
						case OPT_UNLINK_ARTICLE:
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
						case OPT_UTF8_GRAPHICS:
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
						case OPT_URL_HIGHLIGHT:
#ifdef HAVE_KEYPAD
						case OPT_USE_KEYPAD:
#endif /* HAVE_KEYPAD */
						case OPT_USE_MOUSE:
							if (prompt_option_on_off(option))
								changed |= MISC_OPTS;
							break;

						case OPT_ADD_POSTED_TO_FILTER:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(add_posted_to_filter);
							break;

						case OPT_ADVERTISING:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(advertising);
							break;

						case OPT_ALTERNATIVE_HANDLING:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(alternative_handling);
							break;

						case OPT_ASK_FOR_METAMAIL:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(ask_for_metamail);
							break;

						case OPT_AUTO_LIST_THREAD:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(auto_list_thread);
							break;

						case OPT_AUTO_SAVE:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(auto_save);
							break;

						case OPT_BATCH_SAVE:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(batch_save);
							break;

						case OPT_GROUP_CATCHUP_ON_EXIT:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(group_catchup_on_exit);
							break;

						case OPT_MARK_IGNORE_TAGS:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(mark_ignore_tags);
							break;

						case OPT_MARK_SAVED_READ:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(mark_saved_read);
							break;

						case OPT_POST_PROCESS_VIEW:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(post_process_view);
							break;

						case OPT_POS_FIRST_UNREAD:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(pos_first_unread);
							break;

#ifndef DISABLE_PRINTING
						case OPT_PRINT_HEADER:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(print_header);
							break;
#endif /* !DISABLE_PRINTING */

						case OPT_PROCESS_ONLY_UNREAD:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(process_only_unread);
							break;

						case OPT_PROMPT_FOLLOWUPTO:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(prompt_followupto);
							break;

						case OPT_SHOW_SIGNATURES:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(show_signatures);
							break;

						case OPT_SIGDASHES:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(sigdashes);
							break;

						case OPT_SIGNATURE_REPOST:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(signature_repost);
							break;

						case OPT_START_EDITOR_OFFSET:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(start_editor_offset);
							break;

						case OPT_TEX2ISO_CONV:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(tex2iso_conv);
							break;

						case OPT_THREAD_CATCHUP_ON_EXIT:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(thread_catchup_on_exit);
							break;

						case OPT_WRAP_ON_NEXT_UNREAD:
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(wrap_on_next_unread);
							break;

						case OPT_VERBATIM_HANDLING:
							/*
							 * option toggles visibility of other
							 * options -> needs redraw_screen()
							 */
							if (prompt_option_on_off(option)) {
								UPDATE_INT_ATTRIBUTES(verbatim_handling);
								set_last_option_on_screen(first_option_on_screen);
								redraw_screen(option);
							}
							break;

						/* show mini help menu */
						case OPT_BEGINNER_LEVEL:
							if (prompt_option_on_off(option)) {
								set_noteslines(cLINES);
								changed |= MISC_OPTS;
							}
							break;

						/* show all arts or just new/unread arts */
						case OPT_SHOW_ONLY_UNREAD_ARTS:
							if (prompt_option_on_off(option)) {
								UPDATE_INT_ATTRIBUTES(show_only_unread_arts);
								changed |= SHOW_ONLY_UNREAD;
							}
							break;

						/* draw -> / highlighted bar */
						case OPT_DRAW_ARROW:
							if (prompt_option_on_off(option)) {
								unhighlight_option(option);
								if (!tinrc.draw_arrow && !tinrc.inverse_okay) {
									tinrc.inverse_okay = TRUE;
									repaint_option(OPT_INVERSE_OKAY);
								}
								changed |= MISC_OPTS;
							}
							break;

						/* draw inversed screen header lines */
						/* draw inversed group/article/option line if draw_arrow is OFF */
						case OPT_INVERSE_OKAY:
							if (prompt_option_on_off(option)) {
								unhighlight_option(option);
								if (!tinrc.draw_arrow && !tinrc.inverse_okay) {
									tinrc.draw_arrow = TRUE;	/* we don't want to navigate blindly */
									repaint_option(OPT_DRAW_ARROW);
								}
								changed |= MISC_OPTS;
							}
							break;

						case OPT_MAIL_8BIT_HEADER:
							if (prompt_option_on_off(option)) {
								if (tinrc.mail_mime_encoding != MIME_ENCODING_8BIT) {
									tinrc.mail_8bit_header = FALSE;
									print_any_option(OPT_MAIL_8BIT_HEADER);
								}
								UPDATE_INT_ATTRIBUTES(mail_8bit_header);
							}
							break;

						case OPT_POST_8BIT_HEADER:
							if (prompt_option_on_off(option)) {
								/* if post_mime_encoding != 8bit, post_8bit_header is disabled */
								if (tinrc.post_mime_encoding != MIME_ENCODING_8BIT) {
									tinrc.post_8bit_header = FALSE;
									print_any_option(OPT_POST_8BIT_HEADER);
								}
								UPDATE_INT_ATTRIBUTES(post_8bit_header);
							}
							break;

						/* show newsgroup description text next to newsgroups */
						case OPT_SHOW_DESCRIPTION:
							if (prompt_option_on_off(option)) {
								show_description = tinrc.show_description;
								if (show_description)			/* force reread of newgroups file */
									read_descriptions(FALSE);
								changed |= MISC_OPTS;
							}
							break;

#ifdef HAVE_COLOR
						/* use ANSI color */
						case OPT_USE_COLOR:
							if (prompt_option_on_off(option)) {
#	ifdef USE_CURSES
								if (!has_colors())
									use_color = FALSE;
								else
#	endif /* USE_CURSES */
									use_color = tinrc.use_color;
								set_last_option_on_screen(first_option_on_screen);
								redraw_screen(option);
								changed |= MISC_OPTS;
							}
							break;
#endif /* HAVE_COLOR */

#ifdef XFACE_ABLE
						/* use slrnface */
						case OPT_USE_SLRNFACE:
							if (prompt_option_on_off(option)) {
								if (!tinrc.use_slrnface)
									slrnface_stop();
								else
									slrnface_start();
								changed |= MISC_OPTS;
							}
							break;
#endif /* XFACE_ABLE */

						/* word_highlight */
						case OPT_WORD_HIGHLIGHT:
							if (prompt_option_on_off(option)) {
								word_highlight = tinrc.word_highlight;
								set_last_option_on_screen(first_option_on_screen);
								redraw_screen(option);
								changed |= MISC_OPTS;
							}
							break;

#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
						case OPT_RENDER_BIDI:
							if (prompt_option_on_off(option))
								changed |= MISC_OPTS;
							break;
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */

						case OPT_ATTRIB_ADD_POSTED_TO_FILTER:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(add_posted_to_filter);
							break;

						case OPT_ATTRIB_ADVERTISING:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(advertising);
							break;

						case OPT_ATTRIB_ALTERNATIVE_HANDLING:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(alternative_handling);
							break;

						case OPT_ATTRIB_ASK_FOR_METAMAIL:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(ask_for_metamail);
							break;

						case OPT_ATTRIB_AUTO_LIST_THREAD:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(auto_list_thread);
							break;

						case OPT_ATTRIB_AUTO_SAVE:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(auto_save);
							break;

						case OPT_ATTRIB_AUTO_SELECT:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(auto_select);
							break;

						case OPT_ATTRIB_BATCH_SAVE:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(batch_save);
							break;

						case OPT_ATTRIB_DELETE_TMP_FILES:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(delete_tmp_files);
							break;

						case OPT_ATTRIB_GROUP_CATCHUP_ON_EXIT:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(group_catchup_on_exit);
							break;

						case OPT_ATTRIB_MAIL_8BIT_HEADER:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(mail_8bit_header);
							break;

						case OPT_ATTRIB_MARK_IGNORE_TAGS:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(mark_ignore_tags);
							break;

						case OPT_ATTRIB_MARK_SAVED_READ:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(mark_saved_read);
							break;

						case OPT_ATTRIB_MIME_FORWARD:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(mime_forward);
							break;

						case OPT_ATTRIB_POST_8BIT_HEADER:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(post_8bit_header);
							break;

						case OPT_ATTRIB_POST_PROCESS_VIEW:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(post_process_view);
							break;

						case OPT_ATTRIB_POS_FIRST_UNREAD:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(pos_first_unread);
							break;

#ifndef DISABLE_PRINTING
						case OPT_ATTRIB_PRINT_HEADER:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(print_header);
							break;
#endif /* !DISABLE_PRINTING */

						case OPT_ATTRIB_PROCESS_ONLY_UNREAD:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(process_only_unread);
							break;

						case OPT_ATTRIB_PROMPT_FOLLOWUPTO:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(prompt_followupto);
							break;

						case OPT_ATTRIB_QUICK_KILL_CASE:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(quick_kill_case);
							break;

						case OPT_ATTRIB_QUICK_KILL_EXPIRE:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(quick_kill_expire);
							break;

						case OPT_ATTRIB_QUICK_SELECT_CASE:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(quick_select_case);
							break;

						case OPT_ATTRIB_QUICK_SELECT_EXPIRE:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(quick_select_expire);
							break;

						case OPT_ATTRIB_SHOW_ONLY_UNREAD_ARTS:
							if (prompt_option_on_off(option)) {
								SET_NUM_ATTRIBUTE(show_only_unread_arts);
								changed |= SHOW_ONLY_UNREAD;
							}
							break;

						case OPT_ATTRIB_SHOW_SIGNATURES:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(show_signatures);
							break;

						case OPT_ATTRIB_SIGDASHES:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(sigdashes);
							break;

						case OPT_ATTRIB_SIGNATURE_REPOST:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(signature_repost);
							break;

						case OPT_ATTRIB_START_EDITOR_OFFSET:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(start_editor_offset);
							break;

						case OPT_ATTRIB_TEX2ISO_CONV:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(tex2iso_conv);
							break;

						case OPT_ATTRIB_THREAD_CATCHUP_ON_EXIT:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(thread_catchup_on_exit);
							break;

						case OPT_ATTRIB_VERBATIM_HANDLING:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(verbatim_handling);
							break;

						case OPT_ATTRIB_WRAP_ON_NEXT_UNREAD:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(wrap_on_next_unread);
							break;

						case OPT_ATTRIB_X_COMMENT_TO:
							if (prompt_option_on_off(option))
								SET_NUM_ATTRIBUTE(x_comment_to);
							break;

						default:
							break;
					} /* switch (option) */
					break;

				case OPT_LIST:
					switch (option) {
#ifdef HAVE_COLOR
						case OPT_COL_FROM:
						case OPT_COL_HEAD:
						case OPT_COL_HELP:
						case OPT_COL_INVERS_BG:
						case OPT_COL_INVERS_FG:
						case OPT_COL_MESSAGE:
						case OPT_COL_MINIHELP:
						case OPT_COL_NEWSHEADERS:
						case OPT_COL_QUOTE:
						case OPT_COL_QUOTE2:
						case OPT_COL_QUOTE3:
						case OPT_COL_RESPONSE:
						case OPT_COL_SIGNATURE:
						case OPT_COL_SUBJECT:
						case OPT_COL_TEXT:
						case OPT_COL_TITLE:
						case OPT_COL_MARKSTAR:
						case OPT_COL_MARKDASH:
						case OPT_COL_MARKSLASH:
						case OPT_COL_MARKSTROKE:
						case OPT_COL_URLS:
						case OPT_COL_VERBATIM:
#endif /* HAVE_COLOR */
						case OPT_CONFIRM_CHOICE:
						case OPT_GOTO_NEXT_UNREAD:
						case OPT_HIDE_UUE:
						case OPT_INTERACTIVE_MAILER:
						case OPT_KILL_LEVEL:
						case OPT_MAILBOX_FORMAT:
						case OPT_MONO_MARKDASH:
						case OPT_MONO_MARKSLASH:
						case OPT_MONO_MARKSTAR:
						case OPT_MONO_MARKSTROKE:
#ifdef HAVE_UNICODE_NORMALIZATION
						case OPT_NORMALIZATION_FORM:
#endif /* HAVE_UNICODE_NORMALIZATION */
						case OPT_QUOTE_STYLE:
						case OPT_STRIP_BOGUS:
						case OPT_WILDCARD:
						case OPT_WORD_H_DISPLAY_MARKS:
#ifdef USE_HEAPSORT
						case OPT_SORT_FUNCTION:
#endif /* USE_HEAPSORT */
							if (prompt_option_list(option))
								changed |= MISC_OPTS;
							break;

#ifdef HAVE_COLOR
						case OPT_COL_BACK:
						case OPT_COL_NORMAL:
							if (prompt_option_list(option)) {
								redraw_screen(option);
								changed |= MISC_OPTS;
							}
							break;
#endif /* HAVE_COLOR */

						case OPT_AUTO_CC_BCC:
							if (prompt_option_list(option))
								UPDATE_INT_ATTRIBUTES(auto_cc_bcc);
							break;

						case OPT_THREAD_ARTICLES:
							if (prompt_option_list(option)) {
								UPDATE_INT_ATTRIBUTES(thread_articles);
								changed |= THREAD_ARTS;
							}
							break;

						case OPT_SORT_ARTICLE_TYPE:
							if (prompt_option_list(option)) {
								UPDATE_INT_ATTRIBUTES(sort_article_type);
								changed |= SORT_OPTS;
							}
							break;

						case OPT_SORT_THREADS_TYPE:
							if (prompt_option_list(option)) {
								UPDATE_INT_ATTRIBUTES(sort_threads_type);
								changed |= SORT_OPTS;
							}
							break;

						case OPT_THREAD_SCORE:
							if (prompt_option_list(option))
								changed |= THREAD_SCORE;
							break;

						case OPT_TRIM_ARTICLE_BODY:
							if (prompt_option_list(option))
								UPDATE_INT_ATTRIBUTES(trim_article_body);
							break;

						case OPT_POST_PROCESS_TYPE:
							if (prompt_option_list(option))
								UPDATE_INT_ATTRIBUTES(post_process_type);
							break;

						case OPT_SHOW_AUTHOR:
							if (prompt_option_list(option)) {
								UPDATE_INT_ATTRIBUTES(show_author);
								changed |= SHOW_AUTHOR;
							}
							break;

						case OPT_MAIL_MIME_ENCODING:
							if (prompt_option_list(option)) {
#ifdef CHARSET_CONVERSION
								/*
								 * check if we have selected a !7bit encoding but a 7bit network charset
								 * or a !8bit encoding but a 8bit network charset, update encoding if needed
								 */
								is_7bit = FALSE;
								for (i = 0; txt_mime_7bit_charsets[i] != NULL; i++) {
									if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
										is_7bit = TRUE;
										break;
									}
								}
								if (is_7bit) {
									if (tinrc.mail_mime_encoding != MIME_ENCODING_7BIT) {
										tinrc.mail_mime_encoding = MIME_ENCODING_7BIT;
										repaint_option(OPT_MAIL_MIME_ENCODING);
									}
								} else {
									if (tinrc.mail_mime_encoding == MIME_ENCODING_7BIT) {
										tinrc.mail_mime_encoding = MIME_ENCODING_QP;
										repaint_option(OPT_MAIL_MIME_ENCODING);
									}
								}
#endif /* CHARSET_CONVERSION */
								UPDATE_INT_ATTRIBUTES(mail_mime_encoding);
								/* do not use 8 bit headers if mime encoding is not 8bit */
								if (tinrc.mail_mime_encoding != MIME_ENCODING_8BIT) {
									tinrc.mail_8bit_header = FALSE;
									repaint_option(OPT_MAIL_8BIT_HEADER);
									UPDATE_INT_ATTRIBUTES(mail_8bit_header);
								}
							}
							break;

						case OPT_POST_MIME_ENCODING:
							if (prompt_option_list(option)) {
#ifdef CHARSET_CONVERSION
								/*
								 * check if we have selected a !7bit encoding but a 7bit network charset
								 * or a !8bit encoding but a 8bit network charset, update encoding if needed
								 */
								is_7bit = FALSE;
								for (i = 0; txt_mime_7bit_charsets[i] != NULL; i++) {
									if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
										is_7bit = TRUE;
										break;
									}
								}
								if (is_7bit) {
									if (tinrc.post_mime_encoding != MIME_ENCODING_7BIT) {
										tinrc.post_mime_encoding = MIME_ENCODING_7BIT;
										repaint_option(OPT_POST_MIME_ENCODING);
									}
								} else {
									if (tinrc.post_mime_encoding == MIME_ENCODING_7BIT) {
										tinrc.post_mime_encoding = MIME_ENCODING_8BIT;
										repaint_option(OPT_POST_MIME_ENCODING);
									}
								}
#endif /* CHARSET_CONVERSION */
								UPDATE_INT_ATTRIBUTES(post_mime_encoding);
								/* do not use 8 bit headers if mime encoding is not 8bit */
								if (tinrc.post_mime_encoding != MIME_ENCODING_8BIT) {
									tinrc.post_8bit_header = FALSE;
									repaint_option(OPT_POST_8BIT_HEADER);
									UPDATE_INT_ATTRIBUTES(post_8bit_header);
								}
							}
							break;

#ifdef CHARSET_CONVERSION
						case OPT_MM_NETWORK_CHARSET:
							if (prompt_option_list(option)) {
								/*
								 * check if we have selected a 7bit charset but a !7bit encoding
								 * or a 8bit charset but a !8bit encoding, update encoding if needed
								 *
								 * if (mail|post)_mime_encoding != 8bit, disable (mail|post)_8bit_header
								 */
								is_7bit = FALSE;
								UPDATE_INT_ATTRIBUTES(mm_network_charset);
								for (i = 0; txt_mime_7bit_charsets[i] != NULL; i++) {
									if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
										is_7bit = TRUE;
										break;
									}
								}
								if (is_7bit) {
									if (tinrc.mail_mime_encoding != MIME_ENCODING_7BIT) {
										tinrc.mail_mime_encoding = MIME_ENCODING_7BIT;
										tinrc.mail_8bit_header = FALSE;
										repaint_option(OPT_MAIL_MIME_ENCODING);
										repaint_option(OPT_MAIL_8BIT_HEADER);
										UPDATE_INT_ATTRIBUTES(mail_mime_encoding);
										UPDATE_INT_ATTRIBUTES(mail_8bit_header);
									}
									if (tinrc.post_mime_encoding != MIME_ENCODING_7BIT) {
										tinrc.post_mime_encoding = MIME_ENCODING_7BIT;
										tinrc.post_8bit_header = FALSE;
										repaint_option(OPT_POST_MIME_ENCODING);
										repaint_option(OPT_POST_8BIT_HEADER);
										UPDATE_INT_ATTRIBUTES(post_mime_encoding);
										UPDATE_INT_ATTRIBUTES(post_8bit_header);
									}
								} else {
									if (tinrc.mail_mime_encoding == MIME_ENCODING_7BIT) {
										tinrc.mail_mime_encoding = MIME_ENCODING_QP;
										repaint_option(OPT_MAIL_MIME_ENCODING);
										UPDATE_INT_ATTRIBUTES(mail_mime_encoding);
									}
									if (tinrc.post_mime_encoding == MIME_ENCODING_7BIT) {
										tinrc.post_mime_encoding = MIME_ENCODING_8BIT;
										repaint_option(OPT_POST_MIME_ENCODING);
										UPDATE_INT_ATTRIBUTES(post_mime_encoding);
									}
								}
							}
							break;
#endif /* CHARSET_CONVERSION */

						case OPT_ATTRIB_AUTO_CC_BCC:
							if (prompt_option_list(option))
								SET_NUM_ATTRIBUTE(auto_cc_bcc);
							break;

						case OPT_ATTRIB_MAIL_MIME_ENCODING:
							if (prompt_option_list(option))
								SET_NUM_ATTRIBUTE(mail_mime_encoding);
							break;

#ifdef CHARSET_CONVERSION
						case OPT_ATTRIB_MM_NETWORK_CHARSET:
							if (prompt_option_list(option))
								SET_NUM_ATTRIBUTE(mm_network_charset);
							break;
#endif /* CHARSET_CONVERSION */

						case OPT_ATTRIB_POST_MIME_ENCODING:
							if (prompt_option_list(option))
								SET_NUM_ATTRIBUTE(post_mime_encoding);
							break;

						case OPT_ATTRIB_POST_PROCESS_TYPE:
							if (prompt_option_list(option))
								SET_NUM_ATTRIBUTE(post_process_type);
							break;

						case OPT_ATTRIB_QUICK_KILL_HEADER:
							if (prompt_option_list(option))
								SET_NUM_ATTRIBUTE(quick_kill_header);
							break;

						case OPT_ATTRIB_QUICK_SELECT_HEADER:
							if (prompt_option_list(option))
								SET_NUM_ATTRIBUTE(quick_select_header);
							break;

						case OPT_ATTRIB_SHOW_AUTHOR:
							if (prompt_option_list(option)) {
								SET_NUM_ATTRIBUTE(show_author);
								changed |= SHOW_AUTHOR;
							}
							break;

						case OPT_ATTRIB_SORT_ARTICLE_TYPE:
							if (prompt_option_list(option)) {
								SET_NUM_ATTRIBUTE(sort_article_type);
								changed |= SORT_OPTS;
							}
							break;

						case OPT_ATTRIB_SORT_THREADS_TYPE:
							if (prompt_option_list(option)) {
								SET_NUM_ATTRIBUTE(sort_threads_type);
								changed |= SORT_OPTS;
							}
							break;

						case OPT_ATTRIB_THREAD_ARTICLES:
							if (prompt_option_list(option)) {
								SET_NUM_ATTRIBUTE(thread_articles);
								changed |= THREAD_ARTS;
							}
							break;

						case OPT_ATTRIB_TRIM_ARTICLE_BODY:
							if (prompt_option_list(option))
								SET_NUM_ATTRIBUTE(trim_article_body);
							break;

						default:
							break;
					} /* switch (option) */
					break;

				case OPT_STRING:
					switch (option) {
						case OPT_INEWS_PROG:
						case OPT_MAILDIR:
						case OPT_MAILER_FORMAT:
						case OPT_MAIL_ADDRESS:
						case OPT_MAIL_QUOTE_FORMAT:
						case OPT_METAMAIL_PROG:
						case OPT_NEWS_QUOTE_FORMAT:
						case OPT_SAVEDIR:
						case OPT_SIGFILE:
#ifndef DISABLE_PRINTING
						case OPT_PRINTER:
#endif /* !DISABLE_PRINTING */
						case OPT_QUOTE_CHARS:
						case OPT_SPAMTRAP_WARNING_ADDRESSES:
						case OPT_URL_HANDLER:
						case OPT_XPOST_QUOTE_FORMAT:
							if (prompt_option_string(option))
								changed |= MISC_OPTS;
							break;

						case OPT_EDITOR_FORMAT:
							if (prompt_option_string(option)) {
								if (!strlen(tinrc.editor_format))
									STRCPY(tinrc.editor_format, TIN_EDITOR_FMT_ON);
								changed |= MISC_OPTS;
							}
							break;

						case OPT_GROUP_FORMAT:
							if (prompt_option_string(option)) {
								if (!strlen(tinrc.group_format))
									STRCPY(tinrc.group_format, DEFAULT_GROUP_FORMAT);
								changed |= MISC_OPTS;
							}
							break;

						case OPT_ATTRIB_GROUP_FORMAT:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(group_format);
							break;

#ifndef CHARSET_CONVERSION
						case OPT_MM_CHARSET:
							if (prompt_option_string(option)) {
								/*
								 * No charset conversion available, assume local charset
								 * to be network charset.
								 */
								STRCPY(tinrc.mm_local_charset, tinrc.mm_charset);
								changed |= MISC_OPTS;
							}
							break;
#else
#	ifdef NO_LOCALE
						case OPT_MM_LOCAL_CHARSET:
							if (prompt_option_string(option))
							/* no locales -> can't guess local charset */
								changed |= MISC_OPTS;
							break;

#	endif /* NO_LOCALE */
#endif /* !CHARSET_CONVERSION */

						case OPT_NEWS_HEADERS_TO_DISPLAY:
							if (prompt_option_string(option)) {
								build_news_headers_array(scopes[0].attribute, TRUE);
								changed |= MISC_OPTS;
							}
							break;

						case OPT_NEWS_HEADERS_TO_NOT_DISPLAY:
							if (prompt_option_string(option)) {
								build_news_headers_array(scopes[0].attribute, FALSE);
								changed |= MISC_OPTS;
							}
							break;

						case OPT_POSTED_ARTICLES_FILE:
							if (prompt_option_string(option)) {
								char buf[PATH_LEN];

								strfpath(tinrc.posted_articles_file, buf, sizeof(buf), &CURR_GROUP, TRUE);
								STRCPY(tinrc.posted_articles_file, buf);
								changed |= MISC_OPTS;
							}
							break;

#ifdef HAVE_COLOR
						case OPT_QUOTE_REGEX:
							if (prompt_option_string(option)) {
								FreeIfNeeded(quote_regex.re);
								FreeIfNeeded(quote_regex.extra);
								if (!strlen(tinrc.quote_regex))
									STRCPY(tinrc.quote_regex, DEFAULT_QUOTE_REGEX);
								compile_regex(tinrc.quote_regex, &quote_regex, PCRE_CASELESS);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_QUOTE_REGEX2:
							if (prompt_option_string(option)) {
								FreeIfNeeded(quote_regex2.re);
								FreeIfNeeded(quote_regex2.extra);
								if (!strlen(tinrc.quote_regex2))
									STRCPY(tinrc.quote_regex2, DEFAULT_QUOTE_REGEX2);
								compile_regex(tinrc.quote_regex2, &quote_regex2, PCRE_CASELESS);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_QUOTE_REGEX3:
							if (prompt_option_string(option)) {
								FreeIfNeeded(quote_regex3.re);
								FreeIfNeeded(quote_regex3.extra);
								if (!strlen(tinrc.quote_regex3))
									STRCPY(tinrc.quote_regex3, DEFAULT_QUOTE_REGEX3);
								compile_regex(tinrc.quote_regex3, &quote_regex3, PCRE_CASELESS);
								changed |= DISPLAY_OPTS;
							}
							break;
#endif /* HAVE_COLOR */

						case OPT_SELECT_FORMAT:
							if (prompt_option_string(option)) {
								if (!strlen(tinrc.select_format))
									STRCPY(tinrc.select_format, DEFAULT_SELECT_FORMAT);
								changed |= MISC_OPTS;
							}
							break;

						case OPT_SLASHES_REGEX:
							if (prompt_option_string(option)) {
								FreeIfNeeded(slashes_regex.re);
								FreeIfNeeded(slashes_regex.extra);
								if (!strlen(tinrc.slashes_regex))
									STRCPY(tinrc.slashes_regex, DEFAULT_SLASHES_REGEX);
								compile_regex(tinrc.slashes_regex, &slashes_regex, PCRE_CASELESS);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_STARS_REGEX:
							if (prompt_option_string(option)) {
								FreeIfNeeded(stars_regex.re);
								FreeIfNeeded(stars_regex.extra);
								if (!strlen(tinrc.stars_regex))
									STRCPY(tinrc.stars_regex, DEFAULT_STARS_REGEX);
								compile_regex(tinrc.stars_regex, &stars_regex, PCRE_CASELESS);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_STROKES_REGEX:
							if (prompt_option_string(option)) {
								FreeIfNeeded(strokes_regex.re);
								FreeIfNeeded(strokes_regex.extra);
								if (!strlen(tinrc.strokes_regex))
									STRCPY(tinrc.strokes_regex, DEFAULT_STROKES_REGEX);
								compile_regex(tinrc.strokes_regex, &strokes_regex, PCRE_CASELESS);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_UNDERSCORES_REGEX:
							if (prompt_option_string(option)) {
								FreeIfNeeded(underscores_regex.re);
								FreeIfNeeded(underscores_regex.extra);
								if (!strlen(tinrc.underscores_regex))
									STRCPY(tinrc.underscores_regex, DEFAULT_UNDERSCORES_REGEX);
								compile_regex(tinrc.underscores_regex, &underscores_regex, PCRE_CASELESS);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_STRIP_RE_REGEX:
							if (prompt_option_string(option)) {
								FreeIfNeeded(strip_re_regex.re);
								FreeIfNeeded(strip_re_regex.extra);
								if (!strlen(tinrc.strip_re_regex))
									STRCPY(tinrc.strip_re_regex, DEFAULT_STRIP_RE_REGEX);
								compile_regex(tinrc.strip_re_regex, &strip_re_regex, PCRE_ANCHORED);
								changed |= MISC_OPTS;
							}
							break;

						case OPT_STRIP_WAS_REGEX:
							if (prompt_option_string(option)) {
								FreeIfNeeded(strip_was_regex.re);
								FreeIfNeeded(strip_was_regex.extra);
								if (!strlen(tinrc.strip_was_regex)) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
									if (IS_LOCAL_CHARSET("UTF-8") && utf8_pcre())
										STRCPY(tinrc.strip_was_regex, DEFAULT_U8_STRIP_WAS_REGEX);
									else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
										STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
								}
								compile_regex(tinrc.strip_was_regex, &strip_was_regex, 0);
								changed |= MISC_OPTS;
							}
							break;

						case OPT_THREAD_FORMAT:
							if (prompt_option_string(option)) {
								if (!strlen(tinrc.thread_format))
									STRCPY(tinrc.thread_format, DEFAULT_THREAD_FORMAT);
								changed |= MISC_OPTS;
							}
							break;

						case OPT_ATTRIB_THREAD_FORMAT:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(thread_format);
							break;

						case OPT_VERBATIM_BEGIN_REGEX:
							if (prompt_option_string(option)) {
								FreeIfNeeded(verbatim_begin_regex.re);
								FreeIfNeeded(verbatim_begin_regex.extra);
								if (!strlen(tinrc.verbatim_begin_regex))
									STRCPY(tinrc.verbatim_begin_regex, DEFAULT_VERBATIM_BEGIN_REGEX);
								compile_regex(tinrc.verbatim_begin_regex, &verbatim_begin_regex, PCRE_ANCHORED);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_VERBATIM_END_REGEX:
							if (prompt_option_string(option)) {
								FreeIfNeeded(verbatim_end_regex.re);
								FreeIfNeeded(verbatim_end_regex.extra);
								if (!strlen(tinrc.verbatim_end_regex))
									STRCPY(tinrc.verbatim_end_regex, DEFAULT_VERBATIM_END_REGEX);
								compile_regex(tinrc.verbatim_end_regex, &verbatim_end_regex, PCRE_ANCHORED);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_DATE_FORMAT:
							if (prompt_option_string(option)) {
								if (!strlen(tinrc.date_format))
									STRCPY(tinrc.date_format, DEFAULT_DATE_FORMAT);
								changed |= MISC_OPTS;
							}
							break;

						case OPT_ATTRIB_DATE_FORMAT:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(date_format);
							break;

						case OPT_ATTRIB_EDITOR_FORMAT:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(editor_format);
							break;

						case OPT_ATTRIB_FCC:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(fcc);
							break;

						case OPT_ATTRIB_FOLLOWUP_TO:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(followup_to);
							break;

						case OPT_ATTRIB_FROM:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(from);
							break;

#ifdef HAVE_ISPELL
						case OPT_ATTRIB_ISPELL:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(ispell);
							break;
#endif /* HAVE_ISPELL */

						case OPT_ATTRIB_MAILDIR:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(maildir);
							break;

						case OPT_ATTRIB_MAILING_LIST:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(mailing_list);
							break;

						case OPT_ATTRIB_MIME_TYPES_TO_SAVE:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(mime_types_to_save);
							break;

						case OPT_ATTRIB_NEWS_HEADERS_TO_DISPLAY:
							if (prompt_option_string(option)) {
								SET_STRING_ATTRIBUTE(news_headers_to_display);
								build_news_headers_array(curr_scope->attribute, TRUE);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_ATTRIB_NEWS_HEADERS_TO_NOT_DISPLAY:
							if (prompt_option_string(option)) {
								SET_STRING_ATTRIBUTE(news_headers_to_not_display);
								build_news_headers_array(curr_scope->attribute, FALSE);
								changed |= DISPLAY_OPTS;
							}
							break;

						case OPT_ATTRIB_NEWS_QUOTE_FORMAT:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(news_quote_format);
							break;

						case OPT_ATTRIB_ORGANIZATION:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(organization);
							break;

						case OPT_ATTRIB_QUICK_KILL_SCOPE:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(quick_kill_scope);
							break;

						case OPT_ATTRIB_QUICK_SELECT_SCOPE:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(quick_select_scope);
							break;

						case OPT_ATTRIB_QUOTE_CHARS:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(quote_chars);
							break;

						case OPT_ATTRIB_SAVEDIR:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(savedir);
							break;

						case OPT_ATTRIB_SAVEFILE:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(savefile);
							break;

						case OPT_ATTRIB_SIGFILE:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(sigfile);
							break;

#ifdef CHARSET_CONVERSION
						case OPT_ATTRIB_UNDECLARED_CHARSET:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(undeclared_charset);
							break;

#endif /* CHARSET_CONVERSION */
						case OPT_ATTRIB_X_BODY:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(x_body);
							break;

						case OPT_ATTRIB_X_HEADERS:
							if (prompt_option_string(option))
								SET_STRING_ATTRIBUTE(x_headers);
							break;

						default:
							break;
					} /* switch (option) */

					break;

				case OPT_NUM:
					switch (option) {
						case OPT_GETART_LIMIT:
						case OPT_SCROLL_LINES:
							if (prompt_option_num(option))
								changed |= MISC_OPTS;
							break;

#if defined(HAVE_ALARM) && defined(SIGALRM)
						case OPT_NNTP_READ_TIMEOUT_SECS:
							if (prompt_option_num(option)) {
								if (tinrc.nntp_read_timeout_secs < 0)
									tinrc.nntp_read_timeout_secs = 0;
								changed |= MISC_OPTS;
							}
							break;
#endif /* HAVE_ALARM && SIGALRM */

						case OPT_REREAD_ACTIVE_FILE_SECS:
							if (prompt_option_num(option)) {
								if (tinrc.reread_active_file_secs < 0)
									tinrc.reread_active_file_secs = 0;
								changed |= MISC_OPTS;
							}
							break;

						case OPT_RECENT_TIME:
							if (prompt_option_num(option)) {
								if (tinrc.recent_time < 0)
									tinrc.recent_time = 0;
								changed |= MISC_OPTS;
							}
							break;

						case OPT_GROUPNAME_MAX_LENGTH:
							if (prompt_option_num(option)) {
								if (tinrc.groupname_max_length < 0)
									tinrc.groupname_max_length = 0;
								changed |= MISC_OPTS;
							}
							break;

						case OPT_FILTER_DAYS:
							if (prompt_option_num(option)) {
								if (tinrc.filter_days <= 0)
									tinrc.filter_days = 1;
								changed |= MISC_OPTS;
							}
							break;

						case OPT_SCORE_LIMIT_KILL:
						case OPT_SCORE_KILL:
						case OPT_SCORE_LIMIT_SELECT:
						case OPT_SCORE_SELECT:
							if (prompt_option_num(option)) {
								check_score_defaults();
								redraw_screen(option);
								changed |= SCORE_OPTS;
							}
							break;

						case OPT_THREAD_PERC:
							if (prompt_option_num(option)) {
								if (tinrc.thread_perc < 0 || tinrc.thread_perc > 100)
									tinrc.thread_perc = THREAD_PERC_DEFAULT;
								UPDATE_INT_ATTRIBUTES(thread_perc);
							}
							break;

						case OPT_WRAP_COLUMN:
							if (prompt_option_num(option))
								changed |= DISPLAY_OPTS;
							break;

						case OPT_ATTRIB_THREAD_PERC:
							if (prompt_option_num(option))
								SET_NUM_ATTRIBUTE(thread_perc);
							break;

						default:
							break;
					} /* switch (option) */
					break;

				case OPT_CHAR:
					switch (option) {
						/*
						 * TODO: do DASH_TO_SPACE/SPACE_TO_DASH conversion here?
						 */
						case OPT_ART_MARKED_DELETED:
						case OPT_ART_MARKED_INRANGE:
						case OPT_ART_MARKED_RETURN:
						case OPT_ART_MARKED_SELECTED:
						case OPT_ART_MARKED_RECENT:
						case OPT_ART_MARKED_UNREAD:
						case OPT_ART_MARKED_READ:
						case OPT_ART_MARKED_KILLED:
						case OPT_ART_MARKED_READ_SELECTED:
							if (prompt_option_char(option))
								changed |= MISC_OPTS;
							break;

						default:
							break;
					} /* switch (option) */
					break;

				default:
					break;
			} /* switch (option_table[option].var_type) */
			change_option = FALSE;
			show_menu_help(txt_select_config_file_option);
			repaint_option(option);
			highlight_option(option);
		} /* if (change_option) */
	} /* forever */
	/* NOTREACHED */
	return;
}


/*
 * scopes and attributes menu
 */

static t_function
scope_left(
	void)
{
	return GLOBAL_QUIT;
}


static t_function
scope_right(
	void)
{
	return SCOPE_SELECT;
}


static void
show_scope_page(
	void)
{
	int i;

	signal_context = cScope;
	currmenu = &scopemenu;
	mark_offset = 0;

	if (scopemenu.curr < 0)
		scopemenu.curr = 0;

	scopemenu.max = num_scope - 1;

	ClearScreen();
	set_first_screen_item();
	center_line(0, TRUE, _(txt_scopes_menu));

	for (i = scopemenu.first; i < scopemenu.first + NOTESLINES && i < scopemenu.max; ++i)
		build_scope_line(i);

	show_mini_help(SCOPE_LEVEL);

	if (scopemenu.max <= 0) {
		info_message(_(txt_no_scopes));
		return;
	}

	draw_scope_arrow();
}


static void
scope_page(
	enum context level)
{
	char key[MAXKEYLEN];
	int i;
	t_bool changed = FALSE;
	t_function func;
	t_menu *oldmenu = NULL;

	if (currmenu)
		oldmenu = currmenu;
	scopemenu.curr = 0;
	clear_note_area();
	show_scope_page();
	set_xclick_off();

	forever {
		switch ((func = handle_keypad(scope_left, scope_right, NULL, scope_keys))) {
			case GLOBAL_QUIT:
				if (changed)
					write_attributes_file(local_attributes_file);
				clear_note_area();
				if (oldmenu)
					currmenu = oldmenu;
				return;

			case DIGIT_1:
			case DIGIT_2:
			case DIGIT_3:
			case DIGIT_4:
			case DIGIT_5:
			case DIGIT_6:
			case DIGIT_7:
			case DIGIT_8:
			case DIGIT_9:
				if (scopemenu.max)
					prompt_item_num(func_to_key(func, scope_keys), _(txt_scope_select));
				break;

#ifndef NO_SHELL_ESCAPE
			case GLOBAL_SHELL_ESCAPE:
				do_shell_escape();
				break;
#endif /* !NO_SHELL_ESCAPE */

			case GLOBAL_HELP:
				show_help_page(SCOPE_LEVEL, _(txt_scopes_menu_com));
				show_scope_page();
				break;

			case GLOBAL_FIRST_PAGE:
				top_of_list();
				break;

			case GLOBAL_LAST_PAGE:
				end_of_list();
				break;

			case GLOBAL_REDRAW_SCREEN:
				my_retouch();
				set_xclick_off();
				show_scope_page();
				break;

			case GLOBAL_LINE_DOWN:
				move_down();
				break;

			case GLOBAL_LINE_UP:
				move_up();
				break;

			case GLOBAL_PAGE_DOWN:
				page_down();
				break;

			case GLOBAL_PAGE_UP:
				page_up();
				break;

			case GLOBAL_SCROLL_DOWN:
				scroll_down();
				break;

			case GLOBAL_SCROLL_UP:
				scroll_up();
				break;

			case GLOBAL_TOGGLE_HELP_DISPLAY:
				toggle_mini_help(SCOPE_LEVEL);
				show_scope_page();
				break;

			case SCOPE_ADD:
				if ((i = add_new_scope())) {
					changed = TRUE;
					scopemenu.curr = i;
					show_scope_page();
				}
				break;

			case SCOPE_DELETE:
				if (scopemenu.max) {
					if (scopes[scopemenu.curr + 1].global)
						info_message(_(txt_scope_operation_not_allowed));
					else if (delete_scope(scopemenu.curr + 1)) {
						changed = TRUE;
						show_scope_page();
					}
				}
				break;

			case SCOPE_EDIT_ATTRIBUTES_FILE:
				if (changed)
					write_attributes_file(local_attributes_file);
				if (!invoke_editor(local_attributes_file, attrib_file_offset, NULL))
					break;
				free_scopes_and_attributes();
				read_attributes_file(FALSE);
				assign_attributes_to_groups();
				changed = FALSE;
				scopemenu.curr = 0;
				show_scope_page();
				break;

			case SCOPE_MOVE:
				if (scopemenu.max > 1) {
					if (scopes[scopemenu.curr + 1].global)
						info_message(_(txt_scope_operation_not_allowed));
					else if ((i = move_scope(scopemenu.curr + 1))) {
						changed = TRUE;
						scopemenu.curr = i - 1;
						show_scope_page();
					}
				}
				break;

			case SCOPE_RENAME:
				if (scopemenu.max) {
					if (scopes[scopemenu.curr + 1].global)
						info_message(_(txt_scope_operation_not_allowed));
					else if (rename_scope(&scopes[scopemenu.curr + 1])) {
						changed = TRUE;
						show_scope_page();
					}
				}
				break;

			case SCOPE_SELECT:
				if (scopemenu.max) {
					curr_scope = &scopes[scopemenu.curr + 1];
					config_page(NULL, level);
					if (!curr_scope->global && scope_is_empty())
						do_delete_scope(scopemenu.curr + 1);
					curr_scope = NULL;
					changed = TRUE;
					show_scope_page();
				}
				break;

			default:
				info_message(_(txt_bad_command), printascii(key, func_to_key(GLOBAL_HELP, scope_keys)));
				break;
		}
	}
}


static void
draw_scope_arrow(
	void)
{
	draw_arrow_mark(INDEX_TOP + scopemenu.curr - scopemenu.first);
	if (scopemenu.curr == scopemenu.max - 1)
		info_message(_(txt_end_of_scopes));
}


static void
build_scope_line(
	int i)
{
	char *sptr;
	int len = cCOLS - 11;

#ifdef USE_CURSES
	/*
	 * Allocate line buffer
	 * make it the same size like in !USE_CURSES case to simplify some code
	 */
	sptr = my_malloc(cCOLS + 2);
#else
	sptr = screen[INDEX2SNUM(i)].col;
#endif /* USE_CURSES */

	snprintf(sptr, cCOLS, "  %c %s  %-*.*s%s", (scopes[i + 1].global ? '!' : ' '), tin_ltoa(i + 1, 4), len, len, scopes[i + 1].scope, cCRLF);
	WriteLine(INDEX2LNUM(i), sptr);

#ifdef USE_CURSES
	free(sptr);
#endif /* USE_CURSES */
}


/*
 * add a new scope and return the index
 */
static int
add_new_scope(
	void)
{
	char buf[LEN];
	int new_pos = 0;

	if (prompt_default_string(_(txt_scope_enter), buf, sizeof(buf), (char *) NULL, HIST_OTHER))
		new_pos = add_scope(buf);

	return new_pos;
}


/*
 * returns TRUE if the given scope was deleted
 */
static t_bool
delete_scope(
	int curr_pos)
{
	if (prompt_yn(_(txt_scope_delete), FALSE) == 1) {
		do_delete_scope(curr_pos);
		return TRUE;
	}

	return FALSE;
}


static void
do_delete_scope(
	int curr_pos)
{
	do_move_scope(curr_pos, num_scope - 1);
	free_scope(--num_scope);
}


/*
 * returns TRUE if scope was renamed
 */
static t_bool
rename_scope(
	struct t_scope *scope)
{
	char buf[LEN];

	if (prompt_default_string(_(txt_scope_rename), buf, sizeof(buf), scope->scope, HIST_OTHER)) {
		if (buf[0] == '\0')
			return FALSE;
		FreeIfNeeded(scope->scope);
		scope->scope = my_strdup(buf);
		return TRUE;
	}

	return FALSE;
}


/*
 * look if an entry with the given scope exists and return the index
 */
static int
find_scope(
	const char *scope)
{
	int i;

	if (!scope || !*scope)
		return 0;

	for (i = 1; i < num_scope; i++) {
		if ((!scopes[i].global && strcasecmp(scope, scopes[i].scope) == 0))
			return i;
	}

	return 0;
}


/*
 * returns the new position of the moved scope or 0 if repositioning
 * is not possible
 */
static int
move_scope(
	int curr_pos)
{
	char *p;
	int new_pos;

	clear_message();
	if ((p = tin_getline(_(txt_scope_new_position), 1, 0, 0, FALSE, HIST_OTHER)) != NULL)
		new_pos = atoi(p);
	else
		new_pos = curr_pos;
	clear_message();

	if (new_pos == curr_pos || new_pos == 0)
		return 0;

	if (new_pos >= num_scope)
		new_pos = num_scope - 1;

	if (scopes[new_pos].global) {
		info_message(_(txt_scope_new_position_is_global));
		return 0;
	}

	do_move_scope(curr_pos, new_pos);

	return new_pos;
}


/*
 * repositions a scope into scopes[]
 */
static void
do_move_scope(
	int from,
	int to)
{
	struct t_scope tmp;

	if (from == to)
		return;

	tmp = scopes[from];

	if (from > to) {
		while (from-- > to)
			scopes[from + 1] = scopes[from];
	} else {
		while (from++ < to)
			scopes[from - 1] = scopes[from];
	}
	scopes[to] = tmp;
}


/*
 * free all group->attribute arrays and all scopes which are
 * not marked as global
 */
static void
free_scopes_and_attributes(
	void)
{
	int i;

	for_each_group(i) {
		if (active[i].attribute && !active[i].attribute->global) {
			free(active[i].attribute);
			active[i].attribute = (struct t_attribute *) 0;
		}
	}

	while (num_scope > 1 && !scopes[num_scope - 1].global)
			free_scope(--num_scope);
}


/*
 * returns TRUE if no attribute in curr_scope has state == TRUE
 */
static t_bool
scope_is_empty(
	void)
{
	enum option_enum i;

	for (i = 0; i <= last_opt; i++) {
		if (option_is_visible(i) && !option_is_title(i) && check_state(i))
			return FALSE;
	}

	return TRUE;
}


/*
 * returns the state of the given attribute
 */
static t_bool
check_state(
	enum option_enum option)
{
	switch (option) {
		case OPT_ATTRIB_ADD_POSTED_TO_FILTER:
			return curr_scope->state->add_posted_to_filter;
		case OPT_ATTRIB_ADVERTISING:
			return curr_scope->state->advertising;
		case OPT_ATTRIB_ALTERNATIVE_HANDLING:
			return curr_scope->state->alternative_handling;
		case OPT_ATTRIB_ASK_FOR_METAMAIL:
			return curr_scope->state->ask_for_metamail;
		case OPT_ATTRIB_AUTO_CC_BCC:
			return curr_scope->state->auto_cc_bcc;
		case OPT_ATTRIB_AUTO_LIST_THREAD:
			return curr_scope->state->auto_list_thread;
		case OPT_ATTRIB_AUTO_SAVE:
			return curr_scope->state->auto_save;
		case OPT_ATTRIB_AUTO_SELECT:
			return curr_scope->state->auto_select;
		case OPT_ATTRIB_BATCH_SAVE:
			return curr_scope->state->batch_save;
		case OPT_ATTRIB_DATE_FORMAT:
			return curr_scope->state->date_format;
		case OPT_ATTRIB_DELETE_TMP_FILES:
			return curr_scope->state->delete_tmp_files;
		case OPT_ATTRIB_EDITOR_FORMAT:
			return curr_scope->state->editor_format;
		case OPT_ATTRIB_FCC:
			return curr_scope->state->fcc;
		case OPT_ATTRIB_FOLLOWUP_TO:
			return curr_scope->state->followup_to;
		case OPT_ATTRIB_FROM:
			return curr_scope->state->from;
		case OPT_ATTRIB_GROUP_CATCHUP_ON_EXIT:
			return curr_scope->state->group_catchup_on_exit;
		case OPT_ATTRIB_GROUP_FORMAT:
			return curr_scope->state->group_format;
#ifdef HAVE_ISPELL
		case OPT_ATTRIB_ISPELL:
			return curr_scope->state->ispell;
#endif /* HAVE_ISPELL */
		case OPT_ATTRIB_MAILDIR:
			return curr_scope->state->maildir;
		case OPT_ATTRIB_MAIL_8BIT_HEADER:
			return curr_scope->state->mail_8bit_header;
		case OPT_ATTRIB_MAIL_MIME_ENCODING:
			return curr_scope->state->mail_mime_encoding;
		case OPT_ATTRIB_MAILING_LIST:
			return curr_scope->state->mailing_list;
		case OPT_ATTRIB_MARK_IGNORE_TAGS:
			return curr_scope->state->mark_ignore_tags;
		case OPT_ATTRIB_MARK_SAVED_READ:
			return curr_scope->state->mark_saved_read;
		case OPT_ATTRIB_MIME_FORWARD:
			return curr_scope->state->mime_forward;
		case OPT_ATTRIB_MIME_TYPES_TO_SAVE:
			return curr_scope->state->mime_types_to_save;
		case OPT_ATTRIB_NEWS_HEADERS_TO_DISPLAY:
			return curr_scope->state->news_headers_to_display;
		case OPT_ATTRIB_NEWS_HEADERS_TO_NOT_DISPLAY:
			return curr_scope->state->news_headers_to_not_display;
		case OPT_ATTRIB_NEWS_QUOTE_FORMAT:
			return curr_scope->state->news_quote_format;
		case OPT_ATTRIB_ORGANIZATION:
			return curr_scope->state->organization;
		case OPT_ATTRIB_POST_8BIT_HEADER:
			return curr_scope->state->post_8bit_header;
		case OPT_ATTRIB_POST_MIME_ENCODING:
			return curr_scope->state->post_mime_encoding;
		case OPT_ATTRIB_POST_PROCESS_VIEW:
			return curr_scope->state->post_process_view;
		case OPT_ATTRIB_POS_FIRST_UNREAD:
			return curr_scope->state->pos_first_unread;
#ifndef DISABLE_PRINTING
		case OPT_ATTRIB_PRINT_HEADER:
			return curr_scope->state->print_header;
#endif /* !DISABLE_PRINTING */
		case OPT_ATTRIB_PROCESS_ONLY_UNREAD:
			return curr_scope->state->process_only_unread;
		case OPT_ATTRIB_PROMPT_FOLLOWUPTO:
			return curr_scope->state->prompt_followupto;
		case OPT_ATTRIB_QUICK_KILL_SCOPE:
			return curr_scope->state->quick_kill_scope;
		case OPT_ATTRIB_QUICK_KILL_HEADER:
			return curr_scope->state->quick_kill_header;
		case OPT_ATTRIB_QUICK_KILL_CASE:
			return curr_scope->state->quick_kill_case;
		case OPT_ATTRIB_QUICK_KILL_EXPIRE:
			return curr_scope->state->quick_kill_expire;
		case OPT_ATTRIB_QUICK_SELECT_SCOPE:
			return curr_scope->state->quick_select_scope;
		case OPT_ATTRIB_QUICK_SELECT_HEADER:
			return curr_scope->state->quick_select_header;
		case OPT_ATTRIB_QUICK_SELECT_CASE:
			return curr_scope->state->quick_select_case;
		case OPT_ATTRIB_QUICK_SELECT_EXPIRE:
			return curr_scope->state->quick_select_expire;
		case OPT_ATTRIB_QUOTE_CHARS:
			return curr_scope->state->quote_chars;
		case OPT_ATTRIB_SAVEDIR:
			return curr_scope->state->savedir;
		case OPT_ATTRIB_SAVEFILE:
			return curr_scope->state->savefile;
		case OPT_ATTRIB_SHOW_AUTHOR:
			return curr_scope->state->show_author;
		case OPT_ATTRIB_SHOW_ONLY_UNREAD_ARTS:
			return curr_scope->state->show_only_unread_arts;
		case OPT_ATTRIB_SHOW_SIGNATURES:
			return curr_scope->state->show_signatures;
		case OPT_ATTRIB_SIGDASHES:
			return curr_scope->state->sigdashes;
		case OPT_ATTRIB_SIGFILE:
			return curr_scope->state->sigfile;
		case OPT_ATTRIB_SIGNATURE_REPOST:
			return curr_scope->state->signature_repost;
		case OPT_ATTRIB_START_EDITOR_OFFSET:
			return curr_scope->state->start_editor_offset;
		case OPT_ATTRIB_THREAD_ARTICLES:
			return curr_scope->state->thread_articles;
		case OPT_ATTRIB_THREAD_CATCHUP_ON_EXIT:
			return curr_scope->state->thread_catchup_on_exit;
		case OPT_ATTRIB_THREAD_FORMAT:
			return curr_scope->state->thread_format;
		case OPT_ATTRIB_THREAD_PERC:
			return curr_scope->state->thread_perc;
		case OPT_ATTRIB_TRIM_ARTICLE_BODY:
			return curr_scope->state->trim_article_body;
		case OPT_ATTRIB_TEX2ISO_CONV:
			return curr_scope->state->tex2iso_conv;
		case OPT_ATTRIB_SORT_THREADS_TYPE:
			return curr_scope->state->sort_threads_type;
#ifdef CHARSET_CONVERSION
		case OPT_ATTRIB_MM_NETWORK_CHARSET:
			return curr_scope->state->mm_network_charset;
		case OPT_ATTRIB_UNDECLARED_CHARSET:
			return curr_scope->state->undeclared_charset;
#endif /* CHARSET_CONVERSION */
		case OPT_ATTRIB_VERBATIM_HANDLING:
			return curr_scope->state->verbatim_handling;
		case OPT_ATTRIB_WRAP_ON_NEXT_UNREAD:
			return curr_scope->state->wrap_on_next_unread;
		case OPT_ATTRIB_SORT_ARTICLE_TYPE:
			return curr_scope->state->sort_article_type;
		case OPT_ATTRIB_POST_PROCESS_TYPE:
			return curr_scope->state->post_process_type;
		case OPT_ATTRIB_X_BODY:
			return curr_scope->state->x_body;
		case OPT_ATTRIB_X_COMMENT_TO:
			return curr_scope->state->x_comment_to;
		case OPT_ATTRIB_X_HEADERS:
			return curr_scope->state->x_headers;

		default:
			return FALSE;
	}
}


/*
 * set the state of the given attribute to FALSE and the corresponding
 * tinrc.attrib_* to a default value
 */
static void
reset_state(
	enum option_enum option)
{
	struct t_scope *default_scope = &scopes[0];

	switch (option) {
		case OPT_ATTRIB_ADD_POSTED_TO_FILTER:
			curr_scope->state->add_posted_to_filter = FALSE;
			tinrc.attrib_add_posted_to_filter = default_scope->attribute->add_posted_to_filter;
			break;
		case OPT_ATTRIB_ADVERTISING:
			curr_scope->state->advertising = FALSE;
			tinrc.attrib_advertising = default_scope->attribute->advertising;
			break;
		case OPT_ATTRIB_ALTERNATIVE_HANDLING:
			curr_scope->state->alternative_handling = FALSE;
			tinrc.attrib_alternative_handling = default_scope->attribute->alternative_handling;
			break;
		case OPT_ATTRIB_ASK_FOR_METAMAIL:
			curr_scope->state->ask_for_metamail = FALSE;
			tinrc.attrib_ask_for_metamail = default_scope->attribute->ask_for_metamail;
			break;
		case OPT_ATTRIB_AUTO_CC_BCC:
			curr_scope->state->auto_cc_bcc = FALSE;
			tinrc.attrib_auto_cc_bcc = default_scope->attribute->auto_cc_bcc;
			break;
		case OPT_ATTRIB_AUTO_LIST_THREAD:
			curr_scope->state->auto_list_thread = FALSE;
			tinrc.attrib_auto_list_thread = default_scope->attribute->auto_list_thread;
			break;
		case OPT_ATTRIB_AUTO_SAVE:
			curr_scope->state->auto_save = FALSE;
			tinrc.attrib_auto_save = default_scope->attribute->auto_save;
			break;
		case OPT_ATTRIB_AUTO_SELECT:
			curr_scope->state->auto_select = FALSE;
			tinrc.attrib_auto_select = default_scope->attribute->auto_select;
			break;
		case OPT_ATTRIB_BATCH_SAVE:
			curr_scope->state->batch_save = FALSE;
			tinrc.attrib_batch_save = default_scope->attribute->batch_save;
			break;
		case OPT_ATTRIB_DATE_FORMAT:
			FreeAndNull(curr_scope->attribute->date_format);
			curr_scope->state->date_format = FALSE;
			snprintf(tinrc.attrib_date_format, sizeof(tinrc.attrib_date_format), "%s", BlankIfNull(default_scope->attribute->date_format));
			break;
		case OPT_ATTRIB_DELETE_TMP_FILES:
			curr_scope->state->delete_tmp_files = FALSE;
			tinrc.attrib_delete_tmp_files = default_scope->attribute->delete_tmp_files;
			break;
		case OPT_ATTRIB_EDITOR_FORMAT:
			FreeAndNull(curr_scope->attribute->editor_format);
			curr_scope->state->editor_format = FALSE;
			snprintf(tinrc.attrib_editor_format, sizeof(tinrc.attrib_editor_format), "%s", BlankIfNull(default_scope->attribute->editor_format));
			break;
		case OPT_ATTRIB_FCC:
			FreeAndNull(curr_scope->attribute->fcc);
			curr_scope->state->fcc = FALSE;
			snprintf(tinrc.attrib_fcc, sizeof(tinrc.attrib_fcc), "%s", BlankIfNull(default_scope->attribute->fcc));
			break;
		case OPT_ATTRIB_FOLLOWUP_TO:
			FreeAndNull(curr_scope->attribute->followup_to);
			curr_scope->state->followup_to = FALSE;
			snprintf(tinrc.attrib_followup_to, sizeof(tinrc.attrib_followup_to), "%s", BlankIfNull(default_scope->attribute->followup_to));
			break;
		case OPT_ATTRIB_FROM:
			FreeAndNull(curr_scope->attribute->from);
			curr_scope->state->from = FALSE;
			snprintf(tinrc.attrib_from, sizeof(tinrc.attrib_from), "%s", BlankIfNull(default_scope->attribute->from));
			break;
		case OPT_ATTRIB_GROUP_CATCHUP_ON_EXIT:
			curr_scope->state->group_catchup_on_exit = FALSE;
			tinrc.attrib_group_catchup_on_exit = default_scope->attribute->group_catchup_on_exit;
			break;
		case OPT_ATTRIB_GROUP_FORMAT:
			FreeAndNull(curr_scope->attribute->group_format);
			curr_scope->state->group_format = FALSE;
			snprintf(tinrc.attrib_group_format, sizeof(tinrc.attrib_group_format), "%s", BlankIfNull(default_scope->attribute->group_format));
			break;
#ifdef HAVE_ISPELL
		case OPT_ATTRIB_ISPELL:
			FreeAndNull(curr_scope->attribute->ispell);
			curr_scope->state->ispell = FALSE;
			snprintf(tinrc.attrib_ispell, sizeof(tinrc.attrib_ispell), "%s", BlankIfNull(default_scope->attribute->ispell));
			break;
#endif /* HAVE_ISPELL */
		case OPT_ATTRIB_MAILDIR:
			FreeAndNull(curr_scope->attribute->maildir);
			curr_scope->state->maildir = FALSE;
			snprintf(tinrc.attrib_maildir, sizeof(tinrc.attrib_maildir), "%s", BlankIfNull(default_scope->attribute->maildir));
			break;
		case OPT_ATTRIB_MAIL_8BIT_HEADER:
			curr_scope->state->mail_8bit_header = FALSE;
			tinrc.attrib_mail_8bit_header = default_scope->attribute->mail_8bit_header;
			break;
		case OPT_ATTRIB_MAIL_MIME_ENCODING:
			curr_scope->state->mail_mime_encoding = FALSE;
			tinrc.attrib_mail_mime_encoding = default_scope->attribute->mail_mime_encoding;
			break;
		case OPT_ATTRIB_MAILING_LIST:
			FreeAndNull(curr_scope->attribute->mailing_list);
			curr_scope->state->mailing_list = FALSE;
			snprintf(tinrc.attrib_mailing_list, sizeof(tinrc.attrib_mailing_list), "%s", BlankIfNull(default_scope->attribute->mailing_list));
			break;
		case OPT_ATTRIB_MARK_IGNORE_TAGS:
			curr_scope->state->mark_ignore_tags = FALSE;
			tinrc.attrib_mark_ignore_tags = default_scope->attribute->mark_ignore_tags;
			break;
		case OPT_ATTRIB_MARK_SAVED_READ:
			curr_scope->state->mark_saved_read = FALSE;
			tinrc.attrib_mark_saved_read = default_scope->attribute->mark_saved_read;
			break;
		case OPT_ATTRIB_MIME_FORWARD:
			curr_scope->state->mime_forward = FALSE;
			tinrc.attrib_mime_forward = default_scope->attribute->mime_forward;
			break;
		case OPT_ATTRIB_MIME_TYPES_TO_SAVE:
			FreeAndNull(curr_scope->attribute->mime_types_to_save);
			curr_scope->state->mime_types_to_save = FALSE;
			snprintf(tinrc.attrib_mime_types_to_save, sizeof(tinrc.attrib_mime_types_to_save), "%s", BlankIfNull(default_scope->attribute->mime_types_to_save));
			break;
		case OPT_ATTRIB_NEWS_HEADERS_TO_DISPLAY:
			FreeAndNull(curr_scope->attribute->news_headers_to_display);
			build_news_headers_array(curr_scope->attribute, TRUE);
			curr_scope->state->news_headers_to_display = FALSE;
			snprintf(tinrc.attrib_news_headers_to_display, sizeof(tinrc.attrib_news_headers_to_display), "%s", BlankIfNull(default_scope->attribute->news_headers_to_display));
			break;
		case OPT_ATTRIB_NEWS_HEADERS_TO_NOT_DISPLAY:
			FreeAndNull(curr_scope->attribute->news_headers_to_not_display);
			build_news_headers_array(curr_scope->attribute, FALSE);
			curr_scope->state->news_headers_to_not_display = FALSE;
			snprintf(tinrc.attrib_news_headers_to_not_display, sizeof(tinrc.attrib_news_headers_to_not_display), "%s", BlankIfNull(default_scope->attribute->news_headers_to_not_display));
			break;
		case OPT_ATTRIB_QUICK_KILL_SCOPE:
			FreeAndNull(curr_scope->attribute->quick_kill_scope);
			curr_scope->state->quick_kill_scope = FALSE;
			snprintf(tinrc.attrib_quick_kill_scope, sizeof(tinrc.attrib_quick_kill_scope), "%s", BlankIfNull(default_scope->attribute->quick_kill_scope));
			break;
		case OPT_ATTRIB_QUICK_KILL_HEADER:
			curr_scope->state->quick_kill_header = FALSE;
			tinrc.attrib_quick_kill_header = default_scope->attribute->quick_kill_header;
			break;
		case OPT_ATTRIB_QUICK_KILL_CASE:
			curr_scope->state->quick_kill_case = FALSE;
			tinrc.attrib_quick_kill_case = default_scope->attribute->quick_kill_case;
			break;
		case OPT_ATTRIB_QUICK_KILL_EXPIRE:
			curr_scope->state->quick_kill_expire = FALSE;
			tinrc.attrib_quick_kill_expire = default_scope->attribute->quick_kill_expire;
			break;
		case OPT_ATTRIB_QUICK_SELECT_SCOPE:
			FreeAndNull(curr_scope->attribute->quick_select_scope);
			curr_scope->state->quick_select_scope = FALSE;
			snprintf(tinrc.attrib_quick_select_scope, sizeof(tinrc.attrib_quick_select_scope), "%s", BlankIfNull(default_scope->attribute->quick_select_scope));
			break;
		case OPT_ATTRIB_QUICK_SELECT_HEADER:
			curr_scope->state->quick_select_header = FALSE;
			tinrc.attrib_quick_select_header = default_scope->attribute->quick_select_header;
			break;
		case OPT_ATTRIB_QUICK_SELECT_CASE:
			curr_scope->state->quick_select_case = FALSE;
			tinrc.attrib_quick_select_case = default_scope->attribute->quick_select_case;
			break;
		case OPT_ATTRIB_QUICK_SELECT_EXPIRE:
			curr_scope->state->quick_select_expire = FALSE;
			tinrc.attrib_quick_select_expire = default_scope->attribute->quick_select_expire;
			break;
		case OPT_ATTRIB_NEWS_QUOTE_FORMAT:
			FreeAndNull(curr_scope->attribute->news_quote_format);
			curr_scope->state->news_quote_format = FALSE;
			snprintf(tinrc.attrib_news_quote_format, sizeof(tinrc.attrib_news_quote_format), "%s", BlankIfNull(default_scope->attribute->news_quote_format));
			break;
		case OPT_ATTRIB_ORGANIZATION:
			FreeAndNull(curr_scope->attribute->organization);
			curr_scope->state->organization = FALSE;
			snprintf(tinrc.attrib_organization, sizeof(tinrc.attrib_organization), "%s", BlankIfNull(default_scope->attribute->organization));
			break;
		case OPT_ATTRIB_POST_8BIT_HEADER:
			curr_scope->state->post_8bit_header = FALSE;
			tinrc.attrib_post_8bit_header = default_scope->attribute->post_8bit_header;
			break;
		case OPT_ATTRIB_POST_MIME_ENCODING:
			curr_scope->state->post_mime_encoding = FALSE;
			tinrc.attrib_post_mime_encoding = default_scope->attribute->post_mime_encoding;
			break;
		case OPT_ATTRIB_POST_PROCESS_VIEW:
			curr_scope->state->post_process_view = FALSE;
			tinrc.attrib_post_process_view = default_scope->attribute->post_process_view;
			break;
		case OPT_ATTRIB_POS_FIRST_UNREAD:
			curr_scope->state->pos_first_unread = FALSE;
			tinrc.attrib_pos_first_unread = default_scope->attribute->pos_first_unread;
			break;
#ifndef DISABLE_PRINTING
		case OPT_ATTRIB_PRINT_HEADER:
			curr_scope->state->print_header = FALSE;
			tinrc.attrib_print_header = default_scope->attribute->print_header;
			break;
#endif /* !DISABLE_PRINTING */
		case OPT_ATTRIB_PROCESS_ONLY_UNREAD:
			curr_scope->state->process_only_unread = FALSE;
			tinrc.attrib_process_only_unread = default_scope->attribute->process_only_unread;
			break;
		case OPT_ATTRIB_PROMPT_FOLLOWUPTO:
			curr_scope->state->prompt_followupto = FALSE;
			tinrc.attrib_prompt_followupto = default_scope->attribute->prompt_followupto;
			break;
		case OPT_ATTRIB_QUOTE_CHARS:
			FreeAndNull(curr_scope->attribute->quote_chars);
			curr_scope->state->quote_chars = FALSE;
			snprintf(tinrc.attrib_quote_chars, sizeof(tinrc.attrib_quote_chars), "%s", BlankIfNull(default_scope->attribute->quote_chars));
			break;
		case OPT_ATTRIB_SAVEDIR:
			FreeAndNull(curr_scope->attribute->savedir);
			curr_scope->state->savedir = FALSE;
			snprintf(tinrc.attrib_savedir, sizeof(tinrc.attrib_savedir), "%s", BlankIfNull(default_scope->attribute->savedir));
			break;
		case OPT_ATTRIB_SAVEFILE:
			FreeAndNull(curr_scope->attribute->savefile);
			curr_scope->state->savefile = FALSE;
			snprintf(tinrc.attrib_savefile, sizeof(tinrc.attrib_savefile), "%s", BlankIfNull(default_scope->attribute->savefile));
			break;
		case OPT_ATTRIB_SHOW_AUTHOR:
			curr_scope->state->show_author = FALSE;
			tinrc.attrib_show_author = default_scope->attribute->show_author;
			break;
		case OPT_ATTRIB_SHOW_ONLY_UNREAD_ARTS:
			curr_scope->state->show_only_unread_arts = FALSE;
			tinrc.attrib_show_only_unread_arts = default_scope->attribute->show_only_unread_arts;
			break;
		case OPT_ATTRIB_SHOW_SIGNATURES:
			curr_scope->state->show_signatures = FALSE;
			tinrc.attrib_show_signatures = default_scope->attribute->show_signatures;
			break;
		case OPT_ATTRIB_SIGDASHES:
			curr_scope->state->sigdashes = FALSE;
			tinrc.attrib_sigdashes = default_scope->attribute->sigdashes;
			break;
		case OPT_ATTRIB_SIGFILE:
			FreeAndNull(curr_scope->attribute->sigfile);
			curr_scope->state->sigfile = FALSE;
			snprintf(tinrc.attrib_sigfile, sizeof(tinrc.attrib_sigfile), "%s", BlankIfNull(default_scope->attribute->sigfile));
			break;
		case OPT_ATTRIB_SIGNATURE_REPOST:
			curr_scope->state->signature_repost = FALSE;
			tinrc.attrib_signature_repost = default_scope->attribute->signature_repost;
			break;
		case OPT_ATTRIB_START_EDITOR_OFFSET:
			curr_scope->state->start_editor_offset = FALSE;
			tinrc.attrib_start_editor_offset = default_scope->attribute->start_editor_offset;
			break;
		case OPT_ATTRIB_THREAD_ARTICLES:
			curr_scope->state->thread_articles = FALSE;
			tinrc.attrib_thread_articles = default_scope->attribute->thread_articles;
			break;
		case OPT_ATTRIB_THREAD_CATCHUP_ON_EXIT:
			curr_scope->state->thread_catchup_on_exit = FALSE;
			tinrc.attrib_thread_catchup_on_exit = default_scope->attribute->thread_catchup_on_exit;
			break;
		case OPT_ATTRIB_THREAD_FORMAT:
			FreeAndNull(curr_scope->attribute->thread_format);
			curr_scope->state->thread_format = FALSE;
			snprintf(tinrc.attrib_thread_format, sizeof(tinrc.attrib_thread_format), "%s", BlankIfNull(default_scope->attribute->thread_format));
			break;
		case OPT_ATTRIB_THREAD_PERC:
			curr_scope->state->thread_perc = FALSE;
			tinrc.attrib_thread_perc = default_scope->attribute->thread_perc;
			break;
		case OPT_ATTRIB_TRIM_ARTICLE_BODY:
			curr_scope->state->trim_article_body = FALSE;
			tinrc.attrib_trim_article_body = default_scope->attribute->trim_article_body;
			break;
		case OPT_ATTRIB_TEX2ISO_CONV:
			curr_scope->state->tex2iso_conv = FALSE;
			tinrc.attrib_tex2iso_conv = default_scope->attribute->tex2iso_conv;
			break;
		case OPT_ATTRIB_SORT_THREADS_TYPE:
			curr_scope->state->sort_threads_type = FALSE;
			tinrc.attrib_sort_threads_type = default_scope->attribute->sort_threads_type;
			break;
#ifdef CHARSET_CONVERSION
		case OPT_ATTRIB_MM_NETWORK_CHARSET:
			curr_scope->state->mm_network_charset = FALSE;
			tinrc.attrib_mm_network_charset = default_scope->attribute->mm_network_charset;
			break;
		case OPT_ATTRIB_UNDECLARED_CHARSET:
			FreeAndNull(curr_scope->attribute->undeclared_charset);
			curr_scope->state->undeclared_charset = FALSE;
			snprintf(tinrc.attrib_undeclared_charset, sizeof(tinrc.attrib_undeclared_charset), "%s", BlankIfNull(default_scope->attribute->undeclared_charset));
			break;
#endif /* CHARSET_CONVERSION */
		case OPT_ATTRIB_VERBATIM_HANDLING:
			curr_scope->state->verbatim_handling = FALSE;
			tinrc.attrib_verbatim_handling = default_scope->attribute->verbatim_handling;
			break;
		case OPT_ATTRIB_WRAP_ON_NEXT_UNREAD:
			curr_scope->state->wrap_on_next_unread = FALSE;
			tinrc.attrib_wrap_on_next_unread = default_scope->attribute->wrap_on_next_unread;
			break;
		case OPT_ATTRIB_SORT_ARTICLE_TYPE:
			curr_scope->state->sort_article_type = FALSE;
			tinrc.attrib_sort_article_type = default_scope->attribute->sort_article_type;
			break;
		case OPT_ATTRIB_POST_PROCESS_TYPE:
			curr_scope->state->post_process_type = FALSE;
			tinrc.attrib_post_process_type = default_scope->attribute->post_process_type;
			break;
		case OPT_ATTRIB_X_BODY:
			FreeAndNull(curr_scope->attribute->x_body);
			curr_scope->state->x_body = FALSE;
			snprintf(tinrc.attrib_x_body, sizeof(tinrc.attrib_x_body), "%s", BlankIfNull(default_scope->attribute->x_body));
			break;
		case OPT_ATTRIB_X_COMMENT_TO:
			curr_scope->state->x_comment_to = FALSE;
			tinrc.attrib_x_comment_to = default_scope->attribute->x_comment_to;
			break;
		case OPT_ATTRIB_X_HEADERS:
			FreeAndNull(curr_scope->attribute->x_headers);
			curr_scope->state->x_headers = FALSE;
			snprintf(tinrc.attrib_x_headers, sizeof(tinrc.attrib_x_headers), "%s", BlankIfNull(default_scope->attribute->x_headers));
			break;

		default:
			break;
	}
}


#define INITIALIZE_STRING_ATTRIBUTE(option) do { \
		if (curr_scope->state->option) \
			snprintf(CAO(tinrc.attrib_, option), sizeof(CAO(tinrc.attrib_, option)), "%s", curr_scope->attribute->option); \
		else \
			snprintf(CAO(tinrc.attrib_, option), sizeof(CAO(tinrc.attrib_, option)), "%s", BlankIfNull(default_scope->attribute->option)); \
	} while (0)
#define INITIALIZE_NUM_ATTRIBUTE(option) do { \
		if (curr_scope->state->option) \
			CAO(tinrc.attrib_, option) = curr_scope->attribute->option; \
		else \
			CAO(tinrc.attrib_, option) = default_scope->attribute->option; \
	} while (0)

static void
initialize_attributes(
	void)
{
	struct t_scope *default_scope = &scopes[0];

	INITIALIZE_NUM_ATTRIBUTE(add_posted_to_filter);
	INITIALIZE_NUM_ATTRIBUTE(advertising);
	INITIALIZE_NUM_ATTRIBUTE(alternative_handling);
	INITIALIZE_NUM_ATTRIBUTE(ask_for_metamail);
	INITIALIZE_NUM_ATTRIBUTE(auto_cc_bcc);
	INITIALIZE_NUM_ATTRIBUTE(auto_list_thread);
	INITIALIZE_NUM_ATTRIBUTE(auto_save);
	INITIALIZE_NUM_ATTRIBUTE(auto_select);
	INITIALIZE_NUM_ATTRIBUTE(batch_save);
	INITIALIZE_NUM_ATTRIBUTE(delete_tmp_files);
	INITIALIZE_NUM_ATTRIBUTE(group_catchup_on_exit);
	INITIALIZE_NUM_ATTRIBUTE(mail_8bit_header);
	INITIALIZE_NUM_ATTRIBUTE(mail_mime_encoding);
	INITIALIZE_NUM_ATTRIBUTE(mark_ignore_tags);
	INITIALIZE_NUM_ATTRIBUTE(mark_saved_read);
	INITIALIZE_NUM_ATTRIBUTE(mime_forward);
	INITIALIZE_NUM_ATTRIBUTE(pos_first_unread);
	INITIALIZE_NUM_ATTRIBUTE(post_8bit_header);
	INITIALIZE_NUM_ATTRIBUTE(post_mime_encoding);
	INITIALIZE_NUM_ATTRIBUTE(post_process_view);
#ifndef DISABLE_PRINTING
	INITIALIZE_NUM_ATTRIBUTE(print_header);
#endif /* !DISABLE_PRINTING */
	INITIALIZE_NUM_ATTRIBUTE(process_only_unread);
	INITIALIZE_NUM_ATTRIBUTE(prompt_followupto);
	INITIALIZE_NUM_ATTRIBUTE(quick_kill_header);
	INITIALIZE_NUM_ATTRIBUTE(quick_kill_case);
	INITIALIZE_NUM_ATTRIBUTE(quick_kill_expire);
	INITIALIZE_NUM_ATTRIBUTE(quick_select_header);
	INITIALIZE_NUM_ATTRIBUTE(quick_select_case);
	INITIALIZE_NUM_ATTRIBUTE(quick_select_expire);
	INITIALIZE_NUM_ATTRIBUTE(show_author);
	INITIALIZE_NUM_ATTRIBUTE(show_only_unread_arts);
	INITIALIZE_NUM_ATTRIBUTE(show_signatures);
	INITIALIZE_NUM_ATTRIBUTE(sigdashes);
	INITIALIZE_NUM_ATTRIBUTE(signature_repost);
	INITIALIZE_NUM_ATTRIBUTE(start_editor_offset);
	INITIALIZE_NUM_ATTRIBUTE(thread_articles);
	INITIALIZE_NUM_ATTRIBUTE(thread_catchup_on_exit);
	INITIALIZE_NUM_ATTRIBUTE(thread_perc);
	INITIALIZE_NUM_ATTRIBUTE(trim_article_body);
	INITIALIZE_NUM_ATTRIBUTE(tex2iso_conv);
	INITIALIZE_NUM_ATTRIBUTE(verbatim_handling);
	INITIALIZE_NUM_ATTRIBUTE(wrap_on_next_unread);
	INITIALIZE_NUM_ATTRIBUTE(sort_article_type);
	INITIALIZE_NUM_ATTRIBUTE(sort_threads_type);
	INITIALIZE_NUM_ATTRIBUTE(post_process_type);
	INITIALIZE_NUM_ATTRIBUTE(x_comment_to);
	INITIALIZE_STRING_ATTRIBUTE(date_format);
	INITIALIZE_STRING_ATTRIBUTE(editor_format);
	INITIALIZE_STRING_ATTRIBUTE(fcc);
	INITIALIZE_STRING_ATTRIBUTE(followup_to);
	INITIALIZE_STRING_ATTRIBUTE(from);
	INITIALIZE_STRING_ATTRIBUTE(group_format);
#ifdef HAVE_ISPELL
	INITIALIZE_STRING_ATTRIBUTE(ispell);
#endif /* HAVE_ISPELL */
	INITIALIZE_STRING_ATTRIBUTE(maildir);
	INITIALIZE_STRING_ATTRIBUTE(mailing_list);
	INITIALIZE_STRING_ATTRIBUTE(mime_types_to_save);
	INITIALIZE_STRING_ATTRIBUTE(news_headers_to_display);
	INITIALIZE_STRING_ATTRIBUTE(news_headers_to_not_display);
	INITIALIZE_STRING_ATTRIBUTE(news_quote_format);
	INITIALIZE_STRING_ATTRIBUTE(organization);
	INITIALIZE_STRING_ATTRIBUTE(quick_kill_scope);
	INITIALIZE_STRING_ATTRIBUTE(quick_select_scope);
	INITIALIZE_STRING_ATTRIBUTE(quote_chars);
	INITIALIZE_STRING_ATTRIBUTE(savedir);
	INITIALIZE_STRING_ATTRIBUTE(savefile);
	INITIALIZE_STRING_ATTRIBUTE(sigfile);
	INITIALIZE_STRING_ATTRIBUTE(thread_format);
#ifdef CHARSET_CONVERSION
	INITIALIZE_NUM_ATTRIBUTE(mm_network_charset);
	INITIALIZE_STRING_ATTRIBUTE(undeclared_charset);
#endif /* CHARSET_CONVERSION */
	INITIALIZE_STRING_ATTRIBUTE(x_body);
	INITIALIZE_STRING_ATTRIBUTE(x_headers);
}
