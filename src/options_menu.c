/*
 *  Project   : tin - a Usenet reader
 *  Module    : options_menu.c
 *  Author    : Michael Bienia <michael@vorlon.ping.de>
 *  Created   : 2004-09-05
 *  Updated   : 2005-07-02
 *  Notes     : Split from config.c
 *
 * Copyright (c) 2004-2006 Michael Bienia <michael@vorlon.ping.de>
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

static enum option_enum first_option_on_screen, last_option_on_screen;

/*
 * local prototypes
 */
static enum option_enum move_cursor(enum option_enum cur_option, t_bool down);
static enum option_enum next_option(enum option_enum option, t_bool incl_titles);
static enum option_enum opt_scroll_down(enum option_enum option);
static enum option_enum opt_scroll_up(enum option_enum option);
static enum option_enum prev_option(enum option_enum option, t_bool incl_titles);
static enum option_enum set_option_num(int num);
static int get_option_num(enum option_enum option);
static t_bool option_is_title(enum option_enum option);
static t_bool option_on_page(enum option_enum option);
static t_function option_left(void);
static t_function option_right(void);
static void highlight_option(enum option_enum option);
static void print_any_option(enum option_enum the_option);
static void redraw_screen(enum option_enum option);
static void repaint_option(enum option_enum option);
static void set_first_option_on_screen(enum option_enum last_option);
static void set_last_option_on_screen(enum option_enum first_option);
static void show_config_page(void);
static void unhighlight_option(enum option_enum option);
#ifdef USE_CURSES
	static void do_scroll(int jump);
