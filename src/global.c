/*
 *  Project   : tin - a Usenet reader
 *  Module    : global.c
 *  Author    : Jason Faultless <jason@altarstone.com>
 *  Created   : 1999-12-12
 *  Updated   : 2005-10-19
 *  Notes     : Generic nagivation and key handling routines
 *
 * Copyright (c) 1999-2006 Jason Faultless <jason@altarstone.com>
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
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */

/*
 * Local prototypes
 */
#ifdef USE_CURSES
	static void do_scroll(int jump);
#endif /* USE_CURSES */


/*
 * Calculate the first and last objects that will appear on the current screen
 * based on the current position and the max available
 */
void
set_first_screen_item(
	void)
{
	if (currmenu->max == 0) {
		currmenu->first = 0;
		currmenu->curr = -1;
		return;
	}

	if (currmenu->curr >= currmenu->max)
		currmenu->curr = currmenu->max - 1;
	else if (currmenu->curr < -1)
		currmenu->curr = -1;

	if (currmenu->curr < currmenu->first || currmenu->curr > currmenu->first + NOTESLINES - 1) /* current selection is out of screen */
		currmenu->first = (currmenu->curr / NOTESLINES) * NOTESLINES;
}


void
move_up(
	void)
{
	if (!currmenu->max)
		return;

	if (currmenu->curr - 1 < currmenu->first && currmenu->curr != 0) {
		currmenu->first--;
#ifdef USE_CURSES
		do_scroll(-1);
		currmenu->draw_item(currmenu->curr - 1);
#else
		currmenu->redraw();
#endif /* USE_CURSES */
	}
	if (currmenu->curr == 0) {
		currmenu->first = MAX(0, currmenu->max - NOTESLINES);

		if (currmenu->max - 1 >= NOTESLINES) {
			currmenu->curr = currmenu->max - 1;
			currmenu->redraw();
		} else
			move_to_item(currmenu->max - 1);
	} else
		move_to_item(currmenu->curr - 1);
}


void
move_down(
	void)
{
	if (!currmenu->max)
		return;

	if (currmenu->curr + 1 > currmenu->first + NOTESLINES - 1 && currmenu->curr + 1 < currmenu->max) {
		currmenu->first++;
#ifdef USE_CURSES
		do_scroll(1);
		currmenu->draw_item(currmenu->curr + 1);
#else
		currmenu->redraw();
#endif /* USE_CURSES */
	}
	move_to_item((currmenu->curr + 1 >= currmenu->max) ? 0 : (currmenu->curr + 1));
}


void
page_up(
	void)
{
	int scroll_lines;

	if (!currmenu->max)
		return;

	if (currmenu->curr == currmenu->first) {
		scroll_lines = (tinrc.scroll_lines == -2) ? NOTESLINES / 2 : NOTESLINES;
		if (currmenu->first == 0) {
			/* wrap around */
			currmenu->first = MAX(0, currmenu->max - scroll_lines);
			currmenu->curr = currmenu->max - 1;
		} else {
			currmenu->first = MAX(0, currmenu->first - scroll_lines);
			currmenu->curr = currmenu->first;
		}
		currmenu->redraw();
	} else
		move_to_item(currmenu->first);
}


void
page_down(
	void)
{
	int scroll_lines;

	if (!currmenu->max)
		return;

	if (currmenu->curr == currmenu->max - 1) {
		/* wrap around */
		currmenu->first = 0;
		currmenu->curr = 0;
		currmenu->redraw();
	} else {
		scroll_lines = (tinrc.scroll_lines == -2) ? NOTESLINES / 2 : NOTESLINES;
		if (currmenu->first + scroll_lines >= currmenu->max)
			move_to_item(currmenu->max - 1);
		else {
			currmenu->first += scroll_lines;
			currmenu->curr = currmenu->first;
			currmenu->redraw();
		}
	}
}


void
top_of_list(
	void)
{
	if (currmenu->max)
		move_to_item(0);
}


void
end_of_list(
	void)
{
	if (currmenu->max)
		move_to_item(currmenu->max - 1);
}


void
prompt_item_num(
	int ch,
	const char *prompt)
{
	int num;

	clear_message();

	if ((num = prompt_num(ch, prompt)) == -1) {
		clear_message();
		return;
	}

	if (--num < 0) /* index from 0 (internal) vs. 1 (user) */
		num = 0;

	if (num >= currmenu->max)
		num = currmenu->max - 1;

	move_to_item(num);
}


/*
 * Move the on-screen pointer & internal pointer variable to a new position
 */
void
move_to_item(
	int n)
{
	if (currmenu->curr == n)
		return;

	HpGlitch(erase_arrow());
	erase_arrow();

	if ((currmenu->curr = n) < 0)
		currmenu->curr = 0;
	clear_message();

	if (n >= currmenu->first && n < currmenu->first + NOTESLINES)
		currmenu->draw_arrow();
	else
		currmenu->redraw();
}


