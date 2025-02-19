//
// Created by zhanglei on 2025/2/6.
//

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
}

#include "app/include/source/host/collector/iostat/ioconf.h"
#include "app/include/source/host/collector/iostat/common.h"

namespace App::Source::Host::Collector::IOStat {

static unsigned int ioc_parsed = 0;
static struct ioc_entry *ioconf[MAX_BLKDEV + 1];
static unsigned int ioc_refnr[MAX_BLKDEV + 1];

/*
 ***************************************************************************
 * Free ioc_entry structures
 ***************************************************************************
 */
static void ioc_free(void)
{
	unsigned int i;
	struct ioc_entry **p;

	/* Take out all of the references first */
	for (i = 0, p = ioconf; i < MAX_BLKDEV; ++i, ++p) {
		if ((*p == NULL) || ((*p)->live))
			continue;

		if ((*p)->desc != (*p)->blkp->desc) {
			/* Not a shared description */
			free((*p)->desc);
		}
		free(*p);
		*p = NULL;
	}

	/* Now the live ones */
	for (i = 0, p = ioconf; i < MAX_BLKDEV; ++i, ++p) {
		if (*p == NULL)
			continue;
		free((*p)->blkp);
		free(*p);
		*p = NULL;
	}
}

/*
 ***************************************************************************
 * ioc_conv - Turn a number into a string in radix <radix> using symbol
 *   set (and ordering) syms.  Use nozero to generate strings
 *   in which the number system uses a single sym for the
 *   radix value (not 2, like decimal) and adds a column only
 *   at radix+1.  If decimal were like this:
 *
 *   (no zero) 1 2 3 4 5 6 7 8 9 0 11 12 13 14 15 16 17 18 19 10 ...
 ***************************************************************************
 */
static char *ioc_conv(int radix, int nozero, const char *syms,
		      unsigned int val)
{
	static char out[17];
	char *p;
	int j;

	*(p = out + 16) = '\0';

	val += nozero;

	if (val == 0) {
		if (!nozero) {
			*--p = '0';
		}
		return (p);	/* Empty string if nozero radix gets val == 0 */
	}

	while (val > 0) {

		*--p = syms[j = val % radix];
		val /= radix;
		if (nozero && (j == 0)) {
			/* Comp for 10 in nozero bases */
			--val;
		}
	}
	return (p);
}

char *ioc_ito10(unsigned int n)
{
	return (ioc_conv(10, 0, DIGITS, n));
}

char *ioc_ito26(unsigned int n)
{
	return (ioc_conv(26, 1, "zabcdefghijklmnopqrstuvwxy", n));
}

/*
 ***************************************************************************
 * ioc_init() - internalize the ioconf file
 *
 * given:    void
 * does:     parses IOCONF into ioconf, an array of ioc_entry *
 *           Only entries having lines in IOCONF will have valid pointers
 * return:   1 on success
 *           0 on failure
 ***************************************************************************
 */
int ioc_init(void)
{
	FILE *fp;
	unsigned int i, major, indirect, count = 0;
	char buf[IOC_LINESIZ];
	char cfmt[IOC_FMTLEN];
	char dfmt[IOC_FMTLEN];
	char pfmt[IOC_FMTLEN];
	char desc[IOC_DESCLEN];
	struct ioc_entry  *iocp = NULL;
	struct blk_config *blkp = NULL;
	char ioconf_name[64];

	if ((fp = fopen(IOCONF, "r")) == NULL) {
		if ((fp = fopen(LOCAL_IOCONF, "r")) == NULL)
			return 0;
		strncpy(ioconf_name, LOCAL_IOCONF, sizeof(ioconf_name));
	}
	else {
		strncpy(ioconf_name, IOCONF, sizeof(ioconf_name));
	}
	ioconf_name[sizeof(ioconf_name) - 1] = '\0';

	/* Init ioc_refnr array */
	memset(ioc_refnr, 0, sizeof(ioc_refnr));

	while (fgets(buf, sizeof(buf) - 1, fp)) {

		if ((*buf == '#') || (*buf == '\n'))
			continue;

		/*
		 * Preallocate some (probably) needed data structures
		 */
		IOC_ALLOC(blkp, struct blk_config, BLK_CONFIG_SIZE);
		IOC_ALLOC(iocp, struct ioc_entry, IOC_ENTRY_SIZE);
		memset(blkp, 0, BLK_CONFIG_SIZE);
		memset(iocp, 0, IOC_ENTRY_SIZE);

		i = sscanf(buf, "%u:%u:%u:%63s",
			   &major, &indirect, &iocp->ctrlno, desc);

		if (i != 4) {
			i = sscanf(buf, "%u:%u:%u",
				   &major, &indirect, &iocp->ctrlno);
		}

		if ((i == 3) || (i == 4)) {
			/* indirect record */
			if (indirect == 0) {
				/* conventional usage for unsupported device */
				continue;
			}
			if (indirect > MAX_BLKDEV) {
				fprintf(stderr, "%s: Indirect major #%u out of range\n",
					ioconf_name, indirect);
				continue;
			}
			if (major > MAX_BLKDEV) {
				fprintf(stderr, "%s: Major #%u out of range\n",
					ioconf_name, major);
				continue;
			}
			if (ioconf[indirect] == NULL) {
				fprintf(stderr,
					"%s: Indirect record '%u:%u:%u:...'"
					" references not yet seen major %u\n",
					ioconf_name, major, indirect, iocp->ctrlno, major);
				continue;
			}
			/*
			 * Cool. Point this device at its referent.
			 * Skip last: (last field my be empty...)
			 * if it was empty and : was in the sscanf spec
			 * we'd only see 3 fields...
			 */
			if (i == 3) {
				/* reference the mothership */
				iocp->desc = ioconf[indirect]->blkp->desc;
			}
			else {
				IOC_ALLOC(iocp->desc, char, IOC_DESCLEN);
				memcpy(iocp->desc, desc, IOC_DESCLEN);
			}
			ioc_refnr[indirect]++;
			ioconf[major] = iocp;
			iocp->basemajor = indirect;
			iocp->blkp = ioconf[indirect]->blkp;
			iocp->live = 0;
			iocp = NULL;
			continue;
			/* all done with indirect record */
		}

		/* maybe it's a full record? */

		i = sscanf(buf, "%u:%[^:]:%[^:]:%u:%[^:]:%u:%[^:]:%u:%63s",
			   &major, blkp->name,
			   cfmt, &iocp->ctrlno,
			   dfmt, &blkp->dcount,
			   pfmt, &blkp->pcount,
			   desc);

		if (i != 9) {
			fprintf(stderr, "%s: Malformed %u field record: %s\n",
				ioconf_name, i, buf);
			continue;
		}

		/* this is a full-fledged direct record */

		if ((major == 0) || (major >= MAX_BLKDEV)) {
			fprintf(stderr, "%s: major #%u out of range\n",
				__FUNCTION__, major);
			continue;
		}

		/* is this an exception record? */
		if (*cfmt == 'x') {
			struct blk_config *xblkp;

			/*
			 * device has an aliased minor
			 * for now we only support one exception per major
			 * (catering to initrd: (1,250))
			 */
			if (ioconf[major] == NULL) {
				fprintf(stderr, "%s: type 'x' record for"
					" major #%u must follow the base record - ignored\n",
					ioconf_name, major);
				continue;
			}
			xblkp = ioconf[major]->blkp;

			if (xblkp->ext) {
				/*
				 * Enforce one minor exception per major policy
				 * note: this applies to each major number and
				 * all of it's indirect (short form) majors
				 */
				fprintf(stderr, "%s: duplicate 'x' record for"
					" major #%u - ignored\ninput line: %s\n",
					ioconf_name, major, buf);
				continue;
			}
			/*
			 * Decorate the base major struct with the
			 * exception info
			 */
			xblkp->ext_minor = iocp->ctrlno;
			strncpy(xblkp->ext_name, blkp->name, IOC_NAMELEN);
			xblkp->ext_name[IOC_NAMELEN - 1] = '\0';
			xblkp->ext = 1;
			continue;
		}

		/*
		 * Preformat the sprintf format strings for generating
		 * c-d-p info in ioc_name()
		 */

		/* basename of device + provided string + controller # */
		if (*cfmt == '*') {
			strncpy(blkp->cfmt, blkp->name, MINIMUM(sizeof(blkp->name), sizeof(blkp->cfmt) - 1));
			blkp->cfmt[sizeof(blkp->cfmt) - 1] = '\0';
		}
		else {
			sprintf(blkp->cfmt, "%s%s%%d", blkp->name, cfmt);
			++(blkp->ctrl_explicit);
		}

		/* Disk */
		*blkp->dfmt = '\0';
		switch (*dfmt) {
		case 'a':
			blkp->cconv = ioc_ito26;
			strcpy(blkp->dfmt, "%s");
			break;

		case '%':
			strncpy(blkp->dfmt, dfmt + 1, IOC_FMTLEN - 1);
			/* fallthrough to next case */
		case 'd':
			blkp->cconv = ioc_ito10;
			strcat(blkp->dfmt, "%s");
			break;
		}

		/* Partition */
		sprintf(blkp->pfmt, "%s%%d", (*pfmt == '*') ? "" : pfmt);

		/*
		 * We're good to go.
		 * Stuff the ioc_entry and ref it.
		 */
		iocp->live = 1;
		iocp->blkp = blkp;
		iocp->desc = NULL;
		iocp->basemajor = major;
		ioconf[major] = iocp;
		memcpy(blkp->desc, desc, IOC_DESCLEN);
		blkp = NULL; iocp = NULL;
		++count;
	}
	fclose(fp);

	/*
	 * These will become leaks if we ever 'continue'
	 * after IOC_ALLOC( blkp->desc ... ).
	 * Right now, we don't.
	 */
	free(blkp);
	free(iocp);

	/* Indicate that ioconf file has been parsed */
	ioc_parsed = 1;

	return (count);

free_and_return:
	/* Free pointers and return */
	fclose(fp);
	if (blkp) {
		free(blkp);
	}
	if (iocp) {
		free(iocp);
	}

	return 0;
}

/*
 ***************************************************************************
 *  ioc_name() - Generate a name from a maj,min pair
 *
 * IN:
 * @major	Device major number.
 * @minor	Device minor number.
 *
 * RETURNS:
 * Returns NULL if major or minor are out of range
 * otherwise returns a pointer to a static string containing
 * the generated name.
 ***************************************************************************
 */

char *ioc_name(unsigned int major, unsigned int minor)
{
	static char name[IOC_DEVLEN];
	struct ioc_entry *p;
	int base, offset;

	if ((major > MAX_BLKDEV) || (minor > IOC_MAXMINOR)) {
		return (NULL);
	}

	if (!ioc_parsed && !ioc_init())
		return (NULL);

	p = ioconf[major];

	/* Invalid major or minor numbers? */
	if ((p == NULL) || ((minor & 0xff) >= (p->blkp->dcount * p->blkp->pcount))) {
		/*
		 * That minor test is only there for IDE-style devices
		 * that have no minors over 128.
		 */
		strcpy(name, K_NODEV);
		return (name);
	}

	/* Is this an extension record? */
	if (p->blkp->ext && (p->blkp->ext_minor == minor)) {
		strncpy(name, p->blkp->ext_name, sizeof(name));
		name[sizeof(name) - 1] = '\0';
		return (name);
	}

	/* OK.  we're doing an actual device name... */

	/*
	 * Assemble base + optional controller info
	 * this is of course too clever by half
	 * the parser has already cooked cfmt, dfmt to make this easy
	 * (we parse once but may generate lots of names)
	 */
	base = p->ctrlno * p->blkp->dcount;
	if (minor >= 256) {
		base += p->blkp->dcount * (ioc_refnr[p->basemajor] + 1) * (minor >> 8);
	}

	offset = (minor & 0xff) / p->blkp->pcount;
	if (!p->blkp->ctrl_explicit) {
		offset += base;
	}

	/*
	 * These sprintfs can't be coalesced because the first might
	 * ignore its first arg
	 */
	sprintf(name, p->blkp->cfmt, p->ctrlno);
	sprintf(name + strlen(name), p->blkp->dfmt, p->blkp->cconv(offset));

	if (!IS_WHOLE(major, minor)) {
		/*
		 * Tack on partition info, format string cooked (curried?) by
		 * the parser
		 */
		sprintf(name + strlen(name), p->blkp->pfmt, minor % p->blkp->pcount);
	}
	return (name);
}

/*
 ***************************************************************************
 * Transform device mapper name: Get the user assigned name of the logical
 * device instead of the internal device mapper numbering.
 *
 * IN:
 * @major	Device major number.
 * @minor	Device minor number.
 *
 * RETURNS:
 * Assigned name of the logical device.
 ***************************************************************************
 */
char *transform_devmapname(unsigned int major, unsigned int minor)
{
	DIR *dm_dir;
	struct dirent *dp;
	char filen[MAX_FILE_LEN];
	char *dm_name = NULL;
	static char name[MAX_NAME_LEN];
	struct stat aux;
	unsigned int dm_major, dm_minor;

	if ((dm_dir = opendir(DEVMAP_DIR)) == NULL) {
		fprintf(stderr, "Cannot open %s: %s\n", DEVMAP_DIR, strerror(errno));
		exit(4);
	}

	while ((dp = readdir(dm_dir)) != NULL) {
		/* For each file in DEVMAP_DIR */

		snprintf(filen, sizeof(filen), "%s/%s", DEVMAP_DIR, dp->d_name);
		filen[sizeof(filen) - 1] = '\0';

		if (__stat(filen, &aux) == 0) {
			/* Get its minor and major numbers */

			dm_major = __major(aux.st_rdev);
			dm_minor = __minor(aux.st_rdev);

			if ((dm_minor == minor) && (dm_major == major)) {
				strncpy(name, dp->d_name, sizeof(name));
				name[sizeof(name) - 1] = '\0';
				dm_name = name;
				break;
			}
		}
	}
	closedir(dm_dir);

	return dm_name;
}
}