//
// Created by zhanglei on 2025/2/6.
//
#include "app/include/source/host/collector/iostat/rd_stats.h"

extern "C" {
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>
}
#include "app/include/source/host/collector/iostat/common.h"

namespace App::Source::Host::Collector::IOStat {

#ifdef USE_NLS
#include <locale.h>
#include <libintl.h>
#define _(string) gettext(string)
#else
#define _(string) (string)
#endif

/* Generic PSI structure */
struct stats_psi {
	unsigned long long total;
	unsigned long	   avg10;
	unsigned long	   avg60;
	unsigned long	   avg300;
};

/*
 ***************************************************************************
 * Read CPU statistics.
 * Remember that this function is used by several sysstat commands!
 *
 * IN:
 * @st_cpu	Buffer where structures containing stats will be saved.
 * @nr_alloc	Total number of structures allocated. Value is >= 1.
 *
 * OUT:
 * @st_cpu	Buffer with statistics.
 *
 * RETURNS:
 * Highest CPU number(*) for which statistics have been read.
 * 1 means CPU "all", 2 means CPU 0, 3 means CPU 1, etc.
 * Or -1 if the buffer was too small and needs to be reallocated.
 *
 * (*)This doesn't account for all processors in the machine in the case
 * where some CPU are offline and located at the end of the list.
 *
 * USED BY:
 * sadc, iostat, mpstat, pidstat
 ***************************************************************************
 */
__nr_t read_stat_cpu(struct stats_cpu *st_cpu, __nr_t nr_alloc)
{
	FILE *fp;
	struct stats_cpu *st_cpu_i;
	struct stats_cpu sc;
	char line[8192];
	int proc_nr;
	__nr_t cpu_read = 0;

	if ((fp = fopen(STAT, "r")) == NULL) {
		fprintf(stderr, _("Cannot open %s: %s\n"), STAT, strerror(errno));
		exit(2);
	}

	while (fgets(line, sizeof(line), fp) != NULL) {

		if (!strncmp(line, "cpu ", 4)) {

			/*
			 * All the fields don't necessarily exist,
			 * depending on the kernel version used.
			 */
			memset(st_cpu, 0, STATS_CPU_SIZE);

			/*
			 * Read the number of jiffies spent in the different modes
			 * (user, nice, etc.) among all proc. CPU usage is not reduced
			 * to one processor to avoid rounding problems.
			 */
			sscanf(line + 5, "%llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
			       &st_cpu->cpu_user,
			       &st_cpu->cpu_nice,
			       &st_cpu->cpu_sys,
			       &st_cpu->cpu_idle,
			       &st_cpu->cpu_iowait,
			       &st_cpu->cpu_hardirq,
			       &st_cpu->cpu_softirq,
			       &st_cpu->cpu_steal,
			       &st_cpu->cpu_guest,
			       &st_cpu->cpu_guest_nice);

			if (!cpu_read) {
				cpu_read = 1;
			}

			if (nr_alloc == 1)
				/* We just want to read stats for CPU "all" */
				break;
		}

		else if (!strncmp(line, "cpu", 3)) {
			/* All the fields don't necessarily exist */
			memset(&sc, 0, STATS_CPU_SIZE);
			/*
			 * Read the number of jiffies spent in the different modes
			 * (user, nice, etc) for current proc.
			 * This is done only on SMP machines.
			 */
			sscanf(line + 3, "%d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
			       &proc_nr,
			       &sc.cpu_user,
			       &sc.cpu_nice,
			       &sc.cpu_sys,
			       &sc.cpu_idle,
			       &sc.cpu_iowait,
			       &sc.cpu_hardirq,
			       &sc.cpu_softirq,
			       &sc.cpu_steal,
			       &sc.cpu_guest,
			       &sc.cpu_guest_nice);

			if (proc_nr + 2 > nr_alloc) {
				cpu_read = -1;
				break;
			}

			st_cpu_i = st_cpu + proc_nr + 1;
			*st_cpu_i = sc;

			if (proc_nr + 2 > cpu_read) {
				cpu_read = proc_nr + 2;
			}
		}
	}

	fclose(fp);
	return cpu_read;
}

/*
 ***************************************************************************
 * Read interrupts statistics from /proc/interrupts.
 *
 * IN:
 * @st_irq	Structure where stats will be saved.
 * @nr_alloc	Number of CPU structures allocated. Value is >= 1.
 * @nr_int	Number of interrupts, including sum. value is >= 1.
 *
 * OUT:
 * @st_irq	Structure with statistics.
 *
 * RETURNS:
 * Highest CPU number for which stats have been successfully read (2 for CPU0,
 * 3 for CPU 1, etc.) Same logic than for softnet statistics. This number will
 * be saved in a->_nr0. See wrap_read_stat_irq().
 * Returns 0 if no statistics have been read.
 * Returns -1 if the buffer was too small and needs to be reallocated (we
 * mean here, too small for all the CPU, not for the interrupts whose number
 * is considered to be a constant. Remember that only the number of items is
 * saved in file preceding each sample, not the number of sub-items).
 ***************************************************************************
 */
__nr_t read_stat_irq(struct stats_irq *st_irq, __nr_t nr_alloc, __nr_t nr_int)
{
	FILE *fp;
	struct stats_irq *st_cpuall_sum, *st_cpu_irq, *st_cpu_sum, *st_cpuall_irq;
	char *line = NULL, *li;
	int rc = 0, irq_read = 0;
	int cpu, len;
	int cpu_nr = nr_alloc - 1;
	int *cpu_index = NULL, index = 0;
	char *cp, *next;

	if (!cpu_nr) {
		/* We have only one proc and a non SMP kernel */
		cpu_nr = 1;
	}
	SREALLOC(cpu_index, int, sizeof(int) * cpu_nr);

	if ((fp = fopen(INTERRUPTS, "r")) != NULL) {

		SREALLOC(line, char, INTERRUPTS_LINE + 11 * cpu_nr);

		/*
		 * Parse header line to see which CPUs are online
		 */
		while (fgets(line, INTERRUPTS_LINE + 11 * cpu_nr, fp) != NULL) {

			next = line;
			while (((cp = strstr(next, "CPU")) != NULL) && (index < cpu_nr)) {
				cpu = strtol(cp + 3, &next, 10);

				if (cpu + 2 > nr_alloc) {
					rc = -1;
					goto out;
				}
				cpu_index[index++] = cpu;
			}
			if (index)
				/* Header line found */
				break;
		}

		st_cpuall_sum = st_irq;
		/* Save name "sum" for total number of interrupts */
		strcpy(st_cpuall_sum->irq_name, K_LOWERSUM);

		/* Parse each line of interrupts statistics data */
		while ((fgets(line, INTERRUPTS_LINE + 11 * cpu_nr, fp) != NULL) &&
		       (irq_read < nr_int - 1)) {

			/* Skip over "<irq>:" */
			if ((cp = strchr(line, ':')) == NULL)
				/* Chr ':' not found */
				continue;
			cp++;

			irq_read++;
			st_cpuall_irq = st_irq + irq_read;

			/* Remove possible heading spaces in interrupt's name... */
			li = line;
			while (*li == ' ')
				li++;

			len = strcspn(li, ":");
			if (len >= MAX_SA_IRQ_LEN) {
				len = MAX_SA_IRQ_LEN - 1;
			}
			/* ...then save its name */
			strncpy(st_cpuall_irq->irq_name, li, len);
			st_cpuall_irq->irq_name[len] = '\0';

			/* For each interrupt: Get number received by each CPU */
			for (cpu = 0; cpu < index; cpu++) {
				st_cpu_sum = st_irq + (cpu_index[cpu] + 1) * nr_int;
				st_cpu_irq = st_irq + (cpu_index[cpu] + 1) * nr_int + irq_read;
				/*
				 * Interrupt name is saved only for CPU "all".
				 * Now save current interrupt value for current CPU
				 * and total number of interrupts received by current CPU
				 * and number of current interrupt received by all CPU.
				 */
				st_cpu_irq->irq_nr = strtoul(cp, &next, 10);
				st_cpuall_irq->irq_nr += st_cpu_irq->irq_nr;
				st_cpu_sum->irq_nr += st_cpu_irq->irq_nr;
				cp = next;
			}
			st_cpuall_sum->irq_nr += st_cpuall_irq->irq_nr;
		}
out:
		free(line);
		fclose(fp);
	}

	if (index && !rc) {
		rc = cpu_index[index - 1] + 2;
	}

	free(cpu_index);

	return rc;
}

/*
 ***************************************************************************
 * Read memory statistics from /proc/meminfo.
 *
 * IN:
 * @st_memory	Structure where stats will be saved.
 *
 * OUT:
 * @st_memory	Structure with statistics.
 *
 * RETURNS:
 * 1 on success, 0 otherwise.
 *
 * USED BY:
 * sadc, pidstat
 ***************************************************************************
 */
__nr_t read_meminfo(struct stats_memory *st_memory)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(MEMINFO, "r")) == NULL)
		return 0;

	while (fgets(line, sizeof(line), fp) != NULL) {

		if (!strncmp(line, "MemTotal:", 9)) {
			/* Read the total amount of memory in kB */
			sscanf(line + 9, "%llu", &st_memory->tlmkb);
		}
		else if (!strncmp(line, "MemFree:", 8)) {
			/* Read the amount of free memory in kB */
			sscanf(line + 8, "%llu", &st_memory->frmkb);
		}
		else if (!strncmp(line, "MemAvailable:", 13)) {
			/* Read the amount of available memory in kB */
			sscanf(line + 13, "%llu", &st_memory->availablekb);
		}
		else if (!strncmp(line, "Buffers:", 8)) {
			/* Read the amount of buffered memory in kB */
			sscanf(line + 8, "%llu", &st_memory->bufkb);
		}
		else if (!strncmp(line, "Cached:", 7)) {
			/* Read the amount of cached memory in kB */
			sscanf(line + 7, "%llu", &st_memory->camkb);
		}
		else if (!strncmp(line, "SwapCached:", 11)) {
			/* Read the amount of cached swap in kB */
			sscanf(line + 11, "%llu", &st_memory->caskb);
		}
		else if (!strncmp(line, "Active:", 7)) {
			/* Read the amount of active memory in kB */
			sscanf(line + 7, "%llu", &st_memory->activekb);
		}
		else if (!strncmp(line, "Inactive:", 9)) {
			/* Read the amount of inactive memory in kB */
			sscanf(line + 9, "%llu", &st_memory->inactkb);
		}
		else if (!strncmp(line, "SwapTotal:", 10)) {
			/* Read the total amount of swap memory in kB */
			sscanf(line + 10, "%llu", &st_memory->tlskb);
		}
		else if (!strncmp(line, "SwapFree:", 9)) {
			/* Read the amount of free swap memory in kB */
			sscanf(line + 9, "%llu", &st_memory->frskb);
		}
		else if (!strncmp(line, "Dirty:", 6)) {
			/* Read the amount of dirty memory in kB */
			sscanf(line + 6, "%llu", &st_memory->dirtykb);
		}
		else if (!strncmp(line, "Committed_AS:", 13)) {
			/* Read the amount of commited memory in kB */
			sscanf(line + 13, "%llu", &st_memory->comkb);
		}
		else if (!strncmp(line, "AnonPages:", 10)) {
			/* Read the amount of pages mapped into userspace page tables in kB */
			sscanf(line + 10, "%llu", &st_memory->anonpgkb);
		}
		else if (!strncmp(line, "Slab:", 5)) {
			/* Read the amount of in-kernel data structures cache in kB */
			sscanf(line + 5, "%llu", &st_memory->slabkb);
		}
		else if (!strncmp(line, "KernelStack:", 12)) {
			/* Read the kernel stack utilization in kB */
			sscanf(line + 12, "%llu", &st_memory->kstackkb);
		}
		else if (!strncmp(line, "PageTables:", 11)) {
			/* Read the amount of memory dedicated to the lowest level of page tables in kB */
			sscanf(line + 11, "%llu", &st_memory->pgtblkb);
		}
		else if (!strncmp(line, "VmallocUsed:", 12)) {
			/* Read the amount of vmalloc area which is used in kB */
			sscanf(line + 12, "%llu", &st_memory->vmusedkb);
		}
	}

	fclose(fp);
	return 1;
}



