/*
 *  LZ4K Compressor by Vovo
 */
#if 0 // INTERNAL
#include <linux/types.h>
//#include <stddef.h>
#endif // INTERNAL
// INTERNAL START
//#define PC_TEST
//#ifdef PC_TEST
#if 0
#include "lz4k_utils.h"

#else

#include <stddef.h>

#define LZ4K_DEBUG_encode_hash_time_start()
#define LZ4K_DEBUG_encode_hash_time_end()
#define LZ4K_DEBUG_encode_literal_time_start()
#define LZ4K_DEBUG_encode_literal_time_end()
#define LZ4K_DEBUG_encode_literal_2_time_start()
#define LZ4K_DEBUG_encode_literal_2_time_end()
#define LZ4K_DEBUG_encode_match_scan_time_start()
#define LZ4K_DEBUG_encode_match_scan_time_end()
#define LZ4K_DEBUG_encode_match_off_time_start()
#define LZ4K_DEBUG_encode_match_off_time_end()
#define LZ4K_DEBUG_encode_match_len_time_start()
#define LZ4K_DEBUG_encode_match_len_time_end()
#define LZ4K_DEBUG_encode_remain_time_start()
#define LZ4K_DEBUG_encode_remain_time_end()

#define LZ4K_DEBUG_update_lz_type_bit_count()
#define LZ4K_DEBUG_update_match_off_bit_count(bits, orig, encode)
#define LZ4K_DEBUG_update_match_len_bit_count(bits, orig, encode)
#define LZ4K_DEBUG_update_literal_len_bit_count(bits, orig, encode)
#define LZ4K_DEBUG_update_literal_ch_bit_count(bits, orig, encode)
#define LZ4K_DEBUG_update_padding_bit_count(bits)

#define LZ4K_DEBUG_print(arg1, ...)
#define LZ4K_DEBUG_assert(x)
#define LZ4K_DEBUG_dprint(arg1, ...)
#define LZ4K_DEBUG_dprint_verbose(arg1, ...)

#define LZ4K_DEBUG_encode_start()
#define LZ4K_DEBUG_encode_end()

#endif

#define DO_LZ4K_ENCODE_VERIFY // INTERNAL START
#ifdef DO_LZ4K_ENCODE_VERIFY
#include <stdio.h>
#endif // DO_LZ4K_ENCODE_VERIFY, INTERNAL END

#define LAZY_MATCH

#ifdef LAZY_MATCH
#include "lz4k_matcher.h"
#endif
// INTERNAL END
//#define PREVIOUS_MATCH_OFF
//#define SAVING_OFFSET

#ifndef ALLOW_MATCH_2 // INTERNAL
// max_bit = 10
// add 3 null entries: 0, 1, 2, to save a "-3" arithmetic
unsigned short lz4k_matchlen_encode[32] =
{ 0, 0, 0, 1024,  1026,  2049,  2053,  2057,  2061,  3091,  3123,  3083,  2563,  3643,  3707,  3115,  3099,  4119,  3591,  4247,  3655,  4791,  4183,  4311,  3623,  5047,  4727,  4983,  3687,  4855,  5111,  4151,  };
// entry 6, 7 (2057 and 2053) is reversed to speed decode
#else // ALLOW_MATCH_2 INTERNAL START
unsigned short lz4k_matchlen_encode[29] =
{ 0, 0, 512,  514,  769,  1029,  1037,  1027,  1291,  1543,  1847,  1307,  1575,  2063,  2191,  1559,  1911,  2127,  2255,  2095,  2223,  0, 0, 0, 2159,  0, 0, 0, 2287,  };
#endif // ALLOW_MATCH_2 INTERNAL END

