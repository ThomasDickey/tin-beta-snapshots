/*
 *  Project   : tin - a Usenet reader
 *  Module    : nntplib.c
 *  Author    : S. Barber & I. Lea
 *  Created   : 1991-01-12
 *  Updated   : 2017-01-31
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

#ifdef NNTP_ABLE
	/* Flag to show whether tin did reconnect in last get_server() */
	t_bool reconnected_in_last_get_server = FALSE;
	/* Flag used in LIST ACVTIVE loop */
	t_bool did_reconnect = FALSE;
#endif /* NNTP_ABLE */

static TCP *nntp_rd_fp = NULL;

#ifdef NNTP_ABLE
	static TCP *nntp_wr_fp = NULL;
	/* Copy of last NNTP command sent, so we can retry it if needed */
	static char last_put[NNTP_STRLEN];
	static constext *xover_cmds = "XOVER";
	static constext *xhdr_cmds = "XHDR";
	/* Set so we don't reconnect just to QUIT */
	static t_bool quitting = FALSE;
#endif /* NNTP_ABLE */

/*
 * local prototypes
 */
#ifdef NNTP_ABLE
	static int mode_reader(t_bool *sec);
	static int reconnect(int retry);
	static int server_init(char *machine, const char *cservice, unsigned short port, char *text, size_t mlen);
	static int check_extensions(void);
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


#if 0 /* unused */
FILE *
get_nntp_wr_fp(
	FILE *fp)
{
	return (fp == FAKE_NNTP_FP ? nntp_wr_fp : fp);
}
#endif /* 0 */

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

	if (read_saved_news) {
		STRCPY(buf, "reading saved news");
		return buf;
	}

	if (!read_news_via_nntp) {
		STRCPY(buf, "local");	/* what if a server is named "local"? */
		return buf;
	}

#ifdef NNTP_ABLE
	if (cmdline.args & CMDLINE_NNTPSERVER) {
		get_nntpserver(buf, sizeof(buf), cmdline.nntpserver);
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
		get_nntpserver(buf, sizeof(buf), cp);
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
	}

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
	unsigned short port,
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
	sockt_rd = get_tcp6_socket(machine, port);
#		else
	sockt_rd = get_tcp_socket(machine, service, port);
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
		s_fclose(nntp_rd_fp);
		nntp_rd_fp = NULL;
		return -EPROTO;
	}
