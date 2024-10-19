/*
 *  Project   : tin - a Usenet reader
 *  Module    : nntplib.c
 *  Author    : S. Barber & I. Lea
 *  Created   : 1991-01-12
 *  Updated   : 2024-10-19
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

#ifdef USE_ZLIB
#	include <zlib.h>
#endif /* USE_ZLIB */

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
	/* Copy of last NNTP command sent, so we can retry it if needed */
	static char last_put[NNTP_STRLEN];
	static constext *xover_cmds = "XOVER";
	static constext *xhdr_cmds = "XHDR";
	/* Set so we don't reconnect just to QUIT */
	static t_bool quitting = FALSE;
#	ifdef USE_ZLIB
	static t_bool deflate_active = FALSE;
#	endif /* USE_ZLIB */
#endif /* NNTP_ABLE */

/*
 * local prototypes
 */
#ifdef NNTP_ABLE
	static int mode_reader(t_bool *sec);
	static int reconnect(int retry);
	static int server_init(char *machine, const char *cservice, unsigned short port, char *text, size_t mlen);
	static void close_server(t_bool send_no_quit);
	static long int list_motd(FILE *stream);
#	ifdef INET6
		static int get_tcp6_socket(char *machine, unsigned short port);
#	else
		static int get_tcp_socket(char *machine, char *service, unsigned short port);
#	endif /* INET6 */
#	ifdef DECNET
		static int get_dnet_socket(char *machine, char *service);
#	endif /* DECNET */
	static ssize_t nntp_read(int fd, void *tls, void *buf, size_t n);

	struct simplebuf {
		unsigned char buf[4096];
		unsigned lb; /* lower bound */
		unsigned ub; /* upper bound */
	};

	struct nntpbuf {
		struct simplebuf rd;
		struct simplebuf wr;
		int fd;
#	ifdef HAVE_GETPEERNAME
		sa_family_t family;
#	else
		int family;
#	endif /* HAVE_GETPEERNAME */
#	ifdef USE_ZLIB
		z_streamp z_wr;
		z_streamp z_rd;
		unsigned char* z_wr_buf;
		unsigned char* z_rd_buf;
#	endif /* USE_ZLIB */
		void *tls_ctx;
};

#	ifdef USE_ZLIB
/* because compression can make the buffer increase, choose a larger size than
 * for the uncompressed data */
#		define DEFLATE_BUFSZ (5000U)
#		define NNTPBUF_INITIALIZER { { {0}, 0, 0 }, { {0}, 0, 0 }, -1, -1, NULL, NULL, NULL, NULL, NULL }
#	else /* USE_ZLIB */
#		define NNTPBUF_INITIALIZER { { {0}, 0, 0 }, { {0}, 0, 0 }, -1, -1, NULL }
#	endif /* USE_ZLIB */

	static struct nntpbuf nntp_buf = NNTPBUF_INITIALIZER;

#	ifdef USE_ZLIB
		static void enable_deflate(struct nntpbuf *nntpbuf);
#	endif /* USE_ZLIB */
	static int nntpbuf_refill(struct nntpbuf *buf);
	static int nntpbuf_flush(struct nntpbuf* buf);
	static int nntpbuf_puts(const char* data, struct nntpbuf* buf);
	static int nntpbuf_getc(struct nntpbuf *buf);
	static int nntpbuf_ungetc(int c, struct nntpbuf *buf);
	static char *nntpbuf_gets(char *s, int size, struct nntpbuf *buf);
	static void nntpbuf_close(struct nntpbuf *buf);
	static int nntpbuf_is_open(struct nntpbuf *buf);
#endif /* NNTP_ABLE */


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
 *	Side effects: sets/updates $NNTPSERVER in the env.
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
	char *new_env;
	static char *serv_env = NULL;
	int n;
	size_t len;
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
	if (cmdline.nntpserver) {
		char *p, *q;
		int i;

		get_nntpserver(buf, sizeof(buf), cmdline.nntpserver);
		/*
		 * - given port in NEWSRCTABLE_FILE overrides -p, -g, -[kT] and $NNTPPORT
		 *
		 * news.example.com[:123]    ~/.tin/${NNTPSERVER-localhost}/.newsrc  ex
		 * [::1]:1119	~/.tin/NEWSRCS/${NNTPSERVER-localhost}	lh6
		 */

		if (buf[0] == '[') { /* IPv6 address? */
			if ((p = strrchr(buf, ':')) != NULL) {
				if ((q = strrchr(buf, ']')) != NULL) {
					for (i = 1; buf[i] != ']'; i++)
						buf[i - 1] = buf[i];
					buf[i - 1] = '\0';
					if (p > q) { /* looks like [IP:v6]:port */
						i = s2i(++p, 0, 65535);
						if (i && !errno)
							nntp_tcp_port = (unsigned short) i;
#	ifdef DEBUG
						else {
							if (debug & DEBUG_MISC)
								wait_message(3, _(txt_port_not_numeric_in), local_newsrctable_file, buf, p);
						}
#	endif /* DEBUG */
					}
				} /* else { not an IPv6 address } */
			} /* else { not an IPv6 address } */
		} else {
			if ((cp = strchr(buf, ':')) != NULL) { /* >= 1 x ':' in servername? */
				if (strrchr(buf, ':') == cp) { /* == 1 x ':' in servername? otherwise (IPv6, syntaxerror) skip */
					*cp++ = '\0'; /* cut off port from the name */
					i = s2i(cp, 1, 65535);
					if (i && !errno)
						nntp_tcp_port = (unsigned short) i;
#	ifdef DEBUG
					else {
						if (debug & DEBUG_MISC)
							wait_message(3, _(txt_port_not_numeric_in), local_newsrctable_file, buf, cp);
					}
#	endif /* DEBUG */
				}
			}
		}
#	ifdef HAVE_SETENV
		if (setenv("NNTPSERVER", buf, 1)) {
#		ifdef DEBUG
			perror_message("setenv(\"NNTPSERVER\", \"%s\", 1)", buf);
#		endif /* DEBUG */
		}
#	else
#		ifdef HAVE_PUTENV
		if ((n = snprintf(NULL, 0, "NNTPSERVER=%s", buf)) >= 0) {
			len = (size_t) n + 1;
			new_env = my_malloc(len);
			if (snprintf(new_env, len, "NNTPSERVER=%s", buf) == n) {
				if (putenv(new_env) != 0) {
#			ifdef DEBUG
					perror_message("putenv(\"%s\")", new_env);
#			endif /* DEBUG */
					free(new_env);
				} else {
					FreeIfNeeded(serv_env);
					serv_env = new_env; /* we are 'leaking' the last malloced mem at exit here */
				}
			} else
				free(new_env);
		}
#		endif /* HAVE_PUTENV */
#	endif /* HAVE_SETENV */
		return buf;
	}

	if ((cp = getenv("NNTPSERVER")) != NULL && *cp) {
		get_nntpserver(buf, sizeof(buf), cp);
		if ((cp = strchr(buf, ':')) != NULL) {
			if (strrchr(buf, ':') == cp) { /* "count" ':'s to be sure it's not an IPv6 address */
				int i;

				*cp++ = '\0'; /* cut off port from the name */
				i = s2i(cp, 1, 65535);
				if (i && !errno)
					nntp_tcp_port = (unsigned short) i;
#	ifdef DEBUG
				else {
					if (debug & DEBUG_MISC)
						wait_message(3, _(txt_port_not_numeric_in), "$NNTPSERVER", buf, cp);
				}
#	endif /* DEBUG */
			}
		}
		return buf;
	}

	if (file == NULL)
		return NULL;

	if ((fp = tin_fopen(file, "r")) != NULL) {
		while (fgets(buf, (int) sizeof(buf), fp) != NULL) {
			if (*buf == '\n' || *buf == '#')
				continue;

			if ((cp = strrchr(buf, '\n')) != NULL)
				*cp = '\0';

			/*
			 * TODO:
			 *       - we do not export $NNTPSERVER here
			 *       - we do not care about any port here
			 *       should we?
			 */

			(void) fclose(fp);
			str_trim(buf);
			str_lwr(buf);
			return buf;
		}
		(void) fclose(fp);
	}

