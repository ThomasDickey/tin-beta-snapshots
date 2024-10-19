/*
 *  Project   : tin - a Usenet reader
 *  Module    : keymap.c
 *  Author    : D. Nimmich, J. Faultless
 *  Created   : 2000-05-25
 *  Updated   : 2024-10-16
 *  Notes     : This file contains key mapping routines and variables.
 *
 * Copyright (c) 2000-2024 Dirk Nimmich <nimmich@muenster.de>
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
static t_bool process_mapping(const char *keyname, char *keys);
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	static t_bool add_key(struct keylist *keys, const wchar_t key, t_function func, t_bool override);
#else
	static t_bool add_key(struct keylist *keys, const char key, t_function func, t_bool override);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

struct keylist attachment_keys = { NULL, 0, 0 };
struct keylist feed_post_process_keys = { NULL, 0, 0 };
struct keylist feed_supersede_article_keys = { NULL, 0, 0 };
struct keylist feed_type_keys = { NULL, 0, 0 };
struct keylist filter_keys = { NULL, 0, 0 };
struct keylist group_keys = { NULL, 0, 0 };
struct keylist info_keys = { NULL, 0, 0 };
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
struct keylist post_hist_keys = { NULL, 0, 0 };
struct keylist post_post_keys = { NULL, 0, 0 };
struct keylist post_postpone_keys = { NULL, 0, 0 };
struct keylist post_send_keys = { NULL, 0, 0 };
struct keylist prompt_keys = { NULL, 0, 0 };
struct keylist save_append_overwrite_keys = { NULL, 0, 0 };
struct keylist scope_keys = { NULL, 0, 0 };
struct keylist select_keys = { NULL, 0, 0 };
struct keylist thread_keys = { NULL, 0, 0 };
struct keylist url_keys = { NULL, 0, 0 };


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
 * returns TRUE if the key was successfully added else FALSE
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
	for (i = 0; key != '\0' && i < keys->used; i++) {
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
				keys->max = DEFAULT_MAPKEYS_NUM;
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

	for (; *key != '\0'; key++) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		add_key(key_list, (wchar_t) *key, func, FALSE);
#else
		add_key(key_list, *key, func, FALSE);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
	}
}


static void
free_keylist(
	struct keylist *keys)
{
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
	free_keylist(&attachment_keys);
	free_keylist(&select_keys);
	free_keylist(&group_keys);
	free_keylist(&thread_keys);
	free_keylist(&option_menu_keys);
	free_keylist(&page_keys);
	free_keylist(&info_keys);
	free_keylist(&post_hist_keys);
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
#ifdef HAVE_PGP_GPG
	free_keylist(&pgp_mail_keys);
	free_keylist(&pgp_news_keys);
#endif /* HAVE_PGP_GPG */
	free_keylist(&save_append_overwrite_keys);
	free_keylist(&scope_keys);
	free_keylist(&feed_type_keys);
	free_keylist(&feed_post_process_keys);
	free_keylist(&feed_supersede_article_keys);
	free_keylist(&prompt_keys);
	free_keylist(&url_keys);
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
		snprintf(buf, MAXKEYLEN, "%s", _(txt_null));
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	else if (iswgraph(ch)) {	/* Regular printables */
		int i = wctomb(buf, (wchar_t) ch);

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
		snprintf(buf, MAXKEYLEN, "%s", _(txt_tab));
	} else if ((ch == '\n') || (ch == '\r')) {	/* LF, CR */
		snprintf(buf, MAXKEYLEN, "%s", _(txt_cr));
	} else if (ch == ESC) {		/* Escape */
		snprintf(buf, MAXKEYLEN, "%s", _(txt_esc));
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	} else if (iswcntrl(ch)) {	/* Control keys */
#else
	} else if (iscntrl(ch)) {	/* Control keys */
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		buf[0] = '^';
		buf[1] = (char) (((int) ch & 0xFF) + '@');
		buf[2] = '\0';
	} else if (ch == ' ')		/* SPACE */
		snprintf(buf, MAXKEYLEN, "%s", _(txt_space));
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
	char *p, *q;
	char *map = NULL;
	char *l, *locale;
	char *language = NULL;
	char *territory = NULL;
	char *codeset = NULL, *normcodeset = NULL;
	char *modifier = NULL;
	char *fnames[2 * 6] = { NULL }; /* 2 dirs x 6 variants */
	char dirs[3][PATH_LEN]; /* 2 dirs + endmark */
	char buf[LEN];
	int k = 0, j, i = 0, n;
	size_t s;
	struct t_version *upgrade = NULL;
	t_bool ret = TRUE;

	/*
	 * checks ${TIN_HOMEDIR:-"$HOME"}, TIN_DEFAULTS_DIR
	 * for KEYMAP_FILE."locale" or KEYMAP_FILE
	 *
	 * locale is first match from LC_ALL, LC_MESSAGES, LC_CTYPE, LANG
	 *
	 * language[_territory[.codeset]][@modifier]
	 * Beside the first part, all of them are allowed to be missing. If the
	 * full specified locale is not found, less specific ones are looked
	 * for. The various parts will be stripped off, in the following
	 * order:
	 * - codeset
	 * - normalized codeset (like _nl_normalize_codeset() in glibc)
	 * - territory
	 * - modifier
	 */

	sprintf(dirs[k++], "%s", rcdir);
#ifdef TIN_DEFAULTS_DIR
	sprintf(dirs[k++], "%s", TIN_DEFAULTS_DIR);
#endif /* TIN_DEFAULTS_DIR */
	dirs[k][0] = '\0';

	l = my_strdup(get_val("LC_ALL", get_val("LC_MESSAGES", get_val("LC_CTYPE", get_val("LANG", "")))));

	if ((locale = strrchr(l, '/'))) /* skip path */
		locale++;
	else
		locale = l;

	if (*locale) {
		language = my_strdup(locale);
		if ((p = strchr(locale, '_'))) {
			/* language */
			if ((q = strchr(language, '_')))
				*q = '\0';

			/* _territory */
			q = territory = my_malloc(strlen(p) + 1);
			while (*p && *p != '.' && *p != '@')
				*q++ = *p++;
			*q = '\0';
		} else {
			if ((p = strchr(language, '.')))
				*p = '\0';
			else
				if ((p = strchr(language, '@')))
					*p = '\0';
		}
		/* @modifier */
		if ((p = strchr(locale, '@')))
			modifier = my_strdup(p);

		/* .codeset */
		if ((p = strchr(locale, '.'))) {
			q = codeset = my_malloc(strlen(p) + 1);
			while (*p && *p != '@')
				*q++ = *p++;
			*q = '\0';

			/* normalized .codeset */
			q = normcodeset = my_malloc(strlen(codeset) + 1);
			*q++ = *codeset; /* skip initial '.' */
			for (p = codeset + 1; *p != '\0'; p++) {
				if (isdigit((unsigned char) *p)) {
					*q++ = *p;
					continue;
				}
				if (isalpha((unsigned char) *p))
					*q++ = (char) my_tolower((unsigned char) *p);
			}
			*q = '\0';
			/* TODO: prefix "iso" to normcodeset if it consist only of numbers? */
		}

		if (codeset && normcodeset) {
			if (STRCMPEQ(codeset, normcodeset))
				FreeAndNull(normcodeset);
		}
	}

	/* build array of keymap-files to look for */
	for (k = 0; dirs[k][0] != '\0'; k++) {
		if (*locale) {
			if (codeset) {
				if ((n = snprintf(NULL, 0, "%s/%s.%s%s%s%s", dirs[k], KEYMAP_FILE, BlankIfNull(language), BlankIfNull(territory), codeset, BlankIfNull(modifier))) > 0) {
					s = (size_t) n + 1;
					fnames[i] = my_malloc(s);
					if (snprintf(fnames[i], s, "%s/%s.%s%s%s%s", dirs[k], KEYMAP_FILE, BlankIfNull(language), BlankIfNull(territory), codeset, BlankIfNull(modifier)) == n)
						++i;
				}
			}
			if (normcodeset) {
				if ((n = snprintf(NULL, 0, "%s/%s.%s%s%s%s", dirs[k], KEYMAP_FILE, BlankIfNull(language), BlankIfNull(territory), normcodeset, BlankIfNull(modifier))) > 0) {
					s = (size_t) n + 1;
					fnames[i] = my_malloc(s);
					if (snprintf(fnames[i], s, "%s/%s.%s%s%s%s", dirs[k], KEYMAP_FILE, BlankIfNull(language), BlankIfNull(territory), normcodeset, BlankIfNull(modifier)) == n)
						++i;
				}
			}
			if (territory) {
				if ((n = snprintf(NULL, 0, "%s/%s.%s%s%s", dirs[k], KEYMAP_FILE, BlankIfNull(language), territory, BlankIfNull(modifier))) > 0) {
					s = (size_t) n + 1;
					fnames[i] = my_malloc(s);
					if (snprintf(fnames[i], s, "%s/%s.%s%s%s", dirs[k], KEYMAP_FILE, BlankIfNull(language), territory, BlankIfNull(modifier)) == n)
						++i;
				}
			}
			if (modifier) {
				if ((n = snprintf(NULL, 0, "%s/%s.%s%s", dirs[k], KEYMAP_FILE, BlankIfNull(language), modifier)) > 0) {
					s = (size_t) n + 1;
					fnames[i] = my_malloc(s);
					if (snprintf(fnames[i], s, "%s/%s.%s%s", dirs[k], KEYMAP_FILE, BlankIfNull(language), modifier) == n)
						++i;
				}
			}
			if (language) {
				if ((n = snprintf(NULL, 0, "%s/%s.%s", dirs[k], KEYMAP_FILE, language)) > 0) {
					s = (size_t) n + 1;
					fnames[i] = my_malloc(s);
					if (snprintf(fnames[i], s, "%s/%s.%s", dirs[k], KEYMAP_FILE, language) == n)
						++i;
				}
			}
		}
		if ((n = snprintf(NULL, 0, "%s/%s", dirs[k], KEYMAP_FILE)) > 0) {
			s = (size_t) n + 1;
			fnames[i] = my_malloc(s);
			if (snprintf(fnames[i], s, "%s/%s", dirs[k], KEYMAP_FILE) == n)
				++i;
		}
	}

	/* first non empty match wins */
	for (j = 0; j < i && !fp; j++) {
		if ((fp = tin_fopen(fnames[j], "r")) != NULL)
			break;
	}

	free(l);
	FreeIfNeeded(language);
	FreeIfNeeded(modifier);
	FreeIfNeeded(codeset);
	FreeIfNeeded(normcodeset);
	FreeIfNeeded(territory);

	if (fp) /* remember matching keymap-name */
		map = my_strdup(fnames[j]);

	/* free the array of names */
	for (j = 0; j < i; j++)
		FreeIfNeeded(fnames[j]);

	wait_message(0, _(txt_reading_keymap_file), fp ? map : _(txt_none));

	if (!fp)
		return TRUE; /* no keymap file is not an error */

	STRCPY(keymap_file, map); /* remember name for connection-page */
	/* check if keymap file is up-to-date */
	while ((line = fgets(buf, sizeof(buf), fp)) != NULL) {
		if (line[0] == '#') {
			if (upgrade == NULL && match_string(buf, "# Keymap file V", NULL, 0)) {
				/* TODO: keymap downgrade */
				upgrade = check_upgrade(line, "# Keymap file V", KEYMAP_VERSION);
				if (upgrade->state == RC_UPGRADE) {
					fclose(fp);
					upgrade_keymap_file(map);
					upgrade->state = RC_IGNORE;
					if (!(fp = tin_fopen(map, "r"))) { /* TODO: issue error message? */
						free(map);
						FreeAndNull(upgrade);
						return TRUE;
					}
				}
				break;
			}
		}
	}
	rewind(fp);
	free_keymaps();

	while ((line = fgets(buf, sizeof(buf), fp)) != NULL) {
		/*
		 * Ignore blank and comment lines
		 */
		if (line[0] == '#' || line[0] == '\n')
			continue;

		if ((kname = strsep(&line, KEYSEPS)) == NULL)
			continue;
		else {
			keydef = str_trim(line);
			/*
			 * Warn about basic syntax errors
			 */
			if (keydef == NULL || !strlen(keydef)) {
				error_message(0, _(txt_keymap_missing_key), kname);
				ret = FALSE;
				continue;
			}
		}

		/*
		 * TODO: useful? shared keymaps (NFS-Home) may differ
		 * depending on the OS (i.e. one tin has color the other has not)
		 */
		if (!process_mapping(kname, keydef)) {
			error_message(0, _(txt_keymap_invalid_name), kname);
			prompt_continue();
			ret = FALSE;
			continue;
		}
	}
	fclose(fp);
	setup_default_keys();
	if (upgrade && upgrade->state != RC_IGNORE)
		upgrade_prompt_quit(upgrade, map, NULL);

	free(map);
	FreeAndNull(upgrade);
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
	wchar_t key = '\0';
