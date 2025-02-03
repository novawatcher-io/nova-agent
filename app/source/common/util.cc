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

}