#	ifdef NNTP_DEFAULT_SERVER
	if (*(NNTP_DEFAULT_SERVER))
		return strcpy(buf, NNTP_DEFAULT_SERVER);
#	endif /* NNTP_DEFAULT_SERVER */
#else
	/* silence compiler warning (unused parameter) */
	(void) file;
#endif /* NNTP_ABLE */
	return NULL;	/* No entry */
}


/*
 * server_init  Get a connection to the remote server.
 *
 *	Parameters:	"machine" is the machine to connect to.
 *			"service" is the service to connect to on the machine.
 *			"port" is the service port to connect to.
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
	int sock_fd;
#	ifndef INET6
	char temp[256];
	char *service = strncpy(temp, cservice, sizeof(temp) - 1); /* ...calls non-const funcs; temp will be terminated few lines below */
#	endif /* !INET6 */
#	ifdef DECNET
	char *cp;

	cp = strchr(machine, ':');

	if (cp && cp[1] == ':') {
		*cp = '\0';
		sock_fd = get_dnet_socket(machine, service);
	} else
		sock_fd = get_tcp_socket(machine, service, port);
#	else
#		ifdef INET6
	sock_fd = get_tcp6_socket(machine, port);
#		else
	temp[sizeof(temp) - 1] = '\0'; /* ensure service pints to a terminated string */
	sock_fd = get_tcp_socket(machine, service, port);
#		endif /* INET6 */
#	endif /* DECNET */

#	ifdef INET6
	/* silence compiler warning (unused parameter) */
	(void) cservice;
#	endif /* INET6 */

	if (sock_fd < 0)
		return sock_fd;

#	ifdef TLI /* Transport Level Interface */
	if (t_sync(sock_fd) < 0) {	/* Sync up new fd with TLI */
		t_error("server_init: t_sync()");
		close(sock_fd);
		return -EPROTO;
	}
#	endif /* TLI */

#	ifdef NNTPS_ABLE
	if (use_nntps) {
		int result = tintls_open(machine, sock_fd, &nntp_buf.tls_ctx);

		if (result < 0) {
			close(sock_fd);
			return result;
		}

		result = tintls_handshake(nntp_buf.tls_ctx);
		if (result < 0) {
			close(sock_fd);
			return result;
		}
	}
#	endif /* NNTPS_ABLE */

	nntp_buf.fd = sock_fd;

#	if defined(HAVE_GETPEERNAME) || defined(TLI)
	{
		struct sockaddr sa;
		socklen_t sa_len = sizeof(sa);

#	ifdef HAVE_GETPEERNAME
		if (getpeername(nntp_buf.fd, &sa, &sa_len) == 0)
#	else
#		ifdef TLI
		if (t_getpeername(nntp_buf.fd, &sa, &sa_len) == 0)
		/* if (getpeerinaddr(nntp_buf.fd, (struct sockaddr_in *)&sa, &sa_len) == 0) */
#		endif TLI
#	endif /* HAVE_GETPEERNAME */
			nntp_buf.family = sa.sa_family;
	}
#endif /* HAVE_GETPEERNAME || TLI */

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
	struct hostent *hp;
#			ifdef h_addr
	int x = 0;
	char **cp;
#			endif /* h_addr */
#			ifdef HAVE_HOSTENT_H_ADDR_LIST
	static char *alist[2] = { 0, 0 };
#			endif /* HAVE_HOSTENT_H_ADDR_LIST */
	static struct hostent def;
	static struct in_addr defaddr;
	static char namebuf[256];

#			ifdef HAVE_GETSERVBYNAME
	if (getservbyname(service, "tcp") == NULL) {
		my_fprintf(stderr, _(txt_error_unknown_service), service);
		return -EHOSTUNREACH;
	}
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
#			ifdef HAVE_HOSTENT_H_ADDR_LIST
		def.h_addr_list = alist;
		def.h_addr_list[0] = (char *) &defaddr;
#			else
		def.h_addr = (char *) &defaddr;
#			endif /* HAVE_HOSTENT_H_ADDR_LIST */
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
#		else
	memset((char *) &sock_in, '\0', sizeof(sock_in));
	sock_in.sin_family = AF_INET;
#		endif /* !EXCELAN */

	/*
	 * The following is kind of gross. The name server under 4.3
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
		(void) close(s);
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
		(void) close(s);
		return -save_errno;
	}

#			else
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -errno;
	}

	/* And then connect */
#				ifdef HAVE_HOSTENT_H_ADDR_LIST
	memcpy((char *) &sock_in.sin_addr, hp->h_addr_list[0], hp->h_length);
#				else
	memcpy((char *) &sock_in.sin_addr, hp->h_addr, hp->h_length);
#				endif /* HAVE_HOSTENT_H_ADDR_LIST */
	if (connect(s, (struct sockaddr *) &sock_in, sizeof(sock_in)) < 0) {
		save_errno = errno;
		perror("connect");
		(void) close(s);
		return -save_errno;
	}

#			endif /* EXCELAN */
#		endif /* h_addr */
#	endif /* TLI */
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
	int s = -1, err, ec = 0, es = 0;
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
#	ifndef AF_INET6 /* i.e. sco3.2v5.0.7 */
#		define AF_INET6 AF_INET
#	endif /* !AF_INET6 */
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
		if ((s = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
			es = errno;
			continue;
		}
		if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
			ec = errno;
			close(s);
		} else {
			es = ec = err = 0;
			break;
		}
	}
	if (res0 != NULL)
		freeaddrinfo(res0);
	if (err < 0) {
		my_fprintf(stderr, "%s", _(txt_error_socket_or_connect_problem));
		if (es)
			my_fprintf(stderr, "\tsocket(2): %s\n", strerror(es));
		if (ec)
			my_fprintf(stderr, "\tconnect(2): %s\n", strerror(ec));
		sleep(3);
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
	nntpbuf_puts(string, &nntp_buf);
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
	const char *string,
	t_bool hide_from_log)
{
	if (*string && strlen(string)) {
		DEBUG_IO((stderr, "put_server(%s)\n", string));
		nntpbuf_puts(string, &nntp_buf);
		nntpbuf_puts("\r\n", &nntp_buf);

#	ifdef DEBUG
		if (debug & DEBUG_NNTP) {
			if (verbose)	/* only log password when running verbose */
				debug_print_file("NNTP", ">>>%s%s", logtime(), string);
			else
				debug_print_file("NNTP", ">>>%s%s", logtime(), hide_from_log ? "[data hidden, rerun with -v]" : string);
		}
#	endif /* DEBUG */

		/*
		 * remember the last command we wrote to be able to resend it after
		 * a reconnect. reconnection is handled by get_server()
		 *
		 * don't cache "LIST [ACTIVE|COUNTS|NEWSGROUPS] something" as we
		 * would need to resend all of them but we remember just the last
		 * one. we cache "LIST cmd." instead, this will slow down things, but
		 * that's ok on reconnect.
		 */
		if (!hide_from_log && strcmp(last_put, string))
			STRCPY(last_put, string);
		if (!strncmp(string, "LIST ACTIVE ", 12))
			last_put[11] = '\0'; /* "LIST ACTIVE" */
		else if (!strncmp(string, "LIST COUNTS ", 12))
			last_put[11] = '\0'; /* "LIST COUNTS" */
		else if (!strncmp(string, "LIST NEWSGROUPS ", 16))
			last_put[15] = '\0'; /* "LIST NEWSGROUPS" */
	}
	(void) nntpbuf_flush(&nntp_buf);
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
	nntpbuf_close(&nntp_buf);

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
			/* TODO: also/only postpone_article(article_name) ? */
			unlink(backup_article_name);
			rename_file(article_name, dead_article);
			if (tinrc.keep_dead_articles)
				if ((errno = append_file(dead_article, dead_articles)) != 0)
					perror_message(_(txt_enter_append), dead_article, dead_articles);
		}
		if (retry > NNTP_TRY_RECONNECT) {
#	ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", _(txt_reconnect_limit_reached), retry, NNTP_TRY_RECONNECT);
#	endif /* DEBUG */
		}
		tin_done(NNTP_ERROR_EXIT, _(txt_connection_error));		/* user said no to reconnect or no more retries */
	}

	/* reset signal_context */
	signal_context = save_signal_context;
