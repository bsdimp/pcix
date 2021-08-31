/*
 * @(#)dumprestor.h	5.4 - 10/17/83
 */

#include <sys/cdefs.h>
/* hack for ILP32 or LP64 based on sys/param.h on disk11.img */
#define time_t unsigned int
#define off_t int
#define daddr_t int
#define ino_t unsigned short
#define dev_t short

#ifndef ibm
/* some useful types */
typedef unsigned char uchar;
#endif

/* max number of blocks in a tape record */
#ifdef  idea
#define NTREC           (4096/BSIZE)    /* drive can't handle that much  */
#else
#define NTREC           (10240/BSIZE)   /* blocking factor - 10240 bytes */
#endif
/*
 *      bitmap parameters.
 *      note: 8K * 8 == 64K, largest possible inumber
 */
#define MLEN             8              /* number of bits per map word   */
#define MSIZ            (8*1024)        /* number of words in map        */

/*
 * macros for accessing bitmaps
 *      MWORD( map, bitno )     returns word containing specified bit
 *      MBIT(i)                 returns mask for specified bit within map word
 *      BIS                     turns on specified bit in map
 *      BIC                     turns off specified bit in map
 *      BIT                     tests specified bit in map
 */
#define	MWORD(m,i) (m[(unsigned)(i-1)/MLEN])
#define	MBIT(i)	(1<<((unsigned)(i-1)%MLEN))
#define	BIS(i,w)	(MWORD(w,i) |=  MBIT(i))
#define	BIC(i,w)	(MWORD(w,i) &= ~MBIT(i))
#define	BIT(i,w)	(MWORD(w,i) & MBIT(i))

/*
 * format of inode dump
 *      FS_VOLUME
 *      FS_CLRI         (if incremental)
 *              list of inodes unallocated at time of dump
 *
 *      FS_BITS         (just before the first FS_INODE header on each vol.)
 *              list of files on this and succeeding volumes
 *
 *      FS_FINDEX
 *              index of files on this volume.  the last file or two
 *              may not be indexed, for space reasons.  the link field
 *              gives the address of the next FS_INDEX on this volume.
 *
 *      FS_INODE        (before each file)
 *      file data
 *      FS_END or FS_VOLEND
 *
 * format of name dump:
 *      FS_VOLUME
 *      FS_NAME         (before each file)
 *      file data
 *      FS_END
 */

#define MAGIC           (int)60011      /* magic number for headers      */
#define CHECKSUM        (int)84446      /* checksum for all headers      */

/*
 * the file /etc/ddates contains a record for the last dump of each file
 *     system at each level.  This file is used to determine how far back
 *     new dumps should extend.  The record format is ...
 */
struct	idates
{
	char	id_name[16];
	char	id_incno;
	time_t	id_ddate;
};

/*
 * header types.  the hdrlen[] array (dump and restor) assumes that
 * the numbers begin at 0, and that they are in this order.
 * the OINODE and ONAME records are not produced by dump, but were
 * produced by older versions, and restore knows how to interpret
 * them.
 */
#define FS_VOLUME        0
#define FS_FINDEX        1
#define FS_CLRI          2
#define FS_BITS          3
#define FS_OINODE        4
#define FS_ONAME         5
#define FS_VOLEND        6
#define FS_END           7
#define FS_INODE         8
#define FS_NAME          9

/* other constants */
#define FXLEN          80       /* length of file index */

/* commands to findex */
#define TBEG    0               /* start indexing */
#define TEND    1               /* end of this track */
#define TINS    2               /* install new inode in index */

struct hdr {                    /* common part of every header */
	uchar   len;            /* hdr length in dwords */
	uchar   type;           /* FS_* */
	ushort  magic;          /* magic number (MAGIC above) */
	ushort  checksum;
} __packed;

/*
 * the addressing unit is 8-byte "words", also known as dwords
 */
#define BPW     8
#define LOGBPW  3

/* bytes to "words" and back; must fit into char */
/* must be even -- so always room for VOLEND record (8 bytes long) */
#define btow(x)   ( ((x) + BPW - 1) >> LOGBPW)
#define wtob(x)   ( (x) << LOGBPW )

long    XXlong();
short   XXshort();
#define rlong(a)    ((a) = XXlong((a),0))
#define wlong(a)    ((a) = XXlong((a),0))
#define rshort(a)   ((a) = XXshort((a),0))
#define wshort(a)   ((a) = XXshort((a),0))

#define SIZHDR    btow(sizeof(struct hdr))
#define DUMNAME   4     /* dummy name length for FS_NAME */
#define SIZSTR   16     /* size of strings in the volume record */

#define min(a,b)  ( ((a) < (b))? (a): (b) )
#define max(a,b)  ( ((a) > (b))? (a): (b) )

#define BYNAME  100             /* must be illegal v.incno */
#define BYINODE 101

#define DONTCOUNT   -1          /* for counting # files we want */

