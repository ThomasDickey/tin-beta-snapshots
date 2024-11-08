/*
 *  Project   : tin - a Usenet reader
 *  Module    : nntps.c
 *  Author    : E. Berkhan
 *  Created   : 2022-09-10
 *  Updated   : 2024-10-20
 *  Notes     : simple abstraction for various TLS implementations
 *  Copyright : (c) Copyright 2022-2024 Enrik Berkhan <Enrik.Berkhan@inka.de>
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

#if defined(NNTP_ABLE) && defined(NNTPS_ABLE)

#ifndef TNNTPS_H
#	include "tnntps.h"
#endif /* TNNTPS_H */


#ifdef USE_LIBTLS
static struct tls_config *libtls_config = NULL;
#else
#	ifdef USE_GNUTLS
static gnutls_certificate_credentials_t tls_xcreds = NULL;
static char *gnutls_servername = NULL;
static unsigned gnutls_verification_status = 0;
#	else
#		ifdef USE_OPENSSL
static SSL_CTX *openssl_ctx = NULL;
#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* USE_LIBTLS */

#ifdef USE_GNUTLS
static int verification_func(gnutls_session_t session);
#	ifdef DEBUG
static void log_func(int level, const char *msg);
#	endif /* DEBUG */
#else
#	ifdef USE_OPENSSL
#		ifdef DEBUG
static void info_callback(const SSL *s, int where, int ret);
#		endif /* DEBUG */
static void show_errors(const char *msg_fmt);
#	endif /* USE_OPENSSL */
#endif /* USE_GNUTLS */

#if (defined(USE_LIBTLS) && defined(HAVE_LIB_CRYPTO)) || defined(USE_OPENSSL)
static char **get_cert_info(const X509 *cert);
#endif /* (USE_LIBTLS && HAVE_LIB_CRYPTO) || USE_OPENSSL */

static char ca_cert_file_expanded[PATH_LEN];

/*
 * one time init function to initialize the TLS implementation and setup the
 * configuration
 */
int
tintls_init(
	void)
{
	char *ca_cert_file = ca_cert_file_expanded;
#ifdef USE_GNUTLS
	int result = 0;
#endif /* USE_GNUTLS */

	if (tinrc.tls_ca_cert_file && *tinrc.tls_ca_cert_file) {
		if (strfpath(tinrc.tls_ca_cert_file, ca_cert_file_expanded, sizeof(ca_cert_file_expanded), NULL, FALSE) == 0)
			return -EINVAL;
	} else
		ca_cert_file_expanded[0] = '\0';

#ifdef USE_LIBTLS
	/*
	 * libtls does not support compression, no actions needed to disable.
	 */

	if ((libtls_config = tls_config_new()) == NULL) {
		error_message(2, "tls_config_new: out of memory!\n");
		return -ENOMEM;
	}

	/*
	 * Only call tls_config_set_ca_file(3) if ca_cert_file has been
	 * configured by the user. Don't use tls_default_ca_cert_file(3).
	 * Otherwise, a behavioural difference between libretls and libressl
	 * can be triggered.
	 * (see https://git.causal.agency/libretls/about/#Compatibility)
	 */
	if (*ca_cert_file) {
		if (tls_config_set_ca_file(libtls_config, ca_cert_file) != 0) {
			error_message(2, "%s!\n", tls_config_error(libtls_config));
			tls_config_free(libtls_config);
			libtls_config = NULL;
			return -EINVAL;
		}
	}

	if (insecure_nntps) {
		tls_config_insecure_noverifycert(libtls_config);
		tls_config_insecure_noverifyname(libtls_config);
		tls_config_insecure_noverifytime(libtls_config);
	}

#else
#	ifdef USE_GNUTLS

	/*
	 * GnuTLS does no longer support any compression since 3.6.0, no
	 * actions needed to disable, see:
	 * https://www.gnutls.org/manual/gnutls.html#Compression-algorithms-and-the-record-layer
	 */

#		ifdef DEBUG
	if (debug & DEBUG_NNTP) {
		gnutls_global_set_log_level(2);
		gnutls_global_set_log_function(log_func);
		/* gnutls_global_set_audit_log_function(log_func); */
	}
#		endif /* DEBUG */

	if (gnutls_certificate_allocate_credentials(&tls_xcreds) < 0) {
		error_message(2, "gnutls_certificate_allocate_credentials: out of memory!\n");
		return -ENOMEM;
	}

	if (!*ca_cert_file) {
		if ((result = gnutls_certificate_set_x509_system_trust(tls_xcreds)) < 0) {
			error_message(2, "gnutls_certificate_set_x509_system_trust: %s (%d)!\n", gnutls_strerror(result), result);
			gnutls_certificate_free_credentials(tls_xcreds);
			tls_xcreds = NULL;
			return -EINVAL;
		}
	} else {
		if ((result = gnutls_certificate_set_x509_trust_file(tls_xcreds, ca_cert_file, GNUTLS_X509_FMT_PEM)) < 0) {
			error_message(2, "gnutls_certificate_set_x509_trust_file: %s (%d): \"%s\"!\n", gnutls_strerror(result), result, ca_cert_file);
			gnutls_certificate_free_credentials(tls_xcreds);
			tls_xcreds = NULL;
			return -EINVAL;
		}
	}

#	else
#		ifdef USE_OPENSSL
	ERR_clear_error();

	if (RAND_status() != 1) {
		show_errors("RAND_status: %s!\n");
		return -EINVAL;
	}

	if ((openssl_ctx = SSL_CTX_new(TLS_method())) == NULL) {
		show_errors("SSL_CTX_new: %s!\n");
		return -ENOMEM;
	}

	/*
	 * OpenSSL still can support compression, but this option should
	 * already be enabled by default. We want to make sure that TLS
	 * compression is not enabled in any case.
	 * See e.g. RFC 8054 Section 1.1
	 */
	SSL_CTX_set_options(openssl_ctx, SSL_OP_NO_COMPRESSION);

	if (!*ca_cert_file) {
		if (SSL_CTX_set_default_verify_paths(openssl_ctx) != 1) {
			SSL_CTX_free(openssl_ctx);
			openssl_ctx = NULL;
			show_errors("SSL_CTX_set_default_verify_paths: %s!\n");
			return -EINVAL;
		}
	} else {
		if (SSL_CTX_load_verify_locations(openssl_ctx, ca_cert_file, NULL) != 1) {
			SSL_CTX_free(openssl_ctx);
			openssl_ctx = NULL;
			show_errors("SSL_CTX_load_verify_locations: %s!\n");
			return -EINVAL;
		}
	}

	if (insecure_nntps)
		SSL_CTX_set_verify(openssl_ctx, SSL_VERIFY_NONE, NULL);
	else
		SSL_CTX_set_verify(openssl_ctx, SSL_VERIFY_PEER, NULL);

#			ifdef DEBUG
	if (debug & DEBUG_NNTP)
		SSL_CTX_set_info_callback(openssl_ctx, &info_callback);
#			endif /* DEBUG */
#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* USE_LIBTLS */

	return 0;
}


