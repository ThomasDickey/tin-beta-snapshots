/*
 *  Project   : tin - a Usenet reader
 *  Module    : prompt.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2004-02-09
 *  Notes     :
 *
 * Copyright (c) 1991-2004 Iain Lea <iain@bricbrac.de>
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
#ifndef MENUKEYS_H
#	include "menukeys.h"
#endif /* !MENUKEYS_H */

/*
 *  prompt_num
 *  get a number from the user
 *  Return -1 if missing or bad number typed
 */
int
prompt_num(
	int ch,
	const char *prompt)
{
	char *p;
	char tmp[LEN];
	int num;

	clear_message();

	snprintf(tmp, sizeof(tmp), "%c", ch);

	if ((p = tin_getline(prompt, TRUE, tmp, 0, FALSE, HIST_OTHER)) != NULL) {
		STRCPY(tmp, p);
		num = atoi(tmp);
	} else
		num = -1;

	clear_message();

	return num;
}


/*
 *  prompt_string
 *  get a string from the user
 *  Return TRUE if a valid string was typed, FALSE otherwise
 *  TODO: no bounds checking on buf size, tin_getline() defaults to 1024
 */
t_bool
prompt_string(
	const char *prompt,
	char *buf,
	int which_hist)
{
	return prompt_default_string(prompt, buf, 0, (char *) NULL, which_hist);
}


/*
 * prompt_default_string
 * get a string from the user, display default value
 * Return TRUE if a valid string was typed, FALSE otherwise
 */
t_bool
prompt_default_string(
	const char *prompt,
	char *buf,
	int buf_len,
	char *default_prompt,
	int which_hist)
{
	char *p;

	clear_message();

	if ((p = tin_getline(prompt, FALSE, default_prompt, buf_len, FALSE, which_hist)) == NULL) {
		buf[0] = '\0';
		clear_message();
		return FALSE;
	}
	strcpy(buf, p);

	clear_message();

	return TRUE;
}


/*
 *  prompt_menu_string
 *  get a string from the user
 *  Return TRUE if a valid string was typed, FALSE otherwise
 */
t_bool
prompt_menu_string(
	int line,
	const char *prompt,
	char *var)
{
	char *p;

	/*
	 * clear buffer - this is needed, otherwise a lost
	 * connection right before a resync_active() call
	 * would lead to a 'n' answer to the reconnect prompt
	 */
	fflush(stdin);

	MoveCursor(line, 0);

	if ((p = tin_getline(prompt, FALSE, var, 0, FALSE, HIST_OTHER)) == NULL)
		return FALSE;

	strcpy(var, p);

	return TRUE;
}


/*
 * prompt_yn
 * prompt user for 'y'es or 'n'o decision. "prompt" will be displayed in line
 * "line" giving the default answer "default_answer".
 * TODO: 'line' is constant - can we remove it ?
 * The function returns 1 if the user decided "yes", -1 if the user wanted
 * to escape, or 0 for any other key or decision.
 */