/*
 * the headers follow.  note that there are no places that might
 * tempt a compiler to insert gaps for alignment.  for example,
 * making the FS_FINDEX arrays into an array of (inode, address)
 * structs might later cause trouble.  also, there is code in
 * both dump and restor that reorders the bytes in these headers;
 * this code MUST know about any change in the structures.
 */

	/* FS_VOLUME -- begins each volume */
struct fs_volume {
	struct  hdr h;
	ushort  volnum;         /* volume number */
	time_t  date;           /* current date */
	time_t  ddate;          /* starting date */
	daddr_t numwds;         /* number of wds this volume */
	char    disk[SIZSTR];   /* name of disk */
	char    fsname[SIZSTR]; /* name of file system */
	char    user[SIZSTR];   /* name of user */
	short   incno;          /* dump level (or BYNAME) */
};

union fs_rec {

	/* common fields */
	struct hdr h;

	/* FS_VOLUME -- begins each volume */
	struct fs_volume v;

	/* FS_FINDEX -- indexes files on this volume */
	struct {
		struct  hdr h;
		ushort  dummy;          /* get the alignment right */
		ino_t   ino[FXLEN];     /* inumbers */
		daddr_t addr[FXLEN];    /* addresses */
		daddr_t link;           /* next volume record */
	} x;

	/* FS_BITS or FS_CLRI */
	struct {
		struct hdr h;
		ushort  nwds;           /* number of words of bits */
	} b;

	/* FS_OINODE */
	struct {
		struct hdr h;
		ushort  ino;            /* inumber */
		ushort  mode;           /* info from inode */
		ushort  nlink;
		ushort  uid;
		ushort  gid;
		off_t   size;
		time_t  atime;
		time_t  mtime;
		time_t  ctime;
		dev_t   dev;            /* device file is on */
		dev_t   rdev;           /* maj/min devno */
		off_t   dsize;          /* dump size if packed */
	} oi;

	/* FS_INODE */
	struct {
		struct hdr h;
		ushort  ino;            /* inumber */
		ushort  mode;           /* info from inode */
		ushort  nlink;
		ushort  uid;
		ushort  gid;
		off_t   size;
		time_t  atime;
		time_t  mtime;
		time_t  ctime;
		ushort  devmaj;         /* device file is on */
		ushort  devmin;
		ushort  rdevmaj;        /* maj/min devno */
		ushort  rdevmin;
		off_t   dsize;          /* dump size if packed */
		long    pad;
	} i;

	/* FS_ONAME */
	/* must be exactly like FS_INODE except name at end */
	struct {
		struct hdr h;
		ushort  ino;
		ushort  mode;
		ushort  nlink;
		ushort  uid;
		ushort  gid;
		off_t   size;
		time_t  atime;
		time_t  mtime;
		time_t  ctime;
		dev_t   dev;
		dev_t   rdev;
		off_t   dsize;
		char    name[DUMNAME];  /* file name given by user */
	} on;

	/* FS_NAME */
	/* must be exactly like FS_INODE except name at end */
	struct {
		struct hdr h;
		ushort  ino;
		ushort  mode;
		ushort  nlink;
		ushort  uid;
		ushort  gid;
		off_t   size;
		time_t  atime;
		time_t  mtime;
		time_t  ctime;
		ushort  devmaj;         /* device file is on */
		ushort  devmin;
		ushort  rdevmaj;        /* maj/min devno */
		ushort  rdevmin;
		off_t   dsize;
		long    pad;
		char    name[DUMNAME];  /* file name given by user */
	} n;

	/* FS_END or FS_VOLEND */
	struct {
		struct hdr h;
	} e;
};

/*
 * some defaults in case we don't get explicit instructions
 * These defaults ought to be set in the Makefile.
 */

#ifndef DEF_DISK
#define DEF_DISK "/dev/rp1"     /* default file system to dump              */
#endif

#ifndef DEF_TAPE
#define DEF_TAPE "/dev/rmt0-3"  /* default dump device                      */
#endif

#ifndef DEF_TDEN
#define DEF_TDEN 800             /* default dump tape density (BPI)          */
#endif

#ifndef DEF_TLEN
#define DEF_TLEN 2300            /* default dump tape length (feet)          */
#endif

#ifndef NTRACKS
#define NTRACKS 1               /* default number of tracks per tape unit   */
#endif

#ifndef IRG
#define IRG     8               /* default .75 inch inter record gap        */
#endif

#ifndef DEF_DTRK
#define DEF_DTRK 8              /* default track len in blocks              */
#endif

#ifndef DEF_DTOT
#define DEF_DTOT 600            /* default disk len in blocks               */
#endif

#ifndef DEF_DTYP
#define DEF_DTYP 'M'            /* default device type                      */
#endif                          /* M is tape, R is floppy                   */

#define DEF_LEV '9'             /* default dump level                       */
#define A_WHILE 150             /* period between informative comments      */
