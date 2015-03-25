#include "jniNFC.h"
#include "JniList.h"
#include "Utils.h"
#include "comDef.h"
#include "CMetaNfc.h"
#include "meta_nfc_para.h"

#define LOG_TAG "nfc"
#include <cutils/xlog.h>

void GetSettingsReqFromJava(nfc_setting_request& req_c, JNIEnv*& env,
		jobject& req_j) {
	JVALUE nfc_enable_val;
	JVALUE debug_enable_val;
	JVALUE sw_protocol_val;
	JVALUE get_capabilities_val;
	JVALUE test1_val;
	JVALUE test2_val;

	XGetField(env, req_j, "nfc_enable", ENUM_INT, 0, nfc_enable_val);
	XGetField(env, req_j, "debug_enable", ENUM_INT, 0, debug_enable_val);
	XGetField(env, req_j, "sw_protocol", ENUM_INT, 0, sw_protocol_val);
	XGetField(env, req_j, "get_capabilities", ENUM_INT, 0, get_capabilities_val);
	XGetField(env, req_j, "test1", ENUM_OBJECT, SIG_SHORT_ARRAY, test1_val);
	XGetField(env, req_j, "test2", ENUM_OBJECT, SIG_2D_SHORT_ARRAY, test2_val);

	req_c.debug_enable = debug_enable_val.int_val;
	req_c.nfc_enable = nfc_enable_val.int_val;
	req_c.sw_protocol = sw_protocol_val.int_val;
	req_c.get_capabilities = get_capabilities_val.int_val;

	jint m, n;
	XGet2DArrayDimension(env, test2_val.object_val, &m, &n);
	if (m != 4 || n != 4)// dimension
	{
		env->ThrowNew(env->FindClass("android/util/AndroidException"),
				"matrix dimension is not 4*4");
		return;
	}

	jshort array2[4][4];// = { 0 };
	short *arrayx = array2[0];
	XGet2DShortFromArray(env, test2_val.object_val, (jshort*) arrayx);
	//now array2 is available.

	jshort *colData = env->GetShortArrayElements(
			(jshortArray) test1_val.object_val, 0);
	for (int j = 0; j < 16; j++) {
		//data[j] = colData[j];
	}
	env->ReleaseShortArrayElements((jshortArray) test1_val.object_val, colData,
			0);
	////now data is available.
	return;
}

void SetSettingsResToJava(nfc_setting_response& res_c, JNIEnv*& env,
		jobject& res_j) {
	JVALUE status_val;
	JVALUE debug_enable_val;
	JVALUE card_mode_val;
	JVALUE fw_ver_val;
	JVALUE get_capabilities_val;
	JVALUE hw_ver_val;
	JVALUE nfc_enable_val;
	JVALUE reader_mode_val;
	JVALUE sw_protocol_val;
	JVALUE sw_ver_val;

	JVALUE test1_val;
	JVALUE test2_val;

	status_val.int_val = res_c.status;
	debug_enable_val.int_val = res_c.debug_enable;
	card_mode_val.int_val = res_c.card_mode;
	fw_ver_val.int_val = res_c.fw_ver;
	get_capabilities_val.int_val = res_c.get_capabilities;
	hw_ver_val.int_val = res_c.hw_ver;
	nfc_enable_val.int_val = res_c.nfc_enable;
	reader_mode_val.int_val = res_c.reader_mode;
	sw_protocol_val.int_val = res_c.sw_protocol;
	sw_ver_val.int_val = res_c.sw_ver;

	XSetField(env, res_j, "status", ENUM_INT, 0, status_val);
	XSetField(env, res_j, "debug_enable", ENUM_INT, 0, debug_enable_val);
	XSetField(env, res_j, "card_mode", ENUM_INT, 0, card_mode_val);
	XSetField(env, res_j, "fw_ver", ENUM_INT, 0, fw_ver_val);
	XSetField(env, res_j, "get_capabilities", ENUM_INT, 0, get_capabilities_val);
	XSetField(env, res_j, "hw_ver", ENUM_INT, 0, hw_ver_val);
	XSetField(env, res_j, "nfc_enable", ENUM_INT, 0, nfc_enable_val);
	XSetField(env, res_j, "reader_mode", ENUM_INT, 0, reader_mode_val);
	XSetField(env, res_j, "sw_protocol", ENUM_INT, 0, sw_protocol_val);
	XSetField(env, res_j, "sw_ver", ENUM_INT, 0, sw_ver_val);

	short array2[4][4] = { { 0, 1, 2, 3 }, { 1, 10, 20, 30 }, { 2, 100, 200,
			300 }, { 3, 1000, 2000, 3000 } };
	short *arrayx = array2[0];
	test2_val.object_val = XMake2DShortArray(env, (short*) arrayx, 4, 4);
	XSetField(env, res_j, "test2", ENUM_OBJECT, SIG_2D_SHORT_ARRAY, test2_val);

	//  array  operate.
	short array1[16] = { 1, 2, 3, 4, 5, 6 };
	test1_val.object_val = env->NewShortArray(16);
	env->SetShortArrayRegion((jshortArray) test1_val.object_val, 0, 16, array1);
	XSetField(env, res_j, "test1", ENUM_OBJECT, SIG_SHORT_ARRAY, test1_val);
	env->DeleteLocalRef(test1_val.object_val);

	return;
}