#ifndef PREVIOUS_MATCH_OFF // INTERNAL
// max_bit = 10
unsigned short lz4k_matchoff_encode[32] =
{ 1024,  1032,  1292,  1028,  1554,  1308,  1586,  1546,  1578,  1562,  1830,  1594,  1894,  1282,  1814,  1542,  2158,  1878,  2286,  1846,  2078,  1910,  2206,  1806,  2142,  2270,  2110,  1870,  2238,  1838,  2174,  2302,  };
#else // PREVIOUS_MATCH_OFF INTERNAL START
int lz4k_matchoff_encode_count = 44;
unsigned short lz4k_matchoff_encode_symbol[44] =
//{0, 0, 0, 4, 0, 8, 12, 16, 24, 20, 28, 32, 36, 40, 48, 56, 60, 64, 44, 52, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124, 128, 1, 6, 10, 14, 18, 22, 30, 26};
{0, 4, 8, 16, 12, 20, 24, 1, 6, 28, 32, 36, 40, 44, 48, 56, 60, 64, 10, 14, 18, 22, 26, 30, 52, 68, 72, 76, 80, 84, 88, 92, 96, 100, 104, 108, 112, 116, 120, 124, 128, 2, 34, 132};
unsigned short lz4k_matchoff_encode[44] =
//{ 768,  1028,  1292,  1308,  1538,  1570,  1554,  1586,  1546,  1834,  1898,  1818,  1882,  1850,  1914,  1798,  1862,  1830,  2150,  2278,  2070,  2198,  2134,  2262,  2102,  2230,  2166,  2294,  2062,  2190,  2126,  2254,  2094,  2222,  2158,  2286,  2078,  2206,  2142,  2270,  2110,  2238,  2174,  2302,  };
{ 768,  1284,  1300,  1292,  1564,  1596,  1538,  1826,  1890,  1810,  1874,  1842,  1906,  1802,  1866,  1834,  1898,  1818,  1882,  1850,  1914,  1798,  1862,  1830,  1894,  1814,  1878,  1846,  1910,  1806,  2126,  2254,  2094,  2222,  2158,  2286,  2078,  2206,  2142,  2270,  2110,  2238,  2174,  2302,  };
#endif // PREVIOUS_MATCH_OFF INTERNAL END

// max_bit = 7
unsigned short lz4k_literallen_encode[18] =
{0,  128,  385,  517,  525,  515,  651,  775,  807,  667,  919,  983,  951,  1015,  911,  975,  943,  1007,  };
// add 1 null entries: 0 to save a "-1" arithmetic

