#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include "mem.h"
#include "cvar.h"
//#pragma warning(disable:4996)
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

#define    ZONEID    0x1d4a11

#define MINFRAGMENT    64
#define MAX_NUM_ARGVS    50

#define MIN_DEDICATED_COMHUNKMEGS 1
#define MIN_COMHUNKMEGS 56
#ifdef MACOS_X
#define DEF_COMHUNKMEGS "64"
#define DEF_COMZONEMEGS "24"
#else
#define DEF_COMHUNKMEGS "56"
#define DEF_COMZONEMEGS "16"
#endif

typedef struct zonedebug_s {
    char *label;
    char *file;
    int line;
    int allocSize;
} zonedebug_t;

typedef struct memblock_s {
    int        size;           // including the header and possibly tiny fragments
    int     tag;            // a tag of 0 is a free block
    struct memblock_s       *next, *prev;
    int     id;                // should be ZONEID
#ifdef ZONE_DEBUG
    zonedebug_t d;
#endif
} memblock_t;

typedef struct {
    int        size;            // total bytes malloced, including header
    int        used;            // total bytes used
    memblock_t    blocklist;    // start / end cap for linked list
    memblock_t    *rover;
} memzone_t;

// main zone for all "dynamic" memory allocation
memzone_t    *mainzone;
// we also have a small zone for small allocations that would only
// fragment the main zone (think of cvar and cmd strings)
memzone_t    *smallzone;


static    int        s_zoneTotal;
static    int        s_smallZoneTotal;


void Z_CheckHeap( void );

//void Error(const char *error, ...)
//{
//    va_list argptr;
//    char    text[1024];
//    char    text2[1024];
//    int        err;
//
//    err = GetLastError();
//
//    va_start(argptr, error);
//    vsprintf_s(text, sizeof(text), error, argptr);
//    va_end(argptr);
//
//    sprintf(text2, "%s\nGetLastError() = %i", text, err);
//    MessageBoxA(NULL, text2, "Error", 0 /* MB_OK */);
//
//    exit(1);
//}