void GetRegisterNotifReqFromJava(nfc_reg_notif_request& req_c, JNIEnv*& env,
		jobject& req_j) {
	JVALUE reg_type_val;
	XGetField(env, req_j, "reg_type", ENUM_INT, 0, reg_type_val);
	req_c.reg_type = reg_type_val.int_val;

	return;
}
void SetRegisterNotifResToJava(nfc_reg_notif_response& res_c,
		JNIEnv*& env, jobject& res_j) {
	JVALUE status_val;
	JVALUE se_val;
	JVALUE se_status_val;
	JVALUE se_type_val;
	JVALUE length_val;
	JVALUE data_val;

	status_val.int_val = res_c.status;
	se_val.int_val = res_c.se;
	se_status_val.int_val = res_c.se_status;
	se_type_val.int_val = res_c.se_type;
	length_val.int_val = res_c.length;
	if (length_val.int_val != 0) {
		XLOGD("RES: data len %d", length_val.int_val);
		//XLOGD("RES: data %s", res_c.data);
		data_val.object_val = XByteBuf2NewByteArray(env, /*res_c.data*/0,
				/*res_c.length*/0);
		XSetField(env, res_j, "data", ENUM_OBJECT, SIG_BYTEARRAY, data_val);
		XLOGD("RES: SIG_BYTEARRAY set ok");
	}

	XSetField(env, res_j, "status", ENUM_INT, 0, status_val);
	XSetField(env, res_j, "se", ENUM_INT, 0, se_val);
	XSetField(env, res_j, "se_status", ENUM_INT, 0, se_status_val);
	XSetField(env, res_j, "se_type", ENUM_INT, 0, se_type_val);
	XSetField(env, res_j, "length", ENUM_INT, 0, length_val);

	XLOGD("RES: status %d , se %d, se_status %d, se_type %d, length %d",
			status_val.int_val, se_val.int_val, se_status_val.int_val, se_type_val.int_val, length_val.int_val);
	// object may need release local reference.
	env->DeleteLocalRef(data_val.object_val);

	return;
}

void GetDiscoveryNotifReqFromJava(nfc_dis_notif_request& req_c, JNIEnv*& env,
		jobject& req_j) {
	JVALUE dis_type_val;
	JVALUE duration_val;
	XGetField(env, req_j, "dis_type", ENUM_INT, 0, dis_type_val);
	XGetField(env, req_j, "duration", ENUM_INT, 0, duration_val);
	req_c.dis_type = dis_type_val.int_val;
	req_c.duration = duration_val.int_val;

	return;
}