/*
 * one time exit function to release global resources
 */
void
tintls_exit(
	void)
{
#ifdef USE_LIBTLS
	if (libtls_config)
		tls_config_free(libtls_config);
	libtls_config = NULL;
#else
#	ifdef USE_GNUTLS
	gnutls_certificate_free_credentials(tls_xcreds);
	tls_xcreds = NULL;
#	else
#		ifdef USE_OPENSSL
	ERR_clear_error();
	SSL_CTX_free(openssl_ctx);
	openssl_ctx = NULL;
#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* USE_LIBTLS */
}


/*
 * open a TLS session with the server using an already opened TCP connection
 */
int
tintls_open(
	const char *servername,
	int fd,
	void **session_ctx)
{
#ifdef USE_LIBTLS
	struct tls *client;

	if (!session_ctx)
		return -EINVAL;

	*session_ctx = NULL;

	if ((client = tls_client()) == NULL)
		return -ENOMEM;

	if (tls_configure(client, libtls_config) == -1) {
		tls_free(client);
		return -ENOMEM;
	}

	if (tls_connect_socket(client, fd, servername) == -1) {
		tls_free(client);
		tintls_exit();
		return -ENOMEM;
	}

	*session_ctx = client;

#else
#	ifdef USE_GNUTLS
	gnutls_session_t client;
	size_t servername_len;

	if (!session_ctx)
		return -EINVAL;

	*session_ctx = NULL;

	if (gnutls_init(&client, GNUTLS_CLIENT | GNUTLS_AUTO_REAUTH | GNUTLS_POST_HANDSHAKE_AUTH) < 0)
		return -ENOMEM;

	if (gnutls_server_name_set(client, GNUTLS_NAME_DNS, servername, strlen(servername)) < 0) {
		gnutls_deinit(client);
		return -ENOMEM;
	}

	if (gnutls_set_default_priority(client) < 0) {
		gnutls_deinit(client);
		return -EINVAL;
	}

	if (gnutls_credentials_set(client, GNUTLS_CRD_CERTIFICATE, tls_xcreds) < 0) {
		gnutls_deinit(client);
		return -EINVAL;
	}

	servername_len = strlen(servername) + 1;
	gnutls_servername = my_malloc(servername_len);
	strncpy(gnutls_servername, servername, servername_len);
	gnutls_session_set_verify_function(client, &verification_func);

	gnutls_transport_set_int(client, fd);
	gnutls_handshake_set_timeout(client, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

	*session_ctx = client;

#	else
#		ifdef USE_OPENSSL
	SSL *ssl;
	BIO *sock;
	BIO *client;

	if (!session_ctx)
		return -EINVAL;

	*session_ctx = NULL;

	ERR_clear_error();

	sock = BIO_new_socket(fd, 1);
	if (!sock) {
		show_errors("BIO_new_socket: %s!\n");
		return -ENOMEM;
	}

	client = BIO_new_ssl(openssl_ctx, 1);
	if (!client) {
		BIO_free(sock);
		show_errors("BIO_new_ssl: %s!\n");
		return -ENOMEM;
	}

	if (BIO_get_ssl(client, &ssl) != 1) {
		BIO_free(client);
		BIO_free(sock);
		show_errors("BIO_get_ssl: %s!\n");
		return -EINVAL;
	}

	if (SSL_set_tlsext_host_name(ssl, servername) != 1) {
		BIO_free(client);
		BIO_free(sock);
		show_errors("SSL_set_tlsext_host_name: %s!\n");
		return -EINVAL;
	}

	if (SSL_set1_host(ssl, servername) != 1) {
		BIO_free(client);
		BIO_free(sock);
		show_errors("SSL_set1_host: %s!\n");
		return -EINVAL;
	}

	*session_ctx = BIO_push(client, sock);
#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* USE_LIBTLS */

	return 0;
}


/*
 * explicitly trigger the initial handshake
 */
int
tintls_handshake(
	void *session_ctx)
{
#ifdef USE_LIBTLS
	int result;
	struct tls *client = session_ctx;
	const char *subject, *issuer, *version, *cipher;
#	ifdef HAVE_LIB_CRYPTO
	BIO *io_buf = NULL;
	const uint8_t *chain;
	size_t chain_size;
#	endif /* HAVE_LIB_CRYPTO */

	do {
		result = tls_handshake(client);
	} while (result == TLS_WANT_POLLIN || result == TLS_WANT_POLLOUT);

	if (result < 0) {
		const char *err = tls_error(client);

		error_message(2, _(txt_tls_handshake_failed), err ? err : _(txt_unknown_error));
		return -EPROTO;
	}

#	ifdef HAVE_LIB_CRYPTO
	if ((chain = tls_peer_cert_chain_pem(client, &chain_size)))
		io_buf = BIO_new(BIO_s_mem());

	if (chain && io_buf) {
		if (chain_size > 0 && BIO_write(io_buf, chain, chain_size) > 0) {
			char **cert_info;
			X509 *cert = PEM_read_bio_X509(io_buf, NULL, NULL, NULL);

			if (cert && ((cert_info = get_cert_info(cert)))) {
				wait_message(0, _(txt_conninfo_subject), cert_info[0] ? cert_info[0] : _(txt_retr_subject_failed));
				wait_message(0, _(txt_conninfo_issuer), cert_info[1] ? cert_info[1] : _(txt_retr_issuer_failed));
				FreeIfNeeded(cert_info[0]);
				FreeIfNeeded(cert_info[1]);
				free(cert_info);
				X509_free(cert);
			}
		}
		BIO_free(io_buf);
	} else
#	endif /* HAVE_LIB_CRYPTO */
	{ /* Fallback if access to the certificate chain has failed */
		if (!batch_mode || verbose) {
			subject = tls_peer_cert_subject(client);
			issuer = tls_peer_cert_issuer(client);

			if (!subject)
				subject = _(txt_retr_subject_failed);
			if (!issuer)
				issuer = _(txt_retr_issuer_failed);
			wait_message(0, _(txt_conninfo_subject), subject);
			wait_message(0, _(txt_conninfo_issuer), issuer);
		}
	}

	if (!batch_mode || verbose) {
		version = tls_conn_version(client);
		cipher = tls_conn_cipher(client);

		if (!version)
			version = _(txt_retr_version_failed);
		if (!cipher)
			cipher = _(txt_retr_cipher_failed);

		wait_message(0, _(txt_tls_handshake_done), version, cipher);
	}
#else
#	ifdef USE_GNUTLS
	int result;
	gnutls_session_t client = session_ctx;

	do {
		result = gnutls_handshake(client);
	} while (result < 0 && gnutls_error_is_fatal(result) == 0);

	if (result < 0) {
		if (gnutls_verification_status != 0) {
			int status_result;
			int type;
			gnutls_datum_t msg;

			type = gnutls_certificate_type_get2(client, GNUTLS_CTYPE_SERVER);
			status_result = gnutls_certificate_verification_status_print(gnutls_verification_status, type, &msg, 0);

			if (status_result == 0)
				wait_message(0, _(txt_tls_peer_verify_failed), msg.data);
			else
				wait_message(0, _(txt_tls_peer_verify_failed), _(txt_tls_unable_to_get_status));

			gnutls_free(msg.data);
		}

		error_message(2, _(txt_tls_handshake_failed_with_err_num), gnutls_strerror(result), result);

		return -EPROTO;
	} else {
		char *desc;
		const gnutls_datum_t *raw_servercert_chain;
		unsigned int servercert_chainlen;

		if (gnutls_verification_status != 0) {
			int type;
			gnutls_datum_t msg;

			if (!insecure_nntps) {
				error_message(2, _(txt_tls_unexpected_status));
				return -EPROTO;
			}

			type = gnutls_certificate_type_get2(client, GNUTLS_CTYPE_SERVER);
			result = gnutls_certificate_verification_status_print(gnutls_verification_status, type, &msg, 0);

			if (result == 0)
				wait_message(0, _(txt_tls_peer_verify_failed_continuing), msg.data);
			else
				wait_message(0, _(txt_tls_peer_verify_failed_continuing), _(txt_tls_unable_to_get_status));

			gnutls_free(msg.data);

			if (!insecure_nntps)
				return -EPROTO;
		}

		raw_servercert_chain = gnutls_certificate_get_peers(client, &servercert_chainlen);
		if (servercert_chainlen > 0) {
			gnutls_x509_crt_t servercert = NULL;
			gnutls_datum_t subject = { NULL, 0 };
			gnutls_datum_t issuer = { NULL, 0 };

			result = gnutls_x509_crt_init(&servercert);
			if (result < 0) {
				error_message(1, "gnutls_x509_crt_init: %s (%d)\n", gnutls_strerror(result), result);
				goto err_cert;
			}

			result = gnutls_x509_crt_import(servercert, &raw_servercert_chain[0], GNUTLS_X509_FMT_DER);
			if (result < 0) {
				error_message(1, "gnutls_x509_crt_import: %s (%d)\n", gnutls_strerror(result), result);
				goto err_cert;
			}

			result = gnutls_x509_crt_get_dn3(servercert, &subject, 0);
			if (result < 0) {
				error_message(1, "gnutls_x509_crt_get_dn3: %s (%d)\n", gnutls_strerror(result), result);
				goto err_cert;
			}

			result = gnutls_x509_crt_get_issuer_dn3(servercert, &issuer, 0);
			if (result < 0) {
				error_message(1, "gnutls_x509_crt_get_issuer_dn3: %s (%d)\n", gnutls_strerror(result), result);
				goto err_cert;
			}

			if (!batch_mode || verbose) {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				char *dt = my_strdup((char *) subject.data);
				size_t len_dt = strlen(dt);

				process_charsets(&dt, &len_dt, "UTF-8", tinrc.mm_local_charset, FALSE);
				wait_message(0, _(txt_conninfo_subject), dt);
				free(dt);

				dt = my_strdup((char *) issuer.data);
				len_dt = strlen(dt);
				process_charsets(&dt, &len_dt, "UTF-8", tinrc.mm_local_charset, FALSE);
				wait_message(0, _(txt_conninfo_issuer), dt);
				free(dt);
#else
				wait_message(0, _(txt_conninfo_subject), subject.data);
				wait_message(0, _(txt_conninfo_issuer), issuer.data);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			}

err_cert:
			if (issuer.data)
				gnutls_free(issuer.data);
			if (subject.data)
				gnutls_free(subject.data);
			if (servercert)
				gnutls_x509_crt_deinit(servercert);
		}

		if (!batch_mode || verbose) {
			if ((desc = gnutls_session_get_desc(client)) != NULL) {
				wait_message(0, _(txt_tls_handshake_done), desc);
				gnutls_free(desc);
			}
		}
	}
#	else
#		ifdef USE_OPENSSL
	long long_result;
	BIO *client = session_ctx;
	SSL *ssl;
	X509 *peer;

	ERR_clear_error();

	long_result = BIO_get_ssl(client, &ssl);
	if (long_result != 1) {
		show_errors("BIO_get_ssl: %s!\n");
		return -EINVAL;
	}

	long_result = BIO_do_handshake(client);
	if (long_result != 1) {
		long_result = SSL_get_verify_result(ssl);
		if (long_result != X509_V_OK)
			error_message(2, _(txt_tls_handshake_failed), X509_verify_cert_error_string(long_result));
		else
			show_errors(_(txt_tls_handshake_failed));

		return -EPROTO;
	} else if (insecure_nntps) {
		long_result = SSL_get_verify_result(ssl);
		if (long_result != X509_V_OK && (!batch_mode || verbose))
			wait_message(0, _(txt_tls_peer_verify_failed_continuing), X509_verify_cert_error_string(long_result));
	}

	if (!batch_mode || verbose) {
		if ((peer = SSL_get_peer_certificate(ssl))) {
			char **cert_info;

			if ((cert_info = get_cert_info(peer))) {
				wait_message(0, _(txt_conninfo_subject), BlankIfNull(cert_info[0]));
				wait_message(0, _(txt_conninfo_issuer), BlankIfNull(cert_info[1]));
				FreeIfNeeded(cert_info[0]);
				FreeIfNeeded(cert_info[1]);
				free(cert_info);
			}
			X509_free(peer);
		}

		wait_message(0, _(txt_tls_handshake_done), SSL_get_cipher_name(ssl));
	}
#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* USE_LIBTLS */

	return 0;
}


/*
 * read from TLS session
 */
ssize_t
tintls_read(
	void *session_ctx,
	void *buf,
	size_t count)
{
#ifdef USE_LIBTLS
	ssize_t result;
	struct tls *client = session_ctx;

	do {
		result = tls_read(client, buf, count);
	} while (result == TLS_WANT_POLLIN || result == TLS_WANT_POLLOUT);

	return result;
	/* NOTREACHED */
#else
#	ifdef USE_GNUTLS
	ssize_t result = GNUTLS_E_AGAIN;
	gnutls_session_t client = session_ctx;

	while (result == GNUTLS_E_AGAIN || result == GNUTLS_E_INTERRUPTED || result == GNUTLS_E_REHANDSHAKE) {
		result = gnutls_record_recv(client, buf, count);
	}

	return result;
	/* NOTREACHED */
#	else
#		ifdef USE_OPENSSL
	int result;
	size_t bytes_read;
	BIO *client = session_ctx;

	ERR_clear_error();

	result = BIO_read_ex(client, buf, count, &bytes_read);
	if (result == 1)
		return (ssize_t) bytes_read;

#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* USE_LIBTLS */

	return -1;
}


/*
 * write to TLS session
 */
ssize_t
tintls_write(
	void *session_ctx,
	const void *buf,
	size_t count)
{
#ifdef USE_LIBTLS
	ssize_t result;
	struct tls *client = session_ctx;

	do {
		result = tls_write(client, buf, count);
	} while (result == TLS_WANT_POLLOUT || result == TLS_WANT_POLLIN);

	return result;
	/* NOTREACHED */
#else
#	ifdef USE_GNUTLS
	ssize_t result = GNUTLS_E_AGAIN;
	gnutls_session_t client = session_ctx;

	while (result == GNUTLS_E_INTERRUPTED || result == GNUTLS_E_AGAIN) {
		result = gnutls_record_send(client, buf, count);
	}

	return result;
	/* NOTREACHED */
#	else
#		ifdef USE_OPENSSL
	int result;
	BIO *client = session_ctx;
	size_t bytes_written;

	ERR_clear_error();

	result = BIO_write_ex(client, buf, count, &bytes_written);
	if (result == 1)
		return (ssize_t) bytes_written;

#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* USE_LIBTLS */

	return -1;
}


/*
 * close the TLS session, but not the underlying TCP connection
 */
int
tintls_close(
	void *session_ctx)
{
#ifdef USE_LIBTLS
	int result;
	struct tls *client = session_ctx;

	do {
		result = tls_close(client);
	} while (result == TLS_WANT_POLLIN || result == TLS_WANT_POLLOUT);

	tls_free(client);

	if (result == -1)
		return -EPROTO;

	return result;
	/* NOTREACHED */
#else
#	ifdef USE_GNUTLS
	int result;
	gnutls_session_t client = session_ctx;

	do {
		result = gnutls_bye(client, GNUTLS_SHUT_RDWR);
	} while (result < 0 && gnutls_error_is_fatal(result) == 0);

	gnutls_deinit(client);
	FreeAndNull(gnutls_servername);

	if (result < 0)
		return -EPROTO;

#	else
#		ifdef USE_OPENSSL
	BIO *client = session_ctx;
	ERR_clear_error();
	BIO_free_all(client);
#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* USE_LIBTLS */

	return 0;
}


/* TODO: make date-format configurable? */
#define PRINT_VALID_AFTER(ts, what) do { \
		if (my_strftime(what, sizeof(what), "%Y-%m-%dT%H:%M%z", ts)) \
			fprintf(fp, _(txt_valid_not_after), (what)); \
		else \
			fprintf(fp, "%s", txt_conninfo_fmt_error); \
	} while (0)

