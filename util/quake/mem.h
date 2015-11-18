#ifndef MEM_H_SHXYZWFO
#define MEM_H_SHXYZWFO
#include "q_shared.h"
// parameters to the main Error routine
#ifndef ERRORPARM_T_M

typedef enum {
    ERR_NONE,
    ERR_FATAL,                    // exit the entire game with a popup window
    ERR_DROP,                    // print to console and disconnect from game
    ERR_SERVERDISCONNECT,        // don't kill server
    ERR_DISCONNECT,                // client disconnected from the server
    ERR_NEED_CD                    // pop up the need-cd dialog
} errorParm_t;
#define ERRORPARM_T_M
#endif // !ERRORPARM_T_M


#ifndef MEMTAG_T_M
typedef enum {
    TAG_FREE,
    TAG_GENERAL,
    TAG_BOTLIB,
    TAG_RENDERER,
    TAG_SMALL,
    TAG_STATIC
} memtag_t;
#define MEMTAG_T_M
#endif


void Com_InitSmallZoneMemory(void);
void Com_InitZoneMemory(void);
#ifdef ZONE_DEBUG
#define Z_TagMalloc(size, tag)            Z_TagMallocDebug(size, tag, #size, __FILE__, __LINE__)
#define Z_Malloc(size)                    Z_MallocDebug(size, #size, __FILE__, __LINE__)
#define S_Malloc(size)                    S_MallocDebug(size, #size, __FILE__, __LINE__)
void *Z_TagMallocDebug( int size, int tag, char *label, char *file, int line );    // NOT 0 filled memory
void *Z_MallocDebug( int size, char *label, char *file, int line );            // returns 0 filled memory
void *S_MallocDebug( int size, char *label, char *file, int line );            // returns 0 filled memory
#else
void *Z_TagMalloc( int size, int tag );    // NOT 0 filled memory
void *Z_Malloc( int size );            // returns 0 filled memory
void *S_Malloc( int size );            // NOT 0 filled memory only for small allocations
#endif
void Z_Free( void *ptr );
void Z_FreeTags( int tag );
int Z_AvailableMemory( void );
void Z_LogHeap( void );
char *CopyString(const char *in);

void Com_InitHunkMemory(void);
void Hunk_Clear(void);
void Hunk_ClearToMark(void);
void Hunk_SetMark(void);
qboolean Hunk_CheckMark(void);
void Hunk_ClearTempMemory(void);
void *Hunk_AllocateTempMemory(int size);
void Hunk_FreeTempMemory(void *buf);
int    Hunk_MemoryRemaining(void);
void Hunk_Log(void);
void Hunk_Trash(void);

#endif /* end of include guard: MEM_H_SHXYZWFO */
