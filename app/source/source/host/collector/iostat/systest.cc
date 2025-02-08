//
// Created by zhanglei on 2025/2/6.
//
#include "app/include/source/host/collector/iostat/systest.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

namespace App::Source::Host::Collector::IOStat {

time_t __unix_time = 1591016000;	/* Mon Jun  1 12:53:20 2020 UTC */
int __env = 0;


/*
 ***************************************************************************
 * Test mode: Instead of reading system time, use time given on the command
 * line.
 *
 * RETURNS:
 * Number of seconds since the epoch, as given on the command line.
 ***************************************************************************
 */
time_t get_unix_time(time_t *t)
{
	return __unix_time;
}


/*
 ***************************************************************************
 * Test mode: Send bogus information about current kernel.
 *
 * OUT:
 * @h	Structure with kernel information.
 ***************************************************************************
 */
void get_uname(struct utsname *h)
{
	strcpy(h->sysname, "Linux");
	strcpy(h->nodename, "SYSSTAT.TEST");
	strcpy(h->release, "1.2.3-TEST");
	strcpy(h->machine, "x86_64");
}

/*
 ***************************************************************************
 * Test mode: Send bogus information about current filesystem.
 *
 * OUT:
 * @buf	Structure with filesystem information.
 ***************************************************************************
 */
int get_fs_stat(char *c, struct statvfs *buf)
{
	static int p = 0;
	/*
	 * f_bfree, f_blocks and f_bavail used to be unsigned long.
	 * So don't use values greater then UINT_MAX to make sure that values
	 * won't overflow on 32-bit systems.
	 */
	const unsigned long long bfree[4]  = {739427840, 286670336, 1696156672, 2616732672};
	const unsigned long long blocks[4] = {891291136, 502345216, 1829043712, 3502345216};
	const unsigned long long bavail[4] = {722675712, 241253120, 1106515456, 1871315456};
	const unsigned long long files[4]  = {6111232, 19202048, 1921360, 19202048};
	const unsigned long long ffree[4]  = {6008414, 19201593, 1621550, 19051710};

	buf->f_bfree = bfree[p];
	buf->f_blocks = blocks[p];
	buf->f_bavail = bavail[p];
	buf->f_frsize = 1;
	buf->f_files = files[p];
	buf->f_ffree = ffree[p];

	p = (p + 1) & 0x3;

	return 0;
}

/*
 ***************************************************************************
 * Test mode: Ignore environment variable value.
 ***************************************************************************
 */
char *get_env_value(const char *c)
{
	if (!__env)
		return NULL;

	fprintf(stderr, "Reading contents of %s\n", c);
	return getenv(c);
}






/*
 ***************************************************************************
 * Read next file name contained in a "_list" file.
 *
 * IN:
 * @dir	Pointer on current "_list" file.
 *
 * RETURNS:
 * A structure containing the name of the next file to read.
 ***************************************************************************
 */
struct dirent *read_list(DIR *dir)
{
	FILE *fp = (FILE *) dir;
	static struct dirent drd;
	char line[1024];


	if ((fgets(line, sizeof(line), fp) != NULL) && (strlen(line) > 1) &&
		(strlen(line) < sizeof(drd.d_name))) {
		strcpy(drd.d_name, line);
		drd.d_name[strlen(line) - 1] = '\0';
		return &drd;
	}

	return NULL;
}

/*
 ***************************************************************************
 * Close a "_list" file.
 *
 * IN:
 * @dir	Pointer on "_list" file to close.
 ***************************************************************************
 */
void close_list(DIR *dir)
{
	FILE *fp = (FILE *) dir;

	fclose(fp);
}

/*
 ***************************************************************************
 * Replacement function for realpath() system call. Do nothing here.
 *
 * IN:
 * @name	Pathname to process.
 * @c		Unused here.
 *
 * RETURNS:
 * Pathname (unchanged).
 ***************************************************************************
 */
char *get_realname(char *name, char *c)
{
	char *resolved_name;

	if ((resolved_name = (char *) malloc(1024)) == NULL) {
		perror("malloc");
		exit(4);
	}
	strncpy(resolved_name, name, 1024);
	resolved_name[1023] = '\0';

	return resolved_name;
}

/*
 ***************************************************************************
 * Replacement function for fork() system call. Don't fork really but return
 * a known PID number.
 *
 * RETURNS:
 * Known PID number.
 ***************************************************************************
 */
pid_t get_known_pid(void)
{
	return 8741;
}

}