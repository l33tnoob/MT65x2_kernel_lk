/*
 *  drivers/cpufreq/cpufreq_hotplug.c
 *
 *  Copyright (C)  2001 Russell King
 *            (C)  2003 Venkatesh Pallipadi <venkatesh.pallipadi@intel.com>.
 *                      Jun Nakajima <jun.nakajima@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/jiffies.h>
#include <linux/kernel_stat.h>
#include <linux/mutex.h>
#include <linux/hrtimer.h>
#include <linux/tick.h>
#include <linux/ktime.h>
#include <linux/sched.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/du_timer_observer.h>

/*
 * dbs is used in this file as a shortform for demandbased switching
 * It helps to keep variable names smaller, simpler
 */

#define DEF_FREQUENCY_DOWN_DIFFERENTIAL     (10)
#define DEF_FREQUENCY_UP_THRESHOLD          (80)
#define DEF_SAMPLING_DOWN_FACTOR            (1)
#define MAX_SAMPLING_DOWN_FACTOR            (100000)
#define MICRO_FREQUENCY_DOWN_DIFFERENTIAL   (15)
#define MIN_FREQUENCY_DOWN_DIFFERENTIAL     (5)
#define MAX_FREQUENCY_DOWN_DIFFERENTIAL     (20)
#define MICRO_FREQUENCY_UP_THRESHOLD        (85)
#define MICRO_FREQUENCY_MIN_SAMPLE_RATE     (30000)
#define MIN_FREQUENCY_UP_THRESHOLD          (21)
#define MAX_FREQUENCY_UP_THRESHOLD          (100)

/*
 * cpu hotplug - macro
 */
#define DEF_CPU_DOWN_DIFFERENTIAL           (10)
#define MICRO_CPU_DOWN_DIFFERENTIAL         (10)
#define MIN_CPU_DOWN_DIFFERENTIAL           (0)
#define MAX_CPU_DOWN_DIFFERENTIAL           (30)

#define DEF_CPU_UP_THRESHOLD                (90)
#define MICRO_CPU_UP_THRESHOLD              (90)
#define MIN_CPU_UP_THRESHOLD                (50)
#define MAX_CPU_UP_THRESHOLD                (100)

#define DEF_CPU_UP_AVG_TIMES                (10)
#define MIN_CPU_UP_AVG_TIMES                (1)
#define MAX_CPU_UP_AVG_TIMES                (20)

#define DEF_CPU_DOWN_AVG_TIMES              (100)
#define MIN_CPU_DOWN_AVG_TIMES              (20)
#define MAX_CPU_DOWN_AVG_TIMES              (200)

#define DEF_CPU_INPUT_BOOST_ENABLE          (1)
#define DEF_CPU_INPUT_BOOST_NUM             (2)

#define DEF_CPU_RUSH_BOOST_ENABLE           (1)

#define DEF_CPU_RUSH_THRESHOLD              (98)
#define MICRO_CPU_RUSH_THRESHOLD            (98)
#define MIN_CPU_RUSH_THRESHOLD              (80)
#define MAX_CPU_RUSH_THRESHOLD              (100)

#define DEF_CPU_RUSH_AVG_TIMES              (5)
#define MIN_CPU_RUSH_AVG_TIMES              (1)
#define MAX_CPU_RUSH_AVG_TIMES              (10)

#define DEF_CPU_RUSH_TLP_TIMES              (5)
#define MIN_CPU_RUSH_TLP_TIMES              (1)
#define MAX_CPU_RUSH_TLP_TIMES              (10)

/*
 * cpu hotplug - enum
 */
typedef enum {
    CPU_HOTPLUG_WORK_TYPE_NONE = 0,
    CPU_HOTPLUG_WORK_TYPE_BASE,
    CPU_HOTPLUG_WORK_TYPE_LIMIT,
    CPU_HOTPLUG_WORK_TYPE_UP,
    CPU_HOTPLUG_WORK_TYPE_DOWN,
    CPU_HOTPLUG_WORK_TYPE_RUSH,
} cpu_hotplug_work_type_t;

//#define DEBUG_LOG
#define DBS_CHECK_CPU_LOG

/*
 * The polling frequency of this governor depends on the capability of
 * the processor. Default polling frequency is 1000 times the transition
 * latency of the processor. The governor will work on any processor with
 * transition latency <= 10mS, using appropriate sampling
 * rate.
 * For CPUs with transition latency > 10mS (mostly drivers with CPUFREQ_ETERNAL)
 * this governor will not work.
 * All times here are in uS.
 */
#define MIN_SAMPLING_RATE_RATIO			(2)

static unsigned int min_sampling_rate;
static unsigned int min_sampling_rate_change;

#ifdef DBS_CHECK_CPU_LOG
static unsigned int dbs_check_cpu_count = 0;
static unsigned long dbs_check_cpu_timeout = 0;
#endif

#define LATENCY_MULTIPLIER			(1000)
#define MIN_LATENCY_MULTIPLIER			(100)
#define TRANSITION_LATENCY_LIMIT		(10 * 1000 * 1000)

static void do_dbs_timer(struct work_struct *work);
static int cpufreq_governor_dbs(struct cpufreq_policy *policy,
				unsigned int event);

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_HOTPLUG
static
#endif
struct cpufreq_governor cpufreq_gov_hotplug = {
       .name                   = "hotplug",
       .governor               = cpufreq_governor_dbs,
       .max_transition_latency = TRANSITION_LATENCY_LIMIT,
       .owner                  = THIS_MODULE,
};

/*
 * cpu hotplug - global variable, function declaration & definition
 */
int g_cpus_sum_load_current = 0;   //set global for information purpose
#ifdef CONFIG_HOTPLUG_CPU

long g_cpu_up_sum_load = 0;
int g_cpu_up_count = 0;
int g_cpu_up_load_index = 0;
long g_cpu_up_load_history[MAX_CPU_UP_AVG_TIMES] = {0};

long g_cpu_down_sum_load = 0;
int g_cpu_down_count = 0;
int g_cpu_down_load_index = 0;
long g_cpu_down_load_history[MAX_CPU_DOWN_AVG_TIMES] = {0};

cpu_hotplug_work_type_t g_trigger_hp_work = 0;
unsigned int g_next_hp_action = 0;
struct delayed_work hp_work;

int g_tlp_avg_current = 0;       //set global for information purpose
int g_tlp_avg_sum = 0;
int g_tlp_avg_count = 0;
int g_tlp_avg_index = 0;
int g_tlp_avg_average = 0;       //set global for information purpose
int g_tlp_avg_history[MAX_CPU_RUSH_TLP_TIMES] = {0};

int g_tlp_iowait_av = 0;

int g_cpu_rush_count = 0;

static void hp_reset_strategy_nolock(void);
static void hp_reset_strategy(void);

static void hp_to_callback(void);

#else //#ifdef CONFIG_HOTPLUG_CPU

static void hp_reset_strategy_nolock(void) {};

#endif

/*
 * dvfs - function declaration
 */
static void dbs_freq_increase(struct cpufreq_policy *p, unsigned int freq);

#if defined(CONFIG_THERMAL_LIMIT_TEST)
extern unsigned int mt_cpufreq_thermal_test_limited_load(void);
#endif

/* Sampling types */
enum {DBS_NORMAL_SAMPLE, DBS_SUB_SAMPLE};

struct cpu_dbs_info_s {
	cputime64_t prev_cpu_idle;
	cputime64_t prev_cpu_iowait;
	cputime64_t prev_cpu_wall;
	cputime64_t prev_cpu_nice;
	struct cpufreq_policy *cur_policy;
	struct delayed_work work;
	struct cpufreq_frequency_table *freq_table;
	unsigned int freq_lo;
	unsigned int freq_lo_jiffies;
	unsigned int freq_hi_jiffies;
	unsigned int rate_mult;
	int cpu;
	unsigned int sample_type:1;
	/*
	 * percpu mutex that serializes governor limit change with
	 * do_dbs_timer invocation. We do not want do_dbs_timer to run
	 * when user is changing the governor or limits.
	 */
	struct mutex timer_mutex;
};
static DEFINE_PER_CPU(struct cpu_dbs_info_s, hp_cpu_dbs_info);

static unsigned int dbs_enable;	/* number of CPUs using this policy */
static unsigned int dbs_ignore = 1;
static unsigned int dbs_thermal_limited = 0;
static unsigned int dbs_thermal_limited_freq = 0;

/*
 * dbs_mutex protects dbs_enable in governor start/stop.
 */
static DEFINE_MUTEX(dbs_mutex);

/*
 * dbs_hotplug protects all hotplug related global variables
 */
static DEFINE_MUTEX(hp_mutex);

