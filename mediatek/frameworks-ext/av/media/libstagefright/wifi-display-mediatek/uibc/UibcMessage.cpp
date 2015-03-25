

#define LOG_TAG "UibcMessage"
#include <utils/Log.h>

#include "UibcMessage.h"
#include "WifiDisplayUibcType.h"

#include <media/IRemoteDisplayClient.h>

#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/MediaErrors.h>
#include <media/stagefright/foundation/hexdump.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>

#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <KeycodeLabels.h>

#include <ui/DisplayInfo.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>

namespace android {

static const scanCodeBuild_t scanCode_DefaultMap[] = {
    {0x00, false, 0x00}, //0x00	#	NULL
    {0x00, false, 0x00}, //0x01	#	START OF HEADING
    {0x00, false, 0x00}, //0x02	#	START OF TEXT
    {KEY_END, false, 0x00}, //0x03	#	END OF TEXT
    {0x00, false, 0x00}, //0x04	#	END OF TRANSMISSION
    {0x00, false, 0x00}, //0x05	#	ENQUIRY
    {0x00, false, 0x00}, //0x06	#	ACKNOWLEDGE
    {0x00, false, 0x00}, //0x07	#	BELL
    {KEY_BACKSPACE, false, 0x00}, //0x08	#	BACKSPACE
    {KEY_TAB, false, 0x00}, //0x09	#	HORIZONTAL TABULATION
    {KEY_LINEFEED, false, 0x00}, //0x0A	#	LINE FEED
    {0x00, false, 0x00}, //0x0B	#	VERTICAL TABULATION
    {0x00, false, 0x00}, //0x0C	#	FORM FEED
    {KEY_ENTER, false, 0x00}, //0x0D	#	CARRIAGE RETURN
    {0x00, false, 0x00}, //0x0E	#	SHIFT OUT
    {KEY_LEFTSHIFT, false, 0x00}, //0x0F	#	SHIFT IN
    {0x00, false, 0x00}, //0x10	#	DATA LINK ESCAPE
    {KEY_LEFTCTRL , false, 0x00}, //0x11	#	DEVICE CONTROL ONE
    {KEY_LEFTCTRL , false, 0x00}, //0x12	#	DEVICE CONTROL TWO
    {KEY_LEFTCTRL , false, 0x00}, //0x13	#	DEVICE CONTROL THREE
    {KEY_LEFTCTRL , false, 0x00}, //0x14	#	DEVICE CONTROL FOUR
    {0x00, false, 0x00}, //0x15	#	NEGATIVE ACKNOWLEDGE
    {0x00, false, 0x00}, //0x16	#	SYNCHRONOUS IDLE
    {0x00, false, 0x00}, //0x17	#	END OF TRANSMISSION BLOCK
    {KEY_CANCEL , false, 0x00}, //0x18	#	CANCEL
    {0x00, false, 0x00}, //0x19	#	END OF MEDIUM
    {0x00, false, 0x00}, //0x1A	#	SUBSTITUTE
    {KEY_BACK , false, 0x00}, //0x1B	#	ESCAPE
    {0x00, false, 0x00}, //0x1C	#	FILE SEPARATOR
    {0x00, false, 0x00}, //0x1D	#	GROUP SEPARATOR
    {0x00, false, 0x00}, //0x1E	#	RECORD SEPARATOR
    {0x00, false, 0x00}, //0x1F	#	UNIT SEPARATOR
    {KEY_SPACE  , false, 0x00}, //0x20	#	SPACE
    {KEY_1, true, 0x00}, //0x21	#	EXCLAMATION MARK
    {KEY_APOSTROPHE, true, 0x00}, //0x22	#	QUOTATION MARK
    {KEY_3, true, 0x00}, //0x23	#	NUMBER SIGN
    {KEY_4, true, 0x00}, //0x24	#	DOLLAR SIGN
    {KEY_5, true, 0x00}, //0x25	#	PERCENT SIGN
    {KEY_7, true, 0x00}, //0x26	#	AMPERSAND
    {KEY_APOSTROPHE , false, 0x00}, //0x27	#	APOSTROPHE
    {KEY_KPLEFTPAREN , false, 0x00}, //0x28	#	LEFT PARENTHESIS
    {KEY_KPRIGHTPAREN , false, 0x00}, //0x29	#	RIGHT PARENTHESIS
    {KEY_KPASTERISK , false, 0x00}, //0x2A	#	ASTERISK
    {KEY_KPPLUS , false, 0x00}, //0x2B	#	PLUS SIGN
    {KEY_COMMA  , false, 0x00}, //0x2C	#	COMMA
    {KEY_MINUS , false, 0x00}, //0x2D	#	HYPHEN-MINUS
    {KEY_DOT , false, 0x00}, //0x2E	#	FULL STOP
    {KEY_SLASH, false, 0x00}, //0x2F	#	SOLIDUS
    {KEY_0, false, 0x00}, //0x30	#	DIGIT ZERO
    {KEY_1, false, 0x00}, //0x31	#	DIGIT ONE
    {KEY_2, false, 0x00}, //0x32	#	DIGIT TWO
    {KEY_3, false, 0x00}, //0x33	#	DIGIT THREE
    {KEY_4, false, 0x00}, //0x34	#	DIGIT FOUR
    {KEY_5, false, 0x00}, //0x35	#	DIGIT FIVE
    {KEY_6, false, 0x00}, //0x36	#	DIGIT SIX
    {KEY_7, false, 0x00}, //0x37	#	DIGIT SEVEN
    {KEY_8, false, 0x00}, //0x38	#	DIGIT EIGHT
    {KEY_9, false, 0x00}, //0x39	#	DIGIT NINE
    {KEY_SEMICOLON, true, 0x00}, //0x3A	#	COLON
    {KEY_SEMICOLON , false, 0x00}, //0x3B	#	SEMICOLON
    {KEY_COMMA, true, 0x00}, //0x3C	#	LESS-THAN SIGN
    {KEY_EQUAL, false, 0x00}, //0x3D	#	EQUALS SIGN
    {KEY_DOT, true, 0x00}, //0x3E	#	GREATER-THAN SIGN
    {KEY_SLASH, true, 0x00}, //0x3F	#	QUESTION MARK
    {KEY_2, true, 0x00}, //0x40	#	COMMERCIAL AT
    {KEY_A, true, 0x00}, //0x41	#	LATIN CAPITAL LETTER A
    {KEY_B, true, 0x00}, //0x42	#	LATIN CAPITAL LETTER B
    {KEY_C, true, 0x00}, //0x43	#	LATIN CAPITAL LETTER C
    {KEY_D, true, 0x00}, //0x44	#	LATIN CAPITAL LETTER D
    {KEY_E, true, 0x00}, //0x45	#	LATIN CAPITAL LETTER E
    {KEY_F, true, 0x00}, //0x46	#	LATIN CAPITAL LETTER F
    {KEY_G, true, 0x00}, //0x47	#	LATIN CAPITAL LETTER G
    {KEY_H, true, 0x00}, //0x48	#	LATIN CAPITAL LETTER H
    {KEY_I, true, 0x00}, //0x49	#	LATIN CAPITAL LETTER I
    {KEY_J, true, 0x00}, //0x4A	#	LATIN CAPITAL LETTER J
    {KEY_K, true, 0x00}, //0x4B	#	LATIN CAPITAL LETTER K
    {KEY_L, true, 0x00}, //0x4C	#	LATIN CAPITAL LETTER L
    {KEY_M, true, 0x00}, //0x4D	#	LATIN CAPITAL LETTER M
    {KEY_N, true, 0x00}, //0x4E	#	LATIN CAPITAL LETTER N
    {KEY_O, true, 0x00}, //0x4F	#	LATIN CAPITAL LETTER O
    {KEY_P, true, 0x00}, //0x50	#	LATIN CAPITAL LETTER P
    {KEY_Q, true, 0x00}, //0x51	#	LATIN CAPITAL LETTER Q
    {KEY_R, true, 0x00}, //0x52	#	LATIN CAPITAL LETTER R
    {KEY_S, true, 0x00}, //0x53	#	LATIN CAPITAL LETTER S
    {KEY_T, true, 0x00}, //0x54	#	LATIN CAPITAL LETTER T
    {KEY_U, true, 0x00}, //0x55	#	LATIN CAPITAL LETTER U
    {KEY_V, true, 0x00}, //0x56	#	LATIN CAPITAL LETTER V
    {KEY_W, true, 0x00}, //0x57	#	LATIN CAPITAL LETTER W
    {KEY_X, true, 0x00}, //0x58	#	LATIN CAPITAL LETTER X
    {KEY_Y, true, 0x00}, //0x59	#	LATIN CAPITAL LETTER Y
    {KEY_Z, true, 0x00}, //0x5A	#	LATIN CAPITAL LETTER Z
    {KEY_LEFTBRACE, false, 0x00}, //0x5B	#	LEFT SQUARE BRACKET
    {KEY_BACKSLASH , false, 0x00}, //0x5C	#	REVERSE SOLIDUS
    {KEY_RIGHTBRACE, false, 0x00}, //0x5D	#	RIGHT SQUARE BRACKET
    {KEY_6, true, 0x00}, //0x5E	#	CIRCUMFLEX ACCENT
    {KEY_MINUS, true, 0x00}, //0x5F	#	LOW LINE
    {KEY_GRAVE , false, 0x00}, //0x60	#	GRAVE ACCENT
    {KEY_A, false, 0x00}, //0x61	#	LATIN SMALL LETTER A
    {KEY_B, false, 0x00}, //0x62	#	LATIN SMALL LETTER B
    {KEY_C, false, 0x00}, //0x63	#	LATIN SMALL LETTER C
    {KEY_D, false, 0x00}, //0x64	#	LATIN SMALL LETTER D
    {KEY_E, false, 0x00}, //0x65	#	LATIN SMALL LETTER E
    {KEY_F, false, 0x00}, //0x66	#	LATIN SMALL LETTER F
    {KEY_G, false, 0x00}, //0x67	#	LATIN SMALL LETTER G
    {KEY_H, false, 0x00}, //0x68	#	LATIN SMALL LETTER H
    {KEY_I, false, 0x00}, //0x69	#	LATIN SMALL LETTER I
    {KEY_J, false, 0x00}, //0x6A	#	LATIN SMALL LETTER J
    {KEY_K, false, 0x00}, //0x6B	#	LATIN SMALL LETTER K
    {KEY_L, false, 0x00}, //0x6C	#	LATIN SMALL LETTER L
    {KEY_M, false, 0x00}, //0x6D	#	LATIN SMALL LETTER M
    {KEY_N, false, 0x00}, //0x6E	#	LATIN SMALL LETTER N
    {KEY_O, false, 0x00}, //0x6F	#	LATIN SMALL LETTER O
    {KEY_P, false, 0x00}, //0x70	#	LATIN SMALL LETTER P
    {KEY_Q, false, 0x00}, //0x71	#	LATIN SMALL LETTER Q
    {KEY_R, false, 0x00}, //0x72	#	LATIN SMALL LETTER R
    {KEY_S, false, 0x00}, //0x73	#	LATIN SMALL LETTER S
    {KEY_T, false, 0x00}, //0x74	#	LATIN SMALL LETTER T
    {KEY_U, false, 0x00}, //0x75	#	LATIN SMALL LETTER U
    {KEY_V, false, 0x00}, //0x76	#	LATIN SMALL LETTER V
    {KEY_W, false, 0x00}, //0x77	#	LATIN SMALL LETTER W
    {KEY_X, false, 0x00}, //0x78	#	LATIN SMALL LETTER X
    {KEY_Y, false, 0x00}, //0x79	#	LATIN SMALL LETTER Y
    {KEY_Z, false, 0x00}, //0x7A	#	LATIN SMALL LETTER Z
    {0x00, false, 0x00}, //0x7B	# LEFT CURLY BRACKET
    {0x00, false, 0x00}, //0x7C	#	VERTICAL LINE
    {0x00, false, 0x00}, //0x7D	#	RIGHT CURLY BRACKET
    {0x00, false, 0x00}, //0x7E	#	TILDE
    {KEY_DELETE , false, 0x00}, //0x7F	#	DELETE
    {0x00, false, 0x00}, //0x80	#	<control>
    {0x00, false, 0x00}, //0x81	#	<control>
    {0x00, false, 0x00}, //0x82	#	<control>
    {0x00, false, 0x00}, //0x83	#	<control>
    {0x00, false, 0x00}, //0x84	#	<control>
    {0x00, false, 0x00}, //0x85	#	<control>
    {0x00, false, 0x00}, //0x86	#	<control>
    {0x00, false, 0x00}, //0x87	#	<control>
    {0x00, false, 0x00}, //0x88	#	<control>
    {0x00, false, 0x00}, //0x89	#	<control>
    {0x00, false, 0x00}, //0x8A	#	<control>
    {0x00, false, 0x00}, //0x8B	#	<control>
    {0x00, false, 0x00}, //0x8C	#	<control>
    {0x00, false, 0x00}, //0x8D	#	<control>
    {0x00, false, 0x00}, //0x8E	#	<control>
    {0x00, false, 0x00}, //0x8F	#	<control>
    {0x00, false, 0x00}, //0x90	#	<control>
    {0x00, false, 0x00}, //0x91	#	<control>
    {0x00, false, 0x00}, //0x92	#	<control>
    {0x00, false, 0x00}, //0x93	#	<control>
    {0x00, false, 0x00}, //0x94	#	<control>
    {0x00, false, 0x00}, //0x95	#	<control>
    {0x00, false, 0x00}, //0x96	#	<control>
    {0x00, false, 0x00}, //0x97	#	<control>
    {0x00, false, 0x00}, //0x98	#	<control>
    {0x00, false, 0x00}, //0x99	#	<control>
    {0x00, false, 0x00}, //0x9A	#	<control>
    {0x00, false, 0x00}, //0x9B	#	<control>
    {0x00, false, 0x00}, //0x9C	#	<control>
    {0x00, false, 0x00}, //0x9D	#	<control>
    {0x00, false, 0x00}, //0x9E	#	<control>
    {0x00, false, 0x00}, //0x9F	#	<control>
    {0x00, false, 0x00}, //0xA0	#	NO-BREAK SPACE
    {0x00, false, 0x00}, //0xA1	#	INVERTED EXCLAMATION MARK
    {0x00, false, 0x00}, //0xA2	#	CENT SIGN
    {0x00, false, 0x00}, //0xA3	#	POUND SIGN
    {0x00, false, 0x00}, //0xA4	#	CURRENCY SIGN
    {0x00, false, 0x00}, //0xA5	#	YEN SIGN
    {0x00, false, 0x00}, //0xA6	#	BROKEN BAR
    {0x00, false, 0x00}, //0xA7	#	SECTION SIGN
    {0x00, false, 0x00}, //0xA8	#	DIAERESIS
    {0x00, false, 0x00}, //0xA9	#	COPYRIGHT SIGN
    {0x00, false, 0x00}, //0xAA	#	FEMININE ORDINAL INDICATOR
    {0x00, false, 0x00}, //0xAB	#	LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
    {0x00, false, 0x00}, //0xAC	#	NOT SIGN
    {0x00, false, 0x00}, //0xAD	#	SOFT HYPHEN
    {0x00, false, 0x00}, //0xAE	#	REGISTERED SIGN
    {0x00, false, 0x00}, //0xAF	#	MACRON
    {0x00, false, 0x00}, //0xB0	#	DEGREE SIGN
    {0x00, false, 0x00}, //0xB1	#	PLUS-MINUS SIGN
    {0x00, false, 0x00}, //0xB2	#	SUPERSCRIPT TWO
    {0x00, false, 0x00}, //0xB3	#	SUPERSCRIPT THREE
    {0x00, false, 0x00}, //0xB4	#	ACUTE ACCENT
    {0x00, false, 0x00}, //0xB5	#	MICRO SIGN
    {0x00, false, 0x00}, //0xB6	#	PILCROW SIGN
    {0x00, false, 0x00}, //0xB7	#	MIDDLE DOT
    {0x00, false, 0x00}, //0xB8	#	CEDILLA
    {0x00, false, 0x00}, //0xB9	#	SUPERSCRIPT ONE
    {0x00, false, 0x00}, //0xBA	#	MASCULINE ORDINAL INDICATOR
    {0x00, false, 0x00}, //0xBB	#	RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
    {0x00, false, 0x00}, //0xBC	#	VULGAR FRACTION ONE QUARTER
    {0x00, false, 0x00}, //0xBD	#	VULGAR FRACTION ONE HALF
    {0x00, false, 0x00}, //0xBE	#	VULGAR FRACTION THREE QUARTERS
    {0x00, false, 0x00}, //0xBF	#	INVERTED QUESTION MARK
    {KEY_A, true, KEY_GRAVE}, //0xC0	#	LATIN CAPITAL LETTER A WITH GRAVE
    {KEY_A, true, KEY_E}, //0xC1	#	LATIN CAPITAL LETTER A WITH ACUTE
    {KEY_A, true, KEY_I}, //0xC2	#	LATIN CAPITAL LETTER A WITH CIRCUMFLEX
    {KEY_A, true, KEY_N}, //0xC3	#	LATIN CAPITAL LETTER A WITH TILDE
    {KEY_A, true, KEY_U}, //0xC4	#	LATIN CAPITAL LETTER A WITH DIAERESIS
    {0x00, false, 0x00}, //0xC5	#	LATIN CAPITAL LETTER A WITH RING ABOVE
    {0x00, false, 0x00}, //0xC6	#	LATIN CAPITAL LETTER AE
    {0x00, false, 0x00}, //0xC7	#	LATIN CAPITAL LETTER C WITH CEDILLA
    {KEY_E, true, KEY_GRAVE}, //0xC8	#	LATIN CAPITAL LETTER E WITH GRAVE
    {KEY_E, true, KEY_E}, //0xC9	#	LATIN CAPITAL LETTER E WITH ACUTE
    {KEY_E, true, KEY_I}, //0xCA	#	LATIN CAPITAL LETTER E WITH CIRCUMFLEX
    {KEY_E, true, KEY_U}, //0xCB	#	LATIN CAPITAL LETTER E WITH DIAERESIS
    {KEY_I, true, KEY_GRAVE}, //0xCC	#	LATIN CAPITAL LETTER I WITH GRAVE
    {KEY_I, true, KEY_E}, //0xCD	#	LATIN CAPITAL LETTER I WITH ACUTE
    {KEY_I, true, KEY_I}, //0xCE	#	LATIN CAPITAL LETTER I WITH CIRCUMFLEX
    {KEY_I, true, KEY_U}, //0xCF	#	LATIN CAPITAL LETTER I WITH DIAERESIS
    {0x00, false, 0x00}, //0xD0	#	LATIN CAPITAL LETTER ETH (Icelandic)
    {KEY_N, true, KEY_N}, //0xD1	#	LATIN CAPITAL LETTER N WITH TILDE
    {KEY_O, true, KEY_GRAVE}, //0xD2	#	LATIN CAPITAL LETTER O WITH GRAVE
    {KEY_O, true, KEY_E}, //0xD3	#	LATIN CAPITAL LETTER O WITH ACUTE
    {KEY_O, true, KEY_I}, //0xD4	#	LATIN CAPITAL LETTER O WITH CIRCUMFLEX
    {KEY_O, true, KEY_N}, //0xD5	#	LATIN CAPITAL LETTER O WITH TILDE
    {KEY_O, true, KEY_U}, //0xD6	#	LATIN CAPITAL LETTER O WITH DIAERESIS
    {0x00, false, 0x00}, //0xD7	#	MULTIPLICATION SIGN
    {0x00, false, 0x00}, //0xD8	#	LATIN CAPITAL LETTER O WITH STROKE
    {KEY_U, true, KEY_GRAVE}, //0xD9	#	LATIN CAPITAL LETTER U WITH GRAVE
    {KEY_U, true, KEY_E}, //0xDA	#	LATIN CAPITAL LETTER U WITH ACUTE
    {KEY_U, true, KEY_I}, //0xDB	#	LATIN CAPITAL LETTER U WITH CIRCUMFLEX
    {KEY_U, true, KEY_U}, //0xDC	#	LATIN CAPITAL LETTER U WITH DIAERESIS
    {KEY_Y, true, KEY_E}, //0xDD	#	LATIN CAPITAL LETTER Y WITH ACUTE
    {0x00, false, 0x00}, //0xDE	#	LATIN CAPITAL LETTER THORN (Icelandic)
    {0x00, false, 0x00}, //0xDF	#	LATIN SMALL LETTER SHARP S (German)
    {KEY_A, false, KEY_GRAVE}, //0xE0	#	LATIN SMALL LETTER A WITH GRAVE
    {KEY_A, false, KEY_E}, //0xE1	#	LATIN SMALL LETTER A WITH ACUTE
    {KEY_A, false, KEY_I}, //0xE2	#	LATIN SMALL LETTER A WITH CIRCUMFLEX
    {KEY_A, false, KEY_N}, //0xE3	#	LATIN SMALL LETTER A WITH TILDE
    {KEY_A, false, KEY_U}, //0xE4	#	LATIN SMALL LETTER A WITH DIAERESIS
    {0x00, false, 0x00}, //0xE5	#	LATIN SMALL LETTER A WITH RING ABOVE
    {0x00, false, 0x00}, //0xE6	#	LATIN SMALL LETTER AE
    {0x00, false, 0x00}, //0xE7	#	LATIN SMALL LETTER C WITH CEDILLA
    {KEY_E, false, KEY_GRAVE}, //0xE8	#	LATIN SMALL LETTER E WITH GRAVE
    {KEY_E, false, KEY_E}, //0xE9	#	LATIN SMALL LETTER E WITH ACUTE
    {KEY_E, false, KEY_I}, //0xEA	#	LATIN SMALL LETTER E WITH CIRCUMFLEX
    {KEY_E, false, KEY_U}, //0xEB	#	LATIN SMALL LETTER E WITH DIAERESIS
    {KEY_I, false, KEY_GRAVE}, //0xEC	#	LATIN SMALL LETTER I WITH GRAVE
    {KEY_I, false, KEY_E}, //0xED	#	LATIN SMALL LETTER I WITH ACUTE
    {KEY_I, false, KEY_I}, //0xEE	#	LATIN SMALL LETTER I WITH CIRCUMFLEX
    {KEY_I, false, KEY_U}, //0xEF	#	LATIN SMALL LETTER I WITH DIAERESIS
    {0x00, false, 0x00}, //0xF0	#	LATIN SMALL LETTER ETH (Icelandic)
    {KEY_N, false, KEY_N}, //0xF1	#	LATIN SMALL LETTER N WITH TILDE
    {KEY_O, false, KEY_GRAVE}, //0xF2	#	LATIN SMALL LETTER O WITH GRAVE
    {KEY_O, false, KEY_E}, //0xF3	#	LATIN SMALL LETTER O WITH ACUTE
    {KEY_O, false, KEY_I}, //0xF4	#	LATIN SMALL LETTER O WITH CIRCUMFLEX
    {KEY_O, false, KEY_N}, //0xF5	#	LATIN SMALL LETTER O WITH TILDE
    {KEY_O, false, KEY_U}, //0xF6	#	LATIN SMALL LETTER O WITH DIAERESIS
    {0x00, false, 0x00}, //0xF7	#	DIVISION SIGN
    {0x00, false, 0x00}, //0xF8	#	LATIN SMALL LETTER O WITH STROKE
    {KEY_U, false, KEY_GRAVE}, //0xF9	#	LATIN SMALL LETTER U WITH GRAVE
    {KEY_U, false, KEY_E}, //0xFA	#	LATIN SMALL LETTER U WITH ACUTE
    {KEY_U, false, KEY_I}, //0xFB	#	LATIN SMALL LETTER U WITH CIRCUMFLEX
    {KEY_U, false, KEY_U}, //0xFC	#	LATIN SMALL LETTER U WITH DIAERESIS
    {KEY_Y, false, KEY_E}, //0xFD	#	LATIN SMALL LETTER Y WITH ACUTE
    {0x00, false, 0x00}, //0xFE	#	LATIN SMALL LETTER THORN (Icelandic)
    {KEY_Y, false, KEY_U} //0xFF	#	LATIN SMALL LETTER Y WITH DIAERESIS
};

#define UIBC_KEYCODE_UNKNOWN KEY_UNKNOWN

UibcMessage::UibcMessage(UibcMessage::MessageType type,
                         const char* inEventDesc,
                         double widthRatio,
                         double heightRatio)
    : m_PacketData(NULL),
      m_PacketDataLen(0),
      m_DataValid(false) {

    switch (type) {
    case GENERIC_TOUCH_DOWN:
    case GENERIC_TOUCH_UP:
    case GENERIC_TOUCH_MOVE:
        m_PacketDataLen = getUIBCGenericTouchPacket(inEventDesc,
                          &m_PacketData,
                          widthRatio,
                          heightRatio);
        break;

    case GENERIC_KEY_DOWN:
    case GENERIC_KEY_UP:
        m_PacketDataLen = getUIBCGenericKeyPacket(inEventDesc, &m_PacketData);
        break;

    case GENERIC_ZOOM:
        m_PacketDataLen = getUIBCGenericZoomPacket(inEventDesc, &m_PacketData);
        break;

    case GENERIC_VERTICAL_SCROLL:
    case GENERIC_HORIZONTAL_SCROLL:
        m_PacketDataLen = getUIBCGenericScalePacket(inEventDesc, &m_PacketData);
        break;

    case GENERIC_ROTATE:
        m_PacketDataLen = getUIBCGenericRotatePacket(inEventDesc, &m_PacketData);
        break;
    };
}

UibcMessage::~UibcMessage() {
    if (m_PacketData != NULL) {
        free(m_PacketData);
        m_PacketData = NULL;
        m_DataValid = false;
    }
}

status_t UibcMessage::init() {
    int version;
    return OK;
}

status_t UibcMessage::destroy() {

    return OK;
}

char* UibcMessage::getPacketData() {
    return m_PacketData;
}

int UibcMessage::getPacketDataLen() {
    return m_PacketDataLen;
}

bool UibcMessage::isDataValid() {
    return m_DataValid;
}


// format: "typeId, number of pointers, pointer Id1, X coordnate, Y coordnate, , pointer Id2, X coordnate, Y coordnate,..."
int32_t UibcMessage::getUIBCGenericTouchPacket(const char *inEventDesc,
        char** outData,
        double widthRatio,
        double heightRatio) {
    ALOGD("getUIBCGenericTouchPacket (%s)", inEventDesc);
    int32_t typeId = 0, numberOfPointers;
    int32_t uibcBodyLen = 0, genericPacketLen;
    int32_t pointerCounter, eventField;
    int32_t temp;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericTouchPacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericTouchPacket typeId=[%d]\n", typeId);
                break;
            }
            case 1: {
                numberOfPointers = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericTouchPacket numberOfPointers=[%d]\n", numberOfPointers);
                genericPacketLen = numberOfPointers * 5 + 1;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                (*outData)[7] = numberOfPointers & 0xFF; // Number of pointers, 1 octet
                break;
            }
            default: {
                pointerCounter = (i - 2) / 3; // (curIndex - 2 fields, id+numPointers) /  (3 fields, id+x+y)
                eventField = (i - 2) % 3; // 0 = id, 1 = x, 2 = y
                temp = atoi(*(splitedStr + i));
                switch (eventField) {
                case 0: { // Pointer ID, 1 octet
                    //ALOGD("getUIBCGenericTouchPacket Pointer ID=[%d]\n", temp);
                    (*outData)[8 + pointerCounter * 5 + eventField] = temp & 0xFF;
                    break;
                }
                case 1: { // X-coordinate, 2 octets
                    temp = (int32_t)((double)temp * widthRatio);
                    //ALOGD("getUIBCGenericTouchPacket X-coordinate=[%d]\n", temp);
                    (*outData)[8 + pointerCounter * 5 + eventField] = (temp >> 8) & 0xFF;
                    (*outData)[8 + pointerCounter * 5 + eventField + 1] = temp & 0xFF;
                    break;
                }
                case 2: { // Y-coordinate, 2 octets
                    temp = (int32_t)((double)temp * heightRatio);
                    //ALOGD("getUIBCGenericTouchPacket Y-coordinate=[%d]\n", temp);
                    (*outData)[8 + pointerCounter * 5 + eventField + 1] = (temp >> 8) & 0xFF;
                    (*outData)[8 + pointerCounter * 5 + eventField + 2] = temp & 0xFF;
                    break;
                }
                }
                break;
            }
            }

            free(*(splitedStr + i));
        }
        free(splitedStr);
    }
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}

