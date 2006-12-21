/*
 *  Project   : tin - a Usenet reader
 *  Module    : nntplib.c
 *  Author    : S. Barber & I. Lea
 *  Created   : 1991-01-12
 *  Updated   : 2006-10-24
 *  Notes     : NNTP client routines taken from clientlib.c 1.5.11 (1991-02-10)
 *  Copyright : (c) Copyright 1991-99 by Stan Barber & Iain Lea
 *              Permission is hereby granted to copy, reproduce, redistribute
 *              or otherwise use this software  as long as: there is no
 *              monetary  profit  gained  specifically  from the use or
 *              reproduction or this software, it is not  sold, rented,
 *              traded or otherwise marketed, and this copyright notice
 *              is included prominently in any copy made.
 */


#ifndef TIN_H
#	include "tin.h"
#endif /* !TIN_H */
#ifndef TCURSES_H
#	include "tcurses.h"
#endif /* !TCURSES_H */
#ifndef TNNTP_H
#	include "tnntp.h"
#endif /* !TNNTP_H */

char *nntp_server = NULL;
#ifdef NO_POSTING
	t_bool can_post = FALSE;
#else
	t_bool can_post = TRUE;
#endif /* NO_POSTING */

/* Flag to show whether tin did reconnect in last get_server() */
t_bool reconnected_in_last_get_server = FALSE;

static TCP *nntp_rd_fp = NULL;
static TCP *nntp_wr_fp = NULL;

#ifdef NNTP_ABLE
	/* Copy of last NNTP command sent, so we can retry it if needed */
	static char last_put[NNTP_STRLEN];
	static constext *xover_cmds = "XOVER";
#	if 0 /* currently not used */
	static constext *xhdr_cmds = "XHDR";
#	endif /* 0 */
	enum extension_type { NO, LIST_EXTENSIONS, CAPABILITIES };
	/* Set so we don't reconnect just to QUIT */
	static t_bool quitting = FALSE;
#endif /* NNTP_ABLE */

/*
 * local prototypes
 */
#ifdef NNTP_ABLE
	static int mode_reader(t_bool *sec);
	static int new_nntp_command(const char *command, int success, char *message, size_t mlen);
	static int reconnect(int retry);
	static int server_init(char *machine, const char *cservice, int port, char *text, size_t mlen);
	static int check_extensions(t_bool *sec);
	static void close_server(void);
	static void list_motd(void);
#	ifdef INET6
		static int get_tcp6_socket(char *machine, unsigned short port);
#	else
		static int get_tcp_socket(char *machine, char *service, unsigned short port);
#	endif /* INET6 */
#	ifdef DECNET
		static int get_dnet_socket(char *machine, char *service);
#	endif /* DECNET */
#endif /* NNTP_ABLE */


/*
 * Return the actual fd in use for the nntp read-side socket
 * This is a leak of internal state and should go away if possible
 */
FILE *
get_nntp_fp(
	FILE *fp)
{
	return (fp == FAKE_NNTP_FP ? nntp_rd_fp : fp);
}


FILE *
get_nntp_wr_fp(
	FILE *fp)
{
	return (fp == FAKE_NNTP_FP ? nntp_wr_fp : fp);
}


/*
 * getserverbyfile(file)
 *
 * Get the name of a server from a named file.
 *	Handle white space and comments.
 *	Use NNTPSERVER environment variable if set.
 *
 *	Parameters: "file" is the name of the file to read.
 *
 *	Returns: Pointer to static data area containing the
 *	         first non-ws/comment line in the file.
 *	         NULL on error (or lack of entry in file).
 *
 *	Side effects: None.
 */
char *
getserverbyfile(
	const char *file)
{
	static char buf[256];
#ifdef NNTP_ABLE
	FILE *fp;
	char *cp;
#	if !defined(HAVE_SETENV) && defined(HAVE_PUTENV)
	char tmpbuf[256];
	char *new_env;
	static char *old_env = NULL;
#	endif /* !HAVE_SETENV && HAVE_PUTENV */
#endif /* NNTP_ABLE */

	if (!read_news_via_nntp) {
		STRCPY(buf, "local");	/* what if a server is named "local"? */
		return buf;
	}

	if (read_saved_news) {
		STRCPY(buf, "reading saved news");
		return buf;
	}

#ifdef NNTP_ABLE
	if (cmdline_nntpserver[0] != '\0') {
		get_nntpserver(buf, cmdline_nntpserver);
#	ifdef HAVE_SETENV
		setenv("NNTPSERVER", buf, 1);
#	else
#		ifdef HAVE_PUTENV
		snprintf(tmpbuf, sizeof(tmpbuf), "NNTPSERVER=%s", buf);
		new_env = my_strdup(tmpbuf);
		putenv(new_env);
		FreeIfNeeded(old_env);
		old_env = new_env; /* we are 'leaking' the last malloced mem at exit here */
#		endif /* HAVE_PUTENV */
#	endif /* HAVE_SETENV */
		return buf;
	}

	if ((cp = getenv("NNTPSERVER")) != NULL) {
		get_nntpserver(buf, cp);
		return buf;
	}

	if (file == NULL)
		return NULL;

	if ((fp = fopen(file, "r")) != NULL) {

		while (fgets(buf, (int) sizeof(buf), fp) != NULL) {
			if (*buf == '\n' || *buf == '#')
				continue;

			if ((cp = strrchr(buf, '\n')) != NULL)
				*cp = '\0';

			(void) fclose(fp);
			return buf;
		}

		(void) fclose(fp);

		if (cp != NULL) {
			get_nntpserver(buf, cp);
			return buf;
		}
	}

#	ifdef USE_INN_NNTPLIB
	if ((cp = GetConfigValue(_CONF_SERVER)) != NULL) {
		(void) STRCPY(buf, cp);
		return buf;
	}
#	endif /* USE_INN_NNTPLIB */

#	ifdef NNTP_DEFAULT_SERVER
	if (*(NNTP_DEFAULT_SERVER))
		return strcpy(buf, NNTP_DEFAULT_SERVER);
#	endif /* NNTP_DEFAULT_SERVER */

#endif /* NNTP_ABLE */
	return NULL;	/* No entry */
}


/*
 * server_init  Get a connection to the remote server.
 *
 *	Parameters:	"machine" is the machine to connect to.
 *			"service" is the service to connect to on the machine.
 *			"port" is the servive port to connect to.
 *
 *	Returns:	server's initial response code
 *			or -errno
 *
 *	Side effects:	Connects to server.
 *			"nntp_rd_fp" and "nntp_wr_fp" are fp's
 *			for reading and writing to server.
 *			"text" is updated to contain the rest of response string from server
 */
