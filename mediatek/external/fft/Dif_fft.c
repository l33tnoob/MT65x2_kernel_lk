#include "DIF_FFT.h"
#include <math.h>
#include <stdio.h>
//#include "typedefs.h"

#ifdef FIXED_FFT
#include "Basicop.h"
#include "Oper_32b.h"
#include "basicop3.h"
#endif

#define PI				3.1415926535

short Wn_r_256[128]=
{
    32758,
    32729,
    32679,
    32610,
    32522,
    32413,
    32286,
    32138,
    31972,
    31786,
    31581,
    31357,
    31114,
    30853,
    30572,
    30274,
    29957,
    29622,
    29269,
    28899,
    28511,
    28106,
    27684,
    27246,
    26791,
    26320,
    25833,
    25330,
    24812,
    24279,
    23732,
    23170,
    22595,
    22006,
    21403,
    20788,
    20160,
    19520,
    18868,
    18205,
    17531,
    16846,
    16151,
    15447,
    14733,
    14010,
    13279,
    12540,
    11793,
    11039,
    10279,
    9512,
    8740,
    7962,
    7180,
    6393,
    5602,
    4808,
    4011,
    3212,
    2411,
    1608,
    804,
    0,
    -804,
    -1608,
    -2411,
    -3212,
    -4011,
    -4808,
    -5602,
    -6393,
    -7180,
    -7962,
    -8740,
    -9512,
    -10279,
    -11039,
    -11793,
    -12540,
    -13279,
    -14010,
    -14733,
    -15447,
    -16151,
    -16846,
    -17531,
    -18205,
    -18868,
    -19520,
    -20160,
    -20788,
    -21403,
    -22006,
    -22595,
    -23170,
    -23732,
    -24279,
    -24812,
    -25330,
    -25833,
    -26320,
    -26791,
    -27246,
    -27684,
    -28106,
    -28511,
    -28899,
    -29269,
    -29622,
    -29957,
    -30274,
    -30572,
    -30853,
    -31114,
    -31357,
    -31581,
    -31786,
    -31972,
    -32138,
    -32286,
    -32413,
    -32522,
    -32610,
    -32679,
    -32729,
    -32758,
    -32768
};

short Wn_i_256[128]=
{
    -804,
    -1608,
    -2411,
    -3212,
    -4011,
    -4808,
    -5602,
    -6393,
    -7180,
    -7962,
    -8740,
    -9512,
    -10279,
    -11039,
    -11793,
    -12540,
    -13279,
    -14010,
    -14733,
    -15447,
    -16151,
    -16846,
    -17531,
    -18205,
    -18868,
    -19520,
    -20160,
    -20788,
    -21403,
    -22006,
    -22595,
    -23170,
    -23732,
    -24279,
    -24812,
    -25330,
    -25833,
    -26320,
    -26791,
    -27246,
    -27684,
    -28106,
    -28511,
    -28899,
    -29269,
    -29622,
    -29957,
    -30274,
    -30572,
    -30853,
    -31114,
    -31357,
    -31581,
    -31786,
    -31972,
    -32138,
    -32286,
    -32413,
    -32522,
    -32610,
    -32679,
    -32729,
    -32758,
    -32768,
    -32758,
    -32729,
    -32679,
    -32610,
    -32522,
    -32413,
    -32286,
    -32138,
    -31972,
    -31786,
    -31581,
    -31357,
    -31114,
    -30853,
    -30572,
    -30274,
    -29957,
    -29622,
    -29269,
    -28899,
    -28511,
    -28106,
    -27684,
    -27246,
    -26791,
    -26320,
    -25833,
    -25330,
    -24812,
    -24279,
    -23732,
    -23170,
    -22595,
    -22006,
    -21403,
    -20788,
    -20160,
    -19520,
    -18868,
    -18205,
    -17531,
    -16846,
    -16151,
    -15447,
    -14733,
    -14010,
    -13279,
    -12540,
    -11793,
    -11039,
    -10279,
    -9512,
    -8740,
    -7962,
    -7180,
    -6393,
    -5602,
    -4808,
    -4011,
    -3212,
    -2411,
    -1608,
    -804,
    0
};


