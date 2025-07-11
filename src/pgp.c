/*
 *  Project   : tin - a Usenet reader
 *  Module    : pgp.c
 *  Author    : Steven J. Madsen
 *  Created   : 1995-05-12
 *  Updated   : 2025-06-17
 *  Notes     : PGP support
 *
 * Copyright (c) 1995-2025 Steven J. Madsen <steve@erinet.com>
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

#ifdef HAVE_PGP_GPG
#	ifndef TCURSES_H
#		include "tcurses.h"
#	endif /* !TCURSES_H */


/*
 * The first two args are typically the PGP command name and then $PGPOPTS
 * NB: The '1' variations on DO_{SIGN,BOTH} are used when local-user name is
 * used and are valid only when signing
 */
#	if defined(HAVE_PGP) /* pgp-2 */
#		define PGPNAME		PATH_PGP
#		define PGPDIR		".pgp"
#		define PGP_PUBRING	"pubring.pgp"
#		define CHECK_SIGN	"%s %s -f <%s %s"
#		define ADD_KEY		"%s %s -ka %s"
#		define APPEND_KEY	"%s %s -kxa %s %s", PGPNAME, pgpopts, buf, keyfile
#		define DO_ENCRYPT	"%s %s -ate %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_SIGN		"%s %s -ats %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_SIGN1		"%s %s -ats %s %s -u %s", PGPNAME, pgpopts, pt, mailto, mailfrom
#		define DO_BOTH		"%s %s -ates %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_BOTH1		"%s %s -ates %s %s -u %s", PGPNAME, pgpopts, pt, mailto, mailfrom
#	endif /* HAVE_PGP */

#	if defined(HAVE_PGPK) /* pgp-5 */
#		define PGPNAME		"pgp"	/* FIXME: this is AFAIK not PATH_PGPK */
#		define PGPDIR		".pgp"
#		define PGP_PUBRING	"pubring.pkr"
#		define CHECK_SIGN	"%sv %s -f <%s %s"
#		define ADD_KEY		"%sk %s -a %s"
#		define APPEND_KEY	"%sk %s -xa %s -o %s", PGPNAME, pgpopts, keyfile, buf
#		define DO_ENCRYPT	"%se %s -at %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_SIGN		"%ss %s -at %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_SIGN1		"%ss %s -at %s %s -u %s", PGPNAME, pgpopts, pt, mailto, mailfrom
#		define DO_BOTH		"%se %s -ats %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_BOTH1		"%se %s -ats %s %s -u %s", PGPNAME, pgpopts, pt, mailto, mailfrom
#	endif /* HAVE_PGPK */

#	if defined(HAVE_GPG) /* gpg */
#		define PGPNAME		PATH_GPG
#		define PGPDIR		".gnupg"
#		define PGP_PUBRING	"pubring.gpg"
#		define PGP_PUBRING_KBX	"pubring.kbx"	/* since GnuPG 2.1 */
#		if 0 /* gpg 1.4.11 doesn't like this */
#			define CHECK_SIGN	"%s %s --no-batch --decrypt <%s %s"
#		else
#			define CHECK_SIGN	"%s %s < %s %s"
#		endif /* 0 */
#		define ADD_KEY		"%s %s --no-batch --import %s"
#		define APPEND_KEY	"%s %s --no-batch --armor --output %s --export %s", PGPNAME, pgpopts, keyfile, buf
#		define DO_ENCRYPT	\
"%s %s --textmode --armor --no-batch --output %s.asc --recipient %s --encrypt %s", \
PGPNAME, pgpopts, pt, mailto, pt
#		define DO_SIGN		\
"%s %s --textmode --armor --no-batch --output %s.asc --escape-from --clearsign %s", \
PGPNAME, pgpopts, pt, pt
#		define DO_SIGN1		\
"%s %s --textmode --armor --no-batch --local-user %s --output %s.asc --escape-from --clearsign %s", \
PGPNAME, pgpopts, mailfrom, pt, pt
#		define DO_BOTH		\
"%s %s --textmode --armor --no-batch --output %s.asc --recipient %s --sign --encrypt %s", \
PGPNAME, pgpopts, pt, mailto, pt
#		define DO_BOTH1		\
"%s %s --textmode --armor --no-batch --output %s.asc --recipient %s --local-user %s --sign --encrypt %s", \
PGPNAME, pgpopts, pt, mailto, mailfrom, pt
#	endif /* HAVE_GPG */

