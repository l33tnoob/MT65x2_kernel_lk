package com.hissage.struct;

public class SNmsErrorCode {
    /* error code return by functions in engineadapter.java */
    public static final int NMS_ERROR_GENERAL           = -1;   // 一般的错误 
    public static final int NMS_ERROR_INVALID_ARGUMENT  = -2;   // 调用函数时传入非法参数，一般会紧跟着一个assert通知 
    public static final int NMS_ERROR_NETWORK_NOT_OK    = -3;   // 网络暂时不可用错误 
    
    public static final int NMS_ERROR_PHONE_NOT_ACTIVATED = -4; // 手机没有激活 
    public static final int NMS_ERROR_SIM_NOT_ACTIVATED   = -5; // 制定sim card 没有激活 

    public static final int NMS_ERROR_INVALID_MSG_DB_ID      = -6; // 严重错误：非法的msg db id 
    public static final int NMS_ERROR_ADD_MSG_DB_FAILED      = -7; // 严重错误：插入一条记录到msg db 错误 
    public static final int NMS_ERROR_UPDATE_MSG_DB_FAILED   = -8; // 严重错误：更新一条msg db 记录错误 
    public static final int NMS_ERROR_GET_MSG_DB_DATA_FAILED = -9; // 严重错误：获取msg db 数据错误 

    public static final int NMS_ERROR_INVALID_CONTACT_DB_ID       = -10; // 严重错误：非法的contact db id
    public static final int NMS_ERROR_ADD_CONTACT_DB_FAILED       = -11; // 严重错误：插入一条记录到contact db 错误 
    public static final int NMS_ERROR_UPDATE_CONTACT_DB_FAILED    = -12; // 严重错误：更新一条contact db 记录错误 
    public static final int NMS_ERROR_GET_CONTACT_DB_DATA_FAILED  = -13; // 严重错误：获取contact db 数据错误 

    public static final int NMS_ERROR_NEW_IP_MSG_ERROR_FORMAT           = -14; // 发送IP消息/保存draft消息：非法的消息格式 
    public static final int NMS_ERROR_NEW_IP_MSG_INVALID_TO             = -15; // 发送IP消息/保存draft消息：非法的TO字段 
    public static final int NMS_ERROR_NEW_IP_MSG_WITH_UNKNOWN_USER      = -16; // 发送IP消息/保存draft消息：收件人包含非和信用户 
    public static final int NMS_ERROR_NEW_IP_MSG_GCHAT_NO_MBMBER        = -17; // 发送IP群聊消息/保存draft消息：群聊成员为空 
    public static final int NMS_ERROR_NEW_IP_MSG_GCHAT_DEAD             = -18; // 发送IP群聊消息/保存draft消息：群聊处于非激活状态 
    public static final int NMS_ERROR_NEW_IP_MSG_GCHAT_INVALID_SIM_ID   = -19; // 发送IP群聊消息：群聊不属于传入的sim card 

    public static final int NMS_ERROR_RESEND_IP_MSG_FOR_SYS_MSG         = -20; // 重发IP消息：尝试重发系统消息 
    public static final int NMS_ERROR_RESEND_IP_MSG_INVALID_STATUS      = -21; // 重发IP消息：IP消息处于不可重发状态 

    public static final int NMS_ERROR_GCHAT_IS_DEAD                     = -22; // 群聊控制相关：群聊处于非激活状态 
    public static final int NMS_ERROR_GCHAT_INVALID_MEMBERS             = -23; // 群聊控制相关：创建/添加时非法的成员列表 
    public static final int NMS_ERROR_GCHAT_INVALID_MEMBER_WITH_SELF    = -24; // 群聊控制相关：创建/添加时成员包含自己的号码(多卡下为群聊所属sim card的号码)
    public static final int NMS_ERROR_GCHAT_EXCEED_MAX_MEMBER_COUNT     = -25; // 群聊控制相关：创建/添加时成员数超过最大数量 
    public static final int NMS_ERROR_GCHAT_INVALID_NAME                = -26; // 群聊控制相关：非法的群聊名字 
    public static final int NMS_ERROR_GCHAT_SIM_IS_NOT_ACTIVATED        = -27; // 群聊控制相关：群聊所属sim card 处于未激活状态 

    public static final int NMS_ERROR_GET_DOWNLOAD_URL_NOT_INBOX_MSG        = -29; // 获取下载URL失败：不是收到的消息 
    public static final int NMS_ERROR_GET_DOWNLOAD_URL_INVALID_URL          = -30; // 获取下载URL失败：消息保存的是非法的URL 
    public static final int NMS_ERROR_GET_ATTACH_NAME_INVALID_BODY_FORMAT   = -31; // 获取下载附件名字失败：消息没有保存附件名字 
    public static final int NMS_ERROR_SET_DOWNLOADED_FILE_INVALID_FILE_NAME = -32; // 设置下载文件失败：错误的消息文件名 
    public static final int NMS_ERROR_SET_DOWNLOADED_FILE_NOT_EXIST         = -33; // 设置下载文件失败：文件不存在 
    public static final int NMS_ERROR_SET_DOWNLOADED_FILE_NOT_INBOX_MSG     = -34; // 设置下载文件失败：不是收到的消息 

    public static final int NMS_ERROR_SIM_INFO_NOT_FOUND    = -35; // Enable/Disable Sim card 失败：engine没有找到相关的sim card信息 
    public static final int NMS_ERROR_SIM_IN_INVALID_STATUS = -36; // Enable/Disable Sim card 失败：sim card处于一个错误状态 

    public static final int NMS_ERROR_CMD_PROCESSING        = -37; // 系统忙，不能处理config center命令(设置和信信息时需检查该返回值) 
    public static final int NMS_ERROR_CMD_GENERAL           = -38; // 一般的config center错误 

    public static final int NMS_ERROR_CODE_END = -1000;
}
