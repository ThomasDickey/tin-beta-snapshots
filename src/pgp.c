/*
 *  Project   : tin - a Usenet reader
 *  Module    : pgp.c
 *  Author    : Steven J. Madsen
 *  Created   : 1995-05-12
 *  Updated   : 1999-12-02
 *  Notes     : PGP support
 *
 * Copyright (c) 1995-2002 Steven J. Madsen <steve@erinet.com>
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

#ifdef HAVE_PGP_GPG

/*
 * The first two args are typically the PGP command name and then $PGPOPTS
 * NB: The '1' variations on DO_{SIGN,BOTH} are used when local-user name is
 * used and are valid only when signing
 */
#	if defined(HAVE_PGP) /* && !defined(PGPNAME) */ /* pgp-2 */
#		define PGPNAME		PATH_PGP
#		define PGPDIR		".pgp"
#		define PGP_PUBRING	"pubring.pgp"
#		define CHECK_SIGN	"%s %s -f <%s %s"
#		define ADD_KEY		"%s %s -ka %s"
#		define APPEND_KEY	"%s %s -kxa %s %s"
#		define DO_ENCRYPT	"%s %s -ate %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_SIGN		"%s %s -ats %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_SIGN1		"%s %s -ats %s %s -u %s", PGPNAME, pgpopts, pt, mailto, mailfrom
#		define DO_BOTH		"%s %s -ates %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_BOTH1		"%s %s -ates %s %s -u %s", PGPNAME, pgpopts, pt, mailto, mailfrom
#	endif /* HAVE_PGP */

#	if defined(HAVE_PGPK) /* && !defined(PGPNAME) */ /* pgp-5 */
#		define PGPNAME		"pgp"	/* FIXME: this is AFAIK not PATH_PGPK */
#		define PGPDIR		".pgp"
#		define PGP_PUBRING	"pubring.pkr"
#		define CHECK_SIGN	"%sv %s -f <%s %s"
#		define ADD_KEY		"%sk %s -a %s"
#		define APPEND_KEY	"%sk %s -xa %s -o %s"
#		define DO_ENCRYPT	"%se %s -at %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_SIGN		"%ss %s -at %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_SIGN1		"%ss %s -at %s %s -u %s", PGPNAME, pgpopts, pt, mailto, mailfrom
#		define DO_BOTH		"%se %s -ats %s %s", PGPNAME, pgpopts, pt, mailto
#		define DO_BOTH1		"%se %s -ats %s %s -u %s", PGPNAME, pgpopts, pt, mailto, mailfrom
#	endif /* HAVE_PGPK */

#	if defined(HAVE_GPG) && !defined(PGPNAME) /* gpg */
#		define PGPNAME		PATH_GPG
#		define PGPDIR		".gnupg"
#		define PGP_PUBRING	"pubring.gpg"
#		define CHECK_SIGN	"%s %s --no-batch --decrypt <%s %s"
#		define ADD_KEY		"%s %s --no-batch --import %s"
#		define APPEND_KEY	"%s %s --no-batch --armor --output %s --export %s"
#		define LOCAL_USER	"--local-user %s"
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

#	define PGP_SIG_TAG "-----BEGIN PGP SIGNED MESSAGE-----\n"
#	define PGP_KEY_TAG "-----BEGIN PGP PUBLIC KEY BLOCK-----\n"

#	define HEADERS	"%stin-%d.h"
#	ifdef HAVE_LONG_FILE_NAMES
#		define PLAINTEXT	"%stin-%d.pt"
#		define CIPHERTEXT	"%stin-%d.pt.asc"
#		define KEYFILE		"%stin-%d.k.asc"
#	else
#		define PLAINTEXT	"%stn-%d.p"
#		define CIPHERTEXT	"%stn-%d.p.asc"
#		define KEYFILE		"%stn-%d.k.asc"
#	endif /* HAVE_LONG_FILE_NAMES */


#	define PGP_SIGN 0x01
#	define PGP_ENCRYPT 0x02

/*
 * local prototypes
 */
static t_bool pgp_available(void);
static void do_pgp(int what, const char *file, const char *mail_to);
static void join_files(const char *file);
static void pgp_append_public_key(char *file);
static void split_file(const char *file);

static char pgp_data[PATH_LEN];
static char hdr[PATH_LEN], pt[PATH_LEN], ct[PATH_LEN];
static const char *pgpopts = "";