/* RFC 4880 6.2 */
#	define HEADERS	"tin-%ld.h"
#	ifdef HAVE_LONG_FILE_NAMES
#		define PLAINTEXT	"tin-%ld.pt"
#		define CIPHERTEXT	"tin-%ld.pt.asc"
#		define KEYFILE		"tin-%ld.k.asc"
#	else
#		define PLAINTEXT	"tn-%ld.p"
#		define CIPHERTEXT	"tn-%ld.p.asc"
#		define KEYFILE		"tn-%ld.k.asc"
#	endif /* HAVE_LONG_FILE_NAMES */


/*
 * local prototypes
 */
static t_bool pgp_available(void);
static void do_pgp(t_function what, const char *file, const char *mail_to);
static void join_files(const char *file);
static void pgp_append_public_key(const char *file);
static void split_file(const char *file);

static char pgp_data[PATH_LEN];
static char hdr[PATH_LEN], pt[PATH_LEN], ct[PATH_LEN];
static const char *pgpopts = "";


void
init_pgp(
	void)
{
	const char *ptr;

	pgpopts = get_val("PGPOPTS", "");

#	ifdef HAVE_GPG
	if ((ptr = get_val("GNUPGHOME", NULL)) != NULL)
		my_strncpy(pgp_data, ptr, sizeof(pgp_data) - 1);
	else
#	endif /* HAVE_GPG */
	{
		if ((ptr = get_val("PGPPATH", NULL)) != NULL)
			my_strncpy(pgp_data, ptr, sizeof(pgp_data) - 1);
		else
			joinpath(pgp_data, sizeof(pgp_data), homedir, PGPDIR);
	}
}


/*
 * Write the header file then the ciphertext file to the art file
 * This function is void, no way to return errs
 */
static void
join_files(
	const char *file)
{
	FILE *art, *header, *text;

	if ((header = tin_fopen(hdr, "r")) == NULL)
		return;
	if ((text = tin_fopen(ct, "r")) == NULL) {
		fclose(header);
		return;
	}
	if ((art = fopen(file, "w")) != NULL) {
		if (copy_fp(header, art) == 0)
			copy_fp(text, art);
		fclose(art);
	}
	fclose(text);
	fclose(header);
	unlink(hdr);
	unlink(pt);
	unlink(ct);
}


/*
 * Split the file parameter into a header file 'hdr' and a plaintext
 * file 'pt'
 */
static void
split_file(
	const char *file)
{
	FILE *art, *header, *plaintext;
	char buf[LEN];
	char tmp[PATH_LEN];
	mode_t mask;

	snprintf(tmp, sizeof(tmp), HEADERS, (long) process_id);
	joinpath(hdr, sizeof(hdr), tmpdir, tmp);
	snprintf(tmp, sizeof(tmp), PLAINTEXT, (long) process_id);
	joinpath(pt, sizeof(pt), tmpdir, tmp);
	snprintf(tmp, sizeof(tmp), CIPHERTEXT, (long) process_id);
	joinpath(ct, sizeof(ct), tmpdir, tmp);

	if ((art = tin_fopen(file, "r")) == NULL)
		return;

	mask = umask((mode_t) (S_IRWXO|S_IRWXG));

	if ((header = fopen(hdr, "w")) == NULL)
		goto err_art;

	if ((plaintext = fopen(pt, "w")) == NULL)
		goto err_hdr;

	if (fgets(buf, sizeof(buf), art) != NULL) {			/* Copy the hdr up to and including the \n */
		t_bool success = TRUE;

		while (*buf != '\n') {
			fputs(buf, header);
			if (fgets(buf, LEN, art) == NULL) {
				success = FALSE;
				break;
			}
		}
		if (success)
			fputs(buf, header);
		else
			fputs("\n", header);
		copy_fp(art, plaintext);
	}

	fclose(plaintext);
err_hdr:
	fclose(header);
err_art:
	fclose(art);
	umask(mask);
}


static void
do_pgp(
	t_function what,
	const char *file,
	const char *mail_to)
{
	char cmd[LEN];
	char mailfrom[LEN];
	const char *mailto = BlankIfNull(mail_to);

	mailfrom[0] = '\0';

	split_file(file);

	/*
	 * <mailfrom> is valid only when signing and a local address exists
	 */
	if (CURR_GROUP.attribute->from != NULL && *CURR_GROUP.attribute->from && strchr(*CURR_GROUP.attribute->from, '@'))
		strip_name(*CURR_GROUP.attribute->from, mailfrom);

	switch (what) {
		case PGP_KEY_SIGN:
			if (*mailfrom)
				sh_format(cmd, sizeof(cmd), DO_SIGN1);
			else
				sh_format(cmd, sizeof(cmd), DO_SIGN);
			invoke_cmd(cmd);
			break;

		case PGP_KEY_ENCRYPT_SIGN:
			if (*mailfrom)
				sh_format(cmd, sizeof(cmd), DO_BOTH1);
			else
				sh_format(cmd, sizeof(cmd), DO_BOTH);
			invoke_cmd(cmd);
			break;

		case PGP_KEY_ENCRYPT:
			sh_format(cmd, sizeof(cmd), DO_ENCRYPT);
			invoke_cmd(cmd);
			break;

		default:
			break;
	}

	join_files(file);
}


