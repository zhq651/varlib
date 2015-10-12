#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include "mem.h"
#pragma warning(disable:4996)
/*
==============================================================================

						ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/

#define	ZONEID	0x1d4a11
#define MINFRAGMENT	64

typedef struct zonedebug_s {
	char *label;
	char *file;
	int line;
	int allocSize;
} zonedebug_t;

typedef struct memblock_s {
	int		size;           // including the header and possibly tiny fragments
	int     tag;            // a tag of 0 is a free block
	struct memblock_s       *next, *prev;
	int     id;        		// should be ZONEID
#ifdef ZONE_DEBUG
	zonedebug_t d;
#endif
} memblock_t;

typedef struct {
	int		size;			// total bytes malloced, including header
	int		used;			// total bytes used
	memblock_t	blocklist;	// start / end cap for linked list
	memblock_t	*rover;
} memzone_t;

// main zone for all "dynamic" memory allocation
memzone_t	*mainzone;
// we also have a small zone for small allocations that would only
// fragment the main zone (think of cvar and cmd strings)
memzone_t	*smallzone;


static	int		s_zoneTotal;
static	int		s_smallZoneTotal;


void Z_CheckHeap( void );

void Error(const char *error, ...)
{
    va_list argptr;
    char	text[1024];
    char	text2[1024];
    int		err;

    err = GetLastError();

    va_start(argptr, error);
    vsprintf_s(text, sizeof(text), error, argptr);
    va_end(argptr);

    sprintf(text2, "%s\nGetLastError() = %i", text, err);
    MessageBoxA(NULL, text2, "Error", 0 /* MB_OK */);

    exit(1);
}

void Com_Error(int level, char *error, ...)
{
    va_list argptr;
    char text[1024];

    va_start(argptr, error);
    vsprintf_s(text, sizeof(text), error, argptr);
    va_end(argptr);
    Error(text);
} //end of the funcion Com_Error
/*
========================
Z_ClearZone
========================
*/
void Z_ClearZone( memzone_t *zone, int size ) {
	memblock_t	*block;
	
	// set the entire zone to one free block

	zone->blocklist.next = zone->blocklist.prev = block =
		(memblock_t *)( (byte *)zone + sizeof(memzone_t) );
	zone->blocklist.tag = 1;	// in use block
	zone->blocklist.id = 0;
	zone->blocklist.size = 0;
	zone->rover = block;
	zone->size = size;
	zone->used = 0;
	
	block->prev = block->next = &zone->blocklist;
	block->tag = 0;			// free block
	block->id = ZONEID;
	block->size = size - sizeof(memzone_t);
}




/*
=================
Com_InitZoneMemory
=================
*/
void Com_InitSmallZoneMemory(void) {
    s_smallZoneTotal = 512 * 1024;
    // bk001205 - was malloc
    smallzone = calloc(s_smallZoneTotal, 1);
    if (!smallzone) {
        Com_Error(ERR_FATAL, "Small zone data failed to allocate %1.1f megs", (float)s_smallZoneTotal / (1024 * 1024));
    }
    Z_ClearZone(smallzone, s_smallZoneTotal);

    return;
}

void Com_InitZoneMemory(void) {
    // 根据需求自己修改
    //cvar_t	*cv;
    //// allocate the random block zone
    //cv = Cvar_Get("com_zoneMegs", DEF_COMZONEMEGS, CVAR_LATCH | CVAR_ARCHIVE);

    //if (cv->integer < 20) {
    //    s_zoneTotal = 1024 * 1024 * 16;
    //}
    //else {
    //    s_zoneTotal = cv->integer * 1024 * 1024;
    //}

    s_zoneTotal = 1024 * 1024 * 16;

    // bk001205 - was malloc
    mainzone = calloc(s_zoneTotal, 1);
    if (!mainzone) {
        Com_Error(ERR_FATAL, "Zone data failed to allocate %i megs", s_zoneTotal / (1024 * 1024));
    }
    Z_ClearZone(mainzone, s_zoneTotal);

}

/*
========================
Z_AvailableZoneMemory
========================
*/
int Z_AvailableZoneMemory( memzone_t *zone ) {
	return zone->size - zone->used;
}

/*
========================
Z_AvailableMemory
========================
*/
int Z_AvailableMemory( void ) {
	return Z_AvailableZoneMemory( mainzone );
}