#ifdef NNTP_ABLE
static int
server_init(
	char *machine,
	const char *cservice,	/* usually a literal */
	int port,
	char *text,
	size_t mlen)
{
#	ifndef INET6
	char temp[256];
	char *service = strncpy(temp, cservice, sizeof(temp) - 1); /* ...calls non-const funcs */
#	endif /* !INET6 */
	int sockt_rd, sockt_wr;

#	ifdef DECNET
	char *cp;

	cp = strchr(machine, ':');

	if (cp && cp[1] == ':') {
		*cp = '\0';
		sockt_rd = get_dnet_socket(machine, service);
	} else
		sockt_rd = get_tcp_socket(machine, service, port);
#	else
#		ifdef INET6
	sockt_rd = get_tcp6_socket(machine, (unsigned short) port);
#		else
	sockt_rd = get_tcp_socket(machine, service, (unsigned short) port);
#		endif /* INET6 */
#	endif /* DECNET */

	if (sockt_rd < 0)
		return sockt_rd;

	/*
	 * Now we'll make file pointers (i.e., buffered I/O) out of
	 * the socket file descriptor. Note that we can't just
	 * open a fp for reading and writing -- we have to open
	 * up two separate fp's, one for reading, one for writing.
	 */

	if ((nntp_rd_fp = (TCP *) s_fdopen(sockt_rd, "r")) == NULL) {
		perror("server_init: fdopen() #1");
		return -errno;
	}

	if ((sockt_wr = s_dup(sockt_rd)) < 0) {
		perror("server_init: dup()");
		return -errno;
	}

#	ifdef TLI /* Transport Level Interface */
	if (t_sync(sockt_rd) < 0) {	/* Sync up new fd with TLI */
		t_error("server_init: t_sync()");
		nntp_rd_fp = NULL;
		return -EPROTO;
	}
#	else
	if ((nntp_wr_fp = (TCP *) s_fdopen(sockt_wr, "w")) == NULL) {
		perror("server_init: fdopen() #2");
		nntp_rd_fp = NULL;
		return -errno;
	}
#	endif /* TLI */


	last_put[0] = '\0';		/* no retries in get_respcode */
	/*
	 * Now get the server's signon message
	 */
	return (get_respcode(text, mlen));
}
#endif /* NNTP_ABLE */


/*
 * get_tcp_socket -- get us a socket connected to the specified server.
 *
 *	Parameters:	"machine" is the machine the server is running on.
 *			"service" is the service to connect to on the server.
 *			"port" is the port to connect to on the server.
 *
 *	Returns:	Socket connected to the server if all if ok
 *			EPROTO		for internal socket errors
 *			EHOSTUNREACH	if specified NNTP port/server can't be located
 *			errno		any valid errno value on connection
 *
 *	Side effects:	Connects to server.
 *
 *	Errors:		Returned & printed via perror.
 */
#if defined(NNTP_ABLE) && !defined(INET6)
static int
get_tcp_socket(
	char *machine,		/* remote host */
	char *service,		/* nttp/smtp etc. */
	unsigned short port)	/* tcp port number */
{
	int s = -1;
	int save_errno = 0;
	struct sockaddr_in sock_in;
#	ifdef TLI /* Transport Level Interface */
	char device[20];
	char *env_device;
	extern int t_errno;
	extern struct hostent *gethostbyname();
	struct hostent *hp;
	struct t_call *callptr;

	/*
	 * Create a TCP transport endpoint.
	 */
	if ((env_device = getenv("DEV_TCP")) != NULL) /* SCO uses DEV_TCP, most other OS use /dev/tcp */
		STRCPY(device, env_device);
	else
		strcpy(device, "/dev/tcp");

	if ((s = t_open(device, O_RDWR, (struct t_info *) 0)) < 0) {
		t_error(txt_error_topen);
		return -EPROTO;
	}
	if (t_bind(s, (struct t_bind *) 0, (struct t_bind *) 0) < 0) {
		t_error("t_bind");
		t_close(s);
		return -EPROTO;
	}
	memset((char *) &sock_in, '\0', sizeof(sock_in));
	sock_in.sin_family = AF_INET;
	sock_in.sin_port = htons(port);

	if (!isdigit((unsigned char)*machine) ||
#		ifdef HAVE_INET_ATON
	    !inet_aton(machine, &sock_in)
#		else
#			ifdef HAVE_INET_ADDR
	    (long) (sock_in.sin_addr.s_addr = inet_addr(machine)) == INADDR_NONE)
#			endif /* HAVE_INET_ADDR */
#		endif /* HAVE_INET_ATON */
	{
		if ((hp = gethostbyname(machine)) == NULL) {
			my_fprintf(stderr, _(txt_gethostbyname), "gethostbyname() ", machine);
			t_close(s);
			return -EHOSTUNREACH;
		}
		memcpy((char *) &sock_in.sin_addr, hp->h_addr_list[0], hp->h_length);
	}

	/*
	 * Allocate a t_call structure and initialize it.
	 * Let t_alloc() initialize the addr structure of the t_call structure.
	 */
	if ((callptr = (struct t_call *) t_alloc(s, T_CALL, T_ADDR)) == NULL) {
		t_error("t_alloc");
		t_close(s);
		return -EPROTO;
	}

	callptr->addr.maxlen = sizeof(sock_in);
	callptr->addr.len = sizeof(sock_in);
	callptr->addr.buf = (char *) &sock_in;
	callptr->opt.len = 0;			/* no options */
	callptr->udata.len = 0;			/* no user data with connect */

	/*
	 * Connect to the server.
	 */
	if (t_connect(s, callptr, (struct t_call *) 0) < 0) {
		save_errno = t_errno;
		if (save_errno == TLOOK)
			fprintf(stderr, _(txt_error_server_unavailable));
		else
			t_error("t_connect");
		t_free((char *) callptr, T_CALL);
		t_close(s);
		return -save_errno;
	}

	/*
	 * Now replace the timod module with the tirdwr module so that
	 * standard read() and write() system calls can be used on the
	 * descriptor.
	 */

	t_free((char *) callptr, T_CALL);

	if (ioctl(s, I_POP, NULL) < 0) {
		perror("I_POP(timod)");
		t_close(s);
		return -EPROTO;
	}

	if (ioctl(s, I_PUSH, "tirdwr") < 0) {
		perror("I_PUSH(tirdwr)");
		t_close(s);
		return -EPROTO;
	}

#	else
#		ifndef EXCELAN
	struct servent *sp;
	struct hostent *hp;
#			ifdef h_addr
	int x = 0;
	char **cp;
	static char *alist[2] = {0, 0};
#			endif /* h_addr */
	static struct hostent def;
	static struct in_addr defaddr;
	static char namebuf[256];

#			ifdef HAVE_GETSERVBYNAME
	if ((sp = (struct servent *) getservbyname(service, "tcp")) == NULL) {
		my_fprintf(stderr, _(txt_error_unknown_service), service);
		return -EHOSTUNREACH;
	}
#			else
	sp = my_malloc(sizeof(struct servent));
	sp->s_port = htons(IPPORT_NNTP);
#			endif /* HAVE_GETSERVBYNAME */

	/* If not a raw ip address, try nameserver */
	if (!isdigit((unsigned char) *machine) ||
#			ifdef HAVE_INET_ATON
	    !inet_aton(machine, &defaddr)
#			else
#				ifdef HAVE_INET_ADDR
	    (long) (defaddr.s_addr = (long) inet_addr(machine)) == -1
#				endif /* HAVE_INET_ADDR */
#			endif /* HAVE_INET_ATON */
	    )
	{
		hp = gethostbyname(machine);
	} else {
		/* Raw ip address, fake */
		STRCPY(namebuf, machine);
		def.h_name = (char *) namebuf;
#			ifdef h_addr
		def.h_addr_list = alist;
#			endif /* h_addr */
		def.h_addr_list[0] = (char *) &defaddr;
		def.h_length = sizeof(struct in_addr);
		def.h_addrtype = AF_INET;
		def.h_aliases = 0;
		hp = &def;
	}

	if (hp == NULL) {
		my_fprintf(stderr, _(txt_gethostbyname), "\n", machine);
		return -EHOSTUNREACH;
	}

	memset((char *) &sock_in, '\0', sizeof(sock_in));
	sock_in.sin_family = hp->h_addrtype;
	sock_in.sin_port = htons(port);
/*	sock_in.sin_port = sp->s_port; */
#		else
	memset((char *) &sock_in, '\0', sizeof(sock_in));
	sock_in.sin_family = AF_INET;
#		endif /* !EXCELAN */

	/*
	 * The following is kinda gross. The name server under 4.3
	 * returns a list of addresses, each of which should be tried
	 * in turn if the previous one fails. However, 4.2 hostent
	 * structure doesn't have this list of addresses.
	 * Under 4.3, h_addr is a #define to h_addr_list[0].
	 * We use this to figure out whether to include the NS specific
	 * code...
	 */

#		ifdef h_addr
	/*
	 * Get a socket and initiate connection -- use multiple addresses
	 */
	for (cp = hp->h_addr_list; cp && *cp; cp++) {
#			if defined(__hpux) && defined(SVR4)
		unsigned long socksize, socksizelen;
#			endif /* __hpux && SVR4 */

		if ((s = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			return -errno;
		}

		memcpy((char *) &sock_in.sin_addr, *cp, hp->h_length);

#			ifdef HAVE_INET_NTOA
		if (x < 0)
			my_fprintf(stderr, _(txt_trying), (char *) inet_ntoa(sock_in.sin_addr));
#			endif /* HAVE_INET_NTOA */

#			if defined(__hpux) && defined(SVR4)	/* recommended by raj@cup.hp.com */
#				define HPSOCKSIZE 0x8000
		getsockopt(s, SOL_SOCKET, SO_SNDBUF, /* (caddr_t) */ &socksize, /* (caddr_t) */ &socksizelen);
		if (socksize < HPSOCKSIZE) {
			socksize = HPSOCKSIZE;
			setsockopt(s, SOL_SOCKET, SO_SNDBUF, /* (caddr_t) */ &socksize, sizeof(socksize));
		}
		socksize = 0;
		socksizelen = sizeof(socksize);
		getsockopt(s, SOL_SOCKET, SO_RCVBUF, /* (caddr_t) */ &socksize, /* (caddr_t) */ &socksizelen);
		if (socksize < HPSOCKSIZE) {
			socksize = HPSOCKSIZE;
			setsockopt(s, SOL_SOCKET, SO_RCVBUF, /* (caddr_t) */ &socksize, sizeof(socksize));
		}
#			endif /* __hpux && SVR4 */

		if ((x = connect(s, (struct sockaddr *) &sock_in, sizeof(sock_in))) == 0)
			break;

		save_errno = errno;									/* Keep for later */
#			ifdef HAVE_INET_NTOA
		my_fprintf(stderr, _(txt_connection_to), (char *) inet_ntoa(sock_in.sin_addr));
		perror("");
#			endif /* HAVE_INET_NTOA */
		(void) s_close(s);
	}

	if (x < 0) {
		my_fprintf(stderr, _(txt_giving_up));
		return -save_errno;					/* Return the last errno we got */
	}
#		else

#			ifdef EXCELAN
	if ((s = socket(SOCK_STREAM, (struct sockproto *) NULL, &sock_in, SO_KEEPALIVE)) < 0) {
		perror("socket");
		return -errno;
	}

	/* set up addr for the connect */
	memset((char *) &sock_in, '\0', sizeof(sock_in));
	sock_in.sin_family = AF_INET;
	sock_in.sin_port = htons(IPPORT_NNTP);

	if ((sock_in.sin_addr.s_addr = rhost(&machine)) == -1) {
		my_fprintf(stderr, _(txt_gethostbyname), "\n", machine);
		return -1;
	}

	/* And connect */
	if (connect(s, (struct sockaddr *) &sock_in) < 0) {
		save_errno = errno;
		perror("connect");
		(void) s_close(s);
		return -save_errno;
	}

#			else
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -errno;
	}

	/* And then connect */

	memcpy((char *) &sock_in.sin_addr, hp->h_addr_list[0], hp->h_length);

	if (connect(s, (struct sockaddr *) &sock_in, sizeof(sock_in)) < 0) {
		save_errno = errno;
		perror("connect");
		(void) s_close(s);
		return -save_errno;
	}

#			endif /* !EXCELAN */
#		endif /* !h_addr */
#	endif /* !TLI */
	return s;
}
#endif /* NNTP_ABLE && !INET6 */