void
init_pgp(
	void)
{
	char *ptr;

	pgpopts = get_val("PGPOPTS", "");

#ifdef HAVE_GPG
	if ((ptr = getenv("GNUPGHOME")) != NULL)
		my_strncpy(pgp_data, ptr, sizeof(pgp_data));
	else
#endif /* HAVE_GPG */
	{
		if ((ptr = getenv("PGPPATH")) != NULL)
			my_strncpy(pgp_data, ptr, sizeof(pgp_data));
		else
			joinpath(pgp_data, homedir, PGPDIR);
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

	if ((header = fopen(hdr, "r")) != NULL) {
		if ((text = fopen(ct, "r")) != NULL) {
			if ((art = fopen(file, "w")) != NULL) {
				if (copy_fp(header, art))
					copy_fp(text, art);
				fclose(art);
			}
			fclose(text);
		}
		fclose(header);
	}

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
	mode_t mask;

	snprintf(hdr, sizeof(hdr) - 1, HEADERS, TMPDIR, process_id);
	snprintf(pt, sizeof(pt) - 1, PLAINTEXT, TMPDIR, process_id);
	snprintf(ct, sizeof(ct) - 1, CIPHERTEXT, TMPDIR, process_id);

	if ((art = fopen(file, "r")) == NULL)
		return;

	mask = umask((mode_t) (S_IRWXO|S_IRWXG));

	if ((header = fopen(hdr, "w")) == NULL)
		goto err_art;

	if ((plaintext = fopen(pt, "w")) == NULL)
		goto err_hdr;

	fgets(buf, LEN, art);			/* Copy the hdr up to and including the \n */
	while (strcmp(buf, "\n")) {
		fputs(buf, header);
		fgets(buf, LEN, art);
	}
	fputs(buf, header);
	copy_fp(art, plaintext);

	fclose(plaintext);
err_hdr:
	fclose(header);
err_art:
	fclose(art);
	umask(mask);
}


static void
do_pgp(
	int what,
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
	if (what & PGP_SIGN) {
		if ((CURR_GROUP.attribute->from) != NULL)
			strip_name(CURR_GROUP.attribute->from, mailfrom);
		if (strlen(mailfrom)) {
			if (what & PGP_ENCRYPT)
				sh_format(cmd, sizeof(cmd), DO_BOTH1);
			else
				sh_format(cmd, sizeof(cmd), DO_SIGN1);
		} else {
			if (what & PGP_ENCRYPT)
				sh_format(cmd, sizeof(cmd), DO_BOTH);
			else
				sh_format(cmd, sizeof(cmd), DO_SIGN);
		}
	} else
		sh_format(cmd, sizeof(cmd), DO_ENCRYPT);

	invoke_cmd(cmd);
	join_files(file);
}


static void
pgp_append_public_key(
	char *file)
{
	FILE *fp, *key;
	char keyfile[PATH_LEN], cmd[LEN], buf[LEN];

	if ((CURR_GROUP.attribute->from) != NULL && strlen(CURR_GROUP.attribute->from))
		strip_name(CURR_GROUP.attribute->from, buf);
	else
		snprintf(buf, sizeof(buf) - 1, "%s@%s", userid, host_name);

	snprintf(keyfile, sizeof(keyfile) - 1, KEYFILE, TMPDIR, process_id);

/*
 * TODO: I'm guessing the pgp append key command creates 'keyfile' and that
 * we should remove it
 */
	sh_format(cmd, sizeof(cmd), APPEND_KEY, PGPNAME, pgpopts, keyfile, buf);

	if (invoke_cmd(cmd)) {
		if ((fp = fopen(file, "a")) != NULL) {
			if ((key = fopen(keyfile, "r")) != NULL) {
				fputc('\n', fp);			/* Add a blank line */
				copy_fp(key, fp);			/* and copy in the key */
				fclose(key);
			}
			fclose(fp);
		}
		unlink(keyfile);
	}
	return;
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

	joinpath(keyring, pgp_data, PGP_PUBRING);
	if ((fp = fopen(keyring, "r")) == NULL) {
		wait_message(2, _(txt_pgp_not_avail), keyring);
		return FALSE;
	}

	fclose(fp);
	return TRUE;
}


void
invoke_pgp_mail(
	const char *nam,
	char *mail_to)
{
	char ch, ch_default = iKeyPgpSign;
	char keyboth[MAXKEYLEN], keyencrypt[MAXKEYLEN], keyquit[MAXKEYLEN];
	char keysign[MAXKEYLEN];

	if (!pgp_available())
		return;

	ch = prompt_slk_response(ch_default, &menukeymap.pgp_mail,
			_(txt_pgp_mail),
			printascii(keyencrypt, map_to_local(iKeyPgpEncrypt, &menukeymap.pgp_mail)),
			printascii(keysign, map_to_local(iKeyPgpSign, &menukeymap.pgp_mail)),
			printascii(keyboth, map_to_local(iKeyPgpEncSign, &menukeymap.pgp_mail)),
			printascii(keyquit, map_to_local(iKeyQuit, &menukeymap.pgp_mail)));
	switch (ch) {
		case iKeyAbort:
		case iKeyQuit:
			break;

		case iKeyPgpSign:
#ifdef HAVE_PGPK
			ClearScreen();
			MoveCursor(cLINES - 7, 0);
#endif /* HAVE_PGPK */
			do_pgp(PGP_SIGN, nam, NULL);
			break;

		case iKeyPgpEncSign:
#ifdef HAVE_PGPK
			ClearScreen();
			MoveCursor(cLINES - 7, 0);
#endif /* HAVE_PGPK */
			do_pgp(PGP_SIGN | PGP_ENCRYPT, nam, mail_to);
			break;

		case iKeyPgpEncrypt:
			do_pgp(PGP_ENCRYPT, nam, mail_to);
			break;

		default:
			break;
	}
}


void
invoke_pgp_news(
	char *artfile)
{
	char ch, ch_default = iKeyPgpSign;
	char keyinclude[MAXKEYLEN], keyquit[MAXKEYLEN], keysign[MAXKEYLEN];

	if (!pgp_available())
		return;

	ch = prompt_slk_response(ch_default, &menukeymap.pgp_news,
				_(txt_pgp_news),
				printascii(keysign, map_to_local(iKeyPgpSign, &menukeymap.pgp_news)),
				printascii(keyinclude, map_to_local(iKeyPgpIncludekey, &menukeymap.pgp_news)),
				printascii(keyquit, map_to_local(iKeyQuit, &menukeymap.pgp_news)));
	switch (ch) {
		case iKeyAbort:
		case iKeyQuit:
			break;

		case iKeyPgpSign:
#ifdef HAVE_PGPK
			info_message(" ");
			MoveCursor(cLINES - 7, 0);
			my_printf("\n");
#endif /* HAVE_PGPK */
			do_pgp(PGP_SIGN, artfile, NULL);
			break;

		case iKeyPgpIncludekey:
#ifdef HAVE_PGPK
			info_message(" ");
			MoveCursor(cLINES - 7, 0);
			my_printf("\n");
#endif /* HAVE_PGPK */
			do_pgp(PGP_SIGN, artfile, NULL);
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
	char artfile[PATH_LEN], buf[LEN], cmd[LEN];
	t_bool pgp_signed = FALSE;
	t_bool pgp_key = FALSE;

	if (!pgp_available())
		return FALSE;

	joinpath(artfile, homedir, TIN_ARTICLE_NAME);

#	ifdef APPEND_PID
	snprintf(artfile + strlen(artfile), sizeof(artfile) - 1, ".%d", (int) process_id);
#	endif /* APPEND_PID */

	if ((art = fopen(artfile, "w")) == NULL) {
		info_message(_(txt_cannot_open), artfile);
		return FALSE;
	}
	fseek(artinfo->raw, artinfo->hdr.ext->offset, SEEK_SET);		/* -> start of body */

	fgets(buf, LEN, artinfo->raw);		/* Copy the body whilst looking for SIG/KEY tags */
	while (!feof(artinfo->raw)) {
		if (!pgp_signed && !strcmp(buf, PGP_SIG_TAG))
			pgp_signed = TRUE;
		if (!pgp_key && !strcmp(buf, PGP_KEY_TAG))
			pgp_key = TRUE;
		fputs(buf, art);
		fgets(buf, LEN, artinfo->raw);
	}
	fclose(art);

	if (!(pgp_signed || pgp_key)) {
		info_message(_(txt_pgp_nothing));
		return FALSE;
	}
	ClearScreen();

	if (pgp_signed) {
		Raw(FALSE);

		/*
		 * We don't use sh_format here else the redirection get misquoted
		 */
		snprintf(cmd, sizeof(cmd) - 1, CHECK_SIGN, PGPNAME, pgpopts, artfile, REDIRECT_PGP_OUTPUT);
		invoke_cmd(cmd);
		my_printf("\n");
		Raw(TRUE);
	}

	if (pgp_key) {
		if (prompt_yn(cLINES, _(txt_pgp_add), FALSE) == 1) {
			Raw(FALSE);

			sh_format(cmd, sizeof(cmd), ADD_KEY, PGPNAME, pgpopts, artfile);
			invoke_cmd(cmd);
			my_printf("\n");
			Raw(TRUE);
		}
	}

	prompt_continue();
	unlink(artfile);
	return TRUE;
}
#endif /* HAVE_PGP_GPG */
