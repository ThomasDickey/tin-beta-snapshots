/*
 *  Project   : tin - a Usenet reader
 *  Module    : prompt.c
 *  Author    : I. Lea
 *  Created   : 1991-04-01
 *  Updated   : 2025-06-10
 *  Notes     :
 *
 * Copyright (c) 1991-2025 Iain Lea <iain@bricbrac.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */


static char *prompt_slk_message;	/* prompt message for prompt_slk_redraw */
static char *prompt_yn_message;
static char *prompt_yn_choice;

/*
 * Local prototypes
 */
static int prompt_list(int row, int col, int *var, constext *help_text, constext *prompt_text, constext *list[], int size);
#ifdef HAVE_COLOR
	static void show_color_help(void);
	static void clear_color_help(void);
#endif /* HAVE_COLOR */

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
	if ((p = tin_getline(prompt, 1, tmp, 0, FALSE, HIST_OTHER)) != NULL) {
		STRCPY(tmp, p);
		num = strtol(tmp, NULL, 10);
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
	return prompt_default_string(prompt, buf, 0, NULL, which_hist);
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
	const char *default_prompt,
	int which_hist)
{
	char *p;

	clear_message();
	if ((p = tin_getline(prompt, 0, default_prompt, buf_len, FALSE, which_hist)) == NULL) {
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
	char **var)
{
	char *p;

	/*
	 * clear buffer - this is needed, otherwise a lost
	 * connection right before a resync_active() call
	 * would lead to a 'n' answer to the reconnect prompt
	 */
	/* fflush(stdin); */
	MoveCursor(line, 0);
	if ((p = tin_getline(prompt, 0, *var, 0, FALSE, HIST_OTHER)) == NULL)
		return FALSE;

	FreeIfNeeded(*var);
	*var = my_strdup(p);
	return TRUE;
}


/*
 * prompt_yn
 * prompt user for 'y'es or 'n'o decision. "prompt" will be displayed in the
 * last line giving the default answer "default_answer".
 * The function returns 1 if the user decided "yes", -1 if the user wanted
 * to escape, or 0 for any other decision.
 *
 * TODO: add nl_langinfo(YESEXPR) / nl_langinfo(NOEXPR), but use pcre
 *       instead of rpmatch()
 */
int
prompt_yn(
	const char *prompt,
	t_bool default_answer)
{
	char *keyprompt;
	char keyno[MAXKEYLEN], keyyes[MAXKEYLEN];
	int keyyes_len, keyno_len, maxlen, prompt_len;
	t_function func;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wint_t yes, no, prompt_ch, ch;
#else
	char yes, no, prompt_ch;
	int ch;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

/*	fflush(stdin); */		/* Prevent finger trouble from making important decisions */

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	yes = (wint_t) func_to_key(PROMPT_YES, prompt_keys);
	no = (wint_t) func_to_key(PROMPT_NO, prompt_keys);

	printascii(keyyes, (default_answer ? towupper(yes) : yes));
	printascii(keyno, (!default_answer ? towupper(no) : no));
#else
	yes = func_to_key(PROMPT_YES, prompt_keys);
	no = func_to_key(PROMPT_NO, prompt_keys);

	printascii(keyyes, (default_answer ? my_toupper((unsigned char) yes) : yes));
	printascii(keyno, (!default_answer ? my_toupper((unsigned char) no) : no));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	keyyes_len = strwidth(keyyes);
	keyno_len = strwidth(keyno);
	maxlen = MAX(keyyes_len, keyno_len);
	prompt_len = keyyes_len + keyno_len + maxlen + 6;
	prompt_yn_message = my_strdup(prompt);
	prompt_yn_choice = my_malloc(prompt_len + 1);

	input_context = cPromptYN;

	do {
		prompt_ch = (default_answer ? yes : no);
		keyprompt = (default_answer ? keyyes : keyno);

		snprintf(prompt_yn_choice, (size_t) prompt_len, " (%s/%s) %-*s", keyyes, keyno, maxlen, keyprompt);
		prompt_yn_redraw();

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (((ch = ReadWch()) == '\n') || (ch == '\r'))
#else
		if (((ch = (char) ReadCh()) == '\n') || (ch == '\r'))
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			ch = prompt_ch;

		switch (ch) {
			case ESC:	/* (ESC) common arrow keys */
#ifdef HAVE_KEY_PREFIX
			case KEY_PREFIX:
#endif /* HAVE_KEY_PREFIX */
				switch (get_arrow_key((int) ch)) {
					case KEYMAP_UP:
					case KEYMAP_DOWN:
						default_answer = bool_not(default_answer);
						ch = '\0';	/* set to a not bindable key to not leave the loop yet */
						break;

					case KEYMAP_LEFT:
						ch = ESC;
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
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		func = key_to_func((wchar_t) ch, prompt_keys);
#else
		func = key_to_func(ch, prompt_keys);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	} while (func == NOT_ASSIGNED);

	input_context = cNone;
	FreeAndNull(prompt_yn_message);
	FreeAndNull(prompt_yn_choice);

	if (!cmd_line) {
		clear_message();
		my_flush();
	}
	return (func == PROMPT_YES) ? 1 : (func == GLOBAL_ABORT) ? -1 : 0;
}


/*
 * (Re)draws and resize the prompt message for prompt_yn()
 */
void
prompt_yn_redraw(
	void)
{
	char *buf;
	int choice_len = strwidth(prompt_yn_choice);
	int message_len = strwidth(prompt_yn_message);

	if (!cmd_line) {
		MoveCursor(cLINES, 0);
		CleartoEOLN();
	}
	if (message_len + choice_len > cCOLS - 1) {
		buf = strunc(prompt_yn_message, cCOLS - choice_len - 1);
		message_len = strwidth(buf);
		my_printf("%s%s", buf, prompt_yn_choice);
		free(buf);
	} else
		my_printf("%s%s", prompt_yn_message, prompt_yn_choice);

	if (!cmd_line)
		cursoron();
	my_flush();
	if (!cmd_line)
		MoveCursor(cLINES, (message_len + choice_len) - 1);
}


#ifdef HAVE_COLOR
static void
show_color_help(
	void)
{
	char *buf;
	int i, row, col, head, tail, len, col_width = cCOLS / 6;
	int col_normal_orig = tinrc.col_normal;
	int col_back_orig = tinrc.col_back;

	clear_color_help();
	for (row = 2, col = 0; row >= 0; row--, col += 6) {
		MoveCursor(cLINES - row, 0);
		for (i = col; i <= col + 5 && txt_colors[i]; i++) {
			if (i == 0)
				fcol(-1);
			else if (i < 6)
				fcol(15);
			else
				fcol(0);
			bcol(i - 1);
			len = strwidth(_(txt_colors[i]));
			if (col_width > len) {
				head = (col_width - len) / 2;
				tail = col_width - len - head;
				if (tail > 1 || (head == 1 && tail == 1))
					/*
					 * we cannot use %-*s for color names, because names
					 * with multibyte chars would destroy the layout
					 */
					my_printf("%*c%s%*c", head, ' ', _(txt_colors[i]), tail, ' ');
				else
					my_printf("%s ", _(txt_colors[i]));
			} else {
				buf = strunc(_(txt_colors[i]), col_width);
				my_printf("%s", buf);
				free(buf);
			}
		}
		bcol(col_back_orig);
	}
	fcol(col_normal_orig);
}


static void
clear_color_help(
	void)
{
	MoveCursor(cLINES - 2, 0);
	CleartoEOS();
}
#endif /* HAVE_COLOR */


#ifdef HAVE_COLOR
#	define SET_OPT_COLOR() do { \
		if (var == &tinrc.col_invers_bg || var == &tinrc.col_invers_fg) { \
			int orig_var = *var; \
			*var = idx - adjust; \
			StartInverse(); \
			*var = orig_var; \
		} else if (var == &tinrc.col_back) \
			bcol(idx - adjust); \
		else \
			fcol(idx - adjust); \
	} while (0)

#	define RESET_OPT_COLOR() do { \
		if (var != &tinrc.col_back) \
			bcol(old_bcol); \
		if (var != &tinrc.col_normal) \
			fcol(old_fcol); \
		if (var == &tinrc.col_invers_bg || var == &tinrc.col_invers_fg) \
			EndInverse(); \
	} while (0)

#	define RESTORE_OPT_COLOR() do { \
		if (var == &tinrc.col_back) \
			bcol(old_bcol); \
		else if (var == &tinrc.col_normal) \
			fcol(old_fcol); \
		else if (var == &tinrc.col_invers_bg || var == &tinrc.col_invers_fg) \
			EndInverse(); \
	} while (0)
