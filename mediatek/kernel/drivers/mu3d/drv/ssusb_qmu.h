#ifndef __SSUSB_QMU_H__
#define __SSUSB_QMU_H__

#include <linux/mu3d/hal/mu3d_hal_qmu_drv.h>


#ifdef USE_SSUSB_QMU

//#define GPD_BUF_SIZE 65532 //!!!
//#define BD_BUF_SIZE 32768 //set to half of 64K of max size

#undef EXTERN
#define EXTERN

//typedef struct _QMU_DONE_ISR_DATA {
//	struct musb *musb;
//	DEV_UINT32 wQmuVal;
//}QMU_DONE_ISR_DATA;

//EXTERN void usb_initialize_qmu(void);
//EXTERN void txstate_qmu(struct musb *musb, struct musb_request *req);
void qmu_exception_interrupt(struct musb *musb, DEV_UINT32 wQmuVal);
//void qmu_done_interrupt(unsigned long pparam);
void qmu_done_interrupt(struct musb *musb, DEV_UINT32 wQmuVal);


#endif //#ifdef USE_SSUSB_QMU

#endif //#ifndef __SSUSB_QMU_H__
