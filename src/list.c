/*
 *  Project   : tin - a Usenet reader
 *  Module    : list.c
 *  Author    : I. Lea
 *  Created   : 1993-12-18
 *  Updated   : 2005-07-20
 *  Notes     : Low level functions handling the active[] list and its group_hash index
 *
 * Copyright (c) 1993-2006 Iain Lea <iain@bricbrac.de>
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
#ifdef DEBUG
#	ifndef TCURSES_H
#		include "tcurses.h"
#	endif /* !TCURSES_H */
#endif /* DEBUG */

static int group_hash[TABLE_SIZE];	/* group name --> active[] */

/*
 * local prototypes
 */
static t_bool group_add_to_hash(const char *groupname, int idx);
#ifdef DEBUG
	static void debug_print_active_hash(void);
#endif /* DEBUG */


void
init_group_hash(
	void)
{
	int i;

#if 0
	if (num_active == -1) {
#endif /* 0 */
		num_active = 0;
		for (i = 0; i < TABLE_SIZE; i++)
			group_hash[i] = -1;
#if 0
	}
#endif /* 0 */
}


/*
 * hash group name for fast lookup later
 */
unsigned long
hash_groupname(
	const char *group)
{
/* #define NEW_HASH_METHOD 1 */
#ifdef NEW_HASH_METHOD	/* still testing */
	unsigned long hash = 0L, g, hash_value;
	/* prime == smallest prime number greater than size of string table */
	int prime = 1423;
	const char *p;

	for (p = group; *p; p++) {
		hash = (hash << 4) + *p;
		if ((g = hash & 0xf0000000) != 0) {
			hash ^= g >> 24;
			hash ^= g;
		}
	}
	hash_value = hash % prime;
#	ifdef DEBUG
/*	my_printf("hash=[%s] [%ld]\n", group, hash_value); */
#	endif /* DEBUG */
#else
	unsigned long hash_value = 0L;
	unsigned int len = 0;
	const unsigned char *ptr = (const unsigned char *) group;

	while (*ptr) {
		hash_value = (hash_value << 1) ^ *ptr++;
		if (++len & 7)
			continue;
		hash_value %= TABLE_SIZE;
	}
	hash_value %= TABLE_SIZE;
#endif /* NEW_HASH_METHOD */

	return hash_value;
}


/*
 * Find group name in active[] array and return index otherwise -1
 */
int
find_group_index(
	const char *group)
{
	int i;
	unsigned long h;

	h = hash_groupname(group);
	i = group_hash[h];

	/*
	 * hash linked list chaining
	 */
	while (i >= 0) {
		if (STRCMPEQ(group, active[i].name))
			return i;

		i = active[i].next;
	}

	return -1;
}


/*
 * Find group name in active[] array and return pointer to element
 */
struct t_group *
group_find(
	const char *group_name)
{
	int i;

	if ((i = find_group_index(group_name)) != -1)
		return &active[i];

	return (struct t_group *) 0;
}


/*
 * Hash the groupname into group_hash[]
 * Return FALSE if the group is already present
 */
static t_bool
group_add_to_hash(
	const char *groupname,
	int idx)
{
	unsigned long h = hash_groupname(groupname);

	if (group_hash[h] == -1)
		group_hash[h] = idx;
	else {
		int i;

		/*
		 * hash linked list chaining
		 */
		for (i = group_hash[h]; active[i].next >= 0; i = active[i].next) {
			if (STRCMPEQ(active[i].name, groupname))
				return FALSE;			/* Already in chain */
		}

		if (STRCMPEQ(active[i].name, groupname))
			return FALSE;				/* Already on end of chain */

		active[i].next = idx;			/* Append to chain */
	}

	return TRUE;
}


/*
 * Make sure memory available for next active slot
 * Adds group to the group_hash of active groups and name it
 * Return pointer to next free active slot or NULL if duplicate
 */
struct t_group *
group_add(
	const char *group)
{
	if (num_active >= max_active)		/* Grow memory area if needed */
		expand_active();

	if (!(group_add_to_hash(group, num_active)))
		return NULL;

	active[num_active].name = my_strdup(group);

	return &active[num_active++];		/* NB num_active incremented here */
}


/*
 * Reinitialise group_hash[] after change to ordering of active[]
 */
void
group_rehash(
	t_bool yanked_out)
{
	int i;
	int save_num_active = num_active;

	init_group_hash();				/* Clear the existing hash */
	num_active = save_num_active;	/* init_group_hash() resets this */

	for_each_group(i)
		active[i].next = -1;

	/*
	 * Now rehash each group and rebuild my_group[]
	 */
	selmenu.max = 0;

	for_each_group(i) {
		group_add_to_hash(active[i].name, i);

		/*
		 * cf: similar code to toggle_my_groups()
		 * Add a group if all groups are yanked in
		 * If we are yanked out, only consider subscribed groups
		 * Then also honour show_only_unread and BOGUS_SHOW
		 */
		if (!yanked_out) {
			my_group[selmenu.max++] = i;
			continue;
		}

		if (active[i].subscribed) {
			if (tinrc.show_only_unread_groups) {
				if (active[i].newsrc.num_unread > 0 || (active[i].bogus && tinrc.strip_bogus == BOGUS_SHOW))
					my_group[selmenu.max++] = i;
			} else
				my_group[selmenu.max++] = i;
		}
	}

#ifdef DEBUG
	debug_print_active_hash();
#endif /* DEBUG */
}


#ifdef DEBUG
/*
 * Prints out hash distribution of active[]
 */
static void
debug_print_active_hash(
	void)
{
	int empty = 0;
	int collisions[32];
	int i;

	for (i = 0; i < 32; i++)
		collisions[i] = 0;

	for (i = 0; i < TABLE_SIZE; i++) {
		/* my_printf("HASH[%4d]  ", i); */

		if (group_hash[i] == -1) {
			/* my_printf("EMPTY\n"); */
			empty++;
		} else {
			int j;
			int number = 0;

			for (j = group_hash[i]; active[j].next >= 0; j = active[j].next)
				number++;

			if (number > 31)
				fprintf(stderr, "MEGA HASH COLLISION > 31 HASH[%d]=[%d]!!!\n", i, number);
			else
				collisions[number]++;
		}
	}

	fprintf(stderr, "HashTable Active=[%d] Size=[%d] Filled=[%d] Empty=[%d]\n",
		num_active, TABLE_SIZE, TABLE_SIZE - empty, empty);
	fprintf(stderr, "00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32\n");
	fprintf(stderr, "--------------------------------------------------------------------------------------------------\n");
	for (i = 0; i < 32; i++)
		fprintf(stderr, "%2d ", collisions[i]);
	fprintf(stderr, "\n");
}
#endif /* DEBUG */