/*
 ***************************************************************************
 * Compute "extended" device statistics (service time, etc.).
 *
 * IN:
 * @sdc		Structure with current device statistics.
 * @sdp		Structure with previous device statistics.
 * @itv		Interval of time in 1/100th of a second.
 *
 * OUT:
 * @xds		Structure with extended statistics.
 *
 * USED BY:
 * sar, sadf, iostat
 ***************************************************************************
*/
void compute_ext_disk_stats(struct stats_disk *sdc, struct stats_disk *sdp,
			    unsigned long long itv, struct ext_disk_stats *xds)
{
	xds->util  = sdc->tot_ticks < sdp->tot_ticks ?
		     0.0 :
		     S_VALUE(sdp->tot_ticks, sdc->tot_ticks, itv);
	/*
	 * Kernel gives ticks already in milliseconds for all platforms
	 * => no need for further scaling.
	 * Origin (unmerged) flush operations are counted as writes.
	 */
	xds->await = (sdc->nr_ios > sdp->nr_ios) ?
		((sdc->rd_ticks - sdp->rd_ticks) + (sdc->wr_ticks - sdp->wr_ticks) + (sdc->dc_ticks - sdp->dc_ticks)) /
		((double) (sdc->nr_ios - sdp->nr_ios)) : 0.0;
	xds->arqsz = (sdc->nr_ios > sdp->nr_ios) ?
		((sdc->rd_sect - sdp->rd_sect) + (sdc->wr_sect - sdp->wr_sect) + (sdc->dc_sect - sdp->dc_sect)) /
		((double) (sdc->nr_ios - sdp->nr_ios)) : 0.0;
}

