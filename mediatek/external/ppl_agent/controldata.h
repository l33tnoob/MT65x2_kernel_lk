#ifndef __CONTROL_DATA__
#define __CONTROL_DATA__

#define CD_SECRET_LENGTH (20)
#define CD_SALT_LENGTH (20)
#define CD_SIM_FINGERPRINT_LENGTH (40)
#define CD_NUMBER_LENGTH (40)

struct PendingSMS {
	unsigned char	type;
	unsigned char	dest[CD_NUMBER_LENGTH];
};

/*
 * Layout of Control Data is:
 *
 * +--------------------+
 * |Header              |
 * +--------------------+
 * |Body                |
 * +--------------------+
 *
 * Layout of Control Data is:
 *
 * +--------------------+
 * |SIM Fingerprint List|
 * +--------------------+
 * |Trusted Number List |
 * +--------------------+
 * |Pending SMS List    |
 * +--------------------+
 *
 * Status bits:
 * 0x01 - Enable
 * 0x02 - Lock
 * 0x04 - Wipe Request
 *
 */
 
/*
 * Header Layout
 */
struct ControlDataHeader {
	unsigned char	version;
	unsigned char	status;
	unsigned char	sim_fingerprint_n;
	unsigned char	trusted_number_n;
	unsigned char	secret[CD_SECRET_LENGTH];
	unsigned char	salt[CD_SALT_LENGTH];
	unsigned char	pending_sms_n;
};

extern int readControlDataHeader(struct ControlDataHeader * const outbuf);
extern int readControlDataBody(unsigned char * const outbuf, const size_t outbuf_size);
extern int writeControlData(struct ConstrolDataHeader * const dst, const struct ConstrolDataHeader * const inbuf);

#endif /* __CONTROL_DATA__ */