#	if defined(HAVE_ALARM) && defined(SIGALRM)
		alarm((unsigned) TIN_NNTP_TIMEOUT);
#	endif /* HAVE_ALARM && SIGALRM */

	clear_message();
	strcpy(buf, last_put);			/* Keep copy here, it will be clobbered a lot otherwise */

	if (!nntp_open()) {
		/* Re-establish our current group and resend last command */
		if (curr_group != NULL) {
			DEBUG_IO((stderr, _("Rejoin current group\n")));
			snprintf(last_put, sizeof(last_put), "GROUP %s", curr_group->name);
			put_server(last_put, FALSE);
			if (nntpbuf_gets(last_put, NNTP_STRLEN, &nntp_buf) == NULL)
				*last_put = '\0';
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "<<<%s%s", logtime(), last_put);
#	endif /* DEBUG */
			DEBUG_IO((stderr, _("Read (%s)\n"), last_put));
		}
		DEBUG_IO((stderr, _("Resend last command (%s)\n"), buf));
		put_server(buf, FALSE);
		did_reconnect = TRUE;
		retry = NNTP_TRY_RECONNECT;
	}

#	if defined(HAVE_ALARM) && defined(SIGALRM)
		alarm(0);
#	endif /* HAVE_ALARM && SIGALRM */

	return retry;
}


int
fgetc_server(
	FILE *stream)
{
	int c;

	if (stream != FAKE_NNTP_FP) {
		DEBUG_IO((stderr, "fgetc_server: BAD fp\n"));
		return EOF;
	}

	c = nntpbuf_getc(&nntp_buf);
	DEBUG_IO((stderr, "fgetc_server: %c / %d\n", c, c));

	return c;
}


