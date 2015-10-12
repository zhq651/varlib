#ifndef MEM_H_SHXYZWFO
#define MEM_H_SHXYZWFO

// parameters to the main Error routine
typedef enum {
    ERR_NONE,
    ERR_FATAL,					// exit the entire game with a popup window
    ERR_DROP,					// print to console and disconnect from game
    ERR_DISCONNECT,				// don't kill server
    ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;



typedef enum {
    TAG_FREE,
    TAG_GENERAL,
    TAG_BOTLIB,
    TAG_RENDERER,
    TAG_SMALL,
    TAG_STATIC
} memtag_t;


void Com_InitSmallZoneMemory(void);
void Com_InitZoneMemory(void);
#ifdef ZONE_DEBUG
#define Z_TagMalloc(size, tag)			Z_TagMallocDebug(size, tag, #size, __FILE__, __LINE__)
#define Z_Malloc(size)					Z_MallocDebug(size, #size, __FILE__, __LINE__)
#define S_Malloc(size)					S_MallocDebug(size, #size, __FILE__, __LINE__)
void *Z_TagMallocDebug( int size, int tag, char *label, char *file, int line );	// NOT 0 filled memory
void *Z_MallocDebug( int size, char *label, char *file, int line );			// returns 0 filled memory
void *S_MallocDebug( int size, char *label, char *file, int line );			// returns 0 filled memory
#else
void *Z_TagMalloc( int size, int tag );	// NOT 0 filled memory
void *Z_Malloc( int size );			// returns 0 filled memory
void *S_Malloc( int size );			// NOT 0 filled memory only for small allocations
#endif
void Z_Free( void *ptr );
void Z_FreeTags( int tag );
int Z_AvailableMemory( void );
void Z_LogHeap( void );
char *CopyString(const char *in);
#endif /* end of include guard: MEM_H_SHXYZWFO */