#define PRINT_VALID_BEFORE(ts, what) do { \
		if (my_strftime(what, sizeof(what), "%Y-%m-%dT%H:%M%z", ts)) \
			fprintf(fp, _(txt_valid_not_before), (what)); \
		else \
			fprintf(fp, "%s", txt_conninfo_fmt_error); \
	} while (0)


int
tintls_conninfo(
	void *session_ctx,
	FILE *fp)
{
#ifdef USE_LIBTLS
	struct tls *client = session_ctx;
	char fmt_time[22]; /* %Y-%m-%dT%H:%M%z */
#	ifdef HAVE_LIB_CRYPTO
	BIO *io_buf = NULL;
	const uint8_t *chain;
	size_t chain_size;

	fprintf(fp, "%s", _(txt_conninfo_tls_info));
	fprintf(fp, _(txt_conninfo_libtls_info), tls_conn_version(client), tls_conn_cipher(client), tls_conn_cipher_strength(client));

	if ((chain = tls_peer_cert_chain_pem(client, &chain_size)))
		io_buf = BIO_new(BIO_s_mem());

	if (chain && io_buf) {
		X509 *cert;
		char **cert_info;
		const ASN1_TIME *asn1;
		char *wchain, *cptr;
		int i;
		size_t cl;
		struct tm tm;

		fprintf(fp, "%s", _(txt_conninfo_server_cert_info));

		/* string copy of chain */
		i = snprintf(NULL, 0, "%.*s", (int) chain_size, (const char*) chain);
		cl = (size_t) i + 1;
		wchain = my_malloc(cl);
		snprintf(wchain, cl, "%.*s", (int) chain_size, (const char*) chain);
		cptr = wchain;
		i = 0;

		while ((cptr = strstr(cptr, "-----BEGIN CERTIFICATE-----"))) {
			chain_size = strlen(cptr);
			if (chain_size > 0 && BIO_write(io_buf, cptr, chain_size) > 0) {
				cert = PEM_read_bio_X509(io_buf, NULL, NULL, NULL);
				if (cert && ((cert_info = get_cert_info(cert)))) {
					if (i)
						fputs("\n", fp);
					fprintf(fp, _(txt_conninfo_cert), i++);

					fprintf(fp, _(txt_conninfo_subject), BlankIfNull(cert_info[0]));
					fprintf(fp, _(txt_conninfo_issuer), BlankIfNull(cert_info[1]));
					FreeIfNeeded(cert_info[0]);
					FreeIfNeeded(cert_info[1]);
					free(cert_info);

					if ((asn1 = X509_get0_notBefore(cert)) != NULL) {
						if (ASN1_TIME_to_tm(asn1, &tm) == 1) {
							PRINT_VALID_BEFORE(&tm, fmt_time);
						}
					}
					if ((asn1 = X509_get0_notAfter(cert)) != NULL) {
						if (ASN1_TIME_to_tm(asn1, &tm) == 1) {
							PRINT_VALID_AFTER(&tm, fmt_time);
						}
					}
					X509_free(cert);
				}
			}
			BIO_reset(io_buf);
			cptr += 26; /* "-----BEGIN CERTIFICATE-----" */
		}
		free(wchain);
		BIO_free(io_buf);
	} else /* Fallback if access to the certificate chain has failed */
#	else
	fprintf(fp, "%s", _(txt_conninfo_tls_info));
	fprintf(fp, _(txt_conninfo_libtls_info), tls_conn_version(client), tls_conn_cipher(client), tls_conn_cipher_strength(client));
	fprintf(fp, "%s", _(txt_conninfo_server_cert_info));
#	endif /* HAVE_LIB_CRYPTO */
	{
		struct tm *tm;
		time_t t;

		fprintf(fp, _(txt_conninfo_subject), tls_peer_cert_subject(client));
		fprintf(fp, _(txt_conninfo_issuer), tls_peer_cert_issuer(client));

		if ((t = tls_peer_cert_notbefore(client)) != -1) {
			tm = gmtime(&t);
			PRINT_VALID_BEFORE(tm, fmt_time);
		}

		if ((t = tls_peer_cert_notafter(client)) != -1) {
			tm = gmtime(&t);
			PRINT_VALID_AFTER(tm, fmt_time);
		}
	}
#else
#	ifdef USE_GNUTLS
	char *desc;
	char fmt_time[22]; /* %Y-%m-%dT%H:%M%z */
	int retval = -1;
	int result;
	gnutls_session_t client = session_ctx;
	gnutls_datum_t msg;
	const gnutls_datum_t *raw_servercert_chain;
	unsigned int servercert_chainlen;
	unsigned int i;
	time_t t;
	struct tm *tm;

	desc = gnutls_session_get_desc(client);
	fprintf(fp, "%s", _(txt_conninfo_tls_info));
	fprintf(fp, "%s\n", desc);
	gnutls_free(desc);

	msg.data = NULL;

	if (gnutls_verification_status != 0) {
		int type = gnutls_certificate_type_get2(client, GNUTLS_CTYPE_SERVER);

		result = gnutls_certificate_verification_status_print(gnutls_verification_status, type, &msg, 0);

		if (result == 0)
			fprintf(fp, _(txt_conninfo_verify_failed), msg.data,
					insecure_nntps ? _(txt_conninfo_error_tolerated) : _(txt_conninfo_error_unexpected));
		else
			fprintf(fp, "%s", _(txt_conninfo_verify_failed_no_reason));

		gnutls_free(msg.data);
	} else
		fprintf(fp, "%s", _(txt_conninfo_verify_successful));

	raw_servercert_chain = gnutls_certificate_get_peers(client, &servercert_chainlen);
	if (servercert_chainlen > 0)
		fprintf(fp, "%s", _(txt_conninfo_server_cert_info));

	for (i = 0; i < servercert_chainlen; i++) {
		gnutls_x509_crt_t servercert = NULL;
		gnutls_datum_t subject = { NULL, 0 };
		gnutls_datum_t issuer = { NULL, 0 };
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
		char *sub;
		char *iss;
		size_t len_s;
		size_t len_i;
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */

		if (i)
			fputs("\n", fp);
		fprintf(fp, _(txt_conninfo_cert), i);

		result = gnutls_x509_crt_init(&servercert);
		if (result < 0)
			goto err_cert;

		result = gnutls_x509_crt_import(servercert, &raw_servercert_chain[i], GNUTLS_X509_FMT_DER);
		if (result < 0)
			goto err_cert;

		result = gnutls_x509_crt_get_dn3(servercert, &subject, 0);
		if (result < 0)
			goto err_cert;
		else {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			sub = (char *) subject.data;
			len_s = strlen(sub);
			process_charsets(&sub, &len_s, "UTF-8", tinrc.mm_local_charset, FALSE);
			fprintf(fp, _(txt_conninfo_subject), sub);
#else
			fprintf(fp, _(txt_conninfo_subject), subject.data);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		}

		result = gnutls_x509_crt_get_issuer_dn3(servercert, &issuer, 0);
		if (result < 0)
			goto err_cert;
		else {
#if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
			iss = (char *) issuer.data;
			len_i = strlen(iss);
			process_charsets(&iss, &len_i, "UTF-8", tinrc.mm_local_charset, FALSE);
			fprintf(fp, _(txt_conninfo_issuer), iss);
#else
			fprintf(fp, _(txt_conninfo_issuer), issuer.data);
#endif /* MULTIBYTE_ABLE && !NO_LOCALE */
		}

		t = gnutls_x509_crt_get_activation_time(servercert);
		if (t == -1)
			goto err_cert;

		tm = localtime(&t);
		PRINT_VALID_BEFORE(tm, fmt_time);

		t = gnutls_x509_crt_get_expiration_time(servercert);
		if (t == -1)
			goto err_cert;

		tm = localtime(&t);
		PRINT_VALID_AFTER(tm, fmt_time);

		retval = 0;

err_cert:
		if (issuer.data)
			gnutls_free(issuer.data);
		if (subject.data)
			gnutls_free(subject.data);
		if (servercert)
			gnutls_x509_crt_deinit(servercert);
	}

	return retval;
#	else

#		ifdef USE_OPENSSL
	long long_result;
	long verification_result;
	BIO *client = session_ctx;
	SSL *ssl;
	STACK_OF(X509) *chain;

	long_result = BIO_get_ssl(client, &ssl);
	if (long_result != 1)
		return -1;

	fprintf(fp, "%s", _(txt_conninfo_tls_info));
	fprintf(fp, "%s %s\n", SSL_get_version(ssl), SSL_get_cipher_name(ssl));

	verification_result = SSL_get_verify_result(ssl);
	if (verification_result != X509_V_OK)
		fprintf(fp, _(txt_conninfo_verify_failed),
			X509_verify_cert_error_string(verification_result),
			insecure_nntps ? _(txt_conninfo_error_tolerated) : _(txt_conninfo_error_unexpected));
	else
		fprintf(fp, "%s", _(txt_conninfo_verify_successful));

	fprintf(fp, "%s", _(txt_conninfo_server_cert_info));

	if (verification_result == X509_V_OK)
		chain = SSL_get_peer_cert_chain(ssl);
	else
		chain = SSL_get0_verified_chain(ssl);

	if (chain) {
		char name[22]; /* %Y-%m-%dT%H:%M%z */
		char **cert_info;
		const ASN1_TIME *asn1;
		int i;
		struct tm tm;

		for (i = 0; i < sk_X509_num(chain); i++) {
			X509* cert = sk_X509_value(chain, i);

			if (i)
				fputs("\n", fp);
			fprintf(fp, _(txt_conninfo_cert), i);

			if ((cert_info = get_cert_info(cert))) {
				fprintf(fp, _(txt_conninfo_subject), BlankIfNull(cert_info[0]));
				fprintf(fp, _(txt_conninfo_issuer), BlankIfNull(cert_info[1]));
				FreeIfNeeded(cert_info[0]);
				FreeIfNeeded(cert_info[1]);
				free(cert_info);
			}

			if ((asn1 = X509_get0_notBefore(cert)) != NULL) {
				if (ASN1_TIME_to_tm(asn1, &tm) == 1) {
					PRINT_VALID_BEFORE(&tm, name);
				}
			}

			if ((asn1 = X509_get0_notAfter(cert)) != NULL) {
				if (ASN1_TIME_to_tm(asn1, &tm) == 1) {
					PRINT_VALID_AFTER(&tm, name);
				}
			}
		}
	}
#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* USE_LIBTLS */
	return 0;
}