int
ungetc_server(
	int c,
	FILE *stream)
{
	if (stream != FAKE_NNTP_FP) {
		DEBUG_IO((stderr, "fgetc_server: BAD fp\n"));
		return EOF;
	}

	DEBUG_IO((stderr, "ungetc_server: %c / %d\n", c, c));

	return nntpbuf_ungetc(c, &nntp_buf);
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
	alarm((unsigned) TIN_NNTP_TIMEOUT);
#	endif /* HAVE_ALARM && SIGALRM */
	while (!nntpbuf_is_open(&nntp_buf) || nntpbuf_gets(string, size, &nntp_buf) == NULL) {
		if (errno == EINTR) {
			errno = 0;
#	if defined(HAVE_ALARM) && defined(SIGALRM)
			alarm((unsigned) TIN_NNTP_TIMEOUT);		/* Restart the timer */
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
			 * Typhoon v2.1.1.363 closes the connection right after an unknown
			 * command, (i.e. CAPABILITIES) so we avoid to reissue it on a
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
			strncpy(string, _(txt_nntp_ok_goodbye), (size_t) (size - 3));
			string[size - 3] = '\0';
			strcat(string, "\r\n");		/* tin_fgets() needs CRLF */
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
	t_bool send_no_quit)
{
	if (!send_no_quit && nntpbuf_is_open(&nntp_buf)) {
		if ((!batch_mode || verbose) && cCOLS > 1) {
			char *msg;

			msg = strunc(_(txt_disconnecting), (size_t) (cCOLS - 1));
			my_fputs(msg, stdout);
			my_fputc('\n', stdout);
			free(msg);
		}
		nntp_command("QUIT", OK_GOODBYE, NULL, 0);
		quitting = TRUE;										/* Don't reconnect just for this */
	}
	nntpbuf_close(&nntp_buf);
}


#define WS	" \t"
/*
 * Try and use CAPABILITIES here. Get this list before issuing other NNTP
 * commands because the correct methods may be mentioned in the list of
 * extensions.
 *
 * Sets up: t_capabilities nntp_caps
 */
int
check_extensions(
	int rvl)
{
	char *d;
	char *ptr;
	char buf[NNTP_STRLEN];
	int i;
	int ret = 0;
	static unsigned int cap_vers[] = { 2, /* 3,*/ 0 }; /* array of all capability versions we do support */
	static const char *tin_mechs[] = { /* SASL mechanisms our code can handle */
		"PLAIN",			/* RFC 4616 */
		"ANONYMOUS",		/* RFC 4505 */
		"LOGIN",			/* just for testing, remove b4 release */
#if 0
		"EXTERNAL",			/* RFC 4422 */
		"OTP",				/* RFC 2444 */
		"SECURID",			/* RFC 2808 */

		"SCRAM-SHA-224",	/* RFC 5802, RFC 7677 */
		"SCRAM-SHA-256",	/* RFC 5802, RFC 7677 */

		"GSSAPI",			/* RFC 4643 7.3, RFC 4752 */
		"GS2",				/* RFC 5801 */
		"SAML20",			/* RFC 6595 */
		"OPENID",			/* RFC 6616 */
		/* ... */
#endif /* 0 */
		/*
		 * we exclude weak/obsolete
		 * CRAM_MD5, DIGEST_MD5 (via RFC 6151, RFC 6331)
		 * NTLM, LOGIN
		 * SRP, PSSDSS
		 */
		NULL
	};

	buf[0] = '\0';

	/* rvl > 0 = manually send "CAPABILITIES" to avoid endless AUTH loop */
	i = rvl ? rvl : new_nntp_command("CAPABILITIES", INF_CAPABILITIES, buf, sizeof(buf));
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
			FreeAndNull(nntp_caps.headers_range);
			FreeAndNull(nntp_caps.headers_id);
			nntp_caps.list_newsgroups = FALSE;
			nntp_caps.list_overview_fmt = FALSE;
			nntp_caps.list_motd = FALSE;
			nntp_caps.list_subscriptions = FALSE;
			nntp_caps.list_distributions = FALSE;
			nntp_caps.list_moderators = FALSE;
			nntp_caps.list_counts = FALSE;
			nntp_caps.xpat = TRUE; /* toggles to false if fails, INN > 2.7.0 announces it */
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
			FreeAndNull(nntp_caps.sasl_mechs);
			/* nntp_caps.sasl_mech_used will be init in sasl_auth() */
			nntp_caps.compress = FALSE;
			nntp_caps.compress_algorithm = COMPRESS_NONE;
			/* nntp_caps.maxartnum will be init it nntp_open() */
#	if 0
			nntp_caps.streaming = FALSE;
			nntp_caps.ihave = FALSE;
#	endif /* 0 */
#	ifndef BROKEN_LISTGROUP
			nntp_caps.broken_listgroup = FALSE;
#	else
			nntp_caps.broken_listgroup = TRUE;
#	endif /* !BROKEN_LISTGROUP */

			while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#	ifdef DEBUG
				if (debug & DEBUG_NNTP)
					debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
				/* look for version number(s) */
				if (!nntp_caps.version && nntp_caps.type == CAPABILITIES) {
					if (!strncasecmp(ptr, "VERSION", 7)) {
						if (strtok(ptr, WS) != NULL) { /* skip initial VERSION */
							unsigned int v, j;

							while ((d = strtok(NULL, WS)) != NULL) { /* find highest version we do support */
								v = (unsigned int) s2i(d, 2, INT_MAX);
								for (j = 0; cap_vers[j]; j++) {
									if (v == cap_vers[j])
										nntp_caps.version = MAX(nntp_caps.version, v);
								}
							}
						}
					}
				}
				/* CAPABILITIES VERSION 2 (we currently only support that) */
				if (nntp_caps.version == 2) {
					/* check for LIST variants */
					if (!strncasecmp(ptr, "LIST", 4)) {
						if (strtok(ptr, WS) != NULL) { /* skip initial LIST */
							while ((d = strtok(NULL, WS)) != NULL) {
								if (!strcasecmp(d, "ACTIVE.TIMES"))
									nntp_caps.list_active_times = TRUE;
								else if (!strcasecmp(d, "ACTIVE"))
									nntp_caps.list_active = TRUE;
								else if (!strcasecmp(d, "DISTRIB.PATS"))
									nntp_caps.list_distrib_pats = TRUE;
								else if (!strcasecmp(d, "DISTRIBUTIONS")) /* RFC 6048 */
									nntp_caps.list_distributions = TRUE;
								else if (!strcasecmp(d, "HEADERS"))
									nntp_caps.list_headers = TRUE; /* HDR requires LIST HEADERS, but not vice versa */
								else if (!strcasecmp(d, "NEWSGROUPS"))
									nntp_caps.list_newsgroups = TRUE;
								else if (!strcasecmp(d, "OVERVIEW.FMT")) /* OVER requires OVERVIEW.FMT, but not vice versa */
									nntp_caps.list_overview_fmt = TRUE;
								else if (!strcasecmp(d, "MOTD")) /* RFC 6048 */
									nntp_caps.list_motd = TRUE;
								else if (!strcasecmp(d, "SUBSCRIPTIONS")) /* RFC 6048 */
									nntp_caps.list_subscriptions = TRUE;
								else if (!strcasecmp(d, "MODERATORS")) /* RFC 6048 */
									nntp_caps.list_moderators = TRUE;
								else if (!strcasecmp(d, "COUNTS")) /* RFC 6048 */
									nntp_caps.list_counts = TRUE;
							}
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
						if (strtok(ptr, WS) != NULL) {
							while ((d = strtok(NULL, WS)) != NULL) {
								if (!strcasecmp(d, "MSGID"))
									nntp_caps.over_msgid = TRUE;
							}
						}
					}
					/*
					 * NOTE: if we saw HDR, LIST HEADERS _must_ be implemented
					 */
					else if (!strncasecmp(ptr, &xhdr_cmds[1], strlen(&xhdr_cmds[1]))) {
						nntp_caps.hdr_cmd = &xhdr_cmds[1];
						nntp_caps.list_headers = nntp_caps.hdr = TRUE;
						nntp_caps.headers_range = my_strdup("");
						nntp_caps.headers_id = my_strdup("");
					} else if (!strncasecmp(ptr, "AUTHINFO", 8)) {
						if (strtok(ptr, WS) == NULL) /* AUTHINFO without args */
							nntp_caps.authinfo_state = TRUE;
						else {
							while ((d = strtok(NULL, WS)) != NULL) {
								if (!strcasecmp(d, "USER"))
									nntp_caps.authinfo_user = TRUE;
								if (!strcasecmp(d, "SASL"))
									nntp_caps.authinfo_sasl = TRUE;
							}
						}
					} else if (!strncasecmp(ptr, "SASL", 4)) {
						nntp_caps.authinfo_sasl = FALSE;
						FreeAndNull(nntp_caps.sasl_mechs);
						nntp_caps.sasl_mechs = my_malloc(strlen(ptr) + 1); /* more than enough */
						nntp_caps.sasl_mechs[0] = '\0';

						if (strtok(ptr, WS) != NULL) { /* skip initial "SASL" */
							int m;

							while ((d = strtok(NULL, WS)) != NULL) {
								m = 0;
								while (tin_mechs[m]) { /* remember servers mechs we like */
									if (!strcasecmp(d, tin_mechs[m])) {
										strcat(nntp_caps.sasl_mechs, d);
										strcat(nntp_caps.sasl_mechs, " ");
										break;
									}
									m++;
								}
							}
							str_trim(nntp_caps.sasl_mechs);
							if (*nntp_caps.sasl_mechs)
								nntp_caps.authinfo_sasl = TRUE;
						}
					} else if (!strncasecmp(ptr, "COMPRESS", 8)) { /* RFC 8054 */
						if (strtok(ptr, WS) != NULL) {
							while ((d = strtok(NULL, WS)) != NULL) {
								if (!strcasecmp(d, "DEFLATE")) {
									nntp_caps.compress = TRUE;
									nntp_caps.compress_algorithm |= COMPRESS_DEFLATE;
								}
							}
						}
					}
#	if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
					/*
					 * MAXARTNUM - <tnqm14$35bas$1@news.trigofacile.com>
					 *
					 * if server responds with MAXARTNUM we (re)parse it
					 * as we may have changed the state (auth) and it's
					 * the servers job to not advertised MAXARTNUM again
					 * after it had been used ...
					 */
					else if (!strncasecmp(ptr, "MAXARTNUM", 9) && nntp_caps.maxartnum == T_ARTNUM_CONST(0)) {
						if (strtok(ptr, WS) != NULL) {
							while ((d = strtok(NULL, WS)) != NULL)
								nntp_caps.maxartnum = MIN(atoartnum(d), ARTNUM_MAX);
						}
					}
#	endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */
#	if 0 /* we don't need these */
					else if (!strcasecmp(ptr, "IHAVE"))
						nntp_caps.ihave = TRUE;
					else if (!strcasecmp(ptr, "STREAMING"))
						nntp_caps.streaming = TRUE;
#	endif /* 0 */
				/* XZVER, XZHDR, ... */
				} else
					nntp_caps.type = NONE;
			}
			break;

		/*
		 * XanaNewz 2 Server Version 2.0.0.3 doesn't know CAPABILITIES
		 * but responds with 400 _without_ closing the connection. If
		 * you use tin on a XanaNewz 2 Server comment out the following
		 * case.
		 */
#	if 1
		case ERR_GOODBYE:
			ret = i;
			error_message(2, "%s", buf);
			break;
#	endif /* 1 */

		default:
			break;
	}

#	if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
	if (nntp_caps.maxartnum <= T_ARTNUM_CONST(2147483647)) /* RFC 3977 "Article numbers MUST lie between 1 and 2,147,483,647, inclusive." */
		nntp_caps.maxartnum = T_ARTNUM_CONST(0);
#	endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */

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
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "mode_reader(MODE READER)");
#	endif /* DEBUG */
		DEBUG_IO((stderr, "nntp_command(MODE READER)\n"));
		put_server("MODE READER", FALSE);

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
	char line[NNTP_STRLEN] = { '\0' };
	int ret;
	t_bool sec = FALSE;
	/* It appears that is_reconnect guards code that should be run only once */
	static t_bool is_reconnect = FALSE;

	if (!read_news_via_nntp)
		return 0;

#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "nntp_open() BEGIN");
#	endif /* DEBUG */

	if (nntp_server == NULL) {
		error_message(2, _(txt_cannot_get_nntp_server_name));
		error_message(2, _(txt_server_name_in_file_env_var), NNTP_SERVER_FILE);
		return -EHOSTUNREACH;
	}

	if (!batch_mode || verbose)
		wait_message(0, _(txt_connecting_port), nntp_server, nntp_tcp_port);

	if ((!batch_mode || verbose) && use_nntps)
		my_fputc('\n', stdout);

#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "nntp_open() %s:%u", nntp_server, nntp_tcp_port);
#	endif /* DEBUG */

	ret = server_init(nntp_server, NNTP_TCP_NAME, nntp_tcp_port, line, sizeof(line));
	DEBUG_IO((stderr, "server_init returns %d,%s\n", ret, line));

	if ((!batch_mode || verbose) && ret >= 0 && !use_nntps)
		my_fputc('\n', stdout);

#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
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
		while (isspace((unsigned char) *linep))
			linep++;

		STRCPY(bug_nntpserver1, linep);
	}

