
#include <stdio.h>
#include <string.h>
#include "tz_cross/trustzone.h"
#include "tz_cross/ta_test.h"
#include "tz_private/system.h"
#include "tz_private/sys_mem.h"
#include "tz_private/sys_ipc.h"
#include "tz_private/ta_sys_mem.h"

/* test TA enable define.
*/
//#define MTEE_TEST_TA_ENABLE


#ifdef MTEE_TEST_TA_ENABLE
static uint32_t _testPatternA[8] = {1, 2, 3, 4, 5, 6, 7, 8};
#define TEST_SRAM_LOOP 1024

volatile int testLoop = 4096*16, testCount;

void _SecureFunction_Test (void);

void test_secureFunc (void *user_data)
{
    int i;

    //printf ("---> hello!!!\n");
    
    for (i = 0; i < testLoop; i ++)
    {
        testCount ++;
    }
}


void *test_threadFunc (void *user_data) 
{
    //printf ("===> test_threadFunc: 0x%x\n", *((uint32_t *) user_data));

    return user_data;
}

#if 0
#define READ_REGISTER_UINT32(reg) \
    (*(volatile unsigned int * const)(reg))
#define WRITE_REGISTER_UINT32(reg, val) \
    (*(volatile unsigned int * const)(reg)) = (val)
//#define DRV_Reg32(dAddr)        *(volatile VAL_UINT32_T *)((dAddr))
//typedef __u64 u64;
//typedef __u32 u32;
typedef unsigned long long u64;
typedef unsigned int u32;

#define TIMEIDX 2
static u32 read_u32_cnt(void)
{
    //printf("(yjdbg) 0xf0008030: 0x%x\n", READ_REGISTER_UINT32(0xf0008030));
    //printf("(yjdbg) 0xf0008034: 0x%x\n", READ_REGISTER_UINT32(0xf0008034));
    //printf("(yjdbg) 0xf0008020: 0x%x\n", READ_REGISTER_UINT32(0xf0008020));
    //printf("(yjdbg) 0xf0008024: 0x%x\n", READ_REGISTER_UINT32(0xf0008024));
    return READ_REGISTER_UINT32(0xf0008008 + TIMEIDX*0x10);
}
static void init_timer2(void)
{
    WRITE_REGISTER_UINT32(0xf0008000 + TIMEIDX*0x10 , 0x31);
    WRITE_REGISTER_UINT32(0xf0008004 + TIMEIDX*0x10, 0x0);
}
u64 get_incr_only_count(void)
{
    static u64 incr_cnt = 0;
    static u32 prev = 0;
    u32 now, diff;

    // init condition
    if (incr_cnt == 0)
    {
	init_timer2();
        prev = read_u32_cnt();
        incr_cnt = (u64) prev;

        return incr_cnt;
    }

    now = read_u32_cnt();
    if (now > prev)
    {
        diff = now - prev;
    }
    else
    {
        diff = (0xffffffff - now)  + prev + 1;
    }
    prev = now;
    incr_cnt += diff;

    return incr_cnt;
}
#endif