#undef PRINT_VALID_AFTER
#undef PRINT_VALID_BEFORE


#if (defined(USE_LIBTLS) && defined(HAVE_LIB_CRYPTO)) || defined(USE_OPENSSL)
static char **
get_cert_info(
	const X509 *cert)
{
	BIO *io_buf;
	char **res = NULL;
	char *tmp, *subject = NULL, *issuer = NULL;
	long len;
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
	unsigned long flags = XN_FLAG_ONELINE & ~ASN1_STRFLGS_ESC_MSB & ~XN_FLAG_SPC_EQ;
#	else
	unsigned long flags = XN_FLAG_ONELINE & ~XN_FLAG_SPC_EQ;
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */

	if (cert && (io_buf = BIO_new(BIO_s_mem()))) {
		res = my_malloc(sizeof(char *) * 2);
		if (X509_NAME_print_ex(io_buf, X509_get_subject_name(cert), 0, flags) != -1) {
			len = BIO_get_mem_data(io_buf, &tmp);
			if (len > 0) {
				subject = my_malloc((size_t) len + 1);
				memcpy(subject, tmp, (size_t) len);
				subject[len] = '\0';
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				process_charsets(&subject, (size_t *) &len, "UTF-8", tinrc.mm_local_charset, FALSE);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			}
		}
		if (BIO_reset(io_buf) != -1 && X509_NAME_print_ex(io_buf, X509_get_issuer_name(cert), 0, flags) != -1) {
			len = BIO_get_mem_data(io_buf, &tmp);
			if (len > 0) {
				issuer = my_malloc((size_t) len + 1);
				memcpy(issuer, tmp, (size_t) len);
				issuer[len] = '\0';
#	if defined(MULTIBYTE_ABLE) && !defined(NO_LOCALE)
				process_charsets(&issuer, (size_t *) &len, "UTF-8", tinrc.mm_local_charset, FALSE);
#	endif /* MULTIBYTE_ABLE && !NO_LOCALE */
			}
		}
		res[0] = subject;
		res[1] = issuer;
		BIO_free(io_buf);
	}
	return res;
}
#endif /* (USE_LIBTLS && HAVE_LIB_CRYPTO) || USE_OPENSSL */


