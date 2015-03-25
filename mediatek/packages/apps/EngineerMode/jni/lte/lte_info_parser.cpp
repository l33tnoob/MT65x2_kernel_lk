#include <jni.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <cutils/xlog.h>
#include <cutils/properties.h>
#include "lte_info_parser.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include "utils/Errors.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "EMLTE"
#define MAX_LENGTH 20000

using namespace android;

static char hex_to_char(unsigned int hex) {
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    }
    if (hex >= 'A' && hex <= 'F') {
        return 0xA + hex - 'A';
    }
    if (hex >= 'a' && hex <= 'f') {
        return 0xA + hex - 'a';
    }
    return 0;
}

static void hex_to_char(jchar *input, int input_len, char *buf, int buf_size) {
    memset(buf, 0, buf_size);
    if (input_len < buf_size * 2) { // should not happen
        buf_size = input_len / 2;
    }
    for (int i = 0; i < buf_size; i++) {
        buf[i] = (hex_to_char(input[i * 2]) << 4) + hex_to_char(input[i * 2 + 1]);
    }
}

static jintArray get_types(JNIEnv* env, jobject thiz, jint page) {
    XLOGD("get_types %d\n", page);
    switch (page) {
    case 0:
    {
        const jint types[] = {
                ERRC_EM_ERRC_STATE_IND, ERRC_EM_REEST_INFO,
                ERRC_EM_RCM_SIM_STS_INFO, ERRC_EM_SEC_PARAM,
                ERRC_EM_CARRS_EVENT_IND, ERRC_EM_MOB_EVENT_IND,
                ERRC_EM_RECONF_INFO, ERRC_EM_OVER_PROC_DELAY_WARNING};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    case 1:
    {
        const jint types[] = {ERRC_EM_LTE_SUPPORTED_BAND_INFO};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    case 2:
    {
        const jint types[] = {ERRC_EM_MOB_MEAS_INTRARAT_INFO};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    case 3:
    {
        const jint types[] = {ERRC_EM_MOB_MEAS_INTERRAT_UTRAN_INFO};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    case 4:
    {
        const jint types[] = {ERRC_EM_MOB_MEAS_INTERRAT_GERAN_INFO};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    case 5:
    {
        const jint types[] = {ERRC_EM_SYS_SIB_RX_STS_INFO};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    case 6:
    {
        const jint types[] = {ERRC_EM_AUTOS_CSG_INFO};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    case 7:
    {
        const jint types[] = {ESM_L4C_ESM_INFO};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    case 8:
    {
        const jint types[] = {EMM_L4C_EMM_INFO};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    case 9:
    {
        const jint types[] = {
                RR_EM_MEASUREMENT_REPORT_INFO, RR_EM_LAI_INFO,
                EM_CSCE_SERV_CELL_IND_STRUCT_INFO, EM_MEME_INFO_DCH_UMTS_CELL_INFO,
                ERRC_EM_MOB_MEAS_INTRARAT_INFO, EL1TX_EM_TX_INFO,
                ERRC_EM_LTE_SUPPORTED_BAND_INFO};
        int length = sizeof(types)/sizeof(jint);
        jintArray array = env->NewIntArray(length);
        env->SetIntArrayRegion(array, 0, length, (const jint *)types);
        return array;
    }
    default:
        break;
    }
    return NULL;
}

static jint get_size(JNIEnv* env, jobject thiz, jint type) {
    XLOGD("get_size %d\n", type);
    switch (type) {
    // ERRC
    case ERRC_EM_ERRC_STATE_IND:
        return sizeof(em_errc_state_ind_struct);
    case ERRC_EM_REEST_INFO:
        return sizeof(em_errc_reest_info_ind_struct);
    case ERRC_EM_RCM_SIM_STS_INFO:
        return sizeof(em_errc_rcm_sim_sts_info_ind_struct);
    case ERRC_EM_SEC_PARAM:
        return sizeof(em_errc_sec_param_ind_struct);
    case ERRC_EM_CARRS_EVENT_IND:
        return sizeof(em_errc_carrs_event_ind_struct);
    case ERRC_EM_MOB_EVENT_IND:
        return sizeof(em_errc_mob_event_ind_struct);
    case ERRC_EM_RECONF_INFO:
        return sizeof(em_errc_reconf_info_ind_struct);
    case ERRC_EM_OVER_PROC_DELAY_WARNING:
        return sizeof(em_errc_over_proc_delay_warning_ind_struct);
    case ERRC_EM_LTE_SUPPORTED_BAND_INFO:
        return sizeof(em_lte_supported_band_info_ind_struct);
    case ERRC_EM_MOB_MEAS_INTRARAT_INFO:
        return sizeof(em_errc_mob_meas_intrarat_info_ind_struct);
    case ERRC_EM_MOB_MEAS_INTERRAT_UTRAN_INFO:
        return sizeof(em_errc_mob_meas_interrat_utran_info_ind_struct);
    case ERRC_EM_MOB_MEAS_INTERRAT_GERAN_INFO:
        return sizeof(em_errc_mob_meas_interrat_geran_info_ind_struct);
    case ERRC_EM_SYS_SIB_RX_STS_INFO:
        return sizeof(em_errc_sys_sib_rx_sts_info_ind_struct);
    case ERRC_EM_AUTOS_CSG_INFO:
        return sizeof(em_errc_autos_csg_info_ind_struct);
    // ESM
    case ESM_L4C_ESM_INFO:
        return sizeof(em_esm_l4c_esm_info_ind_struct);
    // EMM
    case EMM_L4C_EMM_INFO:
        return sizeof(em_emm_l4c_emm_info_ind_struct);
    // MMDC 2/3G
    case RR_EM_MEASUREMENT_REPORT_INFO:
        return sizeof(em_rrm_measurement_report_info_ind_struct);
    case RR_EM_LAI_INFO:
        return sizeof(rr_em_lai_info_struct);
    case EM_CSCE_SERV_CELL_IND_STRUCT_INFO:
        return sizeof(em_csce_serv_cell_s_status_ind_struct);
    case EM_MEME_INFO_DCH_UMTS_CELL_INFO:
        return sizeof(meme_em_info_umts_cell_status_struct);
    // MMDC 4G
    case EL1TX_EM_TX_INFO:
        return sizeof(em_el1tx_status_ind_struct);
    }
    return -1;
}

static void do_parse(int type, jchar *input, int input_len, char *output) {
    XLOGD("parse %d %d\n", type, input_len);
    switch (type) {
    case ERRC_EM_ERRC_STATE_IND:
    {
        em_errc_state_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_state_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[ERRC State]\n"
            " ERRC State: %d\n"
            "--------------------\n",
            buf.errc_sts);
        break;
    }
    case ERRC_EM_REEST_INFO:
    {
        em_errc_reest_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_reest_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[Re-establishment Info]\n"
            " Re-establishment Cause: %d\n"
            "--------------------\n",
            buf.cause);
        break;
    }
    case ERRC_EM_RCM_SIM_STS_INFO:
    {
        em_errc_rcm_sim_sts_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_rcm_sim_sts_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[SIM Status]\n"
            " SIM Status: %d\n"
            "--------------------\n",
            buf.sim_sts);
        break;
    }
    case ERRC_EM_SEC_PARAM:
    {
        em_errc_sec_param_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_sec_param_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[Security Status]\n"
            " key_chg_ind: %d\n"
            " ncc: %d\n"
            " int_algo: %d\n"
            " enc_algo: %d\n"
            "--------------------\n",
            buf.key_chg_ind,
            buf.ncc,
            buf.int_algo,
            buf.enc_algo);
        break;
    }
    case ERRC_EM_CARRS_EVENT_IND:
    {
        em_errc_carrs_event_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_carrs_event_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[Carrier Search Event]\n"
            " trigger_type: %d\n"
            " carrs_type: %d\n"
            "--------------------\n",
            buf.carrs_evt.trigger_type,
            buf.carrs_evt.carrs_type);
        break;
    }
    case ERRC_EM_MOB_EVENT_IND:
    {
        em_errc_mob_event_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_mob_event_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[Mobility Event]\n"
            " mobility_type: %d\n"
            " mobility_direction: %d\n"
            "--------------------\n",
            buf.mob_evt.mob_type,
            buf.mob_evt.mob_dir);
        break;
    }
    case ERRC_EM_RECONF_INFO:
    {
        em_errc_reconf_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_reconf_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[Connection Re-config Info]\n"
            " reconf_type: %d\n"
            " is_cell_info_valid: %d\n"
            " earfcn: %lu\n"
            " pci: %u\n"
            " C-RNTI: %u\n"
            " t311: %lu\n"
            " t301: %lu\n"
            " t304: %lu\n"
            "--------------------\n",
            buf.reconf_type,
            buf.is_cell_info_valid,
            buf.earfcn,
            buf.pci,
            buf.crnti,
            buf.t311,
            buf.t301,
            buf.t304);
        break;
    }
    case ERRC_EM_OVER_PROC_DELAY_WARNING:
    {
        em_errc_over_proc_delay_warning_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_over_proc_delay_warning_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[Procedure Delay Warning]\n"
            " Delay Procedure: %d\n"
            " Delay Time: %lu\n"
            "--------------------\n",
            buf.delay_proc_id,
            buf.delay_time);
        break;
    }
    case ERRC_EM_LTE_SUPPORTED_BAND_INFO:
    {
        em_lte_supported_band_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_lte_supported_band_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            " num_supported_band: %d\n",
            buf.num_supported_band);
        for (int i = 0; i < 64; i++) {
            snprintf(output + strlen(output), MAX_LENGTH, " supported_band[%d]: %d\n", i, buf.supported_band[i]);
        }
        break;
    }
    case ERRC_EM_MOB_MEAS_INTRARAT_INFO:
    {
        em_errc_mob_meas_intrarat_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_mob_meas_intrarat_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[Serving Info]\n"
            " earfcn: %u\n"
            " pci: %u\n"
            " rsrp: %ld\n"
            " rsrq: %ld\n"
            " mobility_state: %d\n"
            " S_intra_search_p: %u\n"
            " S_intra_search_q: %u\n"
            " S_nonintra_search_p: %u\n"
            " S_nonintra_search_q: %u\n"
            " thresh_serving_low_p: %u\n"
            " thresh_serving_low_q: %u\n"
            " tresel: %u\n"
            "--------------------\n"
            "[Intra-Freq Info]\n"
            " Resel Priority: %d\n"
            " Meas Bandwidth: %d\n"
            " Black List Present: %d\n"
            " P-comp: %u\n"
            " Intra-Freq Nbr Cell Num: %u\n",
            buf.serving_info.earfcn,
            buf.serving_info.pci,
            buf.serving_info.rsrp,
            buf.serving_info.rsrq,
            buf.serving_info.mobility_state,
            buf.serving_info.S_intra_search_p,
            buf.serving_info.S_intra_search_q,
            buf.serving_info.S_nonintra_search_p,
            buf.serving_info.S_nonintra_search_q,
            buf.serving_info.thresh_serving_low_p,
            buf.serving_info.thresh_serving_low_q,
            buf.serving_info.tresel,
            buf.intra_info.priority,
            buf.intra_info.bandwidth,
            buf.intra_info.is_blacklist_present,
            buf.intra_info.pcomp,
            buf.intra_info.cell_num);
    
        for (int i = 0; i < 3; i++) {
            snprintf(output + strlen(output), MAX_LENGTH,
                " Intra-Freq Nbr Cell[%d] pci: %u\n"
                " Intra-Freq Nbr Cell[%d] rsrp: %ld\n"
                " Intra-Freq Nbr Cell[%d] rsrq: %ld\n",
                i, buf.intra_info.intra_cell[i].pci,
                i, buf.intra_info.intra_cell[i].rsrp,
                i, buf.intra_info.intra_cell[i].rsrq);
        }
    
        snprintf(output + strlen(output), MAX_LENGTH,
            "--------------------\n"
            "[Inter-Freq Info]\n"
            " Inter-Freq Num: %u\n",
            buf.inter_info.freq_num);

        for (int i = 0; i < 3; i++) {
            snprintf(output + strlen(output), MAX_LENGTH,
                " Inter-Freq[%d] valid: %d\n"
                " Inter-Freq[%d] earfcn: %u\n"
                " Inter-Freq[%d] pcomp: %u\n"
                " Inter-Freq[%d] priority: %d\n"
                " Inter-Freq[%d] bandwidth: %d\n"
                " Inter-Freq[%d] is_blacklist_present: %d\n"
                " Inter-Freq[%d] treselection: %u\n"
                " Inter-Freq[%d] thresh_x_high_p: %u\n"
                " Inter-Freq[%d] thresh_x_high_q: %u\n"
                " Inter-Freq[%d] thresh_x_low_p: %u\n"
                " Inter-Freq[%d] thresh_x_low_q: %u\n"
                " Inter-Freq[%d] Nbr Cell Num: %u\n"
                " Inter-Freq[%d] Nbr Cell[0] pci: %u\n"
                " Inter-Freq[%d] Nbr Cell[0] rsrp: %ld\n"
                " Inter-Freq[%d] Nbr Cell[0] rsrq: %ld\n"
                " Inter-Freq[%d] Nbr Cell[1] pci: %u\n"
                " Inter-Freq[%d] Nbr Cell[1] rsrp: %ld\n"
                " Inter-Freq[%d] Nbr Cell[1] rsrq: %ld\n"
                " Inter-Freq[%d] Nbr Cell[2] pci: %u\n"
                " Inter-Freq[%d] Nbr Cell[2] rsrp: %ld\n"
                " Inter-Freq[%d] Nbr Cell[2] rsrq: %ld\n\n",
                i, buf.inter_info.inter_freq[i].valid,
                i, buf.inter_info.inter_freq[i].earfcn,
                i, buf.inter_info.inter_freq[i].pcomp,
                i, buf.inter_info.inter_freq[i].priority,
                i, buf.inter_info.inter_freq[i].bandwidth,
                i, buf.inter_info.inter_freq[i].is_blacklist_present,
                i, buf.inter_info.inter_freq[i].treselection,
                i, buf.inter_info.inter_freq[i].thresh_x_high_p,
                i, buf.inter_info.inter_freq[i].thresh_x_high_q,
                i, buf.inter_info.inter_freq[i].thresh_x_low_p,
                i, buf.inter_info.inter_freq[i].thresh_x_low_q,
                i, buf.inter_info.inter_freq[i].cell_num,
                i, buf.inter_info.inter_freq[i].inter_cell[0].pci,
                i, buf.inter_info.inter_freq[i].inter_cell[0].rsrp,
                i, buf.inter_info.inter_freq[i].inter_cell[0].rsrq,
                i, buf.inter_info.inter_freq[i].inter_cell[1].pci,
                i, buf.inter_info.inter_freq[i].inter_cell[1].rsrp,
                i, buf.inter_info.inter_freq[i].inter_cell[1].rsrq,
                i, buf.inter_info.inter_freq[i].inter_cell[2].pci,
                i, buf.inter_info.inter_freq[i].inter_cell[2].rsrp,
                i, buf.inter_info.inter_freq[i].inter_cell[2].rsrq);
        }
        break;
    }
    case ERRC_EM_MOB_MEAS_INTERRAT_UTRAN_INFO:
    {
        em_errc_mob_meas_interrat_utran_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_mob_meas_interrat_utran_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "freq_num: %u\n",
            buf.freq_num);
    
        for (int i = 0; i < 3; i++) {
            snprintf(output + strlen(output), MAX_LENGTH,
                "UMTS Freq[%d] uarfcn: %u\n"
                "UMTS Freq[%d] priority: %d\n"
                "UMTS Freq[%d] threshx_high_p: %u\n"
                "UMTS Freq[%d] threshx_high_q: %u\n"
                "UMTS Freq[%d] threshx_low_p: %u\n"
                "UMTS Freq[%d] threshx_low_q: %u\n"
                "UMTS Freq[%d] ucell_num: %u\n"
                "UMTS Freq[%d] Nbr Cell[0] psc: %u\n"
                "UMTS Freq[%d] Nbr Cell[0] rscp: %ld\n"
                "UMTS Freq[%d] Nbr Cell[0] ec_n0: %ld\n"
                "UMTS Freq[%d] Nbr Cell[1] psc: %u\n"
                "UMTS Freq[%d] Nbr Cell[1] rscp: %ld\n"
                "UMTS Freq[%d] Nbr Cell[1] ec_n0: %ld\n"
                "UMTS Freq[%d] Nbr Cell[2] psc: %u\n"
                "UMTS Freq[%d] Nbr Cell[2] rscp: %ld\n"
                "UMTS Freq[%d] Nbr Cell[2] ec_n0: %ld\n\n",
                i, buf.inter_freq[0].uarfcn,
                i, buf.inter_freq[0].priority,
                i, buf.inter_freq[0].threshx_high_p,
                i, buf.inter_freq[0].threshx_high_q,
                i, buf.inter_freq[0].threshx_low_p,
                i, buf.inter_freq[0].threshx_low_q,
                i, buf.inter_freq[0].ucell_num,
                i, buf.inter_freq[0].ucell[0].psc,
                i, buf.inter_freq[0].ucell[0].rscp,
                i, buf.inter_freq[0].ucell[0].ec_n0,
                i, buf.inter_freq[0].ucell[1].psc,
                i, buf.inter_freq[0].ucell[1].rscp,
                i, buf.inter_freq[0].ucell[1].ec_n0,
                i, buf.inter_freq[0].ucell[2].psc,
                i, buf.inter_freq[0].ucell[2].rscp,
                i, buf.inter_freq[0].ucell[2].ec_n0);
        }
        break;
    }
    case ERRC_EM_MOB_MEAS_INTERRAT_GERAN_INFO:
    {
        em_errc_mob_meas_interrat_geran_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_mob_meas_interrat_geran_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "gcell_num: %u\n",
            buf.gcell_num);
    
        for (int i = 0; i < 6; i++) {
            snprintf(output + strlen(output), MAX_LENGTH,
                "GSM Nbr Cell[%d] priority: %d\n"
                "GSM Nbr Cell[%d] band_ind: %d\n"
                "GSM Nbr Cell[%d] arfcn: %u\n"
                "GSM Nbr Cell[%d] bsic: %u\n"
                "GSM Nbr Cell[%d] rssi: %ld\n"
                "GSM Nbr Cell[%d] thresh_x_high: %u\n"
                "GSM Nbr Cell[%d] thresh_x_low: %u\n\n",
                i, buf.gcell[i].priority,
                i, buf.gcell[i].band_ind,
                i, buf.gcell[i].arfcn,
                i, buf.gcell[i].bsic,
                i, buf.gcell[i].rssi,
                i, buf.gcell[i].thresh_x_high,
                i, buf.gcell[i].thresh_x_low);
        }
        break;
    }
    case ERRC_EM_SYS_SIB_RX_STS_INFO:
    {
        em_errc_sys_sib_rx_sts_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_sys_sib_rx_sts_info_ind_struct));
        for (int i = 0; i < 3; i++) {
            snprintf(output + strlen(output), MAX_LENGTH, i == 0 ? "[Serving]\n" : "[Neighbor %d]:\n", i);
            snprintf(output + strlen(output), MAX_LENGTH,
                " is_rxing: %d\n"
                " earfcn: %lu\n"
                " pci: %u\n"
                " mib_rx_sts: %d\n"
                " sib1_rx_sts: %d\n"
                " sib2_rx_sts: %d\n"
                " sib3_rx_sts: %d\n"
                " sib4_rx_sts: %d\n"
                " sib5_rx_sts: %d\n"
                " sib6_rx_sts: %d\n"
                " sib7_rx_sts: %d\n"
                " sib9_rx_sts: %d\n"
                " sib10_rx_sts: %d\n"
                " sib11_rx_sts: %d\n"
                " sib12_rx_sts: %d\n"
                "--------------------\n",
                buf.sib_rx_info[i].is_rxing,
                buf.sib_rx_info[i].earfcn,
                buf.sib_rx_info[i].pci,
                buf.sib_rx_info[i].mib_rx_sts,
                buf.sib_rx_info[i].sib1_rx_sts,
                buf.sib_rx_info[i].sib2_rx_sts,
                buf.sib_rx_info[i].sib3_rx_sts,
                buf.sib_rx_info[i].sib4_rx_sts,
                buf.sib_rx_info[i].sib5_rx_sts,
                buf.sib_rx_info[i].sib6_rx_sts,
                buf.sib_rx_info[i].sib7_rx_sts,
                buf.sib_rx_info[i].sib9_rx_sts,
                buf.sib_rx_info[i].sib10_rx_sts,
                buf.sib_rx_info[i].sib11_rx_sts,
                buf.sib_rx_info[i].sib12_rx_sts);
        }
        break;
    }
    case ERRC_EM_AUTOS_CSG_INFO:
    {
        em_errc_autos_csg_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_errc_autos_csg_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "num_cells: %u\n",
            buf.autos_info.num_cells);
    
        for (int i = 0; i < 3; i++) {
            snprintf(output + strlen(output), MAX_LENGTH,
                "detected_csg_cell[%d] earfcn: %u\n"
                "detected_csg_cell[%d] pci: %u\n"
                "detected_csg_cell[%d] csg_id: %lu\n\n",
                i, buf.autos_info.detected_csg_cell[i].earfcn,
                i, buf.autos_info.detected_csg_cell[i].pci,
                i, buf.autos_info.detected_csg_cell[i].csg_id);
        }
        break;
    }
    // MMDC 2/3G
    case RR_EM_MEASUREMENT_REPORT_INFO:
    {
        em_rrm_measurement_report_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_rrm_measurement_report_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[2G]\n"
            " Serving frequency: %u\n"
            " Serving physical cell ID: %u\n"
            " RSSI: %d\n"
            " RX_QUAL: %u\n"
            " Tx power: %d\n",
            buf.rr_em_measurement_report_info.serving_arfcn,
            buf.rr_em_measurement_report_info.serving_bsic,
            buf.rr_em_measurement_report_info.serv_rla_in_quarter_dbm,
            buf.rr_em_measurement_report_info.rxqual_sub,
            buf.rr_em_measurement_report_info.tx_power_level);
        break;
    }
    case RR_EM_LAI_INFO:
    {
        rr_em_lai_info_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(rr_em_lai_info_struct));
        snprintf(output, MAX_LENGTH,
            " support band: %u\n"
            "--------------------\n",
            buf.supported_Band);
        break;
    }
    case EM_CSCE_SERV_CELL_IND_STRUCT_INFO:
    {
        em_csce_serv_cell_s_status_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_csce_serv_cell_s_status_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[3G (non-DCH)]\n"
            " Serving frequency: %u\n"
            " Serving physical cell ID: %u\n"
            " RSCP: %ld\n"
            " SINR: %ld\n"
            "--------------------\n",
            buf.serv_cell.uarfcn_DL,
            buf.serv_cell.psc,
            buf.serv_cell.rscp,
            buf.serv_cell.squal);
        break;
    }
    case EM_MEME_INFO_DCH_UMTS_CELL_INFO:
    {
        meme_em_info_umts_cell_status_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(meme_em_info_umts_cell_status_struct));
        snprintf(output, MAX_LENGTH,
            "[3G (DCH)]\n"
            " SINR: %u\n"
            " support band: %d\n\n",
            buf.sinr,
            buf.supportBand);

        for (int i = 0; i < 64; i++) {
            snprintf(output + strlen(output), MAX_LENGTH,
                " umts_cell_list[%d] Serving frequency: %u\n"
                " umts_cell_list[%d] Serving physical cell ID: %u\n"
                " umts_cell_list[%d] RSCP: %ld\n\n",
                i, buf.umts_cell_list[i].UARFCN,
                i, buf.umts_cell_list[i].CELLPARAID,
                i, buf.umts_cell_list[i].RSCP);
        }
        snprintf(output + strlen(output), MAX_LENGTH, "--------------------\n");
        break;
    }
    // MMDC 4G
    case EL1TX_EM_TX_INFO:
    {
        em_el1tx_status_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_el1tx_status_ind_struct));
        snprintf(output, MAX_LENGTH,
            " Tx power: %d\n",
            buf.tx_power_ave);
        break;
    }
    // ESM
    case ESM_L4C_ESM_INFO:
    {
        em_esm_l4c_esm_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_esm_l4c_esm_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[Status]\n"
            " em_esm_sys_state: %d\n"
            " em_esm_rat_state: %d\n"
            " em_esm_active_pt_num: %u\n"
            " em_esm_active_epsb_num: %u\n"
            " em_esm_active_drb_num: %u\n",
            buf.esm_status.em_esm_sys_state,
            buf.esm_status.em_esm_rat_state,
            buf.esm_status.em_esm_active_pt_num,
            buf.esm_status.em_esm_active_epsb_num,
            buf.esm_status.em_esm_active_drb_num);
        snprintf(output + strlen(output), MAX_LENGTH, "--------------------\n");

        snprintf(output + strlen(output), MAX_LENGTH, "[PT]\n");
        for (int i = 0; i < 10; i++) {
            snprintf(output + strlen(output), MAX_LENGTH,
                " PT[%d] is_active: %d\n"
                " PT[%d] pti: %u\n"
                " PT[%d] pt_req_reason: %d\n"
                " PT[%d] pt_state: %d\n"
                " PT[%d] cid: %u\n"
                " PT[%d] ebi: %u\n\n",
                i, buf.esm_pt[i].is_active,
                i, buf.esm_pt[i].pti,
                i, buf.esm_pt[i].pt_req_reason,
                i, buf.esm_pt[i].pt_state,
                i, buf.esm_pt[i].cid,
                i, buf.esm_pt[i].ebi);
        }
        snprintf(output + strlen(output), MAX_LENGTH, "--------------------\n");

        snprintf(output + strlen(output), MAX_LENGTH, "[EPSBC]\n");
        for (int i = 0; i < 11; i++) {
            snprintf(output + strlen(output), MAX_LENGTH,
                " EPSBC[%d] is_active: %d\n"
                " EPSBC[%d] ebi: %u\n"
                " EPSBC[%d] bearer_type: %d\n"
                " EPSBC[%d] is_emergency_bearer: %d\n"
                " EPSBC[%d] linked_ebi: %u\n"
                " EPSBC[%d] qci: %u\n"
                " EPSBC[%d] ip_addr:\n"
                "  ip_addr_type %u\n"
                "  ipv4 %u, %u, %u, %u\n"
                "  ipv6 %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u, %u\n"
                " EPSBC[%d] apn: ",
                i, buf.esm_epsbc[i].is_active,
                i, buf.esm_epsbc[i].ebi,
                i, buf.esm_epsbc[i].bearer_type,
                i, buf.esm_epsbc[i].is_emergency_bearer,
                i, buf.esm_epsbc[i].linked_ebi,
                i, buf.esm_epsbc[i].qci,
                i, buf.esm_epsbc[i].ip_addr.ip_addr_type,
                buf.esm_epsbc[i].ip_addr.ipv4[0],
                buf.esm_epsbc[i].ip_addr.ipv4[1],
                buf.esm_epsbc[i].ip_addr.ipv4[2],
                buf.esm_epsbc[i].ip_addr.ipv4[3],
                buf.esm_epsbc[i].ip_addr.ipv6[0],
                buf.esm_epsbc[i].ip_addr.ipv6[1],
                buf.esm_epsbc[i].ip_addr.ipv6[2],
                buf.esm_epsbc[i].ip_addr.ipv6[3],
                buf.esm_epsbc[i].ip_addr.ipv6[4],
                buf.esm_epsbc[i].ip_addr.ipv6[5],
                buf.esm_epsbc[i].ip_addr.ipv6[6],
                buf.esm_epsbc[i].ip_addr.ipv6[7],
                buf.esm_epsbc[i].ip_addr.ipv6[8],
                buf.esm_epsbc[i].ip_addr.ipv6[9],
                buf.esm_epsbc[i].ip_addr.ipv6[10],
                buf.esm_epsbc[i].ip_addr.ipv6[11],
                buf.esm_epsbc[i].ip_addr.ipv6[12],
                buf.esm_epsbc[i].ip_addr.ipv6[13],
                buf.esm_epsbc[i].ip_addr.ipv6[14],
                buf.esm_epsbc[i].ip_addr.ipv6[15],
                i);
            for (int j = 0; j < buf.esm_epsbc[i].apn.length; j++) {
                snprintf(output + strlen(output), MAX_LENGTH, "%c", buf.esm_epsbc[i].apn.data[j]);
            }
            snprintf(output + strlen(output), MAX_LENGTH, "\n\n");
        }
        break;
    }
    // EMM
    case EMM_L4C_EMM_INFO:
    {
        em_emm_l4c_emm_info_ind_struct buf;
        hex_to_char(input, input_len, (char *)&buf, sizeof(em_emm_l4c_emm_info_ind_struct));
        snprintf(output, MAX_LENGTH,
            "[Service Request / CSFB]\n"
            " Service Request Type: %d\n"
            " Service Request Cause: %d\n"
            " MO CSFB Cause: %d\n"
            " MT CSFB Paging Ind: %d\n",
            buf.emm_call_para.service_request_type,
            buf.emm_call_para.service_request_cause,
            buf.emm_call_para.mo_csfb_cause,
            buf.emm_call_para.mt_csfb_paging_id);
        snprintf(output + strlen(output), MAX_LENGTH, "--------------------\n");

        snprintf(output + strlen(output), MAX_LENGTH,
            "[Connection Status]\n"
            " ECM Status: %d\n",
            buf.emm_conn_para.ecm_status);
        snprintf(output + strlen(output), MAX_LENGTH, "--------------------\n");

        snprintf(output + strlen(output), MAX_LENGTH,
            "[Registeration Status]\n"
            " Attach Type: %d\n"
            " Attach Additional Update Type: %d\n"
            " Attach Result: %d\n"
            " Attach Additional Update Result: %d\n"
            " Attach EMM Cause: %d\n"
            " Attach Attempt Count: %lu\n"
            " Attach Status: %d\n"
            " TAU Type: %d\n"
            " TAU Additional Update Type: %d\n"
            " TAU Result: %d\n"
            " TAU Additional Update Result: %d\n"
            " TAU EMM Cause: %d\n"
            " TAU Attempt Count: %lu\n"
            " TAU Status: %d\n"
            " User Detach Type: %d\n"
            " Network Detach Type: %d\n"
            " Network Detach EMM Cause: %d\n"
            " Detach Attempt Count: %lu\n"
            " Support IMS Voice in S1 mode: %d\n"
            " Support Emergency Bearer in S1 mode: %d\n",
            buf.emm_reg_para.eps_attach_type,
            buf.emm_reg_para.attach_additional_update_type,
            buf.emm_reg_para.eps_attach_result,
            buf.emm_reg_para.attach_additional_update_result,
            buf.emm_reg_para.attach_emm_cause,
            buf.emm_reg_para.attach_attempt_count,
            buf.emm_reg_para.attach_status,
            buf.emm_reg_para.tau_req_update_type,
            buf.emm_reg_para.tau_additional_update_type,
            buf.emm_reg_para.tau_update_result,
            buf.emm_reg_para.tau_additional_update_result,
            buf.emm_reg_para.tau_emm_cause,
            buf.emm_reg_para.tau_attempt_count,
            buf.emm_reg_para.tau_status,
            buf.emm_reg_para.user_detach_type,
            buf.emm_reg_para.nw_detach_type,
            buf.emm_reg_para.nw_detach_emm_cause,
            buf.emm_reg_para.detach_attempt_count,
            buf.emm_reg_para.ims_service_ind,
            buf.emm_reg_para.emergency_service_ind);
        snprintf(output + strlen(output), MAX_LENGTH, "--------------------\n");

        snprintf(output + strlen(output), MAX_LENGTH,
            "[Network Information]\n"
            " Selected PLMN: %u, %u, %u, %u, %u, %u\n"
            " Tracking Area Code: %u\n"
            " CSG Access Mode: %d\n"
            " CSG ID: %lu\n"
            " Duplex Type: %d\n",
            buf.emm_plmnsel_para.selected_plmn.mcc1,
            buf.emm_plmnsel_para.selected_plmn.mcc2,
            buf.emm_plmnsel_para.selected_plmn.mcc3,
            buf.emm_plmnsel_para.selected_plmn.mnc1,
            buf.emm_plmnsel_para.selected_plmn.mnc2,
            buf.emm_plmnsel_para.selected_plmn.mnc3,
            buf.emm_plmnsel_para.tac,
            buf.emm_plmnsel_para.csg_access_mode,
            buf.emm_plmnsel_para.csg_id,
            buf.emm_plmnsel_para.duplex_type);
        snprintf(output + strlen(output), MAX_LENGTH, "--------------------\n");

        snprintf(output + strlen(output), MAX_LENGTH,
            "[UE Information]\n"
            " EMM Update Status: %d\n"
            " CS SIM Status: %d\n"
            " PS SIM Status: %d\n"
            " GUTI - PLMNID: %u, %u, %u, %u, %u, %u\n"
            " GUTI - MME_GID[0]: %u\n"
            " GUTI - MME_GID[1]: %u\n"
            " GUTI - MME_CODE: %u\n"
            " GUTI - MTMSI[0]: %u\n"
            " GUTI - MTMSI[1]: %u\n"
            " GUTI - MTMSI[2]: %u\n"
            " GUTI - MTMSI[3]: %u\n"
            " TIN Value: %d\n",
            buf.emm_usimsrv_para.update_status,
            buf.emm_usimsrv_para.cs_sim_status,
            buf.emm_usimsrv_para.ps_sim_status,
            buf.emm_usimsrv_para.guti.plmn_id.mcc1,
            buf.emm_usimsrv_para.guti.plmn_id.mcc2,
            buf.emm_usimsrv_para.guti.plmn_id.mcc3,
            buf.emm_usimsrv_para.guti.plmn_id.mnc1,
            buf.emm_usimsrv_para.guti.plmn_id.mnc2,
            buf.emm_usimsrv_para.guti.plmn_id.mnc3,
            buf.emm_usimsrv_para.guti.mme_gid[0],
            buf.emm_usimsrv_para.guti.mme_gid[1],
            buf.emm_usimsrv_para.guti.mme_code,
            buf.emm_usimsrv_para.guti.mtmsi[0],
            buf.emm_usimsrv_para.guti.mtmsi[1],
            buf.emm_usimsrv_para.guti.mtmsi[2],
            buf.emm_usimsrv_para.guti.mtmsi[3],
            buf.emm_nvmsrv_para.tin);
        break;
    }
    default:
        break;
    }
    XLOGD("parse output %s\n", output);
}