//void Com_Error(int level, char *error, ...)
//{
//    va_list argptr;
//    char text[1024];
//
//    va_start(argptr, error);
//    vsprintf_s(text, sizeof(text), error, argptr);
//    va_end(argptr);
//    Error(text);
//} //end of the funcion Com_Error
/*
========================
Z_ClearZone
========================
*/
void Z_ClearZone( memzone_t *zone, int size ) {
    memblock_t    *block;
    
    // set the entire zone to one free block

    zone->blocklist.next = zone->blocklist.prev = block =
        (memblock_t *)( (byte *)zone + sizeof(memzone_t) );
    zone->blocklist.tag = 1;    // in use block
    zone->blocklist.id = 0;
    zone->blocklist.size = 0;
    zone->rover = block;
    zone->size = size;
    zone->used = 0;
    
    block->prev = block->next = &zone->blocklist;
    block->tag = 0;            // free block
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
    // ���������Լ��޸�
    cvar_t    *cv;
    // allocate the random block zone
    cv = Cvar_Get("com_zoneMegs", DEF_COMZONEMEGS, CVAR_LATCH | CVAR_ARCHIVE);

    if (cv->integer < 20) {
        s_zoneTotal = 1024 * 1024 * 16;
    }
    else {
        s_zoneTotal = cv->integer * 1024 * 1024;
    }

    //s_zoneTotal = 1024 * 1024 * 16;

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
    memblock_t    *block, *other;
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

    block->tag = 0;        // mark as free
    
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
    int            count;
    memzone_t    *zone;

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
    int        extra, allocSize;
    memblock_t    *start, *rover, *new_, *base;
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
    size += sizeof(memblock_t);    // account for size of block header
    size += 4;                    // space for memory trash tester
    size = (size + 3) & ~3;        // align to 32 bit boundary
    
    base = rover = zone->rover;
    start = base->prev;
    
    do {
        if (rover == start)    {
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
        new_ = (memblock_t *) ((byte *)base + size );
        new_->size = extra;
        new_->tag = 0;            // free block
        new_->prev = base;
        new_->id = ZONEID;
        new_->next = base->next;
        new_->next->prev = new_;
        base->next = new_;
        base->size = size;
    }
    
    base->tag = tag;            // no longer a free block
    
    zone->rover = base->next;    // next allocation will start looking here
    zone->used += base->size;    //
    
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
    void    *buf;
    
  //Z_CheckHeap ();    // DEBUG

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
    memblock_t    *block;
    
    for (block = mainzone->blocklist.next ; ; block = block->next) {
        if (block->next == &mainzone->blocklist) {
            break;            // all blocks have been hit
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
//    char dump[32], *ptr;
//    int  i, j;
//#endif
//    memblock_t    *block;
//    char        buf[4096];
//    int size, allocSize, numBlocks;
//
//    if (!logfile || !FS_Initialized())
//        return;
//    size = allocSize = numBlocks = 0;
//    Com_sprintf(buf, sizeof(buf), "\r\n================\r\n%s log\r\n================\r\n", name);
//    FS_Write(buf, strlen(buf), logfile);
//    for (block = zone->blocklist.next ; block->next != &zone->blocklist; block = block->next) {
//        if (block->tag) {
//#ifdef ZONE_DEBUG
//            ptr = ((char *) block) + sizeof(memblock_t);
//            j = 0;
//            for (i = 0; i < 20 && i < block->d.allocSize; i++) {
//                if (ptr[i] >= 32 && ptr[i] < 127) {
//                    dump[j++] = ptr[i];
//                }
//                else {
//                    dump[j++] = '_';
//                }
//            }
//            dump[j] = '\0';
//            Com_sprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s) [%s]\r\n", block->d.allocSize, block->d.file, block->d.line, block->d.label, dump);
//            FS_Write(buf, strlen(buf), logfile);
//            allocSize += block->d.allocSize;
//#endif
//            size += block->size;
//            numBlocks++;
//        }
//    }
//#ifdef ZONE_DEBUG
//    // subtract debug memory
//    size -= numBlocks * sizeof(zonedebug_t);
//#else
//    allocSize = numBlocks * sizeof(memblock_t); // + 32 bit alignment
//#endif
//    Com_sprintf(buf, sizeof(buf), "%d %s memory in %d blocks\r\n", size, name, numBlocks);
//    FS_Write(buf, strlen(buf), logfile);
//    Com_sprintf(buf, sizeof(buf), "%d %s memory overhead\r\n", size - allocSize, name);
//    FS_Write(buf, strlen(buf), logfile);
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

NOTE:    never write over the memory CopyString returns because
memory from a memstatic_t might be returned
========================
*/
char *CopyString(const char *in) {
    char    *out;

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





//
/*
==============================================================================
Ϊ�˳�������ڴ棬��һ����ڴ�ֳ�low��high�ˣ�����ͷͬʱ��������,
low����high������Ϊtemp��ʱ�ڴ���������Ϊpermanent�ڴ�ʹ�ã���ͬһʱ��
����ֻ����Ϊһ����ɫ���������Ƕ����ġ�

������temp�ڴ�������㣬��ôlow��hight�Ľ�ɫ���ܻ�������lowԭ��Ϊtemp���ܾͻ�ת��
Ϊpermanent

* temp �ڴ���ѭ���������ͷ�ԭ�򣬼�FILO

Goals:
reproducable without history effects -- no out of memory errors on weird map to map changes
allow restarting of the client without fragmentation
minimize total pages in use at run time
minimize total pages needed during load time

Single block of memory with stack allocators coming from both ends towards the middle.

One side is designated the temporary memory allocator.

Temporary memory can be allocated and freed in any order.

A highwater mark is kept of the most in use at any time.

When there is no temporary memory allocated, the permanent and temp sides
can be switched, allowing the already touched temp memory to be used for
permanent storage.

Temp memory must never be allocated on two ends at once, or fragmentation
could occur.

If we have any in-use temp memory, additional temp allocations must come from
that side.

If not, we can choose to make either side the new temp side and push future
permanent allocations to the other side.  Permanent allocations should be
kept on the side that has the current greatest wasted highwater mark.

==============================================================================
*/


#define    HUNK_MAGIC    0x89537892
#define    HUNK_FREE_MAGIC    0x89537893

typedef struct {
    int        magic;
    int        size;
} hunkHeader_t;

typedef struct {
    int        mark;
    int        permanent;
    int        temp;
    int        tempHighwater;
} hunkUsed_t;

typedef struct hunkblock_s {
    int size;
    byte printed;
    struct hunkblock_s *next;
    char *label;
    char *file;
    int line;
} hunkblock_t;

static    hunkblock_t *hunkblocks;

static    hunkUsed_t    hunk_low, hunk_high;
static    hunkUsed_t    *hunk_permanent, *hunk_temp;

static    byte    *s_hunkData = NULL;
static    int        s_hunkTotal;

static    int        s_zoneTotal;
static    int        s_smallZoneTotal;
cvar_t    *com_dedicated;
/*
=================
Hunk_Log
=================
*/
//void Hunk_Log(void) {
//    hunkblock_t    *block;
//    char        buf[4096];
//    int size, numBlocks;
//
//    if (!logfile || !FS_Initialized())
//        return;
//    size = 0;
//    numBlocks = 0;
//    Com_sprintf(buf, sizeof(buf), "\r\n================\r\nHunk log\r\n================\r\n");
//    FS_Write(buf, strlen(buf), logfile);
//    for (block = hunkblocks; block; block = block->next) {
//#ifdef HUNK_DEBUG
//        Com_sprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s)\r\n", block->size, block->file, block->line, block->label);
//        FS_Write(buf, strlen(buf), logfile);
//#endif
//        size += block->size;
//        numBlocks++;
//    }
//    Com_sprintf(buf, sizeof(buf), "%d Hunk memory\r\n", size);
//    FS_Write(buf, strlen(buf), logfile);
//    Com_sprintf(buf, sizeof(buf), "%d hunk blocks\r\n", numBlocks);
//    FS_Write(buf, strlen(buf), logfile);
//}

/*
=================
Hunk_SmallLog
=================
*/
//void Hunk_SmallLog(void) {
//    hunkblock_t    *block, *block2;
//    char        buf[4096];
//    int size, locsize, numBlocks;
//
//    if (!logfile || !FS_Initialized())
//        return;
//    for (block = hunkblocks; block; block = block->next) {
//        block->printed = qfalse;
//    }
//    size = 0;
//    numBlocks = 0;
//    Com_sprintf(buf, sizeof(buf), "\r\n================\r\nHunk Small log\r\n================\r\n");
//    FS_Write(buf, strlen(buf), logfile);
//    for (block = hunkblocks; block; block = block->next) {
//        if (block->printed) {
//            continue;
//        }
//        locsize = block->size;
//        for (block2 = block->next; block2; block2 = block2->next) {
//            if (block->line != block2->line) {
//                continue;
//            }
//            if (Q_stricmp(block->file, block2->file)) {
//                continue;
//            }
//            size += block2->size;
//            locsize += block2->size;
//            block2->printed = qtrue;
//        }
//#ifdef HUNK_DEBUG
//        Com_sprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s)\r\n", locsize, block->file, block->line, block->label);
//        FS_Write(buf, strlen(buf), logfile);
//#endif
//        size += block->size;
//        numBlocks++;
//    }
//    Com_sprintf(buf, sizeof(buf), "%d Hunk memory\r\n", size);
//    FS_Write(buf, strlen(buf), logfile);
//    Com_sprintf(buf, sizeof(buf), "%d hunk blocks\r\n", numBlocks);
//    FS_Write(buf, strlen(buf), logfile);
//}

/*
=================
Com_InitZoneMemory
=================
*/
void Com_InitHunkMemory(void) {
    cvar_t    *cv;
    int nMinAlloc;
    char *pMsg = NULL;

    // make sure the file system has allocated and "not" freed any temp blocks
    // this allows the config and product id files ( journal files too ) to be loaded
    // by the file system without redunant routines in the file system utilizing different 
    // memory systems
    //if (FS_LoadStack() != 0) {
    //    Com_Error(ERR_FATAL, "Hunk initialization failed. File system load stack not zero");
    //}

    // allocate the stack based hunk allocator
    cv = Cvar_Get("com_hunkMegs", DEF_COMHUNKMEGS, CVAR_LATCH | CVAR_ARCHIVE);

    // if we are not dedicated min allocation is 56, otherwise min is 1
    if (com_dedicated && com_dedicated->integer) {
        nMinAlloc = MIN_DEDICATED_COMHUNKMEGS;
        pMsg = "Minimum com_hunkMegs for a dedicated server is %i, allocating %i megs.\n";
    }
    else {
        nMinAlloc = MIN_COMHUNKMEGS;
        pMsg = "Minimum com_hunkMegs is %i, allocating %i megs.\n";
    }

    if (cv->integer < nMinAlloc) {
        s_hunkTotal = 1024 * 1024 * nMinAlloc;
        Com_Printf(pMsg, nMinAlloc, s_hunkTotal / (1024 * 1024));
    }
    else {
        s_hunkTotal = cv->integer * 1024 * 1024;
    }


    // bk001205 - was malloc
    s_hunkData = calloc(s_hunkTotal + 31, 1);
    if (!s_hunkData) {
        Com_Error(ERR_FATAL, "Hunk data failed to allocate %i megs", s_hunkTotal / (1024 * 1024));
    }
    // cacheline align
    s_hunkData = (byte *)(((int)s_hunkData + 31) & ~31);
    Hunk_Clear();

    //Cmd_AddCommand("meminfo", Com_Meminfo_f);
#ifdef ZONE_DEBUG
    Cmd_AddCommand("zonelog", Z_LogHeap);
#endif
#ifdef HUNK_DEBUG
    Cmd_AddCommand("hunklog", Hunk_Log);
    Cmd_AddCommand("hunksmalllog", Hunk_SmallLog);
#endif
}

/*
====================
Hunk_MemoryRemaining
====================
*/
int    Hunk_MemoryRemaining(void) {
    int        low, high;

    low = hunk_low.permanent > hunk_low.temp ? hunk_low.permanent : hunk_low.temp;
    high = hunk_high.permanent > hunk_high.temp ? hunk_high.permanent : hunk_high.temp;

    return s_hunkTotal - (low + high);
}

/*
===================
Hunk_SetMark

The server calls this after the level and game VM have been loaded
===================
*/
void Hunk_SetMark(void) {
    hunk_low.mark = hunk_low.permanent;
    hunk_high.mark = hunk_high.permanent;
}

/*
=================
Hunk_ClearToMark

The client calls this before starting a vid_restart or snd_restart
=================
*/
void Hunk_ClearToMark(void) {
    hunk_low.permanent = hunk_low.temp = hunk_low.mark;
    hunk_high.permanent = hunk_high.temp = hunk_high.mark;
}

/*
=================
Hunk_CheckMark
=================
*/
qboolean Hunk_CheckMark(void) {
    if (hunk_low.mark || hunk_high.mark) {
        return qtrue;
    }
    return qfalse;
}

/*
=================
Hunk_Clear

The server calls this before shutting down or loading a new map
=================
*/
void Hunk_Clear(void) {

    hunk_low.mark = 0;
    hunk_low.permanent = 0;
    hunk_low.temp = 0;
    hunk_low.tempHighwater = 0;

    hunk_high.mark = 0;
    hunk_high.permanent = 0;
    hunk_high.temp = 0;
    hunk_high.tempHighwater = 0;

    hunk_permanent = &hunk_low;
    hunk_temp = &hunk_high;

    Com_Printf("Hunk_Clear: reset the hunk ok\n");
    //VM_Clear();
#ifdef HUNK_DEBUG
    hunkblocks = NULL;
#endif
}

static void Hunk_SwapBanks(void) {
    hunkUsed_t    *swap;

    // can't swap banks if there is any temp already allocated
    if (hunk_temp->temp != hunk_temp->permanent) {
        return;
    }

    // if we have a larger highwater mark on this side, start making
    // our permanent allocations here and use the other side for temp
    if (hunk_temp->tempHighwater - hunk_temp->permanent >
        hunk_permanent->tempHighwater - hunk_permanent->permanent) {
        swap = hunk_temp;
        hunk_temp = hunk_permanent;
        hunk_permanent = swap;
    }
}

/*
=================
Hunk_Alloc

Allocate permanent (until the hunk is cleared) memory
=================
*/
#ifdef HUNK_DEBUG
void *Hunk_AllocDebug(int size, ha_pref preference, char *label, char *file, int line) {
#else
void *Hunk_Alloc(int size, ha_pref preference) {
#endif
    void    *buf;

    if (s_hunkData == NULL)
    {
        Com_Error(ERR_FATAL, "Hunk_Alloc: Hunk memory system not initialized");
    }

    // can't do preference if there is any temp allocated
    if (preference == h_dontcare || hunk_temp->temp != hunk_temp->permanent) {
        Hunk_SwapBanks();
    }
    else {
        if (preference == h_low && hunk_permanent != &hunk_low) {
            Hunk_SwapBanks();
        }
        else if (preference == h_high && hunk_permanent != &hunk_high) {
            Hunk_SwapBanks();
        }
    }

#ifdef HUNK_DEBUG
    size += sizeof(hunkblock_t);
#endif

    // round to cacheline
    size = (size + 31)&~31;

    if (hunk_low.temp + hunk_high.temp + size > s_hunkTotal) {
#ifdef HUNK_DEBUG
        Hunk_Log();
        Hunk_SmallLog();
#endif
        Com_Error(ERR_DROP, "Hunk_Alloc failed on %i", size);
    }

    if (hunk_permanent == &hunk_low) {
        buf = (void *)(s_hunkData + hunk_permanent->permanent);
        hunk_permanent->permanent += size;
    }
    else {
        hunk_permanent->permanent += size;
        buf = (void *)(s_hunkData + s_hunkTotal - hunk_permanent->permanent);
    }

    hunk_permanent->temp = hunk_permanent->permanent;

    Com_Memset(buf, 0, size);

#ifdef HUNK_DEBUG
    {
        hunkblock_t *block;

        block = (hunkblock_t *)buf;
        block->size = size - sizeof(hunkblock_t);
        block->file = file;
        block->label = label;
        block->line = line;
        block->next = hunkblocks;
        hunkblocks = block;
        buf = ((byte *)buf) + sizeof(hunkblock_t);
    }
#endif
    return buf;
}

/*
=================
Hunk_AllocateTempMemory

This is used by the file loading system.
Multiple files can be loaded in temporary memory.
When the files-in-use count reaches zero, all temp memory will be deleted
=================
*/
void *Hunk_AllocateTempMemory(int size) {
    void        *buf;
    hunkHeader_t    *hdr;

    // return a Z_Malloc'd block if the hunk has not been initialized
    // this allows the config and product id files ( journal files too ) to be loaded
    // by the file system without redunant routines in the file system utilizing different 
    // memory systems
    if (s_hunkData == NULL)
    {
        return Z_Malloc(size);
    }

    Hunk_SwapBanks();

    size = ((size + 3)&~3) + sizeof(hunkHeader_t);

    if (hunk_temp->temp + hunk_permanent->permanent + size > s_hunkTotal) {
        Com_Error(ERR_DROP, "Hunk_AllocateTempMemory: failed on %i", size);
    }

    if (hunk_temp == &hunk_low) {
        buf = (void *)(s_hunkData + hunk_temp->temp);
        hunk_temp->temp += size;
    }
    else {
        hunk_temp->temp += size;
        buf = (void *)(s_hunkData + s_hunkTotal - hunk_temp->temp);
    }

    if (hunk_temp->temp > hunk_temp->tempHighwater) {
        hunk_temp->tempHighwater = hunk_temp->temp;
    }

    hdr = (hunkHeader_t *)buf;
    buf = (void *)(hdr + 1);

    hdr->magic = HUNK_MAGIC;
    hdr->size = size;

    // don't bother clearing, because we are going to load a file over it
    return buf;
}



/*
==================
Hunk_FreeTempMemory
==================
*/
void Hunk_FreeTempMemory(void *buf) {
    hunkHeader_t    *hdr;

    // free with Z_Free if the hunk has not been initialized
    // this allows the config and product id files ( journal files too ) to be loaded
    // by the file system without redunant routines in the file system utilizing different 
    // memory systems
    if (s_hunkData == NULL)
    {
        Z_Free(buf);
        return;
    }


    hdr = ((hunkHeader_t *)buf) - 1;
    if (hdr->magic != HUNK_MAGIC) {
        Com_Error(ERR_FATAL, "Hunk_FreeTempMemory: bad magic");
    }

    hdr->magic = HUNK_FREE_MAGIC;

    // this only works if the files are freed in stack order,
    // otherwise the memory will stay around until Hunk_ClearTempMemory
    if (hunk_temp == &hunk_low) {
        if (hdr == (void *)(s_hunkData + hunk_temp->temp - hdr->size)) {
            hunk_temp->temp -= hdr->size;
        }
        else {
            Com_Printf("Hunk_FreeTempMemory: not the final block\n");
        }
    }
    else {
        if (hdr == (void *)(s_hunkData + s_hunkTotal - hunk_temp->temp)) {
            hunk_temp->temp -= hdr->size;
        }
        else {
            Com_Printf("Hunk_FreeTempMemory: not the final block\n");
        }
    }
}


/*
=================
Hunk_ClearTempMemory

The temp space is no longer needed.  If we have left more
touched but unused memory on this side, have future
permanent allocs use this side.
=================
*/
void Hunk_ClearTempMemory(void) {
    if (s_hunkData != NULL) {
        hunk_temp->temp = hunk_temp->permanent;
    }
}

/*
=================
Hunk_Trash
=================
*/
void Hunk_Trash(void) {
    int length, i, rnd;
    char *buf, value;

    return;

    if (s_hunkData == NULL)
        return;

#ifdef _DEBUG
    Com_Error(ERR_DROP, "hunk trashed\n");
    return;
#endif

    Cvar_Set("com_jp", "1");
    Hunk_SwapBanks();

    if (hunk_permanent == &hunk_low) {
        buf = (void *)(s_hunkData + hunk_permanent->permanent);
    }
    else {
        buf = (void *)(s_hunkData + s_hunkTotal - hunk_permanent->permanent);
    }
    length = hunk_permanent->permanent;

    if (length > 0x7FFFF) {
        //randomly trash data within buf
        rnd = random() * (length - 0x7FFFF);
        value = 31;
        for (i = 0; i < 0x7FFFF; i++) {
            value *= 109;
            buf[rnd + i] ^= value;
        }
    }
}