static int MTEE_TestSerivce(MTEE_SESSION_HANDLE handle, uint32_t cmd, 
                                uint32_t paramTypes, MTEE_PARAM param[4])
{
    switch (cmd)
    {
        case TZCMD_TEST_ADD:
            if (TZ_GetParamTypes(paramTypes, 0) == TZPT_VALUE_INPUT &&
                TZ_GetParamTypes(paramTypes, 1) == TZPT_VALUE_INPUT &&
                TZ_GetParamTypes(paramTypes, 2) == TZPT_VALUE_OUTPUT)
            {
                param[2].value.a = param[0].value.a + param[1].value.a;
                return 0;
            }
            else 
                return TZ_RESULT_ERROR_BAD_FORMAT;
            break;

        case TZCMD_TEST_MUL:
            if (paramTypes == TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT))
            {
                param[2].value.a = param[0].value.a * param[1].value.a;
                return 0;
            }
            else 
                return TZ_RESULT_ERROR_BAD_FORMAT;
            break;

        case TZCMD_TEST_SECUREFUNC:
            {
                uint32_t testFunc = 0xdead;

                MTEE_SecureFunction ((MTEE_SecureFunctionPrototype) &test_secureFunc, &testFunc);
                
                return TZ_RESULT_SUCCESS;
            }
            break;

        case TZCMD_TEST_THREAD:
            if (paramTypes == TZ_ParamTypes2(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT))
            {
                MTEE_THREAD_HANDLE handle;
                void *result = NULL;
                uint32_t user_data;
                TZ_RESULT ret;
                
                user_data = param[0].value.a;  

                ret = MTEE_CreateThread (&handle, (MTEE_ThreadFunc) &test_threadFunc, &user_data, "0123456789abcdef0123456789abcdef0123456789abcdef");
                if (ret != TZ_RESULT_SUCCESS)
                {
                    printf ("TZCMD_TEST_THREAD error create = 0x%x\n", ret);
                    return ret;
                }

                ret = MTEE_JoinThread (handle, &result);
                if (ret != TZ_RESULT_SUCCESS)
                {
                    printf ("TZCMD_TEST_THREAD error joint = 0x%x\n", ret);
                    return ret;
                }

                param[1].value.a = *((uint32_t *) result);
                
                return TZ_RESULT_SUCCESS;
            }
            else 
                return TZ_RESULT_ERROR_BAD_FORMAT;
            break;  
 
        case TZCMD_TEST_ADD_MEM:
            if (paramTypes == TZ_ParamTypes3(TZPT_MEMREF_INPUT, TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT))
            {
                int *buffer;
                uint32_t size;
                int result;
                
                buffer = param[0].mem.buffer;
                size = param[1].value.a;

                result = 0;
                for (; size != 0; size --)
                {
                    result += *buffer ++;
                }

                param[2].value.a = result;
                
                return TZ_RESULT_SUCCESS;
            }
            else 
                return TZ_RESULT_ERROR_BAD_FORMAT;
            break;

        case TZCMD_TEST_DO_A:
            if (paramTypes == TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT))
            {
                MTEE_SECUREMEM_HANDLE memHandle;
                MTEE_MEM_PARAM p;
                int i;
                uint32_t *buf;    

                memHandle = param[0].value.a;

                // query memory by handle
                TA_Mem_QueryMem (memHandle, &p);
                buf = (uint32_t *) p.buffer;

                printf ("Do A: memory rank = 0x%x (0x%x)\n", MTEE_QueryMemoryrank (buf), (uint32_t) buf);

                // do something with the secure memory
                for (i = 0; i < 8; i ++)
                {
                    *buf ++ = _testPatternA[i];
                }

                param[1].value.a = (uint32_t) p.buffer;
                param[2].value.a = p.size;
                
                return TZ_RESULT_SUCCESS;
            }
            else 
                return TZ_RESULT_ERROR_BAD_FORMAT;
            break;  

        case TZCMD_TEST_DO_B:
            if (paramTypes == TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT))
            {
                MTEE_SECUREMEM_HANDLE memHandle;
                MTEE_MEM_PARAM p;
                int i;
                uint32_t *buf;

                memHandle = param[0].value.a;

                // query memory by handle
                TA_Mem_QueryMem (memHandle, &p);
                buf = (uint32_t *) p.buffer;

                // do something with the secure memory
                for (i = 0; i < 8; i ++)
                {
                    if (*buf != _testPatternA[i])
                    {
                        param[1].value.a = _testPatternA[i];
                        param[2].value.a = 0;
                        return TZ_RESULT_SUCCESS;
                    }
                    buf ++;
                }

                param[1].value.a = (uint32_t) p.buffer;
                param[2].value.a = p.size;
                
                return TZ_RESULT_SUCCESS;
            }
            else 
                return TZ_RESULT_ERROR_BAD_FORMAT;
            break;       

        case TZCMD_TEST_DO_C:
            if (paramTypes == TZ_ParamTypes1(TZPT_VALUE_OUTPUT))
            {
                int i;
                uint32_t *buf;
                
                buf = MTEE_AllocOnchipMem (TEST_SRAM_LOOP * sizeof (uint32_t));
                
                // do something with the secure memory
                for (i = 0; i < TEST_SRAM_LOOP; i ++)
                {
                    buf[i] = i;
                }

                param[0].value.a = (uint32_t) buf;
                
                return TZ_RESULT_SUCCESS;
            }
            else 
                return TZ_RESULT_ERROR_BAD_FORMAT;
            break; 

        case TZCMD_TEST_DO_D:
            if (paramTypes == TZ_ParamTypes1(TZPT_VALUE_INPUT))
            {
                int i;
                uint32_t *buf;

                buf = (uint32_t *) param[0].value.a;

                // do something with the secure memory
                for (i = 0; i < TEST_SRAM_LOOP; i ++)
                {
                    if (buf[i] != i)
                    {
                        return TZ_RESULT_ERROR_GENERIC;
                    }

                    //printf ("--> 0x%x\n", buf[i]);
                }

                MTEE_FreeOnchipMem (buf);
                
                return TZ_RESULT_SUCCESS;
            }
            else 
                return TZ_RESULT_ERROR_BAD_FORMAT;
            break;      

        case TZCMD_TEST_CP_SBUF2NBUF:
            if (paramTypes == TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_MEMREF_INOUT, TZPT_VALUE_INPUT))
            {
                 MTEE_SECUREMEM_HANDLE memHandle;
                 MTEE_MEM_PARAM p;
                 uint8_t* p_nbuf;
                 uint8_t* p_sbuf;                 
                 uint32_t size;

                 size = (param[1].mem.size > param[2].value.a) ? param[2].value.a : param[1].mem.size;
                 p_nbuf = param[1].mem.buffer;
                 
                 memHandle = param[0].value.a;
                 
                 /* query memory by handle */
                 TA_Mem_QueryMem(memHandle, &p);
                 p_sbuf = (uint8_t*) p.buffer;
                 memcpy(p_nbuf, p_sbuf, size );
                 return TZ_RESULT_SUCCESS;
            }
            return TZ_RESULT_ERROR_BAD_FORMAT;
            break;
        case TZCMD_TEST_CP_NBUF2SBUF:
            if (paramTypes == TZ_ParamTypes3(TZPT_VALUE_INPUT, TZPT_MEMREF_INOUT, TZPT_VALUE_INPUT))
            {
                MTEE_SECUREMEM_HANDLE memHandle;
                MTEE_MEM_PARAM p;
                uint8_t* p_nbuf;
                uint8_t* p_sbuf;
                uint32_t size;
                
                size = (param[1].mem.size > param[2].value.a) ? param[2].value.a : param[1].mem.size;
                p_nbuf = param[1].mem.buffer;
                
                memHandle = param[0].value.a;
                
                /* query memory by handle */
                TA_Mem_QueryMem(memHandle, &p);
                p_sbuf = (uint8_t*) p.buffer;
                memcpy(p_sbuf, p_nbuf, size );
                return TZ_RESULT_SUCCESS;
            }
            return TZ_RESULT_ERROR_BAD_FORMAT;
            break;