// format: "typeId, Key code 1(0x00), Key code 2(0x00)"
int32_t UibcMessage::getUIBCGenericKeyPacket(const char *inEventDesc,
        char** outData) {
    ALOGD("getUIBCGenericKeyPacket (%s)", inEventDesc);
    int32_t typeId;
    int32_t uibcBodyLen, genericPacketLen;
    int32_t temp;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericKeyPacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericKeyPacket typeId=[%d]\n", typeId);
                genericPacketLen = 5;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                (*outData)[7] = 0x00; // resvered
                break;
            }
            case 1: {
                sscanf(*(splitedStr + i), " 0x%04X", &temp);
                if (temp == 0) {
                    uibcBodyLen = 0;
                    (*outData)[8] = 0x00;
                    (*outData)[9] = 0x00;
                }
                //ALOGD("getUIBCGenericKeyPacket key code 1=[%d]\n", temp);
                (*outData)[8] = (temp >> 8) & 0xFF;
                (*outData)[9] = temp & 0xFF;

                break;
            }
            case 2: {
                sscanf(*(splitedStr + i), " 0x%04X", &temp);
                if (temp == 0) {
                    (*outData)[10] = 0x00;
                    (*outData)[11] = 0x00;
                }
                //ALOGD("getUIBCGenericKeyPacket key code 2=[%d]\n", temp);
                (*outData)[10] = (temp >> 8) & 0xFF;
                (*outData)[11] = temp & 0xFF;

                break;
            }
            default: {
            }
            break;
            }
        }

        free(*(splitedStr + i));
    }
    free(splitedStr);
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}

