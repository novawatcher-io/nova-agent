//
// Created by zhanglei on 2025/2/6.
//

#pragma once

extern "C" {
#include <time.h>
#include <dirent.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
}

namespace App::Source::Host::Collector::IOStat {
#ifndef MINORBITS
#define MINORBITS	20
#endif
#define S_MAXMINOR	((1U << MINORBITS) - 1)
#define S_MAXMAJOR	((1U << (32 - MINORBITS)) - 1)

#define PRE	""

#define __time(m)		time(m)
#define __uname(m)		uname(m)
#define __statvfs(m,n)		statvfs(m,n)
#define __getenv(m)		getenv(m)
#define __alarm(m)		alarm(m)
#define __pause()		pause()
#define __stat(m,n)		stat(m,n)
#define __opendir(m)		opendir(m)
#define __readdir(m)		readdir(m)
#define __closedir(m)		closedir(m)
#define __realpath(m,n)		realpath(m,n)
#define __gettimeofday(m,n)	gettimeofday(m,n)
#define __getpwuid(m)		getpwuid(m)
#define __fork(m)		fork(m)
#define __major(m)		major(m)
#define __minor(m)		minor(m)


}