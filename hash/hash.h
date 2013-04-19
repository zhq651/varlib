/* hash - hashing table processing.
   Copyright (C) 1998, 1999, 2001 Free Software Foundation, Inc.
   Written by Jim Meyering <meyering@ascend.com>, 1998.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* A generic hash table package.  */

/* Make sure USE_OBSTACK is defined to 1 if you want the allocator to use
   obstacks instead of malloc, and recompile `hash.c' with same setting.  */

#ifndef HASH_H_
# define HASH_H_
#include <sys/types.h>
#ifdef WIN32
 #include <stdint.h>
#endif /*end if define(WIN32) */


#ifndef _STDBOOL_H_
#define	_STDBOOL_H_	

#define	__bool_true_false_are_defined	1

#ifndef __cplusplus

#define	false	0
#define	true	1

#define	bool	_Bool
#if __STDC_VERSION__ < 199901L && __GNUC__ < 3
typedef	int	_Bool;
#endif

#endif /* !__cplusplus */

#endif /* !_STDBOOL_H_ */

#ifndef __HAVE_HASH32_BUF			/* not overridden by MD hash */

#define	HASH32_BUF_INIT	5381

/*
 * uint32_t
 * hash32_buf(const void *bf, size_t len, uint32_t hash)
 *	return a 32 bit hash of the binary buffer buf (size len),
 *	seeded with an initial hash value of hash (usually HASH32_BUF_INIT).
 */
static __inline uint32_t
hash32_buf(const void *bf, size_t len, uint32_t hash)
{
	const uint8_t *s = (const uint8_t*)bf;

	while (len-- != 0)			/* "nemesi": k=257, r=r*257 */
		hash = hash * 257 + *s++;
	return (hash * 257);
}
#endif	/* __HAVE_HASH32_BUF */


static __inline uint32_t
hash_pjw (const void *x,unsigned int tablesize)
{
  const char *s =(const char *) x;
  unsigned int h = 0;
  unsigned int g;
  while (*s != 0)
    {
      h = (h << 4) + *s++;
      if ((g = h & (unsigned int) 0xf0000000) != 0)
        h = (h ^ (g >> 24)) ^ g;
    }

  return (h%tablesize); 
} 
#ifndef __HAVE_HASH32_STR			/* not overridden by MD hash */

#define	HASH32_STR_INIT	5381
/*
 * uint32_t
 * hash32_str(const void *bf, uint32_t hash)
 *	return a 32 bit hash of NUL terminated ASCII string buf,
 *	seeded with an initial hash value of hash (usually HASH32_STR_INIT).
 */
static __inline uint32_t
hash32_str(const void *bf, uint32_t hash)
{
	const uint8_t *s = (const uint8_t*)bf;
	uint8_t	c;

	while ((c = *s++) != 0)
		hash = hash * 33 + c;		/* "perl": k=33, r=r+r/32 */
	return (hash + (hash >> 5));
}

/*
 * uint32_t
 * hash32_strn(const void *bf, size_t len, uint32_t hash)
 *	return a 32 bit hash of NUL terminated ASCII string buf up to
 *	a maximum of len bytes,
 *	seeded with an initial hash value of hash (usually HASH32_STR_INIT).
 */
static __inline uint32_t
hash32_strn(const void *bf, size_t len, uint32_t hash)
{
	const uint8_t	*s = (const uint8_t*)bf;
	uint8_t	c;

	while ((c = *s++) != 0 && len-- != 0)
		hash = hash * 33 + c;		/* "perl": k=33, r=r+r/32 */
	return (hash + (hash >> 5));
}
#endif	/* __HAVE_HASH32_STR */
#ifndef PARAMS
#define PARAMS(Args) Args
#endif
typedef unsigned (*Hash_hasher) PARAMS ((const void *, unsigned));
typedef bool (*Hash_comparator) PARAMS ((const void *, const void *));
typedef void (*Hash_data_freer) PARAMS ((void *));
typedef bool (*Hash_processor) PARAMS ((void *, void *));

struct hash_entry
  {
    void *data;
    struct hash_entry *next;
  };

struct hash_tuning
  {
    /* This structure is mainly used for `hash_initialize', see the block
       documentation of `hash_reset_tuning' for more complete comments.  */

    float shrink_threshold;	/* ratio of used buckets to trigger a shrink */
    float shrink_factor;	/* ratio of new smaller size to original size */
    float growth_threshold;	/* ratio of used buckets to trigger a growth */
    float growth_factor;	/* ratio of new bigger size to original size */
    bool is_n_buckets;		/* if CANDIDATE really means table size */
  };

typedef struct hash_tuning Hash_tuning;

struct hash_table;

typedef struct hash_table Hash_table;

/* Information and lookup.  */
unsigned hash_get_n_buckets PARAMS ((const Hash_table *));
unsigned hash_get_n_buckets_used PARAMS ((const Hash_table *));
unsigned hash_get_n_entries PARAMS ((const Hash_table *));
unsigned hash_get_max_bucket_length PARAMS ((const Hash_table *));
bool hash_table_ok PARAMS ((const Hash_table *));
void hash_print_statistics PARAMS ((const Hash_table *, void *));
void *hash_lookup PARAMS ((const Hash_table *, const void *));

/* Walking.  */
void *hash_get_first PARAMS ((const Hash_table *));
void *hash_get_next PARAMS ((const Hash_table *, const void *));
unsigned hash_get_entries PARAMS ((const Hash_table *, void **, unsigned));
unsigned hash_do_for_each PARAMS ((const Hash_table *, Hash_processor, void *));

/* Allocation and clean-up.  */
unsigned hash_string PARAMS ((const char *, unsigned));
void hash_reset_tuning PARAMS ((Hash_tuning *));
Hash_table *hash_initialize PARAMS ((unsigned, const Hash_tuning *,
				     Hash_hasher, Hash_comparator,
				     Hash_data_freer));
void hash_clear PARAMS ((Hash_table *));
void hash_free PARAMS ((Hash_table *));

/* Insertion and deletion.  */
bool hash_rehash PARAMS ((Hash_table *, unsigned));
void *hash_insert PARAMS ((Hash_table *, const void *));
void *hash_delete PARAMS ((Hash_table *, const void *));

#undef PARAMS

#endif
