/*
 *  Project   : tin - a Usenet reader
 *  Module    : options_menu.c
 *  Author    : Michael Bienia <michael@vorlon.ping.de>
 *  Created   : 2004-09-05
 *  Updated   : 2009-01-14
 *  Notes     : Split from config.c
 *
 * Copyright (c) 2004-2009 Michael Bienia <michael@vorlon.ping.de>
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
		if (group != NULL) { \
			if ((i = add_tempscope(group->name))) { \
				scopes[i].attribute->option = tinrc.option; \
				scopes[i].state->option = TRUE; \
			} \
		} else { \
			for (i = 1; i < num_scope; i++) { \
				if (scopes[i].temp) \
					scopes[i].state->option = FALSE; \
			} \
		} \
		scopes[0].attribute->option = tinrc.option; \
	} while (0)

#define UPDATE_STRING_ATTRIBUTES(option) do { \
		if (group != NULL) { \
			if ((i = add_tempscope(group->name))) { \
				FreeIfNeeded(scopes[i].attribute->option); \
				scopes[i].attribute->option = my_strdup(tinrc.option); \
			} \
		} else { \
			for (i = 1; i < num_scope; i++) { \
				if (scopes[i].temp) \
					FreeAndNull(scopes[i].attribute->option); \
			} \
		} \
	} while (0)


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
		case OPT_QUOTE_REGEX:
		case OPT_QUOTE_REGEX2:
		case OPT_QUOTE_REGEX3:
			return tinrc.use_color;

		case OPT_COL_MARKSTAR:
		case OPT_COL_MARKDASH:
		case OPT_COL_MARKSLASH:
		case OPT_COL_MARKSTROKE:
			return tinrc.word_highlight && tinrc.use_color;

		case OPT_COL_VERBATIM:
			return tinrc.verbatim_handling && tinrc.use_color;
#endif /* HAVE_COLOR */

		case OPT_WORD_H_DISPLAY_MARKS:
		case OPT_MONO_MARKSTAR:
		case OPT_MONO_MARKDASH:
		case OPT_MONO_MARKSLASH:
		case OPT_MONO_MARKSTROKE:
		case OPT_SLASHES_REGEX:
		case OPT_STARS_REGEX:
		case OPT_STROKES_REGEX:
		case OPT_UNDERSCORES_REGEX:
			return tinrc.word_highlight;

		case OPT_VERBATIM_BEGIN_REGEX:
		case OPT_VERBATIM_END_REGEX:
			return tinrc.verbatim_handling;

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
 * TODO: - when we change something we need to update the related attributes
 *         as well (see line 2009).
 */
void
change_config_file(
	struct t_group *group)
{
	enum option_enum option, old_option;
	int i;
	t_bool change_option = FALSE;
	t_function func;
#ifdef CHARSET_CONVERSION
	t_bool is_7bit;
#endif /* CHARSET_CONVERSION */

	signal_context = cConfig;

	option = 1;
	first_option_on_screen = 0;
	set_last_option_on_screen(0);

