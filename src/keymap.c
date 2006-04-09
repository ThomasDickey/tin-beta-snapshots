/*
 *  Project   : tin - a Usenet reader
 *  Module    : keymap.c
 *  Author    : D. Nimmich, J. Faultless
 *  Created   : 2000-05-25
 *  Updated   : 2005-10-19
 *  Notes     : This file contains key mapping routines and variables.
 *
 * Copyright (c) 2000-2006 Dirk Nimmich <nimmich@muenster.de>
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
#ifndef VERSION_H
#	include "version.h"
#endif /* !VERSION_H */

/*
 * local prototypes
 */
static void add_default_key(struct keylist *key_list, const char *keys, t_function func);
static void add_global_keys(struct keylist *keys);
static void free_keylist(struct keylist *keys);
static void upgrade_keymap_file(char *old);
static t_bool process_keys(t_function func, const char *keys, struct keylist *kl);
static t_bool process_mapping(char *keyname, char *keys);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	static t_bool add_key(struct keylist *keys, const wchar_t key, t_function func, t_bool override);
#else
	static t_bool add_key(struct keylist *keys, const char key, t_function func, t_bool override);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

struct keylist feed_post_process_keys = { NULL, 0, 0 };
struct keylist feed_supersede_article_keys = { NULL, 0, 0 };
struct keylist feed_type_keys = { NULL, 0, 0 };
struct keylist filter_keys = { NULL, 0, 0 };
struct keylist group_keys = { NULL, 0, 0 };
struct keylist info_keys = { NULL, 0, 0 };
struct keylist mark_read_keys = { NULL, 0, 0 };
struct keylist option_menu_keys = { NULL, 0, 0 };
struct keylist page_keys = { NULL, 0, 0 };
#ifdef HAVE_PGP_GPG
	struct keylist pgp_mail_keys = { NULL, 0, 0 };
	struct keylist pgp_news_keys = { NULL, 0, 0 };
#endif /* HAVE_PGP_GPG */
struct keylist post_cancel_keys = { NULL, 0, 0 };
struct keylist post_continue_keys = { NULL, 0, 0 };
struct keylist post_delete_keys = { NULL, 0, 0 };
struct keylist post_edit_keys = { NULL, 0, 0 };
struct keylist post_edit_ext_keys = { NULL, 0, 0 };
struct keylist post_ignore_fupto_keys = { NULL, 0, 0 };
struct keylist post_mail_fup_keys = { NULL, 0, 0 };
struct keylist post_post_keys = { NULL, 0, 0 };
struct keylist post_postpone_keys = { NULL, 0, 0 };
struct keylist post_send_keys = { NULL, 0, 0 };
struct keylist prompt_keys = { NULL, 0, 0 };
struct keylist save_append_overwrite_keys = { NULL, 0, 0 };
struct keylist select_keys = { NULL, 0, 0 };
struct keylist thread_keys = { NULL, 0, 0 };


/*
 * lookup the associated function to the specified key
 */
t_function
key_to_func(
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	const wchar_t key,
#else
	const char key,
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	const struct keylist keys)
{
	size_t i;

	for (i = 0; i < keys.used; i++) {
		if (keys.list[i].key == key)
			return keys.list[i].function;
	}

	return NOT_ASSIGNED;
}


/*
 * lookup the associated key to the specified function
 */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
wchar_t
#else
char
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
func_to_key(
	t_function func,
	const struct keylist keys)
{
	size_t i;

	for (i = 0; i < keys.used; i++) {
		if (keys.list[i].function == func)
			return keys.list[i].key;
	}
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	return (wchar_t) '?';
#else
	return '?';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
}


/*
 * adds a key to a keylist
 * default_key: TRUE if a default key should be added
 * returns TRUE if the key was succesfully added else FALSE
 */
static t_bool
add_key(
	struct keylist *keys,
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	const wchar_t key,
#else
	const char key,
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	t_function func,
	t_bool override)
{
	size_t i;
	struct keynode *entry = NULL;

	/* is a function already associated with this key */
	for (i = 0; i < keys->used; i++) {
		if (keys->list[i].key == key)
			entry = &keys->list[i];
	}

	if (entry != NULL) {
		if (override) {
			entry->function = func;
			return TRUE;
		} else
			return FALSE;
	} else {
		/* add a new entry */
		if (keys->used >= keys->max) {
			if (keys->list == NULL) {
				keys->max = 1;
				keys->list = my_malloc(keys->max * sizeof(struct keynode));
			} else {
				keys->max++;
				keys->list = my_realloc(keys->list, keys->max * sizeof(struct keynode));
			}
		}
		keys->list[keys->used].key = key;
		keys->list[keys->used].function = func;
		keys->used++;

		return TRUE;
	}
}


/*
 * FIXME:
 * as long as we use only ASCII for default keys no need to change 'keys' to wchar_t
 */
static void
add_default_key(
	struct keylist *key_list,
	const char *keys,
	t_function func)
{
	const char *key = keys;
	/* check if the function has already a key assigned before we add the default one */
	if (func_to_key(func, *key_list) != '?')
		return;

	for (; *key != '\0'; key++)
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		add_key(key_list, (wchar_t) *key, func, FALSE);
#else
		add_key(key_list, *key, func, FALSE);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
}


static void
free_keylist(struct keylist *keys) {
	FreeAndNull(keys->list);
	keys->used = keys->max = 0;
}


/*
 * Free all memory for keymaps.
 */
void
free_keymaps(
	void)
{
	free_keylist(&select_keys);
	free_keylist(&group_keys);
	free_keylist(&thread_keys);
	free_keylist(&option_menu_keys);
	free_keylist(&page_keys);
	free_keylist(&info_keys);
	free_keylist(&post_send_keys);
	free_keylist(&post_edit_keys);
	free_keylist(&post_edit_ext_keys);
	free_keylist(&post_post_keys);
	free_keylist(&post_postpone_keys);
	free_keylist(&post_mail_fup_keys);
	free_keylist(&post_ignore_fupto_keys);
	free_keylist(&post_continue_keys);
	free_keylist(&post_delete_keys);
	free_keylist(&post_cancel_keys);
	free_keylist(&filter_keys);
	free_keylist(&mark_read_keys);
#ifdef HAVE_PGP_GPG
	free_keylist(&pgp_mail_keys);
	free_keylist(&pgp_news_keys);
#endif /* HAVE_PGP_GPG */
	free_keylist(&save_append_overwrite_keys);
	free_keylist(&feed_type_keys);
	free_keylist(&feed_post_process_keys);
	free_keylist(&feed_supersede_article_keys);
	free_keylist(&prompt_keys);
}


/*
 * Render ch in human readable ASCII
 * Is there no lib function to do this ?
 * *buf must have a size of at least MAXKEYLEN
 */
char *
printascii(
	char *buf,
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wint_t ch)
#else
	int ch)
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
{
	if (ch == 0)
		strcpy(buf, _("NULL"));
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	else if (iswgraph(ch)) {	/* Regular printables */
		int i = wctomb(buf, ch);

		if (i > 0)
			buf[i] = '\0';
		else
			buf[0] = '\0';
	}
#else
	else if (isgraph(ch)) {		/* Regular printables */
		buf[0] = ch;
		buf[1] = '\0';
	}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	else if (ch == '\t') {	/* TAB */
		strcpy(buf, _(txt_tab));
	} else if ((ch == '\n') || (ch == '\r')) {	/* LF, CR */
		strcpy(buf, _(txt_cr));
	} else if (ch == ESC) {		/* Escape */
		strcpy(buf, _(txt_esc));
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	} else if (iswcntrl(ch)) {	/* Control keys */
#else
	} else if (iscntrl(ch)) {	/* Control keys */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		buf[0] = '^';
		buf[1] = ((int) ch & 0xFF) + '@';
		buf[2] = '\0';
	} else if (ch == ' ')		/* SPACE */
		strcpy(buf, _(txt_space));
	else
		strcpy(buf, "???");	/* Never happens? */

	return buf;
}


#define KEYSEPS		" \t\n"
/*
 * read the keymap file
 * returns TRUE if the keymap file was read without an error else FALSE
 */
t_bool
read_keymap_file(
	void)
{
	FILE *fp = (FILE *) 0;
	char *line, *keydef, *kname;
	char *map, *ptr;
	char buf[LEN], buff[NAME_LEN + 1];
	int upgrade = RC_CHECK;
	t_bool ret = TRUE;

	/*
	 * checks TIN_HOMEDIR/HOME/TIN_DEFAULTS_DIR
	 * for KEYMAP_FILE."locale" or KEYMAP_FILE
	 *
	 * locale is first match from LC_ALL, LC_CTYPE, LC_MESSAGES, LANG
	 *
	 * TODO: fix possible buf-overflows
	 */
	/* get locale suffix */
	map = my_strdup(get_val("LC_ALL", get_val("LC_CTYPE", get_val("LC_MESSAGES", get_val("LANG", "")))));
	if (strlen(map)) {
		if ((ptr = strchr(map, '.')))
			*ptr = '\0';
		snprintf(buff, sizeof(buff), "%s.%s", KEYMAP_FILE, map);
		joinpath(buf, rcdir, buff);
		fp = fopen(buf, "r");
	}
	if (!fp) {
		joinpath(buf, rcdir, KEYMAP_FILE);
		fp = fopen(buf, "r");
	}
#ifdef TIN_DEFAULTS_DIR
	if (strlen(map) && !fp) {
		joinpath(buf, TIN_DEFAULTS_DIR, buff);
		fp = fopen(buf, "r");
	}
	if (!fp) {
		joinpath(buf, TIN_DEFAULTS_DIR, KEYMAP_FILE);
		fp = fopen(buf, "r");
	}
#endif /* TIN_DEFAULTS_DIR */

	FreeIfNeeded(map);

	if (!fp)
		return TRUE; /* no keymap file is not an error */

	map = my_strdup(buf); /* remember keymap-name */

	/* check if keymap file is uptodate */
	while ((line = fgets(buf, sizeof(buf), fp)) != NULL) {
		if (line[0] == '#' && upgrade == RC_CHECK) {
			if ((upgrade = check_upgrade(line, "# Keymap file V", KEYMAP_VERSION)) == RC_UPGRADE) {
				fclose(fp);
				upgrade_keymap_file(map);
				upgrade = RC_IGNORE;
				fp = fopen(map, "r");
			}
			break;
		}
	}

	free_keymaps();
	while ((line = fgets(buf, sizeof(buf), fp)) != NULL) {
		/*
		 * Ignore blank and comment lines
		 */
		if (line[0] == '#' || line[0] == '\n')
			continue;

		if ((kname = strsep(&line, KEYSEPS)) != NULL) {
			keydef = str_trim(line);
			/*
			 * Warn about basic syntax errors
			 */
			if (keydef == NULL || !strlen(keydef)) {
				wait_message(0, _(txt_keymap_missing_key), kname);
				ret = FALSE;
				continue;
			}
		} else
			continue;

		/*
		 * TODO: useful? shared keymaps (NFS-Home) may differ
		 * depending on the OS (i.e. on tin has color the other has not)
		 */
		if (!process_mapping(kname, keydef)) {
			wait_message(0, _(txt_keymap_invalid_name), kname);
			prompt_continue();
			ret = FALSE;
			continue;
		}

	}
	fclose(fp);
	setup_default_keys();
	if (upgrade != RC_IGNORE)
		upgrade_prompt_quit(upgrade, map);

	free(map);
	return ret;
}


/*
 * associate the keys with the internal function and add them to the keylist
 * returns TRUE if all keys could be recognized else FALSE
 */
static t_bool
process_keys(
	t_function func,
	const char *keys,
	struct keylist *kl)
{
	char *keydef, *tmp;
	t_bool error, ret = TRUE;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	wchar_t *wkeydef;
	wchar_t key;
#else
	char key;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	tmp = my_strdup(keys);		/* don't change "keys" */
	keydef = strtok(tmp, KEYSEPS);