void SetDiscoveryNotifResToJava(nfc_dis_notif_response& res_c, JNIEnv*& env,
		jobject& res_j) {
	JVALUE status_val;
	JVALUE which_val;
	JVALUE payload_val;

	status_val.int_val = res_c.status;
	which_val.int_val = res_c.type;

	LOGI("REQ: SetDiscoveryNotifResToJava status %d", status_val.int_val);
	if (status_val.int_val != 0 /*which_val.int_val == 0TYPE_DET_NONE*/) {
		XLOGE("REQ: SetDiscoveryNotifResToJava status %d",
				status_val.int_val);
		JVALUE target_val;
		target_val.object_val = XNewElement(env, CLASS_DIS_NONE_DET_RES);

		JVALUE dummy_val;
		dummy_val.int_val = 0;
		XSetField(env, target_val.object_val, "dummy", ENUM_INT, 0, dummy_val);

		XSetField(env, res_j, "target", ENUM_OBJECT, SIG_OBJECT, target_val);

		XSetField(env, res_j, "status", ENUM_INT, 0, status_val);
		return;
	}

	if (which_val.int_val == 1/*TYPE_DET_TAG*/) {
		XLOGD("REQ: SetDiscoveryNotifResToJava which_val %d  TYPE_DET_TAG",
				which_val.int_val);
		JVALUE target_val;
		target_val.object_val = XNewElement(env, CLASS_DIS_TAG_DET_RES);

		JVALUE tag_type_val;
		JVALUE card_type_val;
		JVALUE uid_val;
		JVALUE sak_val;
		JVALUE atag_val;
		JVALUE appdata_val;
		JVALUE maxdatarate_val;

		tag_type_val.int_val = res_c.nfc_dis_notif_result.nfc_tag_det_resp.tag_type;
		card_type_val.int_val = res_c.nfc_dis_notif_result.nfc_tag_det_resp.card_type;
		//uid_val.int_val = res_c.nfc_dis_notif_result.nfc_tag_det_resp.uid;
		sak_val.int_val = res_c.nfc_dis_notif_result.nfc_tag_det_resp.sak;
		atag_val.int_val = res_c.nfc_dis_notif_result.nfc_tag_det_resp.atag;
		appdata_val.int_val = res_c.nfc_dis_notif_result.nfc_tag_det_resp.appdata;
		maxdatarate_val.int_val = res_c.nfc_dis_notif_result.nfc_tag_det_resp.maxdatarate;

		//  array  operate.
		uid_val.object_val = env->NewByteArray(10);
		env->SetByteArrayRegion((jbyteArray) uid_val.object_val, 0, 10, (const jbyte*)res_c.nfc_dis_notif_result.nfc_tag_det_resp.uid);
		XSetField(env, target_val.object_val, "uid", ENUM_OBJECT, SIG_BYTEARRAY, uid_val);
		env->DeleteLocalRef(uid_val.object_val);

		//for debug
		char UID[11] = {0};
		memcpy(UID, res_c.nfc_dis_notif_result.nfc_tag_det_resp.uid, 10);
		XLOGD("uid %s", UID);

		XSetField(env, target_val.object_val, "tag_type", ENUM_INT, 0,
				tag_type_val);
		XSetField(env, target_val.object_val, "card_type", ENUM_INT, 0,
				card_type_val);
		//XSetField(env, target_val.object_val, "uid", ENUM_INT, 0, uid_val);
		XSetField(env, target_val.object_val, "sak", ENUM_INT, 0, sak_val);
		XSetField(env, target_val.object_val, "atag", ENUM_INT, 0, atag_val);
		XSetField(env, target_val.object_val, "appdata", ENUM_INT, 0,
				appdata_val);
		XSetField(env, target_val.object_val, "maxdatarate", ENUM_INT, 0,
				maxdatarate_val);

		XSetField(env, res_j, "target", ENUM_OBJECT, SIG_OBJECT, target_val);
	} else if (which_val.int_val == 2/*TYPE_DET_P2P*/) {
		XLOGD("REQ: SetDiscoveryNotifResToJava which_val %d  TYPE_DET_P2P",
						which_val.int_val);
		JVALUE target_val;
		target_val.object_val = XNewElement(env, CLASS_DIS_P2P_DET_RES);

		JVALUE p2p_type_val;
		p2p_type_val.int_val = res_c.nfc_dis_notif_result.nfc_p2p_det_resp.p2p_type;
		XSetField(env, target_val.object_val, "p2p_type", ENUM_INT, 0,
				p2p_type_val);

		XSetField(env, res_j, "target", ENUM_OBJECT, SIG_OBJECT, target_val);
	} else {
		LOGE("REQ: SetDiscoveryNotifResToJava unknown which %d",
				which_val.int_val);
		env->ThrowNew(env->FindClass("android/util/AndroidException"),
				"scan type is unknown");
	}

	return;
}