#	if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
	nntp_caps.maxartnum = T_ARTNUM_CONST(0);
#	endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */

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
		check_extensions(0);

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
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "nntp_open(MODE READER)");
#	endif /* DEBUG */
			put_server("MODE READER", FALSE);
			switch (get_only_respcode(buf, sizeof(buf))) {
				/* just honor critical errors */
				case ERR_GOODBYE:
				case ERR_ACCESS:
					error_message(2, "%s", buf);
					return -1;

				default:
					break;
			}
			check_extensions(0);
		}
	}

	if (force_auth_on_conn_open) {
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "nntp_open(authenticate(force_auth_on_conn_open))");
#	endif /* DEBUG */

		if (!authenticate(nntp_server, userid, FALSE))	/* 3rd parameter is FALSE as we need to get prompted for username password here */
			return -1;
	}

	if ((nntp_caps.type == CAPABILITIES && nntp_caps.mode_reader) || nntp_caps.type != CAPABILITIES) {
		if ((ret = mode_reader(&sec))) {
			if (nntp_caps.type == CAPABILITIES)
				can_post = nntp_caps.post && !force_no_post;

			return ret;
		}
		if (nntp_caps.type == CAPABILITIES)
			check_extensions(0);
	}

	if (nntp_caps.type == CAPABILITIES) {
		if (!nntp_caps.reader) {
#	ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", _(txt_capabilities_without_reader));
#	endif /* DEBUG */
			error_message(2, _(txt_capabilities_without_reader));
			return -1; /* give up */
		}
		can_post = nntp_caps.post && !force_no_post;
	}

	if (!is_reconnect && *line) {
#	if 0
	/*
	 * gives wrong results if RFC 3977 server requests auth after
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
		while (isspace((unsigned char) *linep))
			linep++;

		STRCPY(bug_nntpserver2, linep);

		/*
		 * Show user last server response line, do some nice formatting if
		 * response is longer than the screen width.
		 *
		 * TODO: This only breaks the line once, but the response could be
		 * longer than two lines ...
		 */
		if (!batch_mode || verbose) {
			char *chr2, *chr1 = my_strdup((sec ? bug_nntpserver2 : bug_nntpserver1));
			int j = s2i(get_val("COLUMNS", "80"), MIN_COLUMNS_ON_TERMINAL, INT_MAX);

			if (j > MIN_COLUMNS_ON_TERMINAL && ((int) strlen(chr1)) >= j) {
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
	 *
	 * TODO: this should be done when the command is first used!
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
					if ((debug & DEBUG_NNTP) && verbose > 1)
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
#	ifdef DEBUG
					if ((debug & DEBUG_NNTP) && verbose > 1)
						debug_print_file("NNTP", "nntp_open() %s skipping data", &xhdr_cmds[i]);
#	endif /* DEBUG */
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
		/* no XPAT probing here, we do when it's needed */
		nntp_caps.xpat = TRUE;
#	if 0
		switch (new_nntp_command("XPAT Newsgroups <0> *", ERR_NOART, line, sizeof(line))) {
			case ERR_NOART:
				nntp_caps.xpat = TRUE;
				break;

			default:
				break;
		}
#	endif /* 0 */
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
					if ((debug & DEBUG_NNTP) && verbose > 1)
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
#	ifdef DEBUG
					if ((debug & DEBUG_NNTP) && verbose > 1)
						debug_print_file("NNTP", "nntp_open() %s skipping data", xhdr_cmds);
#	endif /* DEBUG */
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

#	if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
	set_maxartnum(is_reconnect);
#	endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */

	/* no no_write logic here as that's always set on initial connect */
	if (((nntp_caps.type == CAPABILITIES && nntp_caps.list_motd) || nntp_caps.type != CAPABILITIES) && !is_reconnect && !batch_mode && show_description && check_for_new_newsgroups) {
		FILE *fp;

		if ((fp = tin_fopen(local_motd_file, "w+")) != NULL) {
			char *motd;
			unsigned int n;
			long m_hash;

			m_hash = list_motd(fp);
			if (m_hash != motd_hash) {
				if (fseek(fp, 0L, SEEK_SET) != -1) {
					n = 0;
#	ifdef HAVE_COLOR
					fcol(tinrc.col_message);
#	endif /* HAVE_COLOR */
					while ((motd = tin_fgets(fp, FALSE)) != NULL) {
						my_printf("%s\n", motd);
						n++;
					}
					my_fflush(stdout);
#	ifdef HAVE_COLOR
					fcol(tinrc.col_normal);
#	endif /* HAVE_COLOR */
					if (n)
						sleep((n >> 1) | 0x01);
					else
						unlink(local_motd_file);

					motd_hash = m_hash;
				} /* else EBADF */
			}
			fclose(fp);
		}
	}

	is_reconnect = TRUE;

#	ifdef USE_ZLIB
	/*
	 * Enable compression if available
	 * Note: after enabling compression, authentication shall not work anymore
	 */
	if (nntp_caps.compress && use_compress) {
		if ((nntp_caps.compress_algorithm & COMPRESS_DEFLATE) == COMPRESS_DEFLATE)
			enable_deflate(&nntp_buf);
	}
#	endif /* USE_ZLIB */
#endif /* NNTP_ABLE */

	DEBUG_IO((stderr, "nntp_open okay\n"));
	return 0;
}


/*
 * 'Public' function to shutdown the NNTP connection
 */
void
nntp_close(
	t_bool send_no_quit)
{
#ifdef NNTP_ABLE
	if (read_news_via_nntp && !read_saved_news) {
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "nntp_close(%s) END", bool_unparse(send_no_quit));
#	endif /* DEBUG */
		close_server(send_no_quit);
	}
#else
	/* silence compiler warning (unused parameter) */
	(void) send_no_quit;
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
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "Error: tin_error<>0 or ptr==NULL in get_only_respcode()");
#	endif /* DEBUG */
		return -1;
	}

#	ifdef DEBUG
	if (debug & DEBUG_NNTP)
		debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
	respcode = (int) strtol(ptr, &end, 10);
	if (end == ptr || respcode < 100 || respcode > 599)
		respcode = -1;
	DEBUG_IO((stderr, "get_only_respcode(%d)\n", respcode));

	/*
	 * we also reconnect on ERR_FAULT if last_put was ARTICLE or LIST or POST
	 * as inn (2.2.3) sends ERR_FAULT on timeout
	 *
	 * what about other LIST cmds? (ACTIVE|COUNTS|OVERVIEW.FMT|...)
	 */
	if (*last_put && ((respcode == ERR_FAULT && (!strncmp(last_put, "ARTICLE", 7) || !strcmp(last_put, "POST") || !strcmp(last_put, "LIST"))) || respcode == ERR_GOODBYE || respcode == OK_GOODBYE) && strcmp(last_put, "QUIT")) {
		if (respcode == ERR_GOODBYE && !strncmp(last_put, "HEAD ", 5)) {
			/* usenetfarm may send ERR_GOODBYE in response to HEAD, we don't want to retry that */
			return respcode;
		}
		/*
		 * Maybe server timed out.
		 * If so, retrying will force a reconnect.
		 */
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "get_only_respcode() timeout");
#	endif /* DEBUG */
		put_server(last_put, FALSE);
		ptr = tin_fgets(FAKE_NNTP_FP, FALSE);

		if (tin_errno || ptr == NULL) {
#	ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "Error: tin_errno<>0 or ptr==NULL in get_only_respcode(retry)");
#	endif /* DEBUG */
			return -1;
		}

