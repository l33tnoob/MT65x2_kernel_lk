/*
 * arch/arm/kernel/topology.c
 *
 * Copyright (C) 2011 Linaro Limited.
 * Written by: Vincent Guittot
 *
 * based on arch/sh/kernel/topology.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/init.h>
#include <linux/percpu.h>
#include <linux/node.h>
#include <linux/nodemask.h>
#ifdef CONFIG_MTK_SCHED_CMP_TGS
#include <linux/of.h>
#endif
#include <linux/sched.h>
#ifdef CONFIG_MTK_SCHED_CMP_TGS
#include <linux/slab.h>
#endif
#include <asm/cputype.h>
#ifdef CONFIG_MTK_SCHED_CMP_TGS
#include <asm/smp_plat.h>
#endif
#include <asm/topology.h>

#define MPIDR_SMP_BITMASK (0x3 << 30)
#define MPIDR_SMP_VALUE (0x2 << 30)

#define MPIDR_MT_BITMASK (0x1 << 24)

/*
 * These masks reflect the current use of the affinity levels.
 * The affinity level can be up to 16 bits according to ARM ARM
 */

#define MPIDR_LEVEL0_MASK 0x3
#define MPIDR_LEVEL0_SHIFT 0

#define MPIDR_LEVEL1_MASK 0xF
#define MPIDR_LEVEL1_SHIFT 8

#define MPIDR_LEVEL2_MASK 0xFF
#define MPIDR_LEVEL2_SHIFT 16

struct cputopo_arm cpu_topology[NR_CPUS];

#ifdef CONFIG_MTK_SCHED_CMP_TGS
#define CPU_DOMAIN_BUGON
DEFINE_PER_CPU(struct cpu_domain *, cmp_cpu_domain) = {NULL};
spinlock_t cluster_lock;
LIST_HEAD(cpu_domains);
static unsigned int nr_clusters = 0;
static void init_cpu_domains(int cpuid, int socket_id);
#endif
#ifdef CONFIG_MTK_SCHED_CMP_PACK_SMALL_TASK
int arch_sd_share_power_line(void)
{
	return 0*SD_SHARE_POWERLINE;
}
#endif /* CONFIG_MTK_SCHED_CMP_PACK_SMALL_TASK */
const struct cpumask *cpu_coregroup_mask(int cpu)
{
	return &cpu_topology[cpu].core_sibling;
}

/*
 * store_cpu_topology is called at boot when only one cpu is running
 * and with the mutex cpu_hotplug.lock locked, when several cpus have booted,
 * which prevents simultaneous write access to cpu_topology array
 */