// max_bit = 9
#ifndef NO_LITERAL_CHAR_ENTROPY // INTERNAL
unsigned short lz4k_literalch_encode[256] =
{ 2048,  3092,  3596,  3660,  3628,  4154,  4282,  4218,  3692,  3612,  3676,  4346,  4102,  4230,  4166,  4294,  3644,  4134,  4262,  4198,  4326,  4861,  5117,  4611,  4118,  4867,  4246,  4182,  4310,  4150,  4278,  4214,  3124,  4342,  4110,  4238,  4174,  4302,  4739,  4995,  3708,  4142,  4675,  4931,  4270,  4803,  4206,  4334,  3586,  4126,  4254,  4190,  4318,  5059,  4643,  4899,  4158,  4771,  5027,  4707,  4963,  4835,  5091,  4627,  2564,  3650,  4286,  4222,  4350,  4883,  4755,  5011,  4097,  4691,  4947,  4225,  4161,  4289,  4129,  4257,  3618,  3682,  4193,  4321,  4113,  4819,  5075,  4659,  4241,  4915,  4787,  5043,  4723,  4979,  4851,  4177,  4305,  3602,  4145,  4273,  4209,  3666,  4337,  4105,  4233,  3634,  5107,  4619,  4169,  4297,  3698,  3594,  3658,  4137,  3626,  4265,  3690,  4201,  4329,  4875,  4121,  4249,  4747,  5003,  4683,  4939,  4811,  5067,  3610,  4651,  4907,  4779,  4185,  5035,  4715,  4971,  4313,  4843,  5099,  4635,  4891,  4763,  5019,  4699,  4153,  4955,  4827,  5083,  4667,  4923,  4795,  5051,  4281,  4731,  4987,  4859,  5115,  4615,  4871,  4743,  4217,  4999,  4679,  4935,  4807,  5063,  4647,  4903,  4345,  4775,  5031,  4711,  4967,  4839,  5095,  4631,  4101,  4887,  4759,  5015,  4229,  4695,  4951,  4823,  4165,  5079,  4663,  4919,  4293,  4791,  5047,  4727,  4133,  4983,  4855,  5111,  4623,  4879,  4751,  5007,  4261,  4687,  4943,  4815,  5071,  4655,  4911,  4783,  4197,  4325,  4117,  4245,  4181,  4309,  5039,  4719,  4149,  4975,  4847,  5103,  4639,  4895,  4767,  5023,  3674,  4277,  4703,  4213,  4959,  4341,  4831,  5087,  4109,  4671,  4927,  4799,  4237,  4173,  4301,  5055,  4141,  4269,  4205,  4333,  4125,  4253,  4735,  4189,  4317,  4157,  4285,  4991,  4221,  4863,  5119,  2056,  };
#else // INTERNAL START
unsigned short lz4k_literalch_encode[256] =
{ 4096, 4097, 4098, 4099, 4100, 4101, 4102, 4103, 4104, 4105, 4106, 4107, 4108, 4109, 4110, 4111, 4112, 4113, 4114, 4115, 4116, 4117, 4118, 4119, 4120, 4121, 4122, 4123, 4124, 4125, 4126, 4127, 4128, 4129, 4130, 4131, 4132, 4133, 4134, 4135, 4136, 4137, 4138, 4139, 4140, 4141, 4142, 4143, 4144, 4145, 4146, 4147, 4148, 4149, 4150, 4151, 4152, 4153, 4154, 4155, 4156, 4157, 4158, 4159, 4160, 4161, 4162, 4163, 4164, 4165, 4166, 4167, 4168, 4169, 4170, 4171, 4172, 4173, 4174, 4175, 4176, 4177, 4178, 4179, 4180, 4181, 4182, 4183, 4184, 4185, 4186, 4187, 4188, 4189, 4190, 4191, 4192, 4193, 4194, 4195, 4196, 4197, 4198, 4199, 4200, 4201, 4202, 4203, 4204, 4205, 4206, 4207, 4208, 4209, 4210, 4211, 4212, 4213, 4214, 4215, 4216, 4217, 4218, 4219, 4220, 4221, 4222, 4223, 4224, 4225, 4226, 4227, 4228, 4229, 4230, 4231, 4232, 4233, 4234, 4235, 4236, 4237, 4238, 4239, 4240, 4241, 4242, 4243, 4244, 4245, 4246, 4247, 4248, 4249, 4250, 4251, 4252, 4253, 4254, 4255, 4256, 4257, 4258, 4259, 4260, 4261, 4262, 4263, 4264, 4265, 4266, 4267, 4268, 4269, 4270, 4271, 4272, 4273, 4274, 4275, 4276, 4277, 4278, 4279, 4280, 4281, 4282, 4283, 4284, 4285, 4286, 4287, 4288, 4289, 4290, 4291, 4292, 4293, 4294, 4295, 4296, 4297, 4298, 4299, 4300, 4301, 4302, 4303, 4304, 4305, 4306, 4307, 4308, 4309, 4310, 4311, 4312, 4313, 4314, 4315, 4316, 4317, 4318, 4319, 4320, 4321, 4322, 4323, 4324, 4325, 4326, 4327, 4328, 4329, 4330, 4331, 4332, 4333, 4334, 4335, 4336, 4337, 4338, 4339, 4340, 4341, 4342, 4343, 4344, 4345, 4346, 4347, 4348, 4349, 4350, 4351, };
#endif // INTERNAL END

#define RESERVE_16_BITS() \
if (bitstobeoutput >= 16) { \
    *( (unsigned short *)op) = (unsigned short) (bits_buffer32 & 0xffff); \
    op += 2; \
    bits_buffer32 = bits_buffer32 >> 16; \
    bitstobeoutput -= 16; \
}

#define STORE_BITS(bits, code) \
    bits_buffer32 |= (code) << bitstobeoutput; \
    bitstobeoutput += (bits);