int
prompt_yn(
	int line,
	const char *prompt,
	t_bool default_answer)
{
	char *keyprompt;
	char keyno[MAXKEYLEN], keyyes[MAXKEYLEN];
	int ch = 'y', prompt_ch = 'y';
	size_t maxlen;
	t_bool yn_loop = TRUE;

/*	fflush(stdin); */		/* Prevent finger trouble from making important decisions */

	(void) printascii(keyyes, (default_answer ? toupper(map_to_local(iKeyPromptYes, &menukeymap.prompt_yn)) : map_to_local(iKeyPromptYes, &menukeymap.prompt_yn)));
	(void) printascii(keyno, (!default_answer ? toupper(map_to_local(iKeyPromptNo, &menukeymap.prompt_yn)) : map_to_local(iKeyPromptNo, &menukeymap.prompt_yn)));
	maxlen = MAX(strlen(keyyes), strlen(keyno));

	while (yn_loop) {
		prompt_ch = map_to_local((default_answer ? iKeyPromptYes : iKeyPromptNo), &menukeymap.prompt_yn);
		keyprompt = (default_answer ? keyyes : keyno);

		if (!cmd_line) {
			MoveCursor(line, 0);
			CleartoEOLN();
		}
		my_printf("%s (%s/%s) %-*s", prompt, keyyes, keyno, (int) maxlen, keyprompt);
		if (!cmd_line)
			cursoron();
		my_flush();
		if (!cmd_line)
			MoveCursor(line, (int) strlen(prompt) + strlen(keyyes) + strlen(keyno) + 5);

		if (((ch = (char) ReadCh()) == '\n') || (ch == '\r'))
			ch = prompt_ch;

		yn_loop = FALSE; /* normal case: leave loop */

		switch (ch) {
			case ESC:	/* (ESC) common arrow keys */
#	ifdef HAVE_KEY_PREFIX
			case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
				switch (get_arrow_key(ch)) {

					case KEYMAP_UP:
					case KEYMAP_DOWN:
						default_answer = bool_not(default_answer);
						yn_loop = TRUE; /* don't leave loop */
						break;

					case KEYMAP_LEFT:
						ch = iKeyAbort;
						break;

					case KEYMAP_RIGHT:
						ch = prompt_ch;
						break;
					default:
						break;
				}
				break;

			default:
				break;
		}
	}

	if (!cmd_line) {
		if (line == cLINES)
			clear_message();
		else {
			MoveCursor(line, (int) strlen(prompt));
			my_fputc(((ch == iKeyAbort) ? prompt_ch : ch), stdout);
		}
		cursoroff();
		my_flush();
	}

	return (tolower((unsigned char) map_to_default(ch, &menukeymap.prompt_yn)) == tolower((unsigned char)iKeyPromptYes)) ? 1 : (ch == iKeyAbort) ? -1 : 0;
}


/*
 * help_text is displayed near the bottom of the screen.
 * var is an index into a list containing size elements.
 * The text from list is shown at row, col + len(prompt_text)
 * Choice is incremented using the space bar, wrapping to 0
 * ESC is used to abort any changes, RET saves changes.
 * The new value is returned.
 */
