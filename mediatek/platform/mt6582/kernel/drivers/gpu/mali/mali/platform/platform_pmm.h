#ifndef __PLATFORM_PMM_H__
#define __PLATFORM_PMM_H__

typedef enum mali_power_mode
{
    MALI_POWER_MODE_ON  = 0x1,
    MALI_POWER_MODE_DEEP_SLEEP,
    MALI_POWER_MODE_LIGHT_SLEEP,
    //MALI_POWER_MODE_NUM
} mali_power_mode;

/** @brief Platform power management initialisation of MALI
 *
 * This is called from the entrypoint of the driver to initialize the platform
 *
 */
void mali_pmm_init(void);

/** @brief Platform power management deinitialisation of MALI
 *
 * This is called on the exit of the driver to terminate the platform
 *
 * @return _MALI_OSK_ERR_OK on success otherwise, a suitable _mali_osk_errcode_t error.
 */
void mali_pmm_deinit(void);

/** @brief Platform power management mode change of MALI
 *
 * This is called on the power state transistion of MALI.
 * @param mode defines the power modes
 */
void mali_pmm_tri_mode(mali_power_mode mode);

/** @brief Platform power management specific GPU utilization handler
 *
 * When GPU utilization handler is enabled, this function will be
 * periodically called.
 *
 * @param utilization The Mali GPU's work loading from 0 ~ 256. 0 = no utilization, 256 = full utilization.
 */
void mali_pmm_utilization_handler(unsigned int utilization);
unsigned long gpu_get_current_utilization(void);

void mali_platform_power_mode_change(mali_power_mode power_mode);

#endif