// minimal match: 3
static size_t
_lz4k_do_compress(const unsigned char *in, size_t in_len,
    unsigned char *out, size_t *out_len, void *wrkmem)
{
    // ip: scan end, ii: compressed end
    const unsigned char *ip = in;
    unsigned char *op = out;
    const unsigned char * const in_end = in + in_len;
#ifndef ALLOW_MATCH_2 // INTERNAL
    const unsigned char * const ip_end = in + in_len - 3;
#else // INTERNAL START
    const unsigned char * const ip_end = in + in_len - 2;
#endif // INTERNAL END
    const unsigned char *ii = ip;
#ifndef LAZY_MATCH // INTERNAL
    const unsigned char ** const dict = wrkmem;
#endif // INTERNAL
    unsigned int bitstobeoutput = 0;
    unsigned int bits_buffer32 = 0;
#ifdef PREVIOUS_MATCH_OFF // INTERNAL START
    size_t previous_m_off_1 = -1;
#endif // PREVIOUS_MATCH_OFF INTERNAL END

    // 1 bit type
    bitstobeoutput = 1;
#ifdef LAZY_MATCH // INTERNAL START
    bits_buffer32 = 1;
#else
    bits_buffer32 = 0;
#endif // INTERNAL END

#ifdef LAZY_MATCH // INTERNAL START
    DECLARE_MATCHER_DATA();
#endif // INTERNAL END
    for (;;)
    {
        const unsigned char *m_pos;
        //register const unsigned char *m_pos;
        LZ4K_DEBUG_encode_hash_time_start();
        //printf("loop: %ld\n", ip - in);
#ifndef LAZY_MATCH // INTERNAL
        {
            size_t dindex;
            // the bottleneck 1
            // read 4 bytes, can check 2 hashes
            unsigned int ip_content = *(unsigned int*)ip;
            unsigned int hash_temp = ip_content ^ (ip_content >> 12);
            // used (2 ** 12) * sizeof(int) = 16kB, can be fit into 32kB L1 cache
            dindex = hash_temp & 0xfff;
            m_pos = dict[dindex];
            dict[dindex] = ip;

            // check 3 bytes
            if (m_pos < in || m_pos >= ip || ( (*(unsigned int*)m_pos << 8) != (ip_content << 8) ) ) {
                // advance a literal
                ++ip;
                // hash of the next
                dindex = (hash_temp >> 8) & 0xfff;
                m_pos = dict[dindex];
                dict[dindex] = ip;
                if (m_pos < in || m_pos >= ip || ( (*(unsigned int*)m_pos << 8) != (ip_content & 0xffffff00) ) ) {
                    ++ip;
                    if (__builtin_expect(!!(ip >= ip_end), 0) )
                        break;
                    LZ4K_DEBUG_encode_hash_time_end();
                    continue;
                }
            }
        }
// INTERNAL START
#else
        //DO_LAZY_MATCH_1();
        {
            int index = ip - in;
            if (best_match_off[index] == 0) {
                //printf("mark 1\n");
                ip += best_match_len[index];
                if (__builtin_expect(!!(ip >= ip_end), 0) )
                    break;
                continue;
            }
            //printf("mark 2\n");
            m_pos = ip - best_match_off[index];
        }
#endif // LAZY_MATCH
// INTERNAL END

        // a match
        // not need to check the 2nd byte, because the hash match, 1st, 3rd byte match, the 2nd byte must match
        LZ4K_DEBUG_encode_hash_time_end();
        // store current literal run
        {
            size_t lit = ip - ii;

            // type, 0: literal, 1: match
            LZ4K_DEBUG_update_lz_type_bit_count();
            if (lit > 0) {
                if (lit == 1) {
                    int value, bits, code;
                    LZ4K_DEBUG_encode_literal_time_start();
                    // reserve at least 16 bits output buffer
                    RESERVE_16_BITS();
                    // output at most 9 + 1 + 1 = 11 bits
                    value = lz4k_literalch_encode[*ii++];
                    bits = value >> 9;
                    code = (value & 0x1ff) << 2;
                    // litlen and type: '0' .. '0'
                    STORE_BITS(bits + 2, code);
                    LZ4K_DEBUG_update_literal_len_bit_count(1, 1, 0);
                    LZ4K_DEBUG_update_literal_ch_bit_count(bits, *(ii-1), code);
                    LZ4K_DEBUG_encode_literal_time_end();
                } else if (lit == 2) {
                    int value, bits, code;
                    int value2, bits2, code2;
                    LZ4K_DEBUG_encode_literal_time_start();
                    // reserve at least 24 bits output buffer
                    RESERVE_16_BITS();
                    if (bitstobeoutput > (32 - 22) ) {
                        *op++ = (unsigned char) (bits_buffer32 & 0xff);
                        bits_buffer32 = bits_buffer32 >> 8;
                        bitstobeoutput -= 8;
                    }
                    // output at most 9 + 9 + 3 + 1 = 22 bits
                    value = lz4k_literalch_encode[*ii++];
                    bits = value >> 9;
                    code = value & 0x1ff;
                    value2 = lz4k_literalch_encode[*ii++];
                    bits2 = value2 >> 9;
                    code2 = value2 & 0x1ff;
                    // litlen and type: '001' .. '0'
                    //bits_buffer32 |= ( (code2 << (4 + bits) ) | (code << 4) | 2) << bitstobeoutput;
                    bits_buffer32 |= ( ( ( (code2 << bits) | code) << 4) | 2) << bitstobeoutput;
                    bitstobeoutput += bits2 + bits + 4;
                    LZ4K_DEBUG_update_literal_len_bit_count(3, 2, 1);
                    LZ4K_DEBUG_update_literal_ch_bit_count(bits, *(ii-2), code);
                    LZ4K_DEBUG_update_literal_ch_bit_count(bits2, *(ii-1), code2);
                    LZ4K_DEBUG_encode_literal_time_end();
                } else {
                    // lit >= 3
                    // reserve at least 24 bits output buffer
                    LZ4K_DEBUG_encode_literal_2_time_start();
                    // litlen in range 1 ~ 4096
                    if (lit <= 17) {
                        // output literal len, at most 7 bits + 1bit
                        int value, bits, code;
                        RESERVE_16_BITS();
                        value = lz4k_literallen_encode[lit];
                        bits = value >> 7;
                        code = (value & 0x7f) << 1;
                        // output type literal
                        STORE_BITS(bits + 1, code);
                        LZ4K_DEBUG_update_literal_len_bit_count(bits, lit, code);
                    } else {
                        // output type literal
                        // litlen .. '11111' .. '0'(type)
                        // output literal len, at most 17 bits + 1bit
                        int code = ( (lit - 1) << 6) | 0x3e;

                        RESERVE_16_BITS();
                        //if (bitstobeoutput >= 8) {
                        if (bitstobeoutput > (32 - 18) ) {
                            *op++ = (unsigned char) (bits_buffer32 & 0xff);
                            bits_buffer32 = bits_buffer32 >> 8;
                            bitstobeoutput -= 8;
                        }
                        STORE_BITS(17 + 1, code);
                        LZ4K_DEBUG_update_literal_len_bit_count(17, lit, code);
                    }

                    // output literals
                    // the bottleneck 2
                    while (1) {
                        // need at least 9 bits output space
                        while (bitstobeoutput < 24) {
                            int value, bits, code;
                            value = lz4k_literalch_encode[*ii++];
                            bits = value >> 9;
                            code = value & 0x1ff;
                            STORE_BITS(bits, code);
                            LZ4K_DEBUG_update_literal_ch_bit_count(bits, *(ii-1), code);
                            if (__builtin_expect(!!(ii == ip), 0) ) {
                                LZ4K_DEBUG_encode_literal_2_time_end();
                                goto break_literal_1;
                            }
                        }
                        // output buffer
                        // at least 24 bits to output
                        *( (unsigned int *)op) = bits_buffer32;
                        op += 3;
                        bits_buffer32 = bits_buffer32 >> 24;
                        bitstobeoutput -= 24;
                    }
                }
            }
            else {
                //RESERVE_16_BITS();
                // reserve at least 1 bit
                // output type: match
                if (bitstobeoutput == 32) {
                    *( (unsigned int *)op) = bits_buffer32;
                    op += 4;
                    bits_buffer32 = 1;
                    bitstobeoutput = 1;
                } else {
                    bits_buffer32 |= 1 << bitstobeoutput;
                    bitstobeoutput += 1;
                }
            }
        }

break_literal_1:

        LZ4K_DEBUG_encode_match_scan_time_start();
#ifdef LAZY_MATCH // INTERNAL START
        DO_LAZY_MATCH_2();
#else // LAZY_MATCH INTERNAL END
        // code the matches, already known there are at least 3 byte matches
        //assert(ii == ip);
        //ii = ip; // actually, ii is guaranteed to be equal to ip here
        // at least 3 match
        m_pos += 3;
        ip += 3;

        // the bottleneck 3
        // 40 % chance break on the first if
        if (__builtin_expect(!!(ip < in_end), 1) && *m_pos == *ip) {
            m_pos++, ip++;
            while (__builtin_expect(!!(ip < (in_end-1) ), 1) && *(unsigned short*)m_pos == *(unsigned short*)ip)
                m_pos += 2, ip += 2;
            if (__builtin_expect(!!(ip < in_end), 1) && *m_pos == *ip)
                m_pos += 1, ip += 1;
        }
#endif // LAZY_MATCH INTERNAL

        LZ4K_DEBUG_encode_match_scan_time_end();
        LZ4K_DEBUG_encode_match_off_time_start();
        // output match offset, at most 13 bits
        RESERVE_16_BITS();

        {
            size_t m_off = ip - m_pos;
            //if ( (m_off % 4) == 0 && m_off <= 128) {
#ifdef PREVIOUS_MATCH_OFF // INTERNAL START
            int index = ii - in;
#ifdef SAVING_OFFSET
            int bits_simple = 1 + (32 - __builtin_clz(index) );
#else
            const int bits_simple = 1 + 12;
#endif
            if (m_off == previous_m_off_1 && 3 < bits_simple) { // 000
                STORE_BITS(3, 0);
                LZ4K_DEBUG_update_match_off_bit_count(3, m_off, 0);
            } else {
                int i = 0;
                int value = 0;
                int bits = 0;
                for (i = 0; i < lz4k_matchoff_encode_count; ++i) {
                    if (m_off == lz4k_matchoff_encode_symbol[i]) {
                        value = lz4k_matchoff_encode[i];
                        break;
                    }
                }
                bits = value >> 8;
#ifdef SAVING_OFFSET
                if (value != 0 && bits < bits_simple) {
#else
                if (value != 0) {
#endif
                    int code = value & 0xff;
                    STORE_BITS(bits, code);
                    LZ4K_DEBUG_update_match_off_bit_count(bits, m_off, code);
                } else {
                    // m_off .. '1'
                    int code = (m_off << 1) | 0x1;
#ifdef SAVING_OFFSET
                    // m_off <= index
                    STORE_BITS(bits_simple, code);
                    LZ4K_DEBUG_update_match_off_bit_count(bits_simple, m_off, code);
#else // SAVING_OFFSET
                    STORE_BITS(13, code);
                    LZ4K_DEBUG_update_match_off_bit_count(13, m_off, code);
#endif // SAVING_OFFSET
                }
            }
            previous_m_off_1 = m_off;
#else // PREVIOUS_MATCH_OFF INTERNAL END
            if ( (m_off & 3) == 0 && m_off <= 128) {
                int value = lz4k_matchoff_encode[ (m_off / 4) - 1];
                int bits = value >> 8;
                int code = value & 0xff;
                STORE_BITS(bits, code);
                LZ4K_DEBUG_update_match_off_bit_count(bits, m_off, code);
            } else {
                // m_off .. '1'
                int code = (m_off << 1) | 0x1;
#ifdef SAVING_OFFSET // INTERNAL START
                int index = ii - in;
                STORE_BITS(1 + (32 - __builtin_clz(index) ), code);
#else // INTERNAL END
                STORE_BITS(13, code);
#endif // INTERNAL
                LZ4K_DEBUG_update_match_off_bit_count(13, m_off, code);
            }
#endif // PREVIOUS_MATCH_OFF INTERNAL
        }
        LZ4K_DEBUG_encode_match_off_time_end();
        LZ4K_DEBUG_encode_match_len_time_start();
        // output match len, at most 16 bits
        RESERVE_16_BITS();

        {
            size_t m_len = ip - ii;
#ifndef ALLOW_MATCH_2 // INTERNAL
            if (m_len < 32) {
                //int value = lz4k_matchlen_encode[m_len - 3];
                int value = lz4k_matchlen_encode[m_len];
                int bits = value >> 9;
                int code = value & 0x1ff;
                STORE_BITS(bits, code);
                LZ4K_DEBUG_update_match_len_bit_count(bits, m_len, code);
            } else {
                // m_len .. '1111'
                int code = (m_len << 4) | 0xf;
                STORE_BITS(16, code);
                LZ4K_DEBUG_update_match_len_bit_count(16, m_len, code);
            }
#else // ALLOW_MATCH_2 INTERNAL START
            if (bitstobeoutput > (32 - 17) ) {
                *op++ = (unsigned char) (bits_buffer32 & 0xff);
                bits_buffer32 = bits_buffer32 >> 8;
                bitstobeoutput -= 8;
            }
            if (m_len <= 20 || m_len == 24 || m_len == 28) {
                int value = lz4k_matchlen_encode[m_len];
                int bits = value >> 8;
                int code = value & 0xff;
                STORE_BITS(bits, code);
                LZ4K_DEBUG_update_match_len_bit_count(bits, m_len, code);
            } else {
                // m_len .. '11111'
                int code = (m_len << 5) | 0x1f;
                STORE_BITS(17, code);
                LZ4K_DEBUG_update_match_len_bit_count(17, m_len, code);
            }
#endif // ALLOW_MATCH_2 INTERNAL END
        }
        LZ4K_DEBUG_dprint_verbose("input: %ld\n", ip - in);

        ii = ip;
        if (__builtin_expect(!!(ip >= ip_end), 0) )
            break;
        LZ4K_DEBUG_encode_match_len_time_end();
    }
    LZ4K_DEBUG_encode_remain_time_start();

    // store final literal run
    if ( (in_end - ii) > 0)
    {
        size_t t = in_end - ii;
        LZ4K_DEBUG_update_lz_type_bit_count();
        if (t == 1) {
            int value, bits, code;
            // reserve at least 16 bits output buffer
            RESERVE_16_BITS();
            // output at most 9 + 1 + 1 = 11 bits
            value = lz4k_literalch_encode[*ii++];
            bits = value >> 9;
            code = (value & 0x1ff) << 2;
            // litlen and type: '0' .. '0'
            bits_buffer32 |= code << bitstobeoutput;
            bitstobeoutput += bits + 2;
            LZ4K_DEBUG_update_literal_len_bit_count(1, 1, 0);
            LZ4K_DEBUG_update_literal_ch_bit_count(bits, *(ii-1), code);
        } else {
            // reserve at least 24 bits output buffer
            while (bitstobeoutput >= 8) {
                *op++ = (unsigned char) (bits_buffer32 & 0xff);
                bits_buffer32 = bits_buffer32 >> 8;
                bitstobeoutput -= 8;
            }
            // output type literal
            //bits_buffer32 |= 1 << bitstobeoutput;
            bitstobeoutput += 1;

            // output literal len
            if (t <= 17) {
                int value = lz4k_literallen_encode[t];
                int bits = value >> 7;
                int code = value & 0x7f;
                bits_buffer32 |= code << bitstobeoutput;
                bitstobeoutput += bits;
                LZ4K_DEBUG_update_literal_len_bit_count(bits, t, code);
            } else {
                // litlen .. '11111'
                int code = ( (t - 1) << 5) | 0x1f;
                bits_buffer32 |= code << bitstobeoutput;
                bitstobeoutput += 17;
                LZ4K_DEBUG_update_literal_len_bit_count(17, t, code);
            }

            // output literals
            //RESERVE_16_BITS();
            while (1) {
                // need at least 9 bits output space
                while (bitstobeoutput < 24) {
                    int value, bits, code;
                    value = lz4k_literalch_encode[*ii++];
                    bits = value >> 9;
                    code = value & 0x1ff;
                    bits_buffer32 |= code << bitstobeoutput;
                    bitstobeoutput += bits;
                    LZ4K_DEBUG_update_literal_ch_bit_count(bits, *(ii-1), code);
                    if (__builtin_expect(!!(--t == 0), 0) )
                        goto break_literal_2;
                }
                // output buffer
                *( (unsigned int *)op) = bits_buffer32;
                op += 3;
                bits_buffer32 = bits_buffer32 >> 24;
                bitstobeoutput -= 24;
            }
        }
    }

break_literal_2:
    // output remaining bits
    while (bitstobeoutput >= 8) {
        *op++ = (unsigned char) (bits_buffer32 & 0xff);
        bits_buffer32 = bits_buffer32 >> 8;
        bitstobeoutput -= 8;
    }
    if (bitstobeoutput != 0) {
        *op++ = (unsigned char) (bits_buffer32 & 0xff);
        LZ4K_DEBUG_update_padding_bit_count(8 - bitstobeoutput);
    }

    // 4 bytes padding
    *( (unsigned int *)op) = bits_buffer32;
    op += 4;

    *out_len = op - out;

    LZ4K_DEBUG_encode_remain_time_end();
    // OK
    return 0;
}

#ifdef DO_LZ4K_ENCODE_VERIFY // INTERNAL START
int lz4k_decompress_ubifs(const unsigned char *in, size_t in_len, unsigned char *out, size_t *pout_len);

static void verify_fail_print(const unsigned char *in, size_t in_len)
{
    printf("[LZ4K] error_print, in_len: %ld\n", in_len);
    char to_print[3*8*4+10];
    char *pp = to_print;
    int i;
    for (i = 0; i < in_len; ++i) {
        pp += sprintf(pp, "%02x ", in[i]);
        if (i % 32 == 31 || i == in_len - 1) {
            pp[0] = '\n';
            pp[1] = 0;
            printf("[LZ4K] %s", to_print);
            pp = to_print;
        }
    }
}
#endif // DO_LZ4K_ENCODE_VERIFY, INTERNAL END

#ifdef PC_TEST // INTERNAL START
int my_encode_count = 0;
#endif // INTERNAL END
int lz4k_compress(const unsigned char *in, size_t in_len, unsigned char *out,
        size_t *out_len, void *wrkmem)
{
    unsigned char *op = out;
    LZ4K_DEBUG_encode_start();

    if (in_len > 4096) {
        LZ4K_DEBUG_print("wrong input len: %d\n", (int)in_len);
        LZ4K_DEBUG_assert(0);
        return -1;
    }

    if (__builtin_expect(!!(in_len == 0), 0) ) {
        *out_len = 0;
        // ERROR
        return -1;
    } else {
        _lz4k_do_compress(in, in_len, op, out_len, wrkmem);
    }

#ifdef PC_TEST // INTERNAL START
    LZ4K_DEBUG_dprint("output len: %d, encode_count: %d\n", (int)*out_len, my_encode_count);
#endif // INTERNAL END
    if (*out_len <= 0) {
        LZ4K_DEBUG_print("wrong output len: %d\n", (int)*out_len);
        LZ4K_DEBUG_assert(0);
        // ERROR
        return -1;
    }
#ifdef PC_TEST // INTERNAL START
    ++my_encode_count;
    //printf("len\t%ld\n", *out_len);
    if ( (my_encode_count % 1000) == 0)
    {
        printf("my_encode_count: %d\n", my_encode_count);
    }
    //assert(0);
#endif // INTERNAL END

#ifdef DO_LZ4K_ENCODE_VERIFY // INTERNAL START
    {
        unsigned char decompressed[4096];
        size_t decomp_len = in_len;
        int result;
        int i;
        result = lz4k_decompress_ubifs(out, *out_len, decompressed, &decomp_len);
        if (result != 0)
            return -1;
        for (i = 0; i < in_len; ++i) {
            if (in[i] != decompressed[i]) {
                printf("decompressed content not match in: %d!\n", i);
                printf("original content:\n");
                verify_fail_print(in, in_len);
                printf("decompressed content:\n");
                verify_fail_print(decompressed, decomp_len);
                return -1;
            }
        }
    }
#endif // DO_LZ4K_ENCODE_VERIFY, INTERNAL END

    LZ4K_DEBUG_encode_end();
    // OK
    return 0;
}