#	ifdef DEBUG
		if (debug & DEBUG_NNTP)
			debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
		respcode = (int) strtol(ptr, &end, 10);
		if (end == ptr || respcode < 100 || respcode > 599)
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
 *       as 401 requires the examination of the returned text besides the
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
#	ifdef USE_ZLIB
		if (deflate_active) /* Do not auth if compression is active */
			tin_done(EXIT_FAILURE, _(txt_error_compression_auth), tin_progname); /* TODO: should we exit with NNTP_ERROR_EXIT? */
#	endif /* USE_ZLIB */

		/*
		 * Server requires authentication.
		 */
#	ifdef DEBUG
		if ((debug & DEBUG_NNTP) && verbose > 1)
			debug_print_file("NNTP", "get_respcode() authentication");
#	endif /* DEBUG */
		STRCPY(savebuf, last_put);

		if (!authenticate(nntp_server, userid, FALSE))
			tin_done(EXIT_FAILURE, _(txt_auth_failed), nntp_caps.type == CAPABILITIES ? ERR_AUTHFAIL : ERR_ACCESS); /* TODO: should we exit with NNTP_ERROR_EXIT? */

		if (nntp_caps.type == CAPABILITIES)
			can_post = nntp_caps.post && !force_no_post;
		else {
			put_server("MODE READER", FALSE);
			if (get_only_respcode(message, mlen) == OK_CANPOST)
				can_post = TRUE && !force_no_post;
		}
		if (curr_group != NULL) {
			DEBUG_IO((stderr, _("Rejoin current group\n")));
			snprintf(last_put, sizeof(last_put), "GROUP %s", curr_group->name);
			put_server(last_put, FALSE);
			if (nntpbuf_gets(last_put, NNTP_STRLEN, &nntp_buf) == NULL)
				*last_put = '\0';
#	ifdef DEBUG
			if (debug & DEBUG_NNTP)
				debug_print_file("NNTP", "<<<%s%s", logtime(), *last_put ? last_put : "NULL");
#	endif /* DEBUG */
			DEBUG_IO((stderr, _("Read (%s)\n"), *last_put ? last_put : txt_null));
		}
		STRCPY(last_put, savebuf);

		put_server(last_put, FALSE);
		ptr = tin_fgets(FAKE_NNTP_FP, FALSE);

		if (tin_errno) {
#	ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "Error: tin_errno <> 0");
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
		if (end == ptr || respcode < 100 || respcode > 599)
			return -1;

		if (message != NULL && mlen > 1) {				/* Pass out the rest of the text */
			strncpy(message, end, mlen - 1);
			message[mlen - 1] = '\0';
		}
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
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "nntp_command(%s)", command);
#	endif /* DEBUG */
	put_server(command, FALSE);

	if (!bool_equal(dangerous_signal_exit, TRUE)) {
		if (get_respcode(message, mlen) != success) {
#	ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "nntp_command(%s) NOT_OK", command);
#	endif /* DEBUG */
			/* error_message(2, "%s", message); */
			return (FILE *) 0;
		}
	}
#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
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
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "new_nntp_command(%s)", command);
#	endif /* DEBUG */
	put_server(command, FALSE);

	if (!bool_equal(dangerous_signal_exit, TRUE)) {
		if ((respcode = get_respcode(message, mlen)) != success) {
#	ifdef DEBUG
			if ((debug & DEBUG_NNTP) && verbose > 1)
				debug_print_file("NNTP", "new_nntp_command(%s) NOT_OK - Expected: %d, got: %d", command, success, respcode);
#	endif /* DEBUG */
			return respcode;
		}
	}
#	ifdef DEBUG
	if ((debug & DEBUG_NNTP) && verbose > 1)
		debug_print_file("NNTP", "new_nntp_command(%s) OK", command);
#	endif /* DEBUG */
	return respcode;
}


static long int
list_motd(
	FILE *stream)
{
	char *ptr, *p, *m;
	char buf[NNTP_STRLEN];
	int i;
	size_t len;
	long m_hash = 0L;
#	if defined(CHARSET_CONVERSION) && defined(USE_ICU_UCSDET)
	char *guessed_charset = NULL;
#	endif /* CHARSET_CONVERSION && USE_ICU_UCSDET */

	if (!stream)
		return m_hash;

	m = my_calloc(1, 1);
	buf[0] = '\0';
	i = new_nntp_command("LIST MOTD", OK_MOTD, buf, sizeof(buf));

	switch (i) {
		case OK_MOTD:
			while ((ptr = tin_fgets(FAKE_NNTP_FP, FALSE)) != NULL) {
#	ifdef DEBUG
				if (debug & DEBUG_NNTP)
					debug_print_file("NNTP", "<<<%s%s", logtime(), ptr);
#	endif /* DEBUG */
				p = my_strdup(ptr);
				len = strlen(p);

				/* original MOTD for hashing as local charset may change */
				m = my_realloc(m, strlen(m) + len + 1);
				strcat(m, p);

				/*
				 * RFC 6048 2.5.2 "The information MUST be in UTF-8"
				 * but the cmd. was widely available before that RFC and
				 * even before RFC 3977(which doesn't mention it *sigh*),
				 * so checking nntp_caps.type doesn't help and we guess if
				 * we can. Some day we may check for nntp_caps.version > 2 ...
				 */
#	if defined(CHARSET_CONVERSION) && defined(USE_ICU_UCSDET)
				if ((guessed_charset = guess_charset(p, 10)) != NULL) {
					process_charsets(&p, &len, guessed_charset, tinrc.mm_local_charset, FALSE);
					free(guessed_charset);
				} else
#	endif /* CHARSET_CONVERSION && USE_ICU_UCSDET */
					process_charsets(&p, &len, "UTF-8", tinrc.mm_local_charset, FALSE);

				fprintf(stream, _(txt_motd), p);
				free(p);
			}
			m_hash = (long int) hash_groupname(m);
			break;

		default:	/* common response codes are 500, 501, 503 */
			break;
	}
	free(m);
	return m_hash;
}


static ssize_t
nntp_write(
	int fd,
	void *tls,
	const void *buf,
	size_t n)
{
	ssize_t bytes_written;

#	ifdef NNTPS_ABLE
	if (tls)
		bytes_written = tintls_write(tls, buf, n);
	else
#	endif /* NNTPS_ABLE */
		bytes_written = write(fd, buf, n);

	/* silence compiler warning (unused parameter) */
	(void) tls;

	return bytes_written;
}


static ssize_t
nntp_read(
	int fd,
	void *tls,
	void *buf,
	size_t n)
{
	ssize_t bytes_read;

#	ifdef NNTPS_ABLE
	if (tls)
		bytes_read = tintls_read(tls, buf, n);
	else
#	endif /* NNTPS_ABLE */
		bytes_read = read(fd, buf, n);

	/* silence compiler warning (unused parameter) */
	(void) tls;

	return bytes_read;
}


#define SZ(a) sizeof((a))

#	ifdef USE_ZLIB
static voidpf
deflate_alloc(
	voidpf user,
	uInt items,
	uInt size)
{
	(void) user;
	return my_calloc(items, size);
}


static void
deflate_free(
	voidpf user,
	voidpf ptr)
{
	(void) user;
	FreeIfNeeded(ptr);
}


static z_streamp
z_stream_init(
	t_bool is_deflate)
{
	int result;

	z_streamp strm = my_calloc(1, sizeof(z_stream));
	strm->zalloc = deflate_alloc;
	strm->zfree = deflate_free;

	if (is_deflate)
		result = deflateInit2(strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY);
	else
		result = inflateInit2(strm, -15);

	if (result != Z_OK)
		FreeAndNull(strm);

	return strm;
}


