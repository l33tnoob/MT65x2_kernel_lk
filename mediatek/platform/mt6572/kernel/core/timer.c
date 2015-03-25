#include <asm/mach/time.h>
#include <mach/mt_timer.h>

extern struct mt_clock mt6572_gpt;
extern int generic_timer_register(void);


struct mt_clock *mt6572_clocks[] =
{
    &mt6572_gpt,
};

static void __init mt6572_timer_init(void)
{
    int i;
    struct mt_clock *clock;
    int err;

    for (i = 0; i < ARRAY_SIZE(mt6572_clocks); i++) {
        clock = mt6572_clocks[i];

        clock->init_func();

        if (clock->clocksource.name) {
            err = clocksource_register(&(clock->clocksource));
            if (err) {
                pr_err("mt6572_timer_init: clocksource_register failed for %s\n", clock->clocksource.name);
            }
        }

        err = setup_irq(clock->irq.irq, &(clock->irq));
        if (err) {
            pr_err("mt6572_timer_init: setup_irq failed for %s\n", clock->irq.name);
        }

        if (clock->clockevent.name)
            clockevents_register_device(&(clock->clockevent));
    }

//#ifndef CONFIG_MT6572_FPGA
    err = generic_timer_register(); 
    if (err) {
        pr_err("generic_timer_register failed, err=%d\n", err);
    }
  // printk("fwq no generic timer");
//#endif
}


struct sys_timer mt6572_timer = {
    .init = mt6572_timer_init,
};