#endif /* HAVE_COLOR */
/*
 * help_text is displayed near the bottom of the screen.
 * var is an index into a list containing size elements.
 * The text from list is shown at row, col + len(prompt_text)
 * Choice is incremented using the space bar, wrapping to 0
 * ESC is used to abort any changes, RET saves changes.
 * The new value is returned.
 */
static int
prompt_list(
	int row,
	int col,
	int *var,
	constext *help_text,
	constext *prompt_text,
	constext *list[],
	int size)
{
	int ch, change, i, idx, offset, width = 0;
	int adjust = (strcasecmp(_(list[0]), _(txt_default)) == 0);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	char *buf;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
#ifdef HAVE_COLOR
	/* initialization to silence compiler warnings */
	t_bool color_opt = use_color && (list == txt_colors || list == txt_backcolors);
	int old_bcol = tinrc.col_back;
	int old_fcol = tinrc.col_normal;
#endif /* HAVE_COLOR */

	idx = *var + adjust;

	/*
	 * Find the length of longest printable text
	 */
	for (i = 0; i < size; i++)
		width = MAX(width, strwidth(_(list[i])));

#ifdef HAVE_COLOR
	if (color_opt) {
		(void) help_text;
		show_color_help();
	} else
#else
	show_menu_help(help_text);
#endif /* HAVE_COLOR */
	cursoron();

	offset = strwidth(_(prompt_text));

	/*
	 * Make sure to not exceed cCOLS
	 */
	if (offset + width >= cCOLS)
		width = cCOLS - offset - 1;

#ifdef HAVE_COLOR
	if (color_opt) {
		MoveCursor(row, col + offset);
		SET_OPT_COLOR();
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((buf = spart(_(list[idx]), width, TRUE)) != NULL) {
			my_printf("%s", buf);
			free(buf);
		} else
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			my_printf("%-*s", width, _(list[idx]));
		RESET_OPT_COLOR();
		my_flush();
	}
#	endif /* HAVE_COLOR */

	input_context = cPromptList;

	do {
		MoveCursor(row, col + offset);
		ch = (char) ReadCh();

#ifdef USE_CURSES
		if (pending_sactions && ch != EOF) {
/*			ungetch('\r'); */ /* reenter option, would differ from !USE_CURSES */
			ungetch(ESC);
		}
#endif /* USE_CURSES */

		/*
		 * change:
		 *   1 = move to the next list element
		 *   0 = do nothing
		 *  -1 = move to the previous list element
		 *
		 *  if an arrow key was pressed change ch to another value
		 *  otherwise we will exit the while loop
		 */
		switch (ch) {
			case ' ':
				change = 1;
				break;

			case ESC:	/* (ESC) common arrow keys */
#ifdef HAVE_KEY_PREFIX
			case KEY_PREFIX:
#endif /* HAVE_KEY_PREFIX */
				switch (get_arrow_key(ch)) {
					case KEYMAP_UP:
						change = -1;
						ch = ' ';
						break;

					case KEYMAP_DOWN:
						change = 1;
						ch = ' ';
						break;

					default:
						change = 0;
						break;
				}
				break;

			default:
				change = 0;
				break;
		}

		if (change && !pending_sactions) {
			/*
			 * increment or decrement list, loop around at the limits
			 */
			do {
				idx += change;
				if (idx < 0)
					idx = size - 1;
				else
					idx %= (size ? size : 1);
			} while (!*list[idx]); /* allow gaps in lists */

#	ifdef HAVE_COLOR
			if (color_opt)
				SET_OPT_COLOR();
#	endif /* HAVE_COLOR */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if ((buf = spart(_(list[idx]), width, TRUE)) != NULL) {
				my_printf("%s", buf);
				free(buf);
			} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
				my_printf("%-*s", width, _(list[idx]));
#	ifdef HAVE_COLOR
			if (color_opt)
				RESET_OPT_COLOR();
#	endif /* HAVE_COLOR */
			my_flush();
		}
	} while (ch != '\r' && ch != '\n' && ch != ESC && !pending_sactions);

	input_context = cNone;

	if (ch == ESC || pending_sactions) {
		pending_sactions = 0;
#	ifdef HAVE_COLOR
		if (color_opt)
			RESTORE_OPT_COLOR();
#	endif /* HAVE_COLOR */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((buf = spart(_(list[*var]), width, TRUE)) != NULL) {
			my_printf("%s", buf);
			free(buf);
		} else
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			my_printf("%-*s", width, _(list[*var]));
		my_flush();
	} else
		*var = idx - adjust;

