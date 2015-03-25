#include <mach/mt_typedefs.h>
#include <linux/types.h>
#include "disp_drv_platform.h"
#include "disp_hal.h"
#include "disp_intr.h"
#include "lcd_drv.h"
#include "dpi_drv.h"
#include "dpi1_drv.h"
#include "dsi_drv.h"

static DISP_INTERRUPT_CALLBACK_STRUCT DISP_CallbackArray[DISP_LCD_INTERRUPT_EVENTS_NUMBER + DISP_DSI_INTERRUPT_EVENTS_NUMBER + DISP_DPI_INTERRUPT_EVENTS_NUMBER];

static void _DISP_InterruptCallbackProxy(DISP_INTERRUPT_EVENTS eventID)
{
    UINT32 offset;

    if(eventID >= DISP_LCD_INTERRUPT_EVENTS_START && eventID <= DISP_LCD_INTERRUPT_EVENTS_END )
    {
        offset = eventID - DISP_LCD_INTERRUPT_EVENTS_START;
        if(DISP_CallbackArray[offset].pFunc)
        {
            DISP_CallbackArray[offset].pFunc(DISP_CallbackArray[offset].pParam);
        }
    }
    else if(eventID >= DISP_DSI_INTERRUPT_EVENTS_START && eventID <= DISP_DSI_INTERRUPT_EVENTS_END )
    {
        offset = eventID - DISP_DSI_INTERRUPT_EVENTS_START + DISP_LCD_INTERRUPT_EVENTS_NUMBER;
        if(DISP_CallbackArray[offset].pFunc)
        {
            DISP_CallbackArray[offset].pFunc(DISP_CallbackArray[offset].pParam);
        }
    }
    else if(eventID >= DISP_DPI_INTERRUPT_EVENTS_START && eventID <= DISP_DPI_INTERRUPT_EVENTS_END )
    {
        offset = eventID - DISP_DPI_INTERRUPT_EVENTS_START + DISP_LCD_INTERRUPT_EVENTS_NUMBER + DISP_DSI_INTERRUPT_EVENTS_NUMBER;
        if(DISP_CallbackArray[offset].pFunc)
        {
            DISP_CallbackArray[offset].pFunc(DISP_CallbackArray[offset].pParam);
        }
    }
    else
    {
        printk("Invalid event id: %d\n", eventID);
        ASSERT(0);
    }

    return;
}

DISP_STATUS DISP_SetInterruptCallback(DISP_INTERRUPT_EVENTS eventID, DISP_INTERRUPT_CALLBACK_STRUCT *pCBStruct)
{
    UINT32 offset;
    ASSERT(pCBStruct != NULL);

    if(eventID >= DISP_LCD_INTERRUPT_EVENTS_START && eventID <= DISP_LCD_INTERRUPT_EVENTS_END )
    {
        ///register callback
        offset = eventID - DISP_LCD_INTERRUPT_EVENTS_START;
        DISP_CallbackArray[offset].pFunc = pCBStruct->pFunc;
        DISP_CallbackArray[offset].pParam = pCBStruct->pParam;

        LCD_CHECK_RET(LCD_SetInterruptCallback(_DISP_InterruptCallbackProxy));
        LCD_CHECK_RET(LCD_EnableInterrupt(eventID));
    }
    else if(eventID >= DISP_DSI_INTERRUPT_EVENTS_START && eventID <= DISP_DSI_INTERRUPT_EVENTS_END )
    {
        ///register callback
        offset = eventID - DISP_DSI_INTERRUPT_EVENTS_START + DISP_LCD_INTERRUPT_EVENTS_NUMBER;
        DISP_CallbackArray[offset].pFunc = pCBStruct->pFunc;
        DISP_CallbackArray[offset].pParam = pCBStruct->pParam;

        DSI_CHECK_RET(DSI_SetInterruptCallback(_DISP_InterruptCallbackProxy));
        DSI_CHECK_RET(DSI_EnableInterrupt(eventID));
    }
    else if(eventID >= DISP_DPI_INTERRUPT_EVENTS_START && eventID <= DISP_DPI_INTERRUPT_EVENTS_END )
    {
        offset = eventID - DISP_DPI_INTERRUPT_EVENTS_START + DISP_LCD_INTERRUPT_EVENTS_NUMBER + DISP_DSI_INTERRUPT_EVENTS_NUMBER;
        DISP_CallbackArray[offset].pFunc = pCBStruct->pFunc;
        DISP_CallbackArray[offset].pParam = pCBStruct->pParam;

        DPI_CHECK_RET(DPI_SetInterruptCallback(_DISP_InterruptCallbackProxy));
        DPI_CHECK_RET(DPI_EnableInterrupt(eventID));
    }
    else
    {
        printk("Invalid event id: %d\n", eventID);
        ASSERT(0);
        return DISP_STATUS_ERROR;        ///TODO: error log
    }
    return DISP_STATUS_OK;
}