DEFINE_MUTEX(hp_onoff_mutex);

static struct dbs_tuners {
    unsigned int sampling_rate;
    unsigned int up_threshold;
    unsigned int down_differential;
    unsigned int ignore_nice;
    unsigned int sampling_down_factor;
    unsigned int powersave_bias;
    unsigned int io_is_busy;
    unsigned int cpu_up_threshold;
    unsigned int cpu_down_differential;
    unsigned int cpu_up_avg_times;
    unsigned int cpu_down_avg_times;
    unsigned int cpu_num_limit;
    unsigned int cpu_num_base;
    unsigned int is_cpu_hotplug_disable;
    unsigned int cpu_input_boost_enable;
    unsigned int cpu_input_boost_num;
    unsigned int cpu_rush_boost_enable;
    unsigned int cpu_rush_boost_num;
    unsigned int cpu_rush_threshold;
    unsigned int cpu_rush_tlp_times;
    unsigned int cpu_rush_avg_times;
} dbs_tuners_ins = {
    .up_threshold = DEF_FREQUENCY_UP_THRESHOLD,
    .sampling_down_factor = DEF_SAMPLING_DOWN_FACTOR,
    .down_differential = DEF_FREQUENCY_DOWN_DIFFERENTIAL,
    .ignore_nice = 0,
    .powersave_bias = 0,
    .cpu_up_threshold = DEF_CPU_UP_THRESHOLD,
    .cpu_down_differential = DEF_CPU_DOWN_DIFFERENTIAL,
    .cpu_up_avg_times = DEF_CPU_UP_AVG_TIMES,
    .cpu_down_avg_times = DEF_CPU_DOWN_AVG_TIMES,
    .cpu_num_limit = 1,
    .cpu_num_base = 1,
    .is_cpu_hotplug_disable = 1,
    .cpu_input_boost_enable = DEF_CPU_INPUT_BOOST_ENABLE,
    .cpu_input_boost_num = DEF_CPU_INPUT_BOOST_NUM,
    .cpu_rush_boost_enable = DEF_CPU_RUSH_BOOST_ENABLE,
    .cpu_rush_boost_num = NR_CPUS,
    .cpu_rush_threshold = DEF_CPU_RUSH_THRESHOLD,
    .cpu_rush_tlp_times = DEF_CPU_RUSH_TLP_TIMES,
    .cpu_rush_avg_times = DEF_CPU_RUSH_AVG_TIMES,
};


/* dvfs thermal limit */
void dbs_freq_thermal_limited(unsigned int limited, unsigned int freq)
{
    dbs_thermal_limited = limited;
    dbs_thermal_limited_freq = freq;
}
EXPORT_SYMBOL(dbs_freq_thermal_limited);

static inline u64 get_cpu_idle_time_jiffy(unsigned int cpu, u64 *wall)
{
	u64 idle_time;
	u64 cur_wall_time;
	u64 busy_time;

	cur_wall_time = jiffies64_to_cputime64(get_jiffies_64());

	busy_time  = kcpustat_cpu(cpu).cpustat[CPUTIME_USER];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SYSTEM];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_IRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_SOFTIRQ];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_STEAL];
	busy_time += kcpustat_cpu(cpu).cpustat[CPUTIME_NICE];

	idle_time = cur_wall_time - busy_time;
	if (wall)
		*wall = jiffies_to_usecs(cur_wall_time);

	return jiffies_to_usecs(idle_time);
}

static inline cputime64_t get_cpu_idle_time(unsigned int cpu, cputime64_t *wall)
{
	u64 idle_time = get_cpu_idle_time_us(cpu, NULL);

	if (idle_time == -1ULL)
		return get_cpu_idle_time_jiffy(cpu, wall);
	else
		idle_time += get_cpu_iowait_time_us(cpu, wall);

	return idle_time;
}

static inline cputime64_t get_cpu_iowait_time(unsigned int cpu, cputime64_t *wall)
{
	u64 iowait_time = get_cpu_iowait_time_us(cpu, wall);

	if (iowait_time == -1ULL)
		return 0;

	return iowait_time;
}

/*
 * Find right freq to be set now with powersave_bias on.
 * Returns the freq_hi to be used right now and will set freq_hi_jiffies,
 * freq_lo, and freq_lo_jiffies in percpu area for averaging freqs.
 */
static unsigned int powersave_bias_target(struct cpufreq_policy *policy,
					  unsigned int freq_next,
					  unsigned int relation)
{
	unsigned int freq_req, freq_reduc, freq_avg;
	unsigned int freq_hi, freq_lo;
	unsigned int index = 0;
	unsigned int jiffies_total, jiffies_hi, jiffies_lo;
	struct cpu_dbs_info_s *dbs_info = &per_cpu(hp_cpu_dbs_info,
						   policy->cpu);

	if (!dbs_info->freq_table) {
		dbs_info->freq_lo = 0;
		dbs_info->freq_lo_jiffies = 0;
		return freq_next;
	}

	cpufreq_frequency_table_target(policy, dbs_info->freq_table, freq_next,
			relation, &index);
	freq_req = dbs_info->freq_table[index].frequency;
	freq_reduc = freq_req * dbs_tuners_ins.powersave_bias / 1000;
	freq_avg = freq_req - freq_reduc;

	/* Find freq bounds for freq_avg in freq_table */
	index = 0;
	cpufreq_frequency_table_target(policy, dbs_info->freq_table, freq_avg,
			CPUFREQ_RELATION_H, &index);
	freq_lo = dbs_info->freq_table[index].frequency;
	index = 0;
	cpufreq_frequency_table_target(policy, dbs_info->freq_table, freq_avg,
			CPUFREQ_RELATION_L, &index);
	freq_hi = dbs_info->freq_table[index].frequency;

	/* Find out how long we have to be in hi and lo freqs */
	if (freq_hi == freq_lo) {
		dbs_info->freq_lo = 0;
		dbs_info->freq_lo_jiffies = 0;
		return freq_lo;
	}
	jiffies_total = usecs_to_jiffies(dbs_tuners_ins.sampling_rate);
	jiffies_hi = (freq_avg - freq_lo) * jiffies_total;
	jiffies_hi += ((freq_hi - freq_lo) / 2);
	jiffies_hi /= (freq_hi - freq_lo);
	jiffies_lo = jiffies_total - jiffies_hi;
	dbs_info->freq_lo = freq_lo;
	dbs_info->freq_lo_jiffies = jiffies_lo;
	dbs_info->freq_hi_jiffies = jiffies_hi;
	return freq_hi;
}

static void hotplug_powersave_bias_init_cpu(int cpu)
{
	struct cpu_dbs_info_s *dbs_info = &per_cpu(hp_cpu_dbs_info, cpu);
	dbs_info->freq_table = cpufreq_frequency_get_table(cpu);
	dbs_info->freq_lo = 0;
}

static void hotplug_powersave_bias_init(void)
{
	int i;
	for_each_online_cpu(i) {
		hotplug_powersave_bias_init_cpu(i);
	}
}

/************************** sysfs interface ************************/

static ssize_t show_sampling_rate_min(struct kobject *kobj,
				      struct attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", min_sampling_rate);
}

define_one_global_ro(sampling_rate_min);

/* cpufreq_hotplug Governor Tunables */
#define show_one(file_name, object)					\
static ssize_t show_##file_name						\
(struct kobject *kobj, struct attribute *attr, char *buf)              \
{									\
	return sprintf(buf, "%u\n", dbs_tuners_ins.object);		\
}

show_one(sampling_rate, sampling_rate);
show_one(io_is_busy, io_is_busy);
show_one(up_threshold, up_threshold);
show_one(down_differential, down_differential);
show_one(sampling_down_factor, sampling_down_factor);
show_one(ignore_nice_load, ignore_nice);
show_one(powersave_bias, powersave_bias);
show_one(cpu_up_threshold, cpu_up_threshold);
show_one(cpu_down_differential, cpu_down_differential);
show_one(cpu_up_avg_times, cpu_up_avg_times);
show_one(cpu_down_avg_times, cpu_down_avg_times);
show_one(cpu_num_limit, cpu_num_limit);
show_one(cpu_num_base, cpu_num_base);
show_one(is_cpu_hotplug_disable, is_cpu_hotplug_disable);
show_one(cpu_input_boost_enable, cpu_input_boost_enable);
show_one(cpu_input_boost_num, cpu_input_boost_num);
show_one(cpu_rush_boost_enable, cpu_rush_boost_enable);
show_one(cpu_rush_boost_num, cpu_rush_boost_num);
show_one(cpu_rush_threshold, cpu_rush_threshold);
show_one(cpu_rush_tlp_times, cpu_rush_tlp_times);
show_one(cpu_rush_avg_times, cpu_rush_avg_times);

