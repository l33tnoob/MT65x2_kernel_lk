/*
 * REE Service call
 *
 * Internal used, TA should not use this.
 */

#ifndef __MTEE_REE_H__
#define __MTEE_REE_H__

/**
 * Calling REE service call
 * The parameter should be filled to REE service parameter buffer returned
 * by MTEE_GetReeParamAddress
 *
 * @param command Ree service command
 * @return Return value from Ree service command.
 */
TZ_RESULT MTEE_ReeServiceCall(uint32_t command);

/**
 * Get REE Service parameter buffer
 *
 * @return pointer to REE service parameter buffer.
 *         Size of the buffer is REE_SERVICE_BUFFER_SIZE.
 */
void *MTEE_GetReeParamAddress(void);

#endif /* __MTEE_REE_H__ */
