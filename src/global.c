/*
 *  Project   : tin - a Usenet reader
 *  Module    : global.c
 *  Author    : Jason Faultless <jason@radar.tele2.co.uk>
 *  Created   : 1999-12-12
 *  Updated   : 2000-01-05
 *  Notes     : Generic nagivation and key handling routines
 *
 * Copyright (c) 1999-2000 Jason Faultless <jason@radar.tele2.co.uk>
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Jason Faultless.
 * 4. The name of the author may not be used to endorse or promote
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
#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */

/*
 * Local prototypes
 */
static int _page_up (int curslot, int maxslot);
static int _page_down (int curslot, int maxslot);
static int mouse_action(int ch, int (*left_action) (void), int (*right_action) (void));

/*
 * Return the new line index following a PageUp request.
 * Take half page scrolling into account
 */
static int
_page_up (
	int curslot,
	int maxslot)
{
	int n, scroll_lines;

	if (curslot == 0)
		return (maxslot - 1);

	scroll_lines = (tinrc.full_page_scroll ? NOTESLINES : NOTESLINES / 2);

	if ((n = curslot % scroll_lines) > 0)
		curslot -= n;
	else
		curslot = ((curslot - scroll_lines) / scroll_lines) * scroll_lines;

	return ((curslot < 0) ? 0 : curslot);
}

/*
 * Return the new line index following a PageDown request.
 * Take half page scrolling into account
 */
static int
_page_down (
	int curslot,
	int maxslot)
{
	int scroll_lines;

	if (curslot == maxslot - 1)
		return 0;

	scroll_lines = (tinrc.full_page_scroll ? NOTESLINES : NOTESLINES / 2);

	curslot = ((curslot + scroll_lines) / scroll_lines) * scroll_lines;

	if (curslot >= maxslot) {
		curslot = (maxslot / scroll_lines) * scroll_lines;
		if (curslot < maxslot - 1)
			curslot = maxslot - 1;
	}

	return (curslot);
}


/*
 * Calculate the first and last objects that will appear on the current screen
 * based on the current position and the max available
 */
void
set_first_screen_item (void)
{
	if (currmenu->curr >= currmenu->max)
		currmenu->curr = currmenu->max - 1;

	if (NOTESLINES <= 0)
		currmenu->first = 0;
	else {
		currmenu->first = (currmenu->curr / NOTESLINES) * NOTESLINES;
		if (currmenu->first < 0)
			currmenu->first = 0;
	}

	currmenu->last = currmenu->first + NOTESLINES;

	if (currmenu->last >= currmenu->max) {
		currmenu->last = currmenu->max;
		currmenu->first = (currmenu->max / NOTESLINES) * NOTESLINES;

		if (currmenu->first == currmenu->last || currmenu->first < 0)
			currmenu->first = ((currmenu->first < 0) ? 0 : currmenu->last - NOTESLINES);
	}

	if (currmenu->max == 0)
		currmenu->first = currmenu->last = 0;
}


void
move_up (
	void)
{
	if (currmenu->max)
		move_to_item ((currmenu->curr == 0) ? (currmenu->max - 1) : (currmenu->curr - 1));
}


void
move_down (
	void)
{
	if (currmenu->max)
		move_to_item ((currmenu->curr + 1 >= currmenu->max) ? 0 : (currmenu->curr + 1));
}


void
page_up (
	void)
{
	if (currmenu->max)
		move_to_item (_page_up (currmenu->curr, currmenu->max));
}


void
page_down (
	void)
{
	if (currmenu->max)
		move_to_item (_page_down (currmenu->curr, currmenu->max));
}


void
top_of_list (
	void)
{
	if (currmenu->max)
		move_to_item (0);
}


void
end_of_list (
	void)
{
	if (currmenu->max)
		move_to_item (currmenu->max - 1);
}


void
prompt_item_num (
	int ch,
	const char *prompt)
{
	int num;

	clear_message ();

	if ((num = prompt_num (ch, prompt)) == -1) {
		clear_message ();
		return;
	}
	num--;		/* index from 0 (internal) vs. 1 (user) */

	if (num < 0)
		num = 0;

	if (num >= currmenu->max)
		num = currmenu->max - 1;

	move_to_item (num);
}


/*
 * Move the on-screen pointer & internal pointer variable to a new position
 */
void
move_to_item (
	int n)
{
	if (currmenu->curr == n)
		return;

	HpGlitch(erase_arrow ());
	erase_arrow ();

	currmenu->curr = n;
	if (currmenu->curr < 0)
		currmenu->curr = 0;
	clear_message ();

	if (n >= currmenu->first && n < currmenu->last)
		currmenu->draw_arrow ();
	else
		currmenu->redraw ();
}



/*
 * TODO - fold the call in select.c into here and make this static
 * Handle mouse clicks. We simply map the event to a return
 * keymap code that will drop through to call the correct function
 */
static int
mouse_action(
	int ch,
	int (*left_action) (void),		/* Typically catchup type event */
	int (*right_action) (void))		/* Typically read next etc.. */
{
	int INDEX_BOTTOM = INDEX_TOP + currmenu->last - currmenu->first;

	switch (xmouse) {
		case MOUSE_BUTTON_1:
		case MOUSE_BUTTON_3:
			if (xrow < INDEX_TOP || xrow >= INDEX_BOTTOM)
				return iKeyPageDown;

			erase_arrow();
			currmenu->curr = xrow - INDEX_TOP + currmenu->first;
			currmenu->draw_arrow();

			if (xmouse == MOUSE_BUTTON_1)
				return right_action();
			break;

		case MOUSE_BUTTON_2:
			if (xrow < INDEX_TOP || xrow >= INDEX_BOTTOM)
				return iKeyPageUp;

			return left_action();

		default:
			break;
	}

	return ch;			/* Pass through */
}


int
handle_keypad (
	int (*left_action) (void),
	int (*right_action) (void),
	const t_menukeys *menukeys)
{
	int ch = ReadCh ();

	switch (ch) {
#ifndef WIN32
		case ESC:	/* common arrow keys */
#	ifdef HAVE_KEY_PREFIX
		case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
			switch (get_arrow_key (ch)) {
#endif /* !WIN32 */
				case KEYMAP_UP:
					ch = iKeyUp;
					break;
				case KEYMAP_DOWN:
					ch = iKeyDown;
					break;
				case KEYMAP_LEFT:
					ch = left_action();
					break;
				case KEYMAP_RIGHT:
					ch = right_action();
					break;
				case KEYMAP_PAGE_UP:
					ch = iKeyPageUp;
					break;
				case KEYMAP_PAGE_DOWN:
					ch = iKeyPageDown;
					break;
				case KEYMAP_HOME:
					ch = iKeyFirstPage;
					break;
				case KEYMAP_END:
					ch = iKeyLastPage;
					break;
#ifndef WIN32
				case KEYMAP_MOUSE:
					ch = mouse_action (ch, left_action, right_action);
					break;
				default:
					break;
			}
			break;
#endif /* !WIN32 */
		default:
			ch = map_to_default (ch, menukeys);
			break;
	}
	return ch;
}


/*
 * bug/gripe/comment mailed to author
 */
void
bug_report (
	void)
{
	mail_bug_report ();
	ClearScreen ();
	currmenu->redraw ();
}
