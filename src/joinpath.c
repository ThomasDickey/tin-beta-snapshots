/*
 *  Project   : tin - a Usenet reader
 *  Module    : joinpath.c
 *  Author    : Thomas Dickey <dickey@invisible-island.net>
 *  Created   : 1997-01-10
 *  Updated   : 2008-12-04
 *  Notes     :
 *
 * Copyright (c) 1997-2025 Thomas Dickey <dickey@invisible-island.net>
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

/*
 * Concatenate dir+file, ensuring that we don't introduce extra '/', since some
 * systems (e.g., Apollo) use "//" for special purposes.
 */
void
joinpath(
	char *result,
	size_t result_size,
	const char *dir,
	const char *file)
{
	size_t result_len;

	(void) strncpy(result, dir, result_size - 1);
	result[result_size - 1] = '\0';
	result_len = strlen(result);
	if ((result_len < (result_size - 1)) && (result[0] == '\0' || result[strlen(result) - 1] != '/')) {
		(void) strcat(result, "/");
		result_len++;
	}
	if (result_len < (result_size - 1)) {
		(void) strncat(result, BlankIfNull(file), result_size - result_len - 1);
		result[result_size - 1] = '\0';
	}
}