#endif /* USE_CURSES */


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

	for (i = 0; i < option && result < LAST_OPT; i = next_option(i, FALSE))
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

	while (num > 0 && result < LAST_OPT) {
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
			return tinrc.use_color;

		case OPT_COL_MARKSTAR:
		case OPT_COL_MARKDASH:
		case OPT_COL_MARKSLASH:
		case OPT_COL_MARKSTROKE:
			return tinrc.word_highlight && tinrc.use_color;
#endif /* HAVE_COLOR */

		case OPT_WORD_H_DISPLAY_MARKS:
		case OPT_MONO_MARKSTAR:
		case OPT_MONO_MARKDASH:
		case OPT_MONO_MARKSLASH:
		case OPT_MONO_MARKSTROKE:
			return tinrc.word_highlight;

		default:
			return TRUE;
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
		int num = get_option_num(option);

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (wbuf != NULL) {
			wbuf2 = wstrunc(wbuf, option_width);
			if ((buf = wchar_t2char(wbuf2)) == NULL) {
				/* conversion failed, truncate original string */
				buf = strunc(_(option_table[option].txt->opt), option_width);
				snprintf(dst, len, "%s %3d. %-*.*s: ", editing ? "->" : "  ", num, (int) option_width, (int) option_width, buf);
			} else
				snprintf(dst, len, "%s %3d. %-*.*s: ", editing ? "->" : "  ", num,
					(int) (strlen(buf) + option_width - wcswidth(wbuf2, option_width + 1)),
					(int) (strlen(buf) + option_width - wcswidth(wbuf2, option_width + 1)), buf);
			free(wbuf2);
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		{
			/* truncate original string */
			buf = strunc(_(option_table[option].txt->opt), option_width);
			snprintf(dst, len, "%s %3d. %-*.*s: ", editing ? "->" : "  ", num, (int) option_width, (int) option_width, buf);
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
			snprintf(ptr, len, "%s", _(ptr2));
			free(ptr2);
			break;

		case OPT_STRING:
			snprintf(ptr, len, "%s", OPT_STRING_list[option_table[option].var_index]);
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
				last_option_on_screen = prev_option(last_option_on_screen, TRUE);
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
	if (last_option_on_screen < LAST_OPT) {
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
		if (option > LAST_OPT)
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
			option = LAST_OPT;
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
	for (i = 1; i < option_lines_per_page && last_option_on_screen < LAST_OPT; i++)
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
	my_retouch();
	set_xclick_off();
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

	ClearScreen();
	center_line(0, TRUE, _(txt_options_menu));

	for (i = first_option_on_screen; i <= last_option_on_screen; i++) {
		while (!option_is_visible(i))
			i++;
		if (i > LAST_OPT)
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
 * options menu so that the user can dynamically change parameters
 *
 * TODO: - why do we use ret_code when we never modify it?  what about calling
 *         code which checks the return value?
 *       - when we change something we need to update the related attributes
 *         as well (see line 2009).
 */
int
change_config_file(
	struct t_group *group)
{
	enum option_enum option, old_option;
	int ret_code = NO_FILTERING;
	int mime_encoding = MIME_ENCODING_7BIT;
	t_bool change_option = FALSE;
	t_function func;

	signal_context = cConfig;

	option = 1;
	first_option_on_screen = 0;
	set_last_option_on_screen(0);

	redraw_screen(option);
	forever {
		switch ((func = handle_keypad(option_left, option_right, NULL, option_menu_keys))) {
			case GLOBAL_QUIT:
				write_config_file(local_config_file);
				/* FALLTHROUGH */
			case CONFIG_NO_SAVE:
				clear_note_area();
				return ret_code;

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
				option = 1;
				first_option_on_screen = 0;
				set_last_option_on_screen(0);
				redraw_screen(option);
				/* highlight_option(option); is already done by redraw_screen() */
				break;

			case GLOBAL_LAST_PAGE:
				unhighlight_option(option);
				option = LAST_OPT;
				last_option_on_screen = LAST_OPT;
				set_first_option_on_screen(LAST_OPT);
				redraw_screen(option);
				/* highlight_option(option); is already done by redraw_screen() */
				break;

			case GLOBAL_PAGE_UP:
				unhighlight_option(option);
				if (option != first_option_on_screen &&	!(option_is_title(first_option_on_screen) && option == next_option(first_option_on_screen, FALSE))) {
					option = first_option_on_screen;
					if (option_is_title(option))
						option = next_option(option, FALSE);
					highlight_option(option);
					break;
				} else if (tinrc.scroll_lines == -2 && first_option_on_screen != 0) {
					int i = option_lines_per_page / 2;

					for (; i > 0; i--) {
						last_option_on_screen = prev_option(last_option_on_screen, TRUE);
						if (last_option_on_screen == LAST_OPT)	/* end on wrap around */
							break;
					}
				} else
					last_option_on_screen = prev_option(first_option_on_screen, TRUE);

				set_first_option_on_screen(last_option_on_screen);
				if (last_option_on_screen == LAST_OPT)
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
				if (option == LAST_OPT) {
					/* wrap around */
					first_option_on_screen = 0;
					option = 0;
				} else {
					enum option_enum old_first = first_option_on_screen;

					if (tinrc.scroll_lines == -2) {
						int i = option_lines_per_page / 2;

						for (; i > 0; i--) {
							first_option_on_screen = next_option(first_option_on_screen, TRUE);
							if (first_option_on_screen == 0)	/* end on wrap_around */
								break;
						}
					} else
						first_option_on_screen = next_option(last_option_on_screen, TRUE);

					if (first_option_on_screen == 0) {
						first_option_on_screen = old_first;
						option = LAST_OPT;
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

			case GLOBAL_SEARCH_SUBJECT_FORWARD:
			case GLOBAL_SEARCH_SUBJECT_BACKWARD:
			case GLOBAL_SEARCH_REPEAT:
				if (func == GLOBAL_SEARCH_REPEAT && last_search != GLOBAL_SEARCH_SUBJECT_FORWARD && last_search != GLOBAL_SEARCH_SUBJECT_BACKWARD)
					break;

				old_option = option;
				option = search_config((func == GLOBAL_SEARCH_SUBJECT_FORWARD), (func == GLOBAL_SEARCH_REPEAT), option, LAST_OPT);
				if (option != old_option) {
					unhighlight_option(old_option);
					if (!option_on_page(option)) {
						first_option_on_screen = option;
						set_last_option_on_screen(option);
						redraw_screen(option);
					} else
						highlight_option(option);
				}
				break;

			case CONFIG_SELECT:
				change_option = TRUE;
				break;

			case GLOBAL_REDRAW_SCREEN:
				set_last_option_on_screen(first_option_on_screen);
				redraw_screen(option);
				break;

			case GLOBAL_VERSION:
				info_message(cvers);
				break;

			default:
				break;
		} /* switch (ch) */

		if (change_option) {
			switch (option_table[option].var_type) {
				case OPT_ON_OFF:
					switch (option) {
						case OPT_ADD_POSTED_TO_FILTER:
						case OPT_ADVERTISING:
						case OPT_ALTERNATIVE_HANDLING:
						case OPT_ASK_FOR_METAMAIL:
						case OPT_AUTO_BCC:
						case OPT_AUTO_CC:
						case OPT_AUTO_LIST_THREAD:
						case OPT_AUTO_RECONNECT:
						case OPT_AUTO_SAVE:
						case OPT_BATCH_SAVE:
						case OPT_CACHE_OVERVIEW_FILES:
						case OPT_CATCHUP_READ_GROUPS:
						case OPT_FORCE_SCREEN_REDRAW:
						case OPT_GROUP_CATCHUP_ON_EXIT:
						case OPT_KEEP_DEAD_ARTICLES:
						case OPT_MARK_IGNORE_TAGS:
						case OPT_MARK_SAVED_READ:
						case OPT_PGDN_GOTO_NEXT:
						case OPT_POS_FIRST_UNREAD:
						case OPT_POST_PROCESS_VIEW:
#ifndef DISABLE_PRINTING
						case OPT_PRINT_HEADER:
#endif /*! DISABLE_PRINTING */
						case OPT_PROCESS_ONLY_UNREAD:
						case OPT_PROMPT_FOLLOWUPTO:
						case OPT_SHOW_ONLY_UNREAD_GROUPS:
						case OPT_SHOW_SIGNATURES:
						case OPT_SIGDASHES:
						case OPT_SIGNATURE_REPOST:
						case OPT_SPACE_GOTO_NEXT_UNREAD:
						case OPT_START_EDITOR_OFFSET:
						case OPT_STRIP_BLANKS:
						case OPT_STRIP_NEWSRC:
						case OPT_TAB_GOTO_NEXT_UNREAD:
						case OPT_TEX2ISO_CONV:
						case OPT_THREAD_CATCHUP_ON_EXIT:
#if defined(HAVE_ICONV_OPEN_TRANSLIT) && defined(CHARSET_CONVERSION)
						case OPT_TRANSLIT:
#endif /* HAVE_ICONV_OPEN_TRANSLIT && CHARSET_CONVERSION */
						case OPT_UNLINK_ARTICLE:
						case OPT_URL_HIGHLIGHT:
#ifdef HAVE_KEYPAD
						case OPT_USE_KEYPAD:
#endif /* HAVE_KEYPAD */
						case OPT_USE_MOUSE:
						case OPT_WRAP_ON_NEXT_UNREAD:
							prompt_option_on_off(option);
							break;

						/* show mini help menu */
						case OPT_BEGINNER_LEVEL:
							if (prompt_option_on_off(option))
								set_noteslines(cLINES);
							break;

						/* show all arts or just new/unread arts */
						case OPT_SHOW_ONLY_UNREAD_ARTS:
							if (prompt_option_on_off(option) && group != NULL) {
								make_threads(group, TRUE);
								pos_first_unread_thread();
							}
							break;

						/* draw -> / highlighted bar */
						case OPT_DRAW_ARROW:
							prompt_option_on_off(option);
							unhighlight_option(option);
							if (!tinrc.draw_arrow && !tinrc.inverse_okay) {
								tinrc.inverse_okay = TRUE;
								repaint_option(OPT_INVERSE_OKAY);
							}
							break;

						/* draw inversed screen header lines */
						/* draw inversed group/article/option line if draw_arrow is OFF */
						case OPT_INVERSE_OKAY:
							prompt_option_on_off(option);
							unhighlight_option(option);
							if (!tinrc.draw_arrow && !tinrc.inverse_okay) {
								tinrc.draw_arrow = TRUE;	/* we don't want to navigate blindly */
								repaint_option(OPT_DRAW_ARROW);
							}
							break;

						case OPT_MAIL_8BIT_HEADER:
							prompt_option_on_off(option);
							if (strcasecmp(txt_mime_encodings[tinrc.mail_mime_encoding], txt_8bit)) {
								tinrc.mail_8bit_header = FALSE;
								print_any_option(OPT_MAIL_8BIT_HEADER);
							}
							break;

						case OPT_POST_8BIT_HEADER:
							prompt_option_on_off(option);
							/* if post_mime_encoding != 8bit, post_8bit_header is disabled */
							if (strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_8bit)) {
								tinrc.post_8bit_header = FALSE;
								print_any_option(OPT_POST_8BIT_HEADER);
							}
							break;

						/* show newsgroup description text next to newsgroups */
						case OPT_SHOW_DESCRIPTION:
							prompt_option_on_off(option);
							show_description = tinrc.show_description;
							if (show_description)			/* force reread of newgroups file */
								read_descriptions(FALSE);
							break;

#ifdef HAVE_COLOR
						/* use ANSI color */
						case OPT_USE_COLOR:
							prompt_option_on_off(option);
#	ifdef USE_CURSES
							if (!has_colors())
								use_color = FALSE;
							else
#	endif /* USE_CURSES */
								use_color = tinrc.use_color;
							set_last_option_on_screen(first_option_on_screen);
							redraw_screen(option);
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
							}
							break;
#endif /* XFACE_ABLE */

						/* word_highlight */
						case OPT_WORD_HIGHLIGHT:
							prompt_option_on_off(option);
							word_highlight = tinrc.word_highlight;
							set_last_option_on_screen(first_option_on_screen);
							redraw_screen(option);
							break;

#if defined(HAVE_LIBICUUC) && defined(MULTIBYTE_ABLE) && defined(HAVE_UNICODE_UBIDI_H) && !defined(NO_LOCALE)
						case OPT_RENDER_BIDI:
							prompt_option_on_off(option);
							break;
#endif /* HAVE_LIBICUUC && MULTIBYTE_ABLE && HAVE_UNICODE_UBIDI_H && !NO_LOCALE */

						default:
							break;
					} /* switch (option) */
					break;

				case OPT_LIST:
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
						case OPT_COL_MARKSTAR:
						case OPT_COL_MARKDASH:
						case OPT_COL_MARKSLASH:
						case OPT_COL_MARKSTROKE:
						case OPT_COL_URLS:
#endif /* HAVE_COLOR */
						case OPT_HIDE_UUE:
						case OPT_INTERACTIVE_MAILER:
						case OPT_WORD_H_DISPLAY_MARKS:
						case OPT_MONO_MARKSTAR:
						case OPT_MONO_MARKDASH:
						case OPT_MONO_MARKSLASH:
						case OPT_MONO_MARKSTROKE:
						case OPT_CONFIRM_CHOICE:
						case OPT_KILL_LEVEL:
						case OPT_MAILBOX_FORMAT:
						case OPT_SHOW_INFO:
						case OPT_SORT_ARTICLE_TYPE:
						case OPT_STRIP_BOGUS:
#ifdef HAVE_UNICODE_NORMALIZATION
						case OPT_NORMALIZATION_FORM:
#endif /* HAVE_UNICODE_NORMALIZATION */
						case OPT_QUOTE_STYLE:
						case OPT_WILDCARD:
							prompt_option_list(option);
							break;

						case OPT_THREAD_ARTICLES:
							/*
							 * If the threading strategy has changed, fix things
							 * so that rethreading will occur
							 */
							if (prompt_option_list(option) && group != NULL) {
								int n, old_base_art = base[grpmenu.curr];

								group->attribute->thread_arts = tinrc.thread_articles;
								make_threads(group, TRUE);
								/* in non-empty groups update cursor position */
								if (grpmenu.max > 0) {
									if ((n = which_thread(old_base_art)) >= 0)
										grpmenu.curr = n;
								}
							}
							set_last_option_on_screen(first_option_on_screen);
							redraw_screen(option);
							clear_message();
							break;

						case OPT_SORT_THREADS_TYPE:
							/*
							 * If the sorting strategy of threads has changed, fix things
							 * so that resorting will occur
							 */
							if (prompt_option_list(option) && group != NULL) {
								group->attribute->sort_threads_type = tinrc.sort_threads_type;
								make_threads(group, TRUE);
							}
							clear_message();
							break;

						case OPT_THREAD_SCORE:
							/*
							 * If the scoring of a thread has changed,
							 * resort base[]
							 */
							if (prompt_option_list(option) && group != NULL)
								find_base(group);
							clear_message();
							break;

						case OPT_POST_PROCESS:
							prompt_option_list(option);
							glob_attributes.post_proc_type = tinrc.post_process;
							if (group != NULL)
								group->attribute->post_proc_type = tinrc.post_process;
							break;

						case OPT_SHOW_AUTHOR:
							prompt_option_list(option);
							if (group != NULL)
								group->attribute->show_author = tinrc.show_author;
							break;

						case OPT_MAIL_MIME_ENCODING:
						case OPT_POST_MIME_ENCODING:
							prompt_option_list(option);
							mime_encoding = *(option_table[option].variable);
							/* do not use 8 bit headers if mime encoding is not 8bit */
							if (strcasecmp(txt_mime_encodings[mime_encoding], txt_8bit)) {
								if (option == (int) OPT_POST_MIME_ENCODING) {
									tinrc.post_8bit_header = FALSE;
									repaint_option(OPT_POST_8BIT_HEADER);
								} else {
									tinrc.mail_8bit_header = FALSE;
									repaint_option(OPT_MAIL_8BIT_HEADER);
								}
							}
							break;

#ifdef CHARSET_CONVERSION
						case OPT_MM_NETWORK_CHARSET:
							if (prompt_option_list(option)) {
								glob_attributes.mm_network_charset = tinrc.mm_network_charset;
								if (group)
									group->attribute->mm_network_charset = tinrc.mm_network_charset;
#	ifdef NO_LOCALE
								strcpy(tinrc.mm_local_charset, txt_mime_charsets[tinrc.mm_network_charset]);
#	endif /* NO_LOCALE */
							}
							/*
							 * check if we have selected a 7bit charset, otherwise
							 * update encoding
							 * we always do this (even if we did not change the
							 * charset) to fixup flaws in the tinrc - once we do
							 * the same while reading the tinrc this can go into
							 * the != original_list_value case.
							 */
							{
								int i;
								t_bool change;

								if (!strcasecmp(txt_mime_encodings[tinrc.post_mime_encoding], txt_7bit)) {
									change = TRUE;
									for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
										if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
											change = FALSE;
											break;
										}
									}
									if (change) {
										tinrc.post_mime_encoding = MIME_ENCODING_8BIT;
										repaint_option(OPT_POST_MIME_ENCODING);
									}
								} else { /* and vice versa, if we have a 7bit chaset but a !7bit encoding, fix that */
									for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
										if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
											tinrc.mail_mime_encoding = tinrc.post_mime_encoding = MIME_ENCODING_7BIT;
											tinrc.mail_8bit_header = tinrc.post_8bit_header = FALSE;
											repaint_option(OPT_POST_MIME_ENCODING);
											repaint_option(OPT_MAIL_MIME_ENCODING);
											repaint_option(OPT_POST_8BIT_HEADER);
											break;
										}
									}
								}

								if (!strcasecmp(txt_mime_encodings[tinrc.mail_mime_encoding], txt_7bit)) {
									change = TRUE;
									for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
										if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
											change = FALSE;
											break;
										}
									}
									if (change) {
										tinrc.mail_mime_encoding = MIME_ENCODING_QP;
										repaint_option(OPT_MAIL_MIME_ENCODING);
									}
								} else { /* and vice versa, if we have a 7bit chaset but a !7bit encoding, fix that */
									for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
										if (!strcasecmp(txt_mime_charsets[tinrc.mm_network_charset], txt_mime_7bit_charsets[i])) {
											tinrc.mail_mime_encoding = tinrc.post_mime_encoding = MIME_ENCODING_7BIT;
											tinrc.mail_8bit_header = tinrc.post_8bit_header = FALSE;
											repaint_option(OPT_POST_MIME_ENCODING);
											repaint_option(OPT_MAIL_MIME_ENCODING);
											repaint_option(OPT_POST_8BIT_HEADER);
											break;
										}
									}
								}
							}
							break;
#endif /* CHARSET_CONVERSION */

						default:
							break;
					} /* switch (option) */
					break;

				case OPT_STRING:
					switch (option) {
						case OPT_EDITOR_FORMAT:
						case OPT_INEWS_PROG:
						case OPT_MAILER_FORMAT:
						case OPT_MAIL_ADDRESS:
						case OPT_MAIL_QUOTE_FORMAT:
						case OPT_METAMAIL_PROG:
#ifndef CHARSET_CONVERSION
						case OPT_MM_CHARSET:
#endif /* !CHARSET_CONVERSION */
						case OPT_NEWS_QUOTE_FORMAT:
						case OPT_QUOTE_CHARS:
						case OPT_SPAMTRAP_WARNING_ADDRESSES:
						case OPT_URL_HANDLER:
						case OPT_XPOST_QUOTE_FORMAT:
							prompt_option_string(option);
							break;

						case OPT_NEWS_HEADERS_TO_DISPLAY:
							prompt_option_string(option);
							if (news_headers_to_display_array)
								FreeIfNeeded(*news_headers_to_display_array);
							FreeIfNeeded(news_headers_to_display_array);
							news_headers_to_display_array = ulBuildArgv(tinrc.news_headers_to_display, &num_headers_to_display);
							break;

						case OPT_NEWS_HEADERS_TO_NOT_DISPLAY:
							prompt_option_string(option);
							if (news_headers_to_not_display_array)
								FreeIfNeeded(*news_headers_to_not_display_array);
							FreeIfNeeded(news_headers_to_not_display_array);
							news_headers_to_not_display_array = ulBuildArgv(tinrc.news_headers_to_not_display, &num_headers_to_not_display);
							break;

#ifndef DISABLE_PRINTING
						case OPT_PRINTER:
#endif /* !DISABLE_PRINTING */
						case OPT_MAILDIR:
						case OPT_SAVEDIR:
						case OPT_SIGFILE:
						case OPT_POSTED_ARTICLES_FILE:
							if (prompt_option_string(option)) {
								char buf[PATH_LEN];

								strfpath(tinrc.posted_articles_file, buf, sizeof(buf), &CURR_GROUP);
								STRCPY(tinrc.posted_articles_file, buf);
							}
							break;

#ifdef HAVE_COLOR
						case OPT_QUOTE_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(quote_regex.re);
							FreeIfNeeded(quote_regex.extra);
							if (!strlen(tinrc.quote_regex))
								STRCPY(tinrc.quote_regex, DEFAULT_QUOTE_REGEX);
							compile_regex(tinrc.quote_regex, &quote_regex, PCRE_CASELESS);
							break;

						case OPT_QUOTE_REGEX2:
							prompt_option_string(option);
							FreeIfNeeded(quote_regex2.re);
							FreeIfNeeded(quote_regex2.extra);
							if (!strlen(tinrc.quote_regex2))
								STRCPY(tinrc.quote_regex2, DEFAULT_QUOTE_REGEX2);
							compile_regex(tinrc.quote_regex2, &quote_regex2, PCRE_CASELESS);
							break;

						case OPT_QUOTE_REGEX3:
							prompt_option_string(option);
							FreeIfNeeded(quote_regex3.re);
							FreeIfNeeded(quote_regex3.extra);
							if (!strlen(tinrc.quote_regex3))
								STRCPY(tinrc.quote_regex3, DEFAULT_QUOTE_REGEX3);
							compile_regex(tinrc.quote_regex3, &quote_regex3, PCRE_CASELESS);
							break;
#endif /* HAVE_COLOR */

						case OPT_SLASHES_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(slashes_regex.re);
							FreeIfNeeded(slashes_regex.extra);
							if (!strlen(tinrc.slashes_regex))
								STRCPY(tinrc.slashes_regex, DEFAULT_SLASHES_REGEX);
							compile_regex(tinrc.slashes_regex, &slashes_regex, PCRE_CASELESS);
							break;

						case OPT_STARS_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(stars_regex.re);
							FreeIfNeeded(stars_regex.extra);
							if (!strlen(tinrc.stars_regex))
								STRCPY(tinrc.stars_regex, DEFAULT_STARS_REGEX);
							compile_regex(tinrc.stars_regex, &stars_regex, PCRE_CASELESS);
							break;

						case OPT_STROKES_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(strokes_regex.re);
							FreeIfNeeded(strokes_regex.extra);
							if (!strlen(tinrc.strokes_regex))
								STRCPY(tinrc.strokes_regex, DEFAULT_STROKES_REGEX);
							compile_regex(tinrc.strokes_regex, &strokes_regex, PCRE_CASELESS);
							break;

						case OPT_UNDERSCORES_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(underscores_regex.re);
							FreeIfNeeded(underscores_regex.extra);
							if (!strlen(tinrc.underscores_regex))
								STRCPY(tinrc.underscores_regex, DEFAULT_UNDERSCORES_REGEX);
							compile_regex(tinrc.underscores_regex, &underscores_regex, PCRE_CASELESS);
							break;

						case OPT_STRIP_RE_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(strip_re_regex.re);
							FreeIfNeeded(strip_re_regex.extra);
							if (!strlen(tinrc.strip_re_regex))
								STRCPY(tinrc.strip_re_regex, DEFAULT_STRIP_RE_REGEX);
							compile_regex(tinrc.strip_re_regex, &strip_re_regex, PCRE_ANCHORED);
							break;

						case OPT_STRIP_WAS_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(strip_was_regex.re);
							FreeIfNeeded(strip_was_regex.extra);
							if (!strlen(tinrc.strip_was_regex)) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
								if (IS_LOCAL_CHARSET("UTF-8")) {
#	if (defined(PCRE_MAJOR) && PCRE_MAJOR >= 4)
									int i;

									pcre_config(PCRE_CONFIG_UTF8, &i);
									if (i)
										STRCPY(tinrc.strip_was_regex, DEFAULT_U8_STRIP_WAS_REGEX);
									else
#	endif /* PCRE_MAJOR && PCRE_MAJOR >= 4 */
										STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
								} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
									STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
							}
							compile_regex(tinrc.strip_was_regex, &strip_was_regex, 0);
							break;

						case OPT_DATE_FORMAT:
							prompt_option_string(option);
							if (!strlen(tinrc.date_format)) {
								STRCPY(tinrc.date_format, DEFAULT_DATE_FORMAT);
							}
							break;

						default:
							break;
					} /* switch (option) */

					break;

				case OPT_NUM:
					switch (option) {
						case OPT_GETART_LIMIT:
						case OPT_SCROLL_LINES:
							prompt_option_num(option);
							break;

						case OPT_REREAD_ACTIVE_FILE_SECS:
							prompt_option_num(option);
							if (tinrc.reread_active_file_secs < 0)
								tinrc.reread_active_file_secs = 0;
							break;

						case OPT_RECENT_TIME:
							prompt_option_num(option);
							if (tinrc.recent_time < 0)
								tinrc.recent_time = 0;
							break;

						case OPT_GROUPNAME_MAX_LENGTH:
							prompt_option_num(option);
							if (tinrc.groupname_max_length < 0)
								tinrc.groupname_max_length = 0;
							break;

						case OPT_FILTER_DAYS:
							prompt_option_num(option);
							if (tinrc.filter_days <= 0)
								tinrc.filter_days = 1;
							break;

						case OPT_SCORE_LIMIT_KILL:
						case OPT_SCORE_KILL:
						case OPT_SCORE_LIMIT_SELECT:
						case OPT_SCORE_SELECT:
							prompt_option_num(option);
							check_score_defaults();
							if (group != NULL) {
								unfilter_articles();
								read_filter_file(filter_file);
								if (filter_articles(group))
									make_threads(group, FALSE);
							}
							redraw_screen(option);
							break;

						case OPT_THREAD_PERC:
							prompt_option_num(option);
							if (tinrc.thread_perc < 0 || tinrc.thread_perc > 100)
								tinrc.thread_perc = THREAD_PERC_DEFAULT;
							break;

						case OPT_WRAP_COLUMN:
							prompt_option_num(option);
							/* recook if in an article is open */
							if (pgart.raw)
								resize_article(TRUE, &pgart);
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
							prompt_option_char(option);
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
	return ret_code;
}
