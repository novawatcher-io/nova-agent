//
// Created by zhanglei on 25-2-1.
//
#include "app/include/common/util.h"

extern "C" {
#include <sys/utsname.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
}
#include "app/include/common/const.h"

namespace App::Common {

int kernel_version(void)
{
    struct utsname uts;
    int major, minor, patch;

    if (uname(&uts) < 0)
        return -1;
    if (sscanf(uts.release, "%d.%d.%d", &major, &minor, &patch) != 3)
        return -1;
    return KRELEASE(major, minor, patch);
}

/* return numerical :999 suffix or null. sideeffect: replace ':' with \0 */
char* cutalias(char* name)
{
    int digit = 0;
    int pos;

    for(pos=strlen(name); pos>0; pos--)
    {
        if (name[pos-1]==':' && digit)
        {
            name[pos-1]='\0';
            return name+pos;
        }
        if (!isdigit(name[pos-1]))
            break;
        digit = 1;
    }
    return NULL;
}

/* return index of last non digit or -1 if it does not end with digits */
int rindex_nondigit(char *name)
{
    int pos = strlen(name);

    for(pos=strlen(name); pos>0; pos--)
    {
        if (!isdigit(name[pos-1]))
            return pos;
    }
    return 0;
}


int nstrcmp(const char *ap, const char *bp)
{
    char *a = (char*)strdup(ap);
    char *b = (char*)strdup(bp);
    char *an, *bn;
    int av = 0, bv = 0;
    char *aalias=cutalias(a);
    char *balias=cutalias(b);
    int aindex=rindex_nondigit(a);
    int bindex=rindex_nondigit(b);
    int complen=(aindex<bindex)?aindex:bindex;
    int res = strncmp(a, b, complen);

    if (res != 0)
    { free(a); free(b); return res; }

    if (aindex > bindex)
    { free(a); free(b); return 1; }

    if (aindex < bindex)
    { free(a); free(b); return -1; }

    an = a+aindex;
    bn = b+bindex;

    av = atoi(an);
    bv = atoi(bn);

    if (av < bv)
    { free(a); free(b); return -1; }

    if (av > bv)
    { free(a); free(b); return 1; }

    av = -1;
    if (aalias != NULL)
        av = atoi(aalias);

    bv = -1;
    if (balias != NULL)
        bv = atoi(balias);

    free(a); free(b);

    if (av < bv)
        return -1;

    if (av > bv)
        return 1;

    return 0;
}

int is_device(char *sysdev, char *name, int allow_virtual)
{
	char syspath[PATH_MAX];
	char *slash;

	/* Some devices may have a slash in their name (eg. cciss/c0d0...) */
	while ((slash = strchr(name, '/'))) {
		*slash = '!';
	}
	snprintf(syspath, sizeof(syspath), "%s/%s/%s%s", sysdev, "block", name,
		 allow_virtual ? "" : "/device");

	return !(access(syspath, F_OK));
}

void read_uptime(unsigned long long *uptime)
{
	FILE *fp = NULL;
	char line[128];
	unsigned long up_sec, up_cent;
	int err = false;

	if ((fp = fopen(uptime_proc_file.c_str(), "r")) == NULL) {
		err = true;
	}
	else if (fgets(line, sizeof(line), fp) == NULL) {
		err = true;
	}
	else if (sscanf(line, "%lu.%lu", &up_sec, &up_cent) == 2) {
		*uptime = (unsigned long long) up_sec * 100 +
			  (unsigned long long) up_cent;
	}
	else {
		err = true;
	}

	if (fp != NULL) {
		fclose(fp);
	}
	if (err) {
        SPDLOG_ERROR("Cannot read {}", uptime_proc_file.c_str());
        return;
	}
}

/*
 ***************************************************************************
 * Compute time interval.
 *
 * IN:
 * @prev_uptime	Previous uptime value (in jiffies or 1/100th of a second).
 * @curr_uptime	Current uptime value (in jiffies or 1/100th of a second).
 *
 * RETURNS:
 * Interval of time in jiffies or 1/100th of a second.
 ***************************************************************************
 */
unsigned long long get_interval(unsigned long long prev_uptime,
				unsigned long long curr_uptime)
{
	unsigned long long itv;

	/* prev_time=0 when displaying stats since system startup */
	itv = curr_uptime - prev_uptime;

	if (!itv) {	/* Paranoia checking */
		itv = 1;
	}

	return itv;
}

}