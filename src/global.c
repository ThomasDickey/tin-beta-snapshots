/*
 *  Project   : tin - a Usenet reader
 *  Module    : global.c
 *  Author    : Jason Faultless <jason@radar.tele2.co.uk>
 *  Created   : 1999-12-12
 *  Updated   : 1999-12-12
 *  Notes     : Generic nagivation and key handling routines
 *  Copyright : (c) 1996-99 by Jason Faultless
 *              You may  freely  copy or  redistribute  this software,
 *              so  long as there is no profit made from its use, sale
 *              trade or  reproduction.  You may not change this copy-
 *              right notice, and it must be included in any copy made
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
set_first_screen_item ()
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

	if (num >= selmenu.max)
		num = selmenu.max - 1;

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

	HpGlitch(currmenu->erase_arrow ());
	currmenu->erase_arrow ();

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
 * Handle (ncurses) mouse clicks. We simply map the event to a return
 * keymap code that will drop through to call the correct function
 */
int
mouse_action(
	int ch,
	int (*left_action) (void),		/* Typically catchup type event */
	int (*right_action) (void))		/* Typically read next etc.. */
{
	int INDEX_BOTTOM = INDEX_TOP + currmenu->last - currmenu->first;
/* fprintf(stderr, "IB %d row %d\n", INDEX_BOTTOM, xrow); */
	switch (xmouse) {
		case MOUSE_BUTTON_1:
		case MOUSE_BUTTON_3:
			if (xrow < INDEX_TOP || xrow >= INDEX_BOTTOM)
				return iKeyPageDown;

			currmenu->erase_arrow();
			currmenu->curr = xrow - INDEX_TOP + currmenu->first;
			currmenu->draw_arrow();

			if (xmouse == MOUSE_BUTTON_1)
				return right_action();
			break;

		case MOUSE_BUTTON_2:
			if (xrow < INDEX_TOP || xrow >= INDEX_BOTTOM)
				return iKeyPageUp;

			return left_action();
			break;

		default:
			break;
	}

	return ch;			/* Pass through */
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