static void
enable_deflate(
	struct nntpbuf* nntpbuf)
{
	char buf[NNTP_STRLEN];
	int result;

	if (nntpbuf->z_rd || nntpbuf->z_wr)
		return;

	nntpbuf->z_rd = z_stream_init(FALSE);
	nntpbuf->z_wr = z_stream_init(TRUE);

	if (nntpbuf->z_rd == NULL || nntpbuf->z_wr == NULL)
		goto error_out;

	nntpbuf->z_rd_buf = my_malloc(DEFLATE_BUFSZ);
	nntpbuf->z_rd->next_in = nntpbuf->z_rd_buf;
	nntpbuf->z_rd->avail_in = 0;

	nntpbuf->z_wr_buf = my_malloc(DEFLATE_BUFSZ);
	nntpbuf->z_wr->next_out = nntpbuf->z_wr_buf;
	nntpbuf->z_wr->avail_out = DEFLATE_BUFSZ;

	buf[0] = '\0';
	result = new_nntp_command("COMPRESS DEFLATE", OK_COMPRESS, buf, sizeof(buf));

	switch (result) {
		case OK_COMPRESS:
			deflate_active = TRUE;
			return;

		case ERR_COMPRESS_ALG:
		case ERR_COMPRESS:
		default: /* unexpected */
			break;
	}

error_out:
	FreeAndNull(nntpbuf->z_rd);
	FreeAndNull(nntpbuf->z_wr);
	FreeAndNull(nntpbuf->z_rd_buf);
	FreeAndNull(nntpbuf->z_wr_buf);
}


static ssize_t
nntpbuf_deflate_write(
	struct nntpbuf* buf)
{
	ssize_t bytes_written = 0, bwritten;
	t_bool deflate_again = TRUE;

	buf->z_wr->next_in = buf->wr.buf + buf->wr.lb;
	buf->z_wr->avail_in = buf->wr.ub - buf->wr.lb;

	while (deflate_again) {
		Bytef *out = buf->z_wr->next_out;

		if (deflate(buf->z_wr, Z_PARTIAL_FLUSH) < 0)
			return EOF;

		if (buf->z_wr->avail_in > 0 || buf->z_wr->avail_out == 0)
			deflate_again = TRUE;
		else
			deflate_again = FALSE;

		while (buf->z_wr->avail_out < DEFLATE_BUFSZ) {
			if ((bwritten = nntp_write(buf->fd, buf->tls_ctx, out, DEFLATE_BUFSZ - buf->z_wr->avail_out)) < 0)
				return EOF;

			buf->z_wr->avail_out += bwritten;
			out += bwritten;
			bytes_written += bwritten;
		}
	}
	buf->wr.lb = buf->wr.ub;

	return bytes_written;
}


static ssize_t
nntpbuf_inflate(
	struct nntpbuf* buf)
{
	int result = inflate(buf->z_rd, Z_NO_FLUSH);

	if (result < 0 && result != Z_BUF_ERROR)
		return EOF;

	/* move leftover input data to beginning of input buffer */
	my_memmove(buf->z_rd_buf, buf->z_rd->next_in, buf->z_rd->avail_in);
	buf->z_rd->next_in = buf->z_rd_buf;

	return (SZ(buf->rd.buf) - buf->rd.ub) - buf->z_rd->avail_out;
}


static ssize_t
nntpbuf_inflate_read(
	struct nntpbuf* buf)
{
	ssize_t bytes_read;
	ssize_t bread;

	buf->z_rd->next_out = buf->rd.buf + buf->rd.ub;
	buf->z_rd->avail_out = SZ(buf->rd.buf) - buf->rd.ub;

	/* call inflate unconditionally to make sure there is no pending output
	   left, before calling the possibly blocking read below */
	if ((bytes_read = nntpbuf_inflate(buf)) < 0)
		return bytes_read;

	while (bytes_read == 0) {
		if (buf->z_rd->avail_in < DEFLATE_BUFSZ) {
			if ((bread = nntp_read(buf->fd, buf->tls_ctx, buf->z_rd->next_in, DEFLATE_BUFSZ - buf->z_rd->avail_in)) <= 0)
				return EOF;

			buf->z_rd->avail_in += bread;
		}

		if ((bread = nntpbuf_inflate(buf)) < 0)
			return bread;

		bytes_read += bread;
	}
	return bytes_read;
}
#	endif /* USE_ZLIB */


/*
 * write data from write buffer to NNTP connection when requested
 */
static int
nntpbuf_flush(
	struct nntpbuf* buf)
{
	if (!buf)
		return EOF;

	while (buf->wr.ub > buf->wr.lb) {
		ssize_t bytes_written;

#	ifdef USE_ZLIB
		if (deflate_active)
			bytes_written = nntpbuf_deflate_write(buf);
		else
#	endif /* USE_ZLIB */
			bytes_written = nntp_write(buf->fd, buf->tls_ctx, buf->wr.buf + buf->wr.lb, buf->wr.ub - buf->wr.lb);

		if (bytes_written < 0)
			return EOF;

		buf->wr.lb += bytes_written;
	}

	buf->wr.ub = buf->wr.lb = 0;

	return 0;
}


/*
 * fputs(3) replacement using the NNTP connection
 */
static int
nntpbuf_puts(
	const char* data,
	struct nntpbuf* buf)
{
	int bytes_written = 0, retval;
	size_t len, l;

	if (!buf || SZ(buf->wr.buf) == 0 || buf->wr.lb > buf->wr.ub)
		return EOF;

	if (!data)
		return 0;

	len = strlen(data);
	while (len) {
		if (buf->wr.ub == SZ(buf->wr.buf)) {
			if ((retval = nntpbuf_flush(buf)) != 0)
				return retval;
		}

		l = len;
		if (l > SZ(buf->wr.buf) - buf->wr.ub)
			l = SZ(buf->wr.buf) - buf->wr.ub;

		memcpy(buf->wr.buf + buf->wr.ub, data, l);
		buf->wr.ub += l;
		data += l;
		bytes_written += l;
		len -= l;
	}

	return bytes_written;
}


/*
 * internal helper to refill the read buffer using the NNTP connection
 */
static int
nntpbuf_refill(
	struct nntpbuf *buf)
{
	unsigned free_b;

	if (buf->rd.ub == buf->rd.lb)
		buf->rd.ub = buf->rd.lb = 0;

	if ((free_b = SZ(buf->rd.buf) - buf->rd.ub)) {
		ssize_t bytes_read;

#	ifdef USE_ZLIB
		if (deflate_active)
			bytes_read = nntpbuf_inflate_read(buf);
		else
#	endif /* USE_ZLIB */
			bytes_read = nntp_read(buf->fd, buf->tls_ctx, buf->rd.buf + buf->rd.ub, free_b);

		if (bytes_read > 0)
			buf->rd.ub += bytes_read;

		return (int) bytes_read;
	}
	return 0;
}


/*
 * fgetc(3) replacement using the NNTP connection
 */
static int
nntpbuf_getc(
	struct nntpbuf *buf)
{
	int c, retval;

	if (buf->rd.ub - buf->rd.lb == 0) {
		if ((retval = nntpbuf_refill(buf)) <= 0)
			return retval;
	}

	c = buf->rd.buf[buf->rd.lb];
	buf->rd.lb++;

	return c;
}


/*
 * ungetc(3) replacement using the NNTP connection
 */
static int
nntpbuf_ungetc(
	int c,
	struct nntpbuf *buf)
{
	if (buf->rd.lb == 0) {
		if (buf->rd.ub == SZ(buf->rd.buf)) {
			errno = ENOSPC;
			return EOF;
		}
		my_memmove(buf->rd.buf + 1, buf->rd.buf, buf->rd.ub);
		buf->rd.lb++;
	}

