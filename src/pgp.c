/*
 *  Project   : tin - a Usenet reader
 *  Module    : pgp.c
 *  Author    : Steven J. Madsen
 *  Created   : 1995-05-12
 *  Updated   : 1999-12-02
 *  Notes     : PGP support
 *  Copyright : (c) 1995-99 by Steven J. Madsen
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

#ifdef HAVE_PGP

/* TODO - fix configure and remove this */
#ifndef HAVE_PGP_5
#	ifndef HAVE_GPG
#		define HAVE_PGP_2
#	endif /* !HAVE_GPG */
#endif /* !HAVE_PGP_5 */

/*
 * This option is valid for all supported packages. It may only be
 * be used when signing articles
 */
#	define LOCAL_USER		"-u %s"

/* TODO - avoid redefinitions */
/*
 * The first two args are the PGP command name and then $PGPOPTS
 */
#	ifdef HAVE_PGP_2
#		define PGPNAME		"pgp"
#		define PGPDIR		".pgp"
#		define PGP_PUBRING	"pubring.pgp"
#		define CHECK_SIGN	"%s %s -f <%s %s"
#		define ADD_KEY		"%s %s -ka %s"
#		define APPEND_KEY	"%s %s -kxa %s %s"
#		define DO_ENCRYPT	"%s %s -ate %s %s"
#		define DO_SIGN		"%s %s -ats %s %s"
#		define DO_BOTH		"%s %s -ates %s %s"
#	endif /* HAVE_PGP_2 */

#	ifdef HAVE_PGP_5
#		define PGPNAME		"pgp"
#		define PGPDIR		".pgp"
#		define PGP_PUBRING	"pubring.pkr"
#		define CHECK_SIGN	"%sv %s -f <%s %s"
#		define ADD_KEY		"%sk %s -a %s"
#		define APPEND_KEY	"%sk %s -xa %s -o %s"
#		define DO_ENCRYPT	"%se %s -at %s %s"
#		define DO_SIGN		"%ss %s -at %s %s"
#		define DO_BOTH		"%se %s -ats %s %s"
#	endif /* HAVE_PGP_5 */

#	ifdef HAVE_GPG
#		define PGPNAME		PATH_GPG
#		define PGPDIR		".gnupg"
#		define PGP_PUBRING	"pubring.gpg"
#		define CHECK_SIGN	"%s %s --no-batch --decrypt <%s %s"
#		define ADD_KEY		"%s %s --no-batch --import %s"
#		define APPEND_KEY	"%s %s --no-batch --armor --export %s --output %s"
#		define DO_ENCRYPT	"%s %s --no-batch --armor --textmode --output %s.asc --encrypt %s --recipient %s"
#		define DO_SIGN		"%s %s --no-batch --armor --textmode --output %s.asc --escape-from --clearsign %s"
#		define DO_BOTH		"%s %s --no-batch --armor --textmode --output %s.asc --sign --encrypt %s --recipient %s"
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
static t_bool pgp_available (void);
static void do_pgp (int what, const char *file, const char *mail_to);
static void join_files (const char *file);
static void pgp_append_public_key (char *file);
static void split_file (const char *file);

static char pgp_data[PATH_LEN];
static const char *pgpopts = "";

static char hdr[PATH_LEN], pt[PATH_LEN], ct[PATH_LEN];


void
init_pgp (
	void)
{
	char *ptr;

	pgpopts = get_val("PGPOPTS", "");

	if ((ptr = getenv("PGPPATH")) != (char *) 0)
		my_strncpy (pgp_data, ptr, sizeof(pgp_data));
	else
		joinpath (pgp_data, homedir, PGPDIR);
}


/*
 * Write the header file then the ciphertext file to the art file
 * This function is void, no way to return errs
 */