#	else
	if ((nntp_wr_fp = (TCP *) s_fdopen(sockt_wr, "w")) == NULL) {
		perror("server_init: fdopen() #2");
		s_fclose(nntp_rd_fp);
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

	if (!isdigit((unsigned char) *machine)
#		ifdef HAVE_INET_ATON
	    || !inet_aton(machine, &sock_in)
#		else
#			ifdef HAVE_INET_ADDR
	    || (long) (sock_in.sin_addr.s_addr = inet_addr(machine)) == INADDR_NONE
#			endif /* HAVE_INET_ADDR */
#		endif /* HAVE_INET_ATON */
	) {
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
			fprintf(stderr, "%s", _(txt_error_server_unavailable));
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
#			endif /* h_addr */
	static char *alist[2] = {0, 0};
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
	if (!isdigit((unsigned char) *machine)
#			ifdef HAVE_INET_ATON
	    || !inet_aton(machine, &defaddr)
#			else
#				ifdef HAVE_INET_ADDR
	    || (long) (defaddr.s_addr = (long) inet_addr(machine)) == -1
#				endif /* HAVE_INET_ADDR */
#			endif /* HAVE_INET_ATON */
	    )
	{
		hp = gethostbyname(machine);
	} else {
		/* Raw ip address, fake */
		STRCPY(namebuf, machine);
		def.h_name = (char *) namebuf;
		def.h_addr_list = alist;
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
		my_fprintf(stderr, "%s", _(txt_giving_up));
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
	snprintf(myport, sizeof(myport), "%u", port);

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
	hints.ai_family = (force_ipv4 ? AF_INET : (force_ipv6 ? AF_INET6 : ADDRFAM));
	hints.ai_socktype = SOCK_STREAM;
	res0 = (struct addrinfo *) 0;
	if ((err = getaddrinfo(mymachine, myport, &hints, &res0))) {
		my_fprintf(stderr, "\ngetaddrinfo: %s\n", gai_strerror(err));
		return -1;
	}
	err = -1;
	for (res = res0; res; res = res->ai_next) {
		if ((s = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
			continue;
		if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
			s_close(s);
		else {
			err = 0;
			break;
		}
	}
	if (res0 != NULL)
		freeaddrinfo(res0);
	if (err < 0) {
		/*
		 * TODO: issue a more useful error-message
		 */
		my_fprintf(stderr, "%s", _(txt_error_socket_or_connect_problem));
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
		s_close(s);
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
	if (debug & DEBUG_NNTP) {
		if (strcmp(string, "\r\n"))
			debug_print_file("NNTP", ">>>%s%s", logtime(), string);
	}
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
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", ">>>%s%s", logtime(), string);
#	endif /* DEBUG */
		/*
		 * remember the last command we wrote to be able to resend it after a
		 * reconnect. reconnection is handled by get_server()
		 *
		 * don't cache "LIST [ACTIVE|COUNTS|NEWSGROUPS] something" as we
		 * would need to resend all of them but we remember just the last
		 * one. we cache "LIST cmd." instead, this will slow down things, but
		 * that's ok on reconnect.
		 */
		if (strcmp(last_put, string))
			STRCPY(last_put, string);
		if (!strncmp(string, "LIST ACTIVE ", 12))
			last_put[11] = '\0'; /* "LIST ACTIVE" */
		else if (!strncmp(string, "LIST COUNTS ", 12))
			last_put[11] = '\0'; /* "LIST COUNTS" */
		else if (!strncmp(string, "LIST NEWSGROUPS ", 16))
			last_put[15] = '\0'; /* "LIST NEWSGROUPS" */
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
	int save_signal_context = signal_context;

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
	 * set signal_context temporary to cReconnect to avoid trouble when receiving
	 * SIGWINCH while being in prompt_yn()
	 */
	signal_context = cReconnect;

	/*
	 * Exit tin if there are no more tries or if the user says no to reconnect.
	 * The exit code stops tin from trying to disconnect again - the connection
	 * is already dead
	 */
	if (retry > NNTP_TRY_RECONNECT || (!tinrc.auto_reconnect && prompt_yn(_(txt_reconnect_to_news_server), TRUE) != 1)) {
		if (!strcmp("POST", last_put)) {
			unlink(backup_article_name(article_name));
			rename_file(article_name, dead_article);
			if (tinrc.keep_dead_articles)
				append_file(dead_article, dead_articles);
		}
		if (retry > NNTP_TRY_RECONNECT) {
#	ifdef DEBUG
			/* TODO: -> lang.c */
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "reconnect(%d) limit %d reached, giving up.", retry, NNTP_TRY_RECONNECT);
#	endif /* DEBUG */
		}
		tin_done(NNTP_ERROR_EXIT, _("NNTP connection error. Exiting..."));		/* user said no to reconnect or no more retries */
	}

	/* reset signal_context */
	signal_context = save_signal_context;

	clear_message();
	strcpy(buf, last_put);			/* Keep copy here, it will be clobbered a lot otherwise */

	if (!nntp_open()) {
		/* Re-establish our current group and resend last command */
		if (curr_group != NULL) {
			DEBUG_IO((stderr, _("Rejoin current group\n")));
			snprintf(last_put, sizeof(last_put), "GROUP %s", curr_group->name);
			put_server(last_put);
			s_gets(last_put, NNTP_STRLEN, nntp_rd_fp);
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "<<<%s%s", logtime(), last_put);
#	endif /* DEBUG */
			DEBUG_IO((stderr, _("Read (%s)\n"), last_put));
		}
		DEBUG_IO((stderr, _("Resend last command (%s)\n"), buf));
		put_server(buf);
		did_reconnect = TRUE;
		retry = NNTP_TRY_RECONNECT;
	}

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
	static int retry_cnt = 0;

	reconnected_in_last_get_server = FALSE;
	errno = 0;

	/*
	 * NULL socket reads indicates socket has closed. Try a few times more
	 *
	 * Leave the s_gets() after a timeout for these cases:
	 *   -some servers do not close the connection but simply do not send any
	 *    response data
	 *   -the network connection went down
	 */
#	if defined(HAVE_ALARM) && defined(SIGALRM)
	alarm(tinrc.nntp_read_timeout_secs);
#	endif /* HAVE_ALARM && SIGALRM */
	while (nntp_rd_fp == NULL || s_gets(string, size, nntp_rd_fp) == NULL) {
		if (errno == EINTR) {
			errno = 0;
#	if defined(HAVE_ALARM) && defined(SIGALRM)
			alarm(tinrc.nntp_read_timeout_secs);		/* Restart the timer */
#	endif /* HAVE_ALARM && SIGALRM */
			continue;
		}
#	if defined(HAVE_ALARM) && defined(SIGALRM)
		alarm(0);
#	endif /* HAVE_ALARM && SIGALRM */
		if (quitting)						/* Don't bother to reconnect */
			tin_done(NNTP_ERROR_EXIT, NULL);		/* And don't try to disconnect again! */

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
		if (strcmp(last_put, "QUIT")) {
			/*
			 * Typhoon v2.1.1.363 colses the connection right after an unknown
			 * command, (i.e. CAPABILITIES) so we avoid the reissue it on a
			 * reconnect if it was the last command.
			 */
			if (!strcmp(last_put, "CAPABILITIES")) {
				strcpy(last_put, "MODE READER");
				nntp_caps.type = BROKEN;
			}
			retry_cnt = reconnect(++retry_cnt);		/* Will abort when out of tries */
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
#	if defined(HAVE_ALARM) && defined(SIGALRM)
	alarm(0);
#	endif /* HAVE_ALARM && SIGALRM */
	retry_cnt = 0;
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


/*
 * Try and use CAPABILITIES here. Get this list before issuing other NNTP
 * commands because the correct methods may be mentioned in the list of
 * extensions.
 *
 * Sets up: t_capabilities nntp_caps
 */
static int
check_extensions(
	void)
{
	char *d;
	char *ptr;
	char buf[NNTP_STRLEN];
	int i;
	int ret = 0;

	buf[0] = '\0';
	i = new_nntp_command("CAPABILITIES", INF_CAPABILITIES, buf, sizeof(buf));
	switch (i) {
		case INF_CAPABILITIES:
			/* clear capabilities */
			nntp_caps.type = CAPABILITIES;
			nntp_caps.version = 0;
			nntp_caps.mode_reader = FALSE;
			nntp_caps.reader = FALSE;
			nntp_caps.post = FALSE;
			nntp_caps.list_active = FALSE;
			nntp_caps.list_active_times = FALSE;
			nntp_caps.list_distrib_pats = FALSE;
			nntp_caps.list_headers = FALSE;
			nntp_caps.list_newsgroups = FALSE;
			nntp_caps.list_overview_fmt = FALSE;
			nntp_caps.list_motd = FALSE;
			nntp_caps.list_subscriptions = FALSE;
			nntp_caps.list_distributions = FALSE;
			nntp_caps.list_moderators = FALSE;
			nntp_caps.list_counts = FALSE;
			nntp_caps.xpat = TRUE; /* only used in select.c:lookup_msgid(); toggles to false if fails, INN > 2.7.0 announces it */
			nntp_caps.hdr = FALSE;
			nntp_caps.hdr_cmd = NULL;
			nntp_caps.over = FALSE;
			nntp_caps.over_msgid = FALSE;
			nntp_caps.over_cmd = NULL;
			nntp_caps.newnews = FALSE;
			FreeAndNull(nntp_caps.implementation);
			nntp_caps.starttls = FALSE;
			nntp_caps.authinfo_user = FALSE;
			nntp_caps.authinfo_sasl = FALSE;
			nntp_caps.authinfo_state = FALSE;
			nntp_caps.sasl = SASL_NONE;
			nntp_caps.compress = FALSE;
			nntp_caps.compress_algorithm = COMPRESS_NONE;
#if 0
			nntp_caps.streaming = FALSE;
			nntp_caps.ihave = FALSE;
#endif /* 0 */
#ifndef BROKEN_LISTGROUP
			nntp_caps.broken_listgroup = FALSE;
#else
			nntp_caps.broken_listgroup = TRUE;
#endif /* !BROKEN_LISTGROUP */

			while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#		ifdef DEBUG
				if (debug & DEBUG_NNTP)
					debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#		endif /* DEBUG */
				/* look for version number(s) */
				if (!nntp_caps.version && nntp_caps.type == CAPABILITIES) {
					if (!strncasecmp(ptr, "VERSION", 7)) {
						d = ptr + 7;
						d = strpbrk(d, " \t");
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							nntp_caps.version = (unsigned int) atoi(d);
							d = strpbrk(d, " \t");
						}
					}
				}
				/* we currently only support CAPABILITIES VERSION 2 */
				if (nntp_caps.version == 2) {
					/*
					 * check for LIST variants
					 */
					if (!strncasecmp(ptr, "LIST", 4)) {
						d = ptr + 4;
						d = strpbrk(d, " \t");
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							if (!strncasecmp(d, "ACTIVE.TIMES", 12))
								nntp_caps.list_active_times = TRUE;
							else if (!strncasecmp(d, "ACTIVE", 6))
								nntp_caps.list_active = TRUE;
							else if (!strncasecmp(d, "DISTRIB.PATS", 12))
								nntp_caps.list_distrib_pats = TRUE;
							else if (!strncasecmp(d, "DISTRIBUTIONS", 13)) /* RFC 6048 */
								nntp_caps.list_distributions = TRUE;
							else if (!strncasecmp(d, "HEADERS", 7))
								nntp_caps.list_headers = TRUE; /* HDR requires LIST HEADERS, but not vice versa */
							else if (!strncasecmp(d, "NEWSGROUPS", 10))
								nntp_caps.list_newsgroups = TRUE;
							else if (!strncasecmp(d, "OVERVIEW.FMT", 12)) /* OVER requires OVERVIEW.FMT, but not vice versa */
								nntp_caps.list_overview_fmt = TRUE;
							else if (!strncasecmp(d, "MOTD", 4)) /* RFC 6048 */
								nntp_caps.list_motd = TRUE;
							else if (!strncasecmp(d, "SUBSCRIPTIONS", 13)) /* RFC 6048 */
								nntp_caps.list_subscriptions = TRUE;
							else if (!strncasecmp(d, "MODERATORS", 10)) /* RFC 6048 */
								nntp_caps.list_moderators = TRUE;
							else if (!strncasecmp(d, "COUNTS", 6)) /* RFC 6048 */
								nntp_caps.list_counts = TRUE;
							d = strpbrk(d, " \t");
						}
					} else if (!strncasecmp(ptr, "IMPLEMENTATION", 14)) {
						FreeIfNeeded(nntp_caps.implementation);
						nntp_caps.implementation = my_strdup(ptr + 14);
						str_trim(nntp_caps.implementation);
					} else if (!strcasecmp(ptr, "MODE-READER")) {
						if (!nntp_caps.reader)
							nntp_caps.mode_reader = TRUE;
					} else if (!strcasecmp(ptr, "READER")) { /* if we saw READER, "LIST ACTIVE" and "LIST NEWSGROUPS" must be implemented */
						nntp_caps.reader = TRUE;
						nntp_caps.mode_reader = FALSE;
						nntp_caps.list_newsgroups = TRUE;
						nntp_caps.list_active = TRUE;
					} else if (!strcasecmp(ptr, "POST"))
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
					else if (!strncasecmp(ptr, &xover_cmds[1], strlen(&xover_cmds[1]))) {
						nntp_caps.list_overview_fmt = nntp_caps.over = TRUE;
						nntp_caps.over_cmd = &xover_cmds[1];
						d = ptr + strlen(&xover_cmds[1]);
						d = strpbrk(d, " \t");
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							if (!strcasecmp(d, "MSGID"))
								nntp_caps.over_msgid = TRUE;
							d = strpbrk(d, " \t");
						}
					}
					/*
					 * NOTE: if we saw HDR, LIST HEADERS _must_ be implemented
					 */
					else if (!strncasecmp(ptr, &xhdr_cmds[1], strlen(&xhdr_cmds[1]))) {
						nntp_caps.hdr_cmd = &xhdr_cmds[1];
						nntp_caps.list_headers = nntp_caps.hdr = TRUE;
					} else if (!strncasecmp(ptr, "AUTHINFO", 8)) {
						d = ptr + 8;
						d = strpbrk(d, " \t");
						if (d == NULL) /* AUTHINFO without args */
							nntp_caps.authinfo_state = TRUE;
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							if (!strncasecmp(d, "USER", 4))
								nntp_caps.authinfo_user = TRUE;
							if (!strncasecmp(d, "SASL", 4))
								nntp_caps.authinfo_sasl = TRUE;
							d = strpbrk(d, " \t");
						}
					} else if (!strncasecmp(ptr, "SASL", 4)) {
						d = ptr + 4;
						d = strpbrk(d, " \t");
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							if (!strncasecmp(d, "CRAM-MD5", 8)) { /* RFC 2195 */
								nntp_caps.authinfo_sasl = TRUE;
								nntp_caps.sasl |= SASL_CRAM_MD5;
							}
							if (!strncasecmp(d, "DIGEST-MD5", 10)) { /* RFC 2831 */
								nntp_caps.authinfo_sasl = TRUE;
								nntp_caps.sasl |= SASL_DIGEST_MD5;
							}
							if (!strncasecmp(d, "PLAIN", 5)) { /* RFC 4616 */
								nntp_caps.authinfo_sasl = TRUE;
								nntp_caps.sasl |= SASL_PLAIN;
							}
							if (!strncasecmp(d, "GSSAPI", 6)) { /* RFC 4752 */
								nntp_caps.authinfo_sasl = TRUE;
								nntp_caps.sasl |= SASL_GSSAPI;
							}
							if (!strncasecmp(d, "EXTERNAL", 8)) { /* RFC 4422 */
								nntp_caps.authinfo_sasl = TRUE;
								nntp_caps.sasl |= SASL_EXTERNAL;
							}
							/* inn also can do these */
							if (!strncasecmp(d, "OTP", 3)) { /* RFC 2444 */
								nntp_caps.authinfo_sasl = TRUE;
								nntp_caps.sasl |= SASL_OTP;
							}
							if (!strncasecmp(d, "NTLM", 4)) { /* Microsoft */
								nntp_caps.authinfo_sasl = TRUE;
								nntp_caps.sasl |= SASL_NTLM;
							}
							if (!strncasecmp(d, "LOGIN", 5)) { /* Microsoft */
								nntp_caps.authinfo_sasl = TRUE;
								nntp_caps.sasl |= SASL_LOGIN;
							}
						}
					} else if (!strncasecmp(ptr, "COMPRESS", 8)) { /* RFC 8054 */
						d = ptr + 8;
						d = strpbrk(d, " \t");
						while (d != NULL && (d + 1 < (ptr + strlen(ptr)))) {
							d++;
							if (!strncasecmp(d, "DEFLATE", 7)) {
								nntp_caps.compress = TRUE;
								nntp_caps.compress_algorithm |= COMPRESS_DEFLATE;
							}
						}
					}
#		if 0 /* we don't need these */
					else if (!strcasecmp(ptr, "IHAVE"))
						nntp_caps.ihave = TRUE;
					else if (!strcasecmp(ptr, "STREAMING"))
						nntp_caps.streaming = TRUE;
#		endif /* 0 */
				/* XZVER, XZHDR, ... */
				} else
					nntp_caps.type = NONE;
			}
			break;

		/*
		 * XanaNewz 2 Server Version 2.0.0.3 doesn't know CAPABILITIES
		 * but respondes with 400 _without_ closing the connection. If
		 * you use tin on a XanaNewz 2 Server comment out the following
		 * case.
		 */
#		if 1
		case ERR_GOODBYE:
			ret = i;
			error_message(2, "%s", buf);
			break;
#		endif /* 1 */

		default:
			break;
	}

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
#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "mode_reader() MODE READER");
#	endif /* DEBUG */
		DEBUG_IO((stderr, "nntp_command(MODE READER)\n"));
		put_server("MODE READER");

		/*
		 * According to RFC 3977 (5.3), MODE READER may only return the
		 * following response codes:
		 *
		 *   200 (OK_CANPOST)     Hello, you can post
		 *   201 (OK_NOPOST)      Hello, you can't post
		 *   502 (ERR_ACCESS)     Service unavailable
		 *
		 * However, there are servers out there (e.g. Delegate 9.8.x) that do
		 * not implement this command and therefore return ERR_COMMAND (500).
		 * Unfortunately there are some new servers out there (i.e. INN 2.4.0
		 * (20020220 prerelease) which do return ERR_COMMAND if they are feed
		 * only servers.
		 */

		switch ((ret = get_respcode(line, sizeof(line)))) {
			case OK_CANPOST:
				can_post = TRUE && !force_no_post;
				*sec = TRUE;
				ret = 0;
				break;

			case OK_NOPOST:
				can_post = FALSE;
				*sec = TRUE;
				ret = 0;
				break;

			case ERR_GOODBYE:
			case ERR_ACCESS:
				error_message(2, "%s", line);
				return ret;

			case ERR_COMMAND:
#	if 1
				ret = 0;
				break;
#	endif /* 1 */

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
	char line[NNTP_STRLEN]= { '\0' };
	int ret;
	t_bool sec = FALSE;
	/* It appears that is_reconnect guards code that should be run only once */
	static t_bool is_reconnect = FALSE;

	if (!read_news_via_nntp)
		return 0;

#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "nntp_open() BEGIN");
#	endif /* DEBUG */

	if (nntp_server == NULL) {
		error_message(2, _(txt_cannot_get_nntp_server_name));
		error_message(2, _(txt_server_name_in_file_env_var), NNTP_SERVER_FILE);
		return -EHOSTUNREACH;
	}

	if (!batch_mode || verbose) {
		if (nntp_tcp_port != IPPORT_NNTP)
			wait_message(0, _(txt_connecting_port), nntp_server, nntp_tcp_port);
		else
			wait_message(0, _(txt_connecting), nntp_server);
	}

#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "nntp_open() %s:%d", nntp_server, nntp_tcp_port);
#	endif /* DEBUG */

	ret = server_init(nntp_server, NNTP_TCP_NAME, nntp_tcp_port, line, sizeof(line));
	DEBUG_IO((stderr, "server_init returns %d,%s\n", ret, line));

	if ((!batch_mode || verbose) && ret >= 0)
		my_fputc('\n', stdout);

#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "nntp_open() %s", line);
#	endif /* DEBUG */

	switch (ret) {
		/*
		 * ret < 0 : some error from system call
		 * ret > 0 : NNTP response code
		 *
		 * According to the ietf-nntp mailinglist:
		 *   200 you may (try to) do anything
		 *   201 you may not POST
		 *   All unrecognised 200 series codes should be assumed as success.
		 *   All unrecognised 300 series codes should be assumed as notice to continue.
		 *   All unrecognised 400 series codes should be assumed as temporary error.
		 *   All unrecognised 500 series codes should be assumed as error.
		 */

		case OK_CANPOST:
			can_post = TRUE && !force_no_post;
			break;

		case OK_NOPOST:
			can_post = FALSE;
			break;

		default:
			if (ret >= 200 && ret <= 299) {
				can_post = TRUE && !force_no_post;
				break;
			}
			if (ret < 0)
				error_message(2, _(txt_failed_to_connect_to_server), nntp_server);
			else
				error_message(2, "%s", line);

			return ret;
	}
	if (!is_reconnect && *line) {
		/* remove leading whitespace and save server's initial response */
		linep = line;
		while (isspace((int) *linep))
			linep++;

		STRCPY(bug_nntpserver1, linep);
	}

	/*
	 * Find out which NNTP extensions are available
	 * - Typhoon v2.1.1.363 closes the connection after an unknown command
	 *   (i.e. CAPABILITIES) but as we are not allowed to cache CAPABILITIES
	 *   we reissue the command on reconnect. To prevent a loop we catch this
	 *   case.
	 *
	 * TODO: The authentication method required may be mentioned in the list
	 *       of extensions. (For details about authentication methods, see
	 *       RFC 4643).
	 */
	if (nntp_caps.type != BROKEN)
		check_extensions();

	/*
	 * If the user wants us to authenticate on connection startup, do it now.
	 * Some news servers return "201 no posting" first, but after successful
	 * authentication you get a "200 posting allowed". To find out if we are
	 * allowed to post after authentication issue a "MODE READER" again and
	 * interpret the response code.
	 */

	if (nntp_caps.type == CAPABILITIES && !nntp_caps.reader) {
		if (nntp_caps.mode_reader) {
			char buf[NNTP_STRLEN];

#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "nntp_open() MODE READER");
#	endif /* DEBUG */
			put_server("MODE READER");
			switch (get_only_respcode(buf, sizeof(buf))) {
				/* just honor ciritical errors */
				case ERR_GOODBYE:
				case ERR_ACCESS:
					error_message(2, "%s", buf);
					return -1;

				default:
					break;
			}
			check_extensions();
		}
	}

	if (force_auth_on_conn_open) {
#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "nntp_open() authenticate(force_auth_on_conn_open)");
#	endif /* DEBUG */

		if (!authenticate(nntp_server, userid, FALSE))	/* 3rd parameter is FALSE as we need to get prompted for username password here */
			return -1;

		if (nntp_caps.type == CAPABILITIES)
			check_extensions();
	}

	if ((nntp_caps.type == CAPABILITIES && nntp_caps.mode_reader) || nntp_caps.type != CAPABILITIES) {
		if ((ret = mode_reader(&sec))) {
			if (nntp_caps.type == CAPABILITIES)
				can_post = nntp_caps.post && !force_no_post;

			return ret;
		}
		if (nntp_caps.type == CAPABILITIES)
			check_extensions();
	}

	if (nntp_caps.type == CAPABILITIES) {
		if (!nntp_caps.reader) {
			error_message(2, _("CAPABILITIES did not announce READER")); /* TODO: -> lang.c */
			return -1; /* give up */
		}
		can_post = nntp_caps.post && !force_no_post;
	}

	if (!is_reconnect && *line) {
#	if 0
	/*
	 * gives wrong results if RFC 3977 server requestes auth after
	 * CAPABILITIES is parsed (with no posting allowed) and after auth
	 * posting is allowed. as we will inform the user later on when he
	 * actually tries to post it should do no harm to skip this message
	 */
		/* Inform user if he cannot post */
		if (!can_post && !batch_mode)
			wait_message(0, "%s\n", _(txt_cannot_post));
#	endif /* 0 */

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
	 * If CAPABILITIES failed, check if NNTP supports XOVER or OVER command
	 * We have to check that we _don't_ get an ERR_COMMAND
	 */
	if (nntp_caps.type != CAPABILITIES) {
		int i, j = 0;

		for (i = 0; i < 2 && j >= 0; i++) {
			j = new_nntp_command(&xover_cmds[i], ERR_NCING, line, sizeof(line));
			switch (j) {
				case ERR_COMMAND:
					break;

				case OK_XOVER:	/* unexpected multiline ok, e.g.: Synchronet 3.13 NNTP Service 1.92 or on reconnect if last cmd was GROUP */
					nntp_caps.over_cmd = &xover_cmds[i];
#	ifdef DEBUG
					if (debug & DEBUG_NNTP)
						debug_print_file("NNTP", "nntp_open() %s skipping data", &xover_cmds[i]);
#	endif /* DEBUG */
					while (tin_fgets(FAKE_NNTP_FP, FALSE))
						;
					j = -1;
					break;

				default:
					nntp_caps.over_cmd = &xover_cmds[i];
					j = -1;
					break;
			}
		}
		for (i = 0, j = 0; i < 2 && j >= 0; i++) {
			j = new_nntp_command(&xhdr_cmds[i], ERR_CMDSYN, line, sizeof(line));
			switch (j) {
				case ERR_COMMAND:
					break;

				case 221:	/* unexpected multiline ok, e.g.: SoftVelocity Discussions 2.5q */
					nntp_caps.hdr_cmd = &xhdr_cmds[i];
#		ifdef DEBUG
					if (debug & DEBUG_NNTP)
						debug_print_file("NNTP", "nntp_open() %s skipping data", &xhdr_cmds[i]);
#		endif /* DEBUG */
					while (tin_fgets(FAKE_NNTP_FP, FALSE))
						;
					j = -1;
					break;

				default:	/* usually ERR_CMDSYN (args missing), Typhoon/Twister sends ERR_NCING */
					nntp_caps.hdr_cmd = &xhdr_cmds[i];
					j = -1;
					break;
			}
		}
		/* no XPAT probing here, we do it in select.c:lookup_msgid() as that's the only place we use it */
		nntp_caps.xpat = TRUE;
#		if 0
		switch (new_nntp_command("XPAT Newsgroups <0> *", ERR_NOART, line, sizeof(line))) {
			case ERR_NOART:
				nntp_caps.xpat = TRUE;
				break;

			default:
				break;
		}
#		endif /* 0 */
	} else {
		if (!nntp_caps.over_cmd) {
			/*
			 * CAPABILITIES didn't mention OVER or XOVER, try XOVER
			 */
			switch (new_nntp_command(xover_cmds, ERR_NCING, line, sizeof(line))) {
				case ERR_COMMAND:
					break;

				case OK_XOVER:	/* unexpected multiline ok, e.g.: Synchronet 3.13 NNTP Service 1.92 or on reconnect if last cmd was GROUP */
					nntp_caps.over_cmd = xover_cmds;
#	ifdef DEBUG
					if (debug & DEBUG_NNTP)
						debug_print_file("NNTP", "nntp_open() %s skipping data", xover_cmds);
#	endif /* DEBUG */
					while (tin_fgets(FAKE_NNTP_FP, FALSE))
						;
					break;

				default:
					nntp_caps.over_cmd = xover_cmds;
					break;
			}
		}
		if (!nntp_caps.hdr_cmd) {
			/*
			 * CAPABILITIES didn't mention HDR or XHDR, try XHDR
			 */
			switch (new_nntp_command(xhdr_cmds, ERR_NCING, line, sizeof(line))) {
				case ERR_COMMAND:
					break;

				case 221:	/* unexpected multiline ok, e.g.: SoftVelocity Discussions 2.5q */
					nntp_caps.hdr_cmd = xhdr_cmds;
#		ifdef DEBUG
					if (debug & DEBUG_NNTP)
						debug_print_file("NNTP", "nntp_open() %s skipping data", xhdr_cmds);
#		endif /* DEBUG */
					while (tin_fgets(FAKE_NNTP_FP, FALSE))
						;
					break;

				default:	/* ERR_NCING or ERR_CMDSYN */
					nntp_caps.hdr_cmd = xhdr_cmds;
					break;
			}
		}
	}

	if (!nntp_caps.over_cmd) {
		if (!is_reconnect && !batch_mode) {
			wait_message(2, _(txt_no_xover_support));

			if (tinrc.cache_overview_files)
				wait_message(2, _(txt_caching_on));
			else
				wait_message(2, _(txt_caching_off));
		}
	}
#	if 0
	else {
		/*
		 * TODO: issue warning if old index files found?
		 *	      in index_newsdir?
		 */
	}
#	endif /* 0 */

	if (!is_reconnect && !batch_mode && show_description && check_for_new_newsgroups) {
		/*
		 * TODO:
		 * - add a tinrc var to turn LIST MOTD on/off?
		 *   (currently done automatically for -d, -q and -Q)
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
	if (read_news_via_nntp && !read_saved_news) {
#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "nntp_close() END");
#	endif /* DEBUG */
		close_server();
	}
#endif /* NNTP_ABLE */
}


#ifdef NNTP_ABLE
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
	int respcode;
	char *end, *ptr;

	ptr = tin_fgets(FAKE_NNTP_FP, FALSE);

	if (tin_errno || ptr == NULL) {
#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "<<<%sError: tin_error<>0 or ptr==NULL in get_only_respcode()", logtime());
#	endif /* DEBUG */
		return -1;
	}

#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
	respcode = (int) strtol(ptr, &end, 10);
	if (end == ptr) /* no leading numbers in response */
		respcode = -1;
	DEBUG_IO((stderr, "get_only_respcode(%d)\n", respcode));

	/*
	 * we also reconnect on ERR_FAULT if last_put was ARTICLE or LIST or POST
	 * as inn (2.2.3) sends ERR_FAULT on timeout
	 *
	 * what about other LIST cmds? (ACTIVE|COUNTS|OVERVIEW.FMT|...)
	 */
	if (last_put[0] != '\0' && ((respcode == ERR_FAULT && (!strncmp(last_put, "ARTICLE", 7) || !strcmp(last_put, "POST") || !strcmp(last_put, "LIST"))) || respcode == ERR_GOODBYE || respcode == OK_GOODBYE) && strcmp(last_put, "QUIT")) {
		/*
		 * Maybe server timed out.
		 * If so, retrying will force a reconnect.
		 */
#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "get_only_respcode() timeout");
#	endif /* DEBUG */
		put_server(last_put);
		ptr = tin_fgets(FAKE_NNTP_FP, FALSE);

		if (tin_errno || ptr == NULL) {
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "<<<%sError: tin_errno<>0 or ptr==NULL in get_only_respcode(retry)", logtime());
#	endif /* DEBUG */
			return -1;
		}

#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
		respcode = (int) strtol(ptr, &end, 10);
		if (end == ptr) /* no leading numbers in response */
			respcode = -1;
		DEBUG_IO((stderr, "get_only_respcode(%d)\n", respcode));
	}
	if (message != NULL && mlen > 1 && *end != '\0')		/* Pass out the rest of the text */
		my_strncpy(message, ++end, mlen - 1);

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
 *
 * TODO: make this handle 401 and 483 (RFC 3977) return codes.
 *       as 401 requires to examine the returned text besides the
 *       return value, we have to "fix" all nntp_command(..., NULL, 0) and
 *       get_only_respcode(NULL, 0) calls to do this properly.
 */
int
get_respcode(
	char *message,
	size_t mlen)
{
	int respcode;
	char savebuf[NNTP_STRLEN];
	char *ptr, *end;

	respcode = get_only_respcode(message, mlen);
	if ((respcode == ERR_NOAUTH) || (respcode == NEED_AUTHINFO)) {
		/*
		 * Server requires authentication.
		 */
#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "get_respcode() authentication");
#	endif /* DEBUG */
		STRCPY(savebuf, last_put);

		if (!authenticate(nntp_server, userid, FALSE))
			tin_done(EXIT_FAILURE, _(txt_auth_failed), nntp_caps.type == CAPABILITIES ? ERR_AUTHFAIL : ERR_ACCESS);

		if (nntp_caps.type == CAPABILITIES) {
			check_extensions();
			can_post = nntp_caps.post && !force_no_post;
		} else {
			put_server("MODE READER");
			if (get_only_respcode(message, mlen) == OK_CANPOST)
				can_post = TRUE && !force_no_post;
		}
		if (curr_group != NULL) {
			DEBUG_IO((stderr, _("Rejoin current group\n")));
			snprintf(last_put, sizeof(last_put), "GROUP %s", curr_group->name);
			put_server(last_put);
			s_gets(last_put, NNTP_STRLEN, nntp_rd_fp);
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "<<<%s%s", logtime(), last_put);
#	endif /* DEBUG */
			DEBUG_IO((stderr, _("Read (%s)\n"), last_put));
		}
		STRCPY(last_put, savebuf);

		put_server(last_put);
		ptr = tin_fgets(FAKE_NNTP_FP, FALSE);

		if (tin_errno) {
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "<<<%sError: tin_errno <> 0", logtime());
#	endif /* DEBUG */
			return -1;
		}

#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
		if (ptr == NULL)
			return -1;

		respcode = (int) strtol(ptr, &end, 10);
		if (end == ptr)	/* no leading numbers in response */
			return -1;

		if (message != NULL && mlen > 1)				/* Pass out the rest of the text */
			strncpy(message, end, mlen - 1);
	}
	return respcode;
}


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
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "nntp_command(%s)", command);
#	endif /* DEBUG */
	put_server(command);

	if (!bool_equal(dangerous_signal_exit, TRUE)) {
		if (get_respcode(message, mlen) != success) {
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "nntp_command(%s) NOT_OK", command);
#	endif /* DEBUG */
			/* error_message(2, "%s", message); */
			return (FILE *) 0;
		}
	}