/*
========================
Z_Free
========================
*/
void Z_Free( void *ptr ) {
	memblock_t	*block, *other;
	memzone_t *zone;
	
	if (!ptr) {
		Com_Error( ERR_DROP, "Z_Free: NULL pointer" );
	}

	block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
	if (block->id != ZONEID) {
		Com_Error( ERR_FATAL, "Z_Free: freed a pointer without ZONEID" );
	}
	if (block->tag == 0) {
		Com_Error( ERR_FATAL, "Z_Free: freed a freed pointer" );
	}
	// if static memory
	if (block->tag == TAG_STATIC) {
		return;
	}

	// check the memory trash tester
	if ( *(int *)((byte *)block + block->size - 4 ) != ZONEID ) {
		Com_Error( ERR_FATAL, "Z_Free: memory block wrote past end" );
	}

	if (block->tag == TAG_SMALL) {
		zone = smallzone;
	}
	else {
		zone = mainzone;
	}

	zone->used -= block->size;
	// set the block to something that should cause problems
	// if it is referenced...
	memset( ptr, 0xaa, block->size - sizeof( *block ) );

	block->tag = 0;		// mark as free
	
	other = block->prev;
	if (!other->tag) {
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if (block == zone->rover) {
			zone->rover = other;
		}
		block = other;
	}

	zone->rover = block;

	other = block->next;
	if ( !other->tag ) {
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
		if (other == zone->rover) {
			zone->rover = block;
		}
	}
}


/*
================
Z_FreeTags
================
*/
void Z_FreeTags( int tag ) {
	int			count;
	memzone_t	*zone;

	if ( tag == TAG_SMALL ) {
		zone = smallzone;
	}
	else {
		zone = mainzone;
	}
	count = 0;
	// use the rover as our pointer, because
	// Z_Free automatically adjusts it
	zone->rover = zone->blocklist.next;
	do {
		if ( zone->rover->tag == tag ) {
			count++;
			Z_Free( (void *)(zone->rover + 1) );
			continue;
		}
		zone->rover = zone->rover->next;
	} while ( zone->rover != &zone->blocklist );
}