int
prompt_list(
	int row,
	int col,
	int var,
	constext *help_text,
	constext *prompt_text,
	constext *list[],
	int size)
{
	int ch, var_orig;
	int i, offset;
	int adjust = (strcasecmp(_(list[0]), _(txt_default)) == 0);
	size_t width = 0;

	var += adjust;
	size += adjust;
	var_orig = var;

	/*
	 * Find the length of longest printable text
	 */
	for (i = 0; i < size; i++)
		width = MAX(width, strlen(_(list[i])));

	show_menu_help(help_text);
	cursoron();

	do {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		size_t len;

		len = mbstowcs(NULL, _(prompt_text), 0);
		if (len != (size_t) (-1)) {
			wchar_t *wbuf = my_malloc(sizeof(wchar_t) * (len + 1));

			mbstowcs(wbuf, _(prompt_text), len + 1);
			wconvert_to_printable(wbuf);
			offset = wcswidth(wbuf, len + 1);
			if (offset == -1) {
				/* something went wrong, use wcslen as fallback */
				offset = (int) wcslen(wbuf);
			}

			free(wbuf);
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			offset = (int) strlen(_(prompt_text));

		MoveCursor(row, col + offset);
		if ((ch = (char) ReadCh()) == ' ') {

			/*
			 * Increment list, looping around at the max
			 */
			++var;
			var %= size;

			my_printf("%-*s", (int) width, _(list[var]));
			my_flush();
		}
	} while (ch != '\r' && ch != '\n' && ch != ESC);

	if (ch == ESC) {
		var = var_orig;

		my_printf("%-*s", (int) width, _(list[var]));
		my_flush();
	}

	cursoroff();

	return (var - adjust);
}


/*
 * Special case of prompt_list() Toggle between ON and OFF
 */
void
prompt_on_off(
	int row,
	int col,
	t_bool *var,
	constext *help_text,
	constext *prompt_text)
{
	t_bool ret;

	ret = prompt_list(row, col, (int) *var, help_text, _(prompt_text), txt_onoff, 2) ? TRUE : FALSE;
	*var = (ret != 0);
}


/*
 * Displays option text and actual option value for string based options in
 * one line, help text for that option near the bottom of the screen. Allows
 * change of the old value by normal editing; history function of tin_getline()
 * will be used properly so that editing won't leave the actual line.
 *
 * The function returns TRUE, if the value was changed, FALSE otherwise.
 */
t_bool
prompt_option_string(
	int option) /* return value is always ignored */
{
	char prompt[LEN];
	char *variable = OPT_STRING_list[option_table[option].var_index];

	show_menu_help(option_table[option].txt->help);
	fmt_option_prompt(prompt, sizeof(prompt) - 1, TRUE, option);
	return prompt_menu_string(option_row(option), prompt, variable);
}


/*
 * Displays option text and current option value for number based options in
 * one line, help text for that option near the bottom of the screen. Allows
 * change of the old value by normal editing; history function of tin_getline()
 * will be used properly so that editing won't leave the current line.
 *
 * The function returns TRUE if the value was changed, FALSE otherwise.
 */
t_bool
prompt_option_num(
	int option) /* return value is always ignored */
{
	char prompt[LEN];
	char number[LEN];
	char *p;
	int num;

	show_menu_help(option_table[option].txt->help);
	MoveCursor(option_row(option), 0);
	fmt_option_prompt(prompt, sizeof(prompt) - 1, TRUE, option);
	snprintf(&number[0], sizeof(number), "%d", *(option_table[option].variable));

	if ((p = tin_getline(prompt, 2, number, 0, FALSE, HIST_OTHER)) == NULL)
		return FALSE;

	STRCPY(number, p);
	num = atoi(number);
	*(option_table[option].variable) = num;

	clear_message();

	return TRUE;
}


/*
 * Displays option text and actual option value for character based options
 * in one line, help text for that option near the bottom of the screen.
 * Allows change of the old value by normal editing.
 *
 * The function returns TRUE if the value was changed, FALSE otherwise.
 */
t_bool
prompt_option_char(
	int option) /* return value is always ignored */
{
	char prompt[LEN];
	char input[2];
	char *p = &input[0];
	char *variable = OPT_CHAR_list[option_table[option].var_index];

	input[0] = *variable;
	input[1] = '\0';

	do {
		show_menu_help(option_table[option].txt->help);
		MoveCursor(option_row(option), 0);
		fmt_option_prompt(prompt, sizeof(prompt) - 1, TRUE, option);

		if ((p = tin_getline(prompt, FALSE, p, 1, FALSE, HIST_OTHER)) == NULL) {
			clear_message();
			return FALSE;
		}
		if (!*p)
			info_message(_(txt_info_enter_valid_character));
	} while (!*p);

	*variable = p[0];

	clear_message();
	return TRUE;
}


/*
 * Get a string. Make it the new default.
 * If none given, use the default.
 * Return the string or NULL if we can't get anything useful
 */
char *
prompt_string_default(
	const char *prompt,
	char *def,
	const char *failtext,
	int history)
{
	char pattern[LEN];

	clear_message();

	if (!prompt_string(prompt, pattern, history)) {
		clear_message();
		return NULL;
	}

	if (pattern[0] != '\0')			/* got a string - make it the default */
		my_strncpy(def, pattern, LEN);
	else {
		if (def[0] == '\0') {		/* no default - give up */
			error_message(failtext);
			return NULL;
		}
	}

	return def;					/* use the default */
}


/*
 * Get a message ID for the 'L' command. Add <> if needed
 * If the msgid exists and is reachable, return its index
 * in arts[], else ART_UNAVAILABLE
 */
int
prompt_msgid(
	void)
{
	char buf[LEN];

	if (prompt_string(_(txt_enter_message_id), buf + 1, HIST_MESSAGE_ID) && buf[1]) {
		char *ptr = str_trim(buf + 1);
		struct t_msgid *msgid;

		/*
		 * If the user failed to supply Message-ID in <>, add them
		 */
		if (buf[1] != '<') {
			buf[0] = '<';
			strcat(buf, ">");
			ptr = buf;
		}

		if ((msgid = find_msgid(ptr)) == NULL) {
			info_message(_(txt_art_unavailable));
			return ART_UNAVAILABLE;
		}

		/*
		 * Is it expired or otherwise not on the spool ?
		 */
		if (msgid->article == ART_UNAVAILABLE) {
			info_message(_(txt_art_unavailable));
			return ART_UNAVAILABLE;
		}

		/*
		 * If the article is no longer part of a thread, then there is
		 * no way to display it
		 */
		if (which_thread(msgid->article) == -1) {
			info_message(_(txt_no_last_message));
			return ART_UNAVAILABLE;
		}

		return msgid->article;
	}

	return ART_UNAVAILABLE;
}


/*
 * Format a message such that it'll fit within the screen width
 * Useful for fitting long Subjects and newsgroup names into prompts
 * result will contain a pointer to the malloced memory containing the
 * sized message
 */
char *
sized_message(
	char **result,
	const char *format,
	const char *subject)
{
	char *buf;
	int max_len;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wformat;
	size_t format_len;

	format_len = mbstowcs(NULL, format, 0);
	if (format_len != (size_t) (-1)) {
		wformat = my_malloc(sizeof(wchar_t) * (format_len + 1));
		mbstowcs(wformat, format, format_len + 1);
		wconvert_to_printable(wformat);
		/* The formatting info (%s) wastes 2 chars, but our prompt needs 1 char */
		max_len = cCOLS - wcswidth(wformat, format_len + 1) + 2 - 1;
		free(wformat);
	} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		max_len = cCOLS - strlen(format) + 2 - 1;	/* The formatting info (%s) wastes 2 chars, but our prompt needs 1 char */

	buf = my_malloc(strlen(subject) + 1);
	strunc(subject, buf, strlen(subject) + 1, max_len);

	*result = fmt_string(format, buf);
	free(buf);

	return *result;
}


/*
 * Implement the Single-Letter-Key mini menus at the bottom of the screen
 * eg, Press a)ppend, o)verwrite, q)uit :
 */
int
prompt_slk_response(
	int ch_default,
	const t_menukeys /* char */ *responses,
	const char *fmt,
	...)
{
	va_list ap;
	char ch;
	char buf[LEN];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);	/* We need to do this, else wait_message() will clobber us */
	va_end(ap);

	ch_default = map_to_local(ch_default, responses);
	do {
		wait_message(0, "%s%c", buf, ch_default);

		/* Get the cursor _just_ right */
		MoveCursor(cLINES, (int) strlen(buf));

		if ((ch = ReadCh()) == '\r' || ch == '\n')
			ch = ch_default;

		/*
		 * TODO: ignore special-keys which are represented as a
		 *       multibyte ESC-seq to avoid interpreting them as 'ESC' only
		 *       like it's done in the ugly code below.
		 */
#if 0
		if (ch == ESC) {
			switch (get_arrow_key(ch)) {
				case KEYMAP_UP:
				case KEYMAP_DOWN:
				case KEYMAP_LEFT:
				case KEYMAP_RIGHT:
				case KEYMAP_PAGE_DOWN:
				case KEYMAP_PAGE_UP:
				case KEYMAP_HOME:
				case KEYMAP_END:
					ch = '\0';

				default:
					break;
			}
		}
#endif /* 0 */

	} while (!strchr(responses->localkeys, ch));

	clear_message();
	return map_to_default(ch, responses);
}


/*
 * Wait until a key is pressed. We specify the <RETURN> key otherwise
 * pedants will point out that:
 * i)  There is no 'any' key on a keyboard
 * ii) CTRL, SHIFT etc don't work
 */
void
prompt_continue(
	void)
{
	int ch;

#ifdef USE_CURSES
	cmd_line = TRUE;
#endif /* USE_CURSES */
	info_message(_(txt_return_key));

	switch ((ch = ReadCh())) {
		case ESC:
#	ifdef HAVE_KEY_PREFIX
		case KEY_PREFIX:
#	endif /* HAVE_KEY_PREFIX */
			(void) get_arrow_key(ch);
			/* FALLTHROUGH */
		default:
			break;
	}

#ifdef USE_CURSES
	cmd_line = FALSE;
	my_retouch();
#endif /* USE_CURSES */
}