void GetTestReqFromJava(NFC_REQ& req_c, JNIEnv*& env,
		jobject& req_j)
{
	JVALUE which_val;
		JVALUE req_target_val;

		JVALUE type_val;
		JVALUE action_val;
		JVALUE modulation_val;
		JVALUE bitrate_val;

		XGetField(env, req_j, "which", ENUM_INT, 0, which_val);
		XLOGD("which %d", which_val.int_val);
		switch(which_val.int_val)
		{
		case TEST_ID_ALWAYSE_ON_WITH:
			req_c.op = NFC_OP_TX_ALWAYSON_TEST;
			XLOGD("enter NFC_OP_TX_ALWAYSON_TEST");
			XGetField(env, req_j, "target", ENUM_OBJECT, SIG_OBJECT, req_target_val);
			XLOGD("get SIG_TAG_TEST_ALWAYSEON_REQ OK");
			XGetField(env, req_target_val.object_val, "type", ENUM_INT, 0, type_val);
			XGetField(env, req_target_val.object_val, "action", ENUM_INT, 0, action_val);
			XGetField(env, req_target_val.object_val, "modulation_type", ENUM_BYTE, 0, modulation_val);
			XGetField(env, req_target_val.object_val, "bitrate", ENUM_BYTE, 0, bitrate_val);

			req_c.cmd.m_nfc_tx_alwayson_req.type = type_val.int_val;
			req_c.cmd.m_nfc_tx_alwayson_req.action = action_val.int_val;
			req_c.cmd.m_nfc_tx_alwayson_req.modulation_type = modulation_val.int_val;
			req_c.cmd.m_nfc_tx_alwayson_req.bitrate = bitrate_val.int_val;

			break;
		case TEST_ID_ALWAYSE_ON_WO:
			req_c.op = NFC_OP_TX_ALWAYSON_WO_ACK_TEST;
			XGetField(env, req_j, "target", ENUM_OBJECT, SIG_OBJECT, req_target_val);

			XGetField(env, req_target_val.object_val, "type", ENUM_INT, 0, type_val);
			XGetField(env, req_target_val.object_val, "action", ENUM_INT, 0, action_val);
			XGetField(env, req_target_val.object_val, "modulation_type", ENUM_BYTE, 0, modulation_val);
			XGetField(env, req_target_val.object_val, "bitrate", ENUM_BYTE, 0, bitrate_val);

			req_c.cmd.m_nfc_tx_alwayson_req.type = type_val.int_val;
			req_c.cmd.m_nfc_tx_alwayson_req.action = action_val.int_val;
			req_c.cmd.m_nfc_tx_alwayson_req.modulation_type = modulation_val.byte_val;
			req_c.cmd.m_nfc_tx_alwayson_req.bitrate = bitrate_val.byte_val;

			break;
		case TEST_ID_CARD_EMUL_MODE:
			req_c.op = NFC_OP_CARD_MODE_TEST;
			XGetField(env, req_j, "target", ENUM_OBJECT, SIG_OBJECT, req_target_val);

			JVALUE technology_val;
			JVALUE protocols_val;
			XGetField(env, req_target_val.object_val, "type", ENUM_INT, 0, type_val);
			XGetField(env, req_target_val.object_val, "action", ENUM_INT, 0, action_val);
			XGetField(env, req_target_val.object_val, "technology", ENUM_SHORT, 0, technology_val);
			XGetField(env, req_target_val.object_val, "protocols", ENUM_SHORT, 0, protocols_val);

			req_c.cmd.m_nfc_card_emulation_req.type = type_val.int_val;
			req_c.cmd.m_nfc_card_emulation_req.action = action_val.int_val;
			req_c.cmd.m_nfc_card_emulation_req.technology = technology_val.short_val;
			req_c.cmd.m_nfc_card_emulation_req.protocols = protocols_val.short_val;
			break;
		case TEST_ID_READER_MODE:
			req_c.op = NFC_OP_READER_MODE_TEST;

			XGetField(env, req_j, "target", ENUM_OBJECT, SIG_OBJECT, req_target_val);

			XGetField(env, req_target_val.object_val, "type", ENUM_INT, 0, type_val);
			XGetField(env, req_target_val.object_val, "action", ENUM_INT, 0, action_val);
			req_c.cmd.m_script_req.type = type_val.int_val;
			req_c.cmd.m_script_req.action = action_val.int_val;
			break;
		case TEST_ID_P2P_MODE:
			req_c.op = NFC_OP_P2P_MODE_TEST;
			XGetField(env, req_j, "target", ENUM_OBJECT, SIG_OBJECT, req_target_val);

			XGetField(env, req_target_val.object_val, "type", ENUM_INT, 0, type_val);
			XGetField(env, req_target_val.object_val, "action", ENUM_INT, 0, action_val);
			req_c.cmd.m_script_req.type = type_val.int_val;
			req_c.cmd.m_script_req.action = action_val.int_val;
			break;
		case TEST_ID_SWP_SELF:
			req_c.op = NFC_OP_SWP_SELF_TEST;
			XGetField(env, req_j, "target", ENUM_OBJECT, SIG_OBJECT, req_target_val);

			XGetField(env, req_target_val.object_val, "type", ENUM_INT, 0, type_val);
			XGetField(env, req_target_val.object_val, "action", ENUM_INT, 0, action_val);
			req_c.cmd.m_script_req.type = type_val.int_val;
			req_c.cmd.m_script_req.action = action_val.int_val;
			break;
		case TEST_ID_ANTENNA_SELF:
			req_c.op = NFC_OP_ANTENNA_SELF_TEST;
			XGetField(env, req_j, "target", ENUM_OBJECT, SIG_OBJECT, req_target_val);

			XGetField(env, req_target_val.object_val, "type", ENUM_INT, 0, type_val);
			XGetField(env, req_target_val.object_val, "action", ENUM_INT, 0, action_val);
			req_c.cmd.m_script_req.type = type_val.int_val;
			req_c.cmd.m_script_req.action = action_val.int_val;
			break;
		case TEST_ID_UID_RW:
			req_c.op = NFC_OP_TAG_UID_RW;
			XGetField(env, req_j, "target", ENUM_OBJECT, SIG_OBJECT, req_target_val);

			JVALUE uid_type_val;
			JVALUE data_val;
			XGetField(env, req_target_val.object_val, "type", ENUM_INT, 0, type_val);
			XGetField(env, req_target_val.object_val, "action", ENUM_INT, 0, action_val);
			XGetField(env, req_target_val.object_val, "uid_type", ENUM_INT, 0, uid_type_val);
			XGetField(env, req_target_val.object_val, "data", ENUM_OBJECT, SIG_BYTEARRAY, data_val);

			req_c.cmd.m_script_uid_req.type = type_val.int_val;
			req_c.cmd.m_script_uid_req.action = action_val.int_val;
			req_c.cmd.m_script_uid_req.uid_type = uid_type_val.int_val;
			XCopyByteArray2ByteBuf(env, (char*)req_c.cmd.m_script_uid_req.data, data_val.object_val);
			break;
		default:
			break;
		}
		return;
}