#if 0
        case TZCMD_TEST_COUNT:
            //printf("(yjdbg) TZCMD_TEST_COUNT\n");
            if (paramTypes == TZ_ParamTypes2(TZPT_VALUE_OUTPUT, TZPT_VALUE_OUTPUT))
            {
		u64 tmp = get_incr_only_count();
                //printf("(yjdbg) TZCMD_TEST_COUNT - 0x%x\n", READ_REGISTER_UINT32(0xf0008020 + 0x08));
                //printf("(yjdbg) TZCMD_TEST_COUNT - 0x%llx\n", tmp);
                param[0].value.a = tmp & 0xffffffff;
                param[1].value.a = tmp >> 32;
            }
            return TZ_RESULT_SUCCESS;
            break;
#endif

        default:
            return TZ_RESULT_ERROR_GENERIC;
    }
}

static TZ_RESULT MTEE_TestInit(const struct MTEE_TA_Function *ta_func)
{
    // Fill in code for TEE system init TA.
    // N/A for Test TA
    return TZ_RESULT_SUCCESS;
}

static TZ_RESULT MTEE_TestSessionCreate(MTEE_SESSION_HANDLE handle,
                                       const struct MTEE_TA_Function *ta_func)
{
    // Fill in code when create a new session for this TA.
    // N/A for Test TA
    printf("TestSession create 0x%x\n", handle);
    return TZ_RESULT_SUCCESS;
}

static void MTEE_TestSessionClose(MTEE_SESSION_HANDLE handle,
                                 const struct MTEE_TA_Function *ta_func)
{
    // Fill in code when a session for this TA is closed.
    // N/A for Test TA
    printf("TestSession close 0x%x\n", handle);
}


static const struct MTEE_TA_Function test_ta_func =
{
    .TAName       = "Test TA",
    .UUID         = TZ_TA_TEST_UUID,
    .init_func    = MTEE_TestInit,
    .create_func  = MTEE_TestSessionCreate,
    .close_func   = MTEE_TestSessionClose,
    .service_func = MTEE_TestSerivce,
};

TA_REGISTER(test_ta_func);
#endif /* MTEE_TEST_TA_ENABLE */