void DIF_FFT(Complex x[],unsigned int Nu)
{
    unsigned int i,j,k,ip,I;
    unsigned int N,LE,LE1,Nv2;
    Complex Wn,W,t,temp;
    N=1;
    N<<=Nu;
    LE=N*2;
    for(i=1; i<=Nu; i++) // the butterfly part
    {
        LE/=2;
        LE1=LE/2;
        Wn.real=1.0;
        Wn.image=0; // Wn(0)
        W.real=(double)cos(PI/(double)LE1);
        W.image=(double)-sin(PI/(double)LE1); // Step of Wn increment
        for(j=1; j<=LE1; j++)
        {
            for(k=j; k<=N; k+=LE)
            {
                I=k-1; // index of upper part of butterfly
                ip=I+LE1; // index of lower part of butterfly
                t.real=x[I].real+x[ip].real;
                t.image=x[I].image+x[ip].image;  // the output of butterfly upper part
                temp.real=x[I].real-x[ip].real;
                temp.image=x[I].image-x[ip].image; // the output of butterfly lower part
                x[ip].real=temp.real*Wn.real-temp.image*Wn.image;  // lower part has to multiply with Wn(k)
                x[ip].image=temp.real*Wn.image+temp.image*Wn.real;
                x[I].real=t.real;
                x[I].image=t.image; // copy t to x[i] directly
            }
            temp.real=W.real*Wn.real-W.image*Wn.image;  // Increment Wn(j) to Wn(j+LE)
            Wn.image=W.real*Wn.image+W.image*Wn.real;
            Wn.real=temp.real;
        }
    }
    Nv2=N/2;
    j=1;
    for(i=1; i<=N-1; i++) // bit-reverse
    {
        if(i<j)
        {
            t.real=x[j-1].real;
            x[j-1].real=x[i-1].real;
            x[i-1].real=t.real;
            t.image=x[j-1].image;
            x[j-1].image=x[i-1].image;
            x[i-1].image=t.image;
        }
        k=Nv2;
        while(k<j )
        {
            j-=k;
            k/=2;
        }
        j+=k;
    }
}