void SetTestResToJava(NFC_CNF& res_c, JNIEnv*& env,
		jobject& res_j) {

	JVALUE target_val;
	JVALUE result_val;

	int data_size = 0;

	switch(res_c.op)
	{
	case NFC_OP_TX_ALWAYSON_TEST:
	case NFC_OP_TX_ALWAYSON_WO_ACK_TEST:
	case NFC_OP_CARD_MODE_TEST:
	case NFC_OP_READER_MODE_TEST:
	case NFC_OP_P2P_MODE_TEST:
	case NFC_OP_SWP_SELF_TEST:
	case NFC_OP_ANTENNA_SELF_TEST:
		target_val.object_val = XNewElement(env, CLASS_TAG_TEST_SCRIPT_RES);

		result_val.int_val = res_c.result.m_script_cnf.result;
		XSetField(env, target_val.object_val, "result", ENUM_INT, 0, result_val);

		XSetField(env, res_j, "target", ENUM_OBJECT, SIG_OBJECT, target_val);
		break;
	case NFC_OP_TAG_UID_RW:
		target_val.object_val = XNewElement(env, CLASS_TAG_TEST_UID_RES);

		JVALUE uid_type_val;
		JVALUE data_val;
		result_val.int_val = res_c.result.m_script_uid_cnf.result;
		uid_type_val.int_val = res_c.result.m_script_uid_cnf.uid_type;
		XSetField(env, target_val.object_val, "result", ENUM_INT, 0, result_val);
		XSetField(env, target_val.object_val, "uid_type", ENUM_INT, 0, uid_type_val);
		data_size = uid_type_val.int_val;  // modify , type == length
		/*
		data_size = 0;
		if(uid_type_val.int_val == 1)
		{
			data_size = 4;
		}
		else if(uid_type_val.int_val == 2)
		{
			data_size = 7;
		}
		*/
		LOGI("Before set data1.");
		if(uid_type_val.int_val != 0 && res_c.result.m_script_uid_cnf.data != 0 && data_size != 0)
		{
			LOGI("Before set data2. data:%x, size:%d", res_c.result.m_script_uid_cnf.data, data_size);
			data_val.object_val = XByteBuf2NewByteArray(env, (char*)res_c.result.m_script_uid_cnf.data, data_size);
			LOGI("After set data1.");
			XSetField(env, target_val.object_val, "data", ENUM_OBJECT, SIG_BYTEARRAY, data_val);
			LOGI("After set data2.");
		}

		LOGI("Before set uid");
		XSetField(env, res_j, "target", ENUM_OBJECT, SIG_OBJECT, target_val);
		LOGI("Aftere set uid");
		break;
	default:
		LOGE("Unknown OP in SetTestResToJava:%d", res_c.op);
		env->ThrowNew(env->FindClass("android/util/AndroidException"),
						"Unknown OP in SetTestResToJava");
		break;

	}
	return;
}
//////////////////////////////////////////////////////////////////////////////////