#ifdef USE_OPENSSL
static void
show_errors(
	const char *msg_fmt)
{
	unsigned long next_error;

	while ((next_error = ERR_get_error())) {
		error_message(0, msg_fmt, ERR_error_string(next_error, NULL));
	}
}
#endif /* USE_OPENSSL */


#ifdef USE_GNUTLS
static int
verification_func(
	gnutls_session_t session)
{
	int result;

	gnutls_verification_status = ~0;

	result = gnutls_certificate_verify_peers3(session, gnutls_servername, &gnutls_verification_status);

	if (insecure_nntps)
		return 0;

	if (result != 0)
		return result;

	return gnutls_verification_status;
}
#endif /* USE_GNUTLS */


#ifdef DEBUG
#	ifdef USE_GNUTLS
static void
log_func(
	int level,
	const char *msg)
{
	int msglen = (int) strlen(msg);

	if (msglen <= 0)
		return;

	if (msg[msglen - 1] == '\n')
		msglen--;

	debug_print_file("NNTP", "TLS%s%.*s [%d]", logtime(), msglen, msg, level);
}


#	else
#		ifdef USE_OPENSSL
static void
info_callback(
	const SSL *s,
	int where,
	int ret)
{
	const char *str;
	int w = where & ~SSL_ST_MASK;

	if (w & SSL_ST_CONNECT)
		str = "SSL_connect";
	else if (w & SSL_ST_ACCEPT)
		str = "SSL_accept";
	else
		str = "undefined";

	if (where & SSL_CB_LOOP) {
		debug_print_file("NNTP", "TLS%s: %s:%s", logtime(), str, SSL_state_string_long(s));
	} else if (where & SSL_CB_HANDSHAKE_START) {
		debug_print_file("NNTP", "TLS%s: %s:handshake start", logtime(), str);
	} else if (where & SSL_CB_HANDSHAKE_DONE) {
		debug_print_file("NNTP", "TLS%s: %s:handshake done", logtime(), str);
	} else if (where & SSL_CB_ALERT) {
		str = (where & SSL_CB_READ) ? "read" : "write";
		debug_print_file("NNTP", "TLS%s: SSL3 alert %s:%s:%s", logtime(), str,
				SSL_alert_type_string_long(ret),
				SSL_alert_desc_string_long(ret));
	} else if (where & SSL_CB_EXIT) {
		if (ret == 0) {
			debug_print_file("NNTP", "TLS%s: %s:failed in %s", logtime(),
					str, SSL_state_string_long(s));
		} else if (ret < 0) {
			debug_print_file("NNTP", "TLS%s: %s:error in %s", logtime(),
					str, SSL_state_string_long(s));
		}
	}
}
#		endif /* USE_OPENSSL */
#	endif /* USE_GNUTLS */
#endif /* DEBUG */


#else
int
tintls_init(
	void)
{
	return 0;
}


void
tintls_exit(
	void)
{
	return;
}
#endif /* NNTP_ABLE && NNTPS_ABLE */