#ifdef FIXED_FFT
//Complex_Fixed C_z[256];
void DIF_FFT_Fix16(Complex_Fixed C_z[], unsigned int Nu, short* Shift_of_C_z) // fixed point version, Nu has to be 8
{

    int iii;
    short i,j,k,ip,I;
    short LE,LE1,Nv2,N_FFT;
    //short FFT_Cos[6]={32610,32138,30274,23170,0,-32768};  //Q15
    //short FFT_Sin[6]={-3212,-6393,-12540,-23170,-32768,0};
    short /*W_r,W_i,*/Wn_r,Wn_i,t_r,t_i,temp_r,temp_i;
    short FFT_Count=0,FFT_Step=1;

    Word32 temp1_L,temp2_L;
    int denorm;
    int min_Qr,min_Qi;
    int mx_specQr, mx_specQi;
    int cnt1;
    //int Max_Q = 16-14; // original 32-30

    //Nu=8;
    N_FFT=256;
    LE=N_FFT*2;
    /*
    for( i=0;i<N_FFT;i++ ) // Q15
    {
    	C_z[i].real=(short)(32768*x[i].real);
    	C_z[i].image=(short)(32768*x[i].image);
    }
    */

    //int
    denorm=0;
    //int
    min_Qr=16;
    min_Qi=16;
    //int mx_specQr, mx_specQi;
    //int cnt1;
    // calculate Q
    /*
    for (cnt1=0;cnt1<N_FFT;cnt1++)
    {
    	if (C_z[cnt1].real)
    	{
    		mx_specQr=norm_s(C_z[cnt1].real);
    		if (mx_specQr < min_Qr)  min_Qr=mx_specQr; // The max value has the smallest Q
    	}
    	if (C_z[cnt1].image)
    	{
    		mx_specQi=norm_s(C_z[cnt1].image);
    		if (mx_specQi < min_Qi)  min_Qi=mx_specQi;
    	}
    }
    if (min_Qi < min_Qr) min_Qr = min_Qi;
    if (min_Qr<Max_Q)
    {
    	for (cnt1=0;cnt1<N_FFT;cnt1++)
    	{
    	#ifdef ENA_MacRound
    		C_z[cnt1].real=shr_r(C_z[cnt1].real,(Max_Q-min_Qr));
    		C_z[cnt1].image=shr_r(C_z[cnt1].image,(Max_Q-min_Qr));
    	#else
    		C_z[cnt1].real=shr(C_z[cnt1].real,(Max_Q-min_Qr));
    		C_z[cnt1].image=shr(C_z[cnt1].image,(Max_Q-min_Qr));
    	#endif
    	}
    	denorm = (Max_Q - min_Qr);
    }else{
    	for (cnt1=0;cnt1<N_FFT;cnt1++)
    	{
    		C_z[cnt1].real=shl(C_z[cnt1].real,(min_Qr-Max_Q));
    		C_z[cnt1].image=shl(C_z[cnt1].image,(min_Qr-Max_Q));
    	}
    	denorm = (Max_Q - min_Qr);
    }
    */
    for(i=1; i<=Nu; i++)
    {
        LE/=2;
        LE1=LE/2;
        Wn_r=(32767);
        Wn_i=0;  //Q15
        //W_r=FFT_Cos[i-1];
        //W_i=FFT_Sin[i-1];

        for(j=1; j<=LE1; j++)
        {
            for(k=j; k<=N_FFT; k+=LE)
            {
                I=k-1;
                ip=I+LE1;
                t_r=add(C_z[I].real,C_z[ip].real);
                t_i=add(C_z[I].image,C_z[ip].image);
                temp_r=sub(C_z[I].real,C_z[ip].real);
                temp_i=sub(C_z[I].image,C_z[ip].image);
                temp1_L=L_mult(temp_r,Wn_r);
                temp2_L=L_msu(temp1_L,temp_i,Wn_i);
                C_z[ip].real=(short)(temp2_L>>16);

                temp1_L=L_mult(temp_r,Wn_i);
                temp2_L=L_mac(temp1_L,temp_i,Wn_r);
                C_z[ip].image=(short)(temp2_L>>16);

                C_z[I].real=t_r;
                C_z[I].image=t_i;
            }

            FFT_Count=FFT_Count+FFT_Step;
            Wn_r= Wn_r_256[FFT_Count-1];
            Wn_i= Wn_i_256[FFT_Count-1];

        }

        /*
        for(j=1;j<=LE1;j++)
        {
        for(k=j;k<=N_FFT;k+=LE)
        {
        	 I=k-1;
        	 ip=I+LE1;
        	 t_r=add(C_z[I].real,C_z[ip].real);
        	 t_i=add(C_z[I].image,C_z[ip].image);
        	 temp_r=sub(C_z[I].real,C_z[ip].real);
        	 temp_i=sub(C_z[I].image,C_z[ip].image);
        	 //temp1_L=L_mult(temp_r,Wn_r);
        	 //temp2_L=L_msu(temp1_L,temp_i,Wn_i);
        	 //C_z[ip].real=(short)(temp2_L>>16);
        	 C_z[ip].real = temp_r;

        	 //temp1_L=L_mult(temp_r,Wn_i);
        	 //temp2_L=L_mac(temp1_L,temp_i,Wn_r);
        	 //C_z[ip].image=(short)(temp2_L>>16);
        	 C_z[ip].image= temp_i;

        	 C_z[I].real=t_r;
        	 C_z[I].image=t_i;
        }

        //FFT_Count=FFT_Count+FFT_Step;
        //Wn_r= Wn_r_256[FFT_Count-1];
        //Wn_i= Wn_i_256[FFT_Count-1];

        }

        min_Qr=16,min_Qi=16;
        for (cnt1=0;cnt1<N_FFT;cnt1++)
        {
        	if (C_z[cnt1].real)
        	{
        		mx_specQr=norm_s(C_z[cnt1].real);
        		if (mx_specQr < min_Qr)  min_Qr=mx_specQr; // The max value has the smallest Q
        	}
        	if (C_z[cnt1].image)
        	{
        		mx_specQi=norm_s(C_z[cnt1].image);
        		if (mx_specQi < min_Qi)  min_Qi=mx_specQi;
        	}
        }
        if (min_Qi < min_Qr) min_Qr = min_Qi;

        if (min_Qr<Max_Q)
        {
        	for (cnt1=0;cnt1<N_FFT;cnt1++)
        	{
        	#ifdef ENA_MacRound
        		C_z[cnt1].real=shr_r(C_z[cnt1].real,(Max_Q-min_Qr));
        		C_z[cnt1].image=shr_r(C_z[cnt1].image,(Max_Q-min_Qr));
        	#else
        		C_z[cnt1].real=shr(C_z[cnt1].real,(Max_Q-min_Qr));
        		C_z[cnt1].image=shr(C_z[cnt1].image,(Max_Q-min_Qr));
        	#endif
        	}
        	denorm += (Max_Q - min_Qr);
        }

         for(j=1;j<=LE1;j++)
         {
        	for(k=j;k<=N_FFT;k+=LE)
        	{
        		 I=k-1;
        		 ip=I+LE1;
        		 //t_r=add(C_z[I].real,C_z[ip].real);
        		 //t_i=add(C_z[I].image,C_z[ip].image);
        		 //temp_r=sub(C_z[I].real,C_z[ip].real);
        		 //temp_i=sub(C_z[I].image,C_z[ip].image);
        		 temp_r=C_z[ip].real;
        		 temp_i=C_z[ip].image;
        		 temp1_L=L_mult(temp_r,Wn_r);
        		 temp2_L=L_msu(temp1_L,temp_i,Wn_i);
        		 C_z[ip].real=(short)(temp2_L>>16);

        		 temp1_L=L_mult(temp_r,Wn_i);
        		 temp2_L=L_mac(temp1_L,temp_i,Wn_r);
        		 C_z[ip].image=(short)(temp2_L>>16);

        		 //C_z[I].real=t_r;
        		 //C_z[I].image=t_i;
        	}

        	FFT_Count=FFT_Count+FFT_Step;
        	Wn_r= Wn_r_256[FFT_Count-1];
        	Wn_i= Wn_i_256[FFT_Count-1];

         }
         */

        FFT_Count=0;
        FFT_Step=FFT_Step<<1;

        min_Qr=16,min_Qi=16;
        for (cnt1=0; cnt1<N_FFT; cnt1++)
        {
            //if (C_z[cnt1].real)
            //{
            mx_specQr=norm_s_NR(C_z[cnt1].real);
            if (mx_specQr < min_Qr)  min_Qr=mx_specQr;
            //}
            //if (C_z[cnt1].image)
            //{
            mx_specQi=norm_s_NR(C_z[cnt1].image);
            if (mx_specQi < min_Qi)  min_Qi=mx_specQi;
            //}
        }
        if (min_Qi < min_Qr) min_Qr = min_Qi;

        if( i<Nu )
        {
            if (min_Qr!=Max_Q)
            {
                for (cnt1=0; cnt1<N_FFT; cnt1++)
                {
                    //#ifdef ENA_MacRound
                    C_z[cnt1].real=shr(C_z[cnt1].real,(Max_Q-min_Qr));
                    C_z[cnt1].image=shr(C_z[cnt1].image,(Max_Q-min_Qr));
                    //#else
                    //	C_z[cnt1].real=shr(C_z[cnt1].real,(Max_Q-min_Qr));
                    //	C_z[cnt1].image=shr(C_z[cnt1].image,(Max_Q-min_Qr));
                    //#endif
                }
                denorm += (Max_Q - min_Qr);
            }
        }
        else
        {
            if ( min_Qr>0 ) // shift to the maxumal value in the last run
            {
                for (cnt1=0; cnt1<N_FFT; cnt1++)
                {
                    C_z[cnt1].real=shl(C_z[cnt1].real,min_Qr);
                    C_z[cnt1].image=shl(C_z[cnt1].image,min_Qr);
                }
                denorm -= min_Qr;
            }
        }

    }

    Nv2=N_FFT/2;
    j=1;

    for(i=1; i<=N_FFT-1; i++)
    {
        if(i<j)
        {
            t_r=C_z[j-1].real;
            C_z[j-1].real=C_z[i-1].real;
            C_z[i-1].real=t_r;
            t_i=C_z[j-1].image;
            C_z[j-1].image=C_z[i-1].image;
            C_z[i-1].image=t_i;
        }

        k=Nv2;

        while( k<j )
        {
            j-=k;
            k/=2;
        }
        j+=k;
    }

    *Shift_of_C_z += denorm;
    //printf("denorm=%d\n",denorm);
    /*
    for( i=0;i<N_FFT;i++ ) // Q15
    {
    	if( denorm>0 )
    	{
    		x[i].real=((float)((int(C_z[i].real))<<denorm))/32768.0;
    		x[i].image=((float)((int(C_z[i].image))<<denorm))/32768.0;
    	}else{
    		x[i].real=((float)((int(C_z[i].real))>>(-denorm)))/32768.0;
    		x[i].image=((float)((int(C_z[i].image))>>(-denorm)))/32768.0;
    	}
    }
    */
}