/**
 * update_sampling_rate - update sampling rate effective immediately if needed.
 * @new_rate: new sampling rate
 *
 * If new rate is smaller than the old, simply updaing
 * dbs_tuners_int.sampling_rate might not be appropriate. For example,
 * if the original sampling_rate was 1 second and the requested new sampling
 * rate is 10 ms because the user needs immediate reaction from hotplug
 * governor, but not sure if higher frequency will be required or not,
 * then, the governor may change the sampling rate too late; up to 1 second
 * later. Thus, if we are reducing the sampling rate, we need to make the
 * new value effective immediately.
 */
static void update_sampling_rate(unsigned int new_rate)
{
	int cpu;

	dbs_tuners_ins.sampling_rate = new_rate
				     = max(new_rate, min_sampling_rate);

	for_each_online_cpu(cpu) {
		struct cpufreq_policy *policy;
		struct cpu_dbs_info_s *dbs_info;
		unsigned long next_sampling, appointed_at;

		policy = cpufreq_cpu_get(cpu);
		if (!policy)
			continue;
		dbs_info = &per_cpu(hp_cpu_dbs_info, policy->cpu);
		cpufreq_cpu_put(policy);

		mutex_lock(&dbs_info->timer_mutex);

		if (!delayed_work_pending(&dbs_info->work)) {
			mutex_unlock(&dbs_info->timer_mutex);
			continue;
		}

		next_sampling  = jiffies + usecs_to_jiffies(new_rate);
		appointed_at = dbs_info->work.timer.expires;


		if (time_before(next_sampling, appointed_at)) {

			mutex_unlock(&dbs_info->timer_mutex);
			cancel_delayed_work_sync(&dbs_info->work);
			mutex_lock(&dbs_info->timer_mutex);

			to_init(0, "dvfs kworker", new_rate * 1000 * 10, NULL);
			schedule_delayed_work_on(dbs_info->cpu, &dbs_info->work,
						 usecs_to_jiffies(new_rate));

		}
		mutex_unlock(&dbs_info->timer_mutex);
	}
}

static ssize_t store_sampling_rate(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	update_sampling_rate(input);
	return count;
}

static ssize_t store_io_is_busy(struct kobject *a, struct attribute *b,
				   const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;
	dbs_tuners_ins.io_is_busy = !!input;
	return count;
}

static ssize_t store_up_threshold(struct kobject *a, struct attribute *b,
				  const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_FREQUENCY_UP_THRESHOLD ||
			input < MIN_FREQUENCY_UP_THRESHOLD) {
		return -EINVAL;
	}
	dbs_tuners_ins.up_threshold = input;
	return count;
}

static ssize_t store_down_differential(struct kobject *a, struct attribute *b,
                const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_FREQUENCY_DOWN_DIFFERENTIAL ||
			input < MIN_FREQUENCY_DOWN_DIFFERENTIAL) {
		return -EINVAL;
	}
	dbs_tuners_ins.down_differential = input;
	return count;
}

static ssize_t store_sampling_down_factor(struct kobject *a,
			struct attribute *b, const char *buf, size_t count)
{
	unsigned int input, j;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_SAMPLING_DOWN_FACTOR || input < 1)
		return -EINVAL;
	dbs_tuners_ins.sampling_down_factor = input;

	/* Reset down sampling multiplier in case it was active */
	for_each_online_cpu(j) {
		struct cpu_dbs_info_s *dbs_info;
		dbs_info = &per_cpu(hp_cpu_dbs_info, j);
		dbs_info->rate_mult = 1;
	}
	return count;
}

static ssize_t store_ignore_nice_load(struct kobject *a, struct attribute *b,
				      const char *buf, size_t count)
{
	unsigned int input;
	int ret;

	unsigned int j;

	ret = sscanf(buf, "%u", &input);
	if (ret != 1)
		return -EINVAL;

	if (input > 1)
		input = 1;

	if (input == dbs_tuners_ins.ignore_nice) { /* nothing to do */
		return count;
	}
	dbs_tuners_ins.ignore_nice = input;

	/* we need to re-evaluate prev_cpu_idle */
	for_each_online_cpu(j) {
		struct cpu_dbs_info_s *dbs_info;
		dbs_info = &per_cpu(hp_cpu_dbs_info, j);
		dbs_info->prev_cpu_idle = get_cpu_idle_time(j,
						&dbs_info->prev_cpu_wall);
		if (dbs_tuners_ins.ignore_nice)
			dbs_info->prev_cpu_nice = kcpustat_cpu(j).cpustat[CPUTIME_NICE];

	}
	return count;
}

static ssize_t store_powersave_bias(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1)
		return -EINVAL;

	if (input > 1000)
		input = 1000;

	dbs_tuners_ins.powersave_bias = input;
	hotplug_powersave_bias_init();
	return count;
}

/*
 * cpu hotplug - store_xxx function definition
 */
static ssize_t store_cpu_up_threshold(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_CPU_UP_THRESHOLD ||
		input < MIN_CPU_UP_THRESHOLD) {
		return -EINVAL;
	}
	
	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_up_threshold = input;
	hp_reset_strategy_nolock();
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_down_differential(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_CPU_DOWN_DIFFERENTIAL ||
		input < MIN_CPU_DOWN_DIFFERENTIAL) {
		return -EINVAL;
	}
	
	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_down_differential = input;
	hp_reset_strategy_nolock();
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_up_avg_times(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_CPU_UP_AVG_TIMES ||
		input < MIN_CPU_UP_AVG_TIMES) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_up_avg_times = input;
	hp_reset_strategy_nolock();
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_down_avg_times(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_CPU_DOWN_AVG_TIMES ||
		input < MIN_CPU_DOWN_AVG_TIMES) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_down_avg_times = input;
	hp_reset_strategy_nolock();
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_num_limit(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > num_possible_cpus() ||
		input < 1) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_num_limit = input;
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_num_base(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	unsigned int online_cpus_count;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > num_possible_cpus() ||
		input < 1) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);

	dbs_tuners_ins.cpu_num_base = input;
	online_cpus_count = num_online_cpus();
#ifdef CONFIG_HOTPLUG_CPU
	if (online_cpus_count < input && online_cpus_count < dbs_tuners_ins.cpu_num_limit)
	{
		struct cpu_dbs_info_s *this_dbs_info;
		struct cpufreq_policy *policy;
		
		this_dbs_info = &per_cpu(hp_cpu_dbs_info, 0);
		policy = this_dbs_info->cur_policy;
		
		dbs_freq_increase(policy, policy->max);
		g_trigger_hp_work = CPU_HOTPLUG_WORK_TYPE_BASE;
		to_init(11, "hotplug kworker", 100, NULL);
		schedule_delayed_work_on(0, &hp_work, 0);
	}
#endif

	mutex_unlock(&hp_mutex);
	
	return count;
}

static ssize_t store_is_cpu_hotplug_disable(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 1 ||
		input < 0) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	if (dbs_tuners_ins.is_cpu_hotplug_disable && !input)
		hp_reset_strategy_nolock();
	dbs_tuners_ins.is_cpu_hotplug_disable = input;
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_input_boost_enable(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 1 ||
		input < 0) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_input_boost_enable = input;
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_input_boost_num(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > num_possible_cpus() ||
		input < 2) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_input_boost_num = input;
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_rush_boost_enable(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > 1 ||
		input < 0) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_rush_boost_enable = input;
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_rush_boost_num(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > num_possible_cpus() ||
		input < 2) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_rush_boost_num = input;
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_rush_threshold(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_CPU_RUSH_THRESHOLD ||
		input < MIN_CPU_RUSH_THRESHOLD) {
		return -EINVAL;
	}
	
	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_rush_threshold = input;
	//hp_reset_strategy_nolock(); //no need
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_rush_tlp_times(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_CPU_RUSH_TLP_TIMES ||
		input < MIN_CPU_RUSH_TLP_TIMES) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_rush_tlp_times = input;
	hp_reset_strategy_nolock();
	mutex_unlock(&hp_mutex);

	return count;
}

static ssize_t store_cpu_rush_avg_times(struct kobject *a, struct attribute *b,
				    const char *buf, size_t count)
{
	unsigned int input;
	int ret;
	ret = sscanf(buf, "%u", &input);

	if (ret != 1 || input > MAX_CPU_RUSH_AVG_TIMES ||
		input < MIN_CPU_RUSH_AVG_TIMES) {
		return -EINVAL;
	}

	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_rush_avg_times = input;
	hp_reset_strategy_nolock();
	mutex_unlock(&hp_mutex);

	return count;
}