// format: "typeId,  X coordnate, Y coordnate, integer part, fraction part"
int32_t UibcMessage::getUIBCGenericZoomPacket(const char *inEventDesc, char** outData) {
    ALOGD("getUIBCGenericZoomPacket (%s)", inEventDesc);
    int32_t typeId;
    int32_t uibcBodyLen, genericPacketLen;
    int32_t eventField;
    int32_t xCoord, yCoord, integerPart, FractionPart;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericZoomPacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericZoomPacket typeId=[%d]\n", typeId);

                genericPacketLen = 6;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                break;
            }

            case 1: {
                xCoord = atoi(*(splitedStr + i));
                (*outData)[7] = (xCoord >> 8) & 0xFF;
                (*outData)[8] = xCoord & 0xFF;
                //ALOGD("getUIBCGenericZoomPacket xCoord=[%d]\n", xCoord);
                break;
            }
            case 2: {
                yCoord = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericZoomPacket yCoord=[%d]\n", yCoord);
                break;
            }
            case 3: {
                integerPart = atoi(*(splitedStr + i));
                (*outData)[11] = integerPart & 0xFF;
                //ALOGD("getUIBCGenericZoomPacket integerPart=[%d]\n", integerPart);
                break;
            }
            case 4: {
                FractionPart = atoi(*(splitedStr + i));
                (*outData)[12] = FractionPart & 0xFF;
                //ALOGD("getUIBCGenericZoomPacket FractionPart=[%d]\n", FractionPart);

                break;
            }
            default: {
                break;
            }
            }

            free(*(splitedStr + i));
        }
        free(splitedStr);
    }
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}