#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "nntp_command(%s) OK", command);
#	endif /* DEBUG */
	return FAKE_NNTP_FP;
}


/*
 * same as above, but with a slightly more useful return code.
 * TODO: use it instead of nntp_command in the rest of the code
 *       (wherever it is more useful).
 */
int
new_nntp_command(
	const char *command,
	int success,
	char *message,
	size_t mlen)
{
	int respcode = 0;

DEBUG_IO((stderr, "new_nntp_command(%s)\n", command));
#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "new_nntp_command(%s)", command);
#	endif /* DEBUG */
	put_server(command);

	if (!bool_equal(dangerous_signal_exit, TRUE)) {
		if ((respcode = get_respcode(message, mlen)) != success) {
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "new_nntp_command(%s) NOT_OK - Expected: %d, got: %d", command, success, respcode);
#	endif /* DEBUG */
			return respcode;
		}
	}
#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "new_nntp_command(%s) OK", command);
#	endif /* DEBUG */
	return respcode;
}


static void
list_motd(
	void)
{
	char *ptr;
	char *p;
	char buf[NNTP_STRLEN];
	int i;
	size_t len;
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
				if (debug & DEBUG_NNTP)
					debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
				/*
				 * RFC 6048 2.5.2 "The information MUST be in UTF-8"
				 *
				 * TODO: - store a hash value of the entire motd in the server-rc
				 *         and only if it differs from the old value display the
				 *         motd?
				 *       - use some sort of pager?
				 *       - -> lang.c
				 */
				p = my_strdup(ptr);
				len = strlen(p);
				process_charsets(&p, &len, "UTF-8", tinrc.mm_local_charset, FALSE);
				my_printf("%s%s\n", _("MOTD: "), p);
				free(p);
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