static jcharArray parse(JNIEnv* env, jobject thiz, jint type, jcharArray data) {
    XLOGD("lte_parse %d\n", type);
    jchar *input = NULL;
    int input_len = 0;
    if (data) {
        input = env->GetCharArrayElements(data, NULL);
        input_len = env->GetArrayLength(data);
    }
    char *output = new char[MAX_LENGTH];
    memset(output, 0, MAX_LENGTH);

    do_parse(type, input, input_len, output);

    jsize end = strlen(output);
    jcharArray resultArray = env->NewCharArray(end);
    unsigned short *usResultArray = new unsigned short[MAX_LENGTH];
    memset(usResultArray, 0, sizeof(MAX_LENGTH * sizeof(unsigned short)));
    for (int i = 0; i < end; i++) {
        usResultArray[i] = output[i];
    }

    env->SetCharArrayRegion(resultArray, 0, end, (const jchar *)usResultArray);
    delete output;
    delete usResultArray;
    return resultArray;
}

static JNINativeMethod methods[] = {
        { "getTypes", "(I)[I",(void *) get_types },
        { "size", "(I)I",(void *) get_size },
        { "parse", "(I[C)[C",(void *) parse },
};

static int register_methods(JNIEnv *env) {
    XLOGE("register_methods");
    return AndroidRuntime::registerNativeMethods(env,
            "com/mediatek/engineermode/lte/UrcParser", methods, NELEM(methods));
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    XLOGD("Enter JNI_OnLoad()...\n");
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        XLOGE("ERROR: GetEnv failed\n");
        goto bail;
    }
    assert(env != NULL);

    if (register_methods(env) < 0) {
        XLOGE("ERROR: native registration failed\n");
        goto bail;
    }

    result = JNI_VERSION_1_4;

    XLOGD("Leave JNI_OnLoad()...\n");
    bail: return result;
}

