/*
 *  Project   : tin - a Usenet reader
 *  Module    : joinpath.c
 *  Author    : Thomas Dickey <dickey@herndon4.his.com>
 *  Created   : 1997-01-10
 *  Updated   : 1997-01-10
 *  Notes     :
 *
 * Copyright (c) 1997-2003 Thomas Dickey <dickey@herndon4.his.com>
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Thomas Dickey.
 * 4. The name of the author may not be used to endorse or promote
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

/*
 * Concatenate dir+file, ensuring that we don't introduce extra '/', since some
 * systems (e.g., Apollo) use "//" for special purposes.
 * TODO ../vms/vmsfile.c defines a joinpath, maybe VMS should be guarded here
 * TODO ../amiga/amiga.c defines a joinpath, plus this section is ifndef AMIGA
 *      so can we junk the #ifdef __amigaos from this code ??????
 */
#ifndef M_AMIGA
void
joinpath(
	char *result,
	const char *dir,
	const char *file)
{
#	ifdef __amigaos
	int i = 0, tmp = 0, tmp2 = 1;
#	endif /* __amigaos */
#	ifdef M_UNIX
	(void) strcpy(result, dir);
	if (result[0] == '\0' || result[strlen(result) - 1] != '/')
		(void) strcat(result, "/");
	(void) strcat(result, BlankIfNull(file));
#	endif /* M_UNIX */
/*
 * JK - horrible hack to convert "/foo/baz/bar" to "foo:baz/bar" (editors bug with *NIX-paths)
 * "foo:baz/bar" -styled paths should always work on Amiga
 */
#	ifdef __amigaos
	if (result[0] == '/')
		while ((result[tmp++] = result[tmp2++]) != 0)
			;
	while (result[i] != '/' && result[i] != ':')
		i++;
	result[i] = ':';
#	endif /* __amigaos */
}
#endif /* !M_AMIGA */