#ifdef HAVE_COLOR
	if (color_opt)
		clear_color_help();
#endif /* HAVE_COLOR */

	cursoroff();
	return *var;
}
#ifdef HAVE_COLOR
#	undef SET_OPT_COLOR
#	undef RESET_OPT_COLOR
#	undef RESTORE_OPT_COLOR
#endif /* HAVE_COLOR */


/*
 * Special case of prompt_option_list() Toggle between ON and OFF
 * The function returns TRUE, if the value was changed, FALSE otherwise.
 */
t_bool
prompt_option_on_off(
	enum option_enum option)
{
	char prompt[LEN];
	t_bool *variable = OPT_ON_OFF_list[option_table[option].var_index];
	t_bool old_value = *variable;

	fmt_option_prompt(prompt, sizeof(prompt), TRUE, option);
	*variable = prompt_list(option_row(option), 0, (int *) variable, option_table[option].txt->help, prompt, txt_onoff, 2) ? TRUE : FALSE;
	return bool_not(bool_equal(*variable, old_value));
}


/*
 * The function returns TRUE, if the value was changed, FALSE otherwise.
 */
t_bool
prompt_option_list(
	enum option_enum option)
{
	char prompt[LEN];
	int *variable = option_table[option].variable;
	int old_value = *variable;
	int opt_count = 0;