#else
	char key = '\0';
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	tmp = my_strdup(keys);		/* don't change "keys" */
	keydef = strtok(tmp, KEYSEPS);

	while (keydef != NULL) {
		error = FALSE;
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		if ((wkeydef = char2wchar_t(keydef)) == NULL) {
			error_message(1, _(txt_invalid_multibyte_sequence));
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
					if (isupper((unsigned char) keydef[1])) {
						key = ctrl(keydef[1]);
						break;
					}
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
					/* FALLTHROUGH */
				default:
					error_message(0, _(txt_keymap_invalid_key), keydef);
					ret = FALSE;
					error = TRUE;
					break;
			}
		} else {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			if (iswdigit((wint_t) (key = wkeydef[0])))
#else
			if (isdigit((int) (key = keydef[0])))
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			{
				error_message(0, _(txt_keymap_invalid_key), keydef);
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
	const char *keyname,				/* Keyname we're searching for */
	char *keys)				/* Key to assign to keyname if found */
{
	switch (keyname[0]) {
		case 'A':
			if (STRCMPEQ(keyname, "AttachPipe")) {
				process_keys(ATTACHMENT_PIPE, keys, &attachment_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "AttachSelect")) {
				process_keys(ATTACHMENT_SELECT, keys, &attachment_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "AttachSave")) {
				process_keys(ATTACHMENT_SAVE, keys, &attachment_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "AttachTag")) {
				process_keys(ATTACHMENT_TAG, keys, &attachment_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "AttachTagPattern")) {
				process_keys(ATTACHMENT_TAG_PATTERN, keys, &attachment_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "AttachToggleTagged")) {
				process_keys(ATTACHMENT_TOGGLE_TAGGED, keys, &attachment_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "AttachUntag")) {
				process_keys(ATTACHMENT_UNTAG, keys, &attachment_keys);

				return TRUE;
			}
			break;

		case 'B':
			if (STRCMPEQ(keyname, "BugReport")) {
				process_keys(GLOBAL_BUGREPORT, keys, &attachment_keys);
				process_keys(GLOBAL_BUGREPORT, keys, &group_keys);
				process_keys(GLOBAL_BUGREPORT, keys, &option_menu_keys);
				process_keys(GLOBAL_BUGREPORT, keys, &select_keys);
				process_keys(GLOBAL_BUGREPORT, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'C':
			if (STRCMPEQ(keyname, "Catchup")) {
				process_keys(CATCHUP, keys, &group_keys);
				process_keys(CATCHUP, keys, &page_keys);
				process_keys(CATCHUP, keys, &select_keys);
				process_keys(CATCHUP, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "CatchupNextUnread")) {
				process_keys(CATCHUP_NEXT_UNREAD, keys, &group_keys);
				process_keys(CATCHUP_NEXT_UNREAD, keys, &page_keys);
				process_keys(CATCHUP_NEXT_UNREAD, keys, &select_keys);
				process_keys(CATCHUP_NEXT_UNREAD, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ConfigFirstPage")) {
				process_keys(GLOBAL_FIRST_PAGE, keys, &option_menu_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ConfigLastPage")) {
				process_keys(GLOBAL_LAST_PAGE, keys, &option_menu_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ConfigNoSave")) {
				process_keys(CONFIG_NO_SAVE, keys, &option_menu_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ConfigResetAttrib")) {
				process_keys(CONFIG_RESET_ATTRIB, keys, &option_menu_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ConfigScopeMenu")) {
				process_keys(CONFIG_SCOPE_MENU, keys, &option_menu_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ConfigSelect")) {
				process_keys(CONFIG_SELECT, keys, &option_menu_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ConfigToggleAttrib")) {
				process_keys(CONFIG_TOGGLE_ATTRIB, keys, &option_menu_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ConnectionInfo")) {
				process_keys(GLOBAL_CONNECTION_INFO, keys, &group_keys);
				process_keys(GLOBAL_CONNECTION_INFO, keys, &page_keys);
				process_keys(GLOBAL_CONNECTION_INFO, keys, &select_keys);
				process_keys(GLOBAL_CONNECTION_INFO, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'D':
			if (STRCMPEQ(keyname, "DisplayPostHist")) {
				process_keys(GLOBAL_DISPLAY_POST_HISTORY, keys, &group_keys);
				process_keys(GLOBAL_DISPLAY_POST_HISTORY, keys, &page_keys);
				process_keys(GLOBAL_DISPLAY_POST_HISTORY, keys, &select_keys);
				process_keys(GLOBAL_DISPLAY_POST_HISTORY, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "Down")) {
				process_keys(GLOBAL_LINE_DOWN, keys, &attachment_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &group_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &info_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &option_menu_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &page_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &post_hist_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &scope_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &select_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &thread_keys);
				process_keys(GLOBAL_LINE_DOWN, keys, &url_keys);

				return TRUE;
			}
			break;

		case 'E':
			if (STRCMPEQ(keyname, "EditFilter")) {
				process_keys(GLOBAL_EDIT_FILTER, keys, &group_keys);
				process_keys(GLOBAL_EDIT_FILTER, keys, &page_keys);
				process_keys(GLOBAL_EDIT_FILTER, keys, &select_keys);
				process_keys(GLOBAL_EDIT_FILTER, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'F':
			if (STRCMPEQ(keyname, "FeedArt")) {
				process_keys(FEED_ARTICLE, keys, &feed_type_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FeedHot")) {
				process_keys(FEED_HOT, keys, &feed_type_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FeedPat")) {
				process_keys(FEED_PATTERN, keys, &feed_type_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FeedRange")) {
				process_keys(FEED_RANGE, keys, &feed_type_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FeedRepost")) {
				process_keys(FEED_KEY_REPOST, keys, &feed_supersede_article_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FeedSupersede")) {
				process_keys(FEED_SUPERSEDE, keys, &feed_supersede_article_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FeedTag")) {
				process_keys(FEED_TAGGED, keys, &feed_type_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FeedThd")) {
				process_keys(FEED_THREAD, keys, &feed_type_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FilterEdit")) {
				process_keys(FILTER_EDIT, keys, &filter_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FilterSave")) {
				process_keys(FILTER_SAVE, keys, &filter_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "FirstPage")) {
				process_keys(GLOBAL_FIRST_PAGE, keys, &attachment_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &group_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &info_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &option_menu_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &page_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &post_hist_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &scope_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &select_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &thread_keys);
				process_keys(GLOBAL_FIRST_PAGE, keys, &url_keys);

				return TRUE;
			}
			break;

		case 'G':
			if (STRCMPEQ(keyname, "GroupAutoSave")) {
				process_keys(GROUP_AUTOSAVE, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupCancel")) {
				process_keys(GROUP_CANCEL, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupDoAutoSel")) {
				process_keys(GROUP_DO_AUTOSELECT, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupGoto")) {
				process_keys(GROUP_GOTO, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupListThd")) {
				process_keys(GROUP_LIST_THREAD, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupMail")) {
				process_keys(GROUP_MAIL, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupMarkThdRead")) {
				process_keys(GROUP_MARK_THREAD_READ, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupMarkUnselArtRead")) {
				process_keys(GROUP_MARK_UNSELECTED_ARTICLES_READ, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupNextGroup")) {
				process_keys(GROUP_NEXT_GROUP, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupNextUnreadArt")) {
				process_keys(GROUP_NEXT_UNREAD_ARTICLE, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupNextUnreadArtOrGrp")) {
				process_keys(GROUP_NEXT_UNREAD_ARTICLE_OR_GROUP, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupPrevGroup")) {
				process_keys(GROUP_PREVIOUS_GROUP, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupPrevUnreadArt")) {
				process_keys(GROUP_PREVIOUS_UNREAD_ARTICLE, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupReadBasenote")) {
				process_keys(GROUP_READ_BASENOTE, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupRepost")) {
				process_keys(GROUP_REPOST, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupReverseSel")) {
				process_keys(GROUP_REVERSE_SELECTIONS, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupSave")) {
				process_keys(GROUP_SAVE, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupSelPattern")) {
				process_keys(GROUP_SELECT_PATTERN, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupSelThd")) {
				process_keys(GROUP_SELECT_THREAD, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupSelThdIfUnreadSelected")) {
				process_keys(GROUP_SELECT_THREAD_IF_UNREAD_SELECTED, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupTag")) {
				process_keys(GROUP_TAG, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupTagParts")) {
				process_keys(GROUP_TAG_PARTS, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupToggleGetartLimit")) {
				process_keys(GROUP_TOGGLE_GET_ARTICLES_LIMIT, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupToggleReadUnread")) {
				process_keys(GROUP_TOGGLE_READ_UNREAD, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupToggleSubjDisplay")) {
				process_keys(GROUP_TOGGLE_SUBJECT_DISPLAY, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupToggleThdSel")) {
				process_keys(GROUP_TOGGLE_SELECT_THREAD, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupToggleThreading")) {
				process_keys(GROUP_TOGGLE_THREADING, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupUndoSel")) {
				process_keys(GROUP_UNDO_SELECTIONS, keys, &group_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "GroupUntag")) {
				process_keys(GROUP_UNTAG, keys, &group_keys);

				return TRUE;
			}
			break;

		case 'H':
			if (STRCMPEQ(keyname, "Help")) {
				process_keys(GLOBAL_HELP, keys, &attachment_keys);
				process_keys(GLOBAL_HELP, keys, &group_keys);
				process_keys(GLOBAL_HELP, keys, &option_menu_keys);
				process_keys(GLOBAL_HELP, keys, &page_keys);
				process_keys(GLOBAL_HELP, keys, &post_hist_keys);
				process_keys(GLOBAL_HELP, keys, &scope_keys);
				process_keys(GLOBAL_HELP, keys, &select_keys);
				process_keys(GLOBAL_HELP, keys, &thread_keys);
				process_keys(GLOBAL_HELP, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "HelpFirstPage")) {
				process_keys(GLOBAL_FIRST_PAGE, keys, &info_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "HelpLastPage")) {
				process_keys(GLOBAL_LAST_PAGE, keys, &info_keys);

				return TRUE;
			}
			break;

		case 'L':
			if (STRCMPEQ(keyname, "LastPage")) {
				process_keys(GLOBAL_LAST_PAGE, keys, &attachment_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &group_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &info_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &option_menu_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &page_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &post_hist_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &scope_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &select_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &thread_keys);
				process_keys(GLOBAL_LAST_PAGE, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "LastViewed")) {
				process_keys(GLOBAL_LAST_VIEWED, keys, &group_keys);
				process_keys(GLOBAL_LAST_VIEWED, keys, &page_keys);
				process_keys(GLOBAL_LAST_VIEWED, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "LookupMessage")) {
				process_keys(GLOBAL_LOOKUP_MESSAGEID, keys, &group_keys);
				process_keys(GLOBAL_LOOKUP_MESSAGEID, keys, &page_keys);
#ifdef NNTP_ABLE
				process_keys(GLOBAL_LOOKUP_MESSAGEID, keys, &select_keys);
#endif /* NNTP_ABLE */
				process_keys(GLOBAL_LOOKUP_MESSAGEID, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'M':
			if (STRCMPEQ(keyname, "MarkArticleUnread")) {
				process_keys(MARK_ARTICLE_UNREAD, keys, &group_keys);
				process_keys(MARK_ARTICLE_UNREAD, keys, &page_keys);
				process_keys(MARK_ARTICLE_UNREAD, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "MarkThreadUnread")) {
				process_keys(MARK_THREAD_UNREAD, keys, &group_keys);
				process_keys(MARK_THREAD_UNREAD, keys, &page_keys);
				process_keys(MARK_THREAD_UNREAD, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "MarkFeedRead")) {
				process_keys(MARK_FEED_READ, keys, &group_keys);
				process_keys(MARK_FEED_READ, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "MarkFeedUnread")) {
				process_keys(MARK_FEED_UNREAD, keys, &group_keys);
				process_keys(MARK_FEED_UNREAD, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "MenuFilterKill")) {
				process_keys(GLOBAL_MENU_FILTER_KILL, keys, &group_keys);
				process_keys(GLOBAL_MENU_FILTER_KILL, keys, &page_keys);
				process_keys(GLOBAL_MENU_FILTER_KILL, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "MenuFilterSelect")) {
				process_keys(GLOBAL_MENU_FILTER_SELECT, keys, &group_keys);
				process_keys(GLOBAL_MENU_FILTER_SELECT, keys, &page_keys);
				process_keys(GLOBAL_MENU_FILTER_SELECT, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'O':
			if (STRCMPEQ(keyname, "OptionMenu")) {
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
			if (STRCMPEQ(keyname, "PageArticleInfo")) {
				process_keys(PAGE_ARTICLE_INFO, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageAutoSave")) {
				process_keys(PAGE_AUTOSAVE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageBotThd")) {
				process_keys(PAGE_BOTTOM_THREAD, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageCancel")) {
				process_keys(PAGE_CANCEL, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageDown")) {
				process_keys(GLOBAL_PAGE_DOWN, keys, &attachment_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &group_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &info_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &option_menu_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &page_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &post_hist_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &scope_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &select_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &thread_keys);
				process_keys(GLOBAL_PAGE_DOWN, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageEditArticle")) {
				process_keys(PAGE_EDIT_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageFirstPage")) {
				process_keys(GLOBAL_FIRST_PAGE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageFollowup")) {
				process_keys(PAGE_FOLLOWUP, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageFollowupQuote")) {
				process_keys(PAGE_FOLLOWUP_QUOTE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageFollowupQuoteHeaders")) {
				process_keys(PAGE_FOLLOWUP_QUOTE_HEADERS, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageGotoParent")) {
				process_keys(PAGE_GOTO_PARENT, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageGroupSel")) {
				process_keys(PAGE_GROUP_SELECT, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageLastPage")) {
				process_keys(GLOBAL_LAST_PAGE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageListThd")) {
				process_keys(PAGE_LIST_THREAD, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageKillThd")) {
				process_keys(PAGE_MARK_THREAD_READ, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageMail")) {
				process_keys(PAGE_MAIL, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageNextArt")) {
				process_keys(PAGE_NEXT_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageNextThd")) {
				process_keys(PAGE_NEXT_THREAD, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageNextUnread")) {
				process_keys(PAGE_NEXT_UNREAD, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageNextUnreadArt")) {
				process_keys(PAGE_NEXT_UNREAD_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PagePGPCheckArticle")) {
#ifdef HAVE_PGP_GPG
				process_keys(PAGE_PGP_CHECK_ARTICLE, keys, &page_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PagePrevArt")) {
				process_keys(PAGE_PREVIOUS_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PagePrevUnreadArt")) {
				process_keys(PAGE_PREVIOUS_UNREAD_ARTICLE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageReply")) {
				process_keys(PAGE_REPLY, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageReplyQuote")) {
				process_keys(PAGE_REPLY_QUOTE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageReplyQuoteHeaders")) {
				process_keys(PAGE_REPLY_QUOTE_HEADERS, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageRepost")) {
				process_keys(PAGE_REPOST, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageReveal")) {
				process_keys(PAGE_REVEAL, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageSave")) {
				process_keys(PAGE_SAVE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageSkipIncludedText")) {
				process_keys(PAGE_SKIP_INCLUDED_TEXT, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageTag")) {
				process_keys(PAGE_TAG, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageTopThd")) {
				process_keys(PAGE_TOP_THREAD, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageToggleAllHeaders")) {
				process_keys(PAGE_TOGGLE_HEADERS, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageToggleHighlight")) {
				process_keys(PAGE_TOGGLE_HIGHLIGHTING, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageToggleRaw")) {
				process_keys(PAGE_TOGGLE_RAW, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageToggleRot")) {
				process_keys(PAGE_TOGGLE_ROT13, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageToggleTabs")) {
				process_keys(PAGE_TOGGLE_TABS, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageToggleTex2iso")) {
				process_keys(PAGE_TOGGLE_TEX2ISO, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageToggleUue")) {
				process_keys(PAGE_TOGGLE_UUE, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageUp")) {
				process_keys(GLOBAL_PAGE_UP, keys, &attachment_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &group_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &info_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &option_menu_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &page_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &post_hist_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &scope_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &select_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &thread_keys);
				process_keys(GLOBAL_PAGE_UP, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageViewAttach")) {
				process_keys(PAGE_VIEW_ATTACHMENTS, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PageViewUrl")) {
				process_keys(PAGE_VIEW_URL, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PgpEncrypt")) {
#ifdef HAVE_PGP_GPG
				process_keys(PGP_KEY_ENCRYPT, keys, &pgp_mail_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PgpEncSign")) {
#ifdef HAVE_PGP_GPG
				process_keys(PGP_KEY_ENCRYPT_SIGN, keys, &pgp_mail_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PgpIncludekey")) {
#ifdef HAVE_PGP_GPG
				process_keys(PGP_INCLUDE_KEY, keys, &pgp_news_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PgpSign")) {
#ifdef HAVE_PGP_GPG
				process_keys(PGP_KEY_SIGN, keys, &pgp_news_keys);
				process_keys(PGP_KEY_SIGN, keys, &pgp_mail_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (STRCMPEQ(keyname, "Pipe")) {
				process_keys(GLOBAL_PIPE, keys, &attachment_keys);
				process_keys(GLOBAL_PIPE, keys, &group_keys);
				process_keys(GLOBAL_PIPE, keys, &page_keys);
				process_keys(GLOBAL_PIPE, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "Post")) {
				process_keys(GLOBAL_POST, keys, &group_keys);
				process_keys(GLOBAL_POST, keys, &page_keys);
				process_keys(GLOBAL_POST, keys, &post_ignore_fupto_keys);
				process_keys(GLOBAL_POST, keys, &post_mail_fup_keys);
				process_keys(GLOBAL_POST, keys, &post_post_keys);
				process_keys(GLOBAL_POST, keys, &select_keys);
				process_keys(GLOBAL_POST, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostAbort")) {
				process_keys(POST_ABORT, keys, &post_continue_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostCancel")) {
				process_keys(POST_CANCEL, keys, &post_cancel_keys);
				process_keys(POST_CANCEL, keys, &post_delete_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostContinue")) {
				process_keys(POST_CONTINUE, keys, &post_continue_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostEdit")) {
				process_keys(POST_EDIT, keys, &post_cancel_keys);
				process_keys(POST_EDIT, keys, &post_edit_keys);
				process_keys(POST_EDIT, keys, &post_edit_ext_keys);
				process_keys(POST_EDIT, keys, &post_post_keys);
				process_keys(POST_EDIT, keys, &post_send_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostIgnore")) {
				process_keys(POST_IGNORE_FUPTO, keys, &post_ignore_fupto_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostIspell")) {
#ifdef HAVE_ISPELL
				process_keys(POST_ISPELL, keys, &post_post_keys);
				process_keys(POST_ISPELL, keys, &post_send_keys);
#endif /* HAVE_ISPELL */

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostMail")) {
				process_keys(POST_MAIL, keys, &post_mail_fup_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostPGP")) {
#ifdef HAVE_PGP_GPG
				process_keys(POST_PGP, keys, &post_post_keys);
				process_keys(POST_PGP, keys, &post_send_keys);
#endif /* HAVE_PGP_GPG */

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostedArticlesSelect")) {
				process_keys(POSTED_SELECT, keys, &post_hist_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostponeAll")) {
				process_keys(POSTPONE_ALL, keys, &post_postpone_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "Postponed")) {
				process_keys(GLOBAL_POSTPONED, keys, &group_keys);
				process_keys(GLOBAL_POSTPONED, keys, &page_keys);
				process_keys(GLOBAL_POSTPONED, keys, &select_keys);
				process_keys(GLOBAL_POSTPONED, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostponeOverride")) {
				process_keys(POSTPONE_OVERRIDE, keys, &post_postpone_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostPost")) {
				process_keys(GLOBAL_POST, keys, &post_ignore_fupto_keys);
				process_keys(GLOBAL_POST, keys, &post_mail_fup_keys);
				process_keys(GLOBAL_POST, keys, &post_post_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostPostpone")) {
				process_keys(POST_POSTPONE, keys, &post_edit_keys);
				process_keys(POST_POSTPONE, keys, &post_post_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostSend")) {
				process_keys(POST_SEND, keys, &post_send_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PostSupersede")) {
				process_keys(POST_SUPERSEDE, keys, &post_delete_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PProcNo")) {
				process_keys(POSTPROCESS_NO, keys, &feed_post_process_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PProcShar")) {
				process_keys(POSTPROCESS_SHAR, keys, &feed_post_process_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PProcYes")) {
				process_keys(POSTPROCESS_YES, keys, &feed_post_process_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "Print")) {
#ifndef DISABLE_PRINTING
				process_keys(GLOBAL_PRINT, keys, &group_keys);
				process_keys(GLOBAL_PRINT, keys, &page_keys);
				process_keys(GLOBAL_PRINT, keys, &thread_keys);
#endif /* !DISABLE_PRINTING */

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PromptNo")) {
				process_keys(PROMPT_NO, keys, &post_postpone_keys);
				process_keys(PROMPT_NO, keys, &prompt_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "PromptYes")) {
				process_keys(PROMPT_YES, keys, &post_postpone_keys);
				process_keys(PROMPT_YES, keys, &prompt_keys);

				return TRUE;
			}
			break;

		case 'Q':
			if (STRCMPEQ(keyname, "QuickFilterKill")) {
				process_keys(GLOBAL_QUICK_FILTER_KILL, keys, &group_keys);
				process_keys(GLOBAL_QUICK_FILTER_KILL, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "QuickFilterSelect")) {
				process_keys(GLOBAL_QUICK_FILTER_SELECT, keys, &group_keys);
				process_keys(GLOBAL_QUICK_FILTER_SELECT, keys, &page_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "Quit")) {
				process_keys(GLOBAL_QUIT, keys, &attachment_keys);
				process_keys(GLOBAL_QUIT, keys, &feed_post_process_keys);
				process_keys(GLOBAL_QUIT, keys, &feed_supersede_article_keys);
				process_keys(GLOBAL_QUIT, keys, &feed_type_keys);
				process_keys(GLOBAL_QUIT, keys, &filter_keys);
				process_keys(GLOBAL_QUIT, keys, &group_keys);
				process_keys(GLOBAL_QUIT, keys, &info_keys);
				process_keys(GLOBAL_QUIT, keys, &option_menu_keys);
				process_keys(GLOBAL_QUIT, keys, &page_keys);
				process_keys(GLOBAL_QUIT, keys, &post_hist_keys);
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
				process_keys(GLOBAL_QUIT, keys, &scope_keys);
				process_keys(GLOBAL_QUIT, keys, &select_keys);
				process_keys(GLOBAL_QUIT, keys, &thread_keys);
				process_keys(GLOBAL_QUIT, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "QuitTin")) {
				process_keys(GLOBAL_QUIT_TIN, keys, &group_keys);
				process_keys(GLOBAL_QUIT_TIN, keys, &page_keys);
				process_keys(GLOBAL_QUIT_TIN, keys, &select_keys);
				process_keys(GLOBAL_QUIT_TIN, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'R':
			if (STRCMPEQ(keyname, "RedrawScr")) {
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &attachment_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &group_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &option_menu_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &page_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &scope_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &select_keys);
				process_keys(GLOBAL_REDRAW_SCREEN, keys, &thread_keys);

				return TRUE;
			}
			break;

		case 'S':
			if (STRCMPEQ(keyname, "SaveAppendFile")) {
				process_keys(SAVE_APPEND_FILE, keys, &save_append_overwrite_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SaveOverwriteFile")) {
				process_keys(SAVE_OVERWRITE_FILE, keys, &save_append_overwrite_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ScopeAdd")) {
				process_keys(SCOPE_ADD, keys, &scope_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ScopeDelete")) {
				process_keys(SCOPE_DELETE, keys, &scope_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ScopeEditAttributesFile")) {
				process_keys(SCOPE_EDIT_ATTRIBUTES_FILE, keys, &scope_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ScopeMove")) {
				process_keys(SCOPE_MOVE, keys, &scope_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ScopeRename")) {
				process_keys(SCOPE_RENAME, keys, &scope_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ScopeSelect")) {
				process_keys(SCOPE_SELECT, keys, &scope_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ScrollDown")) {
				process_keys(GLOBAL_SCROLL_DOWN, keys, &attachment_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &group_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &option_menu_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &post_hist_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &scope_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &select_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &thread_keys);
				process_keys(GLOBAL_SCROLL_DOWN, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ScrollUp")) {
				process_keys(GLOBAL_SCROLL_UP, keys, &attachment_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &group_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &option_menu_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &post_hist_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &scope_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &select_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &thread_keys);
				process_keys(GLOBAL_SCROLL_UP, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SearchAuthB")) {
				process_keys(GLOBAL_SEARCH_AUTHOR_BACKWARD, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_AUTHOR_BACKWARD, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_AUTHOR_BACKWARD, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SearchAuthF")) {
				process_keys(GLOBAL_SEARCH_AUTHOR_FORWARD, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_AUTHOR_FORWARD, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_AUTHOR_FORWARD, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SearchBody")) {
				process_keys(GLOBAL_SEARCH_BODY, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_BODY, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_BODY, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SearchRepeat")) {
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &attachment_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &info_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &option_menu_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &post_hist_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &select_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &thread_keys);
				process_keys(GLOBAL_SEARCH_REPEAT, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SearchSubjB")) {
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &attachment_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &info_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &option_menu_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &post_hist_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &select_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &thread_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_BACKWARD, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SearchSubjF")) {
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &attachment_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &group_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &info_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &option_menu_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &page_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &post_hist_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &select_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &thread_keys);
				process_keys(GLOBAL_SEARCH_SUBJECT_FORWARD, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectEnterNextUnreadGrp")) {
				process_keys(SELECT_ENTER_NEXT_UNREAD_GROUP, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectGoto")) {
				process_keys(SELECT_GOTO, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectMarkGrpUnread")) {
				process_keys(SELECT_MARK_GROUP_UNREAD, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectMoveGrp")) {
				process_keys(SELECT_MOVE_GROUP, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectNextUnreadGrp")) {
				process_keys(SELECT_NEXT_UNREAD_GROUP, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectQuitNoWrite")) {
				process_keys(SELECT_QUIT_NO_WRITE, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectReadGrp")) {
				process_keys(SELECT_ENTER_GROUP, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectResetNewsrc")) {
				process_keys(SELECT_RESET_NEWSRC, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectSortActive")) {
				process_keys(SELECT_SORT_ACTIVE, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectSubscribe")) {
				process_keys(SELECT_SUBSCRIBE, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectSubscribePat")) {
				process_keys(SELECT_SUBSCRIBE_PATTERN, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectSyncWithActive")) {
				process_keys(SELECT_SYNC_WITH_ACTIVE, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectToggleDescriptions")) {
				process_keys(SELECT_TOGGLE_DESCRIPTIONS, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectToggleReadDisplay")) {
				process_keys(SELECT_TOGGLE_READ_DISPLAY, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectUnsubscribe")) {
				process_keys(SELECT_UNSUBSCRIBE, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectUnsubscribePat")) {
				process_keys(SELECT_UNSUBSCRIBE_PATTERN, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SelectYankActive")) {
				process_keys(SELECT_YANK_ACTIVE, keys, &select_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "SetRange")) {
				process_keys(GLOBAL_SET_RANGE, keys, &group_keys);
				process_keys(GLOBAL_SET_RANGE, keys, &select_keys);
				process_keys(GLOBAL_SET_RANGE, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ShellEscape")) {
#ifndef NO_SHELL_ESCAPE
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &attachment_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &group_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &option_menu_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &page_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &post_hist_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &scope_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &select_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &thread_keys);
				process_keys(GLOBAL_SHELL_ESCAPE, keys, &url_keys);
#endif /* !NO_SHELL_ESCAPE */

				return TRUE;
			}
			break;

		case 'T':
			if (STRCMPEQ(keyname, "ThreadAutoSave")) {
				process_keys(THREAD_AUTOSAVE, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadCancel")) {
				process_keys(THREAD_CANCEL, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadFollowup")) {
				process_keys(THREAD_FOLLOWUP, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadFollowupQuote")) {
				process_keys(THREAD_FOLLOWUP_QUOTE, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadMail")) {
				process_keys(THREAD_MAIL, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadMarkArtRead")) {
				process_keys(THREAD_MARK_ARTICLE_READ, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadReadArt")) {
				process_keys(THREAD_READ_ARTICLE, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadReadNextArtOrThread")) {
				process_keys(THREAD_READ_NEXT_ARTICLE_OR_THREAD, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadReverseSel")) {
				process_keys(THREAD_REVERSE_SELECTIONS, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadSave")) {
				process_keys(THREAD_SAVE, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadSelArt")) {
				process_keys(THREAD_SELECT_ARTICLE, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadTag")) {
				process_keys(THREAD_TAG, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadTagParts")) {
				process_keys(THREAD_TAG_PARTS, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadToggleArtSel")) {
				process_keys(THREAD_TOGGLE_ARTICLE_SELECTION, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadToggleSubjDisplay")) {
				process_keys(THREAD_TOGGLE_SUBJECT_DISPLAY, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadUndoSel")) {
				process_keys(THREAD_UNDO_SELECTIONS, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ThreadUntag")) {
				process_keys(THREAD_UNTAG, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ToggleColor")) {
#ifdef HAVE_COLOR
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &group_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &info_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &option_menu_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &page_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &post_hist_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &select_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &thread_keys);
				process_keys(GLOBAL_TOGGLE_COLOR, keys, &url_keys);
#endif /* HAVE_COLOR */

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ToggleHelpDisplay")) {
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &attachment_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &group_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &info_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &page_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &scope_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &select_keys);
				process_keys(GLOBAL_TOGGLE_HELP_DISPLAY, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ToggleInfoLastLine")) {
				process_keys(GLOBAL_TOGGLE_INFO_LAST_LINE, keys, &attachment_keys);
				process_keys(GLOBAL_TOGGLE_INFO_LAST_LINE, keys, &group_keys);
				process_keys(GLOBAL_TOGGLE_INFO_LAST_LINE, keys, &page_keys);
				process_keys(GLOBAL_TOGGLE_INFO_LAST_LINE, keys, &select_keys);
				process_keys(GLOBAL_TOGGLE_INFO_LAST_LINE, keys, &thread_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "ToggleInverseVideo")) {
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &group_keys);
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &page_keys);
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &post_hist_keys);
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &select_keys);
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &thread_keys);
				process_keys(GLOBAL_TOGGLE_INVERSE_VIDEO, keys, &url_keys);

				return TRUE;
			}
			break;

		case 'U':
			if (STRCMPEQ(keyname, "Up")) {
				process_keys(GLOBAL_LINE_UP, keys, &attachment_keys);
				process_keys(GLOBAL_LINE_UP, keys, &group_keys);
				process_keys(GLOBAL_LINE_UP, keys, &info_keys);
				process_keys(GLOBAL_LINE_UP, keys, &option_menu_keys);
				process_keys(GLOBAL_LINE_UP, keys, &page_keys);
				process_keys(GLOBAL_LINE_UP, keys, &post_hist_keys);
				process_keys(GLOBAL_LINE_UP, keys, &scope_keys);
				process_keys(GLOBAL_LINE_UP, keys, &select_keys);
				process_keys(GLOBAL_LINE_UP, keys, &thread_keys);
				process_keys(GLOBAL_LINE_UP, keys, &url_keys);

				return TRUE;
			}
			if (STRCMPEQ(keyname, "UrlSelect")) {
				process_keys(URL_SELECT, keys, &url_keys);

				return TRUE;
			}
			break;

		case 'V':
			if (STRCMPEQ(keyname, "Version")) {
				process_keys(GLOBAL_VERSION, keys, &attachment_keys);
				process_keys(GLOBAL_VERSION, keys, &group_keys);
				process_keys(GLOBAL_VERSION, keys, &page_keys);
				process_keys(GLOBAL_VERSION, keys, &post_hist_keys);
				process_keys(GLOBAL_VERSION, keys, &select_keys);
				process_keys(GLOBAL_VERSION, keys, &thread_keys);
				process_keys(GLOBAL_VERSION, keys, &url_keys);

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
	char newk[NAME_LEN + 1], buf[LEN];
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
	char *pagedown[3] = { NULL, NULL, NULL };
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

	if ((oldfp = tin_fopen(old, "r")) == NULL)
		return;

	snprintf(newk, sizeof(newk), "%s.%ld", old, (long) process_id);
	if ((newfp = fopen(newk, "w")) == NULL) {
		perror_message(_(txt_cannot_open_for_saving), newk);
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
			if (pagedown[0] || pagedown[1] || pagedown[2]) {
				fprintf(newfp, "PageDown\t\t");
				if (pagedown[0])
					fprintf(newfp, "\t%s", pagedown[0]);
				if (pagedown[1])
					fprintf(newfp, "\t%s", pagedown[1]);
				if (pagedown[2])
					fprintf(newfp, "\t%s", pagedown[2]);
				fprintf(newfp, "\n");
				FreeAndNull(pagedown[0]);
				FreeAndNull(pagedown[1]);
				FreeAndNull(pagedown[2]);
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
			/* TODO: this was merged with GLOBAL_POST "Post" */
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
				/* TODO ConfigFirstPage/keyname was merged with GLOBAL_FIRST_PAGE/GLOBAL_LAST_PAGE */
				if (STRCMPEQ(keyname, "ConfigFirstPage2"))
					fprintf(newfp, "ConfigFirstPage\t\t\t%s\n", keydef);
				else if (STRCMPEQ(keyname, "ConfigLastPage2"))
					fprintf(newfp, "ConfigLastPage\t\t\t%s\n", keydef);
				else if (STRCMPEQ(keyname, "ConfigSelect"))
					config_select[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "ConfigSelect2"))
					config_select[1] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'D':
				if (STRCMPEQ(keyname, "Down"))
					down[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "Down2"))
					down[1] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'G':
				if (STRCMPEQ(keyname, "GroupAutoSel"))
					menu_filter_select[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupQuickAutoSel"))
					quick_filter_select[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupQuickKill"))
					quick_filter_kill[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupKill"))
					menu_filter_kill[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupReadBasenote"))
					groupreadbasenote[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupReadBasenote2"))
					groupreadbasenote[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupEditFilter"))
					edit_filter[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupBugReport"))
					bugreport[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupMarkArtUnread"))
					mark_article_unread[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupMarkThdUnread"))
					mark_thread_unread[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupCatchup"))
					catchup[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "GroupCatchupNextUnread"))
					catchup_next_unread[0] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'H':
				/* TODO: HelpFirstPage/HelpLastPage was merged with GLOBAL_FIRST_PAGE/GLOBAL_LAST_PAGE */
				if (STRCMPEQ(keyname, "HelpFirstPage2"))
					fprintf(newfp, "HelpFirstPage\t\t\t%s\n", keydef);
				else if (STRCMPEQ(keyname, "HelpLastPage2"))
					fprintf(newfp, "HelpLastPage\t\t\t%s\n", keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'N':
				/* Nrc* got removed */
				if (STRCMPEQ(keyname, "NrctblCreate"))
					;
				else if (STRCMPEQ(keyname, "NrctblDefault"))
					;
				else if (STRCMPEQ(keyname, "NrctblAlternative"))
					;
				else if (STRCMPEQ(keyname, "NrctblQuit"))
					;
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'P':
				/*
				 * TODO: - PageFirstPage/PageLastPage was merged with GLOBAL_FIRST_PAGE/GLOBAL_LAST_PAGE
				 *       - PostPost with GLOBAL_POST "Post"
				 */
				if (STRCMPEQ(keyname, "PageAutoSel"))
					menu_filter_select[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageQuickAutoSel"))
					quick_filter_select[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageQuickKill"))
					quick_filter_kill[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageAutoKill"))
					menu_filter_kill[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageDown"))
					pagedown[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageDown2"))
					pagedown[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageDown3"))
					pagedown[2] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageEditFilter"))
					edit_filter[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageNextThd"))
					pagenextthd[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageNextThd2"))
					pagenextthd[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageUp"))
					pageup[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageUp2"))
					pageup[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageUp3"))
					pageup[2] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "Postponed"))
					postponed[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "Postponed2"))
					postponed[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PostPost"))
					postpost[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PostPost2"))
					postpost[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PostPost3"))
					postpost[2] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PostSend"))
					postsend[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PostSend2"))
					postsend[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageMarkArtUnread"))
					mark_article_unread[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageMarkThdUnread"))
					mark_thread_unread[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageCatchup"))
					catchup[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageCatchupNextUnread"))
					catchup_next_unread[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "PageToggleHeaders"))
					fprintf(newfp, "PageToggleRaw\t\t\t%s\n", keydef);
				else if (STRCMPEQ(keyname, "PromptNo") || STRCMPEQ(keyname, "PromptYes")) {
					if (strlen(keydef) == 1 && islower((unsigned char) keydef[0]))
						fprintf(newfp, "%s\t\t\t%c\t%c\n", keyname, keydef[0], my_toupper((unsigned char) keydef[0]));
					else
						fprintf(newfp, "%s", backup);
				} else
					fprintf(newfp, "%s", backup);
				break;

			case 'S':
				if (STRCMPEQ(keyname, "SelectEditFilter"))
					;
				else if (STRCMPEQ(keyname, "SelectEnterNextUnreadGrp"))
					selectentergroup[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "SelectEnterNextUnreadGrp2"))
					selectentergroup[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "SelectMarkGrpUnread"))
					selectmarkgrpunread[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "SelectMarkGrpUnread2"))
					selectmarkgrpunread[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "SelectReadGrp"))
					selectreadgrp[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "SelectReadGrp2"))
					selectreadgrp[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "SelectBugReport"))
					bugreport[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "SelectCatchup"))
					catchup[2] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "SelectCatchupNextUnread"))
					catchup_next_unread[2] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'T':
				if (STRCMPEQ(keyname, "ThreadEditFilter"))
					;
				else if (STRCMPEQ(keyname, "ThreadAutoSel"))
					menu_filter_select[2] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "ThreadKill"))
					menu_filter_kill[2] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "ThreadReadArt"))
					threadreadart[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "ThreadReadArt2"))
					threadreadart[1] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "ThreadBugReport"))
					bugreport[2] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "ThreadMarkArtUnread"))
					mark_article_unread[2] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "ThreadMarkThdUnread"))
					mark_thread_unread[2] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "ThreadCatchup"))
					catchup[3] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "ThreadCatchupNextUnread"))
					catchup_next_unread[3] = my_strdup(keydef);
				else
					fprintf(newfp, "%s", backup);
				break;

			case 'U':
				if (STRCMPEQ(keyname, "Up"))
					up[0] = my_strdup(keydef);
				else if (STRCMPEQ(keyname, "Up2"))
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
		if (bugreport[0]) {
			fprintf(newfp, "\t%s", bugreport[0]);
			if (bugreport[1] && !strcmp(bugreport[0], bugreport[1]))
				FreeAndNull(bugreport[1]);
			if (bugreport[2] && !strcmp(bugreport[0], bugreport[2]))
				FreeAndNull(bugreport[2]);
		}
		if (bugreport[1]) {
			fprintf(newfp, "\t%s", bugreport[1]);
			if (bugreport[2] && !strcmp(bugreport[1], bugreport[2]))
				FreeAndNull(bugreport[2]);
		}
		if (bugreport[2])
			fprintf(newfp, "\t%s", bugreport[2]);
		fprintf(newfp, "\n");
		FreeAndNull(bugreport[0]);
		FreeAndNull(bugreport[1]);
		FreeAndNull(bugreport[2]);
	}
	if (catchup[0] || catchup[1] || catchup[2] || catchup[3]) {
		fprintf(newfp, "Catchup\t");
		if (catchup[0]) {
			fprintf(newfp, "\t%s", catchup[0]);
			if (catchup[1] && !strcmp(catchup[0], catchup[1]))
				FreeAndNull(catchup[1]);
			if (catchup[2] && !strcmp(catchup[0], catchup[2]))
				FreeAndNull(catchup[2]);
			if (catchup[3] && !strcmp(catchup[0], catchup[3]))
				FreeAndNull(catchup[3]);
		}
		if (catchup[1]) {
			fprintf(newfp, "\t%s", catchup[1]);
			if (catchup[2] && !strcmp(catchup[1], catchup[2]))
				FreeAndNull(catchup[2]);
			if (catchup[3] && !strcmp(catchup[1], catchup[3]))
				FreeAndNull(catchup[3]);
		}
		if (catchup[2]) {
			fprintf(newfp, "\t%s", catchup[2]);
			if (catchup[3] && !strcmp(catchup[2], catchup[3]))
				FreeAndNull(catchup[3]);
		}
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
		if (catchup_next_unread[0]) {
			fprintf(newfp, "\t%s", catchup_next_unread[0]);
			if (catchup_next_unread[1] && !strcmp(catchup_next_unread[0], catchup_next_unread[1]))
				FreeAndNull(catchup_next_unread[1]);
			if (catchup_next_unread[2] && !strcmp(catchup_next_unread[0], catchup_next_unread[2]))
				FreeAndNull(catchup_next_unread[2]);
			if (catchup_next_unread[3] && !strcmp(catchup_next_unread[0], catchup_next_unread[3]))
				FreeAndNull(catchup_next_unread[3]);
		}
		if (catchup_next_unread[1]) {
			fprintf(newfp, "\t%s", catchup_next_unread[1]);
			if (catchup_next_unread[2] && !strcmp(catchup_next_unread[1], catchup_next_unread[2]))
				FreeAndNull(catchup_next_unread[2]);
			if (catchup_next_unread[3] && !strcmp(catchup_next_unread[1], catchup_next_unread[3]))
				FreeAndNull(catchup_next_unread[3]);
		}
		if (catchup_next_unread[2]) {
			fprintf(newfp, "\t%s", catchup_next_unread[2]);
			if (catchup_next_unread[3] && !strcmp(catchup_next_unread[2], catchup_next_unread[3]))
				FreeAndNull(catchup_next_unread[3]);
		}
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
		if (mark_article_unread[0]) {
			fprintf(newfp, "\t%s", mark_article_unread[0]);
			if (mark_article_unread[1] && !strcmp(mark_article_unread[0], mark_article_unread[1]))
				FreeAndNull(mark_article_unread[1]);
			if (mark_article_unread[2] && !strcmp(mark_article_unread[0], mark_article_unread[2]))
				FreeAndNull(mark_article_unread[2]);
		}
		if (mark_article_unread[1]) {
			fprintf(newfp, "\t%s", mark_article_unread[1]);
			if (mark_article_unread[2] && !strcmp(mark_article_unread[1], mark_article_unread[2]))
				FreeAndNull(mark_article_unread[2]);
		}
		if (mark_article_unread[2])
			fprintf(newfp, "\t%s", mark_article_unread[2]);
		fprintf(newfp, "\n");
		FreeAndNull(mark_article_unread[0]);
		FreeAndNull(mark_article_unread[1]);
		FreeAndNull(mark_article_unread[2]);
	}
	if (mark_thread_unread[0] || mark_thread_unread[1] || mark_thread_unread[2]) {
		fprintf(newfp, "MarkThreadUnread\t");
		if (mark_thread_unread[0]) {
			fprintf(newfp, "\t%s", mark_thread_unread[0]);
			if (mark_thread_unread[1] && !strcmp(mark_thread_unread[0], mark_thread_unread[1]))
				FreeAndNull(mark_thread_unread[1]);
			if (mark_thread_unread[2] && !strcmp(mark_thread_unread[0], mark_thread_unread[2]))
				FreeAndNull(mark_thread_unread[2]);
		}
		if (mark_thread_unread[1]) {
			fprintf(newfp, "\t%s", mark_thread_unread[1]);
			if (mark_thread_unread[2] && !strcmp(mark_thread_unread[1], mark_thread_unread[2]))
				FreeAndNull(mark_thread_unread[2]);
		}
		if (mark_thread_unread[2])
			fprintf(newfp, "\t%s", mark_thread_unread[2]);
		fprintf(newfp, "\n");
		FreeAndNull(mark_thread_unread[0]);
		FreeAndNull(mark_thread_unread[1]);
		FreeAndNull(mark_thread_unread[2]);
	}
	if (menu_filter_kill[0] || menu_filter_kill[1] || menu_filter_kill[2]) {
		fprintf(newfp, "MenuFilterKill\t");
		if (menu_filter_kill[0]) {
			fprintf(newfp, "\t%s", menu_filter_kill[0]);
			if (menu_filter_kill[1] && !strcmp(menu_filter_kill[0], menu_filter_kill[1]))
				FreeAndNull(menu_filter_kill[1]);
			if (menu_filter_kill[2] && !strcmp(menu_filter_kill[0], menu_filter_kill[2]))
				FreeAndNull(menu_filter_kill[2]);
		}
		if (menu_filter_kill[1]) {
			fprintf(newfp, "\t%s", menu_filter_kill[1]);
			if (menu_filter_kill[2] && !strcmp(menu_filter_kill[1], menu_filter_kill[2]))
				FreeAndNull(menu_filter_kill[2]);
		}
		if (menu_filter_kill[2])
			fprintf(newfp, "\t%s", menu_filter_kill[2]);
		fprintf(newfp, "\n");
		FreeAndNull(menu_filter_kill[0]);
		FreeAndNull(menu_filter_kill[1]);
		FreeAndNull(menu_filter_kill[2]);
	}
	if (menu_filter_select[0] || menu_filter_select[1] || menu_filter_select[2]) {
		fprintf(newfp, "MenuFilterSelect\t");
		if (menu_filter_select[0]) {
			fprintf(newfp, "\t%s", menu_filter_select[0]);
			if (menu_filter_select[1] && !strcmp(menu_filter_select[0], menu_filter_select[1]))
				FreeAndNull(menu_filter_select[1]);
			if (menu_filter_select[2] && !strcmp(menu_filter_select[0], menu_filter_select[2]))
				FreeAndNull(menu_filter_select[2]);
		}
		if (menu_filter_select[1]) {
			fprintf(newfp, "\t%s", menu_filter_select[1]);
			if (menu_filter_select[2] && !strcmp(menu_filter_select[1], menu_filter_select[2]))
				FreeAndNull(menu_filter_select[2]);
		}
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
	if ((errno = rename_file(newk, old)) != 0)
		perror_message(_(txt_rename_error), newk, old); /* TODO: unlink(newk)? */
	wait_message(0, _(txt_keymap_upgraded), KEYMAP_VERSION);
	prompt_continue();
}


/*
 * add the default key bindings for still free keys
 */
void
setup_default_keys(
	void)
{
	/* attachment level */
	add_default_key(&attachment_keys, "1", DIGIT_1);
	add_default_key(&attachment_keys, "2", DIGIT_2);
	add_default_key(&attachment_keys, "3", DIGIT_3);
	add_default_key(&attachment_keys, "4", DIGIT_4);
	add_default_key(&attachment_keys, "5", DIGIT_5);
	add_default_key(&attachment_keys, "6", DIGIT_6);
	add_default_key(&attachment_keys, "7", DIGIT_7);
	add_default_key(&attachment_keys, "8", DIGIT_8);
	add_default_key(&attachment_keys, "9", DIGIT_9);
	add_default_key(&attachment_keys, "b", GLOBAL_PAGE_UP);
	add_default_key(&attachment_keys, " ", GLOBAL_PAGE_DOWN);
	add_default_key(&attachment_keys, "h", GLOBAL_HELP);
	add_default_key(&attachment_keys, "\n\r", ATTACHMENT_SELECT);
	add_default_key(&attachment_keys, "H", GLOBAL_TOGGLE_HELP_DISPLAY);
	add_default_key(&attachment_keys, "", GLOBAL_REDRAW_SCREEN);
	add_default_key(&attachment_keys, "j", GLOBAL_LINE_DOWN);
	add_default_key(&attachment_keys, "k", GLOBAL_LINE_UP);
	add_default_key(&attachment_keys, "g^", GLOBAL_FIRST_PAGE);
	add_default_key(&attachment_keys, "G$", GLOBAL_LAST_PAGE);
	add_default_key(&attachment_keys, "i", GLOBAL_TOGGLE_INFO_LAST_LINE);
	add_default_key(&attachment_keys, "p", ATTACHMENT_PIPE);
	add_default_key(&attachment_keys, "q", GLOBAL_QUIT);
	add_default_key(&attachment_keys, "s", ATTACHMENT_SAVE);
	add_default_key(&attachment_keys, "t", ATTACHMENT_TAG);
	add_default_key(&attachment_keys, "v", GLOBAL_VERSION);
	add_default_key(&attachment_keys, "U", ATTACHMENT_UNTAG);
	add_default_key(&attachment_keys, "=", ATTACHMENT_TAG_PATTERN);
	add_default_key(&attachment_keys, "@", ATTACHMENT_TOGGLE_TAGGED);
	add_default_key(&attachment_keys, "|", GLOBAL_PIPE);
	add_default_key(&attachment_keys, ">", GLOBAL_SCROLL_DOWN);
	add_default_key(&attachment_keys, "<", GLOBAL_SCROLL_UP);
	add_default_key(&attachment_keys, "/", GLOBAL_SEARCH_SUBJECT_FORWARD);
	add_default_key(&attachment_keys, "?", GLOBAL_SEARCH_SUBJECT_BACKWARD);
	add_default_key(&attachment_keys, "\\", GLOBAL_SEARCH_REPEAT);
#ifndef NO_SHELL_ESCAPE
	add_default_key(&attachment_keys, "!", GLOBAL_SHELL_ESCAPE);
#endif /* !NO_SHELL_ESCAPE */

	/* scope level */
	add_default_key(&scope_keys, "1", DIGIT_1);
	add_default_key(&scope_keys, "2", DIGIT_2);
	add_default_key(&scope_keys, "3", DIGIT_3);
	add_default_key(&scope_keys, "4", DIGIT_4);
	add_default_key(&scope_keys, "5", DIGIT_5);
	add_default_key(&scope_keys, "6", DIGIT_6);
	add_default_key(&scope_keys, "7", DIGIT_7);
	add_default_key(&scope_keys, "8", DIGIT_8);
	add_default_key(&scope_keys, "9", DIGIT_9);
	add_default_key(&scope_keys, "a", SCOPE_ADD);
	add_default_key(&scope_keys, "b", GLOBAL_PAGE_UP);
	add_default_key(&scope_keys, " ", GLOBAL_PAGE_DOWN);
	add_default_key(&scope_keys, "d", SCOPE_DELETE);
	add_default_key(&scope_keys, "h", GLOBAL_HELP);
	add_default_key(&scope_keys, "\n\r", SCOPE_SELECT);
	add_default_key(&scope_keys, "E", SCOPE_EDIT_ATTRIBUTES_FILE);
	add_default_key(&scope_keys, "H", GLOBAL_TOGGLE_HELP_DISPLAY);
	add_default_key(&scope_keys, "", GLOBAL_REDRAW_SCREEN);
	add_default_key(&scope_keys, "m", SCOPE_MOVE);
	add_default_key(&scope_keys, "j", GLOBAL_LINE_DOWN);
	add_default_key(&scope_keys, "k", GLOBAL_LINE_UP);
	add_default_key(&scope_keys, "g^", GLOBAL_FIRST_PAGE);
	add_default_key(&scope_keys, "G$", GLOBAL_LAST_PAGE);
	add_default_key(&scope_keys, "q", GLOBAL_QUIT);
	add_default_key(&scope_keys, "r", SCOPE_RENAME);
	add_default_key(&scope_keys, ">", GLOBAL_SCROLL_DOWN);
	add_default_key(&scope_keys, "<", GLOBAL_SCROLL_UP);
#ifndef NO_SHELL_ESCAPE
	add_default_key(&scope_keys, "!", GLOBAL_SHELL_ESCAPE);
#endif /* !NO_SHELL_ESCAPE */

	/* select level */
	add_global_keys(&select_keys);
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
	add_default_key(&select_keys, "J", GLOBAL_CONNECTION_INFO);
#ifdef NNTP_ABLE
	add_default_key(&select_keys, "L", GLOBAL_LOOKUP_MESSAGEID);
#endif /* NNTP_ABLE */
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
	add_default_key(&group_keys, "", GLOBAL_MENU_FILTER_SELECT);
	add_default_key(&group_keys, "\n\r", GROUP_READ_BASENOTE);
	add_default_key(&group_keys, "", GLOBAL_MENU_FILTER_KILL);
	add_default_key(&group_keys, "", MARK_FEED_READ);
	add_default_key(&group_keys, "", MARK_FEED_UNREAD);
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
	add_default_key(&group_keys, "D", GROUP_CANCEL);
	add_default_key(&group_keys, "E", GLOBAL_EDIT_FILTER);
	add_default_key(&group_keys, "G", GROUP_TOGGLE_GET_ARTICLES_LIMIT);
	add_default_key(&group_keys, "J", GLOBAL_CONNECTION_INFO);
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
	add_default_key(&thread_keys, "", GLOBAL_MENU_FILTER_SELECT);
	add_default_key(&thread_keys, "", GLOBAL_MENU_FILTER_KILL);
	add_default_key(&thread_keys, "", MARK_FEED_READ);
	add_default_key(&thread_keys, "", MARK_FEED_UNREAD);
	add_default_key(&thread_keys, "\n\r", THREAD_READ_ARTICLE);
	add_default_key(&thread_keys, "a", GLOBAL_SEARCH_AUTHOR_FORWARD);
	add_default_key(&thread_keys, "c", CATCHUP);
	add_default_key(&thread_keys, "d", THREAD_TOGGLE_SUBJECT_DISPLAY);
	add_default_key(&thread_keys, "f", THREAD_FOLLOWUP_QUOTE);
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
	add_default_key(&thread_keys, "D", THREAD_CANCEL);
	add_default_key(&thread_keys, "E", GLOBAL_EDIT_FILTER);
	add_default_key(&thread_keys, "F", THREAD_FOLLOWUP);
	add_default_key(&thread_keys, "J", GLOBAL_CONNECTION_INFO);
	add_default_key(&thread_keys, "K", THREAD_MARK_ARTICLE_READ);
	add_default_key(&thread_keys, "L", GLOBAL_LOOKUP_MESSAGEID);
	add_default_key(&thread_keys, "S", THREAD_AUTOSAVE);
	add_default_key(&thread_keys, "T", THREAD_TAG_PARTS);
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
	add_default_key(&page_keys, "", GLOBAL_MENU_FILTER_SELECT);
	add_default_key(&page_keys, "", PAGE_REPLY_QUOTE_HEADERS);
#ifdef HAVE_PGP_GPG
	add_default_key(&page_keys, "", PAGE_PGP_CHECK_ARTICLE);
#endif /* HAVE_PGP_GPG */
	add_default_key(&page_keys, "", PAGE_TOGGLE_RAW);
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
	add_default_key(&page_keys, "J", GLOBAL_CONNECTION_INFO);
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
	add_default_key(&page_keys, "*", PAGE_TOGGLE_HEADERS);
	add_default_key(&page_keys, ":", PAGE_SKIP_INCLUDED_TEXT);
	add_default_key(&page_keys, "_", PAGE_TOGGLE_HIGHLIGHTING);
	add_default_key(&page_keys, "'", PAGE_ARTICLE_INFO);

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
	add_default_key(&option_menu_keys, "\t", CONFIG_TOGGLE_ATTRIB);
	add_default_key(&option_menu_keys, "", GLOBAL_REDRAW_SCREEN);
	add_default_key(&option_menu_keys, "j", GLOBAL_LINE_DOWN);
	add_default_key(&option_menu_keys, "k", GLOBAL_LINE_UP);
	add_default_key(&option_menu_keys, "g^", GLOBAL_FIRST_PAGE);
	add_default_key(&option_menu_keys, "G$", GLOBAL_LAST_PAGE);
	add_default_key(&option_menu_keys, "h", GLOBAL_HELP);
	add_default_key(&option_menu_keys, "q", GLOBAL_QUIT);
	add_default_key(&option_menu_keys, "r", CONFIG_RESET_ATTRIB);
	add_default_key(&option_menu_keys, "v", GLOBAL_VERSION);
	add_default_key(&option_menu_keys, "Q", CONFIG_NO_SAVE);
	add_default_key(&option_menu_keys, "S", CONFIG_SCOPE_MENU);
	add_default_key(&option_menu_keys, ">", GLOBAL_SCROLL_DOWN);
	add_default_key(&option_menu_keys, "<", GLOBAL_SCROLL_UP);
	add_default_key(&option_menu_keys, "/", GLOBAL_SEARCH_SUBJECT_FORWARD);
	add_default_key(&option_menu_keys, "?", GLOBAL_SEARCH_SUBJECT_BACKWARD);
	add_default_key(&option_menu_keys, "\\", GLOBAL_SEARCH_REPEAT);
#ifndef NO_SHELL_ESCAPE
	add_default_key(&option_menu_keys, "!", GLOBAL_SHELL_ESCAPE);
#endif /* !NO_SHELL_ESCAPE */

	/* posted articles level */
	add_default_key(&post_hist_keys, "", GLOBAL_ABORT);
	add_default_key(&post_hist_keys, "1", DIGIT_1);
	add_default_key(&post_hist_keys, "2", DIGIT_2);
	add_default_key(&post_hist_keys, "3", DIGIT_3);
	add_default_key(&post_hist_keys, "4", DIGIT_4);
	add_default_key(&post_hist_keys, "5", DIGIT_5);
	add_default_key(&post_hist_keys, "6", DIGIT_6);
	add_default_key(&post_hist_keys, "7", DIGIT_7);
	add_default_key(&post_hist_keys, "8", DIGIT_8);
	add_default_key(&post_hist_keys, "9", DIGIT_9);
	add_default_key(&post_hist_keys, "b", GLOBAL_PAGE_UP);
	add_default_key(&post_hist_keys, " ", GLOBAL_PAGE_DOWN);
	add_default_key(&post_hist_keys, "h", GLOBAL_HELP);
	add_default_key(&post_hist_keys, "\n\r", POSTED_SELECT);
	add_default_key(&post_hist_keys, "H", GLOBAL_TOGGLE_HELP_DISPLAY);
	add_default_key(&post_hist_keys, "I", GLOBAL_TOGGLE_INVERSE_VIDEO);
	add_default_key(&post_hist_keys, "", GLOBAL_REDRAW_SCREEN);
	add_default_key(&post_hist_keys, "j", GLOBAL_LINE_DOWN);
	add_default_key(&post_hist_keys, "k", GLOBAL_LINE_UP);
	add_default_key(&post_hist_keys, "g^", GLOBAL_FIRST_PAGE);
	add_default_key(&post_hist_keys, "G$", GLOBAL_LAST_PAGE);
	add_default_key(&post_hist_keys, "i", GLOBAL_TOGGLE_INFO_LAST_LINE);
	add_default_key(&post_hist_keys, "q", GLOBAL_QUIT);
	add_default_key(&post_hist_keys, "v", GLOBAL_VERSION);
	add_default_key(&post_hist_keys, ">", GLOBAL_SCROLL_DOWN);
	add_default_key(&post_hist_keys, "<", GLOBAL_SCROLL_UP);
	add_default_key(&post_hist_keys, "/", GLOBAL_SEARCH_SUBJECT_FORWARD);
	add_default_key(&post_hist_keys, "?", GLOBAL_SEARCH_SUBJECT_BACKWARD);
	add_default_key(&post_hist_keys, "\\", GLOBAL_SEARCH_REPEAT);
#ifndef NO_SHELL_ESCAPE
	add_default_key(&post_hist_keys, "!", GLOBAL_SHELL_ESCAPE);
#endif /* !NO_SHELL_ESCAPE */
#ifdef HAVE_COLOR
	add_default_key(&post_hist_keys, "&", GLOBAL_TOGGLE_COLOR);
#endif /* HAVE_COLOR */

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
	add_default_key(&feed_type_keys, "r", FEED_RANGE);
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

	/* url level */
	add_default_key(&url_keys, "", GLOBAL_ABORT);
	add_default_key(&url_keys, "1", DIGIT_1);
	add_default_key(&url_keys, "2", DIGIT_2);
	add_default_key(&url_keys, "3", DIGIT_3);
	add_default_key(&url_keys, "4", DIGIT_4);
	add_default_key(&url_keys, "5", DIGIT_5);
	add_default_key(&url_keys, "6", DIGIT_6);
	add_default_key(&url_keys, "7", DIGIT_7);
	add_default_key(&url_keys, "8", DIGIT_8);
	add_default_key(&url_keys, "9", DIGIT_9);
	add_default_key(&url_keys, "b", GLOBAL_PAGE_UP);
	add_default_key(&url_keys, " ", GLOBAL_PAGE_DOWN);
	add_default_key(&url_keys, "h", GLOBAL_HELP);
	add_default_key(&url_keys, "\n\r", URL_SELECT);
	add_default_key(&url_keys, "H", GLOBAL_TOGGLE_HELP_DISPLAY);
	add_default_key(&url_keys, "I", GLOBAL_TOGGLE_INVERSE_VIDEO);
	add_default_key(&url_keys, "", GLOBAL_REDRAW_SCREEN);
	add_default_key(&url_keys, "j", GLOBAL_LINE_DOWN);
	add_default_key(&url_keys, "k", GLOBAL_LINE_UP);
	add_default_key(&url_keys, "g^", GLOBAL_FIRST_PAGE);
	add_default_key(&url_keys, "G$", GLOBAL_LAST_PAGE);
	add_default_key(&url_keys, "i", GLOBAL_TOGGLE_INFO_LAST_LINE);
	add_default_key(&url_keys, "q", GLOBAL_QUIT);
	add_default_key(&url_keys, "v", GLOBAL_VERSION);
	add_default_key(&url_keys, ">", GLOBAL_SCROLL_DOWN);
	add_default_key(&url_keys, "<", GLOBAL_SCROLL_UP);
	add_default_key(&url_keys, "/", GLOBAL_SEARCH_SUBJECT_FORWARD);
	add_default_key(&url_keys, "?", GLOBAL_SEARCH_SUBJECT_BACKWARD);
	add_default_key(&url_keys, "\\", GLOBAL_SEARCH_REPEAT);
#ifndef NO_SHELL_ESCAPE
	add_default_key(&url_keys, "!", GLOBAL_SHELL_ESCAPE);
#endif /* !NO_SHELL_ESCAPE */
#ifdef HAVE_COLOR
	add_default_key(&url_keys, "&", GLOBAL_TOGGLE_COLOR);
#endif /* HAVE_COLOR */
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
	add_default_key(keys, " ", GLOBAL_PAGE_DOWN);
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
#endif /* !NO_SHELL_ESCAPE */
#ifdef HAVE_COLOR
	add_default_key(keys, "&", GLOBAL_TOGGLE_COLOR);
#endif /* HAVE_COLOR */
}


#if 0
/*
 * add dump_all_keys() to end of read_keymap_file() or the like
 * and pipe generated file through something like
 * 	sort|uniq|\
 * 	awk '/^[^#]/{a[$1]=a[$1]"\t"$2}END{for(x in a){print x"\t"a[x]}}'|sort
 * inspection
 */
#ifdef DEBUG
static void dump_keylist(const struct keylist keys);
static void dump_all_keys(void);

static void
dump_keylist(
	const struct keylist keys)
{
	char keybuf[MAXKEYLEN];
	const char *key_funcnames[] = {
		"#Unassigned",
		"#DIGIT_0",
		"#DIGIT_1",
		"#DIGIT_2",
		"#DIGIT_3",
		"#DIGIT_4",
		"#DIGIT_5",
		"#DIGIT_6",
		"#DIGIT_7",
		"#DIGIT_8",
		"#DIGIT_9",
		"AttachPipe",
		"AttachSave",
		"AttachSelect",
		"AttachTag",
		"AttachTagPattern",
		"AttachToggleTagged",
		"AttachUntag",
		"#SPECIAL_CATCHUP_LEFT",
		"#SPECIAL_MOUSE_TOGGLE",
		"Catchup",
		"CatchupNextUnread",
		"ConfigResetAttrib",
		"ConfigScopeMenu",
		"ConfigSelect",
		"ConfigNoSave",
		"ConfigToggleAttrib",
		"FeedArt",
		"FeedThd",
		"FeedHot",
		"FeedPat",
		"FeedRange",
		"FeedTag",
		"FeedRepost",
		"FeedSupersede",
		"FilterEdit",
		"FilterSave",
		"#GLOBAL_ABORT",
		"BugReport",
		"ConnectionInfo",
		"DisplayPostHist",
		"EditFilter",
		"FirstPage",
		"Help",
		"LastPage",
		"LastViewed",
		"Down",
		"Up",
		"LookupMessage",
		"MenuFilterKill",
		"MenuFilterSelect",
		"OptionMenu",
		"PageDown",
		"PageUp",
		"Pipe",
		"Post",
		"Postponed",
#	ifndef DISABLE_PRINTING
		"Print",
#	endif /* !DISABLE_PRINTING */
		"QuickFilterKill",
		"QuickFilterSelect",
		"Quit",
		"QuitTin",
		"RedrawScr",
		"ScrollDown",
		"ScrollUp",
		"SearchBody",
		"SearchRepeat",
		"SearchAuthB",
		"SearchAuthF",
		"SearchSubjB",
		"SearchSubjF",
		"SetRange",
#	ifndef NO_SHELL_ESCAPE
		"ShellEscape",
#	endif /* !NO_SHELL_ESCAPE */
#	ifdef HAVE_COLOR
		"ToggleColor",
#	endif /* HAVE_COLOR */
		"ToggleHelpDisplay",
		"ToggleInfoLastLine",
		"ToggleInverseVideo",
		"Version",
		"GroupAutoSave",
		"GroupCancel",
		"GroupDoAutoSel",
		"GroupGoto",
		"GroupListThd",
		"GroupMail",
		"GroupMarkThdRead",
		"GroupMarkUnselArtRead",
		"GroupNextGroup",
		"GroupNextUnreadArt",
		"GroupNextUnreadArtOrGrp",
		"GroupPrevGroup",
		"GroupPrevUnreadArt",
		"GroupReadBasenote",
		"GroupRepost",
		"GroupReverseSel",
		"GroupSave",
		"GroupSelPattern",
		"GroupSelThd",
		"GroupSelThdIfUnreadSelected",
		"GroupTag",
		"GroupTagParts",
		"GroupToggleGetartLimit",
		"GroupToggleReadUnread",
		"GroupToggleSubjDisplay",
		"GroupToggleThdSel",
		"GroupToggleThreading",
		"GroupUndoSel",
		"GroupUntag",
		"MarkArticleUnread",
		"MarkThreadUnread",
		"MarkFeedRead",
		"MarkFeedUnread",
		"PageArticleInfo",
		"PageAutoSave",
		"PageBotThd",
		"PageCancel",
		"PageEditArticle",
		"PageFollowup",
		"PageFollowupQuote",
		"PageFollowupQuoteHeaders",
		"PageGotoParent",
		"PageGroupSel",
		"PageListThd",
		"PageMail",
		"PageKillThd",
		"PageNextArt",
		"PageNextThd",
		"PageNextUnread",
		"PageNextUnreadArt",
#	ifdef HAVE_PGP_GPG
		"PagePGPCheckArticle",
#	endif /* HAVE_PGP_GPG */
		"PagePrevArt",
		"PagePrevUnreadArt",
		"PageReveal",
		"PageReply",
		"PageReplyQuote",
		"PageReplyQuoteHeaders",
		"PageRepost",
		"PageSave",
		"PageSkipIncludedText",
		"PageTag",
		"PageToggleAllHeaders",
		"PageToggleHighlight",
		"PageToggleRaw",
		"PageToggleRot",
		"PageToggleTabs",
		"PageToggleTex2iso",
		"PageToggleUue",
		"PageTopThd",
		"PageViewAttach",
		"PageViewUrl",
#	ifdef HAVE_PGP_GPG
		"PgpEncrypt",
		"PgpEncSign",
		"PgpIncludekey",
		"PgpSign",
#	endif /* HAVE_PGP_GPG */
		"PostAbort",
		"PostCancel",
		"PostContinue",
		"PostEdit",
		"PostIgnore",
#	ifdef HAVE_ISPELL
		"PostIspell",
#	endif /* HAVE_ISPELL */
		"PostMail",
#	ifdef HAVE_PGP_GPG
		"PostPGP",
#	endif /* HAVE_PGP_GPG */
		"PostPostpone",
		"PostSend",
		"PostSupersede",
		"PostedArticlesSelect",
		"PostponeAll",
		"PostponeOverride",
		"PProcNo",
		"PProcShar",
		"PProcYes",
		"PromptNo",
		"PromptYes",
		"SaveAppendFile",
		"SaveOverwriteFile",
		"ScopeAdd",
		"ScopeDelete",
		"ScopeEditAttributesFile",
		"ScopeMove",
		"ScopeRename",
		"ScopeSelect",
		"SelectReadGrp",
		"SelectEnterNextUnreadGrp",
		"SelectGoto",
		"SelectMarkGrpUnread",
		"SelectMoveGrp",
		"SelectNextUnreadGrp",
		"SelectResetNewsrc",
		"SelectSortActive",
		"SelectSubscribe",
		"SelectSubscribePat",
		"SelectSyncWithActive",
		"SelectToggleDescriptions",
		"SelectToggleReadDisplay",
		"SelectUnsubscribe",
		"SelectUnsubscribePat",
		"SelectQuitNoWrite",
		"SelectYankActive",
		"ThreadAutoSave",
		"ThreadCancel",
		"ThreadFollowup",
		"ThreadFollowupQuote",
		"ThreadMail",
		"ThreadMarkArtRead",
		"ThreadReadNextArtOrThread",
		"ThreadReadArt",
		"ThreadReverseSel",
		"ThreadSave",
		"ThreadSelArt",
		"ThreadTag",
		"ThreadTagParts",
		"ThreadToggleArtSel",
		"ThreadToggleSubjDisplay",
		"ThreadUndoSel",
		"ThreadUntag",
		"UrlSelect",
		NULL
	};
	size_t i;

	for (i = 0; i < keys.used; i++) {
		if (keys.list[i].key && keys.list[i].function)
			debug_print_file("KEYMAP", "%s\t%s", key_funcnames[keys.list[i].function], printascii(keybuf, keys.list[i].key));
	}
}


static void
dump_all_keys(
	void
)
{
	if (debug) { /* order? */
		dump_keylist(attachment_keys);
		dump_keylist(select_keys);
		dump_keylist(group_keys);
		dump_keylist(thread_keys);
		dump_keylist(option_menu_keys);
		dump_keylist(page_keys);
		dump_keylist(info_keys);
		dump_keylist(post_hist_keys);
		dump_keylist(post_send_keys);
		dump_keylist(post_edit_keys);
		dump_keylist(post_edit_ext_keys);
		dump_keylist(post_post_keys);
		dump_keylist(post_postpone_keys);
		dump_keylist(post_mail_fup_keys);
		dump_keylist(post_ignore_fupto_keys);
		dump_keylist(post_continue_keys);
		dump_keylist(post_delete_keys);
		dump_keylist(post_cancel_keys);
		dump_keylist(filter_keys);
#	ifdef HAVE_PGP_GPG
		dump_keylist(pgp_mail_keys);
		dump_keylist(pgp_news_keys);
#	endif /* HAVE_PGP_GPG */
		dump_keylist(save_append_overwrite_keys);
		dump_keylist(scope_keys);
		dump_keylist(feed_type_keys);
		dump_keylist(feed_post_process_keys);
		dump_keylist(feed_supersede_article_keys);
		dump_keylist(prompt_keys);
		dump_keylist(url_keys);
	}
}
#endif /* DEBUG */
#endif /* 0 */