#if defined(NNTP_ABLE) && defined(INET6)
/*
 * get_tcp6_socket -- get us a socket connected to the server.
 *
 * Parameters:   "machine" is the machine the server is running on.
 *                "port" is the portnumber to connect to.
 *
 * Returns:      Socket connected to the news server if
 *               all is ok, else -1 on error.
 *
 * Side effects: Connects to server via IPv4 or IPv6.
 *
 * Errors:       Printed via my_fprintf.
 */
static int
get_tcp6_socket(
	char *machine,
	unsigned short port)
{
	char mymachine[MAXHOSTNAMELEN + 1];
	char myport[12];
	int s = -1, err = -1;
	struct addrinfo hints, *res, *res0;

	snprintf(mymachine, sizeof(mymachine), "%s", machine);
	snprintf(myport, sizeof(myport), "%d", port);

/* just in case */
#	ifdef AF_UNSPEC
#		define ADDRFAM	AF_UNSPEC
#	else
#		ifdef PF_UNSPEC
#			define ADDRFAM	PF_UNSPEC
#		else
#			define ADDRFAM	AF_INET
#		endif /* PF_UNSPEC */
#	endif /* AF_UNSPEC */
	memset(&hints, 0, sizeof(hints));
/*	hints.ai_flags = AI_CANONNAME; */
	hints.ai_family = ADDRFAM;
	hints.ai_socktype = SOCK_STREAM;
	res = (struct addrinfo *) 0;
	res0 = (struct addrinfo *) 0;
	err = getaddrinfo(mymachine, myport, &hints, &res0);
	if (err != 0) {
		my_fprintf(stderr, "\ngetaddrinfo: %s\n", gai_strerror(err));
		return -1;
	}
	err = -1;
	for (res = res0; res; res = res->ai_next) {
		if ((s = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
			continue;
		if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
			close(s);
		else {
			err = 0;
			break;
		}
	}
	if (res0 != NULL)
		freeaddrinfo(res0);
	if (err < 0) {
		/*
		 * TODO: issue a more usefull error-message
		 */
		my_fprintf(stderr, _(txt_error_socket_or_connect_problem));
		return -1;
	}
	return s;
}
#endif /* NNTP_ABLE && INET6 */


#ifdef DECNET
/*
 * get_dnet_socket -- get us a socket connected to the server.
 *
 *	Parameters:	"machine" is the machine the server is running on.
 *			"service" is the name of the service to connect to.
 *
 *	Returns:	Socket connected to the news server if
 *			all is ok, else -1 on error.
 *
 *	Side effects:	Connects to server.
 *
 *	Errors:		Printed via nerror.
 */
static int
get_dnet_socket(
	char *machine,
	char *service)
{
#	ifdef NNTP_ABLE
	int s, area, node;
	struct sockaddr_dn sdn;
	struct nodeent *getnodebyname(), *np;

	memset((char *) &sdn, '\0', sizeof(sdn));

	switch (s = sscanf(machine, "%d%*[.]%d", &area, &node)) {
		case 1:
			node = area;
			area = 0;
			/* FALLTHROUGH */
		case 2:
			node += area * 1024;
			sdn.sdn_add.a_len = 2;
			sdn.sdn_family = AF_DECnet;
			sdn.sdn_add.a_addr[0] = node % 256;
			sdn.sdn_add.a_addr[1] = node / 256;
			break;

		default:
			if ((np = getnodebyname(machine)) == NULL) {
				my_fprintf(stderr, _(txt_gethostbyname), "", machine);
				return -1;
			} else {
				memcpy((char *) sdn.sdn_add.a_addr, np->n_addr, np->n_length);
				sdn.sdn_add.a_len = np->n_length;
				sdn.sdn_family = np->n_addrtype;
			}
			break;
	}
	sdn.sdn_objnum = 0;
	sdn.sdn_flags = 0;
	sdn.sdn_objnamel = strlen("NNTP");
	memcpy(&sdn.sdn_objname[0], "NNTP", sdn.sdn_objnamel);

	if ((s = socket(AF_DECnet, SOCK_STREAM, 0)) < 0) {
		nerror("socket");
		return -1;
	}

	/* And then connect */

	if (connect(s, (struct sockaddr *) &sdn, sizeof(sdn)) < 0) {
		nerror("connect");
		close(s);
		return -1;
	}

	return s;
#	else
	return -1;
#	endif /* NNTP_ABLE */
}
#endif /* DECNET */


/*----------------------------------------------------------------------
 * Ideally the code after this point should be the only interface to the
 * NNTP internals...
 */

/*
 * u_put_server -- send data to the server. Do not flush output.
 */
#ifdef NNTP_ABLE
void
u_put_server(
	const char *string)
{
	s_puts(string, nntp_wr_fp);
#	ifdef DEBUG
	debug_nntp(">>>", string);
#	endif /* DEBUG */
}


/*
 * Send 'string' to the NNTP server, terminating it with CR and LF, as per
 * ARPA standard.
 *
 * Returns: Nothing.
 *
 *	Side effects: Talks to the server.
 *	              Closes connection if things are not right.
 *
 * Note: This routine flushes the buffer each time it is called. For large
 *       transmissions (i.e., posting news) don't use it. Instead, do the
 *       fprintf's yourself, and then a final fflush.
 *       Only cache commands, don't cache data transmissions.
 */
void
put_server(
	const char *string)
{
	if (*string && strlen(string)) {
		DEBUG_IO((stderr, "put_server(%s)\n", string));
		s_puts(string, nntp_wr_fp);
		s_puts("\r\n", nntp_wr_fp);
#	ifdef DEBUG
		debug_nntp(">>>", string);
#	endif /* DEBUG */
		/*
		 * remember the last command we wrote to be able to resend it after a
		 * reconnect. reconnection is handled by get_server()
		 */
		if (last_put != string)
			STRCPY(last_put, string);
	}
	(void) s_flush(nntp_wr_fp);
}


/*
 * Reconnect to server after a timeout, reissue last command to
 * get us back into the pre-timeout state
 */
static int
reconnect(
	int retry)
{
	char buf[NNTP_STRLEN];

	/*
	 * Tear down current connection
	 * Close the NNTP connection with prejudice
	 */
	if (nntp_wr_fp)
		s_fclose(nntp_wr_fp);
	if (nntp_rd_fp)
		s_fclose(nntp_rd_fp);
	nntp_rd_fp = nntp_wr_fp = NULL;

	if (!tinrc.auto_reconnect)
		ring_bell();

	DEBUG_IO((stderr, _("\nServer timed out, trying reconnect # %d\n"), retry));

	/*
	 * Exit tin if the user says no to reconnect. The exit code stops tin from trying
	 * to disconnect again - the connection is already dead
	 */
	if (!tinrc.auto_reconnect && prompt_yn(_(txt_reconnect_to_news_server), TRUE) != 1)
		tin_done(NNTP_ERROR_EXIT);		/* user said no to reconnect */

	clear_message();

	strcpy(buf, last_put);			/* Keep copy here, it will be clobbered a lot otherwise */

	if (!nntp_open()) {
		/*
		 * Re-establish our current group and resend last command
		 */
		if (curr_group != NULL) {
			DEBUG_IO((stderr, _("Rejoin current group\n")));
			snprintf(last_put, sizeof(last_put), "GROUP %s", curr_group->name);
			put_server(last_put);
			s_gets(last_put, NNTP_STRLEN, nntp_rd_fp);
#	ifdef DEBUG
			debug_nntp("<<<", last_put);
#	endif /* DEBUG */
			DEBUG_IO((stderr, _("Read (%s)\n"), last_put));
		}
		DEBUG_IO((stderr, _("Resend last command (%s)\n"), buf));
		put_server(buf);
		return 0;
	}

	if (--retry == 0)					/* No more tries? */
		tin_done(NNTP_ERROR_EXIT);

	return retry;
}


/*
 * Read a line of data from the NNTP socket. If something gives, do reconnect
 *
 *	Parameters:	"string" has the buffer space for the line received.
 *			"size" is maximum size of the buffer to read.
 *
 *	Returns:	NULL on end of stream, or a line of data.
 *				Basically, we try to act like fgets() on an NNTP stream.
 *
 *	Side effects:	Talks to server, changes contents of "string".
 *			Reopens connection when necessary and requested.
 *			Exits via tin_done() if fatal error occurs.
 */
char *
get_server(
	char *string,
	int size)
{
	int retry = NNTP_TRY_RECONNECT;

	reconnected_in_last_get_server = FALSE;
	errno = 0;

	/*
	 * NULL socket reads indicates socket has closed. Try a few times more
	 */
	while (nntp_rd_fp == NULL || s_gets(string, size, nntp_rd_fp) == NULL) {

		if (quitting)						/* Don't bother to reconnect */
			tin_done(NNTP_ERROR_EXIT);		/* And don't try to disconnect again! */

#	ifdef DEBUG
		if (errno != 0 && errno != EINTR)	/* Will only confuse end users */
			perror_message("get_server()");
#	endif /* DEBUG */

		/*
		 * Reconnect only if command was not "QUIT" anyway (in which case a
		 * reconnection would be useless because the connection will be
		 * closed immediately). Also prevents tin from asking to reconnect
		 * when user is quitting tin if tinrc.auto_reconnect is false.
		 */
		if (strncmp(last_put, "QUIT", 4)) {
			retry = reconnect(retry);		/* Will abort when out of tries */
			reconnected_in_last_get_server = TRUE;
		} else {
			/*
			 * Use standard NNTP closing message and response code if user is
			 * quitting tin and leave loop.
			 */
			strncpy(string, _(txt_nntp_ok_goodbye), size - 2);
			strcat(string, cCRLF);		/* tin_fgets() needs CRLF */
			break;
		}
	}
	return string;
}


/*
 * Send "QUIT" command and close the connection to the server
 *
 * Side effects: Closes the connection to the server.
 *	              You can't use "put_server" or "get_server" after this
 *	              routine is called.
 *
 * TODO: remember servers response string and if it contains anything else
 *       than just "." (i.e. transfer statistics) present it to the user?
 *
 */
static void
close_server(
	void)
{
	if (nntp_wr_fp == NULL || nntp_rd_fp == NULL)
		return;

	if (!batch_mode || verbose)
		my_fputs(_(txt_disconnecting), stdout);
	nntp_command("QUIT", OK_GOODBYE, NULL, 0);
	quitting = TRUE;										/* Don't reconnect just for this */

	(void) s_fclose(nntp_wr_fp);
	(void) s_fclose(nntp_rd_fp);
	s_end();
	nntp_wr_fp = nntp_rd_fp = NULL;
}
#endif /* NNTP_ABLE */


#ifdef NNTP_ABLE
/*
 * Try and use CAPABILITIES/LIST EXTENSIONS here. Get this list before
 * issuing other NNTP commands because the correct methods may be mentioned
 * in the list of extensions.
 *
 * Sets up: t_capabilities nntp_caps
 */
static int
check_extensions(
	t_bool *sec)
{
	char *ptr;
	int ret = 0;
#	if 1 /* "CAPABILITIES" replaces "LIST EXTENSIONS" */
	char buf[NNTP_STRLEN];
	char *d;
	int i;

	buf[0] = '\0';
	i = new_nntp_command("CAPABILITIES", INF_CAPABILITIES, buf, sizeof(buf));
	switch (i) {
		case INF_CAPABILITIES:
			nntp_caps.type = CAPABILITIES;
			while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#		ifdef DEBUG
				debug_nntp("<<<", ptr);
#		endif /* DEBUG */
				/* look for version number(s) */
				if (!nntp_caps.version && nntp_caps.type == CAPABILITIES) {
					if (!strcasecmp(ptr, "VERSION")) {
						d = ptr + 7;
						d = strpbrk(d, " \t");
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							nntp_caps.version = MAX(nntp_caps.version, (unsigned int) atoi(d));
							d = strpbrk(d, " \t");
						}
					}
				}
				/* we currently only support CAPABILITIES VERSION 2 */
				if (nntp_caps.version == 2) {
					/*
					 * check for LIST variants - this code is untested
					 */
					if (!strcasecmp(ptr, "LIST")) {
						d = ptr + 4;
						d = strpbrk(d, " \t");
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							if (!strcasecmp(d, "ACTIVE.TIMES"))
								nntp_caps.list_active_times = TRUE;
							else if (!strcasecmp(d, "ACTIVE"))
								nntp_caps.list_active_times = TRUE;
							else if (!strcasecmp(d, "DISTRIB.PATS"))
								nntp_caps.list_distrib_pats = TRUE;
							else if (!strcasecmp(d, "DISTRIBUTIONS")) /* LIST DISTRIBUTIONS, "private" extension, RFC 2980 */
								nntp_caps.list_distributions = TRUE;
							else if (!strcasecmp(d, "HEADERS"))
								nntp_caps.list_headers = TRUE; /* HDR requires LIST HEADERS, but not vice versa */
							else if (!strcasecmp(d, "NEWSGROUPS"))
								nntp_caps.list_newsgroups = TRUE;
							else if (!strcasecmp(d, "OVERVIEW.FMT")) /* OVER requires OVERVIEW.FMT, but not vice versa */
								nntp_caps.list_overview_fmt = TRUE;
							else if (!strcasecmp(d, "MOTD")) /* "private" extension */
								nntp_caps.list_motd = TRUE;
							else if (!strcasecmp(d, "SUBSCRIPTIONS")) /* "private" extension, RFC 2980 */
								nntp_caps.list_subscriptions = TRUE;
							else if (!strcasecmp(d, "MODERATORS")) /* "private" extension */
								nntp_caps.list_moderators = TRUE;
							d = strpbrk(d, " \t");
						}
					} else if (!strcasecmp(ptr, "IMPLEMENTATION"))
						nntp_caps.implementation = my_strdup(ptr + 14);
					else if (!strcasecmp(ptr, "MODE-READER")) {
						if (!nntp_caps.reader)
							nntp_caps.mode_reader = TRUE;
					} else if (!strcasecmp(ptr, "READER")) {
						nntp_caps.reader = TRUE;
						nntp_caps.mode_reader = FALSE;
					} else if (!strcasecmp(d, "POST"))
						nntp_caps.post = TRUE;
					else if (!strcasecmp(ptr, "NEWNEWS"))
						nntp_caps.newnews = TRUE;
					else if (!strcasecmp(ptr, "XPAT")) /* extension, RFC 2980 */
						nntp_caps.xpat = TRUE;
					else if (!strcasecmp(ptr, "STARTTLS"))
						nntp_caps.starttls = TRUE;
					/*
					 * NOTE: if we saw OVER, LIST OVERVIEW.FMT _must_ be implemented
					 */
					else if (!strcasecmp(ptr, &xover_cmds[1])) {
						nntp_caps.over = TRUE;
						nntp_caps.list_overview_fmt = TRUE;
						nntp_caps.over_cmd = &xover_cmds[1];
						d = ptr + strlen(&xover_cmds[1]);
						d = strpbrk(d, " \t");
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							if (!strcasecmp(d, "MSGID"))
								nntp_caps.over_msgid = TRUE;
							d = strpbrk(d, " \t");
						}
					} else if (!strcasecmp(ptr, "AUTHINFO")) {
						d = ptr + 8;
						d = strpbrk(d, " \t");
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							if (!strcasecmp(d, "USER"))
								nntp_caps.authinfo_user = TRUE;
							if (!strcasecmp(d, "SASL"))
								nntp_caps.authinfo_sasl = TRUE;
							d = strpbrk(d, " \t");
						}
					}
#		if 0
					/*
					 * NOTE: if we saw HDR, LIST HEADERS _must_ be implemented
					 */
					else if (!strcasecmp(ptr, &xhdr_cmds[1])) {
						nntp_caps.hdr_cmd = &xhdr_cmds[1];
						nntp_caps.hdr = TRUE;
						nntp_caps.list_headers = TRUE;
					}
					else if (!strcasecmp(ptr, "IHAVE"))
						nntp_caps.ihave = TRUE;
#		endif /* 0 */
					/*
					 * TODO: SASL, STREAMING
					 */
				} else
					nntp_caps.type = NO;
			}
			break;

		case ERR_GOODBYE:
			ret = i;
			error_message(buf);
			break;

		default:
			break;
	}