define_one_global_rw(sampling_rate);
define_one_global_rw(io_is_busy);
define_one_global_rw(up_threshold);
define_one_global_rw(down_differential);
define_one_global_rw(sampling_down_factor);
define_one_global_rw(ignore_nice_load);
define_one_global_rw(powersave_bias);
define_one_global_rw(cpu_up_threshold);
define_one_global_rw(cpu_down_differential);
define_one_global_rw(cpu_up_avg_times);
define_one_global_rw(cpu_down_avg_times);
define_one_global_rw(cpu_num_limit);
define_one_global_rw(cpu_num_base);
define_one_global_rw(is_cpu_hotplug_disable);
define_one_global_rw(cpu_input_boost_enable);
define_one_global_rw(cpu_input_boost_num);
define_one_global_rw(cpu_rush_boost_enable);
define_one_global_rw(cpu_rush_boost_num);
define_one_global_rw(cpu_rush_threshold);
define_one_global_rw(cpu_rush_tlp_times);
define_one_global_rw(cpu_rush_avg_times);

static struct attribute *dbs_attributes[] = {
    &sampling_rate_min.attr,
    &sampling_rate.attr,
    &up_threshold.attr,
    &down_differential.attr,
    &sampling_down_factor.attr,
    &ignore_nice_load.attr,
    &powersave_bias.attr,
    &io_is_busy.attr,
    &cpu_up_threshold.attr,
    &cpu_down_differential.attr,
    &cpu_up_avg_times.attr,
    &cpu_down_avg_times.attr,
    &cpu_num_limit.attr,
    &cpu_num_base.attr,
    &is_cpu_hotplug_disable.attr,
    &cpu_input_boost_enable.attr,
    &cpu_input_boost_num.attr,
    &cpu_rush_boost_enable.attr,
    &cpu_rush_boost_num.attr,
    &cpu_rush_threshold.attr,
    &cpu_rush_tlp_times.attr,
    &cpu_rush_avg_times.attr,
    NULL
};

static struct attribute_group dbs_attr_group = {
	.attrs = dbs_attributes,
	.name = "hotplug",
};

/************************** sysfs end ************************/

static void dbs_freq_increase(struct cpufreq_policy *p, unsigned int freq)
{
	if (dbs_tuners_ins.powersave_bias)
		freq = powersave_bias_target(p, freq, CPUFREQ_RELATION_H);
	else if (p->cur == p->max)
	{
		if (dbs_ignore == 0)
		{
			if((dbs_thermal_limited == 1) && (freq > dbs_thermal_limited_freq))
			{
				freq = dbs_thermal_limited_freq;
				printk("[dbs_freq_increase] thermal limit freq = %d\n", freq);
			}
			dbs_ignore = 1;
		}
		else
			return;
	}

	__cpufreq_driver_target(p, freq, dbs_tuners_ins.powersave_bias ?
			CPUFREQ_RELATION_L : CPUFREQ_RELATION_H);
}

int hp_get_dynamic_cpu_hotplug_enable(void)
{
	return !(dbs_tuners_ins.is_cpu_hotplug_disable);
}
EXPORT_SYMBOL(hp_get_dynamic_cpu_hotplug_enable);

void hp_set_dynamic_cpu_hotplug_enable(int enable)
{
	if (enable > 1 || enable < 0)
		return;
	
	mutex_lock(&hp_mutex);
	if (dbs_tuners_ins.is_cpu_hotplug_disable && enable)
		hp_reset_strategy_nolock();
	dbs_tuners_ins.is_cpu_hotplug_disable = !enable;
	mutex_unlock(&hp_mutex);
}
EXPORT_SYMBOL(hp_set_dynamic_cpu_hotplug_enable);

void hp_limited_cpu_num(int num)
{
	if (num > num_possible_cpus() || num < 1)
		return;
	
	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_num_limit = num;
	mutex_unlock(&hp_mutex);
}
EXPORT_SYMBOL(hp_limited_cpu_num);

void hp_based_cpu_num(int num)
{
	unsigned int online_cpus_count;

	if (num > num_possible_cpus() || num < 1)
		return;

	mutex_lock(&hp_mutex);

	dbs_tuners_ins.cpu_num_base = num;
	online_cpus_count = num_online_cpus();
#ifdef CONFIG_HOTPLUG_CPU
	if (online_cpus_count < num && online_cpus_count < dbs_tuners_ins.cpu_num_limit)
	{
		struct cpu_dbs_info_s *this_dbs_info;
		struct cpufreq_policy *policy;
		
		this_dbs_info = &per_cpu(hp_cpu_dbs_info, 0);
		policy = this_dbs_info->cur_policy;
		
		dbs_freq_increase(policy, policy->max);
		g_trigger_hp_work = CPU_HOTPLUG_WORK_TYPE_BASE;
		to_init(11, "hotplug kworker", 100, NULL);
		schedule_delayed_work_on(0, &hp_work, 0);
	}
#endif

	mutex_unlock(&hp_mutex);
}
EXPORT_SYMBOL(hp_based_cpu_num);

int hp_get_cpu_rush_boost_enable(void)
{
	return dbs_tuners_ins.cpu_rush_boost_enable;
}
EXPORT_SYMBOL(hp_get_cpu_rush_boost_enable);

void hp_set_cpu_rush_boost_enable(int enable)
{
	if (enable > 1 || enable < 0)
		return;
	
	mutex_lock(&hp_mutex);
	dbs_tuners_ins.cpu_rush_boost_enable = enable;
	mutex_unlock(&hp_mutex);
}
EXPORT_SYMBOL(hp_set_cpu_rush_boost_enable);

#ifdef CONFIG_HOTPLUG_CPU

#ifdef CONFIG_MTK_SCHED_RQAVG_KS
extern void sched_get_nr_running_avg(int *avg, int *iowait_avg);
#else //#ifdef CONFIG_MTK_SCHED_RQAVG_KS
static void sched_get_nr_running_avg(int *avg, int *iowait_avg)
{
	//*avg = num_possible_cpus() * 100;
	*avg = 1 * 100;
}
#endif //#ifdef CONFIG_MTK_SCHED_RQAVG_KS

static void hp_reset_strategy_nolock(void)
{
	g_cpu_up_count = 0;
	g_cpu_up_sum_load = 0;
	g_cpu_up_load_index = 0;
	g_cpu_up_load_history[dbs_tuners_ins.cpu_up_avg_times - 1] = 0;
	//memset(g_cpu_up_load_history, 0, sizeof(long) * MAX_CPU_UP_AVG_TIMES);
	
	g_cpu_down_count = 0;
	g_cpu_down_sum_load = 0;
	g_cpu_down_load_index = 0;
	g_cpu_down_load_history[dbs_tuners_ins.cpu_down_avg_times - 1] = 0;
	//memset(g_cpu_down_load_history, 0, sizeof(long) * MAX_CPU_DOWN_AVG_TIMES);
	
	g_tlp_avg_sum = 0;
	g_tlp_avg_count = 0;
	g_tlp_avg_index = 0;
	g_tlp_avg_history[dbs_tuners_ins.cpu_rush_tlp_times - 1] = 0;
	g_cpu_rush_count = 0;
    
	g_trigger_hp_work = CPU_HOTPLUG_WORK_TYPE_NONE;
}

static void hp_reset_strategy(void)
{
	mutex_lock(&hp_mutex);
	
	hp_reset_strategy_nolock();
	
	mutex_unlock(&hp_mutex);
}

