#include <stdlib.h>
#include <typedefs.h>
#include <platform.h>
#include <dramc.h>
#include <emi_hw.h>
#define DLE_MAX 16
extern int RANK_CURR;
char *opt_dle_value = "0";
int dle_result[DLE_MAX];
static int global_dle_value;

#define TEST_PASS (0)
#define DRAM_START (0x80000000)
#define RANK_SIZE (0x20000000)
//#define NEW_RX_FORMAT
int dramc_dma_test(unsigned int start, unsigned int len, void *ext_arg){
    int err =  TEST_PASS;
    int check_result = (int)ext_arg;
    unsigned int data;
    int i;

    unsigned int *src_buffp1;
    unsigned int *dst_buffp1;;

    src_buffp1 = 0x80000000;
    dst_buffp1 = 0x80001000;
    
    for (i = 0 ; i < (len/sizeof(unsigned int)) ; i++) {
        *((unsigned int *)dst_buffp1+i) = 0;
    }
    for (i = 0 ; i < (len/sizeof(unsigned int)) ; i++) {
        *((unsigned int *)src_buffp1+i) = i;
    }
    //memset(dst_buffp1, 0, len);

     *((volatile unsigned int *)(0x11000098)) = 0x00070000; //BURST_LEN:7-8,R/W
     *((volatile unsigned int *)(0x1100009C)) = src_buffp1;
     *((volatile unsigned int *)(0x110000A0)) = dst_buffp1;
     *((volatile unsigned int *)(0x110000A4)) = len;
     *((volatile unsigned int *)(0x11000088)) = 0x1; //start dma

    while(*((volatile unsigned int *)(0x11000088)));

    for (i = 0 ; i < (len/sizeof(unsigned int)) ; i++) {
        if (*((unsigned int *)dst_buffp1+i) != i) {
     //       print("0x%p: 0x%x != 0x%x\n", (unsigned int *)dst_buffp1+i, *((unsigned int *)dst_buffp1+i), i);
            err = -1;
            break;
        }
    }

    if ((int)ext_arg == 0xFFFFFFFF)
    {
        return err;
    }
    return err;

}

/*in MT6589 test agent 1 only used in DLE calibration*/
int dramc_ta1(unsigned int start, unsigned int len, void *ext_arg){
    int err =  TEST_PASS;
    int check_result = (int)ext_arg;
    unsigned int data;
    /* set test patern length*/
    data = DRAMC_READ_REG(0x40);
    DRAMC_WRITE_REG((data & 0xFF000000) | len, 0x40);


    DRAMC_WRITE_SET((1 << 29) ,DRAMC_CONF2); //Test Agent1


    //dbg_print("0x3C:%x\n",DRAMC_READ_REG(0x3c));
    //dbg_print("0x40:%x\n",DRAMC_READ_REG(0x40));
    //dbg_print("DRAMC_CONF2:%x\n",DRAMC_READ_REG(DRAMC_CONF2));
    while(!(DRAMC_READ_REG(DRAMC_TESTRPT)&(1 << 10)));

    /*
     * NoteXXX: Need to wait for at least 400 ns
     *          After checking the simulation result,
     *          there will be a delay on DLE_CNT_OK/DM_CMP_ERR updates after getting DM_CMP_CPT.
     *          i.e; After getting the complete status, need to wait for a while before reading DLE_CNT_OK/DM_CMP_ERR in the TESTRPT register.
     */
    delay_a_while(400);
    //dbg_print("DRAMC_TESTRPT:%x\n",DRAMC_READ_REG(DRAMC_TESTRPT));
    if (check_result) {
        if (DRAMC_READ_REG(DRAMC_TESTRPT) & (1 << 14)) {
            err = -1;
        }
        /*} else if (!(DRAMC_READ_REG(DRAMC_TESTRPT) & (1 << 18))) {
            err = -1;
        }*/
    }

    DRAMC_WRITE_CLEAR((1 << 29) ,DRAMC_CONF2); //disable test agent1


    //dbg_print("ext_arg:%x,err:%x\n",ext_arg,err);
    if ((int)ext_arg == 0xFFFFFFFF)
    {
        return err;
    }

    return;

}
char *dram_driving_tbl[] =
{
	/* DRAM I/O Driving */
	//"1",  /* 34.3	ohm:	0001 */
	//"2",  /* 40	ohm:	0010 */
	//"3",  /* 48	ohm:	0011 */
	"4",  /* 60	ohm:	0100 */
	//"5",  /* 68.6	ohm:	0101 */
	//"6",  /* 80	ohm:	0110 */
	//"7",  /* 120	ohm:	0111 */
	NULL,
};