#		ifdef DEBUG
	debug_print_nntp_extensions();
#		endif /* DEBUG */
	if ((ret != ERR_GOODBYE) && !*sec && !nntp_caps.reader) {
		if (nntp_caps.type == CAPABILITIES && !nntp_caps.mode_reader)
			return -1; /* no mode-switching and no reader mode, give up */
		if ((ret = mode_reader(&*sec)) != 0)
			return ret;
		else if (nntp_caps.type == CAPABILITIES) /* 2nd pass */
			ret = check_extensions(&*sec);
	}

#	else
	/*
	 * TODO: the following code can go away in tin 2.0 as it's
	 *       obsolete by RFC 3977
	 *
	 * "LIST EXTENSIONS" is somewhat troublesome as there are a lot
	 * of broken implementations out there and it is a multiline response
	 */
	if (nntp_caps.type == NO) {
		char buf[NNTP_STRLEN];
		int i;

		buf[0] = '\0';
		i = new_nntp_command("LIST EXTENSIONS", OK_EXTENSIONS, buf, sizeof(buf));
		switch (i) {
			case 215:	/* Netscape-Collabra/3.52 (badly broken); NetWare-News-Server/5.1 */
#		ifdef DEBUG
				debug_nntp("LIST EXTENSIONS", "skipping data");
#		endif /* DEBUG */
				while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL)
					;
				break;

			case OK_EXTENSIONS:	/* as defined draft-ietf-nntpext-base-27.txt */
			case 205:	/* M$ Exchange 5.5 */
				nntp_caps.type = LIST_EXTENSIONS;
				while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
					if (nntp_caps.type == LIST_EXTENSIONS) {
#		ifdef DEBUG
						debug_nntp("<<<", ptr);
#		endif /* DEBUG */
						/*
						 * some servers (e.g. Hamster 1.3) have leading spaces in
						 * the extension list
						 */
						str_trim(ptr);
						/*
						 * Check for (X)OVER
						 * XOVER should not be listed in EXTENSIONS (but sometimes
						 * is) checking for it if OVER is not found does no harm.
						 */
						if (!nntp_caps.over_cmd) {
							for (i = 1; i >= 0; i--) {
								if (strcasecmp(ptr, &xover_cmds[i]) == 0) {
									nntp_caps.over_cmd = &xover_cmds[i];
									break;
								}
							}
						}
					}
				}
				/*
				 * as the server did support LIST EXTENSIONS it's likely that it
				 * also can do LIST MOTD (we don't bother to parse the LIST
				 * EXTENSIONS output for MOTD as it never was standardized;
				 * draft-ietf-nntpext-base-24.txt only described OVER, HDR and
				 * LISTGROUP).
				 */
				nntp_caps.list_motd = TRUE;
				break;

			case ERR_GOODBYE:
				ret = i;
				error_message(buf);
				break;

			default:
				break;
		}