static void hp_work_handler(struct work_struct *work)
{
	to_update_jiffies(11);
	if (mutex_trylock(&hp_onoff_mutex))
	{
		if (!dbs_tuners_ins.is_cpu_hotplug_disable)
		{
			unsigned int online_cpus_count = num_online_cpus();
			unsigned int i;

			printk("[power/hotplug] hp_work_handler(%d)(%d)(%d)(%d)(%ld)(%ld)(%d)(%d) begin\n", g_trigger_hp_work, g_tlp_avg_average, g_tlp_avg_current,
				g_cpus_sum_load_current, g_cpu_up_sum_load, g_cpu_down_sum_load,
				dbs_tuners_ins.cpu_num_base, dbs_tuners_ins.cpu_num_limit);

			switch (g_trigger_hp_work)
			{
				case CPU_HOTPLUG_WORK_TYPE_RUSH:
					for (i = online_cpus_count; i < min(g_next_hp_action, dbs_tuners_ins.cpu_num_limit); ++i)
					{
						to_init(12, "cpu_up", 3000, NULL);
						cpu_up(i);
						to_update_jiffies(12);
					}
					break;
				case CPU_HOTPLUG_WORK_TYPE_BASE:
					for (i = online_cpus_count; i < min(dbs_tuners_ins.cpu_num_base, dbs_tuners_ins.cpu_num_limit); ++i)
					{
						to_init(13, "cpu_up", 3000, NULL);
						cpu_up(i);
						to_update_jiffies(13);
					}
					break;
				case CPU_HOTPLUG_WORK_TYPE_LIMIT:
					for (i = online_cpus_count - 1; i >= dbs_tuners_ins.cpu_num_limit; --i)
						cpu_down(i);
					break;
				case CPU_HOTPLUG_WORK_TYPE_UP:
					for (i = online_cpus_count; i < g_next_hp_action; ++i)
					{
						to_init(14, "cpu_up", 3000, NULL);
						cpu_up(i);
						to_update_jiffies(14);
					}
					break;
				case CPU_HOTPLUG_WORK_TYPE_DOWN:
					for (i = online_cpus_count - 1; i >= g_next_hp_action; --i)
						cpu_down(i);
					break;
				default:
					for (i = online_cpus_count; i < min(dbs_tuners_ins.cpu_input_boost_num, dbs_tuners_ins.cpu_num_limit); ++i)
					{
						to_init(15, "cpu_up", 3000, NULL);
						cpu_up(i);
						to_update_jiffies(15);
					}
					//printk("[power/hotplug] cpu input boost\n");
					break;
			}

			hp_reset_strategy();
			dbs_ignore = 0; // force trigger frequency scaling

			printk("[power/hotplug] hp_work_handler end\n");

			/*
			if (g_next_hp_action) // turn on CPU
			{
				if (online_cpus_count < num_possible_cpus())
				{
					printk("hp_work_handler: cpu_up(%d) kick off\n", online_cpus_count);
					cpu_up(online_cpus_count);
					hp_reset_strategy();
					printk("hp_work_handler: cpu_up(%d) completion\n", online_cpus_count);
		
					dbs_ignore = 0; // force trigger frequency scaling
				}
			}
			else // turn off CPU
			{
				if (online_cpus_count > 1)
				{
					printk("hp_work_handler: cpu_down(%d) kick off\n", (online_cpus_count - 1));
					cpu_down((online_cpus_count - 1));
					hp_reset_strategy();
					printk("hp_work_handler: cpu_down(%d) completion\n", (online_cpus_count - 1));
		
					dbs_ignore = 0; // force trigger frequency scaling
				}
			}
			*/
		}
		mutex_unlock(&hp_onoff_mutex);
	}
}

#endif

