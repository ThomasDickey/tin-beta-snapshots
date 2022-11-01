/*
 *  Project   : tin - a Usenet reader
 *  Module    : tnntps.h
 *  Author    : Enrik Berkhan
 *  Created   : 2022-09-10
 *  Updated   : 2022-10-19
 *  Notes     : TLS #include files, #defines & struct's
 *
 * Copyright (c) 2022 Enrik Berkhan <Enrik.Berkhan@inka.de>
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


#ifndef TNNTPS_H
#	define TNNTPS_H 1

#	ifdef HAVE_LIB_LIBTLS

#		include <tls.h>

#		if TLS_API < 20200120
#			error "Please use LibreSSL TLS_API >= 20200120"
#		else
#			define USE_LIBTLS 1
#		endif /* TLS_API < 20200120 */

#	else

#		ifdef HAVE_LIB_OPENSSL
#			include <openssl/ssl.h>
#			include <openssl/err.h>
#			include <openssl/rand.h>

#			if OPENSSL_VERSION_NUMBER < 0x1010100fL
#				error "Please use OpenSSL >= 1.1.1"
#			else
#				define USE_OPENSSL 1
#			endif /* OPENSSL_VERSION_NUMBER < 0x1010100fL */

#		else

#			ifdef HAVE_LIB_GNUTLS
#				include <gnutls/gnutls.h>
#				include <gnutls/x509.h>

#				if GNUTLS_VERSION_NUMBER < 0x030700
#					error "Please use GnuTLS >= 3.7.0"
#				else
#					define USE_GNUTLS 1
#				endif /* GNUTLS_VERSION_NUMBER < 0x030700 */
#			endif /* HAVE_LIB_GNUTLS */

#		endif /* HAVE_LIB_OPENSSL */

#	endif /* HAVE_LIB_LIBTLS */

#endif /* !TNNTPS_H */