static void
pgp_append_public_key(
	const char *file)
{
	FILE *fp, *key;
	char cmd[LEN], buf[LEN];
	char keyfile[PATH_LEN], tmp[PATH_LEN];

	if (CURR_GROUP.attribute->from != NULL && *CURR_GROUP.attribute->from && strchr(*CURR_GROUP.attribute->from, '@'))
		strip_name(*CURR_GROUP.attribute->from, buf);
	else /* FIXME: avoid hardcoded length */
		snprintf(buf, sizeof(buf), "%.*s@%.765s", LOGIN_NAME_MAX - 1, userid, BlankIfNull(get_host_name()));

	snprintf(tmp, sizeof(tmp), KEYFILE, (long) process_id);
	joinpath(keyfile, sizeof(keyfile), tmpdir, tmp);

/*
 * TODO: I'm guessing the pgp append key command creates 'keyfile' and that
 * we should remove it
 */
	sh_format(cmd, sizeof(cmd), APPEND_KEY);
	if (invoke_cmd(cmd)) {
		if ((fp = fopen(file, "a")) != NULL) {
			if ((key = tin_fopen(keyfile, "r")) != NULL) {
				fputc('\n', fp);			/* Add a blank line */
				copy_fp(key, fp);			/* and copy in the key */
				fclose(key);
			}
			fclose(fp);
		}
		unlink(keyfile);
	}
}


/*
 * Simply check for existence of keyring file
 */
static t_bool
pgp_available(
	void)
{
	FILE *fp;
	char keyring[PATH_LEN];

#ifdef HAVE_GPG
	/* GnuPG >= 2.1 may use PGP_PUBRING_KBX */
	joinpath(keyring, sizeof(keyring), pgp_data, PGP_PUBRING_KBX);
	if ((fp = fopen(keyring, "r")) != NULL) {
		fclose(fp);
		return TRUE;
	}
#endif /* HAVE_GPG */

	joinpath(keyring, sizeof(keyring), pgp_data, PGP_PUBRING);
	if ((fp = fopen(keyring, "r")) != NULL) {
		fclose(fp);
		return TRUE;
	}

	wait_message(2, _(txt_pgp_not_avail), keyring);
	return FALSE;
}


void
invoke_pgp_mail(
	const char *nam,
	const char *mail_to)
{
	char keyboth[MAXKEYLEN], keyencrypt[MAXKEYLEN], keyquit[MAXKEYLEN];
	char keysign[MAXKEYLEN];
	t_function func, default_func = PGP_KEY_SIGN;

	if (!pgp_available())
		return;

	func = prompt_slk_response(default_func, pgp_mail_keys, _(txt_pgp_mail),
			PrintFuncKey(keyencrypt, PGP_KEY_ENCRYPT, pgp_mail_keys),
			PrintFuncKey(keysign, PGP_KEY_SIGN, pgp_mail_keys),
			PrintFuncKey(keyboth, PGP_KEY_ENCRYPT_SIGN, pgp_mail_keys),
			PrintFuncKey(keyquit, GLOBAL_QUIT, pgp_mail_keys));
	switch (func) {
		case PGP_KEY_SIGN:
#	ifdef HAVE_PGPK
			ClearScreen();
			MoveCursor(cLINES - 7, 0);
#	endif /* HAVE_PGPK */
			do_pgp(func, nam, NULL);
			break;

		case PGP_KEY_ENCRYPT_SIGN:
#	ifdef HAVE_PGPK
			ClearScreen();
			MoveCursor(cLINES - 7, 0);
#	endif /* HAVE_PGPK */
			do_pgp(func, nam, mail_to);
			break;

		case PGP_KEY_ENCRYPT:
			do_pgp(func, nam, mail_to);
			break;

		case GLOBAL_ABORT:
		case GLOBAL_QUIT:
		default:
			break;
	}
}