	while (option_table[option].opt_list[opt_count] != NULL)
		++opt_count;
	fmt_option_prompt(prompt, sizeof(prompt), TRUE, option);
	*variable = prompt_list(option_row(option), 0, variable, option_table[option].txt->help, prompt, option_table[option].opt_list, opt_count);
	return *variable != old_value;
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
	enum option_enum option)
{
	char *variable;
	char *old_value;
	char prompt[LEN];
	t_bool is_same;

	if (*OPT_STRING_list[option_table[option].var_index])
		variable = my_strdup(*OPT_STRING_list[option_table[option].var_index]);
	else
		variable = my_strdup("");
	old_value = my_strdup(variable);
	show_menu_help(option_table[option].txt->help);
	fmt_option_prompt(prompt, sizeof(prompt) - 1, TRUE, option);
	if (prompt_menu_string(option_row(option), prompt, &variable)) {
		if (!option_is_default(option))
			FreeIfNeeded(*OPT_STRING_list[option_table[option].var_index]);
		*OPT_STRING_list[option_table[option].var_index] = my_strdup(variable);
		is_same = strcmp(old_value, variable) ? TRUE : FALSE;
		free(old_value);
		free(variable);
		return is_same;
	} else {
		free(old_value);
		free(variable);
		return FALSE;
	}
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
	enum option_enum option) /* return value is always ignored */
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
	num = strtol(number, NULL, 10);
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
	enum option_enum option) /* return value is always ignored */
{
	char prompt[LEN];
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t input[2];
	wchar_t *variable = OPT_CHAR_list[option_table[option].var_index];
	int max_chars = (int) sizeof(wchar_t) + 1;
	wchar_t *wp;
	char *p;
	char *curr_val;
#else
	char input[2];
	char *variable = OPT_CHAR_list[option_table[option].var_index];
	char *p;
	char *curr_val = &input[0];
	int max_chars = 1;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	input[0] = *variable;
	input[1] = '\0';

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	if ((curr_val = wchar_t2char(input))) {
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	do {
		show_menu_help(option_table[option].txt->help);
		MoveCursor(option_row(option), 0);
		fmt_option_prompt(prompt, sizeof(prompt) - 1, TRUE, option);

		if ((p = tin_getline(prompt, 0, curr_val, max_chars, FALSE, HIST_OTHER)) == NULL) {
			clear_message();
			return FALSE;
		}
		if (!*p)
			info_message(_(txt_info_enter_valid_character));
	} while (!*p);

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((wp = char2wchar_t(p))) {
			*variable = wp[0];
			free(wp);
		}
		free(curr_val);
	}
#else
	*variable = p[0];
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

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

	if (*pattern) {			/* got a string - make it the default */
		FreeIfNeeded(def);
		def = my_strdup(pattern);
	} else {
		if (!def || !*def) {		/* no default - give up */
			error_message(2, "%s", failtext);
			return NULL;
		}
	}

	return def;					/* use the default */
}