/*
 ***************************************************************************
 * Since ticks may vary slightly from CPU to CPU, we'll want
 * to recalculate itv based on this CPU's tick count, rather
 * than that reported by the "cpu" line. Otherwise we
 * occasionally end up with slightly skewed figures, with
 * the skew being greater as the time interval grows shorter.
 *
 * IN:
 * @scc	Current sample statistics for current CPU.
 * @scp	Previous sample statistics for current CPU.
 *
 * RETURNS:
 * Interval of time based on current CPU, expressed in jiffies.
 *
 * USED BY:
 * sar, sadf, mpstat
 ***************************************************************************
 */
unsigned long long get_per_cpu_interval(struct stats_cpu *scc,
					struct stats_cpu *scp)
{
	unsigned long long ishift = 0LL;

	if ((scc->cpu_user - scc->cpu_guest) < (scp->cpu_user - scp->cpu_guest)) {
		/*
		 * Sometimes the nr of jiffies spent in guest mode given by the guest
		 * counter in /proc/stat is slightly higher than that included in
		 * the user counter. Update the interval value accordingly.
		 */
		ishift += (scp->cpu_user - scp->cpu_guest) -
		          (scc->cpu_user - scc->cpu_guest);
	}
	if ((scc->cpu_nice - scc->cpu_guest_nice) < (scp->cpu_nice - scp->cpu_guest_nice)) {
		/*
		 * Idem for nr of jiffies spent in guest_nice mode.
		 */
		ishift += (scp->cpu_nice - scp->cpu_guest_nice) -
		          (scc->cpu_nice - scc->cpu_guest_nice);
	}