JNIEXPORT jobject JNICALL Java_com_mediatek_engineermode_nfc_NfcNativeCallClass_getSettings(
		JNIEnv * env, jclass jc, jobject req_j) {

	NFC_REQ req_c;
	req_c.op = NFC_OP_SETTING;
	GetSettingsReqFromJava(req_c.cmd.m_setting_req, env, req_j);

	NFC_CNF resp;
	memset(&resp, 0, sizeof(NFC_CNF));

	XLOGD("JNI-REQ: nfc_enable, debug_enable, sw_protocol, %d,%d,%d",
			req_c.cmd.m_setting_req.nfc_enable,
			req_c.cmd.m_setting_req.debug_enable,
			req_c.cmd.m_setting_req.sw_protocol);

	CMetaNfc::SendCommand(&req_c, 0,0, /*IN OUT*/&resp);
	if(RESULT_STATUS_TIMEOUT == resp.status)
	{
		resp.result.m_setting_cnf.status = RESULT_STATUS_TIMEOUT;
		//resp.result.m_setting_cnf.sw_protocol = 2; // java side OFF.
	}

	jobject res_j = XNewElement(env, CLASS_SETTING_RES);

	SetSettingsResToJava(resp.result.m_setting_cnf, env, res_j);
	return res_j;

}

