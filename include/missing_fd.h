/*
 *  Project   : tin - a Usenet reader
 *  Module    : missing_fd.h
 *  Author    : Dennis Grevenstein <dennis.grevenstein@gmail.com>
 *  Created   : 2019-03-09
 *  Updated   : 2019-03-13
 *  Notes     :
 *
 * Copyright (c) 2019-2022 Dennis Grevenstein <dennis.grevenstein@gmail.com>
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


#ifndef TIN_MISSING_FD_H
#	define TIN_MISSING_FD_H
#	ifndef HAVE_TYPE_FD_SET

	/* for SunOS 3.5 */
	/* luna - 09-MAR-2019 */
#	if defined(sun) || defined(__sun) /* && (!defined(__SVR4) || !defined(__svr4__)) && defined(BSD) && BSD < 199306 */
typedef	long	fd_mask;
#		ifndef	FD_SETSIZE
#			ifdef SUNDBE /* Sun DataBase Excelerator */
#				define FD_SETSIZE      2048
#			else
#				define FD_SETSIZE      256
#			endif /* SUNDBE */
#		endif /* !FD_SETSIZE */
#		ifndef NBBY
#			define NBBY 8	/* bits per byte */
#		endif /* !NBBY */
#		define	NFDBITS	(sizeof(fd_mask) * NBBY)	/* bits per mask */
#		ifndef howmany
#			define	howmany(x, y)	(((x) + ((y) - 1)) / (y))
#		endif /* !howmany */

typedef	struct fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;


#		define	FD_SET(n, p)	((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#		define	FD_CLR(n, p)	((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#		define	FD_ISSET(n, p)	((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#		define	FD_ZERO(p)		memset((void *)(p), 0, sizeof (*(p)))

#		endif /* sun || __sun */
#	endif /* !HAVE_TYPE_FD_SET */
#endif /* !TIN_MISSING_FD_H */