// format: "typeId,  unit, direction, amount to scroll"
int32_t UibcMessage::getUIBCGenericScalePacket(const char *inEventDesc, char** outData) {
    ALOGD("getUIBCGenericScalePacket (%s)", inEventDesc);
    int32_t typeId;
    int32_t uibcBodyLen, genericPacketLen;
    int32_t temp;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericScalePacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericScalePacket typeId=[%d]\n", typeId);
                genericPacketLen = 2;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                (*outData)[7] = 0x00; // Clear the byte
                (*outData)[8] = 0x00; // Clear the byte
                /*
                B15B14; Scroll Unit Indication bits.
                0b00; the unit is a pixel (normalized with respect to the WFD Source display resolution that is conveyed in an RTSP M4 request message).
                0b01; the unit is a mouse notch (where the application is responsible for representing the number of pixels per notch).
                0b10-0b11; Reserved.

                B13; Scroll Direction Indication bit.
                0b0; Scrolling to the right. Scrolling to the right means the displayed content being shifted to the left from a user perspective.
                0b1; Scrolling to the left. Scrolling to the left means the displayed content being shifted to the right from a user perspective.

                B12:B0; Number of Scroll bits.
                Number of units for a Horizontal scroll.
                */
                break;
            }
            case 1: {
                temp = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericScalePacket unit=[%d]\n", temp);
                (*outData)[7] = (temp >> 8) & 0xFF;
                break;
            }
            case 2: {
                temp = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericScalePacket direction=[%d]\n", temp);
                (*outData)[7] |= ((temp >> 10) & 0xFF);
                break;

            }
            case 3: {
                temp = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericScalePacket amount to scroll=[%d]\n", temp);
                (*outData)[7] |= ((temp >> 12) & 0xFF);
                (*outData)[8] = temp & 0xFF;

                break;
            }
            default: {
                break;
            }
            }

            free(*(splitedStr + i));
        }

        free(splitedStr);
    }
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}