JNIEXPORT jobject JNICALL Java_com_mediatek_engineermode_nfc_NfcNativeCallClass_getRegisterNotif(
		JNIEnv * env, jclass jc, jobject req_j) {

	NFC_REQ req_c;
	req_c.op = NFC_OP_REG_NOTIFY;

	GetRegisterNotifReqFromJava(req_c.cmd.m_reg_notify_req, env, req_j);

	XLOGD("REQ: bitmap %d", req_c.cmd.m_reg_notify_req.reg_type);

	NFC_CNF resp;
	memset(&resp, 0, sizeof(NFC_CNF));

	CMetaNfc::SendCommand(&req_c, 0,0, /*IN OUT*/&resp);
/*
	//res_c = exec(req_c);
	res_c.se = 1; //ele 1 detected.
	res_c.se_status = 1; //virtual;
	res_c.se_type = 55; //bitmap
	res_c.length = 10;

	nfc_reg_notif_response_wrap* presw_c = 0;

	int sz = sizeof(nfc_reg_notif_response_wrap) + res_c.length;
	presw_c = (nfc_reg_notif_response_wrap*) malloc(sz);

	if (presw_c == 0) {
		env->ThrowNew(env->FindClass("android/util/AndroidException"),
				"getRegisterNotif out of memory.");
		return 0;
	}
	memset(presw_c, 0, sz);

	memcpy(presw_c, &res_c, sizeof(nfc_reg_notif_response));
	if (res_c.length != 0) {
		////exec(resw_c.data)
		memcpy(presw_c->data, "ABCDEABCDE", 10);
	}
*/
	jobject res_j = XNewElement(env, CLASS_REG_NOTIF_RES);

	SetRegisterNotifResToJava(resp.result.m_reg_notify_cnf, env, res_j);

	//free(presw_c);
	return res_j;

}

JNIEXPORT jobject JNICALL Java_com_mediatek_engineermode_nfc_NfcNativeCallClass_getDiscoveryNotif(
		JNIEnv * env, jclass jc, jobject req_j) {

	NFC_REQ req_c;
	req_c.op = NFC_OP_DISCOVERY;

	GetDiscoveryNotifReqFromJava(req_c.cmd.m_dis_notify_req, env, req_j);

	NFC_CNF resp;
	memset(&resp, 0, sizeof(NFC_CNF));

	int errcode = CMetaNfc::SendCommand(&req_c, 0,0, /*IN OUT*/&resp);
	if(RESULT_STATUS_TIMEOUT == resp.status)
	{
		resp.result.m_dis_notify_cnf.status = RESULT_STATUS_TIMEOUT;
	}

	jobject res_j = XNewElement(env, CLASS_DIS_NOTIF_RES);

	SetDiscoveryNotifResToJava(resp.result.m_dis_notify_cnf, env, res_j);

	return res_j;

}

