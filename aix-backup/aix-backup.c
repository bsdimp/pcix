#include <sys/types.h>
#include <stdio.h>
#include <err.h>

#include "aix-backup.h"

const char *types[] = {
	"VOLUME",
	"FINDEX",
	"CLRI",
	"BITS",
	"OINODE",
	"ONAME",
	"VOLEND",
	"END",
	"INODE",
	"NAME"
};
	
/* Holds the contents of a volume -- since we're reading floppies this isn't a big deal */
struct volume 
{
	struct fs_volume	*vol;		/* Volume header */
	size_t			buflen;
	uint8_t			*buffer;
};

	

void
aix_scan(char **parts)
{
	union fs_rec buffer;
	FILE *fp;
	int v;

	v = 0;
	while (*parts) {
		fn = *parts++;
		fp = fopen(fn, "r");
		if (fp == NULL)
			err(1, "fopen %s", fn);
		if (fread(&buffer, sizeof(struct fs_volume), 1, fp) != 1)
			err(1, "Can't read header %s", fn);
		vol = malloc(sizeof(struct volume));
		if (vol == NULL)
			errx(1, "No memory");
	}
	fp = fopen(*parts++, "r");
	if (fp == NULL)
		perror("fopen");

#if 0
	/* Hacky dump what's on the tape. */
	do {
		if (fread(&buffer.h, BPW, 1, fp) != 1)
			break;
		printf("%d bytes long type %s magic %#x %d, checksum %#x\n",
		    buffer.h.len * BPW, types[buffer.h.type], buffer.h.magic,
		    buffer.h.magic, buffer.h.checksum);
		fflush(stdout);
		if (buffer.h.magic != MAGIC)
			errx(1, "Bad magic %#x instad of %#x\n",
			    buffer.h.magic, MAGIC);
		fread(((char *)&buffer) + BPW, wtob(buffer.h.len - 1), 1, fp);
		switch(buffer.h.type) {
		case FS_VOLUME:
			break;
		case FS_FINDEX:
			break;
		case FS_CLRI:
			/* FALLTHROUGH */
		case FS_BITS:
			fseek(fp, wtob(buffer.b.nwds), 1);
			break;
		case FS_OINODE:
			break;
		case FS_ONAME:
			break;
		case FS_VOLEND:
			break;
		case FS_END:
			break;
		case FS_INODE:
			fseek(fp, wtob(btow(buffer.i.dsize)), 1);
			break;
		case FS_NAME:
			printf("-- File %s %o %d %d\n", buffer.n.name, buffer.n.mode, buffer.n.size, buffer.n.dsize);
			fseek(fp, wtob(btow(buffer.n.size)), 1);
			break;
		}
	} while (!feof(fp));
}
#endif

int
main(int argc, char **argv)
{
	aix_scan(++argv);
}
