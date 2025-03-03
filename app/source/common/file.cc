//
// Created by zhanglei on 25-1-25.
//
#include "app/include/common/file.h"
#include <bits/types.h>

extern "C" {
#include <sys/vfs.h>
}
#include <fstream>
#include <iostream>


namespace App::Common {

#ifndef HAVE_MAJOR
# define major(dev)  (((dev) >> 8) & 0xff)
# define minor(dev)  ((dev) & 0xff)
# define makedev(maj, min)  (((maj) << 8) | (min))
#endif
#undef HAVE_MAJOR

/* Many space usage primitives use all 1 bits to denote a value that is
   not applicable or unknown.  Propagate this information by returning
   a uintmax_t value that is all 1 bits if X is all 1 bits, even if X
   is unsigned and narrower than uintmax_t.  */
#define PROPAGATE_ALL_ONES(x) \
((sizeof (x) < sizeof (uintmax_t) \
&& (~ (x) == (sizeof (x) < sizeof (int) \
? - (1 << (sizeof (x) * CHAR_BIT)) \
: 0))) \
? UINTMAX_MAX : (uintmax_t) (x))

static char *
terminate_at_blank (char *str)
{
    char *s = strchr (str, ' ');
    if (s)
        *s = '\0';
    return s;
}

// https://github.com/coreutils/gnulib/blob/master/lib/mountlist.c#L463
bool me_remote(const char* Fs_name, const char* Fs_type) {
    return (strchr (Fs_name, ':') != NULL
    || ((Fs_name)[0] == '/'
    && (Fs_name)[1] == '/'
    && (strcmp (Fs_type, "smbfs") == 0
    || strcmp (Fs_type, "smb3") == 0
    || strcmp (Fs_type, "cifs") == 0))
    || strcmp (Fs_type, "acfs") == 0
    || strcmp (Fs_type, "afs") == 0
    || strcmp (Fs_type, "coda") == 0
    || strcmp (Fs_type, "auristorfs") == 0
    || strcmp (Fs_type, "fhgfs") == 0
    || strcmp (Fs_type, "gpfs") == 0
    || strcmp (Fs_type, "ibrix") == 0
    || strcmp (Fs_type, "ocfs2") == 0
    || strcmp (Fs_type, "vxfs") == 0
    || strcmp ("-hosts", Fs_name) == 0);
}

// https://github.com/coreutils/gnulib/blob/master/lib/mountlist.c#L463
bool me_dummy(const char* Fs_name, const char* Fs_type) {
    return   (strcmp (Fs_type, "none") == 0) || (strcmp (Fs_type, "autofs") == 0
   || strcmp (Fs_type, "proc") == 0
   || strcmp (Fs_type, "subfs") == 0
   /* for Linux 2.6/3.x */
   || strcmp (Fs_type, "debugfs") == 0
   || strcmp (Fs_type, "devpts") == 0
   || strcmp (Fs_type, "fusectl") == 0
   || strcmp (Fs_type, "fuse.portal") == 0
   || strcmp (Fs_type, "mqueue") == 0
   || strcmp (Fs_type, "rpc_pipefs") == 0
   || strcmp (Fs_type, "sysfs") == 0
   || strcmp (Fs_type, "squashfs") == 0
   || strcmp (Fs_type, "nsfs") == 0
   || strcmp (Fs_type, "cgroup") == 0
   || strcmp (Fs_type, "devtmpfs") == 0
   /* FreeBSD, Linux 2.4 */
   || strcmp (Fs_type, "devfs") == 0
   /* for NetBSD 3.0 */
   || strcmp (Fs_type, "kernfs") == 0
   /* for Irix 6.5 */
   || strcmp (Fs_type, "ignore") == 0);
}

/* Unescape the paths in mount tables.
   STR is updated in place.  */

static void
unescape_tab (char *str)
{
    size_t i, j = 0;
    size_t len = strlen (str) + 1;
    for (i = 0; i < len; i++)
    {
        if (str[i] == '\\' && (i + 4 < len)
            && str[i + 1] >= '0' && str[i + 1] <= '3'
            && str[i + 2] >= '0' && str[i + 2] <= '7'
            && str[i + 3] >= '0' && str[i + 3] <= '7')
        {
            str[j++] = (str[i + 1] - '0') * 64 +
                       (str[i + 2] - '0') * 8 +
                       (str[i + 3] - '0');
            i += 3;
        }
        else
            str[j++] = str[i];
    }
}


struct std::unique_ptr<mount_entry> BasicFileReader::ReadFileSystemList(bool need_fs_type) {
    // read mount info
    std::ifstream mount("/proc/self/mountinfo");
    std::string line;
    struct mount_entry* me;
    std::unique_ptr<mount_entry> head;