JNIEXPORT jobject JNICALL Java_com_mediatek_engineermode_nfc_NfcNativeCallClass_setSEOption(
		JNIEnv * env, jclass jc, jobject req_j) {
	NFC_REQ req_c;
	req_c.op = NFC_OP_SECURE_ELEMENT;

	JVALUE set_SEtype_val;
	XGetField(env, req_j, "set_SEtype", ENUM_INT, 0, set_SEtype_val);
	req_c.cmd.m_se_set_req.set_SEtype = set_SEtype_val.int_val;

	XLOGD("REQ: set_SEtype %d", req_c.cmd.m_se_set_req.set_SEtype);

	NFC_CNF resp;
	memset(&resp, 0, sizeof(NFC_CNF));

	CMetaNfc::SendCommand(&req_c, 0,0, /*IN OUT*/&resp);

	jobject res_j = XNewElement(env, CLASS_SE_SET_RES);

	JVALUE status_val;
	status_val.int_val = resp.result.m_se_set_cnf.status;
	XSetField(env, res_j, "status", ENUM_INT, 0, status_val);

	return res_j;

}


JNIEXPORT jobject JNICALL Java_com_mediatek_engineermode_nfc_NfcNativeCallClass_testEntry
(JNIEnv * env, jclass jc, jobject req_j)
{
	NFC_REQ req_c;

	XLOGD("enter testEntry");
	GetTestReqFromJava(req_c, env, req_j);
	XLOGD("testEntry GetTestReqFromJava over");

	NFC_CNF resp;
	memset(&resp, 0, sizeof(NFC_CNF));


	CMetaNfc::SendCommand(&req_c, 0,0, /*IN OUT*/&resp);
	if(RESULT_STATUS_TIMEOUT == resp.status)
	{
		resp.op = req_c.op;
		resp.result.m_script_cnf.result = RESULT_STATUS_TIMEOUT;
	}
	XLOGD("testEntry SendCommand over");

	jobject res_j = XNewElement(env, CLASS_TAG_TEST_RES);

	XLOGD("testEntry new CLASS_TAG_TEST_RES over");
	SetTestResToJava(resp, env, res_j);

	XLOGD("testEntry SetTestResToJava over");
	return res_j;
}
static int oncerInit = 0;
JNIEXPORT int JNICALL Java_com_mediatek_engineermode_nfc_NfcNativeCallClass_initNfcDriver
(JNIEnv * env, jclass jc)
{
	XLOGD("META_NFC_init was called!");
	if(oncerInit == 0)
	{
		oncerInit = 1;
		XLOGD("First call initNfcDriver");
		CMetaNfc::Init();
		int ret = META_NFC_init();
		if(ret != 0)
		{
			oncerInit = 0;
			XLOGD("initNfcDriver Failed.");
		}
		return ret;
	}
	else
	{
		XLOGD("DUP call initNfcDriver, Nothing will be done.");
		return 0;
	}

}

JNIEXPORT void JNICALL Java_com_mediatek_engineermode_nfc_NfcNativeCallClass_deinitNfcDriver(
		JNIEnv * env, jclass jc) {
	XLOGD("META_NFC_deinit was called!");

	if (oncerInit == 1) {
		oncerInit = 0;
		XLOGD("First call DEinitNfcDriver");

		NFC_REQ req_c;
		XLOGD("enter deinitNfcDriver");
		req_c.op = NFC_OP_DEINIT;

		NFC_CNF resp;
		memset(&resp, 0, sizeof(NFC_CNF));

		CMetaNfc::SendCommand(&req_c, 0,0, /*IN OUT*/&resp);
		if(RESULT_STATUS_TIMEOUT == resp.status)
		{
			resp.op = req_c.op;
			resp.result.m_script_cnf.result = RESULT_STATUS_TIMEOUT;
			LOGE("DEinitNfcDriver Timeout.");
		}
		XLOGD("DEinitNfcDriver SendCommand over");

		CMetaNfc::DeInit();
		META_NFC_deinit();
		return ;
	} else {
		XLOGD("DUP call DE initNfcDriver, Nothing will be done.");
		return;
	}
}

extern "C" int WriteToEM(void *Local_buf, unsigned short Local_len, void *Peer_buf,
		unsigned short Peer_len) {

	XLOGD("WriteToEM was called!");
	CMetaNfc::NotifyResponse((NFC_CNF*) Local_buf);

	return 0;

}