	redraw_screen(option);
	forever {
		switch ((func = handle_keypad(option_left, option_right, NULL, option_menu_keys))) {
			case GLOBAL_QUIT:
				write_config_file(local_config_file);
				assign_attributes_to_groups();
				/*write_attributes_file(local_attributes_file);*/
				/* FALLTHROUGH */
			case CONFIG_NO_SAVE:
				clear_note_area();
				return;

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
					i = option_lines_per_page / 2;

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
						case OPT_URL_HIGHLIGHT:
#ifdef HAVE_KEYPAD
						case OPT_USE_KEYPAD:
#endif /* HAVE_KEYPAD */
						case OPT_USE_MOUSE:
							prompt_option_on_off(option);
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
#endif /*! DISABLE_PRINTING */

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
							if (prompt_option_on_off(option))
								UPDATE_INT_ATTRIBUTES(verbatim_handling);
							set_last_option_on_screen(first_option_on_screen);
							redraw_screen(option);
							break;

						/* show mini help menu */
						case OPT_BEGINNER_LEVEL:
							if (prompt_option_on_off(option))
								set_noteslines(cLINES);
							break;

						/* show all arts or just new/unread arts */
						case OPT_SHOW_ONLY_UNREAD_ARTS:
							if (prompt_option_on_off(option)) {
								UPDATE_INT_ATTRIBUTES(show_only_unread_arts);
								if (group != NULL) {
									group->attribute->show_only_unread_arts = tinrc.show_only_unread_arts;
									make_threads(group, TRUE);
									pos_first_unread_thread();
								}
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
							prompt_option_list(option);
							break;

						case OPT_AUTO_CC_BCC:
							if (prompt_option_list(option))
								UPDATE_INT_ATTRIBUTES(auto_cc_bcc);
							break;

						case OPT_THREAD_ARTICLES:
							/*
							 * If the threading strategy has changed, fix things
							 * so that rethreading will occur
							 */
							if (prompt_option_list(option)) {
								UPDATE_INT_ATTRIBUTES(thread_articles);
								if (group != NULL) {
									int old_base_art = base[grpmenu.curr];

									group->attribute->thread_articles = tinrc.thread_articles;
									make_threads(group, TRUE);
									/* in non-empty groups update cursor position */
									if (grpmenu.max > 0) {
										if ((i = which_thread(old_base_art)) >= 0)
											grpmenu.curr = i;
									}
								}
							}
							set_last_option_on_screen(first_option_on_screen);
							redraw_screen(option);
							clear_message();
							break;

						case OPT_SORT_ARTICLE_TYPE:
							if (prompt_option_list(option))
								UPDATE_INT_ATTRIBUTES(sort_article_type);
							break;

						case OPT_SORT_THREADS_TYPE:
							/*
							 * If the sorting strategy of threads has changed, fix things
							 * so that resorting will occur
							 */
							if (prompt_option_list(option)) {
								UPDATE_INT_ATTRIBUTES(sort_threads_type);
								if (group != NULL) {
									group->attribute->sort_threads_type = tinrc.sort_threads_type;
									make_threads(group, TRUE);
								}
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

						case OPT_TRIM_ARTICLE_BODY:
							if (prompt_option_list(option))
								UPDATE_INT_ATTRIBUTES(trim_article_body);
							break;

						case OPT_POST_PROCESS_TYPE:
							if (prompt_option_list(option))
								UPDATE_INT_ATTRIBUTES(post_process_type);
							break;

						case OPT_SHOW_AUTHOR:
							if (prompt_option_list(option))
								UPDATE_INT_ATTRIBUTES(show_author);
							break;

						case OPT_SHOW_INFO:
							if (prompt_option_list(option))
								UPDATE_INT_ATTRIBUTES(show_info);
							break;

						case OPT_MAIL_MIME_ENCODING:
							if (prompt_option_list(option)) {
#ifdef CHARSET_CONVERSION
								/*
								 * check if we have selected a !7bit encoding but a 7bit network charset
								 * or a !8bit encoding but a 8bit network charset, update encoding if needed
								 */
								is_7bit = FALSE;
								for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
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
								for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
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
								for (i = 0; *txt_mime_7bit_charsets[i]; i++) {
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

						default:
							break;
					} /* switch (option) */
					break;

				case OPT_STRING:
					switch (option) {
						case OPT_INEWS_PROG:
						case OPT_MAILER_FORMAT:
						case OPT_MAIL_QUOTE_FORMAT:
						case OPT_METAMAIL_PROG:
#ifndef DISABLE_PRINTING
						case OPT_PRINTER:
#endif /* !DISABLE_PRINTING */
						case OPT_SPAMTRAP_WARNING_ADDRESSES:
						case OPT_URL_HANDLER:
						case OPT_XPOST_QUOTE_FORMAT:
							prompt_option_string(option);
							break;

						case OPT_EDITOR_FORMAT:
							if (prompt_option_string(option)) {
								if (!strlen(tinrc.editor_format))
									STRCPY(tinrc.editor_format, TIN_EDITOR_FMT_ON);
								UPDATE_STRING_ATTRIBUTES(editor_format);
							}
							break;

						case OPT_MAIL_ADDRESS:
							if (prompt_option_string(option)) {
								if (group != NULL) {
									if ((i = add_tempscope(group->name))) {
										FreeIfNeeded(scopes[i].attribute->from);
										scopes[i].attribute->from = my_strdup(tinrc.mail_address);
									}
								} else {
									for (i = 1; i < num_scope; i++) {
										if (scopes[i].temp)
											FreeAndNull(scopes[i].attribute->from);
									}
								}
							}
							break;

#ifndef CHARSET_CONVERSION
						case OPT_MM_CHARSET:
							prompt_option_string(option);
							/*
							 * No charset conversion available, assume local charset
							 * to be network charset.
							 */
							STRCPY(tinrc.mm_local_charset, tinrc.mm_charset);
							break;
#else
#	ifdef NO_LOCALE
						case OPT_MM_LOCAL_CHARSET:
							prompt_option_string(option);
							/* no locales -> can't guess local charset */
							break;

#	endif /* NO_LOCALE */
#endif /* !CHARSET_CONVERSION */

						case OPT_NEWS_HEADERS_TO_DISPLAY:
							if (prompt_option_string(option)) {
								UPDATE_STRING_ATTRIBUTES(news_headers_to_display);
								if (group != NULL && i)
									build_news_headers_array(scopes[i].attribute, TRUE);
								else {
									for (i = 1; i < num_scope; i++) {
										if (scopes[i].temp && scopes[i].attribute->headers_to_display) {
											if (scopes[i].attribute->headers_to_display->header)
												FreeIfNeeded(*scopes[i].attribute->headers_to_display->header);
											FreeAndNull(scopes[i].attribute->headers_to_display->header);
											free(scopes[i].attribute->headers_to_display);
											scopes[i].attribute->headers_to_display = (struct t_newsheader *) 0;
										}
									}
								}
								build_news_headers_array(scopes[0].attribute, TRUE);
							}
							break;

						case OPT_NEWS_HEADERS_TO_NOT_DISPLAY:
							if (prompt_option_string(option)) {
								UPDATE_STRING_ATTRIBUTES(news_headers_to_not_display);
								if (group != NULL && i)
									build_news_headers_array(scopes[i].attribute, FALSE);
								else {
									for (i = 1; i < num_scope; i++) {
										if (scopes[i].temp && scopes[i].attribute->headers_to_not_display) {
											if (scopes[i].attribute->headers_to_not_display->header)
												FreeIfNeeded(*scopes[i].attribute->headers_to_not_display->header);
											FreeAndNull(scopes[i].attribute->headers_to_not_display->header);
											free(scopes[i].attribute->headers_to_not_display);
											scopes[i].attribute->headers_to_not_display = (struct t_newsheader *) 0;
										}
									}
								}
								build_news_headers_array(scopes[0].attribute, FALSE);
							}
							break;

						case OPT_NEWS_QUOTE_FORMAT:
							if (prompt_option_string(option))
								UPDATE_STRING_ATTRIBUTES(news_quote_format);
							break;

						case OPT_MAILDIR:
							if (prompt_option_string(option))
								UPDATE_STRING_ATTRIBUTES(maildir);
							break;

						case OPT_SAVEDIR:
							if (prompt_option_string(option))
								UPDATE_STRING_ATTRIBUTES(savedir);
							break;

						case OPT_SIGFILE:
							if (prompt_option_string(option))
								UPDATE_STRING_ATTRIBUTES(sigfile);
							break;

						case OPT_POSTED_ARTICLES_FILE:
							if (prompt_option_string(option)) {
								char buf[PATH_LEN];

								strfpath(tinrc.posted_articles_file, buf, sizeof(buf), &CURR_GROUP);
								STRCPY(tinrc.posted_articles_file, buf);
							}
							break;

						case OPT_QUOTE_CHARS:
							if (prompt_option_string(option))
								UPDATE_STRING_ATTRIBUTES(quote_chars);
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
								if (IS_LOCAL_CHARSET("UTF-8") && utf8_pcre())
									STRCPY(tinrc.strip_was_regex, DEFAULT_U8_STRIP_WAS_REGEX);
								else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
									STRCPY(tinrc.strip_was_regex, DEFAULT_STRIP_WAS_REGEX);
							}
							compile_regex(tinrc.strip_was_regex, &strip_was_regex, 0);
							break;

						case OPT_VERBATIM_BEGIN_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(verbatim_begin_regex.re);
							FreeIfNeeded(verbatim_begin_regex.extra);
							if (!strlen(tinrc.verbatim_begin_regex))
								STRCPY(tinrc.verbatim_begin_regex, DEFAULT_VERBATIM_BEGIN_REGEX);
							compile_regex(tinrc.verbatim_begin_regex, &verbatim_begin_regex, PCRE_ANCHORED);
							break;

						case OPT_VERBATIM_END_REGEX:
							prompt_option_string(option);
							FreeIfNeeded(verbatim_end_regex.re);
							FreeIfNeeded(verbatim_end_regex.extra);
							if (!strlen(tinrc.verbatim_end_regex))
								STRCPY(tinrc.verbatim_end_regex, DEFAULT_VERBATIM_END_REGEX);
							compile_regex(tinrc.verbatim_end_regex, &verbatim_end_regex, PCRE_ANCHORED);
							break;

						case OPT_DATE_FORMAT:
							if (prompt_option_string(option)) {
								if (!strlen(tinrc.date_format))
									STRCPY(tinrc.date_format, DEFAULT_DATE_FORMAT);
								UPDATE_STRING_ATTRIBUTES(date_format);
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
							if (prompt_option_num(option)) {
								if (tinrc.thread_perc < 0 || tinrc.thread_perc > 100)
									tinrc.thread_perc = THREAD_PERC_DEFAULT;
								UPDATE_INT_ATTRIBUTES(thread_perc);
							}
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
	return;
}