// format: "typeId,  integer part, fraction part"
int32_t UibcMessage::getUIBCGenericRotatePacket(const char * inEventDesc, char** outData) {
    ALOGD("getUIBCGenericRotatePacket (%s)", inEventDesc);
    int32_t typeId;
    int32_t uibcBodyLen, genericPacketLen;
    int32_t temp;
    int32_t integerPart, FractionPart;

    char** splitedStr = UibcMessage::str_split((char*)inEventDesc, ',');

    if (splitedStr) {
        int i;
        for (i = 0; * (splitedStr + i); i++) {
            //ALOGD("getUIBCGenericRotatePacket splitedStr tokens=[%s]\n", *(splitedStr + i));

            switch (i) {
            case 0: {
                typeId = atoi(*(splitedStr + i));
                //ALOGD("getUIBCGenericRotatePacket typeId=[%d]\n", typeId);
                genericPacketLen = 2;
                uibcBodyLen = genericPacketLen + 7; // Generic herder leh = 7
                (*outData) = (char*)malloc(uibcBodyLen + 1);
                // UIBC header
                (*outData)[0] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[1] = 0x00; //Version (3 bits),T (1 bit),Reserved(8 bits),Input Category (4 bits)
                (*outData)[2] = (uibcBodyLen >> 8) & 0xFF; //Length(16 bits)
                (*outData)[3] = uibcBodyLen & 0xFF; //Length(16 bits)
                //Generic Input Body Format
                (*outData)[4] = typeId & 0xFF; // Tyoe ID, 1 octet
                (*outData)[5] = (genericPacketLen >> 8) & 0xFF; // Length, 2 octets
                (*outData)[6] = genericPacketLen & 0xFF; // Length, 2 octets
                break;
            }
            case 1: {
                integerPart = atoi(*(splitedStr + i));
                (*outData)[7] = integerPart & 0xFF;
                //ALOGD("getUIBCGenericRotatePacket integerPart=[%d]\n", integerPart);
                break;
            }
            case 2: {
                FractionPart = atoi(*(splitedStr + i));
                (*outData)[8] = FractionPart & 0xFF;
                //ALOGD("getUIBCGenericRotatePacket FractionPart=[%d]\n", FractionPart);

                break;
            }
            default: {
                break;
            }
            }
            free(*(splitedStr + i));
        }

        free(splitedStr);
    }
    hexdump((*outData), uibcBodyLen);
    m_DataValid = true;
    return uibcBodyLen;
}