void dram_driving_factor_handler(char *factor_value) {
    int curr_val = atoi(factor_value);

}



char *dle_tbl[] =
{
	/* DLE: 0x0~0xf */
	/*"0", "1",*/ "2", "3", "4",
	//"5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15",
        "5","6", "7", "8", "9","10",
	NULL,
};

void dle_factor_handler(char *factor_value) {
    int curr_val = atoi(factor_value);

    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_DDR2CTL/* 0x7C */) & 0xFFFFFF8F)	/* Reserve original values for DRAMC_DDR2CTL[0:3, 7:31] */
		| ((curr_val & 0x7) << 4),			/* DATLAT: DRAMC_DDR2CTL[4:6], 3 bits */
		DRAMC_DDR2CTL/* 0x7C */);

    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_PADCTL4/* 0xE4 */) & 0xFFFFFFEF)	/* Reserve original values for DRAMC_DDR2CTL[0:3, 5:31] */
		| (((curr_val >> 3) & 0x1) << 4),			/* DATLAT3: DRAMC_PADCTL1[4], 1 bit */
		DRAMC_PADCTL4/* 0xE4 */);
}

void driving_factor_handler(char *factor_value);

#if defined(NEW_RX_FORMAT) /*MT6583*/
tuning_factor rx_tuning_factors[] = {
    /* {"<-DQ->|<-DQS-> Input Delay",		dqi_dqs_dly_tbl,		NULL, NULL, dqi_dqsi_dly_factor_handler},
    {"Tx I/O Driving (DRVP, DRVN)",     driving_tbl_for_rx, NULL, NULL, driving_factor_handler},
    */
    {"DRAM Driving Strength",			dram_driving_tbl,	NULL, NULL, dram_driving_factor_handler},
    {"DLE",					dle_tbl,		NULL, NULL, dle_factor_handler},
};
#else
tuning_factor rx_tuning_factors[] = {
   /*   {"DQ Input Delay",				dqi_dly_tbl,		NULL, NULL, dqi_dly_factor_handler},
    {"DQS Input Delay",				dqsi_dly_tbl,		NULL, NULL, dqsi_dly_factor_handler},
    {"DRAM Driving Strength",			dram_driving_tbl,	NULL, NULL, dram_driving_factor_handler},*/
    {"DLE",					dle_tbl,		NULL, NULL, dle_factor_handler},
 /*   {"Tx I/O Driving (DRVP, DRVN)",		driving_tbl_for_rx,	NULL, NULL, driving_factor_handler}, */
};
#endif


void dqso_dly_factor_handler(char *factor_value) 
{
    int curr_val = atoi(factor_value);
    curr_val = curr_val;

    DRAMC_WRITE_REG((DRAMC_READ_REG(DRAMC_PADCTL3/* 0x14 */) & 0xFFFF0000)	/* Reserve original values for __DRAMC_PADCTL3[16:31] */
		| ((curr_val & 0xF) << 0)					/* DQS0DLY: __DRAMC_PADCTL3[0:3],	4 bits */
		| ((curr_val & 0xF) << 4)					/* DQS1DLY: __DRAMC_PADCTL3[4:7],	4 bits */
		| ((curr_val & 0xF) << 8)					/* DQS2DLY: __DRAMC_PADCTL3[8:11],	4 bits */
		| ((curr_val & 0xF) << 12),				/* DQS3DLY: __DRAMC_PADCTL3[12:15],	4 bits */
		DRAMC_PADCTL3/* 0x14 */);
}

char *driving_tbl[] =
{
	/* Tx I/O Driving */
#if LPDDR == 2
        "8", "9", "10", "11", "12", 
#elif LPDDR == 3
        "6", "7","8", "9", "10",
        //"4", "5", "6", "7", "8", "9", "10", "11", "12", "15",
#else
        "6", "7", "8", "9", "10",

#endif
	NULL,
};





void ett_print_dram_driving(char *name, int curr_val) {
}