static void dbs_check_cpu(struct cpu_dbs_info_s *this_dbs_info)
{
	unsigned int max_load_freq;

	struct cpufreq_policy *policy;
	unsigned int j;

#ifdef CONFIG_HOTPLUG_CPU
	long cpus_sum_load_last_up = 0;
	long cpus_sum_load_last_down = 0;
	unsigned int online_cpus_count;
	
	int v_tlp_avg_last = 0;
#endif

	#ifdef DBS_CHECK_CPU_LOG
	dbs_check_cpu_count++;
	if (time_after(jiffies, dbs_check_cpu_timeout))
	{
		printk(KERN_DEBUG "dbs_check_cpu: dbs_check_cpu_count = %d, jiffies = %d\n", dbs_check_cpu_count, jiffies);
		dbs_check_cpu_timeout = jiffies + msecs_to_jiffies(2500);
		dbs_check_cpu_count = 0;
	}
	#endif
	
	this_dbs_info->freq_lo = 0;
	policy = this_dbs_info->cur_policy;

	/*
	 * Every sampling_rate, we check, if current idle time is less
	 * than 20% (default), then we try to increase frequency
	 * Every sampling_rate, we look for a the lowest
	 * frequency which can sustain the load while keeping idle time over
	 * 30%. If such a frequency exist, we try to decrease to this frequency.
	 *
	 * Any frequency increase takes it to the maximum frequency.
	 * Frequency reduction happens at minimum steps of
	 * 5% (default) of current frequency
	 */

	/* Get Absolute Load - in terms of freq */
	max_load_freq = 0;
	g_cpus_sum_load_current = 0;

	for_each_cpu(j, policy->cpus) {
		struct cpu_dbs_info_s *j_dbs_info;
		cputime64_t cur_wall_time, cur_idle_time, cur_iowait_time;
		unsigned int idle_time, wall_time, iowait_time;
		unsigned int load, load_freq;
		int freq_avg;

		j_dbs_info = &per_cpu(hp_cpu_dbs_info, j);

		cur_idle_time = get_cpu_idle_time(j, &cur_wall_time);
		cur_iowait_time = get_cpu_iowait_time(j, &cur_wall_time);

		wall_time = (unsigned int)
			(cur_wall_time - j_dbs_info->prev_cpu_wall);
		j_dbs_info->prev_cpu_wall = cur_wall_time;

		idle_time = (unsigned int)
			(cur_idle_time - j_dbs_info->prev_cpu_idle);
		j_dbs_info->prev_cpu_idle = cur_idle_time;

		iowait_time = (unsigned int)
			(cur_iowait_time - j_dbs_info->prev_cpu_iowait);
		j_dbs_info->prev_cpu_iowait = cur_iowait_time;

		if (dbs_tuners_ins.ignore_nice) {
			u64 cur_nice;
			unsigned long cur_nice_jiffies;

			cur_nice = kcpustat_cpu(j).cpustat[CPUTIME_NICE] -
					 j_dbs_info->prev_cpu_nice;
			/*
			 * Assumption: nice time between sampling periods will
			 * be less than 2^32 jiffies for 32 bit sys
			 */
			cur_nice_jiffies = (unsigned long)
					cputime64_to_jiffies64(cur_nice);

			j_dbs_info->prev_cpu_nice = kcpustat_cpu(j).cpustat[CPUTIME_NICE];
			idle_time += jiffies_to_usecs(cur_nice_jiffies);
		}

		/*
		 * For the purpose of hotplug, waiting for disk IO is an
		 * indication that you're performance critical, and not that
		 * the system is actually idle. So subtract the iowait time
		 * from the cpu idle time.
		 */

		if (dbs_tuners_ins.io_is_busy && idle_time >= iowait_time)
			idle_time -= iowait_time;

		if (unlikely(!wall_time || wall_time < idle_time))
			continue;

		load = 100 * (wall_time - idle_time) / wall_time;

        #if defined(CONFIG_THERMAL_LIMIT_TEST)
        if(mt_cpufreq_thermal_test_limited_load() > 0)
            load = mt_cpufreq_thermal_test_limited_load();
        #endif
		
		g_cpus_sum_load_current += load;

		freq_avg = __cpufreq_driver_getavg(policy, j);
		if (freq_avg <= 0)
			freq_avg = policy->cur;

		load_freq = load * freq_avg;
		if (load_freq > max_load_freq)
			max_load_freq = load_freq;
			
		#ifdef DEBUG_LOG
		printk("dbs_check_cpu: cpu = %d\n", j);
		printk("dbs_check_cpu: wall_time = %d, idle_time = %d, load = %d\n", wall_time, idle_time, load);
		printk("dbs_check_cpu: freq_avg = %d, max_load_freq = %d, g_cpus_sum_load_current = %d\n", freq_avg, max_load_freq, g_cpus_sum_load_current);
		#endif
	}

	/* Check for frequency increase */
	if (max_load_freq > dbs_tuners_ins.up_threshold * policy->cur) {
		/* If switching to max speed, apply sampling_down_factor */
		if (policy->cur < policy->max)
			this_dbs_info->rate_mult =
				dbs_tuners_ins.sampling_down_factor;
		to_init(2, "dvfs up", 5, NULL);
		dbs_freq_increase(policy, policy->max);
		to_update_jiffies(2);
		goto hp_check;
	}

	/* Check for frequency decrease */
	/* if we cannot reduce the frequency anymore, break out early */
	if (policy->cur == policy->min)
		goto hp_check;

	/*
	 * The optimal frequency is the frequency that is the lowest that
	 * can support the current CPU usage without triggering the up
	 * policy. To be safe, we focus 10 points under the threshold.
	 */
	if (max_load_freq <
	    (dbs_tuners_ins.up_threshold - dbs_tuners_ins.down_differential) *
	     policy->cur) {
		unsigned int freq_next;
		freq_next = max_load_freq /
				(dbs_tuners_ins.up_threshold -
				 dbs_tuners_ins.down_differential);

		/* No longer fully busy, reset rate_mult */
		this_dbs_info->rate_mult = 1;

		if (freq_next < policy->min)
			freq_next = policy->min;

		if (!dbs_tuners_ins.powersave_bias) {
			//20140115 marc.huang, always run this path
			to_init(3, "dvfs down", 5, NULL);
			__cpufreq_driver_target(policy, freq_next,
					CPUFREQ_RELATION_L);
			to_update_jiffies(3);
		} else {
			int freq = powersave_bias_target(policy, freq_next,
					CPUFREQ_RELATION_L);
			to_init(4, "dvfs down", 5, NULL);
			__cpufreq_driver_target(policy, freq,
				CPUFREQ_RELATION_L);
			to_update_jiffies(4);
		}
	}

hp_check:

	/* If Hot Plug policy disable, return directly */
	if (dbs_tuners_ins.is_cpu_hotplug_disable)
		return;

#ifdef CONFIG_HOTPLUG_CPU
	if (g_trigger_hp_work != CPU_HOTPLUG_WORK_TYPE_NONE)
	{
		printk("[power/hotplug] no hp_check due to g_trigger_hp_work: %d\n", g_trigger_hp_work);
		return;
	}

	mutex_lock(&hp_mutex);
	
	online_cpus_count = num_online_cpus();
	
	to_init(10, "get tlp", 5, NULL);
	sched_get_nr_running_avg(&g_tlp_avg_current, &g_tlp_iowait_av);
	to_update_jiffies(10);
	
	v_tlp_avg_last = g_tlp_avg_history[g_tlp_avg_index];
	g_tlp_avg_history[g_tlp_avg_index] = g_tlp_avg_current;
	g_tlp_avg_sum += g_tlp_avg_current;
	
	g_tlp_avg_index = (g_tlp_avg_index + 1 == dbs_tuners_ins.cpu_rush_tlp_times)? 0 : g_tlp_avg_index + 1;
	g_tlp_avg_count++;
	if (g_tlp_avg_count >= dbs_tuners_ins.cpu_rush_tlp_times)
	{
		if (g_tlp_avg_sum > v_tlp_avg_last)
			g_tlp_avg_sum -= v_tlp_avg_last;
		else
			g_tlp_avg_sum = 0;
	}
	g_tlp_avg_average = g_tlp_avg_sum / dbs_tuners_ins.cpu_rush_tlp_times;
	
	to_update_jiffies(5);
	if (dbs_tuners_ins.cpu_rush_boost_enable)
	{
		//printk("@@@@@@@@@@@@@@@@@@@@@@@@@@@ tlp: %d @@@@@@@@@@@@@@@@@@@@@@@@@@@\n", g_tlp_avg_average);

		if (g_cpus_sum_load_current > dbs_tuners_ins.cpu_rush_threshold * online_cpus_count)
			++g_cpu_rush_count;
		else
			g_cpu_rush_count = 0;

		if ((g_cpu_rush_count >= dbs_tuners_ins.cpu_rush_avg_times) &&
			(online_cpus_count * 100 < g_tlp_avg_average) &&
			(online_cpus_count < dbs_tuners_ins.cpu_num_limit) &&
			(online_cpus_count < num_possible_cpus()))
		{
			dbs_freq_increase(policy, policy->max);
			printk("dbs_check_cpu: turn on CPU\n");
			g_next_hp_action = g_tlp_avg_average / 100 + (g_tlp_avg_average % 100 ? 1 : 0);
			if (g_next_hp_action > num_possible_cpus())
				g_next_hp_action = num_possible_cpus();
			g_trigger_hp_work = CPU_HOTPLUG_WORK_TYPE_RUSH;
			to_init(11, "hotplug kworker", 100, NULL);
			schedule_delayed_work_on(0, &hp_work, 0);

			goto hp_check_end;
		}
	}
	
	if (online_cpus_count < dbs_tuners_ins.cpu_num_base && online_cpus_count < dbs_tuners_ins.cpu_num_limit)
	{
		dbs_freq_increase(policy, policy->max);
		printk("dbs_check_cpu: turn on CPU\n");
		g_trigger_hp_work = CPU_HOTPLUG_WORK_TYPE_BASE;
		to_init(11, "hotplug kworker", 100, NULL);
		schedule_delayed_work_on(0, &hp_work, 0);

		goto hp_check_end;
	}
	
	if (online_cpus_count > dbs_tuners_ins.cpu_num_limit)
	{
		dbs_freq_increase(policy, policy->max);
		printk("dbs_check_cpu: turn off CPU\n");
		g_trigger_hp_work = CPU_HOTPLUG_WORK_TYPE_LIMIT;
		to_init(11, "hotplug kworker", 100, NULL);
		schedule_delayed_work_on(0, &hp_work, 0);

		goto hp_check_end;
	}
	
	/* Check CPU loading to power up slave CPU */
	if (online_cpus_count < num_possible_cpus()) 
	{
		cpus_sum_load_last_up = g_cpu_up_load_history[g_cpu_up_load_index];
		g_cpu_up_load_history[g_cpu_up_load_index] = g_cpus_sum_load_current;
		g_cpu_up_sum_load += g_cpus_sum_load_current;

		g_cpu_up_count++;
		g_cpu_up_load_index = (g_cpu_up_load_index + 1 == dbs_tuners_ins.cpu_up_avg_times)? 0 : g_cpu_up_load_index + 1;
		
		if (g_cpu_up_count >= dbs_tuners_ins.cpu_up_avg_times)
		{
			if (g_cpu_up_sum_load > cpus_sum_load_last_up)
				g_cpu_up_sum_load -= cpus_sum_load_last_up;
			else
				g_cpu_up_sum_load = 0;

			//g_cpu_up_sum_load /= dbs_tuners_ins.cpu_up_avg_times;			
			if (g_cpu_up_sum_load > 
				(dbs_tuners_ins.cpu_up_threshold * online_cpus_count  * dbs_tuners_ins.cpu_up_avg_times))
			{
				if (online_cpus_count < dbs_tuners_ins.cpu_num_limit)
				{
					#ifdef DEBUG_LOG
					printk("dbs_check_cpu: g_cpu_up_sum_load = %d\n", g_cpu_up_sum_load);
					#endif
					dbs_freq_increase(policy, policy->max);
					printk("dbs_check_cpu: turn on CPU\n");
					g_next_hp_action = online_cpus_count + 1;
					g_trigger_hp_work = CPU_HOTPLUG_WORK_TYPE_UP;
					to_init(11, "hotplug kworker", 100, NULL);
					schedule_delayed_work_on(0, &hp_work, 0);
					
					goto hp_check_end;
				}
			}
		}
		
		#ifdef DEBUG_LOG
		printk("dbs_check_cpu: g_cpu_up_count = %d, g_cpu_up_sum_load = %d\n", g_cpu_up_count, g_cpu_up_sum_load);
		printk("dbs_check_cpu: cpu_up_threshold = %d\n", (dbs_tuners_ins.cpu_up_threshold * online_cpus_count));
		#endif
		
	}

	/* Check CPU loading to power down slave CPU */
	if (online_cpus_count > 1)
	{
		cpus_sum_load_last_down = g_cpu_down_load_history[g_cpu_down_load_index];
		g_cpu_down_load_history[g_cpu_down_load_index] = g_cpus_sum_load_current;
		g_cpu_down_sum_load += g_cpus_sum_load_current;
		
		g_cpu_down_count++;
		g_cpu_down_load_index = (g_cpu_down_load_index + 1 == dbs_tuners_ins.cpu_down_avg_times)? 0 : g_cpu_down_load_index + 1;

		if (g_cpu_down_count >= dbs_tuners_ins.cpu_down_avg_times)
		{
			long cpu_down_threshold;
			
			if (g_cpu_down_sum_load > cpus_sum_load_last_down)
				g_cpu_down_sum_load -= cpus_sum_load_last_down;
			else
				g_cpu_down_sum_load = 0;

			g_next_hp_action = online_cpus_count;
			cpu_down_threshold = ((dbs_tuners_ins.cpu_up_threshold - dbs_tuners_ins.cpu_down_differential) * dbs_tuners_ins.cpu_down_avg_times);
			while ((g_cpu_down_sum_load < cpu_down_threshold * (g_next_hp_action - 1)) &&
				//(g_next_hp_action > tlp_cpu_num) &&
				(g_next_hp_action > dbs_tuners_ins.cpu_num_base))
				--g_next_hp_action;
			
			//printk("### g_next_hp_action: %d, tlp_cpu_num: %d, g_cpu_down_sum_load / dbs_tuners_ins.cpu_down_avg_times: %d ###\n", g_next_hp_action, tlp_cpu_num, g_cpu_down_sum_load / dbs_tuners_ins.cpu_down_avg_times);
			if (g_next_hp_action < online_cpus_count)
			{
				#ifdef DEBUG_LOG
				printk("dbs_check_cpu: g_cpu_down_sum_load = %d\n", g_cpu_down_sum_load);
				#endif
				dbs_freq_increase(policy, policy->max);
				printk("dbs_check_cpu: turn off CPU\n");
				g_trigger_hp_work = CPU_HOTPLUG_WORK_TYPE_DOWN;
				to_init(11, "hotplug kworker", 100, NULL);
				schedule_delayed_work_on(0, &hp_work, 0);
			}
		}
		#ifdef DEBUG_LOG
		printk("dbs_check_cpu: g_cpu_down_count = %d, g_cpu_down_sum_load = %d\n", g_cpu_down_count, g_cpu_down_sum_load);
		printk("dbs_check_cpu: cpu_down_threshold = %d\n", ((dbs_tuners_ins.cpu_up_threshold - dbs_tuners_ins.cpu_down_differential) * (online_cpus_count - 1)));
		#endif
	}

hp_check_end:
	mutex_unlock(&hp_mutex);

#endif //#ifdef CONFIG_HOTPLUG_CPU

	return;
}