    if (mount.is_open()) {
        while (getline(mount, line)) {
            int rc, mntroot_s;
            unsigned int devmaj, devmin;
            rc = sscanf(line.c_str(), "%*u "        /* id - discarded  */
                              "%*u "        /* parent - discarded  */
                              "%u:%u "      /* dev major:minor  */
                              "%n",         /* mountroot (start)  */
                              &devmaj, &devmin,
                              &mntroot_s);
            if (rc != 2 && rc != 3)  /* 3 if %n included in count.  */
                continue;

            /* find end of MNTROOT.  */
            char *mntroot = (char*)line.data() + mntroot_s;
            char *blank = terminate_at_blank (mntroot);
            if (! blank)
                continue;

            /* find end of TARGET.  */
            char *target = blank + 1;
            blank = terminate_at_blank (target);
            if (! blank)
                continue;

            /* skip optional fields, terminated by " - "  */
            char *dash = strstr (blank + 1, " - ");
            if (! dash)
                continue;

            /* advance past the " - " separator.  */
            char *fstype = dash + 3;
            blank = terminate_at_blank (fstype);
            if (! blank)
                continue;

            /* find end of SOURCE.  */
            char *source = blank + 1;
            if (! terminate_at_blank (source))
                continue;

            /* manipulate the sub-strings in place.  */
             unescape_tab (source);
             unescape_tab (target);
             unescape_tab (mntroot);
             unescape_tab (fstype);

            if (me_dummy(source, fstype)) {
                continue;
            }

            if (me_remote(source, fstype)) {
                continue;
            }



            struct statfs fsd;
            if (statfs (target, &fsd) != 0)
                continue;

            if (fsd.f_blocks <= 0) {
                continue;
            }

            struct stat buf;
            int ret = stat(target, &buf);
            if (ret == -1) {
                continue;
            }


             if (head == nullptr) {
                 head = std::make_unique<mount_entry>();
                 me = head.get();
            } else {
                me->me_next = std::make_unique<mount_entry>();
                me = me->me_next.get();
            }

            me->me_devname =  (source);
            me->me_mntroot =  (mntroot);
            me->me_type =  (fstype);
            me->me_mountdir =  (target);
            me->me_type_malloced = 1;
            me->me_dev = makedev (devmaj, devmin);
            me->me_remote =  me_remote(me->me_devname.c_str(), me->me_type.c_str());
            me->me_dummy = me_dummy(me->me_devname.c_str(), me->me_type.c_str());

            __fsword_t block_size = fsd.f_frsize ? PROPAGATE_ALL_ONES(fsd.f_frsize) : PROPAGATE_ALL_ONES(fsd.f_bsize);
            me->available = fsd.f_ffree * block_size;
            me->size = fsd.f_files * block_size;
            if (me->size * block_size > me->available * block_size) {
                me->used = me->size * block_size - me->available * block_size;
            } else {
                me->used = 0;
            }
            /* Add to the linked list. */
            continue;
        }
        mount.close(); // 关闭文件
        return head;
    } else {
        SPDLOG_ERROR("open  /proc/self/mountinfo failed");
        return nullptr;
    }
}
}


// for (me = mount_list; me; me = me->me_next)
//     get_dev (me->me_devname, me->me_mountdir, nullptr, nullptr, me->me_type,
//              me->me_dummy, me->me_remote, nullptr, true);