//static
scanCodeBuild_t UibcMessage::asciiToScancodeBuild(UINT16 asciiCode) {
    scanCodeBuild_t ret = scanCode_DefaultMap[0];

    ALOGD("asciiCode: %d", asciiCode);

    ret = scanCode_DefaultMap[asciiCode];

    ALOGD("scanCode: %d", ret.scanCode);
    return ret;
}

//static
short UibcMessage::scancodeToAcsii(UINT8 scanCode) {
    short ret = UIBC_KEYCODE_UNKNOWN;

    ALOGD("scanCode : %d", scanCode);

    for (unsigned int i = 0; i < (sizeof(scanCode_DefaultMap) / sizeof(scanCode_DefaultMap[0])); i++) {
        if (scanCode == scanCode_DefaultMap[i].scanCode) {
            ret = i;
            break;
        }
    }

    ALOGD("asciiCode: %d", ret);
    return ret;
}

//static
void UibcMessage::getScreenResolution(int* x, int* y) {
    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
                              ISurfaceComposer::eDisplayIdMain);
    DisplayInfo info;
    SurfaceComposerClient::getDisplayInfo(display, &info);
    *x = info.w;
    *y = info.h;
}

char** UibcMessage::str_split(char * a_str, const char a_delim) {
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = (char**)malloc(sizeof(char*) * count);

    if (result) {
        size_t idx  = 0;
        char* token = strtok(a_str, ",");

        while (token) {
            CHECK_LT(idx , count);
            *(result + idx++) = strdup(token);
            token = strtok(0, ",");
        }
        CHECK_EQ(idx , count - 1);
        *(result + idx) = 0;
    }

    return result;
}

}