void DIF_FFT_Fix32(Complex_Fixed_32 C_z[], unsigned int Nu, short* Shift_of_C_z) // 32 bit fixed point version, Nu has to be 8
{
    int i,j,ip,I, cnt1;
    int LE,LE1,Nv2,N_FFT;
    long  Wn_r,Wn_i;									//DLM modify 20060205
    //short W_r,W_i;									//DLM mask
    long t_r,t_i,temp_r,temp_i;					//DLM modify
    //short FFT_Cos[9]={32766,32758,32729,32610,32138,30274,23170,0,-32768};  //Q15
    //short FFT_Sin[9]={-402,-804,-1608,-3212,-6393,-12540,-23170,-32768,0};
    //short Data1_H,Data1_L,Data2_H,Data2_L;
    short FFT_Count=0,FFT_Step=1;
    //extern short EVB_Sum;

    int denorm=0;
    int min_Qr=32,min_Qi;
    int mx_specQr, mx_specQi;
    int W32_temp1_L;
    int W32_temp2_L;
    int k;
    //int DLMshr_Q=0;

    Nu=8; //Nu has to be 8
    N_FFT=256;
    LE=N_FFT*2;

    //int min_Qr=32,
    min_Qi=32;
    //int mx_specQr, mx_specQi;

    /*
    for( i=0;i<N_FFT;i++ ) // Q31
    {
    	C_z[i].real=((int)(32768*x[i].real))<<16;  // 16 bit dynamic range
    	C_z[i].image=((int)(32768*x[i].image))<<16;
    }
    */

    // calculate Q
    /*
    for (cnt1=0;cnt1<N_FFT;cnt1++)
    {
    	if (C_z[cnt1].real)
    	{
    		mx_specQr=norm_l(C_z[cnt1].real);
    		if (mx_specQr < min_Qr)  min_Qr=mx_specQr; // The max value has the smallest Q
    	}
    	if (C_z[cnt1].image)
    	{
    		mx_specQi=norm_l(C_z[cnt1].image);
    		if (mx_specQi < min_Qi)  min_Qi=mx_specQi;
    	}
    }
    //if ((min_Qr == 0) || (min_Qi == 0))
    //		min_Qr += 0;		//dummy  code
    if (min_Qi < min_Qr) min_Qr = min_Qi;
    //if ((min_Qr==0)||(min_Qr>1))
    if (min_Qr<2)
    {
    	for (cnt1=0;cnt1<N_FFT;cnt1++)
    	{
    	#ifdef ENA_MacRound
    		C_z[cnt1].real=L_shr_r(C_z[cnt1].real,(2-min_Qr));
    		C_z[cnt1].image=L_shr_r(C_z[cnt1].image,(2-min_Qr));
    	#else
    		C_z[cnt1].real=L_shr(C_z[cnt1].real,(2-min_Qr));
    		C_z[cnt1].image=L_shr(C_z[cnt1].image,(2-min_Qr));
    	#endif
    	}
    	denorm = (2 - min_Qr);
    }
    */
    for(i=1; i<=Nu; i++)
    {
        LE/=2;
        LE1=LE/2;
        Wn_r=0x7fffffff;
        Wn_i=0;				//Q31,DLM modify
        //W_r=FFT_Cos[i-1];						//DLM mask
        //W_i=FFT_Sin[i-1];						//DLM mask

        for(j=1; j<=LE1; j++)
        {

            for(k=j; k<=N_FFT; k+=LE)
            {
                I=k-1;
                ip=I+LE1;

                t_r=ModifyW32W16_add(C_z[I].real,C_z[ip].real);
                t_i=ModifyW32W16_add(C_z[I].image,C_z[ip].image);
                temp_r=ModifyW32W16_sub(C_z[I].real,C_z[ip].real);
                temp_i=ModifyW32W16_sub(C_z[I].image,C_z[ip].image);

                //int
                W32_temp1_L=ModifyW32W16_mult_r(temp_r,Wn_r);
                //int
                W32_temp2_L=ModifyW32W16_msu_r(W32_temp1_L,temp_i,Wn_i);
                C_z[ip].real=W32_temp2_L;

                W32_temp1_L=ModifyW32W16_mult_r(temp_r,Wn_i);
                W32_temp2_L=ModifyW32W16_mac(W32_temp1_L,temp_i,Wn_r);
                C_z[ip].image=W32_temp2_L;

                C_z[I].real=t_r;
                C_z[I].image=t_i;

            }

            FFT_Count=FFT_Count+FFT_Step;
            Wn_r= ((int)Wn_r_256[FFT_Count-1])<<16;
            Wn_i= ((int)Wn_i_256[FFT_Count-1])<<16;

        }
        FFT_Count=0;
        FFT_Step=FFT_Step<<1;

        min_Qr=32,min_Qi=32;
        for (cnt1=0; cnt1<N_FFT; cnt1++)
        {
            if (C_z[cnt1].real)
            {
                mx_specQr=norm_l_NR(C_z[cnt1].real);
                if (mx_specQr < min_Qr)  min_Qr=mx_specQr;
            }
            if (C_z[cnt1].image)
            {
                mx_specQi=norm_l_NR(C_z[cnt1].image);
                if (mx_specQi < min_Qi)  min_Qi=mx_specQi;
            }
        }
        if (min_Qi < min_Qr)
            min_Qr = min_Qi;
        if( i<Nu ) // shift to Q29 for each FFT path
        {
            if ( min_Qr<2 )
            {
                for (cnt1=0; cnt1<N_FFT; cnt1++)
                {
                    C_z[cnt1].real=L_shr(C_z[cnt1].real,(2-min_Qr));
                    C_z[cnt1].image=L_shr(C_z[cnt1].image,(2-min_Qr));
                }
                denorm += (2 - min_Qr);
            }
        }
        else
        {
            if ( min_Qr>0 ) // shift to the maxumal value in the last run
            {
                for (cnt1=0; cnt1<N_FFT; cnt1++)
                {
                    C_z[cnt1].real=L_shl(C_z[cnt1].real,min_Qr);
                    C_z[cnt1].image=L_shl(C_z[cnt1].image,min_Qr);
                }
                denorm -= min_Qr;
            }
        }
    }

    Nv2=N_FFT/2;
    j=1;
    for(i=1; i<=N_FFT-1; i++)
    {
        if(i<j)
        {
            t_r=C_z[j-1].real;
            C_z[j-1].real=C_z[i-1].real;
            C_z[i-1].real=t_r;
            t_i=C_z[j-1].image;
            C_z[j-1].image=C_z[i-1].image;
            C_z[i-1].image=t_i;
        }

        //int
        k=Nv2;

        while( k<j )
        {
            j-=k;
            k/=2;
        }
        j+=k;
    }

    *Shift_of_C_z += denorm;
    //for( i=0;i<N_FFT;i++ ) // de norm Q(31 - 'denorm')
    //{
    //C_z[i].real>>=(16-min_Qr);	// imitate 16bit storage, for any C_z, give it only 16bit dynamic range
    //C_z[i].real<<=(16-min_Qr);	// imitate 16bit storage
    //C_z[i].image>>=(16-min_Qr);	// imitate 16bit storage
    //C_z[i].image<<=(16-min_Qr);	// imitate 16bit storage
    //x[i].real=((float)(C_z[i].real>>(16-denorm)))/(32768.0);
    //x[i].image=((float)(C_z[i].image>>(16-denorm)))/(32768.0);
    //}

}
#endif