char *
prompt_string_ptr_default(
	const char *prompt,
	char **def,
	const char *failtext,
	int history)
{
	char pattern[LEN];

	clear_message();

	if (!prompt_string(prompt, pattern, history)) {
		clear_message();
		return NULL;
	}

	if (*pattern)	{		/* got a string - make it the default */
		FreeIfNeeded(*def);
		*def = my_strdup(pattern);
	} else {
		if (!*def || **def == '\0') {		/* no default - give up */
			error_message(2, "%s", failtext);
			return NULL;
		}
	}

	return *def;					/* use the default */
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
		/*
		 * TODO: Find a better solution, as we end up in the select
		 *       level after leaving the thread or page level with
		 *       show_only_unread_arts = TRUE.
		 *
		 *       Without !curr_group->attribute->show_only_unread_arts
		 *       read articles are not found if
		 *       show_only_unread_arts = TRUE.
		 */
		/* if (which_thread(msgid->article) == -1) { */
		if (!curr_group->attribute->show_only_unread_arts && curr_group->attribute->thread_articles && which_thread(msgid->article) == -1) {
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
	int max_len = cCOLS - strwidth(format) + 2 - 1;	/* The formatting info (%s) wastes 2 chars, but our prompt needs 1 char */

	buf = strunc(subject, max_len);

	*result = fmt_string(format, buf);
	free(buf);

	return *result;
}


/*
 * Implement the Single-Letter-Key mini menus at the bottom of the screen
 * eg, Press a)ppend, o)verwrite, q)uit :
 */
t_function
prompt_slk_response(
	t_function default_func,
	const struct keylist keys,
	const char *fmt,
	...)
{
	va_list ap;
	char *buf;
	int n, r;
	size_t blen;
	t_function func;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t ch;
#else
	char ch;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	va_start(ap, fmt);
	n = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	if (n < 0)
		return default_func;

	blen = (size_t) n + 1;
	buf = my_malloc(blen);

	va_start(ap, fmt);
	r = vsnprintf(buf, blen, fmt, ap);
	va_end(ap);

	if (r != n) {
		free(buf);
		return default_func;
	}

	blen += 2;
	prompt_slk_message = my_malloc(blen);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	{
		char *tmp;
		wchar_t wtmp[2] = { '\0', '\0' };

		wtmp[0] = func_to_key(default_func, keys);
		tmp = wchar_t2char(wtmp);
		snprintf(prompt_slk_message, blen, "%s%s", buf, tmp);
		FreeIfNeeded(tmp);
	}
#else
	snprintf(prompt_slk_message, blen, "%s%c", buf, func_to_key(default_func, keys));
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	free(buf);
	input_context = cPromptSLK;

	do {
		prompt_slk_redraw();		/* draw the prompt */

#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((ch = (wchar_t) ReadWch()) == '\r' || ch == '\n')
#else
		if ((ch = ReadCh()) == '\r' || ch == '\n')
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			func = default_func;
		else
			func = key_to_func(ch, keys);

#if 1
		/*
		 * ignore special-keys which are represented as a multibyte ESC-seq
		 * to avoid interpreting them as 'ESC' only
		 */
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
					ch = '\0'; /* <- do not remove this! */
					func = NOT_ASSIGNED;
					break;

				default:
					break;
			}
		}
#endif /* 1 */
	} while (func == NOT_ASSIGNED);

	input_context = cNone;
	FreeAndNull(prompt_slk_message);

	clear_message();
	return func;
}


/* (Re)draws the prompt message for prompt_slk_response() */
void
prompt_slk_redraw(
	void)
{
	int column;

	wait_message(0, "%s", prompt_slk_message);

	/* get the cursor _just_ right */
	column = strwidth(prompt_slk_message) - 1;
	MoveCursor(cLINES, column);
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
	enum context save_signal_context = signal_context;

	cmd_line = TRUE;
	info_message(_(txt_return_key));
	signal_context = cMain;
	input_context = cPromptCONT;

	switch ((ch = ReadCh())) {
		case ESC:
#ifdef HAVE_KEY_PREFIX
		case KEY_PREFIX:
#endif /* HAVE_KEY_PREFIX */
			(void) get_arrow_key(ch);
			break;

		default:
			break;
	}

	input_context = cNone;
	signal_context = save_signal_context;

#ifdef USE_CURSES
	my_fputc('\n', stdout);
#endif /* USE_CURSES */
	cmd_line = FALSE;
#ifdef USE_CURSES
	my_retouch();
#endif /* USE_CURSES */
}
