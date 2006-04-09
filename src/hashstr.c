/*
 *  Project   : tin - a Usenet reader
 *  Module    : hashstr.c
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2003-09-19
 *  Notes     :
 *
 * Copyright (c) 1991-2006 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>

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

/*
 * Maintain a table of all strings we have seen.
 * If a new string comes in, add it to the table and return a pointer
 * to it. If we've seen it before, just return the pointer to it.
 *
 * 	Usage: hash_str("some string") returns char *
 *
 * Spillovers are chained on the end
 */

/*
 * Arbitrary table size, but make sure it's prime!
 */
#define HASHNODE_TABLE_SIZE	2411

static struct t_hashnode *table[HASHNODE_TABLE_SIZE];
static struct t_hashnode *add_string(const char *s);

char *
hash_str(
	const char *s)
{
	const unsigned char *t = (const unsigned char *) s;
	int len = 0;
	long h;				/* result of hash: index into hash table */
	struct t_hashnode **p;	/* used to descend the spillover structs */

	if (s == NULL)
		return NULL;

	h = 0;
	while (*t) {
		h = (h << 1) ^ *t++;
		if (++len & 7)
			continue;
		h %= (long) HASHNODE_TABLE_SIZE;
	}
	h %= (long) HASHNODE_TABLE_SIZE;

	p = &table[h];

	while (*p) {
		if (STRCMPEQ(s, (*p)->txt))
			return (*p)->txt;
		p = &(*p)->next;
	}

	*p = add_string(s);
	return (*p)->txt;			/* Return ptr to text, _not_ the struct */
}


/*
 * Add a string to the hash table
 * Each entry will have the following structure:
 *
 * t_hashnode *next		Pointer to the next hashnode in chain
 * int aptr					'magic' ptr used to speed subj threading
 * T							The text itself. The ptr that hash_str()
 * E							returns points here - the earlier fields
 * X							are 'hidden'.
 * T
 * \0							String terminator
 */
static struct t_hashnode *
add_string(
	const char *s)
{
	struct t_hashnode *p;

	p = my_malloc(sizeof(struct t_hashnode) + strlen(s));

	p->next = (struct t_hashnode *) 0;
	p->aptr = -1;					/* -1 is the default value */

	strcpy(p->txt, s);			/* Copy in the text */

	return p;
}


void
hash_init(
	void)
{
	int i;

	for (i = 0; i < HASHNODE_TABLE_SIZE; i++)
		table[i] = (struct t_hashnode *) 0;
}


void
hash_reclaim(
	void)
{
	int i;
	struct t_hashnode *p, *next;

	for (i = 0; i < HASHNODE_TABLE_SIZE; i++) {
		if (table[i] != NULL) {
			p = table[i];
			while (p != NULL) {
				next = p->next;
				free(p);
				p = next;
			}
			table[i] = (struct t_hashnode *) 0;
		}
	}
}