void store_cpu_topology(unsigned int cpuid)
{
	struct cputopo_arm *cpuid_topo = &cpu_topology[cpuid];
	unsigned int mpidr;
	unsigned int cpu;

	/* If the cpu topology has been already set, just return */
	if (cpuid_topo->core_id != -1)
		return;

	mpidr = read_cpuid_mpidr();

	/* create cpu topology mapping */
	if ((mpidr & MPIDR_SMP_BITMASK) == MPIDR_SMP_VALUE) {
		/*
		 * This is a multiprocessor system
		 * multiprocessor format & multiprocessor mode field are set
		 */

		if (mpidr & MPIDR_MT_BITMASK) {
			/* core performance interdependency */
			cpuid_topo->thread_id = (mpidr >> MPIDR_LEVEL0_SHIFT)
				& MPIDR_LEVEL0_MASK;
			cpuid_topo->core_id = (mpidr >> MPIDR_LEVEL1_SHIFT)
				& MPIDR_LEVEL1_MASK;
			cpuid_topo->socket_id = (mpidr >> MPIDR_LEVEL2_SHIFT)
				& MPIDR_LEVEL2_MASK;
		} else {
			/* largely independent cores */
			cpuid_topo->thread_id = -1;
			cpuid_topo->core_id = (mpidr >> MPIDR_LEVEL0_SHIFT)
				& MPIDR_LEVEL0_MASK;
			cpuid_topo->socket_id = (mpidr >> MPIDR_LEVEL1_SHIFT)
				& MPIDR_LEVEL1_MASK;
		}
	} else {
		/*
		 * This is an uniprocessor system
		 * we are in multiprocessor format but uniprocessor system
		 * or in the old uniprocessor format
		 */
		cpuid_topo->thread_id = -1;
		cpuid_topo->core_id = 0;
		cpuid_topo->socket_id = -1;
	}
#ifdef CONFIG_MTK_SCHED_CMP_TGS
	if(cpuid_topo->socket_id != -1)	
		init_cpu_domains(cpuid, cpuid_topo->socket_id);
#endif  
	/* update core and thread sibling masks */
	for_each_possible_cpu(cpu) {
		struct cputopo_arm *cpu_topo = &cpu_topology[cpu];

		if (cpuid_topo->socket_id == cpu_topo->socket_id) {
			cpumask_set_cpu(cpuid, &cpu_topo->core_sibling);
			if (cpu != cpuid)
				cpumask_set_cpu(cpu,
					&cpuid_topo->core_sibling);

			if (cpuid_topo->core_id == cpu_topo->core_id) {
				cpumask_set_cpu(cpuid,
					&cpu_topo->thread_sibling);
				if (cpu != cpuid)
					cpumask_set_cpu(cpu,
						&cpuid_topo->thread_sibling);
			}
		}
	}
	smp_wmb();

	printk(KERN_INFO "CPU%u: thread %d, cpu %d, socket %d, mpidr %x\n",
		cpuid, cpu_topology[cpuid].thread_id,
		cpu_topology[cpuid].core_id,
		cpu_topology[cpuid].socket_id, mpidr);
}

/*
 * init_cpu_topology is called at boot when only one cpu is running
 * which prevent simultaneous write access to cpu_topology array
 */
void init_cpu_topology(void)
{
	unsigned int cpu;

	/* init core mask */
	for_each_possible_cpu(cpu) {
		struct cputopo_arm *cpu_topo = &(cpu_topology[cpu]);

		cpu_topo->thread_id = -1;
		cpu_topo->core_id =  -1;
		cpu_topo->socket_id = -1;
		cpumask_clear(&cpu_topo->core_sibling);
		cpumask_clear(&cpu_topo->thread_sibling);
	}
	smp_wmb();
}
#ifdef CONFIG_MTK_SCHED_CMP_TGS
int cluster_id(int cpu)
{ 
	if((cpu >= 0) && (cpu < NR_CPUS) && (per_cpu(cmp_cpu_domain, cpu) != NULL))
		return cmp_cpu_domain(cpu)->cluster_id;
	else {
#ifdef CPU_DOMAIN_BUGON
		BUG_ON(1);
#endif  	
		return -1;
	}	
}
int nr_cpu_of_cluster(int cluster_id, bool exclusiveOffline)
{
	unsigned char found = 0;
	struct cpumask dst;
	struct	cpu_domain *cluster;
	struct list_head *pos;
	
	if(!list_empty(&cpu_domains))
	{
		list_for_each(pos, &cpu_domains) {
			cluster = list_entry(pos, struct cpu_domain, cpu_domains);
			if(cluster->cluster_id == cluster_id) //Found
			{
				found =1;
				break;
			}
		}
		if(!found) {
#ifdef CPU_DOMAIN_BUGON
			BUG_ON(1);
#endif 				
			return -1;
		}
		if(exclusiveOffline)
			dst = cluster->cpus;
		else
			dst = cluster->possible_cpus;
		return cpumask_weight(&dst);
	}
	else {
#ifdef CPU_DOMAIN_BUGON
		BUG_ON(1);
#endif 		
		return -1;
	}	 
}
unsigned int cluster_nr(void)
{
	return nr_clusters;
}
struct cpu_domain *get_cpu_domain(int cpu)
{
	if(per_cpu(cmp_cpu_domain, cpu) != NULL)
  	return cmp_cpu_domain(cpu);
  else
  	return NULL;
}