void ett_rextdn_sw_calibration(){
    unsigned int drvp, drvn;
    unsigned int drvp_sel, drvn_sel;
    unsigned int tmp;
    unsigned int sel;

    dbg_print("Start REXTDN SW calibration...\n");
    for(sel = 0; sel < 16; sel ++){ 
        
    /* 1.initialization */
            DRAMC_WRITE_REG(0x0,DRAMC_DLLSEL) ;
            /* set DRVP_SEL as 0, set DRVN_SEL as 0 */
            DRAMC_WRITE_REG(0x0,0x100) ;
            drvp_sel = (sel>>2)&0x3;
            drvn_sel = (sel>>0)&0x3;
            dbg_print("DRVP_SEL:%d,DRVN_SEL:%d\n",drvp_sel,drvn_sel);
    /* 2.DRVP calibration */
            /* setup DRVP_SEL*/
            DRAMC_WRITE_REG(drvp_sel<<16,0x100) ;
            /*enable P drive calibration*/
            DRAMC_WRITE_REG(0x600f0,DRAMC_DLLSEL) ;
            /*Start P drive calibration*/
            for(drvp = 0 ; drvp <=15; drvp ++){
                DRAMC_WRITE_REG((drvp << 12),DRAMC_DLLSEL); 
                /* check the 3dc[31] from 0 to 1 */
                if ((DRAMC_READ_REG(0x3dc) >> 31)  == 1){
                    dbg_print("P drive:%d\n",drvp);
                    break;
                }
            }
            if (drvp == 16)
                dbg_print("No valid P drive");
    /* 3.DRVN calibration */
            /* setup DRVN_SEL*/
            DRAMC_WRITE_REG(drvn_sel<<16,0x100) ;
            /*enable N drive calibration*/
            tmp = (DRAMC_READ_REG(DRAMC_DLLSEL)&0xfffbffff)|(0x0001000F);
            DRAMC_WRITE_REG(tmp,DRAMC_DLLSEL) ;
            /*Start N drive calibration*/
            for(drvn = 0 ; drvn <=15; drvn ++){
                DRAMC_WRITE_REG((drvn << 8),DRAMC_DLLSEL); 
                /* check the 3dc[31] from 1 to 0 */
                if ((DRAMC_READ_REG(0x3dc) >> 31)  == 0){
                    /* fixup the drvn by minus 1 */
                    drvn--;
                    dbg_print("N drive:%d\n",drvn);
                    break;
                }
            }
            if (drvp == 16)
                dbg_print("No valid N drive");

    /* 4.setup DRVP,DRVN according to calibration */
            dbg_print("P drive:%d\n",drvp);
            dbg_print("N drive:%d\n",drvn);

    }
    return;

}
#if defined(NEW_RX_FORMAT) /*MT6583*/
tuning_factor dle_tuning_factors[] = 
{
    { .name = "DRAM Driving Strength",
      .factor_tbl = dram_driving_tbl,
      .curr_val = NULL, 
      .opt_val = NULL, 
      .factor_handler = dram_driving_factor_handler,
    },
    {
     .name = "DLE",
     .factor_tbl = dle_tbl,
      .curr_val = NULL, 
      .opt_val = NULL, 
      .factor_handler = dle_factor_handler,
    },
};
#else
tuning_factor dle_tuning_factors[] = 
{
    {
     .name = "DLE",
     .factor_tbl = dle_tbl,
      .curr_val = NULL, 
      .opt_val = NULL, 
      .factor_handler = dle_factor_handler,
    },
};
#endif

test_case dle_test_cases[] = 
{
    //{dramc_ta1, 0x0, 0x7FF, 0xFFFFFFFF}
    {dramc_dma_test, 0x0, 0x80, 0xFFFFFFFF}

};


void ett_print_dle_banner(unsigned int n) 
{
    unsigned int i;

    if (n == 1)
    {
#ifdef RELEASE

        dbg_print("=============================================\n");
        dbg_print("(");
        for ( i = 2 ; i < ETT_TUNING_FACTOR_NUMS(dle_tuning_factors) ; i++)
        {
            dbg_print("%d ", atoi(dle_tuning_factors[i].curr_val));
        }
        dbg_print(")\n");
        dbg_print("=============================================\n");
#else

        dbg_print("=============================================\n");
        for ( i = 2 ; i < ETT_TUNING_FACTOR_NUMS(dle_tuning_factors) ; i++)
        {
            if (dle_tuning_factors[i].factor_tbl == dram_driving_tbl)
            {
                ett_print_dram_driving(dle_tuning_factors[i].name, atoi(dle_tuning_factors[i].curr_val));
            }
            else 
            {
                dbg_print("%s = %d\n", dle_tuning_factors[i].name, atoi(dle_tuning_factors[i].curr_val));
            }
        }

        dbg_print("X-axis: %s\n", dle_tuning_factors[0].name);
        dbg_print("Y-axis: %s\n", dle_tuning_factors[1].name);
        dbg_print("=============================================\n");
#ifdef NEW_RX_FORMAT
        dbg_print("    F  C   8   4   0   4   8   C   10  14  18  1C  20  24  28  2C  30  34  38  3C  40  44  48  4C  50  54  58  5C  60  64  68  6C  70  74  78  7C 7F\n");
        dbg_print("    <--*---*---*-->|<--*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*-->\n");
//        dbg_print("        F    E    D    C    B    A    9    8    7    6    5    4    3    2    1    0    4    8    C   10   14   18   1C   20   24   28   2C   30   34   38   3C   40   44   48   4C   50   54   58   5C   60   64   68   6C   70   74   78   7C\n");
//        dbg_print("    -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
#else
        dbg_print("        0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F\n");
        dbg_print("    --------------------------------------------------------------------------------\n");
#endif
#endif
    }
}