void (*cpufreq_freq_check)(void) = NULL;

static void do_dbs_timer(struct work_struct *work)
{
	struct cpu_dbs_info_s *dbs_info =
		container_of(work, struct cpu_dbs_info_s, work.work);
	unsigned int cpu = dbs_info->cpu;
	int sample_type = dbs_info->sample_type;

	int delay;

	to_update_jiffies(0);
	mutex_lock(&dbs_info->timer_mutex);

	/* Common NORMAL_SAMPLE setup */
	dbs_info->sample_type = DBS_NORMAL_SAMPLE;
	if (!dbs_tuners_ins.powersave_bias ||
	    sample_type == DBS_NORMAL_SAMPLE) {
		to_init(1, "dbs_check_cpu", 10, NULL);
		dbs_check_cpu(dbs_info);
		to_update_jiffies(1);
		if (dbs_info->freq_lo) {
			/* Setup timer for SUB_SAMPLE */
			dbs_info->sample_type = DBS_SUB_SAMPLE;
			delay = dbs_info->freq_hi_jiffies;
		} else {
			/* We want all CPUs to do sampling nearly on
			 * same jiffy
			 */
			//20140115 marc.huang, always run this path
			delay = usecs_to_jiffies(dbs_tuners_ins.sampling_rate
				* dbs_info->rate_mult);

			if (num_online_cpus() > 1)
				delay -= jiffies % delay;
		}
	} else {
		__cpufreq_driver_target(dbs_info->cur_policy,
			dbs_info->freq_lo, CPUFREQ_RELATION_H);
		delay = dbs_info->freq_lo_jiffies;
	}
	
	to_init(0, "dvfs kworker", jiffies_to_msecs(delay) * 10, NULL);
	schedule_delayed_work_on(cpu, &dbs_info->work, delay);
	mutex_unlock(&dbs_info->timer_mutex);

	if(cpufreq_freq_check != NULL)
	{
		to_init(6, "cpufreq_freq_check", 10, NULL);
		cpufreq_freq_check();
		to_update_jiffies(6);
	}
}

static inline void dbs_timer_init(struct cpu_dbs_info_s *dbs_info)
{
	/* We want all CPUs to do sampling nearly on same jiffy */
	int delay = usecs_to_jiffies(dbs_tuners_ins.sampling_rate);

	if (num_online_cpus() > 1)
		delay -= jiffies % delay;

	dbs_info->sample_type = DBS_NORMAL_SAMPLE;
	INIT_DELAYED_WORK_DEFERRABLE(&dbs_info->work, do_dbs_timer);
	to_init(0, "dvfs kworker", jiffies_to_msecs(delay) * 10, NULL);
#ifdef CONFIG_HOTPLUG_CPU
	to_init(5, "dvfs kworker(hotplug)", 1000, hp_to_callback);
#endif //#ifdef CONFIG_HOTPLUG_CPU
	schedule_delayed_work_on(dbs_info->cpu, &dbs_info->work, delay);
}

static inline void dbs_timer_exit(struct cpu_dbs_info_s *dbs_info)
{
	cancel_delayed_work_sync(&dbs_info->work);
}

/*
 * Not all CPUs want IO time to be accounted as busy; this dependson how
 * efficient idling at a higher frequency/voltage is.
 * Pavel Machek says this is not so for various generations of AMD and old
 * Intel systems.
 * Mike Chan (androidlcom) calis this is also not true for ARM.
 * Because of this, whitelist specific known (series) of CPUs by default, and
 * leave all others up to the user.
 */
static int should_io_be_busy(void)
{
#if defined(CONFIG_X86)
	/*
	 * For Intel, Core 2 (model 15) andl later have an efficient idle.
	 */
	if (boot_cpu_data.x86_vendor == X86_VENDOR_INTEL &&
	    boot_cpu_data.x86 == 6 &&
	    boot_cpu_data.x86_model >= 15)
		return 1;
#endif
	return 1; // io wait time should be subtracted from idle time
}

#ifdef CONFIG_HOTPLUG_CPU
static void dbs_input_event(struct input_handle *handle, unsigned int type,
		unsigned int code, int value)
{
	//int i;

	//if ((dbs_tuners_ins.powersave_bias == POWERSAVE_BIAS_MAXLEVEL) ||
	//	(dbs_tuners_ins.powersave_bias == POWERSAVE_BIAS_MINLEVEL)) {
	//	/* nothing to do */
	//	return;
	//}

	//for_each_online_cpu(i) {
	//	queue_work_on(i, input_wq, &per_cpu(dbs_refresh_work, i));
	//}
	//printk("$$$ in_interrupt(): %d, in_irq(): %d, type: %d, code: %d, value: %d $$$\n", in_interrupt(), in_irq(), type, code, value);
	
	if ((type == EV_KEY) && (code == BTN_TOUCH) && (value == 1) && (dbs_tuners_ins.cpu_input_boost_enable))
	{
		//if (!in_interrupt())
		//{
			unsigned int online_cpus_count = num_online_cpus();
			if (online_cpus_count < dbs_tuners_ins.cpu_input_boost_num && online_cpus_count < dbs_tuners_ins.cpu_num_limit)
			{
				to_init(11, "hotplug kworker", 100, NULL);
				schedule_delayed_work_on(0, &hp_work, 0);
			}
			
		//}
	}
}

static int dbs_input_connect(struct input_handler *handler,
		struct input_dev *dev, const struct input_device_id *id)
{
	struct input_handle *handle;
	int error;

	handle = kzalloc(sizeof(struct input_handle), GFP_KERNEL);
	if (!handle)
		return -ENOMEM;

	handle->dev = dev;
	handle->handler = handler;
	handle->name = "cpufreq";

	error = input_register_handle(handle);
	if (error)
		goto err2;

	error = input_open_device(handle);
	if (error)
		goto err1;

	return 0;
err1:
	input_unregister_handle(handle);
err2:
	kfree(handle);
	return error;
}

static void dbs_input_disconnect(struct input_handle *handle)
{
	input_close_device(handle);
	input_unregister_handle(handle);
	kfree(handle);
}

static const struct input_device_id dbs_ids[] = {
	{ .driver_info = 1 },
	{ },
};

static struct input_handler dbs_input_handler = {
	.event		= dbs_input_event,
	.connect	= dbs_input_connect,
	.disconnect	= dbs_input_disconnect,
	.name		= "cpufreq_ond",
	.id_table	= dbs_ids,
};
#endif //#ifdef CONFIG_HOTPLUG_CPU