static void
join_files (
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
split_file (
	const char *file)
{
	FILE *art, *header, *plaintext;
	char buf[LEN];
	mode_t mask;

	snprintf(hdr, sizeof(hdr)-1, HEADERS, TMPDIR, process_id);
	snprintf(pt, sizeof(pt)-1, PLAINTEXT, TMPDIR, process_id);
	snprintf(ct, sizeof(ct)-1, CIPHERTEXT, TMPDIR, process_id);

	if ((art = fopen(file, "r")) == (FILE *) 0)
		return;

	mask = umask((mode_t) (S_IRWXO|S_IRWXG));

	if ((header = fopen(hdr, "w")) == (FILE *) 0)
		goto err_art;

	if ((plaintext = fopen(pt, "w")) == (FILE *) 0)
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
do_pgp (
	int what,
	const char *file,
	const char *mail_to)
{
	char address[LEN];
	char cmd[LEN];
	const char *mailto = (mail_to) ? mail_to : "";

	split_file(file);

	/*
	 * -u <from addr> is valid only when signing
	 */
	if (*tinrc.mail_address)
		strip_name (tinrc.mail_address, address);

	if (what & PGP_SIGN && *address != '\0') {
		char formatbuf[80];

		sprintf(formatbuf, "%s %s", (what & PGP_ENCRYPT) ? DO_BOTH : DO_SIGN, LOCAL_USER);
fprintf(stderr, "FMT: !%s!\n", formatbuf);
		sh_format (cmd, sizeof(cmd), formatbuf, PGPNAME, pgpopts, pt,
#ifdef HAVE_GPG
							pt,
#endif /* HAVE_GPG */
							mailto, address);
	} else
		sh_format (cmd, sizeof(cmd), DO_ENCRYPT, PGPNAME, pgpopts, pt,
#ifdef HAVE_GPG
							pt,
#endif /* HAVE_GPG */
							mailto);

fprintf(stderr, "PLOK: !%s!\n", cmd);
	invoke_cmd(cmd);
	join_files(file);
}


static void
pgp_append_public_key (
	char *file)
{
	FILE *fp, *key;
	char keyfile[PATH_LEN], cmd[LEN], buf[LEN];

	if (*tinrc.mail_address)
		strip_name (tinrc.mail_address, buf);
	else
		snprintf(buf, sizeof(buf)-1, "%s@%s", userid, host_name);

	snprintf(keyfile, sizeof(buf)-1, KEYFILE, TMPDIR, process_id);

/* TODO I'm guessing the pgp append key command creates 'keyfile' and that we should remove it */
	sh_format (cmd, sizeof(cmd), APPEND_KEY, PGPNAME, pgpopts, buf, keyfile);
	if (invoke_cmd (cmd)) {
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
pgp_available (
	void)
{
	FILE *fp;
	char keyring[PATH_LEN];

	joinpath(keyring, pgp_data, PGP_PUBRING);
	if ((fp = fopen(keyring, "r")) == (FILE *) 0) {
		wait_message(2, _(txt_pgp_not_avail), keyring);
		return FALSE;
	}

	fclose(fp);
	return TRUE;
}


void
invoke_pgp_mail (
	char *nam,
	char *mail_to)
{
	char ch, ch_default = 's';

	if (!pgp_available())
		return;

	ch = prompt_slk_response(ch_default, "beqs\033", _(txt_pgp_mail));
	switch (ch) {
		case ESC:
		case 'q':
			break;

		case 's':
#ifdef HAVE_PGP_5
			ClearScreen();
			MoveCursor (cLINES - 7, 0);
#endif /* HAVE_PGP_5 */
			do_pgp(PGP_SIGN, nam, NULL);
			break;

		case 'b':
#ifdef HAVE_PGP_5
			ClearScreen();
			MoveCursor (cLINES - 7, 0);
#endif /* HAVE_PGP_5 */
			do_pgp(PGP_SIGN | PGP_ENCRYPT, nam, mail_to);
			break;

		case 'e':
			do_pgp(PGP_ENCRYPT, nam, mail_to);
			break;

		default:
			break;
	}
}


void
invoke_pgp_news (
	char *artfile)
{
	char ch, ch_default = 's';

	if (!pgp_available())
		return;

	ch = prompt_slk_response(ch_default, "iqs\033", _(txt_pgp_news));
	switch (ch) {
		case ESC:
		case 'q':
			break;

		case 's':
#ifdef HAVE_PGP_5
			info_message (" ");
			MoveCursor (cLINES - 7, 0);
			my_printf("\n");
#endif /* HAVE_PGP_5 */
			do_pgp(PGP_SIGN, artfile, NULL);
			break;

		case 'i':
#ifdef HAVE_PGP_5
			info_message (" ");
			MoveCursor (cLINES - 7, 0);
			my_printf("\n");
#endif /* HAVE_PGP_5 */
			do_pgp(PGP_SIGN, artfile, NULL);
			pgp_append_public_key(artfile);
			break;

		default:
			break;
	}
}


t_bool
pgp_check_article (
	void)
{
	FILE *art;
	char artfile[PATH_LEN], buf[LEN], cmd[LEN];
	t_bool pgp_signed = FALSE;
	t_bool pgp_key = FALSE;

	if (!pgp_available())
		return FALSE;

	joinpath(artfile, homedir, ".article");

#	ifdef APPEND_PID
	snprintf (artfile+strlen(artfile), sizeof(artfile)-1, ".%d", (int) process_id);
#	endif /* APPEND_PID */

	if ((art = fopen(artfile, "w")) == (FILE *) 0) {
		info_message(_(txt_cannot_open), artfile);
		return FALSE;
	}
	fseek(note_fp, mark_body, SEEK_SET);		/* -> start of body */

	fgets(buf, LEN, note_fp);					/* Copy the body whilst looking for SIG/KEY tags */
	while (!feof(note_fp)) {
		if (!pgp_signed && strcmp(buf, PGP_SIG_TAG) == 0)
			pgp_signed = TRUE;
		if (!pgp_key && strcmp(buf, PGP_KEY_TAG) == 0)
			pgp_key = TRUE;
		fputs(buf, art);
		fgets(buf, LEN, note_fp);
	}
	fclose(art);

	if (!(pgp_signed || pgp_key)) {
		info_message(_(txt_pgp_nothing));
		return FALSE;
	}
	ClearScreen();

	if (pgp_signed) {
		Raw(FALSE);

		sh_format (cmd, sizeof(cmd), CHECK_SIGN, PGPNAME, pgpopts, artfile, REDIRECT_PGP_OUTPUT);
		invoke_cmd(cmd);
		my_printf("\n");
		Raw(TRUE);
	}

	if (pgp_key) {
		if (prompt_yn (cLINES, _(txt_pgp_add), FALSE) == 1) {
			Raw (FALSE);

			sh_format (cmd, sizeof(cmd), ADD_KEY, PGPNAME, pgpopts, artfile);
			invoke_cmd(cmd);
			my_printf ("\n");
			Raw (TRUE);
		}
	}

	continue_prompt ();
	unlink(artfile);
	return TRUE;
}
#endif /* HAVE_PGP */