/*
 * scroll the screen one line down
 * the selected item is only moved if it is scrolled off the screen
 */
void
scroll_down(
	void)
{
	if (!currmenu->max || currmenu->first + NOTESLINES >= currmenu->max)
		return;

	currmenu->first++;
#ifdef USE_CURSES
	do_scroll(1);
	currmenu->draw_item(currmenu->first + NOTESLINES - 1);
	stow_cursor();
	if (currmenu->curr < currmenu->first)
		move_to_item(currmenu->curr + 1);
#else
	if (currmenu->curr < currmenu->first)
		currmenu->curr++;
	currmenu->redraw();
#endif /* USE_CURSES */
}


/*
 * scroll the screen one line up
 * the selected item is only moved if it is scrolled off the screen
 */
void
scroll_up(
	void)
{
	if (!currmenu->max || currmenu->first == 0)
		return;

	currmenu->first--;
#ifdef USE_CURSES
	do_scroll(-1);
	currmenu->draw_item(currmenu->first);
	stow_cursor();
	if (currmenu->curr >= currmenu->first + NOTESLINES)
		move_to_item(currmenu->curr - 1);
#else
	if (currmenu->curr >= currmenu->first + NOTESLINES)
		currmenu->curr--;
	currmenu->redraw();
#endif /* USE_CURSES */
}


#ifdef USE_CURSES
/* TODO: merge with options_menu.c:do_scroll() and move to tcurses.c */
/* scroll the screen 'jump' lines down or up (if 'jump' < 0) */
static void
do_scroll(
	int jump)
{
	scrollok(stdscr, TRUE);
	MoveCursor(INDEX_TOP, 0);
	SetScrollRegion(INDEX_TOP, INDEX_TOP + NOTESLINES - 1);
	ScrollScreen(jump);
	SetScrollRegion(0, LINES - 1);
	scrollok(stdscr, FALSE);
}
#endif /* USE_CURSES */


/*
 * Handle mouse clicks. We simply map the event to a return
 * keymap code that will drop through to call the correct function
 */
t_function
global_mouse_action(
	t_function (*left_action) (void),
	t_function (*right_action) (void))
{
	int INDEX_BOTTOM = INDEX_TOP + NOTESLINES;

	switch (xmouse) {
		case MOUSE_BUTTON_1:
		case MOUSE_BUTTON_3:
			if (xrow < INDEX_TOP || xrow >= INDEX_BOTTOM)
				return GLOBAL_PAGE_DOWN;

			erase_arrow();
			currmenu->curr = xrow - INDEX_TOP + currmenu->first;
			currmenu->draw_arrow();

			if (xmouse == MOUSE_BUTTON_1)
				return right_action();
			break;

		case MOUSE_BUTTON_2:
			if (xrow < INDEX_TOP || xrow >= INDEX_BOTTOM)
				return GLOBAL_PAGE_UP;

			return left_action();

		default:
			break;
	}
	return NOT_ASSIGNED;
}


t_function
handle_keypad(
	t_function (*left_action) (void),
	t_function (*right_action) (void),
	t_function (*mouse_action) (
		t_function (*left_action) (void),
		t_function (*right_action) (void)),
	const struct keylist keys)
{
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wint_t ch = ReadWch();
#else
	int ch = ReadCh();
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	t_function func = NOT_ASSIGNED;

	switch (ch) {
		case ESC:	/* common arrow keys */
#	ifdef HAVE_KEY_PREFIX
		case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
			switch (get_arrow_key((int) ch)) {
				case KEYMAP_UP:
					func = GLOBAL_LINE_UP;
					break;

				case KEYMAP_DOWN:
					func = GLOBAL_LINE_DOWN;
					break;

				case KEYMAP_LEFT:
					func = left_action();
					break;

				case KEYMAP_RIGHT:
					func = right_action();
					break;

				case KEYMAP_PAGE_UP:
					func = GLOBAL_PAGE_UP;
					break;

				case KEYMAP_PAGE_DOWN:
					func = GLOBAL_PAGE_DOWN;
					break;

				case KEYMAP_HOME:
					func = GLOBAL_FIRST_PAGE;
					break;

				case KEYMAP_END:
					func = GLOBAL_LAST_PAGE;
					break;

				case KEYMAP_MOUSE:
					if (mouse_action)
						func = mouse_action(left_action, right_action);
					break;

				default:
					break;
			}
			break;

		default:
			func = key_to_func(ch, keys);
			break;
	}
	return func;
}


/*
 * bug/gripe/comment mailed to author
 */
void
bug_report(
	void)
{
	mail_bug_report();
	ClearScreen();
	currmenu->redraw();
}