	while (keydef != NULL) {
		error = FALSE;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((wkeydef = char2wchar_t(keydef)) == NULL) {
			wait_message(1, _(txt_invalid_multibyte_sequence));
			ret = FALSE;

			keydef = strtok(NULL, KEYSEPS);
			continue;
		}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

		/*
		 * Parse the key sequence into 'key'
		 * Special sequences are:
		 * ^A -> control chars
		 * TAB -> ^I
		 * SPACE -> ' '
		 */
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if (wcslen(wkeydef) > 1) {
			switch (wkeydef[0])	/* Only test 1st char - crude but effective */
#else
		if (strlen(keydef) > 1) {
			switch (keydef[0])	/* Only test 1st char - crude but effective */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			{
				case 'N':
					key = '\0';
					break;

				case 'S':
					key = ' ';
					break;

				case 'T':
					key = ctrl('I');
					break;

				case '^':
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
					/* allow only ^A to ^Z */
					if (wkeydef[1] >= 'A' && wkeydef[1] <= 'Z') {
						key = ctrl(wkeydef[1]);
						break;
					}
#else
					if (isupper((int)(unsigned char) keydef[1])) {
						key = ctrl(keydef[1]);
						break;
					}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
					/* FALLTHROUGH */
				default:
					wait_message(0, _(txt_keymap_invalid_key), keydef);
					ret = FALSE;
					error = TRUE;
					break;
			}
		} else {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if (iswdigit(key = wkeydef[0]))
#else
			if (isdigit(key = keydef[0]))
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			{
				wait_message(0, _(txt_keymap_invalid_key), keydef);
				ret = FALSE;
				error = TRUE;
			}
		}

		if (!error)
			add_key(kl, key, func, TRUE);

		keydef = strtok(NULL, KEYSEPS);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		FreeIfNeeded(wkeydef);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	}
	free(tmp);

	return ret;
}


/*
 * map a keyname to the internal function name and assign the keys
 * returns TRUE if a mapping was found else FALSE
 */
static t_bool
process_mapping(
	char *keyname,				/* Keyname we're searching for */
	char *keys)				/* Key to assign to keyname if found */
{
	switch (keyname[0]) {
		case 'B':
			if (strcmp(keyname, "BugReport") == 0) {
				process_keys(GLOBAL_BUGREPORT, keys, &group_keys);
				process_keys(GLOBAL_BUGREPORT, keys, &select_keys);
				process_keys(GLOBAL_BUGREPORT, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'C':
			if (strcmp(keyname, "Catchup") == 0) {
				process_keys(CATCHUP, keys, &group_keys);
				process_keys(CATCHUP, keys, &page_keys);
				process_keys(CATCHUP, keys, &select_keys);
				process_keys(CATCHUP, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "CatchupNextUnread") == 0) {
				process_keys(CATCHUP_NEXT_UNREAD, keys, &group_keys);
				process_keys(CATCHUP_NEXT_UNREAD, keys, &page_keys);
				process_keys(CATCHUP_NEXT_UNREAD, keys, &select_keys);
				process_keys(CATCHUP_NEXT_UNREAD, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ConfigFirstPage") == 0) {
				process_keys(GLOBAL_FIRST_PAGE, keys, &option_menu_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ConfigLastPage") == 0) {
				process_keys(GLOBAL_LAST_PAGE, keys, &option_menu_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ConfigNoSave") == 0) {
				process_keys(CONFIG_NO_SAVE, keys, &option_menu_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ConfigSelect") == 0) {
				process_keys(CONFIG_SELECT, keys, &option_menu_keys);

				return TRUE;
			}
			break;

		case 'D':
			if (strcmp(keyname, "DisplayPostHist") == 0) {
				process_keys(GLOBAL_DISPLAY_POST_HISTORY, keys, &group_keys);
				process_keys(GLOBAL_DISPLAY_POST_HISTORY, keys, &page_keys);
				process_keys(GLOBAL_DISPLAY_POST_HISTORY, keys, &select_keys);
				process_keys(GLOBAL_DISPLAY_POST_HISTORY, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "Down") == 0) {
				process_keys(GLOBAL_LINE_DOWN, keys, &group_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &info_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &option_menu_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &page_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &select_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'E':
			if (strcmp(keyname, "EditFilter") == 0) {
				process_keys(GLOBAL_EDIT_FILTER, keys, &group_keys);
				process_keys(GLOBAL_EDIT_FILTER, keys, &page_keys);
				process_keys(GLOBAL_EDIT_FILTER, keys, &select_keys);
				process_keys(GLOBAL_EDIT_FILTER, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'F':
			if (strcmp(keyname, "FeedArt") == 0) {
				process_keys(FEED_ARTICLE, keys, &feed_type_keys);

				return TRUE;
			}
			if (strcmp(keyname, "FeedHot") == 0) {
				process_keys(FEED_HOT, keys, &feed_type_keys);

				return TRUE;
			}
			if (strcmp(keyname, "FeedPat") == 0) {
				process_keys(FEED_PATTERN, keys, &feed_type_keys);

				return TRUE;
			}
			if (strcmp(keyname, "FeedRepost") == 0) {
				process_keys(FEED_KEY_REPOST, keys, &feed_supersede_article_keys);

				return TRUE;
			}
			if (strcmp(keyname, "FeedSupersede") == 0) {
				process_keys(FEED_SUPERSEDE, keys, &feed_supersede_article_keys);

				return TRUE;
			}
			if (strcmp(keyname, "FeedTag") == 0) {
				process_keys(FEED_TAGGED, keys, &feed_type_keys);

				return TRUE;
			}
			if (strcmp(keyname, "FeedThd") == 0) {
				process_keys(FEED_THREAD, keys, &feed_type_keys);

				return TRUE;
			}
			if (strcmp(keyname, "FilterEdit") == 0) {
				process_keys(FILTER_EDIT, keys, &filter_keys);

				return TRUE;
			}
			if (strcmp(keyname, "FilterSave") == 0) {
				process_keys(FILTER_SAVE, keys, &filter_keys);

				return TRUE;
			}
			if (strcmp(keyname, "FirstPage") == 0) {
				process_keys(GLOBAL_FIRST_PAGE, keys, &group_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &info_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &option_menu_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &page_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &select_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'G':
			if (strcmp(keyname, "GroupAutoSave") == 0) {
				process_keys(GROUP_AUTOSAVE, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupDoAutoSel") == 0) {
				process_keys(GROUP_DO_AUTOSELECT, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupGoto") == 0) {
				process_keys(GROUP_GOTO, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupListThd") == 0) {
				process_keys(GROUP_LIST_THREAD, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupMail") == 0) {
				process_keys(GROUP_MAIL, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupMarkThdRead") == 0) {
				process_keys(GROUP_MARK_THREAD_READ, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupMarkUnselArtRead") == 0) {
				process_keys(GROUP_MARK_UNSELECTED_ARTICLES_READ, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupNextGroup") == 0) {
				process_keys(GROUP_NEXT_GROUP, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupNextUnreadArt") == 0) {
				process_keys(GROUP_NEXT_UNREAD_ARTICLE, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupNextUnreadArtOrGrp") == 0) {
				process_keys(GROUP_NEXT_UNREAD_ARTICLE_OR_GROUP, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupPrevGroup") == 0) {
				process_keys(GROUP_PREVIOUS_GROUP, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupPrevUnreadArt") == 0) {
				process_keys(GROUP_PREVIOUS_UNREAD_ARTICLE, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupReadBasenote") == 0) {
				process_keys(GROUP_READ_BASENOTE, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupRepost") == 0) {
				process_keys(GROUP_REPOST, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupReverseSel") == 0) {
				process_keys(GROUP_REVERSE_SELECTIONS, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupSave") == 0) {
				process_keys(GROUP_SAVE, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupSelPattern") == 0) {
				process_keys(GROUP_SELECT_PATTERN, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupSelThd") == 0) {
				process_keys(GROUP_SELECT_THREAD, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupSelThdIfUnreadSelected") == 0) {
				process_keys(GROUP_SELECT_THREAD_IF_UNREAD_SELECTED, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupTag") == 0) {
				process_keys(GROUP_TAG, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupTagParts") == 0) {
				process_keys(GROUP_TAG_PARTS, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupToggleGetartLimit") == 0) {
				process_keys(GROUP_TOGGLE_GET_ARTICLES_LIMIT, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupToggleReadUnread") == 0) {
				process_keys(GROUP_TOGGLE_READ_UNREAD, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupToggleSubjDisplay") == 0) {
				process_keys(GROUP_TOGGLE_SUBJECT_DISPLAY, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupToggleThdSel") == 0) {
				process_keys(GROUP_TOGGLE_SELECT_THREAD, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupToggleThreading") == 0) {
				process_keys(GROUP_TOGGLE_THREADING, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupUndoSel") == 0) {
				process_keys(GROUP_UNDO_SELECTIONS, keys, &group_keys);

				return TRUE;
			}
			if (strcmp(keyname, "GroupUntag") == 0) {
				process_keys(GROUP_UNTAG, keys, &group_keys);

				return TRUE;
			}
			break;

		case 'H':
			if (strcmp(keyname, "Help") == 0) {
				process_keys(GLOBAL_HELP, keys, &group_keys);
				process_keys(GLOBAL_HELP, keys, &page_keys);
				process_keys(GLOBAL_HELP, keys, &select_keys);
				process_keys(GLOBAL_HELP, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "HelpFirstPage") == 0) {
				process_keys(GLOBAL_FIRST_PAGE, keys, &info_keys);

				return TRUE;
			}
			if (strcmp(keyname, "HelpLastPage") == 0) {
				process_keys(GLOBAL_LAST_PAGE, keys, &info_keys);

				return TRUE;
			}
			break;

		case 'L':
			if (strcmp(keyname, "LastPage") == 0) {
				process_keys(GLOBAL_LAST_PAGE, keys, &group_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &info_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &option_menu_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &page_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &select_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "LastViewed") == 0) {
				process_keys(GLOBAL_LAST_VIEWED, keys, &group_keys);
				process_keys(GLOBAL_LAST_VIEWED, keys, &page_keys);
				process_keys(GLOBAL_LAST_VIEWED, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "LookupMessage") == 0) {
				process_keys(GLOBAL_LOOKUP_MESSAGEID, keys, &group_keys);
				process_keys(GLOBAL_LOOKUP_MESSAGEID, keys, &page_keys);
				process_keys(GLOBAL_LOOKUP_MESSAGEID, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'M':
			if (strcmp(keyname, "MarkReadCur") == 0) {
				process_keys(MARK_READ_CURRENT, keys, &mark_read_keys);

				return TRUE;
			}
			if (strcmp(keyname, "MarkReadTag") == 0) {
				process_keys(MARK_READ_TAGGED, keys, &mark_read_keys);

				return TRUE;
			}
			if (strcmp(keyname, "MarkArticleUnread") == 0) {
				process_keys(MARK_ARTICLE_UNREAD, keys, &group_keys);
				process_keys(MARK_ARTICLE_UNREAD, keys, &page_keys);
				process_keys(MARK_ARTICLE_UNREAD, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "MarkThreadUnread") == 0) {
				process_keys(MARK_THREAD_UNREAD, keys, &group_keys);
				process_keys(MARK_THREAD_UNREAD, keys, &page_keys);
				process_keys(MARK_THREAD_UNREAD, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "MenuFilterKill") == 0) {
				process_keys(GLOBAL_MENU_FILTER_KILL, keys, &group_keys);
				process_keys(GLOBAL_MENU_FILTER_KILL, keys, &page_keys);
				process_keys(GLOBAL_MENU_FILTER_KILL, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "MenuFilterSelect") == 0) {
				process_keys(GLOBAL_MENU_FILTER_SELECT, keys, &group_keys);
				process_keys(GLOBAL_MENU_FILTER_SELECT, keys, &page_keys);
				process_keys(GLOBAL_MENU_FILTER_SELECT, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'O':
			if (strcmp(keyname, "OptionMenu") == 0) {
				process_keys(GLOBAL_OPTION_MENU, keys, &group_keys);
				process_keys(GLOBAL_OPTION_MENU, keys, &page_keys);
				process_keys(GLOBAL_OPTION_MENU, keys, &post_edit_ext_keys);
				process_keys(GLOBAL_OPTION_MENU, keys, &post_post_keys);
				process_keys(GLOBAL_OPTION_MENU, keys, &select_keys);
				process_keys(GLOBAL_OPTION_MENU, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'P':
			if (strcmp(keyname, "PageAutoSave") == 0) {
				process_keys(PAGE_AUTOSAVE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageBotThd") == 0) {
				process_keys(PAGE_BOTTOM_THREAD, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageCancel") == 0) {
				process_keys(PAGE_CANCEL, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageDown") == 0) {
				process_keys(GLOBAL_PAGE_DOWN, keys, &group_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &info_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &option_menu_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &page_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &select_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageDown3") == 0) {
				process_keys(GLOBAL_PAGE_DOWN, keys, &group_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &info_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &option_menu_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &select_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &thread_keys);
				process_keys(PAGE_PAGE_DOWN3, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageEditArticle") == 0) {
				process_keys(PAGE_EDIT_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageFirstPage") == 0) {
				process_keys(GLOBAL_FIRST_PAGE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageFollowup") == 0) {
				process_keys(PAGE_FOLLOWUP, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageFollowupQuote") == 0) {
				process_keys(PAGE_FOLLOWUP_QUOTE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageFollowupQuoteHeaders") == 0) {
				process_keys(PAGE_FOLLOWUP_QUOTE_HEADERS, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageGotoParent") == 0) {
				process_keys(PAGE_GOTO_PARENT, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageGroupSel") == 0) {
				process_keys(PAGE_GROUP_SELECT, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageLastPage") == 0) {
				process_keys(GLOBAL_LAST_PAGE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageListThd") == 0) {
				process_keys(PAGE_LIST_THREAD, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageKillThd") == 0) {
				process_keys(PAGE_MARK_THREAD_READ, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageMail") == 0) {
				process_keys(PAGE_MAIL, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageNextArt") == 0) {
				process_keys(PAGE_NEXT_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageNextThd") == 0) {
				process_keys(PAGE_NEXT_THREAD, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageNextUnread") == 0) {
				process_keys(PAGE_NEXT_UNREAD, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageNextUnreadArt") == 0) {
				process_keys(PAGE_NEXT_UNREAD_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PagePGPCheckArticle") == 0) {
#ifdef HAVE_PGP_GPG
				process_keys(PAGE_PGP_CHECK_ARTICLE, keys, &page_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (strcmp(keyname, "PagePrevArt") == 0) {
				process_keys(PAGE_PREVIOUS_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PagePrevUnreadArt") == 0) {
				process_keys(PAGE_PREVIOUS_UNREAD_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageReply") == 0) {
				process_keys(PAGE_REPLY, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageReplyQuote") == 0) {
				process_keys(PAGE_REPLY_QUOTE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageReplyQuoteHeaders") == 0) {
				process_keys(PAGE_REPLY_QUOTE_HEADERS, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageRepost") == 0) {
				process_keys(PAGE_REPOST, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageReveal") == 0) {
				process_keys(PAGE_REVEAL, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageSave") == 0) {
				process_keys(PAGE_SAVE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageSkipIncludedText") == 0) {
				process_keys(PAGE_SKIP_INCLUDED_TEXT, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageTag") == 0) {
				process_keys(PAGE_TAG, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageTopThd") == 0) {
				process_keys(PAGE_TOP_THREAD, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageToggleHeaders") == 0) {
				process_keys(PAGE_TOGGLE_HEADERS, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageToggleHighlight") == 0) {
				process_keys(PAGE_TOGGLE_HIGHLIGHTING, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageToggleRot") == 0) {
				process_keys(PAGE_TOGGLE_ROT13, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageToggleTabs") == 0) {
				process_keys(PAGE_TOGGLE_TABS, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageToggleTex2iso") == 0) {
				process_keys(PAGE_TOGGLE_TEX2ISO, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageToggleUue") == 0) {
				process_keys(PAGE_TOGGLE_UUE, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageUp") == 0) {
				process_keys(GLOBAL_PAGE_UP, keys, &group_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &info_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &option_menu_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &page_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &select_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageViewAttach") == 0) {
				process_keys(PAGE_VIEW_ATTACHMENTS, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PageViewUrl") == 0) {
				process_keys(PAGE_VIEW_URL, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PgpEncrypt") == 0) {
#ifdef HAVE_PGP_GPG
				process_keys(PGP_KEY_ENCRYPT, keys, &pgp_mail_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (strcmp(keyname, "PgpEncSign") == 0) {
#ifdef HAVE_PGP_GPG
				process_keys(PGP_KEY_ENCRYPT_SIGN, keys, &pgp_mail_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (strcmp(keyname, "PgpIncludekey") == 0) {
#ifdef HAVE_PGP_GPG
				process_keys(PGP_INCLUDE_KEY, keys, &pgp_news_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (strcmp(keyname, "PgpSign") == 0) {
#ifdef HAVE_PGP_GPG
				process_keys(PGP_KEY_SIGN, keys, &pgp_news_keys);
				process_keys(PGP_KEY_SIGN, keys, &pgp_mail_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (strcmp(keyname, "Pipe") == 0) {
				process_keys(GLOBAL_PIPE, keys, &group_keys);
				process_keys(GLOBAL_PIPE, keys, &page_keys);
				process_keys(GLOBAL_PIPE, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "Post") == 0) {
				process_keys(GLOBAL_POST, keys, &group_keys);
				process_keys(GLOBAL_POST, keys, &page_keys);
				process_keys(GLOBAL_POST, keys, &post_ignore_fupto_keys);
				process_keys(GLOBAL_POST, keys, &post_mail_fup_keys);
				process_keys(GLOBAL_POST, keys, &post_post_keys);
				process_keys(GLOBAL_POST, keys, &select_keys);
				process_keys(GLOBAL_POST, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostAbort") == 0) {
				process_keys(POST_ABORT, keys, &post_continue_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostCancel") == 0) {
				process_keys(POST_CANCEL, keys, &post_cancel_keys);
				process_keys(POST_CANCEL, keys, &post_delete_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostContinue") == 0) {
				process_keys(POST_CONTINUE, keys, &post_continue_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostEdit") == 0) {
				process_keys(POST_EDIT, keys, &post_cancel_keys);
				process_keys(POST_EDIT, keys, &post_edit_keys);
				process_keys(POST_EDIT, keys, &post_edit_ext_keys);
				process_keys(POST_EDIT, keys, &post_post_keys);
				process_keys(POST_EDIT, keys, &post_send_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostIgnore") == 0) {
				process_keys(POST_IGNORE_FUPTO, keys, &post_ignore_fupto_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostIspell") == 0) {
#ifdef HAVE_ISPELL
				process_keys(POST_ISPELL, keys, &post_post_keys);
				process_keys(POST_ISPELL, keys, &post_send_keys);
#endif /* HAVE_ISPELL */

				return TRUE;
			}
			if (strcmp(keyname, "PostMail") == 0) {
				process_keys(POST_MAIL, keys, &post_mail_fup_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostPGP") == 0) {
#ifdef HAVE_PGP_GPG
				process_keys(POST_PGP, keys, &post_post_keys);
				process_keys(POST_PGP, keys, &post_send_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (strcmp(keyname, "PostponeAll") == 0) {
				process_keys(POSTPONE_ALL, keys, &post_postpone_keys);

				return TRUE;
			}
			if (strcmp(keyname, "Postponed") == 0) {
				process_keys(GLOBAL_POSTPONED, keys, &group_keys);
				process_keys(GLOBAL_POSTPONED, keys, &page_keys);
				process_keys(GLOBAL_POSTPONED, keys, &select_keys);
				process_keys(GLOBAL_POSTPONED, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostponeOverride") == 0) {
				process_keys(POSTPONE_OVERRIDE, keys, &post_postpone_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostPost") == 0) {
				process_keys(GLOBAL_POST, keys, &post_ignore_fupto_keys);
				process_keys(GLOBAL_POST, keys, &post_mail_fup_keys);
				process_keys(GLOBAL_POST, keys, &post_post_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostPostpone") == 0) {
				process_keys(POST_POSTPONE, keys, &post_edit_keys);
				process_keys(POST_POSTPONE, keys, &post_post_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostSend") == 0) {
				process_keys(POST_SEND, keys, &post_send_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PostSupersede") == 0) {
				process_keys(POST_SUPERSEDE, keys, &post_delete_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PProcNo") == 0) {
				process_keys(POSTPROCESS_NO, keys, &feed_post_process_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PProcShar") == 0) {
				process_keys(POSTPROCESS_SHAR, keys, &feed_post_process_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PProcYes") == 0) {
				process_keys(POSTPROCESS_YES, keys, &feed_post_process_keys);

				return TRUE;
			}
			if (strcmp(keyname, "Print") == 0) {
#ifndef DISABLE_PRINTING
				process_keys(GLOBAL_PRINT, keys, &group_keys);
				process_keys(GLOBAL_PRINT, keys, &page_keys);
				process_keys(GLOBAL_PRINT, keys, &thread_keys);
#endif /* !DISABLE_PRINTING */

				return TRUE;
			}
			if (strcmp(keyname, "PromptNo") == 0) {
				process_keys(PROMPT_NO, keys, &post_postpone_keys);
				process_keys(PROMPT_NO, keys, &prompt_keys);

				return TRUE;
			}
			if (strcmp(keyname, "PromptYes") == 0) {
				process_keys(PROMPT_YES, keys, &post_postpone_keys);
				process_keys(PROMPT_YES, keys, &prompt_keys);

				return TRUE;
			}
			break;

		case 'Q':
			if (strcmp(keyname, "QuickFilterKill") == 0) {
				process_keys(GLOBAL_QUICK_FILTER_KILL, keys, &group_keys);
				process_keys(GLOBAL_QUICK_FILTER_KILL, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "QuickFilterSelect") == 0) {
				process_keys(GLOBAL_QUICK_FILTER_SELECT, keys, &group_keys);
				process_keys(GLOBAL_QUICK_FILTER_SELECT, keys, &page_keys);

				return TRUE;
			}
			if (strcmp(keyname, "Quit") == 0) {
				process_keys(GLOBAL_QUIT, keys, &feed_post_process_keys);
				process_keys(GLOBAL_QUIT, keys, &feed_supersede_article_keys);
				process_keys(GLOBAL_QUIT, keys, &feed_type_keys);
				process_keys(GLOBAL_QUIT, keys, &filter_keys);
				process_keys(GLOBAL_QUIT, keys, &group_keys);
				process_keys(GLOBAL_QUIT, keys, &info_keys);
				process_keys(GLOBAL_QUIT, keys, &mark_read_keys);
				process_keys(GLOBAL_QUIT, keys, &option_menu_keys);
				process_keys(GLOBAL_QUIT, keys, &page_keys);
#ifdef HAVE_PGP_GPG
				process_keys(GLOBAL_QUIT, keys, &pgp_mail_keys);
				process_keys(GLOBAL_QUIT, keys, &pgp_news_keys);
#endif /* HAVE_PGP_GPG */
				process_keys(GLOBAL_QUIT, keys, &post_cancel_keys);
				process_keys(GLOBAL_QUIT, keys, &post_continue_keys);
				process_keys(GLOBAL_QUIT, keys, &post_delete_keys);
				process_keys(GLOBAL_QUIT, keys, &post_edit_keys);
				process_keys(GLOBAL_QUIT, keys, &post_edit_ext_keys);
				process_keys(GLOBAL_QUIT, keys, &post_ignore_fupto_keys);
				process_keys(GLOBAL_QUIT, keys, &post_mail_fup_keys);
				process_keys(GLOBAL_QUIT, keys, &post_post_keys);
				process_keys(GLOBAL_QUIT, keys, &post_postpone_keys);
				process_keys(GLOBAL_QUIT, keys, &post_send_keys);
				process_keys(GLOBAL_QUIT, keys, &prompt_keys);
				process_keys(GLOBAL_QUIT, keys, &save_append_overwrite_keys);
				process_keys(GLOBAL_QUIT, keys, &select_keys);
				process_keys(GLOBAL_QUIT, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "QuitTin") == 0) {
				process_keys(GLOBAL_QUIT_TIN, keys, &group_keys);
				process_keys(GLOBAL_QUIT_TIN, keys, &page_keys);
				process_keys(GLOBAL_QUIT_TIN, keys, &select_keys);
				process_keys(GLOBAL_QUIT_TIN, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'R':
			if (strcmp(keyname, "RedrawScr") == 0) {
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &group_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &option_menu_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &page_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &select_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'S':
			if (strcmp(keyname, "SaveAppendFile") == 0) {
				process_keys(SAVE_APPEND_FILE, keys, &save_append_overwrite_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SaveOverwriteFile") == 0) {
				process_keys(SAVE_OVERWRITE_FILE, keys, &save_append_overwrite_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ScrollDown") == 0) {
				process_keys(GLOBAL_SCROLL_DOWN, keys, &group_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &option_menu_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &select_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ScrollUp") == 0) {
				process_keys(GLOBAL_SCROLL_UP, keys, &group_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &option_menu_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &select_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SearchAuthB") == 0) {
				process_keys(GLOBAL_SEARCH_AUTHOR_BACKWARD, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_AUTHOR_BACKWARD, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_AUTHOR_BACKWARD, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SearchAuthF") == 0) {
				process_keys(GLOBAL_SEARCH_AUTHOR_FORWARD, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_AUTHOR_FORWARD, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_AUTHOR_FORWARD, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SearchBody") == 0) {
				process_keys(GLOBAL_SEARCH_BODY, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_BODY, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_BODY, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SearchRepeat") == 0) {
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &info_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &option_menu_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &select_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SearchSubjB") == 0) {
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &info_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &option_menu_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &select_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SearchSubjF") == 0) {
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &info_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &option_menu_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &select_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectEnterNextUnreadGrp") == 0) {
				process_keys(SELECT_ENTER_NEXT_UNREAD_GROUP, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectGoto") == 0) {
				process_keys(SELECT_GOTO, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectMarkGrpUnread") == 0) {
				process_keys(SELECT_MARK_GROUP_UNREAD, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectMoveGrp") == 0) {
				process_keys(SELECT_MOVE_GROUP, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectNextUnreadGrp") == 0) {
				process_keys(SELECT_NEXT_UNREAD_GROUP, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectQuitNoWrite") == 0) {
				process_keys(SELECT_QUIT_NO_WRITE, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectReadGrp") == 0) {
				process_keys(SELECT_ENTER_GROUP, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectResetNewsrc") == 0) {
				process_keys(SELECT_RESET_NEWSRC, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectSortActive") == 0) {
				process_keys(SELECT_SORT_ACTIVE, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectSubscribe") == 0) {
				process_keys(SELECT_SUBSCRIBE, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectSubscribePat") == 0) {
				process_keys(SELECT_SUBSCRIBE_PATTERN, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectSyncWithActive") == 0) {
				process_keys(SELECT_SYNC_WITH_ACTIVE, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectToggleDescriptions") == 0) {
				process_keys(SELECT_TOGGLE_DESCRIPTIONS, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectToggleReadDisplay") == 0) {
				process_keys(SELECT_TOGGLE_READ_DISPLAY, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectUnsubscribe") == 0) {
				process_keys(SELECT_UNSUBSCRIBE, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectUnsubscribePat") == 0) {
				process_keys(SELECT_UNSUBSCRIBE_PATTERN, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SelectYankActive") == 0) {
				process_keys(SELECT_YANK_ACTIVE, keys, &select_keys);

				return TRUE;
			}
			if (strcmp(keyname, "SetRange") == 0) {
				process_keys(GLOBAL_SET_RANGE, keys, &group_keys);
				process_keys(GLOBAL_SET_RANGE, keys, &select_keys);
				process_keys(GLOBAL_SET_RANGE, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ShellEscape") == 0) {
#ifndef NO_SHELL_ESCAPE
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &group_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &page_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &select_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &thread_keys);
#endif /* !NO_SHELL_ESCAPE */

				return TRUE;
			}
			break;

		case 'T':
			if (strcmp(keyname, "ThreadAutoSave") == 0) {
				process_keys(THREAD_AUTOSAVE, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadMail") == 0) {
				process_keys(THREAD_MAIL, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadMarkArtRead") == 0) {
				process_keys(THREAD_MARK_ARTICLE_READ, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadReadArt") == 0) {
				process_keys(THREAD_READ_ARTICLE, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadReadNextArtOrThread") == 0) {
				process_keys(THREAD_READ_NEXT_ARTICLE_OR_THREAD, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadReverseSel") == 0) {
				process_keys(THREAD_REVERSE_SELECTIONS, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadSave") == 0) {
				process_keys(THREAD_SAVE, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadSelArt") == 0) {
				process_keys(THREAD_SELECT_ARTICLE, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadTag") == 0) {
				process_keys(THREAD_TAG, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadToggleArtSel") == 0) {
				process_keys(THREAD_TOGGLE_ARTICLE_SELECTION, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadToggleSubjDisplay") == 0) {
				process_keys(THREAD_TOGGLE_SUBJECT_DISPLAY, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadUndoSel") == 0) {
				process_keys(THREAD_UNDO_SELECTIONS, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ThreadUntag") == 0) {
				process_keys(THREAD_UNTAG, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ToggleColor") == 0) {
#ifdef HAVE_COLOR
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &group_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &page_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &select_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &thread_keys);
#endif /* HAVE_COLOR */

				return TRUE;
			}
			if (strcmp(keyname, "ToggleHelpDisplay") == 0) {
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &group_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &info_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &page_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &select_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ToggleInfoLastLine") == 0) {
				process_keys(GLOBAL_TOGGLE_INFO_LAST_LINE, keys, &group_keys);
				process_keys(GLOBAL_TOGGLE_INFO_LAST_LINE, keys, &page_keys);
				process_keys(GLOBAL_TOGGLE_INFO_LAST_LINE, keys, &select_keys);
				process_keys(GLOBAL_TOGGLE_INFO_LAST_LINE, keys, &thread_keys);

				return TRUE;
			}
			if (strcmp(keyname, "ToggleInverseVideo") == 0) {
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &group_keys);
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &page_keys);
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &select_keys);
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'U':
			if (strcmp(keyname, "Up") == 0) {
				process_keys(GLOBAL_LINE_UP, keys, &group_keys);
				process_keys(GLOBAL_LINE_UP, keys, &info_keys);
				process_keys(GLOBAL_LINE_UP, keys, &option_menu_keys);
				process_keys(GLOBAL_LINE_UP, keys, &page_keys);
				process_keys(GLOBAL_LINE_UP, keys, &select_keys);
				process_keys(GLOBAL_LINE_UP, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'V':
			if (strcmp(keyname, "Version") == 0) {
				process_keys(GLOBAL_VERSION, keys, &group_keys);
				process_keys(GLOBAL_VERSION, keys, &page_keys);
				process_keys(GLOBAL_VERSION, keys, &select_keys);
				process_keys(GLOBAL_VERSION, keys, &thread_keys);

				return TRUE;
			}
			break;

		default:
			break;
	}

	return FALSE;
}


/*
 * upgrades the keymap file to the current version
 */
static void
upgrade_keymap_file(
	char *old)
{
	FILE *oldfp, *newfp;
	char *line, *backup;
	const char *keyname, *keydef;
	char new[NAME_LEN + 1], buf[LEN];
	char *bugreport[3] = { NULL, NULL, NULL };
	char *catchup[4] = { NULL, NULL, NULL, NULL };
	char *catchup_next_unread[4] = { NULL, NULL, NULL, NULL };
	char *config_select[2] = { NULL, NULL };
	char *edit_filter[2] = { NULL, NULL };
	char *down[2] = { NULL, NULL };
	char *groupreadbasenote[2] = { NULL, NULL };
	char *mark_article_unread[3] = { NULL, NULL, NULL };
	char *mark_thread_unread[3] = { NULL, NULL, NULL };
	char *menu_filter_kill[3] = { NULL, NULL, NULL };
	char *menu_filter_select[3] = { NULL, NULL, NULL };
	char *pagedown[2] = { NULL, NULL };
	char *pagenextthd[2] = { NULL, NULL };
	char *pageup[3] = { NULL, NULL, NULL };
	char *postponed[2] = { NULL, NULL };
	char *postpost[3] = { NULL, NULL, NULL };
	char *postsend[2] = { NULL, NULL };
	char *quick_filter_kill[2] = { NULL, NULL };
	char *quick_filter_select[2] = { NULL, NULL };
	char *selectentergroup[2] = { NULL, NULL };
	char *selectmarkgrpunread[2] = { NULL, NULL };
	char *selectreadgrp[2] = { NULL, NULL };
	char *threadreadart[2] = { NULL, NULL };
	char *up[2] = { NULL, NULL };

	if ((oldfp = fopen(old, "r")) == NULL)
		return;

	snprintf(new, sizeof(new), "%s.%d", old, (int) process_id);
	if ((newfp = fopen(new, "w")) == NULL) {
		fclose(oldfp);
		return;
	}
	fprintf(newfp, "# Keymap file V%s for the TIN newsreader\n", KEYMAP_VERSION);

	forever {
		line = fgets(buf, sizeof(buf), oldfp);

		if (line == NULL || line[0] == '\n') {
			/*
			 * we are at the end of a block or file
			 * write out the merged lines (if available)
			 */
			if (config_select[0] || config_select[1]) {
				fprintf(newfp, "ConfigSelect\t\t");
				if (config_select[0])
					fprintf(newfp, "\t%s", config_select[0]);
				if (config_select[1])
					fprintf(newfp, "\t%s", config_select[1]);
				fprintf(newfp, "\n");
				FreeAndNull(config_select[0]);
				FreeAndNull(config_select[1]);
			}
			if (down[0] || down[1]) {
				fprintf(newfp, "Down\t\t\t");
				if (down[0])
					fprintf(newfp, "\t%s", down[0]);
				if (down[1])
					fprintf(newfp, "\t%s", down[1]);
				fprintf(newfp, "\n");
				FreeAndNull(down[0]);
				FreeAndNull(down[1]);
			}
			if (groupreadbasenote[0] || groupreadbasenote[1]) {
				fprintf(newfp, "GroupReadBasenote\t");
				if (groupreadbasenote[0])
					fprintf(newfp, "\t%s", groupreadbasenote[0]);
				if (groupreadbasenote[1])
					fprintf(newfp, "\t%s", groupreadbasenote[1]);
				fprintf(newfp, "\n");
				FreeAndNull(groupreadbasenote[0]);
				FreeAndNull(groupreadbasenote[1]);
			}
			if (pagedown[0] || pagedown[1]) {
				fprintf(newfp, "PageDown\t\t");
				if (pagedown[0])
					fprintf(newfp, "\t%s", pagedown[0]);
				if (pagedown[1])
					fprintf(newfp, "\t%s", pagedown[1]);
				fprintf(newfp, "\n");
				FreeAndNull(pagedown[0]);
				FreeAndNull(pagedown[1]);
			}
			if (pagenextthd[0] || pagenextthd[1]) {
				fprintf(newfp, "PageNextThd\t\t");
				if (pagenextthd[0])
					fprintf(newfp, "\t%s", pagenextthd[0]);
				if (pagenextthd[1])
					fprintf(newfp, "\t%s", pagenextthd[1]);
				fprintf(newfp, "\n");
				FreeAndNull(pagenextthd[0]);
				FreeAndNull(pagenextthd[1]);
			}
			if (pageup[0] || pageup[1] || pageup[2]) {
				fprintf(newfp, "PageUp\t\t\t");
				if (pageup[0])
					fprintf(newfp, "\t%s", pageup[0]);
				if (pageup[1])
					fprintf(newfp, "\t%s", pageup[1]);
				if (pageup[2])
					fprintf(newfp, "\t%s", pageup[2]);
				fprintf(newfp, "\n");
				FreeAndNull(pageup[0]);
				FreeAndNull(pageup[1]);
				FreeAndNull(pageup[2]);
			}
			if (postponed[0] || postponed[1]) {
				fprintf(newfp, "Postponed\t\t");
				if (postponed[0])
					fprintf(newfp, "\t%s", postponed[0]);
				if (postponed[1])
					fprintf(newfp, "\t%s", postponed[1]);
				fprintf(newfp, "\n");
				FreeAndNull(postponed[0]);
				FreeAndNull(postponed[1]);
			}
			if (postpost[0] || postpost[1] || postpost[2]) {
				fprintf(newfp, "PostPost\t\t");
				if (postpost[0])
					fprintf(newfp, "\t%s", postpost[0]);
				if (postpost[1])
					fprintf(newfp, "\t%s", postpost[1]);
				if (postpost[2])
					fprintf(newfp, "\t%s", postpost[2]);
				fprintf(newfp, "\n");
				FreeAndNull(postpost[0]);
				FreeAndNull(postpost[1]);
				FreeAndNull(postpost[2]);
			}
			if (postsend[0] || postsend[1]) {
				fprintf(newfp, "PostSend\t\t");
				if (postsend[0])
					fprintf(newfp, "\t%s", postsend[0]);
				if (postsend[1])
					fprintf(newfp, "\t%s", postsend[1]);
				fprintf(newfp, "\n");
				FreeAndNull(postsend[0]);
				FreeAndNull(postsend[1]);
			}
			if (selectentergroup[0] || selectentergroup[1]) {
				fprintf(newfp, "SelectEnterNextUnreadGrp");
				if (selectentergroup[0])
					fprintf(newfp, "\t%s", selectentergroup[0]);
				if (selectentergroup[1])
					fprintf(newfp, "\t%s", selectentergroup[1]);
				fprintf(newfp, "\n");
				FreeAndNull(selectentergroup[0]);
				FreeAndNull(selectentergroup[1]);
			}
			if (selectmarkgrpunread[0] || selectmarkgrpunread[1]) {
				fprintf(newfp, "SelectMarkGrpUnread\t");
				if (selectmarkgrpunread[0])
					fprintf(newfp, "\t%s", selectmarkgrpunread[0]);
				if (selectmarkgrpunread[1])
					fprintf(newfp, "\t%s", selectmarkgrpunread[1]);
				fprintf(newfp, "\n");
				FreeAndNull(selectmarkgrpunread[0]);
				FreeAndNull(selectmarkgrpunread[1]);
			}
			if (selectreadgrp[0] || selectreadgrp[1]) {
				fprintf(newfp, "SelectReadGrp\t\t");
				if (selectreadgrp[0])
					fprintf(newfp, "\t%s", selectreadgrp[0]);
				if (selectreadgrp[1])
					fprintf(newfp, "\t%s", selectreadgrp[1]);
				fprintf(newfp, "\n");
				FreeAndNull(selectreadgrp[0]);
				FreeAndNull(selectreadgrp[1]);
			}
			if (threadreadart[0] || threadreadart[1]) {
				fprintf(newfp, "ThreadReadArt\t\t");
				if (threadreadart[0])
					fprintf(newfp, "\t%s", threadreadart[0]);
				if (threadreadart[1])
					fprintf(newfp, "\t%s", threadreadart[1]);
				fprintf(newfp, "\n");
				FreeAndNull(threadreadart[0]);
				FreeAndNull(threadreadart[1]);
			}
			if (up[0] || up[1]) {
				fprintf(newfp, "Up\t\t\t");
				if (up[0])
					fprintf(newfp, "\t%s", up[0]);
				if (up[1])
					fprintf(newfp, "\t%s", up[1]);
				fprintf(newfp, "\n");
				FreeAndNull(up[0]);
				FreeAndNull(up[1]);
			}
			if (line == NULL)
				break;	/* jump out of the while loop */
			else {
				fprintf(newfp, "\n");
				continue;
			}
		}

		if (line[0] == '#') {
			if (strncmp(line, "# Keymap file V", strlen("# Keymap file V")) != 0)
				fprintf(newfp, "%s", line);
			continue;
		}

		backup = my_strdup(line);

		if ((keyname = strsep(&line, KEYSEPS)) == NULL) {
			free(backup);
			continue;
		}
		if ((keydef = str_trim(line)) == NULL)
			keydef = "";

		switch (keyname[0]) {
			case 'C':
				if (strcmp(keyname, "ConfigFirstPage2") == 0)
					fprintf(newfp, "ConfigFirstPage\t\t\t%s\n", keydef);
				else if (strcmp(keyname, "ConfigLastPage2") == 0)
					fprintf(newfp, "ConfigLastPage\t\t\t%s\n", keydef);
				else if (strcmp(keyname, "ConfigSelect") == 0)
					config_select[0] = my_strdup(keydef);
				else if (strcmp(keyname, "ConfigSelect2") == 0)
					config_select[1] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'D':
				if (strcmp(keyname, "Down") == 0)
					down[0] = my_strdup(keydef);
				else if (strcmp(keyname, "Down2") == 0)
					down[1] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'G':
				if (strcmp(keyname, "GroupAutoSel") == 0)
					menu_filter_select[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupQuickAutoSel") == 0)
					quick_filter_select[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupQuickKill") == 0)
					quick_filter_kill[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupKill") == 0)
					menu_filter_kill[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupReadBasenote") == 0)
					groupreadbasenote[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupReadBasenote2") == 0)
					groupreadbasenote[1] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupEditFilter") == 0)
					edit_filter[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupBugReport") == 0)
					bugreport[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupMarkArtUnread") == 0)
					mark_article_unread[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupMarkThdUnread") == 0)
					mark_thread_unread[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupCatchup") == 0)
					catchup[0] = my_strdup(keydef);
				else if (strcmp(keyname, "GroupCatchupNextUnread") == 0)
					catchup_next_unread[0] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'H':
				if (strcmp(keyname, "HelpFirstPage2") == 0)
					fprintf(newfp, "HelpFirstPage\t\t\t%s\n", keydef);
				else if (strcmp(keyname, "HelpLastPage2") == 0)
					fprintf(newfp, "HelpLastPage\t\t\t%s\n", keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'N':
				/* Nrc* got removed */
				if (strcmp(keyname, "NrctblCreate") == 0)
					;
				else if (strcmp(keyname, "NrctblDefault") == 0)
					;
				else if (strcmp(keyname, "NrctblAlternative") == 0)
					;
				else if (strcmp(keyname, "NrctblQuit") == 0)
					;
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'P':
				if (strcmp(keyname, "PageAutoSel") == 0)
					menu_filter_select[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageQuickAutoSel") == 0)
					quick_filter_select[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageQuickKill") == 0)
					quick_filter_kill[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageAutoKill") == 0)
					menu_filter_kill[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageDown") == 0)
					pagedown[0] = my_strdup(keydef);
				else if (strcmp(keyname, "PageDown2") == 0)
					pagedown[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageEditFilter") == 0)
					edit_filter[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageNextThd") == 0)
					pagenextthd[0] = my_strdup(keydef);
				else if (strcmp(keyname, "PageNextThd2") == 0)
					pagenextthd[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageUp") == 0)
					pageup[0] = my_strdup(keydef);
				else if (strcmp(keyname, "PageUp2") == 0)
					pageup[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageUp3") == 0)
					pageup[2] = my_strdup(keydef);
				else if (strcmp(keyname, "Postponed") == 0)
					postponed[0] = my_strdup(keydef);
				else if (strcmp(keyname, "Postponed2") == 0)
					postponed[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PostPost") == 0)
					postpost[0] = my_strdup(keydef);
				else if (strcmp(keyname, "PostPost2") == 0)
					postpost[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PostPost3") == 0)
					postpost[2] = my_strdup(keydef);
				else if (strcmp(keyname, "PostSend") == 0)
					postsend[0] = my_strdup(keydef);
				else if (strcmp(keyname, "PostSend2") == 0)
					postsend[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageMarkArtUnread") == 0)
					mark_article_unread[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageMarkThdUnread") == 0)
					mark_thread_unread[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageCatchup") == 0)
					catchup[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PageCatchupNextUnread") == 0)
					catchup_next_unread[1] = my_strdup(keydef);
				else if (strcmp(keyname, "PromptNo") == 0 || strcmp(keyname, "PromptYes") == 0) {
					if (strlen(keydef) == 1 && islower(keydef[0]))
						fprintf(newfp, "%s\t\t\t%c\t%c\n", keyname, keydef[0], toupper(keydef[0]));
					else
						fprintf(newfp, "%s", backup);
				} else
					fprintf(newfp, "%s", backup);
				break;

			case 'S':
				if (strcmp(keyname, "SelectEditFilter") == 0)
					;
				else if (strcmp(keyname, "SelectEnterNextUnreadGrp") == 0)
					selectentergroup[0] = my_strdup(keydef);
				else if (strcmp(keyname, "SelectEnterNextUnreadGrp2") == 0)
					selectentergroup[1] = my_strdup(keydef);
				else if (strcmp(keyname, "SelectMarkGrpUnread") == 0)
					selectmarkgrpunread[0] = my_strdup(keydef);
				else if (strcmp(keyname, "SelectMarkGrpUnread2") == 0)
					selectmarkgrpunread[1] = my_strdup(keydef);
				else if (strcmp(keyname, "SelectReadGrp") == 0)
					selectreadgrp[0] = my_strdup(keydef);
				else if (strcmp(keyname, "SelectReadGrp2") == 0)
					selectreadgrp[1] = my_strdup(keydef);
				else if (strcmp(keyname, "SelectBugReport") == 0)
					bugreport[1] = my_strdup(keydef);
				else if (strcmp(keyname, "SelectCatchup") == 0)
					catchup[2] = my_strdup(keydef);
				else if (strcmp(keyname, "SelectCatchupNextUnread") == 0)
					catchup_next_unread[2] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'T':
				if (strcmp(keyname, "ThreadEditFilter") == 0)
					;
				else if (strcmp(keyname, "ThreadAutoSel") == 0)
					menu_filter_select[2] = my_strdup(keydef);
				else if (strcmp(keyname, "ThreadKill") == 0)
					menu_filter_kill[2] = my_strdup(keydef);
				else if (strcmp(keyname, "ThreadReadArt") == 0)
					threadreadart[0] = my_strdup(keydef);
				else if (strcmp(keyname, "ThreadReadArt2") == 0)
					threadreadart[1] = my_strdup(keydef);
				else if (strcmp(keyname, "ThreadBugReport") == 0)
					bugreport[2] = my_strdup(keydef);
				else if (strcmp(keyname, "ThreadMarkArtUnread") == 0)
					mark_article_unread[2] = my_strdup(keydef);
				else if (strcmp(keyname, "ThreadMarkThdUnread") == 0)
					mark_thread_unread[2] = my_strdup(keydef);
				else if (strcmp(keyname, "ThreadCatchup") == 0)
					catchup[3] = my_strdup(keydef);
				else if (strcmp(keyname, "ThreadCatchupNextUnread") == 0)
					catchup_next_unread[3] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'U':
				if (strcmp(keyname, "Up") == 0)
					up[0] = my_strdup(keydef);
				else if (strcmp(keyname, "Up2") == 0)
					up[1] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			default:
				fprintf(newfp, "%s", backup);
		}
		free(backup);
	}
	fprintf(newfp, "\n#####\n");
	/* joined/renamed keys from different sections */
	if (bugreport[0] || bugreport[1] || bugreport[2]) {
		fprintf(newfp, "BugReport\t");
		if (bugreport[0] && bugreport[1] && !strcmp(bugreport[0], bugreport[1]))
			FreeAndNull(bugreport[1]);
		if (bugreport[0] && bugreport[2] && !strcmp(bugreport[0], bugreport[2]))
			FreeAndNull(bugreport[2]);
		if (bugreport[1] && bugreport[2] && !strcmp(bugreport[1], bugreport[2]))
			FreeAndNull(bugreport[2]);
		if (bugreport[0])
			fprintf(newfp, "\t%s", bugreport[0]);
		if (bugreport[1])
			fprintf(newfp, "\t%s", bugreport[1]);
		if (bugreport[2])
			fprintf(newfp, "\t%s", bugreport[2]);
		fprintf(newfp, "\n");
		FreeAndNull(bugreport[0]);
		FreeAndNull(bugreport[1]);
		FreeAndNull(bugreport[2]);
	}
	if (catchup[0] || catchup[1] || catchup[2] || catchup[3]) {
		fprintf(newfp, "Catchup\t");
		if (catchup[0] && catchup[1] && !strcmp(catchup[0], catchup[1]))
			FreeAndNull(catchup[1]);
		if (catchup[0] && catchup[2] && !strcmp(catchup[0], catchup[2]))
			FreeAndNull(catchup[2]);
		if (catchup[0] && catchup[3] && !strcmp(catchup[0], catchup[3]))
			FreeAndNull(catchup[3]);
		if (catchup[1] && catchup[2] && !strcmp(catchup[1], catchup[2]))
			FreeAndNull(catchup[2]);
		if (catchup[1] && catchup[3] && !strcmp(catchup[1], catchup[3]))
			FreeAndNull(catchup[3]);
		if (catchup[2] && catchup[3] && !strcmp(catchup[2], catchup[3]))
			FreeAndNull(catchup[3]);
		if (catchup[0])
			fprintf(newfp, "\t%s", catchup[0]);
		if (catchup[1])
			fprintf(newfp, "\t%s", catchup[1]);
		if (catchup[2])
			fprintf(newfp, "\t%s", catchup[2]);
		if (catchup[3])
			fprintf(newfp, "\t%s", catchup[3]);
		fprintf(newfp, "\n");
		FreeAndNull(catchup[0]);
		FreeAndNull(catchup[1]);
		FreeAndNull(catchup[2]);
		FreeAndNull(catchup[3]);
	}
	if (catchup_next_unread[0] || catchup_next_unread[1] || catchup_next_unread[2] || catchup_next_unread[3]) {
		fprintf(newfp, "CatchupNextUnread\t");
		if (catchup_next_unread[0] && catchup_next_unread[1] && !strcmp(catchup_next_unread[0], catchup_next_unread[1]))
			FreeAndNull(catchup_next_unread[1]);
		if (catchup_next_unread[0] && catchup_next_unread[2] && !strcmp(catchup_next_unread[0], catchup_next_unread[2]))
			FreeAndNull(catchup_next_unread[2]);
		if (catchup_next_unread[0] && catchup_next_unread[3] && !strcmp(catchup_next_unread[0], catchup_next_unread[3]))
			FreeAndNull(catchup_next_unread[3]);
		if (catchup_next_unread[1] && catchup_next_unread[2] && !strcmp(catchup_next_unread[1], catchup_next_unread[2]))
			FreeAndNull(catchup_next_unread[2]);
		if (catchup_next_unread[1] && catchup_next_unread[3] && !strcmp(catchup_next_unread[1], catchup_next_unread[3]))
			FreeAndNull(catchup_next_unread[3]);
		if (catchup_next_unread[2] && catchup_next_unread[3] && !strcmp(catchup_next_unread[2], catchup_next_unread[3]))
			FreeAndNull(catchup_next_unread[3]);
		if (catchup_next_unread[0])
			fprintf(newfp, "\t%s", catchup_next_unread[0]);
		if (catchup_next_unread[1])
			fprintf(newfp, "\t%s", catchup_next_unread[1]);
		if (catchup_next_unread[2])
			fprintf(newfp, "\t%s", catchup_next_unread[2]);
		if (catchup_next_unread[3])
			fprintf(newfp, "\t%s", catchup_next_unread[3]);
		fprintf(newfp, "\n");
		FreeAndNull(catchup_next_unread[0]);
		FreeAndNull(catchup_next_unread[1]);
		FreeAndNull(catchup_next_unread[2]);
		FreeAndNull(catchup_next_unread[3]);
	}
	if (edit_filter[0] || edit_filter[1]) {
		fprintf(newfp, "EditFilter\t");
		if (edit_filter[0])
			fprintf(newfp, "\t%s", edit_filter[0]);
		if (edit_filter[1] && edit_filter[0] && strcmp(edit_filter[0], edit_filter[1]))
			fprintf(newfp, "\t%s", edit_filter[1]);
		fprintf(newfp, "\n");
		FreeAndNull(edit_filter[0]);
		FreeAndNull(edit_filter[1]);
	}
	if (mark_article_unread[0] || mark_article_unread[1] || mark_article_unread[2]) {
		fprintf(newfp, "MarkArticleUnread\t");
		if (mark_article_unread[0] && mark_article_unread[1] && !strcmp(mark_article_unread[0], mark_article_unread[1]))
			FreeAndNull(mark_article_unread[1]);
		if (mark_article_unread[0] && mark_article_unread[2] && !strcmp(mark_article_unread[0], mark_article_unread[2]))
			FreeAndNull(mark_article_unread[2]);
		if (mark_article_unread[1] && mark_article_unread[2] && !strcmp(mark_article_unread[1], mark_article_unread[2]))
			FreeAndNull(mark_article_unread[2]);
		if (mark_article_unread[0])
			fprintf(newfp, "\t%s", mark_article_unread[0]);
		if (mark_article_unread[1])
			fprintf(newfp, "\t%s", mark_article_unread[1]);
		if (mark_article_unread[2])
			fprintf(newfp, "\t%s", mark_article_unread[2]);
		fprintf(newfp, "\n");
		FreeAndNull(mark_article_unread[0]);
		FreeAndNull(mark_article_unread[1]);
		FreeAndNull(mark_article_unread[2]);
	}
	if (mark_thread_unread[0] || mark_thread_unread[1] || mark_thread_unread[2]) {
		fprintf(newfp, "MarkThreadUnread\t");
		if (mark_thread_unread[0] && mark_thread_unread[1] && !strcmp(mark_thread_unread[0], mark_thread_unread[1]))
			FreeAndNull(mark_thread_unread[1]);
		if (mark_thread_unread[0] && mark_thread_unread[2] && !strcmp(mark_thread_unread[0], mark_thread_unread[2]))
			FreeAndNull(mark_thread_unread[2]);
		if (mark_thread_unread[1] && mark_thread_unread[2] && !strcmp(mark_thread_unread[1], mark_thread_unread[2]))
			FreeAndNull(mark_thread_unread[2]);
		if (mark_thread_unread[0])
			fprintf(newfp, "\t%s", mark_thread_unread[0]);
		if (mark_thread_unread[1])
			fprintf(newfp, "\t%s", mark_thread_unread[1]);
		if (mark_thread_unread[2])
			fprintf(newfp, "\t%s", mark_thread_unread[2]);
		fprintf(newfp, "\n");
		FreeAndNull(mark_thread_unread[0]);
		FreeAndNull(mark_thread_unread[1]);
		FreeAndNull(mark_thread_unread[2]);
	}
	if (menu_filter_kill[0] || menu_filter_kill[1] || menu_filter_kill[2]) {
		fprintf(newfp, "MenuFilterKill\t");
		if (menu_filter_kill[0] && menu_filter_kill[1] && !strcmp(menu_filter_kill[0], menu_filter_kill[1]))
			FreeAndNull(menu_filter_kill[1]);
		if (menu_filter_kill[0] && menu_filter_kill[2] && !strcmp(menu_filter_kill[0], menu_filter_kill[2]))
			FreeAndNull(menu_filter_kill[2]);
		if (menu_filter_kill[1] && menu_filter_kill[2] && !strcmp(menu_filter_kill[1], menu_filter_kill[2]))
			FreeAndNull(menu_filter_kill[2]);
		if (menu_filter_kill[0])
			fprintf(newfp, "\t%s", menu_filter_kill[0]);
		if (menu_filter_kill[1])
			fprintf(newfp, "\t%s", menu_filter_kill[1]);
		if (menu_filter_kill[2])
			fprintf(newfp, "\t%s", menu_filter_kill[2]);
		fprintf(newfp, "\n");
		FreeAndNull(menu_filter_kill[0]);
		FreeAndNull(menu_filter_kill[1]);
		FreeAndNull(menu_filter_kill[2]);
	}
	if (menu_filter_select[0] || menu_filter_select[1] || menu_filter_select[2]) {
		fprintf(newfp, "MenuFilterSelect\t");
		if (menu_filter_select[0] && menu_filter_select[1] && !strcmp(menu_filter_select[0], menu_filter_select[1]))
			FreeAndNull(menu_filter_select[1]);
		if (menu_filter_select[0] && menu_filter_select[2] && !strcmp(menu_filter_select[0], menu_filter_select[2]))
			FreeAndNull(menu_filter_select[2]);
		if (menu_filter_select[1] && menu_filter_select[2] && !strcmp(menu_filter_select[1], menu_filter_select[2]))
			FreeAndNull(menu_filter_select[2]);
		if (menu_filter_select[0])
			fprintf(newfp, "\t%s", menu_filter_select[0]);
		if (menu_filter_select[1])
			fprintf(newfp, "\t%s", menu_filter_select[1]);
		if (menu_filter_select[2])
			fprintf(newfp, "\t%s", menu_filter_select[2]);
		fprintf(newfp, "\n");
		FreeAndNull(menu_filter_select[0]);
		FreeAndNull(menu_filter_select[1]);
		FreeAndNull(menu_filter_select[2]);
	}
	if (quick_filter_kill[0] || quick_filter_kill[1]) {
		fprintf(newfp, "QuickFilterKill\t");
		if (quick_filter_kill[0])
			fprintf(newfp, "\t%s", quick_filter_kill[0]);
		if (quick_filter_kill[1] && quick_filter_kill[0] && strcmp(quick_filter_kill[0], quick_filter_kill[1]))
			fprintf(newfp, "\t%s", quick_filter_kill[1]);
		fprintf(newfp, "\n");
		FreeAndNull(quick_filter_kill[0]);
		FreeAndNull(quick_filter_kill[1]);
	}
	if (quick_filter_select[0] || quick_filter_select[1]) {
		fprintf(newfp, "QuickFilterSelect\t");
		if (quick_filter_select[0])
			fprintf(newfp, "\t%s", quick_filter_select[0]);
		if (quick_filter_select[1] && quick_filter_select[0] && strcmp(quick_filter_select[0], quick_filter_select[1]))
			fprintf(newfp, "\t%s", quick_filter_select[1]);
		fprintf(newfp, "\n");
		FreeAndNull(quick_filter_select[0]);
		FreeAndNull(quick_filter_select[1]);
	}

	fclose(oldfp);
	fclose(newfp);
	rename(new, old);
	wait_message(0, _(txt_keymap_upgraded), KEYMAP_VERSION);
	prompt_continue();

	return;
}


/*
 * add the default key bindings for still free keys
 */
void
setup_default_keys(
	void)
{
	/* select level */
	add_global_keys(&select_keys);
	add_default_key(&select_keys, " ", GLOBAL_PAGE_DOWN);
	add_default_key(&select_keys, "\n\r", SELECT_ENTER_GROUP);
	add_default_key(&select_keys, "", SELECT_RESET_NEWSRC);
	add_default_key(&select_keys, "c", CATCHUP);
	add_default_key(&select_keys, "d", SELECT_TOGGLE_DESCRIPTIONS);
	add_default_key(&select_keys, "g", SELECT_GOTO);
	add_default_key(&select_keys, "m", SELECT_MOVE_GROUP);
	add_default_key(&select_keys, "n\t", SELECT_ENTER_NEXT_UNREAD_GROUP);
	add_default_key(&select_keys, "r", SELECT_TOGGLE_READ_DISPLAY);
	add_default_key(&select_keys, "s", SELECT_SUBSCRIBE);
	add_default_key(&select_keys, "u", SELECT_UNSUBSCRIBE);
	add_default_key(&select_keys, "y", SELECT_YANK_ACTIVE);
	add_default_key(&select_keys, "z", SELECT_MARK_GROUP_UNREAD);
	add_default_key(&select_keys, "C", CATCHUP_NEXT_UNREAD);
	add_default_key(&select_keys, "E", GLOBAL_EDIT_FILTER);
	add_default_key(&select_keys, "N", SELECT_NEXT_UNREAD_GROUP);
	add_default_key(&select_keys, "S", SELECT_SUBSCRIBE_PATTERN);
	add_default_key(&select_keys, "U", SELECT_UNSUBSCRIBE_PATTERN);
	add_default_key(&select_keys, "X", SELECT_QUIT_NO_WRITE);
	add_default_key(&select_keys, "Y", SELECT_SYNC_WITH_ACTIVE);
	add_default_key(&select_keys, "Z", SELECT_MARK_GROUP_UNREAD);
	add_default_key(&select_keys, ".", SELECT_SORT_ACTIVE);
	add_default_key(&select_keys, ">", GLOBAL_SCROLL_DOWN);
	add_default_key(&select_keys, "<", GLOBAL_SCROLL_UP);

	/* group level */
	add_global_keys(&group_keys);
	add_default_key(&group_keys, " ", GLOBAL_PAGE_DOWN);
	add_default_key(&group_keys, "", GLOBAL_MENU_FILTER_SELECT);
	add_default_key(&group_keys, "\n\r", GROUP_READ_BASENOTE);
	add_default_key(&group_keys, "", GLOBAL_MENU_FILTER_KILL);
	add_default_key(&group_keys, "a", GLOBAL_SEARCH_AUTHOR_FORWARD);
	add_default_key(&group_keys, "c", CATCHUP);
	add_default_key(&group_keys, "d", GROUP_TOGGLE_SUBJECT_DISPLAY);
	add_default_key(&group_keys, "g", GROUP_GOTO);
	add_default_key(&group_keys, "l", GROUP_LIST_THREAD);
	add_default_key(&group_keys, "m", GROUP_MAIL);
	add_default_key(&group_keys, "n", GROUP_NEXT_GROUP);
#ifndef DISABLE_PRINTING
	add_default_key(&group_keys, "o", GLOBAL_PRINT);
#endif /* !DISABLE_PRINTING */
	add_default_key(&group_keys, "p", GROUP_PREVIOUS_GROUP);
	add_default_key(&group_keys, "r", GROUP_TOGGLE_READ_UNREAD);
	add_default_key(&group_keys, "s", GROUP_SAVE);
	add_default_key(&group_keys, "t", GROUP_TAG);
	add_default_key(&group_keys, "u", GROUP_TOGGLE_THREADING);
	add_default_key(&group_keys, "x", GROUP_REPOST);
	add_default_key(&group_keys, "z", MARK_ARTICLE_UNREAD);
	add_default_key(&group_keys, "A", GLOBAL_SEARCH_AUTHOR_BACKWARD);
	add_default_key(&group_keys, "B", GLOBAL_SEARCH_BODY);
	add_default_key(&group_keys, "C", CATCHUP_NEXT_UNREAD);
	add_default_key(&group_keys, "E", GLOBAL_EDIT_FILTER);
	add_default_key(&group_keys, "G", GROUP_TOGGLE_GET_ARTICLES_LIMIT);
	add_default_key(&group_keys, "K", GROUP_MARK_THREAD_READ);
	add_default_key(&group_keys, "L", GLOBAL_LOOKUP_MESSAGEID);
	add_default_key(&group_keys, "N", GROUP_NEXT_UNREAD_ARTICLE);
	add_default_key(&group_keys, "P", GROUP_PREVIOUS_UNREAD_ARTICLE);
	add_default_key(&group_keys, "S", GROUP_AUTOSAVE);
	add_default_key(&group_keys, "T", GROUP_TAG_PARTS);
	add_default_key(&group_keys, "U", GROUP_UNTAG);
	add_default_key(&group_keys, "X", GROUP_MARK_UNSELECTED_ARTICLES_READ);
	add_default_key(&group_keys, "Z", MARK_THREAD_UNREAD);
	add_default_key(&group_keys, "\t", GROUP_NEXT_UNREAD_ARTICLE_OR_GROUP);
	add_default_key(&group_keys, "-", GLOBAL_LAST_VIEWED);
	add_default_key(&group_keys, "|", GLOBAL_PIPE);
	add_default_key(&group_keys, "[", GLOBAL_QUICK_FILTER_SELECT);
	add_default_key(&group_keys, "]", GLOBAL_QUICK_FILTER_KILL);
	add_default_key(&group_keys, "*", GROUP_SELECT_THREAD);
	add_default_key(&group_keys, ".", GROUP_TOGGLE_SELECT_THREAD);
	add_default_key(&group_keys, "@", GROUP_REVERSE_SELECTIONS);
	add_default_key(&group_keys, "~", GROUP_UNDO_SELECTIONS);
	add_default_key(&group_keys, "=", GROUP_SELECT_PATTERN);
	add_default_key(&group_keys, ";", GROUP_SELECT_THREAD_IF_UNREAD_SELECTED);
	add_default_key(&group_keys, "+", GROUP_DO_AUTOSELECT);
	add_default_key(&group_keys, ">", GLOBAL_SCROLL_DOWN);
	add_default_key(&group_keys, "<", GLOBAL_SCROLL_UP);

	/* thread keys */
	add_global_keys(&thread_keys);
	add_default_key(&thread_keys, " ", GLOBAL_PAGE_DOWN);
	add_default_key(&thread_keys, "", GLOBAL_MENU_FILTER_SELECT);
	add_default_key(&thread_keys, "", GLOBAL_MENU_FILTER_KILL);
	add_default_key(&thread_keys, "\n\r", THREAD_READ_ARTICLE);
	add_default_key(&thread_keys, "a", GLOBAL_SEARCH_AUTHOR_FORWARD);
	add_default_key(&thread_keys, "c", CATCHUP);
	add_default_key(&thread_keys, "d", THREAD_TOGGLE_SUBJECT_DISPLAY);
	add_default_key(&thread_keys, "m", THREAD_MAIL);
#ifndef DISABLE_PRINTING
	add_default_key(&thread_keys, "o", GLOBAL_PRINT);
#endif /* !DISABLE_PRINTING */
	add_default_key(&thread_keys, "s", THREAD_SAVE);
	add_default_key(&thread_keys, "t", THREAD_TAG);
	add_default_key(&thread_keys, "z", MARK_ARTICLE_UNREAD);
	add_default_key(&thread_keys, "A", GLOBAL_SEARCH_AUTHOR_BACKWARD);
	add_default_key(&thread_keys, "B", GLOBAL_SEARCH_BODY);
	add_default_key(&thread_keys, "C", CATCHUP_NEXT_UNREAD);
	add_default_key(&thread_keys, "E", GLOBAL_EDIT_FILTER);
	add_default_key(&thread_keys, "K", THREAD_MARK_ARTICLE_READ);
	add_default_key(&thread_keys, "L", GLOBAL_LOOKUP_MESSAGEID);
	add_default_key(&thread_keys, "S", THREAD_AUTOSAVE);
	add_default_key(&thread_keys, "U", THREAD_UNTAG);
	add_default_key(&thread_keys, "Z", MARK_THREAD_UNREAD);
	add_default_key(&thread_keys, "\t", THREAD_READ_NEXT_ARTICLE_OR_THREAD);
	add_default_key(&thread_keys, "-", GLOBAL_LAST_VIEWED);
	add_default_key(&thread_keys, "|", GLOBAL_PIPE);
	add_default_key(&thread_keys, "*", THREAD_SELECT_ARTICLE);
	add_default_key(&thread_keys, ".", THREAD_TOGGLE_ARTICLE_SELECTION);
	add_default_key(&thread_keys, "@", THREAD_REVERSE_SELECTIONS);
	add_default_key(&thread_keys, "~", THREAD_UNDO_SELECTIONS);
	add_default_key(&thread_keys, ">", GLOBAL_SCROLL_DOWN);
	add_default_key(&thread_keys, "<", GLOBAL_SCROLL_UP);

	/* page level */
	add_global_keys(&page_keys);
	add_default_key(&page_keys, "", GLOBAL_PAGE_DOWN);
	add_default_key(&page_keys, "", GLOBAL_MENU_FILTER_SELECT);
	add_default_key(&page_keys, "", PAGE_REPLY_QUOTE_HEADERS);
#ifdef HAVE_PGP_GPG
	add_default_key(&page_keys, "", PAGE_PGP_CHECK_ARTICLE);
#endif /* HAVE_PGP_GPG */
	add_default_key(&page_keys, "", PAGE_TOGGLE_HEADERS);
	add_default_key(&page_keys, "", GLOBAL_MENU_FILTER_KILL);
	add_default_key(&page_keys, "\n\r", PAGE_NEXT_THREAD);
	add_default_key(&page_keys, "", PAGE_TOGGLE_TABS);
	add_default_key(&page_keys, "", PAGE_FOLLOWUP_QUOTE_HEADERS);
	add_default_key(&page_keys, "a", GLOBAL_SEARCH_AUTHOR_FORWARD);
	add_default_key(&page_keys, "c", CATCHUP);
	add_default_key(&page_keys, "e", PAGE_EDIT_ARTICLE);
	add_default_key(&page_keys, "f", PAGE_FOLLOWUP_QUOTE);
	add_default_key(&page_keys, "g", GLOBAL_FIRST_PAGE);
	add_default_key(&page_keys, "l", PAGE_LIST_THREAD);
	add_default_key(&page_keys, "m", PAGE_MAIL);
	add_default_key(&page_keys, "n", PAGE_NEXT_ARTICLE);
#ifndef DISABLE_PRINTING
	add_default_key(&page_keys, "o", GLOBAL_PRINT);
#endif /* !DISABLE_PRINTING */
	add_default_key(&page_keys, "p", PAGE_PREVIOUS_ARTICLE);
	add_default_key(&page_keys, "r", PAGE_REPLY_QUOTE);
	add_default_key(&page_keys, "s", PAGE_SAVE);
	add_default_key(&page_keys, "t", PAGE_TAG);
	add_default_key(&page_keys, "u", PAGE_GOTO_PARENT);
	add_default_key(&page_keys, "x", PAGE_REPOST);
	add_default_key(&page_keys, "z", MARK_ARTICLE_UNREAD);
	add_default_key(&page_keys, "A", GLOBAL_SEARCH_AUTHOR_BACKWARD);
	add_default_key(&page_keys, "B", GLOBAL_SEARCH_BODY);
	add_default_key(&page_keys, "C", CATCHUP_NEXT_UNREAD);
	add_default_key(&page_keys, "D", PAGE_CANCEL);
	add_default_key(&page_keys, "E", GLOBAL_EDIT_FILTER);
	add_default_key(&page_keys, "F", PAGE_FOLLOWUP);
	add_default_key(&page_keys, "G", GLOBAL_LAST_PAGE);
	add_default_key(&page_keys, "K", PAGE_MARK_THREAD_READ);
	add_default_key(&page_keys, "L", GLOBAL_LOOKUP_MESSAGEID);
	add_default_key(&page_keys, "N", PAGE_NEXT_UNREAD_ARTICLE);
	add_default_key(&page_keys, "P", PAGE_PREVIOUS_UNREAD_ARTICLE);
	add_default_key(&page_keys, "R", PAGE_REPLY);
	add_default_key(&page_keys, "S", PAGE_AUTOSAVE);
	add_default_key(&page_keys, "T", PAGE_GROUP_SELECT);
	add_default_key(&page_keys, "U", PAGE_VIEW_URL);
	add_default_key(&page_keys, "V", PAGE_VIEW_ATTACHMENTS);
	add_default_key(&page_keys, "Z", MARK_THREAD_UNREAD);
	add_default_key(&page_keys, "\t", PAGE_NEXT_UNREAD);
	add_default_key(&page_keys, " ", PAGE_PAGE_DOWN3);
	add_default_key(&page_keys, "-", GLOBAL_LAST_VIEWED);
	add_default_key(&page_keys, "|", GLOBAL_PIPE);
	add_default_key(&page_keys, "<", PAGE_TOP_THREAD);
	add_default_key(&page_keys, ">", PAGE_BOTTOM_THREAD);
	add_default_key(&page_keys, "\"", PAGE_TOGGLE_TEX2ISO);
	add_default_key(&page_keys, "(", PAGE_TOGGLE_UUE);
	add_default_key(&page_keys, ")", PAGE_REVEAL);
	add_default_key(&page_keys, "[", GLOBAL_QUICK_FILTER_SELECT);
	add_default_key(&page_keys, "]", GLOBAL_QUICK_FILTER_KILL);
	add_default_key(&page_keys, "%", PAGE_TOGGLE_ROT13);
	add_default_key(&page_keys, ":", PAGE_SKIP_INCLUDED_TEXT);
	add_default_key(&page_keys, "_", PAGE_TOGGLE_HIGHLIGHTING);

	/* info pager */
	add_default_key(&info_keys, "", GLOBAL_ABORT);
	add_default_key(&info_keys, "j", GLOBAL_LINE_DOWN);
	add_default_key(&info_keys, "k", GLOBAL_LINE_UP);
	add_default_key(&info_keys, " ", GLOBAL_PAGE_DOWN);
	add_default_key(&info_keys, "b", GLOBAL_PAGE_UP);
	add_default_key(&info_keys, "g^", GLOBAL_FIRST_PAGE);
	add_default_key(&info_keys, "G$", GLOBAL_LAST_PAGE);
	add_default_key(&info_keys, "q", GLOBAL_QUIT);
	add_default_key(&info_keys, "H", GLOBAL_TOGGLE_HELP_DISPLAY);
	add_default_key(&info_keys, "/", GLOBAL_SEARCH_SUBJECT_FORWARD);
	add_default_key(&info_keys, "?", GLOBAL_SEARCH_SUBJECT_BACKWARD);
	add_default_key(&info_keys, "\\", GLOBAL_SEARCH_REPEAT);

	/* options menu */
	add_default_key(&option_menu_keys, "1", DIGIT_1);
	add_default_key(&option_menu_keys, "2", DIGIT_2);
	add_default_key(&option_menu_keys, "3", DIGIT_3);
	add_default_key(&option_menu_keys, "4", DIGIT_4);
	add_default_key(&option_menu_keys, "5", DIGIT_5);
	add_default_key(&option_menu_keys, "6", DIGIT_6);
	add_default_key(&option_menu_keys, "7", DIGIT_7);
	add_default_key(&option_menu_keys, "8", DIGIT_8);
	add_default_key(&option_menu_keys, "9", DIGIT_9);
	add_default_key(&option_menu_keys, "b", GLOBAL_PAGE_UP);
	add_default_key(&option_menu_keys, " ", GLOBAL_PAGE_DOWN);
	add_default_key(&option_menu_keys, "\n\r", CONFIG_SELECT);
	add_default_key(&option_menu_keys, "", GLOBAL_REDRAW_SCREEN);
	add_default_key(&option_menu_keys, "j", GLOBAL_LINE_DOWN);
	add_default_key(&option_menu_keys, "k", GLOBAL_LINE_UP);
	add_default_key(&option_menu_keys, "g^", GLOBAL_FIRST_PAGE);
	add_default_key(&option_menu_keys, "G$", GLOBAL_LAST_PAGE);
	add_default_key(&option_menu_keys, "q", GLOBAL_QUIT);
	add_default_key(&option_menu_keys, "v", GLOBAL_VERSION);
	add_default_key(&option_menu_keys, "Q", CONFIG_NO_SAVE);
	add_default_key(&option_menu_keys, ">", GLOBAL_SCROLL_DOWN);
	add_default_key(&option_menu_keys, "<", GLOBAL_SCROLL_UP);
	add_default_key(&option_menu_keys, "/", GLOBAL_SEARCH_SUBJECT_FORWARD);
	add_default_key(&option_menu_keys, "?", GLOBAL_SEARCH_SUBJECT_BACKWARD);
	add_default_key(&option_menu_keys, "\\", GLOBAL_SEARCH_REPEAT);

	/* prompt keys */
	add_default_key(&prompt_keys, "", GLOBAL_ABORT);
	add_default_key(&prompt_keys, "nN", PROMPT_NO);
	add_default_key(&prompt_keys, "q", GLOBAL_QUIT);
	add_default_key(&prompt_keys, "yY", PROMPT_YES);

	/* post keys */
	add_default_key(&post_send_keys, "", GLOBAL_ABORT);
	add_default_key(&post_send_keys, "e", POST_EDIT);
#ifdef HAVE_PGP_GPG
	add_default_key(&post_send_keys, "g", POST_PGP);
#endif /* HAVE_PGP_GPG */
#ifdef HAVE_ISPELL
	add_default_key(&post_send_keys, "i", POST_ISPELL);
#endif /* HAVE_ISPELL */
	add_default_key(&post_send_keys, "q", GLOBAL_QUIT);
	add_default_key(&post_send_keys, "s", POST_SEND);

	add_default_key(&post_edit_keys, "", GLOBAL_ABORT);
	add_default_key(&post_edit_keys, "e", POST_EDIT);
	add_default_key(&post_edit_keys, "o", POST_POSTPONE);
	add_default_key(&post_edit_keys, "q", GLOBAL_QUIT);

	add_default_key(&post_edit_ext_keys, "", GLOBAL_ABORT);
	add_default_key(&post_edit_ext_keys, "e", POST_EDIT);
	add_default_key(&post_edit_ext_keys, "q", GLOBAL_QUIT);
	add_default_key(&post_edit_ext_keys, "M", GLOBAL_OPTION_MENU);

	add_default_key(&post_post_keys, "", GLOBAL_ABORT);
	add_default_key(&post_post_keys, "e", POST_EDIT);
#ifdef HAVE_PGP_GPG
	add_default_key(&post_post_keys, "g", POST_PGP);
#endif /* HAVE_PGP_GPG */
#ifdef HAVE_ISPELL
	add_default_key(&post_post_keys, "i", POST_ISPELL);
#endif /* HAVE_ISPELL */
	add_default_key(&post_post_keys, "o", POST_POSTPONE);
	add_default_key(&post_post_keys, "p", GLOBAL_POST);
	add_default_key(&post_post_keys, "q", GLOBAL_QUIT);
	add_default_key(&post_post_keys, "M", GLOBAL_OPTION_MENU);

	add_default_key(&post_postpone_keys, "", GLOBAL_ABORT);
	add_default_key(&post_postpone_keys, "n", PROMPT_NO);
	add_default_key(&post_postpone_keys, "q", GLOBAL_QUIT);
	add_default_key(&post_postpone_keys, "y", PROMPT_YES);
	add_default_key(&post_postpone_keys, "A", POSTPONE_ALL);
	add_default_key(&post_postpone_keys, "Y", POSTPONE_OVERRIDE);

	add_default_key(&post_mail_fup_keys, "", GLOBAL_ABORT);
	add_default_key(&post_mail_fup_keys, "m", POST_MAIL);
	add_default_key(&post_mail_fup_keys, "p", GLOBAL_POST);
	add_default_key(&post_mail_fup_keys, "q", GLOBAL_QUIT);

	add_default_key(&post_ignore_fupto_keys, "", GLOBAL_ABORT);
	add_default_key(&post_ignore_fupto_keys, "i", POST_IGNORE_FUPTO);
	add_default_key(&post_ignore_fupto_keys, "p", GLOBAL_POST);
	add_default_key(&post_ignore_fupto_keys, "q", GLOBAL_QUIT);

	add_default_key(&post_continue_keys, "", GLOBAL_ABORT);
	add_default_key(&post_continue_keys, "a", POST_ABORT);
	add_default_key(&post_continue_keys, "c", POST_CONTINUE);
	add_default_key(&post_continue_keys, "q", GLOBAL_QUIT);

	add_default_key(&post_delete_keys, "", GLOBAL_ABORT);
	add_default_key(&post_delete_keys, "d", POST_CANCEL);
	add_default_key(&post_delete_keys, "q", GLOBAL_QUIT);
	add_default_key(&post_delete_keys, "s", POST_SUPERSEDE);

	add_default_key(&post_cancel_keys, "", GLOBAL_ABORT);
	add_default_key(&post_cancel_keys, "e", POST_EDIT);
	add_default_key(&post_cancel_keys, "d", POST_CANCEL);
	add_default_key(&post_cancel_keys, "q", GLOBAL_QUIT);

	/* feed keys */
	add_default_key(&feed_post_process_keys, "", GLOBAL_ABORT);
	add_default_key(&feed_post_process_keys, "n", POSTPROCESS_NO);
	add_default_key(&feed_post_process_keys, "s", POSTPROCESS_SHAR);
	add_default_key(&feed_post_process_keys, "y", POSTPROCESS_YES);
	add_default_key(&feed_post_process_keys, "q", GLOBAL_QUIT);

	add_default_key(&feed_type_keys, "", GLOBAL_ABORT);
	add_default_key(&feed_type_keys, "a", FEED_ARTICLE);
	add_default_key(&feed_type_keys, "h", FEED_HOT);
	add_default_key(&feed_type_keys, "p", FEED_PATTERN);
	add_default_key(&feed_type_keys, "q", GLOBAL_QUIT);
	add_default_key(&feed_type_keys, "t", FEED_THREAD);
	add_default_key(&feed_type_keys, "T", FEED_TAGGED);

	add_default_key(&feed_supersede_article_keys, "", GLOBAL_ABORT);
	add_default_key(&feed_supersede_article_keys, "q", GLOBAL_QUIT);
	add_default_key(&feed_supersede_article_keys, "r", FEED_KEY_REPOST);
	add_default_key(&feed_supersede_article_keys, "s", FEED_SUPERSEDE);

	/* filter keys */
	add_default_key(&filter_keys, "", GLOBAL_ABORT);
	add_default_key(&filter_keys, "e", FILTER_EDIT);
	add_default_key(&filter_keys, "q", GLOBAL_QUIT);
	add_default_key(&filter_keys, "s", FILTER_SAVE);

	/* mark read */
	add_default_key(&mark_read_keys, "", GLOBAL_ABORT);
	add_default_key(&mark_read_keys, "c", MARK_READ_CURRENT);
	add_default_key(&mark_read_keys, "q", GLOBAL_QUIT);
	add_default_key(&mark_read_keys, "t", MARK_READ_TAGGED);

#ifdef HAVE_PGP_GPG
	/* pgp mail */
	add_default_key(&pgp_mail_keys, "", GLOBAL_ABORT);
	add_default_key(&pgp_mail_keys, "b", PGP_KEY_ENCRYPT_SIGN);
	add_default_key(&pgp_mail_keys, "e", PGP_KEY_ENCRYPT);
	add_default_key(&pgp_mail_keys, "q", GLOBAL_QUIT);
	add_default_key(&pgp_mail_keys, "s", PGP_KEY_SIGN);

	/* pgp news */
	add_default_key(&pgp_news_keys, "", GLOBAL_ABORT);
	add_default_key(&pgp_news_keys, "i", PGP_INCLUDE_KEY);
	add_default_key(&pgp_news_keys, "q", GLOBAL_QUIT);
	add_default_key(&pgp_news_keys, "s", PGP_KEY_SIGN);
#endif /* HAVE_PGP_GPG */

	/* save */
	add_default_key(&save_append_overwrite_keys, "", GLOBAL_ABORT);
	add_default_key(&save_append_overwrite_keys, "a", SAVE_APPEND_FILE);
	add_default_key(&save_append_overwrite_keys, "o", SAVE_OVERWRITE_FILE);
	add_default_key(&save_append_overwrite_keys, "q", GLOBAL_QUIT);
}


/*
 * used to add the common keys of SELECT_LEVEL, GROUP_LEVEL, THREAD_LEVEL
 * and PAGE_LEVEL
 */
static void
add_global_keys(
	struct keylist *keys)
{
	add_default_key(keys, "", GLOBAL_ABORT);
	add_default_key(keys, "0", DIGIT_0);
	add_default_key(keys, "1", DIGIT_1);
	add_default_key(keys, "2", DIGIT_2);
	add_default_key(keys, "3", DIGIT_3);
	add_default_key(keys, "4", DIGIT_4);
	add_default_key(keys, "5", DIGIT_5);
	add_default_key(keys, "6", DIGIT_6);
	add_default_key(keys, "7", DIGIT_7);
	add_default_key(keys, "8", DIGIT_8);
	add_default_key(keys, "9", DIGIT_9);
	add_default_key(keys, "b", GLOBAL_PAGE_UP);
	add_default_key(keys, "", GLOBAL_REDRAW_SCREEN);
	add_default_key(keys, "j", GLOBAL_LINE_DOWN);
	add_default_key(keys, "k", GLOBAL_LINE_UP);
	add_default_key(keys, "O", GLOBAL_POSTPONED);
	add_default_key(keys, "h", GLOBAL_HELP);
	add_default_key(keys, "i", GLOBAL_TOGGLE_INFO_LAST_LINE);
	add_default_key(keys, "q", GLOBAL_QUIT);
	add_default_key(keys, "v", GLOBAL_VERSION);
	add_default_key(keys, "w", GLOBAL_POST);
	add_default_key(keys, "H", GLOBAL_TOGGLE_HELP_DISPLAY);
	add_default_key(keys, "I", GLOBAL_TOGGLE_INVERSE_VIDEO);
	add_default_key(keys, "M", GLOBAL_OPTION_MENU);
	add_default_key(keys, "Q", GLOBAL_QUIT_TIN);
	add_default_key(keys, "R", GLOBAL_BUGREPORT);
	add_default_key(keys, "W", GLOBAL_DISPLAY_POST_HISTORY);
	add_default_key(keys, "^", GLOBAL_FIRST_PAGE);
	add_default_key(keys, "$", GLOBAL_LAST_PAGE);
	add_default_key(keys, "/", GLOBAL_SEARCH_SUBJECT_FORWARD);
	add_default_key(keys, "?", GLOBAL_SEARCH_SUBJECT_BACKWARD);
	add_default_key(keys, "\\", GLOBAL_SEARCH_REPEAT);
	add_default_key(keys, "#", GLOBAL_SET_RANGE);
#ifndef NO_SHELL_ESCAPE
	add_default_key(keys, "!", GLOBAL_SHELL_ESCAPE);
#endif /* NO_SHELL_ESCAPE */
#ifdef HAVE_COLOR
	add_default_key(keys, "&", GLOBAL_TOGGLE_COLOR);
#endif /* HAVE COLOR */
}