unsigned int ett_print_dle_result() 
{
    unsigned int i, score = 1;
    int result;

    for ( i = 0 ; i < ETT_TEST_CASE_NUMS(dle_test_cases) ; i++) {
        /* fixup the test start_address */
        dle_test_cases[i].start = DRAM_START; 
        if (dle_test_cases[i].test_case(dle_test_cases[i].start, dle_test_cases[i].range, dle_test_cases[i].ext_arg) == TEST_PASS) {
            if (i == (ETT_TEST_CASE_NUMS(dle_test_cases)-1)) {
#ifdef NEW_RX_FORMAT
                dbg_print("1");
#else
                dbg_print("    1");
#endif
            }
        dle_result[global_dle_value++] = score; 
        } else {
#ifdef NEW_RX_FORMAT
            dbg_print("0");
#else
            dbg_print("    0");
#endif
            score = 0;
            dle_result[global_dle_value++] = score; 
            break;
        }

    }

    return score;
}

void ett_print_dle_before_each_round_of_loop_zero() 
{
    DDR_PHY_RESET();
    //opt_dle_value = 0;
}

void ett_print_dle_after_each_round_of_loop_zero() 
{
    delay_a_while(200);
}

void ett_print_dle_before_start_loop_zero() 
{
#if !defined(RELEASE)
    dbg_print("%B:|", atoi(dle_tuning_factors[1].curr_val));
#endif
#if defined(NEW_RX_FORMAT)
    //dle_factor_handler("0");
#endif
}

void ett_print_dle_after_finish_loop_n(unsigned int n) 
{
    if (n == 0) {
        dbg_print("\n");
    }
}

void ett_calc_dle_opt_value(unsigned int n, unsigned int *score, unsigned int *high_score) 
{
    
}

 /*
     *     Related Registers (Latency between DRAMC and PHY)
     *             - PADCTL4  bit4        DATLAT3
     *             - DDR2CTL  bit 6~4  DATLAT
     *     Algorithm
     *             -  Set DLE from 4 to 14 and check if data is correct.>
     */

int do_dle_calib(void) {
    int ix;
    global_dle_value = 0;
    print_callbacks cbs = {
        .ett_print_banner = ett_print_dle_banner,
        .ett_print_before_start_loop_zero = ett_print_dle_before_start_loop_zero,
        .ett_print_before_each_round_of_loop_zero = ett_print_dle_before_each_round_of_loop_zero,
        .ett_print_result = ett_print_dle_result,
        .ett_print_after_each_round_of_loop_zero = ett_print_dle_after_each_round_of_loop_zero,
        .ett_calc_opt_value = ett_calc_dle_opt_value,
        .ett_print_after_finish_loop_n = ett_print_dle_after_finish_loop_n,
    };
/*
   * Main function
   *     - Create dle_result[]
   * Output
   *     -   |    0    0    1    1    1    1    0    0    0    0    0 >
   */

    if (ETT_TUNING_FACTOR_NUMS(dle_tuning_factors) > 0) {
        ett_recursive_factor_tuning(ETT_TUNING_FACTOR_NUMS(dle_tuning_factors)-1, dle_tuning_factors, &cbs);
    }
#if 1
    for(ix = 0; (ix < DLE_MAX) && (dle_tbl[ix+1] !=NULL); ix++){
        if (dle_result[ix] == 1 && dle_result[ix+1] == 1){
            if (dle_tbl[ix+1] > *opt_dle_value)
                opt_dle_value = dle_tbl[ix+1];
            print("opt_dle value:%c\n",*opt_dle_value);
            break;
        }
    }

	//opt_dle_value="4";
    print("opt_dle value:%c\n",*opt_dle_value);
    /* setup the opt dle value according to calibration result*/
    if (*opt_dle_value!=0x30)
    { //0x30 == "0"
        dle_factor_handler(opt_dle_value);
        return 0;
    }
    else
    {
        dbg_print("cannot find opt_dle value\n");
        return -1;
    }
#endif
}