static int cpufreq_governor_dbs(struct cpufreq_policy *policy,
				   unsigned int event)
{
	unsigned int cpu = policy->cpu;
	struct cpu_dbs_info_s *this_dbs_info;
	unsigned int j;
	int rc;

	this_dbs_info = &per_cpu(hp_cpu_dbs_info, cpu);

	switch (event) {
	case CPUFREQ_GOV_START:
		if ((!cpu_online(cpu)) || (!policy->cur))
			return -EINVAL;

		mutex_lock(&dbs_mutex);

		dbs_enable++;
		for_each_cpu(j, policy->cpus) {
			struct cpu_dbs_info_s *j_dbs_info;
			j_dbs_info = &per_cpu(hp_cpu_dbs_info, j);
			j_dbs_info->cur_policy = policy;

			j_dbs_info->prev_cpu_idle = get_cpu_idle_time(j,
						&j_dbs_info->prev_cpu_wall);
			if (dbs_tuners_ins.ignore_nice)
				j_dbs_info->prev_cpu_nice =
						kcpustat_cpu(j).cpustat[CPUTIME_NICE];
		}
		this_dbs_info->cpu = cpu;
		this_dbs_info->rate_mult = 1;
		hotplug_powersave_bias_init_cpu(cpu);
		/*
		 * Start the timerschedule work, when this governor
		 * is used for first time
		 */
		if (dbs_enable == 1) {
			unsigned int latency;

			rc = sysfs_create_group(cpufreq_global_kobject,
						&dbs_attr_group);
			if (rc) {
				mutex_unlock(&dbs_mutex);
				return rc;
			}

			/* policy latency is in nS. Convert it to uS first */
			latency = policy->cpuinfo.transition_latency / 1000;
			if (latency == 0)
				latency = 1;
			/* Bring kernel and HW constraints together */
			min_sampling_rate = max(min_sampling_rate,
					MIN_LATENCY_MULTIPLIER * latency);
			dbs_tuners_ins.sampling_rate =
				max(min_sampling_rate,
				    latency * LATENCY_MULTIPLIER);
			dbs_tuners_ins.io_is_busy = should_io_be_busy();
			
			#ifdef DEBUG_LOG
			printk("cpufreq_governor_dbs: min_sampling_rate = %d\n", min_sampling_rate);
			printk("cpufreq_governor_dbs: dbs_tuners_ins.sampling_rate = %d\n", dbs_tuners_ins.sampling_rate);
			printk("cpufreq_governor_dbs: dbs_tuners_ins.io_is_busy = %d\n", dbs_tuners_ins.io_is_busy);
			#endif
		}
	#ifdef CONFIG_HOTPLUG_CPU
		if (!cpu)
			rc = input_register_handler(&dbs_input_handler);
	#endif
		mutex_unlock(&dbs_mutex);

		mutex_init(&this_dbs_info->timer_mutex);
		dbs_timer_init(this_dbs_info);
		break;

	case CPUFREQ_GOV_STOP:
		dbs_timer_exit(this_dbs_info);

		mutex_lock(&dbs_mutex);
		mutex_destroy(&this_dbs_info->timer_mutex);
		dbs_enable--;
	#ifdef CONFIG_HOTPLUG_CPU
		/* If device is being removed, policy is no longer
		 * valid. */
		this_dbs_info->cur_policy = NULL;
		if (!cpu)
			input_unregister_handler(&dbs_input_handler);
	#endif
		mutex_unlock(&dbs_mutex);
		if (!dbs_enable)
			sysfs_remove_group(cpufreq_global_kobject,
					   &dbs_attr_group);

		break;

	case CPUFREQ_GOV_LIMITS:
		mutex_lock(&this_dbs_info->timer_mutex);
		if (policy->max < this_dbs_info->cur_policy->cur)
			__cpufreq_driver_target(this_dbs_info->cur_policy,
				policy->max, CPUFREQ_RELATION_H);
		else if (policy->min > this_dbs_info->cur_policy->cur)
			__cpufreq_driver_target(this_dbs_info->cur_policy,
				policy->min, CPUFREQ_RELATION_L);
		mutex_unlock(&this_dbs_info->timer_mutex);
		break;
	}
	return 0;
}

//int cpufreq_gov_dbs_get_sum_load(void)
//{
//	return g_cpus_sum_load_current;
//}
void cpufreq_min_sampling_rate_change(unsigned int sample_rate)
{
    min_sampling_rate = sample_rate; 
    dbs_tuners_ins.sampling_rate = sample_rate;
}
EXPORT_SYMBOL(cpufreq_min_sampling_rate_change);

static int __init cpufreq_gov_dbs_init(void)
{
	u64 idle_time;
	int cpu = get_cpu();

	idle_time = get_cpu_idle_time_us(cpu, NULL);
	put_cpu();
	if (idle_time != -1ULL) {
		/* Idle micro accounting is supported. Use finer thresholds */
		dbs_tuners_ins.up_threshold = MICRO_FREQUENCY_UP_THRESHOLD;
		dbs_tuners_ins.down_differential =
					MICRO_FREQUENCY_DOWN_DIFFERENTIAL;
		dbs_tuners_ins.cpu_up_threshold =
					MICRO_CPU_UP_THRESHOLD;
		dbs_tuners_ins.cpu_down_differential =
					MICRO_CPU_DOWN_DIFFERENTIAL;
		/*
		 * In nohz/micro accounting case we set the minimum frequency
		 * not depending on HZ, but fixed (very low). The deferred
		 * timer might skip some samples if idle/sleeping as needed.
		*/
		min_sampling_rate = MICRO_FREQUENCY_MIN_SAMPLE_RATE;
		
		/* cpu rush boost */
		dbs_tuners_ins.cpu_rush_threshold =
					MICRO_CPU_RUSH_THRESHOLD;
		dbs_tuners_ins.cpu_rush_boost_num =
					num_possible_cpus();
	} else {
		/* For correct statistics, we need 10 ticks for each measure */
		min_sampling_rate =
			MIN_SAMPLING_RATE_RATIO * jiffies_to_usecs(10);
	}

	dbs_tuners_ins.cpu_num_limit = num_possible_cpus();
	dbs_tuners_ins.cpu_num_base = 1;

	if (dbs_tuners_ins.cpu_num_limit > 1)
		dbs_tuners_ins.is_cpu_hotplug_disable = 0;

	#ifdef CONFIG_HOTPLUG_CPU
	INIT_DELAYED_WORK_DEFERRABLE(&hp_work, hp_work_handler);
	g_next_hp_action = num_online_cpus();
	#endif

	#ifdef DEBUG_LOG
	printk("cpufreq_gov_dbs_init: min_sampling_rate = %d\n", min_sampling_rate);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.up_threshold = %d\n", dbs_tuners_ins.up_threshold);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.down_differential = %d\n", dbs_tuners_ins.down_differential);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_up_threshold = %d\n", dbs_tuners_ins.cpu_up_threshold);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_down_differential = %d\n", dbs_tuners_ins.cpu_down_differential);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_up_avg_times = %d\n", dbs_tuners_ins.cpu_up_avg_times);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_down_avg_times = %d\n", dbs_tuners_ins.cpu_down_avg_times);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_num_limit = %d\n", dbs_tuners_ins.cpu_num_limit);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_num_base = %d\n", dbs_tuners_ins.cpu_num_base);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.is_cpu_hotplug_disable = %d\n", dbs_tuners_ins.is_cpu_hotplug_disable);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_input_boost_enable = %d\n", dbs_tuners_ins.cpu_input_boost_enable);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_input_boost_num = %d\n", dbs_tuners_ins.cpu_input_boost_num);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_rush_boost_enable = %d\n", dbs_tuners_ins.cpu_rush_boost_enable);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_rush_boost_num = %d\n", dbs_tuners_ins.cpu_rush_boost_num);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_rush_threshold = %d\n", dbs_tuners_ins.cpu_rush_threshold);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_rush_tlp_times = %d\n", dbs_tuners_ins.cpu_rush_tlp_times);
	printk("cpufreq_gov_dbs_init: dbs_tuners_ins.cpu_rush_avg_times = %d\n", dbs_tuners_ins.cpu_rush_avg_times);
	#endif 

	#ifdef DBS_CHECK_CPU_LOG
	dbs_check_cpu_timeout = jiffies + msecs_to_jiffies(2500);
	#endif
	
	return cpufreq_register_governor(&cpufreq_gov_hotplug);
}

static void __exit cpufreq_gov_dbs_exit(void)
{
	#ifdef CONFIG_HOTPLUG_CPU
	cancel_delayed_work_sync(&hp_work);
	#endif

	cpufreq_unregister_governor(&cpufreq_gov_hotplug);
}


MODULE_AUTHOR("Venkatesh Pallipadi <venkatesh.pallipadi@intel.com>");
MODULE_AUTHOR("Alexey Starikovskiy <alexey.y.starikovskiy@intel.com>");
MODULE_DESCRIPTION("'cpufreq_hotplug' - A dynamic cpufreq governor for "
	"Low Latency Frequency Transition capable processors");
MODULE_LICENSE("GPL");

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_HOTPLUG
fs_initcall(cpufreq_gov_dbs_init);
#else
module_init(cpufreq_gov_dbs_init);
#endif
module_exit(cpufreq_gov_dbs_exit);

/*******************************************************************************
 * timer observer callback
 ******************************************************************************/
#ifdef CONFIG_HOTPLUG_CPU
static void hp_to_callback(void)
{
	printk("[power/hotplug] hp_to_callback(%d)(%d)(%d)(%d)(%d)(%ld)(%d)(%ld)(%d)(%d)(%d)\n", 
	    g_trigger_hp_work, g_tlp_avg_average, g_tlp_avg_current, g_cpu_rush_count, 
		g_cpus_sum_load_current, g_cpu_up_sum_load, g_cpu_up_count, g_cpu_down_sum_load, g_cpu_down_count,
		dbs_tuners_ins.cpu_num_base, dbs_tuners_ins.cpu_num_limit);
}
#endif //#ifdef CONFIG_HOTPLUG_CPU