#		ifdef DEBUG
		debug_print_nntp_extensions();
#		endif /* DEBUG */
		if ((ret != ERR_GOODBYE) && !*sec)
			ret = mode_reader(&*sec);
	}
#	endif /* 1 */
	return ret;
}


/*
 * Switch INN into NNRP mode with 'mode reader'
 */
static int
mode_reader(
	t_bool *sec)
{
	int ret = 0;

	if (!nntp_caps.reader) {
		char line[NNTP_STRLEN];
#ifdef DEBUG
		debug_nntp("mode_reader", "mode reader");
#endif /* DEBUG */
		DEBUG_IO((stderr, "nntp_command(MODE READER)\n"));
		put_server("MODE READER");

		/*
		 * According to the latest NNTP draft (May 2005), MODE READER may only
		 * return the following response codes:
		 *
		 *   200 (OK_CANPOST)     Hello, you can post
		 *   201 (OK_NOPOST)      Hello, you can't post
		 *  (202 (OK_NOIHAVE)     discussed on the itef mailinglist; withdrawn)
		 *  (203 (OK_NOPOSTIHAVE) discussed on the itef mailinglist; withdrawn)
		 *   502 (ERR_ACCESS)     Service unavailable
		 *
		 * However, there may be old servers out there that do not implement this
		 * command and therefore return ERR_COMMAND (500). Unfortunately there
		 * are some new servers out there (i.e. INN 2.4.0 (20020220 prerelease)
		 * which do return ERR_COMMAND if they are feed only servers.
		 */

		switch ((ret = get_respcode(line, sizeof(line)))) {
			case OK_CANPOST:
	/*		case OK_NOIHAVE: */
				can_post = TRUE && !force_no_post;
				*sec = TRUE;
				ret = 0;
				break;

			case OK_NOPOST:
	/*		case OK_NOPOSTIHAVE: */
				can_post = FALSE;
				*sec = TRUE;
				ret = 0;
				break;

			case ERR_GOODBYE:
			case ERR_ACCESS:
				error_message(line);
				return ret;

			case ERR_COMMAND:
			default:
				break;
		}
	}
	return ret;
}
#endif /* NNTP_ABLE */