	/*
	 * Workaround for CPU coming back online: With recent kernels
	 * some fields (user, nice, system) restart from their previous value,
	 * whereas others (idle, iowait) restart from zero.
	 * For the latter we need to set their previous value to zero to
	 * avoid getting an interval value < 0.
	 * (I don't know how the other fields like hardirq, steal... behave).
	 * Don't assume the CPU has come back from offline state if previous
	 * value was greater than ULLONG_MAX - 0x7ffff (the counter probably
	 * overflew).
	 */
	if ((scc->cpu_iowait < scp->cpu_iowait) && (scp->cpu_iowait < (ULLONG_MAX - 0x7ffff))) {
		/*
		 * The iowait value reported by the kernel can also decrement as
		 * a result of inaccurate iowait tracking. Waiting on IO can be
		 * first accounted as iowait but then instead as idle.
		 * Therefore if the idle value during the same period did not
		 * decrease then consider this is a problem with the iowait
		 * reporting and correct the previous value according to the new
		 * reading. Otherwise, treat this as CPU coming back online.
		 */
		if ((scc->cpu_idle > scp->cpu_idle) || (scp->cpu_idle >= (ULLONG_MAX - 0x7ffff))) {
			scp->cpu_iowait = scc->cpu_iowait;
		}
		else {
			scp->cpu_iowait = 0;
		}
	}
	if ((scc->cpu_idle < scp->cpu_idle) && (scp->cpu_idle < (ULLONG_MAX - 0x7ffff))) {
		scp->cpu_idle = 0;
	}

	/*
	 * Don't take cpu_guest and cpu_guest_nice into account
	 * because cpu_user and cpu_nice already include them.
	 */
	return ((scc->cpu_user    + scc->cpu_nice   +
		 scc->cpu_sys     + scc->cpu_iowait +
		 scc->cpu_idle    + scc->cpu_steal  +
		 scc->cpu_hardirq + scc->cpu_softirq) -
		(scp->cpu_user    + scp->cpu_nice   +
		 scp->cpu_sys     + scp->cpu_iowait +
		 scp->cpu_idle    + scp->cpu_steal  +
		 scp->cpu_hardirq + scp->cpu_softirq) +
		 ishift);
}


}