/*
================
Z_TagMalloc
================
*/
#ifdef ZONE_DEBUG
void *Z_TagMallocDebug( int size, int tag, char *label, char *file, int line ) {
#else
void *Z_TagMalloc( int size, int tag ) {
#endif
	int		extra, allocSize;
	memblock_t	*start, *rover, *new, *base;
	memzone_t *zone;

	if (!tag) {
		Com_Error( ERR_FATAL, "Z_TagMalloc: tried to use a 0 tag" );
	}

	if ( tag == TAG_SMALL ) {
		zone = smallzone;
	}
	else {
		zone = mainzone;
	}

	allocSize = size;
	//
	// scan through the block list looking for the first free block
	// of sufficient size
	//
	size += sizeof(memblock_t);	// account for size of block header
	size += 4;					// space for memory trash tester
	size = (size + 3) & ~3;		// align to 32 bit boundary
	
	base = rover = zone->rover;
	start = base->prev;
	
	do {
		if (rover == start)	{
#ifdef ZONE_DEBUG
			Z_LogHeap();
#endif
			// scaned all the way around the list
			Com_Error( ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone",
								size, zone == smallzone ? "small" : "main");
			return NULL;
		}
		if (rover->tag) {
			base = rover = rover->next;
		} else {
			rover = rover->next;
		}
	} while (base->tag || base->size < size);
	
	//
	// found a block big enough
	//
	extra = base->size - size;
	if (extra > MINFRAGMENT) {
		// there will be a free fragment after the allocated block
		new = (memblock_t *) ((byte *)base + size );
		new->size = extra;
		new->tag = 0;			// free block
		new->prev = base;
		new->id = ZONEID;
		new->next = base->next;
		new->next->prev = new;
		base->next = new;
		base->size = size;
	}
	
	base->tag = tag;			// no longer a free block
	
	zone->rover = base->next;	// next allocation will start looking here
	zone->used += base->size;	//
	
	base->id = ZONEID;

#ifdef ZONE_DEBUG
	base->d.label = label;
	base->d.file = file;
	base->d.line = line;
	base->d.allocSize = allocSize;
#endif

	// marker for memory trash testing
	*(int *)((byte *)base + base->size - 4) = ZONEID;

	return (void *) ((byte *)base + sizeof(memblock_t));
}

/*
========================
Z_Malloc
========================
*/
#ifdef ZONE_DEBUG
void *Z_MallocDebug( int size, char *label, char *file, int line ) {
#else
void *Z_Malloc( int size ) {
#endif
	void	*buf;
	
  //Z_CheckHeap ();	// DEBUG

#ifdef ZONE_DEBUG
	buf = Z_TagMallocDebug( size, TAG_GENERAL, label, file, line );
#else
	buf = Z_TagMalloc( size, TAG_GENERAL );
#endif
	memset( buf, 0, size );

	return buf;
}

#ifdef ZONE_DEBUG
void *S_MallocDebug( int size, char *label, char *file, int line ) {
	return Z_TagMallocDebug( size, TAG_SMALL, label, file, line );
}
#else
void *S_Malloc( int size ) {
	return Z_TagMalloc( size, TAG_SMALL );
}
#endif

/*
========================
Z_CheckHeap
========================
*/
void Z_CheckHeap( void ) {
	memblock_t	*block;
	
	for (block = mainzone->blocklist.next ; ; block = block->next) {
		if (block->next == &mainzone->blocklist) {
			break;			// all blocks have been hit
		}
		if ( (byte *)block + block->size != (byte *)block->next)
			Com_Error( ERR_FATAL, "Z_CheckHeap: block size does not touch the next block\n" );
		if ( block->next->prev != block) {
			Com_Error( ERR_FATAL, "Z_CheckHeap: next block doesn't have proper back link\n" );
		}
		if ( !block->tag && !block->next->tag ) {
			Com_Error( ERR_FATAL, "Z_CheckHeap: two consecutive free blocks\n" );
		}
	}
}

/*
========================
Z_LogZoneHeap
========================
*/
//void Z_LogZoneHeap( memzone_t *zone, char *name ) {
//#ifdef ZONE_DEBUG
//	char dump[32], *ptr;
//	int  i, j;
//#endif
//	memblock_t	*block;
//	char		buf[4096];
//	int size, allocSize, numBlocks;
//
//	if (!logfile || !FS_Initialized())
//		return;
//	size = allocSize = numBlocks = 0;
//	Com_sprintf(buf, sizeof(buf), "\r\n================\r\n%s log\r\n================\r\n", name);
//	FS_Write(buf, strlen(buf), logfile);
//	for (block = zone->blocklist.next ; block->next != &zone->blocklist; block = block->next) {
//		if (block->tag) {
//#ifdef ZONE_DEBUG
//			ptr = ((char *) block) + sizeof(memblock_t);
//			j = 0;
//			for (i = 0; i < 20 && i < block->d.allocSize; i++) {
//				if (ptr[i] >= 32 && ptr[i] < 127) {
//					dump[j++] = ptr[i];
//				}
//				else {
//					dump[j++] = '_';
//				}
//			}
//			dump[j] = '\0';
//			Com_sprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s) [%s]\r\n", block->d.allocSize, block->d.file, block->d.line, block->d.label, dump);
//			FS_Write(buf, strlen(buf), logfile);
//			allocSize += block->d.allocSize;
//#endif
//			size += block->size;
//			numBlocks++;
//		}
//	}
//#ifdef ZONE_DEBUG
//	// subtract debug memory
//	size -= numBlocks * sizeof(zonedebug_t);
//#else
//	allocSize = numBlocks * sizeof(memblock_t); // + 32 bit alignment
//#endif
//	Com_sprintf(buf, sizeof(buf), "%d %s memory in %d blocks\r\n", size, name, numBlocks);
//	FS_Write(buf, strlen(buf), logfile);
//	Com_sprintf(buf, sizeof(buf), "%d %s memory overhead\r\n", size - allocSize, name);
//	FS_Write(buf, strlen(buf), logfile);
//}

/*
========================
Z_LogHeap
========================
*/
void Z_LogHeap( void ) {
	//Z_LogZoneHeap( mainzone, "MAIN" );
	//Z_LogZoneHeap( smallzone, "SMALL" );
}

// static mem blocks to reduce a lot of small zone overhead
typedef struct memstatic_s {
	memblock_t b;
	byte mem[2];
} memstatic_t;

// bk001204 - initializer brackets
memstatic_t emptystring =
	{ {(sizeof(memblock_t)+2 + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'\0', '\0'} };
memstatic_t numberstring[] = {
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'0', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'1', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'2', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'3', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'4', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'5', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'6', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'7', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'8', '\0'} }, 
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, NULL, NULL, ZONEID}, {'9', '\0'} }
};


/*
========================
CopyString

NOTE:	never write over the memory CopyString returns because
memory from a memstatic_t might be returned
========================
*/
char *CopyString(const char *in) {
    char	*out;

    if (!in[0]) {
        return ((char *)&emptystring) + sizeof(memblock_t);
    }
    else if (!in[1]) {
        if (in[0] >= '0' && in[0] <= '9') {
            return ((char *)&numberstring[in[0] - '0']) + sizeof(memblock_t);
        }
    }
    out = S_Malloc(strlen(in) + 1);
    strcpy(out, in);
    return out;
}