	buf->rd.lb--;
	buf->rd.buf[buf->rd.lb] = (unsigned char) c;

	return c;
}


/*
 * fgets(3) replacement using the NNTP connection
 */
static char *
nntpbuf_gets(
	char *s,
	int size,
	struct nntpbuf *buf)
{
	int write_at = 0;

	if (s == NULL || size == 0)
		return s;

	s[--size] = '\0';

	while (size) {
		if (buf->rd.ub - buf->rd.lb == 0) {
			if (nntpbuf_refill(buf) <= 0)
				return NULL;
		}

		while (size && (buf->rd.ub - buf->rd.lb) > 0) {
			((unsigned char *) s)[write_at++] = buf->rd.buf[buf->rd.lb++];
			size--;

			if (s[write_at - 1] == '\n' && size) {
				s[write_at] = '\0';
				return s;
			}
		}
	}

	return s;
}


static void
nntpbuf_close(
	struct nntpbuf *buf)
{
	if (!buf)
		return;

#	ifdef NNTPS_ABLE
	if (buf->tls_ctx) {
		int result = tintls_close(buf->tls_ctx);

		if (result != 0) {
			/* TODO: warn? */
		}
	}
	buf->tls_ctx = NULL;
#	endif /* NNTPS_ABLE */

	if (buf->fd >= 0)
		close(buf->fd);

	buf->fd = -1;

	buf->rd.lb = buf->rd.ub = 0;
	buf->wr.lb = buf->wr.ub = 0;

#	ifdef USE_ZLIB
	if (deflate_active) {
		if (buf->z_rd)
			inflateEnd(buf->z_rd);
		FreeAndNull(buf->z_rd);
		FreeAndNull(buf->z_rd_buf);

		if (buf->z_wr)
			deflateEnd(buf->z_wr);
		FreeAndNull(buf->z_wr);
		FreeAndNull(buf->z_wr_buf);

		deflate_active = FALSE;
	}
#	endif /* USE_ZLIB */
}


static int
nntpbuf_is_open(
	struct nntpbuf *buf)
{
	return buf->fd != -1;
}
#undef SZ


/*
 * TODO: - add servers (and clients) "DATE" ((both) in GMT)?
 *         NOTE: - we may still use localtime() in NEWGROUPS (and two
 *                 didgit year) if not using a RFC 3977 server.
 *               - instead of displaying the clients "current" time (in GMT)
 *                 it might be more helpful to display the time of the last
 *                 connection to that server (as used in NEWGROUP)
 *       - mention used IP
 */
int
nntp_conninfo(
	FILE *stream)
{
	int retval = 0;

	fprintf(stream, "%s", _(txt_conninfo_conn_details));
	fprintf(stream, _(txt_conninfo_server), nntp_server);
	fprintf(stream, _(txt_conninfo_port), nntp_tcp_port);

#	if defined(HAVE_GETPEERNAME) || defined(TLI)
	switch (nntp_buf.family) {
		case AF_INET:
			fprintf(stream, _(txt_conninfo_type), "IPv4");
			break;

#		ifdef INET6
		case AF_INET6:
			fprintf(stream, _(txt_conninfo_type), "IPv6");
			break;
#		endif /* INET6 */

#		ifdef DECNET
		case AF_DECnet:
			fprintf(stream, _(txt_conninfo_type), "DECnet");
#		endif /* DECNET */

		default: /* should not happen */
#		ifdef DEBUG
			fprintf(stream, "CONNECTIONTYPE: unknown type %d\n", nntp_buf.family);
#		endif /* DEBUG */
			break;
	}
#	endif /* HAVE_GETPEERNAME || TLI */

	if (nntp_caps.type == CAPABILITIES) {
		fprintf(stream, "\n");
		if (nntp_caps.implementation)
			fprintf(stream, _(txt_conninfo_implementation), nntp_caps.implementation);
		if (nntp_caps.compress) {
			fprintf(stream, "%s", _(txt_conninfo_compress));
			if ((nntp_caps.compress_algorithm & COMPRESS_DEFLATE) == COMPRESS_DEFLATE) {
#	ifdef USE_ZLIB
				fprintf(stream, _(txt_conninfo_deflate), deflate_active ? _(txt_conninfo_enabled) : _(txt_conninfo_inactive));
#	else
				fprintf(stream, "%s", _(txt_conninfo_deflate_unsupported));
#	endif /* USE_ZLIB */
			}
		}
#	if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
		if (nntp_caps.maxartnum) {
			int n;
			size_t len;
			char *buf;

			if ((n = snprintf(NULL, 0, "%"T_ARTNUM_PFMT, nntp_caps.maxartnum)) > 0) {
				len = (size_t) n + 1;
				buf = my_malloc(len);

				if (snprintf(buf, len, "%"T_ARTNUM_PFMT, nntp_caps.maxartnum) == n)
					fprintf(stream, _(txt_conninfo_maxartnum), buf);
				free(buf);
			}
		}
#	endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */
	}

#	if defined(HAVE_ALARM) && defined(SIGALRM)
	fprintf(stream, _(txt_conninfo_timeout), TIN_NNTP_TIMEOUT, TIN_NNTP_TIMEOUT ? "" : _(txt_conninfo_disabled));
#	endif /* HAVE_ALARM && SIGALRM */

#	ifdef USE_GSASL
	if (nntp_caps.type == CAPABILITIES && nntp_caps.authinfo_sasl) {
		fprintf(stream, _(txt_usable_sasl_mechs), nntp_caps.sasl_mechs ? nntp_caps.sasl_mechs : _(txt_none));
		fprintf(stream, _(txt_used_sasl_mech), nntp_caps.sasl_mech_used ? nntp_caps.sasl_mech_used : _(txt_none));
	}
#	endif /* USE_GSASL */

	{
		char *motd;
		FILE *fp_motd;

		fprintf(stream, "\n");
		if ((fp_motd = tin_fopen(local_motd_file, "r")) != NULL) { /* use local cache */
			while ((motd = tin_fgets(fp_motd, FALSE)) != NULL)
				fprintf(stream, "%s\n", motd);

			fclose(fp_motd);
		} else
			(void) list_motd(stream);
	}

#	ifdef NNTPS_ABLE
	if (nntp_buf.tls_ctx)
		retval = tintls_conninfo(nntp_buf.tls_ctx, stream);
#	endif /* NNTPS_ABLE */

	return retval;
}


#	if defined(MAXARTNUM) && defined(USE_LONG_ARTICLE_NUMBERS)
/*
 * MAXARTNUM - <tnqm14$35bas$1@news.trigofacile.com>
 */
void
set_maxartnum(
	t_bool reconnect)
{
	char cmd[NNTP_STRLEN];
	char line[NNTP_STRLEN] = { '\0' };
	static int cnt = 0;

	if (nntp_caps.maxartnum < T_ARTNUM_CONST(2147483647) || (!reconnect && cnt))
		return;
	else
		cnt++;

	snprintf(cmd, sizeof(cmd), "MAXARTNUM %"T_ARTNUM_PFMT, nntp_caps.maxartnum);

	switch (new_nntp_command(cmd, OK_EXTENSIONS, line, sizeof(line))) {
		case OK_EXTENSIONS:
			nntp_caps.mode_reader = FALSE;
			nntp_caps.reader = TRUE;
			nntp_caps.list_newsgroups = TRUE;
			nntp_caps.list_active = TRUE;
			break;

		default:
			nntp_caps.maxartnum = T_ARTNUM_CONST(0);
			break;
	}
}
#	endif /* MAXARTNUM && USE_LONG_ARTICLE_NUMBERS */
#endif /* NNTP_ABLE */