struct cpumask *get_domain_cpus(int cluster_id, bool exclusiveOffline)
{
	unsigned char found = 0;
	struct	cpu_domain *cluster;
	struct list_head *pos;
	
	if(!list_empty(&cpu_domains))
	{
		list_for_each(pos, &cpu_domains) {
			cluster = list_entry(pos, struct cpu_domain, cpu_domains);
			if(cluster->cluster_id == cluster_id) //Found
			{
				found =1;
				break;
			}
		}
		if(!found) {				
			return 0;
		}
		if(exclusiveOffline)
			return &cluster->cpus;
		else
			return &cluster->possible_cpus;
	}
	else {
#ifdef CPU_DOMAIN_BUGON
		BUG_ON(1);
#endif 		
		return 0;
	}
}

void init_cpu_domain_early(void)
{
	unsigned int mpidr;
	int socket_id;
	mpidr = read_cpuid_mpidr();

	if ((mpidr & MPIDR_SMP_BITMASK) == MPIDR_SMP_VALUE) {
		if (mpidr & MPIDR_MT_BITMASK) {
			socket_id = (mpidr >> MPIDR_LEVEL2_SHIFT)
				& MPIDR_LEVEL2_MASK;
		} else {
			/* largely independent cores */
			socket_id = (mpidr >> MPIDR_LEVEL1_SHIFT)
				& MPIDR_LEVEL1_MASK;
		}
	} else {
		socket_id = -1;
	}
	spin_lock_init (&cluster_lock);
	if(socket_id != -1)
		init_cpu_domains(smp_processor_id(), socket_id);
}
static void init_cpu_domains(int cpuid, int socket_id)
{
	
	struct	cpu_domain *cluster;
	struct list_head *pos;
	
	if(per_cpu(cmp_cpu_domain, cpuid) == NULL)
	{
		spin_lock(&cluster_lock);
		if(list_empty(&cpu_domains))
		{
			cluster = (struct cpu_domain *)
				kmalloc(sizeof(struct cpu_domain), GFP_ATOMIC);
				memset(cluster, 0, sizeof(struct cpu_domain));
			cluster->cluster_id = socket_id;
			list_add(&cluster->cpu_domains, &cpu_domains);
			per_cpu(cmp_cpu_domain, cpuid) = cluster;
			cpumask_set_cpu(cpuid, &cluster->possible_cpus);
			cpumask_and(&cluster->cpus, cpu_online_mask, &cluster->possible_cpus);
			nr_clusters++;				
		}
		else
		{
			list_for_each(pos, &cpu_domains) {
				cluster = list_entry(pos, struct cpu_domain, cpu_domains);
				if(cluster->cluster_id == socket_id)
				{

					per_cpu(cmp_cpu_domain, cpuid) = cluster;
					cpumask_set_cpu(cpuid, &cluster->possible_cpus);
					cpumask_and(&cluster->cpus, cpu_online_mask, &cluster->possible_cpus);
					//printk("cpu%d cluster info is found\n", cpuid);
			  	break;
			  }
			}
			if(pos == &cpu_domains)//Not found, allocate one
			{
				cluster = (struct cpu_domain *)
				kmalloc(sizeof(struct cpu_domain), GFP_ATOMIC);
				memset(cluster, 0, sizeof(struct cpu_domain));
				cluster->cluster_id = socket_id;
				list_add(&cluster->cpu_domains, &cpu_domains);
				per_cpu(cmp_cpu_domain, cpuid) = cluster;
				cpumask_set_cpu(cpuid, &cluster->possible_cpus);
				cpumask_and(&cluster->cpus, cpu_online_mask, &cluster->possible_cpus);
				nr_clusters++;
			}
		}
		spin_unlock(&cluster_lock);
	}	
}
#endif
