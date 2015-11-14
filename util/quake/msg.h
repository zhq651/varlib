
#include "q_shared.h"
//#define	PRE_RELEASE_DEMO
#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))
//============================================================================

//
// msg.c
//
typedef struct {
    qboolean	allowoverflow;	// if false, do a Com_Error
    qboolean	overflowed;		// set to true if the buffer size failed (with allowoverflow set)
    qboolean	oob;			// set to true if the buffer size failed (with allowoverflow set)
    byte	*data;
    int		maxsize;
    int		cursize;
    int		readcount;
    int		bit;				// for bitwise reads and writes
} msg_t;

void MSG_Init(msg_t *buf, byte *data, int length);
void MSG_InitOOB(msg_t *buf, byte *data, int length);
void MSG_Clear(msg_t *buf);
void MSG_WriteData(msg_t *buf, const void *data, int length);
void MSG_Bitstream(msg_t *buf);

// TTimo
// copy a msg_t in case we need to store it as is for a bit
// (as I needed this to keep an msg_t from a static var for later use)
// sets data buffer as MSG_Init does prior to do the copy
void MSG_Copy(msg_t *buf, byte *data, int length, msg_t *src);


void MSG_WriteBits(msg_t *msg, int value, int bits);

void MSG_WriteChar(msg_t *sb, int c);
void MSG_WriteByte(msg_t *sb, int c);
void MSG_WriteShort(msg_t *sb, int c);
void MSG_WriteLong(msg_t *sb, int c);
void MSG_WriteFloat(msg_t *sb, float f);
void MSG_WriteString(msg_t *sb, const char *s);
void MSG_WriteBigString(msg_t *sb, const char *s);
void MSG_WriteAngle16(msg_t *sb, float f);

void	MSG_BeginReading(msg_t *sb);
void	MSG_BeginReadingOOB(msg_t *sb);

int		MSG_ReadBits(msg_t *msg, int bits);

int		MSG_ReadChar(msg_t *sb);
int		MSG_ReadByte(msg_t *sb);
int		MSG_ReadShort(msg_t *sb);
int		MSG_ReadLong(msg_t *sb);
float	MSG_ReadFloat(msg_t *sb);
char	*MSG_ReadString(msg_t *sb);
char	*MSG_ReadBigString(msg_t *sb);
char	*MSG_ReadStringLine(msg_t *sb);
float	MSG_ReadAngle16(msg_t *sb);
void	MSG_ReadData(msg_t *sb, void *buffer, int size);

/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
* Compression book.  The ranks are not actually stored, but implicitly defined
* by the location of a node within a doubly-linked list */

#define NYT HMAX					/* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX+1)

typedef struct nodetype {
    struct	nodetype *left, *right, *parent; /* tree structure */
    struct	nodetype *next, *prev; /* doubly-linked list */
    struct	nodetype **head; /* highest ranked node in block */
    int		weight;
    int		symbol;
} node_t;

#define HMAX 256 /* Maximum symbol */

typedef struct {
    int			blocNode;
    int			blocPtrs;

    node_t*		tree;
    node_t*		lhead;
    node_t*		ltail;
    node_t*		loc[HMAX + 1];
    node_t**	freelist;

    node_t		nodeList[768];
    node_t*		nodePtrs[768];
} huff_t;

typedef struct {
    huff_t		compressor;
    huff_t		decompressor;
} huffman_t;

void	Huff_Compress(msg_t *buf, int offset);
void	Huff_Decompress(msg_t *buf, int offset);
void	Huff_Init(huffman_t *huff);
void	Huff_addRef(huff_t* huff, byte ch);
int		Huff_Receive(node_t *node, int *ch, byte *fin);
void	Huff_transmit(huff_t *huff, int ch, byte *fout);
void	Huff_offsetReceive(node_t *node, int *ch, byte *fin, int *offset);
void	Huff_offsetTransmit(huff_t *huff, int ch, byte *fout, int *offset);
void	Huff_putBit(int bit, byte *fout, int *offset);
int		Huff_getBit(byte *fout, int *offset);

extern huffman_t clientHuffTables;