/*
 * Open a connection to the NNTP server. Authenticate if necessary or
 * desired, and test if the server supports XOVER.
 * Returns: 0	success
 *        > 0	NNTP error response code
 *        < 0	-errno from system call or similar error
 */
int
nntp_open(
	void)
{
#ifdef NNTP_ABLE
	char *linep;
	char line[NNTP_STRLEN];
	int i, ret;
	t_bool sec = FALSE;
	/* It appears that is_reconnect guards code that should be run only once */
	static t_bool is_reconnect = FALSE;

	if (!read_news_via_nntp)
		return 0;

#	ifdef DEBUG
	debug_nntp("nntp_open", "BEGIN");
#	endif /* DEBUG */

	if (nntp_server == NULL) {
		error_message(_(txt_cannot_get_nntp_server_name));
		error_message(_(txt_server_name_in_file_env_var), NNTP_SERVER_FILE);
		return -EHOSTUNREACH;
	}

	if (!batch_mode) {
		if (nntp_tcp_port != IPPORT_NNTP)
			wait_message(0, _(txt_connecting_port), nntp_server, nntp_tcp_port);
		else
			wait_message(0, _(txt_connecting), nntp_server);
	}

#	ifdef DEBUG
	debug_nntp("nntp_open", nntp_server);
#	endif /* DEBUG */

	ret = server_init(nntp_server, NNTP_TCP_NAME, nntp_tcp_port, line, sizeof(line));
	DEBUG_IO((stderr, "server_init returns %d,%s\n", ret, line));

	if (!batch_mode && ret >= 0 && cmd_line)
		my_fputc('\n', stdout);

#	ifdef DEBUG
	debug_nntp("nntp_open", line);
#	endif /* DEBUG */

	switch (ret) {
		/*
		 * ret < 0 : some error from system call
		 * ret > 0 : NNTP response code
		 *
		 * According to the ietf-nntp mailinglist:
		 *   200 you may (try to) do anything
		 *   201 you may not POST
		 *  (202 you may not IHAVE)
		 *  (203 you may not do EITHER)
		 *   All unrecognised 200 series codes should be assumed as success.
		 *   All unrecognised 300 series codes should be assumed as notice to continue.
		 *   All unrecognised 400 series codes should be assumed as temporary error.
		 *   All unrecognised 500 series codes should be assumed as error.
		 */

		case OK_CANPOST:
/*		case OK_NOIHAVE: */
			can_post = TRUE && !force_no_post;
			break;

		case OK_NOPOST:
/*		case OK_NOPOSTIHAVE: */
			can_post = FALSE;
			break;

		default:
			if (ret >= 200 && ret <= 299) {
				can_post = TRUE && !force_no_post;
				break;
			}
			if (ret < 0)
				error_message(_(txt_failed_to_connect_to_server), nntp_server);
			else
				error_message(line);

			return ret;
	}
	if (!is_reconnect) {
		/* remove leading whitespace and save server's initial response */
		linep = line;
		while (isspace((int) *linep))
			linep++;

		STRCPY(bug_nntpserver1, linep);
	}

	/*
	 * Find out which NNTP extensions are available
	 * TODO: The authentication method required may be mentioned in the list of
	 *       extensions. (For details about authentication methods, see
	 *       draft-ietf-nntpext-authinfo-07.txt).
	 */
	if ((ret = check_extensions(&sec)))
		return ret; /* required "MODE READER" failed, exit */

	/*
	 * If the user wants us to authenticate on connection startup, do it now.
	 * Some news servers return "201 no posting" first, but after successful
	 * authentication you get a "200 posting allowed". To find out if we are
	 * allowed to post after authentication issue a "MODE READER" again and
	 * interpret the response code.
	 */
	if (force_auth_on_conn_open) {
#	ifdef DEBUG
		debug_nntp("nntp_open", "authenticate");
#	endif /* DEBUG */
		authenticate(nntp_server, userid, TRUE);
		if ((ret = mode_reader(&sec)))
			return ret; /* "MODE READER" failed, exit */
	}

	if (!is_reconnect) {
		/* Inform user if he cannot post */
		if (!can_post && !batch_mode)
			wait_message(0, "%s\n", _(txt_cannot_post));

		/* Remove leading white space and save server's second response */
		linep = line;
		while (isspace((int) *linep))
			linep++;

		STRCPY(bug_nntpserver2, linep);

		/*
		 * Show user last server response line, do some nice formatting if
		 * response is longer than a screen wide.
		 *
		 * TODO: This only breaks the line once, but the response could be
		 * longer than two lines ...
		 */
		if (!batch_mode || verbose) {
			char *chr1, *chr2;
			int j;

			j = atoi(get_val("COLUMNS", "80"));
			chr1 = my_strdup((sec ? bug_nntpserver2 : bug_nntpserver1));

			if (((int) strlen(chr1)) >= j) {
				chr2 = chr1 + strlen(chr1) - 1;
				while (chr2 - chr1 >= j)
					chr2--;
				while (chr2 > chr1 && *chr2 != ' ')
					chr2--;
				if (chr2 != chr1)
					*chr2 = '\n';
			}

			wait_message(0, "%s\n", chr1);
			free(chr1);
		}
	}

	/*
	 * If LIST EXTENSIONS failed, check if NNTP supports XOVER or OVER command
	 * (successor of XOVER as of latest NNTP Draft (Jan 2002)
	 * We have to check that we _don't_ get an ERR_COMMAND
	 */
	if (nntp_caps.type == NO) {
		int j = 0;

		for (i = 0; i < 2 && j >= 0; i++) {
			j = new_nntp_command(&xover_cmds[i], ERR_NCING, line, sizeof(line));
			switch (j) {
				case ERR_COMMAND:
					break;

				case 224:	/* unexpected multiline ok, e.g.: Synchronet 3.13 NNTP Service 1.92 */
					nntp_caps.over_cmd = &xover_cmds[i];
#	ifdef DEBUG
					debug_nntp(&xover_cmds[i], "skipping data");
#	endif /* DEBUG */
					while ((linep = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL)
						;
					j = -1;
					break;

				default:
					nntp_caps.over_cmd = &xover_cmds[i];
					j = -1;
					break;
			}
		}
	} else {
		if (!nntp_caps.over_cmd) {
			/*
			 * CAPABILITIES/LIST EXTENSIONS didn't mention OVER or XOVER, try
			 * XOVER
			 */
			i = new_nntp_command(xover_cmds, ERR_NCING, line, sizeof(line));

			switch (i) {
				case ERR_COMMAND:
					break;

				case 224:	/* unexpected multiline ok, e.g.: Synchronet 3.13 NNTP Service 1.92 */
					nntp_caps.over_cmd = xover_cmds;
#	ifdef DEBUG
					debug_nntp(xover_cmds, "skipping data");
#	endif /* DEBUG */
					while ((linep = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL)
						;
					break;

				default:
					nntp_caps.over_cmd = xover_cmds;
					break;
			}
		}
#	if 0 /* unused */
		if (!nntp_caps.hdr_cmd) {
			/*
			 * LIST EXTENSIONS didn't mention HDR or XHDR, try
			 * XHDR
			 */
			if (!nntp_command(xhdr_cmds, ERR_COMMAND, NULL, 0))
				nntp_caps.hdr_cmd = xhdr_cmds;
		}
#	endif /* 0 */
	}

	if (!nntp_caps.over_cmd) {
		if (!is_reconnect && !batch_mode) {
			wait_message(2, _(txt_no_xover_support));

			if (tinrc.cache_overview_files)
				wait_message(2, _(txt_caching_on));
			else
				wait_message(2, _(txt_caching_off));
		}
#	if 0
	} else {
		/*
		 * TODO: issue warning if old index files found?
		 *	      in index_newsdir?
		 */
#	endif /* 0 */
	}

#	if 0
	/*
	 * TODO: if we're using -n, check for LIST NEWSGROUPS <wildmat>
	 * see also comments in open_newsgroups_fp()
	 */
	if (newsrc_active && !list_active) { /* -n */
		/* code goes here */
	}
#	endif /* 0 */

	if (!is_reconnect && !batch_mode && show_description && check_for_new_newsgroups) {
		/*
		 * TODO:
		 * - document that "-d" and/or "-q" turns off "LIST MOTD" (or add
		 *   a tinrc var to turn LIST MOTD on/off)
		 */
		if (nntp_caps.list_motd)
			list_motd();
	}

	is_reconnect = TRUE;

#endif /* NNTP_ABLE */

	DEBUG_IO((stderr, "nntp_open okay\n"));
	return 0;
}


/*
 * 'Public' function to shutdown the NNTP connection
 */
void
nntp_close(
	void)
{
#ifdef NNTP_ABLE
	if (read_news_via_nntp) {
#	ifdef DEBUG
		debug_nntp("nntp_close", "END");
#	endif /* DEBUG */
		close_server();
	}
#endif /* NNTP_ABLE */
}


/*
 * Get a response code from the server.
 * Returns:
 *	+ve NNTP return code
 *	-1  on an error or user abort. We don't differentiate.
 * If 'message' is not NULL, then any trailing text after the response
 * code is copied into it.
 * Does not perform authentication if required; use get_respcode()
 * instead.
 */
int
get_only_respcode(
	char *message,
	size_t mlen)
{
	int respcode = 0;
#ifdef NNTP_ABLE
	char *ptr, *end;

	ptr = tin_fgets(FAKE_NNTP_FP, FALSE);

	if (tin_errno || ptr == NULL) {
#	ifdef DEBUG
		debug_nntp("<<<", "Error: tin_error<>0 or ptr==NULL in get_only_respcode()");
#	endif /* DEBUG */
		return -1;
	}

#	ifdef DEBUG
	debug_nntp("<<<", ptr);
#	endif /* DEBUG */
	respcode = (int) strtol(ptr, &end, 10);
	DEBUG_IO((stderr, "get_only_respcode(%d)\n", respcode));

	/*
	 * we also reconnect on ERR_FAULT if last_put was ARTICLE or LIST or POST
	 * as inn (2.2.3) sends ERR_FAULT on timeout
	 */
	if (((respcode == ERR_FAULT && !strncmp(last_put, "ARTICLE", 7)) || (respcode == ERR_FAULT && !strncmp(last_put, "POST", 4)) || (respcode == ERR_FAULT && !strcmp(last_put, "LIST")) ||  respcode == ERR_GOODBYE || respcode == OK_GOODBYE) && last_put[0] != '\0' && strcmp(last_put, "QUIT")) {
		/*
		 * Maybe server timed out.
		 * If so, retrying will force a reconnect.
		 */
#	ifdef DEBUG
		debug_nntp("get_only_respcode", "timeout");
#	endif /* DEBUG */
		put_server(last_put);
		ptr = tin_fgets(FAKE_NNTP_FP, FALSE);

		if (tin_errno) {
#	ifdef DEBUG
			debug_nntp("<<<", "Error: tin_errno <> 0");
#	endif /* DEBUG */
			return -1;
		}

#	ifdef DEBUG
		debug_nntp("<<<", ptr);
#	endif /* DEBUG */
		respcode = (int) strtol(ptr, &end, 10);
		DEBUG_IO((stderr, "get_only_respcode(%d)\n", respcode));
	}
	if (message != NULL && mlen > 1)		/* Pass out the rest of the text */
		my_strncpy(message, end, mlen - 1);

#endif /* NNTP_ABLE */
	return respcode;
}


/*
 * Get a response code from the server.
 * Returns:
 *	+ve NNTP return code
 *	-1  on an error
 * If 'message' is not NULL, then any trailing text after the response
 *	code is copied into it.
 * Performs authentication if required and repeats the last command if
 * necessary after a timeout.
 */
int
get_respcode(
	char *message,
	size_t mlen)
{
	int respcode = 0;
#ifdef NNTP_ABLE
	char savebuf[NNTP_STRLEN];
	char *ptr, *end;

	respcode = get_only_respcode(message, mlen);
	if ((respcode == ERR_NOAUTH) || (respcode == NEED_AUTHINFO)) {
		/*
		 * Server requires authentication.
		 */
#	ifdef DEBUG
		debug_nntp("get_respcode", "authentication");
#	endif /* DEBUG */
		strncpy(savebuf, last_put, sizeof(savebuf) - 1);		/* Take copy, as authenticate() will clobber this */

		if (authenticate(nntp_server, userid, FALSE)) {
			if (curr_group != NULL) {
				DEBUG_IO((stderr, _("Rejoin current group\n")));
				snprintf(last_put, sizeof(last_put), "GROUP %s", curr_group->name);
				put_server(last_put);
				s_gets(last_put, NNTP_STRLEN, nntp_rd_fp);
#	ifdef DEBUG
				debug_nntp("<<<", last_put);
#	endif /* DEBUG */
				DEBUG_IO((stderr, _("Read (%s)\n"), last_put));
			}
			strcpy(last_put, savebuf);

			put_server(last_put);
			ptr = tin_fgets(FAKE_NNTP_FP, FALSE);

			if (tin_errno) {
#	ifdef DEBUG
				debug_nntp("<<<", "Error: tin_errno <> 0");
#	endif /* DEBUG */
				return -1;
			}

#	ifdef DEBUG
			debug_nntp("<<<", ptr);
#	endif /* DEBUG */
			respcode = (int) strtol(ptr, &end, 10);
			if (message != NULL && mlen > 1)				/* Pass out the rest of the text */
				strncpy(message, end, mlen - 1);

		} else {
			error_message(_(txt_auth_failed), ERR_ACCESS);
			/*	return -1; */
			tin_done(EXIT_FAILURE);
		}
	}
#endif /* NNTP_ABLE */
	return respcode;
}


#ifdef NNTP_ABLE
/*
 * Do an NNTP command. Send command to server, and read the reply.
 * If the reply code matches success, then return an open file stream
 * Return NULL if we did not see the response we wanted.
 * If message is not NULL, then the trailing text of the reply string is
 * copied into it for the caller to process.
 */
FILE *
nntp_command(
	const char *command,
	int success,
	char *message,
	size_t mlen)
{
DEBUG_IO((stderr, "nntp_command(%s)\n", command));
#	ifdef DEBUG
	debug_nntp("nntp command", command);
#	endif /* DEBUG */
	put_server(command);

	if (!bool_equal(dangerous_signal_exit, TRUE)) {
		if (get_respcode(message, mlen) != success) {
#	ifdef DEBUG
			debug_nntp(command, "NOT_OK");
#	endif /* DEBUG */
			/* error_message("%s", message); */
			return (FILE *) 0;
		}
	}
#	ifdef DEBUG
	debug_nntp(command, "OK");
#	endif /* DEBUG */
	return FAKE_NNTP_FP;
}


/*
 * same as above, but with a slightly more usefull return code.
 * TODO: use it instead of nntp_command in the rest of the code
 *       (wherever it is more usefull).
 */
static int
new_nntp_command(
	const char *command,
	int success,
	char *message,
	size_t mlen)
{
	int respcode = 0;

DEBUG_IO((stderr, "nntp_command(%s)\n", command));
#	ifdef DEBUG
	debug_nntp("nntp command", command);
#	endif /* DEBUG */
	put_server(command);

	if (!bool_equal(dangerous_signal_exit, TRUE)) {
		if ((respcode = get_respcode(message, mlen)) != success) {
#	ifdef DEBUG
			debug_nntp(command, "NOT_OK - Expected: %d, got: %d", success, respcode);
#	endif /* DEBUG */
			return respcode;
		}
	}
#	ifdef DEBUG
	debug_nntp(command, "OK");
#	endif /* DEBUG */
	return respcode;
}


static void
list_motd(
	void)
{
	char *ptr;
	char buf[NNTP_STRLEN];
	int i;
	unsigned int l = 0;

	buf[0] = '\0';
	i = new_nntp_command("LIST MOTD", OK_MOTD, buf, sizeof(buf));

	switch (i) {
		case OK_MOTD:
#	ifdef HAVE_COLOR
			fcol(tinrc.col_message);
#	endif /* HAVE_COLOR */
			while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#	ifdef DEBUG
				debug_nntp("<<<", ptr);
#	endif /* DEBUG */
				/*
				 * TODO: - store a hash value of the entire motd in the server-rc
				 *         and only if it differs from the old value display the
				 *         motd?
				 *       - use some sort of pager?
				 *       - -> lang.c
				 */
				my_printf("%s%s\n", _("MOTD: "), ptr);
				l++;
			}
#	ifdef HAVE_COLOR
			fcol(tinrc.col_normal);
#	endif /* HAVE_COLOR */
			if (l) {
				my_flush();
				sleep((l >> 1) | 0x01);
			}
			break;

		default:	/* common response codes are 500, 501, 503 */
			break;
	}
}
#endif /* NNTP_ABLE */
