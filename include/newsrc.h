/*
 *  Project   : tin - a Usenet reader
 *  Module    : newsrc.h
 *  Author    : I. Lea & R. Skrenta
 *  Created   : 1991-04-01
 *  Updated   : 2003-11-18
 *  Notes     : newsrc bit handling
 *
 * Copyright (c) 1997-2006 Iain Lea <iain@bricbrac.de>, Rich Skrenta <skrenta@pbm.com>
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

#ifndef NEWSRC_H
#define NEWSRC_H 1

/*
 * The following macros are used to simplify and speed up the
 * manipulation of the bitmaps in memory which record which articles
 * are read or unread in each news group.
 *
 * Data representation:
 *
 * Each bitmap is handled as an array of bytes; the least-significant
 * bit of the 0th byte is the 0th bit; the most significant bit of
 * the 0th byte is the 7th bit. Thus, the most-significant bit of the
 * 128th byte is the 1023rd bit, and in general the mth bit of the nth
 * byte is considered to be bit (n*8)+m of the map as a whole.	Conversely,
 * the position of bit q in the map is the bit (q & 7) of byte (q >> 3).
 * A bitmap of b bits will be allocated as ((b+7) >> 3) bytes.
 *
 * The routines could be changed to operate on a word-oriented bitmap by
 * changing the constants used from 8 to 16, 3 to 4, 7 to 15, etc. and
 * changing the allocate/deallocate routines.
 *
 * In the newsrc context, a 0 bit represents an article which is read
 * or expired; a 1 represents an unread article. The 0th bit corresponds
 * to the minimum article number for this group, and (max-min+7)/8 bytes
 * are allocated to the bitmap.
 *
 * Constants:
 *
 * NBITS   = total number of bits per byte;
 * NMAXBIT = number of bits per byte or word;
 * NBITPOS = number of bit in NMAXBIT;
 * NBITSON = byte/word used to set all bits in byte/word to 1;
 * NBITNEG1 = binary negation of 1, used in constructing masks.
 *
 * Macro naming and use:
 *
 * The NOFFSET and NBITIDX macro construct the byte and bit indexes in
 * the map, given a bit number.
 *
 * The NSET0 macro sets a bit to binary 0
 * The NSET1 macro sets a bit to binary 1
 * The NSETBLK0 macro sets the same bit or bits to binary 0
 * The NSETBLK1 macro sets the same bit or bits to binary 1
 * The NTEST macro tests a single bit.
 * These are used frequently to access the group bitmap.
 *
 * NSETBLK0 and NSETBLK1 operate on whole numbers of bytes, and are
 * mainly useful for initializing complete bitmaps to one state or
 * another. Both use the memset function, which is assumed to be
 * optimized for the target architecture. NSETBLK is currently used to
 * initialize the group bitmap to 1s (unread).
 *
 * NSETRNG0 and NSETRNG1 operate on ranges of bits, from a low bit number
 * to a high bit number (inclusive), and are especially useful for
 * efficiently setting a contiguous range of bits to one state or another.
 * NSETRNG0 is currently used on the group bitmap to mark the ranges the
 * newsrc file says are read or expired.
 *
 * The algorithm is this. If the high number is less than the low, then
 * do nothing (error); if both fall within the same byte, construct a
 * single mask expressing the range and AND or OR it into the byte; else:
 * construct a mask for the byte containing the low bit, AND or OR it in;
 * use memset to fill in the intervening bytes efficiently; then construct
 * a mask for the byte containing the high bit, and AND or OR this mask
 * in. Masks are constructed by left-shift of 0xff (to set high-order bits
 * to 1), negating a left-shift of 0xfe (to set low-order bits to 1), and
 * the various negations and combinations of the same. This procedure is
 * complex, but 1 to 2 orders of magnitude faster than a shift inside a
 * loop for each bit inside a loop for each individual byte.
 *
 */
#define NBITS		8
#define NMAXBIT		7
#define NBITPOS		3
#define NBITSON		0xff
#define NBITNEG1	0xfe
#define NOFFSET(b)	((b) >> NBITPOS)
#define NBITIDX(b)	((b) & NMAXBIT)

#define NBITMASK(beg,end)	(unsigned char) ~(((1 << (((NMAXBIT - beg) - (NMAXBIT - end)) + 1)) - 1) << (NMAXBIT - end))

#define NTEST(n,b)	(n[NOFFSET(b)] &   (1 << NBITIDX(b)))

#define NSETBLK1(n,i)	(memset (n, NBITSON, (size_t) NOFFSET(i)+1))
#define NSETBLK0(n,i)	(memset (n, 0, (size_t) NOFFSET(i)+1))

/* dbmalloc checks memset() parameters, so we'll use it to check the assignments */
#ifdef USE_DBMALLOC
#	define NSET1(n,b) memset(n + NOFFSET(b), n[NOFFSET(b)] | NTEST(n,b), 1)
#	define NSET0(n,b) memset(n + NOFFSET(b), n[NOFFSET(b)] & ~NTEST(n,b), 1)
#	define BIT_OR(n, b, mask)   memset(n + NOFFSET(b), n[NOFFSET(b)] | (mask), 1)
#	define BIT_AND(n, b, mask)  memset(n + NOFFSET(b), n[NOFFSET(b)] & (mask), 1)
#	include <dbmalloc.h> /* dbmalloc 1.4 */
#else
#	define NSET1(n,b)	(n[NOFFSET(b)] |=  (1 << NBITIDX(b)))
#	define NSET0(n,b)	(n[NOFFSET(b)] &= ~(1 << NBITIDX(b)))
#	define BIT_OR(n, b, mask)   n[NOFFSET(b)] |= mask
#	define BIT_AND(n, b, mask)  n[NOFFSET(b)] &= mask
#endif /* USE_DBMALLOC */

#define BITS_TO_BYTES(n)	((size_t) ((n + NBITS - 1) / NBITS))

#endif /* !NEWSRC_H */