void
invoke_pgp_news(
	const char *artfile)
{
	char keyinclude[MAXKEYLEN], keyquit[MAXKEYLEN], keysign[MAXKEYLEN];
	t_function func, default_func = PGP_KEY_SIGN;

	if (!pgp_available())
		return;

	func = prompt_slk_response(default_func, pgp_news_keys, _(txt_pgp_news),
				PrintFuncKey(keysign, PGP_KEY_SIGN, pgp_news_keys),
				PrintFuncKey(keyinclude, PGP_INCLUDE_KEY, pgp_news_keys),
				PrintFuncKey(keyquit, GLOBAL_QUIT, pgp_news_keys));
	switch (func) {
		case GLOBAL_ABORT:
		case GLOBAL_QUIT:
			break;

		case PGP_KEY_SIGN:
#	ifdef HAVE_PGPK
			info_message(" ");
			MoveCursor(cLINES - 7, 0);
			my_printf("\n");
#	endif /* HAVE_PGPK */
			do_pgp(func, artfile, NULL);
			break;

		case PGP_INCLUDE_KEY:
#	ifdef HAVE_PGPK
			info_message(" ");
			MoveCursor(cLINES - 7, 0);
			my_printf("\n");
#	endif /* HAVE_PGPK */
			do_pgp(PGP_KEY_SIGN, artfile, NULL);
			pgp_append_public_key(artfile);
			break;

		default:
			break;
	}
}


t_bool
pgp_check_article(
	t_openartinfo *artinfo)
{
	FILE *art;
	char artfile[PATH_LEN], buf[LEN], *cmd;
	int n;
	size_t len;
	t_bool pgp_signed = FALSE;
	int pgp_key = 0;

	if (!pgp_available())
		return FALSE;

	joinpath(artfile, sizeof(artfile), homedir, TIN_ARTICLE_NAME);
#	ifdef APPEND_PID
	snprintf(artfile + strlen(artfile), sizeof(artfile) - strlen(artfile), ".%ld", (long) process_id);
#	endif /* APPEND_PID */
	if ((art = fopen(artfile, "w")) == NULL) {
		info_message(_(txt_cannot_open), artfile);
		return FALSE;
	}
	/* -> start of body */
	if (fseek(artinfo->raw, artinfo->hdr.ext->offset, SEEK_SET) != 0) {
		fclose(art);
		return FALSE;
	}

	while (!feof(artinfo->raw) && !ferror(artinfo->raw)) {
		if (fgets(buf, sizeof(buf), artinfo->raw) != NULL) {
			if (!pgp_signed && !strcmp(buf, PGP_SIG_TAG))
				pgp_signed = TRUE;
			if (!strcmp(buf, PGP_PUBLIC_KEY_TAG))
				++pgp_key;
			fputs(buf, art);
		} else
			break;
	}
	fclose(art);

	if (!pgp_signed && !pgp_key) {
		info_message(_(txt_pgp_nothing));
		return FALSE;
	}
	ClearScreen();

	if (pgp_signed) {
		Raw(FALSE);
		/*
		 * We don't use sh_format here else the redirection gets misquoted
		 */
		if ((n = snprintf(NULL, 0, CHECK_SIGN, PGPNAME, pgpopts, artfile, REDIRECT_PGP_OUTPUT)) > 0) {
			len = (size_t) n + 1;
			cmd = my_malloc(len);
			if (snprintf(cmd, len, CHECK_SIGN, PGPNAME, pgpopts, artfile, REDIRECT_PGP_OUTPUT) == n)
				invoke_cmd(cmd);
			/* TODO: useful error message */
			free(cmd);
			my_printf("\n");
		}
		Raw(TRUE);
	}
#	ifndef USE_CURSES
	EndWin();
	Raw(FALSE);
#	endif /* !USE_CURSES */
	prompt_continue();
#	ifndef USE_CURSES
	Raw(TRUE);
	InitWin();
#	endif /* !USE_CURSES */
	if (pgp_key) {
		if (prompt_yn(P_(txt_pgp_add_sp[0], txt_pgp_add_sp[1], pgp_key), FALSE) == 1) {
			Raw(FALSE);
			n = snprintf(NULL, 0, ADD_KEY, PGPNAME, pgpopts, artfile);
			len = (size_t) (n << 1) + 1; /* double size for quoting */
			cmd = my_malloc(len);
			sh_format(cmd, len, ADD_KEY, PGPNAME, pgpopts, artfile);
			invoke_cmd(cmd);
			free(cmd);
			my_printf("\n");
			Raw(TRUE);
		}
	}

	unlink(artfile);
	return TRUE;
}
#endif /* HAVE_PGP_GPG */
