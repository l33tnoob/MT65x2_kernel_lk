#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>
#include <linux/slab.h>
#include <linux/xlog.h>

#include <linux/device.h>
#include <linux/errno.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <linux/input/mt.h>
#include <linux/version.h>

#define UIBC_TAG	"UIBC:"
#define MAX_POINTERS 5

#define idVal(_x) (_x * 3 + 1)
#define xVal(_x) (_x * 3 + 2)
#define yVal(_x) (_x * 3 + 3)

static unsigned short uibc_keycode[256] = {
    KEY_RESERVED,
    BTN_LEFT,
    KEY_BACK,
    BTN_TOUCH,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_ENTER,
    KEY_BACK,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_SPACE,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_LEFTBRACE,
    KEY_RIGHTBRACE,
    KEY_BACKSLASH,
    KEY_BACKSLASH,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_GRAVE,
    KEY_COMMA,
    KEY_DOT,
    KEY_SLASH,
    KEY_CAPSLOCK,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_SYSRQ,
    KEY_SCROLLLOCK,
    KEY_PAUSE,
    KEY_INSERT,
    KEY_HOME,
    KEY_PAGEUP,
    KEY_DELETE,
    KEY_END,
    KEY_PAGEDOWN,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_NUMLOCK,
    KEY_KPSLASH,
    KEY_KPASTERISK,
    KEY_KPMINUS,
    KEY_KPPLUS,
    KEY_KPENTER,
    KEY_KP1,
    KEY_KP2,
    KEY_KP3,
    KEY_KP4,
    KEY_KP5,
    KEY_KP6,
    KEY_KP7,
    KEY_KP8,
    KEY_KP9,
    KEY_KP0,
    KEY_KPDOT,
    KEY_102ND,
    KEY_COMPOSE,
    KEY_POWER,
    KEY_KPEQUAL,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    KEY_F16,
    KEY_F17,
    KEY_F18,
    KEY_F19,
    KEY_F20,
    KEY_F21,
    KEY_F22,
    KEY_F23,
    KEY_F24,
    KEY_OPEN,
    KEY_HELP,
    KEY_PROPS,
    KEY_FRONT,
    KEY_STOP,
    KEY_AGAIN,
    KEY_UNDO,
    KEY_CUT,
    KEY_COPY,
    KEY_PASTE,
    KEY_FIND,
    KEY_MUTE,
    KEY_VOLUMEUP,
    KEY_VOLUMEDOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_KPCOMMA,
    KEY_UNKNOWN,
    KEY_RO,
    KEY_KATAKANAHIRAGANA,
    KEY_YEN,
    KEY_HENKAN,
    KEY_MUHENKAN,
    KEY_KPJPCOMMA,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_HANGEUL,
    KEY_HANJA,
    KEY_KATAKANA,
    KEY_HIRAGANA,
    KEY_ZENKAKUHANKAKU,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    BTN_A,
    BTN_B,
    BTN_C,
    BTN_X,
    BTN_Y,
    BTN_Z,
    BTN_TL,
    BTN_TR,
    BTN_TL2,
    BTN_TR2,
    BTN_SELECT,
    BTN_START,
    BTN_MODE,
    BTN_THUMBL,
    BTN_THUMBR,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_KPLEFTPAREN,
    KEY_KPRIGHTPAREN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_UNKNOWN,
    KEY_MENU,
    KEY_ZOOMRESET,
    KEY_ZOOMIN,
    KEY_ZOOMOUT,
    KEY_COFFEE,
    KEY_MESSENGER,
    KEY_IMAGES,
    KEY_AUDIO,
    KEY_VIDEO,
    KEY_LEFTCTRL,
    KEY_LEFTSHIFT,
    KEY_LEFTALT,
    KEY_LEFTMETA,
    KEY_RIGHTCTRL,
    KEY_RIGHTSHIFT,
    KEY_RIGHTALT,
    KEY_RIGHTMETA,
    KEY_PLAYPAUSE,
    KEY_STOPCD,
    KEY_PREVIOUSSONG,
    KEY_NEXTSONG,
    KEY_EJECTCD,
    KEY_VOLUMEUP,
    KEY_VOLUMEDOWN,
    KEY_MUTE,
    KEY_WWW,
    KEY_BACK,
    KEY_FORWARD,
    KEY_STOP,
    KEY_SEARCH,
    KEY_SCROLLUP,
    KEY_SCROLLDOWN,
    KEY_EDIT,
    KEY_SLEEP,
    KEY_COFFEE,
    KEY_REFRESH,
    KEY_CALC,
    KEY_HOMEPAGE,
    KEY_MAIL,
    KEY_CONFIG,
    KEY_UNKNOWN,
};

#define UIBC_KBD_NAME  "uibc"
#define UIBC_KEY_PRESS 1
#define UIBC_KEY_RELEASE 0
#define UIBC_KEY_RESERVE	2
#define UIBC_POINTER_X	3
#define UIBC_POINTER_Y	4
#define UIBC_KEYBOARD	5
#define UIBC_MOUSE	6
#define UIBC_TOUCH_DOWN		7
#define UIBC_TOUCH_UP		8
#define UIBC_TOUCH_MOVE		9

#define UIBC_MOUSE_INDEX	0
#define UIBC_GAMEPAD_INDEX	151


static struct input_dev *uibc_input_dev;

struct uibckeyboard {
    struct input_dev *input;
    unsigned short keymap[ARRAY_SIZE(uibc_keycode)];
};

struct uibckeyboard *uibckbd;
int uibc_registered = 0;

static long uibc_kbd_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    void __user *uarg = (void __user *)arg;
    static short XValue = 0;
    static short YValue = 0;
    short keycode;
    short touchPosition[16];
    int err, i;

    xlog_printk(ANDROID_LOG_INFO, UIBC_TAG, "uibc_kbd_dev_ioctl,cmd=%d\n", cmd);
    switch(cmd) {
    case UIBC_KEYBOARD: {
        uibc_input_dev->keycodemax = ARRAY_SIZE(uibc_keycode);
        for (i = 0; i < ARRAY_SIZE(uibckbd->keymap); i++)
            __set_bit(uibckbd->keymap[i], uibc_input_dev->keybit);
        err = input_register_device(uibc_input_dev);
        if (err) {
            xlog_printk(ANDROID_LOG_ERROR, UIBC_TAG, "register input device failed (%d)\n", err);
            input_free_device(uibc_input_dev);
            return err;
        }
        uibc_registered = 1;
        break;
    }
    case UIBC_MOUSE: {
        __set_bit(BTN_LEFT, uibc_input_dev->keybit);
        __set_bit(BTN_TOUCH, uibc_input_dev->keybit);
        err = input_register_device(uibc_input_dev);
        if (err) {
            xlog_printk(ANDROID_LOG_ERROR, UIBC_TAG, "register input device failed (%d)\n", err);
            input_free_device(uibc_input_dev);
            return err;
        }
        uibc_registered = 1;
        break;
    }
    case UIBC_KEY_PRESS: {
        if (copy_from_user(&keycode, uarg, sizeof(keycode)))
            return -EFAULT;
        xlog_printk(ANDROID_LOG_DEBUG, UIBC_TAG, "uibc keycode %d \n", keycode);
        input_report_key(uibc_input_dev, keycode, 1);
        input_sync(uibc_input_dev);
        break;
    }
    case UIBC_KEY_RELEASE: {
        if (copy_from_user(&keycode, uarg, sizeof(keycode)))
            return -EFAULT;
        input_report_key(uibc_input_dev, keycode, 0);
        input_sync(uibc_input_dev);
        break;
    }
    case UIBC_POINTER_X: {
        if (copy_from_user(&XValue, uarg, sizeof(XValue)))
            return -EFAULT;
        xlog_printk(ANDROID_LOG_DEBUG, UIBC_TAG, "uibc pointer X %d \n", XValue);
        break;
    }
    case UIBC_POINTER_Y: {
        if (copy_from_user(&YValue, uarg, sizeof(YValue)))
            return -EFAULT;
        xlog_printk(ANDROID_LOG_DEBUG, UIBC_TAG, "uibc pointer Y %d \n", YValue);
        input_report_rel(uibc_input_dev, REL_X, XValue);
        input_report_rel(uibc_input_dev, REL_Y, YValue);
        input_sync(uibc_input_dev);
        XValue = 0;
        YValue = 0;
        break;
    }
    case UIBC_TOUCH_DOWN: {
        if (copy_from_user(&touchPosition, uarg, sizeof(touchPosition)))
            return -EFAULT;
        xlog_printk(ANDROID_LOG_DEBUG, UIBC_TAG, "uibc UIBC_TOUCH_DOWN id=%d,(%d,%d)\n",
                    touchPosition[idVal(0)],
                    touchPosition[xVal(0)],
                    touchPosition[yVal(0)]);
        input_report_key(uibc_input_dev,
                         BTN_TOUCH, 1);
        input_report_abs(uibc_input_dev,
                         ABS_MT_TRACKING_ID,
                         touchPosition[idVal(0)]);
        input_report_abs(uibc_input_dev,
                         ABS_MT_POSITION_X,
                         touchPosition[xVal(0)]);
        input_report_abs(uibc_input_dev,
                         ABS_MT_POSITION_Y,
                         touchPosition[yVal(0)]);
        input_mt_sync(uibc_input_dev);
        input_sync(uibc_input_dev);
        break;
    }
    case UIBC_TOUCH_UP: {
        if (copy_from_user(&touchPosition, uarg, sizeof(touchPosition)))
            return -EFAULT;
        xlog_printk(ANDROID_LOG_DEBUG, UIBC_TAG, "uibc UIBC_TOUCH_UP");
        input_report_key(uibc_input_dev, BTN_TOUCH, 0);
        input_sync(uibc_input_dev);
        break;
    }
    case UIBC_TOUCH_MOVE: {
        if (copy_from_user(&touchPosition, uarg, sizeof(touchPosition)))
            return -EFAULT;
        for (i = 0; i < MAX_POINTERS; i++) {
            if (touchPosition[xVal(i)] == 0 && touchPosition[yVal(i)] == 0)
                continue;
            input_report_abs(uibc_input_dev,
                             ABS_MT_TRACKING_ID,
                             touchPosition[idVal(i)]);
            input_report_abs(uibc_input_dev,
                             ABS_MT_POSITION_X,
                             touchPosition[xVal(i)]);
            input_report_abs(uibc_input_dev,
                             ABS_MT_POSITION_Y,
                             touchPosition[yVal(i)]);
            input_mt_sync(uibc_input_dev);
        }
        input_sync(uibc_input_dev);
        break;
    }
    default:
        return -EINVAL;
    }
    return 0;
}

static int uibc_kbd_dev_open(struct inode *inode, struct file *file) {
    int TPD_RES_X, TPD_RES_Y;

    xlog_printk(ANDROID_LOG_INFO, UIBC_TAG, "*** uibckeyboard uibc_kbd_dev_open ***\n");

    TPD_RES_X = simple_strtoul(LCM_WIDTH, NULL, 0);
    TPD_RES_Y = simple_strtoul(LCM_HEIGHT, NULL, 0);

    uibckbd = kzalloc(sizeof(struct uibckeyboard), GFP_KERNEL);
    uibc_input_dev = input_allocate_device();
    if (!uibckbd || !uibc_input_dev)
        goto fail;

    memcpy(uibckbd->keymap, uibc_keycode,
           sizeof(uibc_keycode));
    uibckbd->input = uibc_input_dev;

    set_bit(INPUT_PROP_DIRECT, uibc_input_dev->propbit);

    set_bit(EV_ABS, uibc_input_dev->evbit);
    set_bit(EV_KEY, uibc_input_dev->evbit);
    set_bit(EV_REL, uibc_input_dev->evbit);

    set_bit(REL_X, uibc_input_dev->relbit);
    set_bit(REL_Y, uibc_input_dev->relbit);

    set_bit(ABS_X, uibc_input_dev->absbit);
    set_bit(ABS_Y, uibc_input_dev->absbit);
    set_bit(ABS_MT_TRACKING_ID, uibc_input_dev->absbit);
    set_bit(ABS_MT_POSITION_X, uibc_input_dev->absbit);
    set_bit(ABS_MT_POSITION_Y, uibc_input_dev->absbit);

    input_set_abs_params(uibc_input_dev, ABS_MT_POSITION_X, 0, TPD_RES_X, 0, 0);
    input_set_abs_params(uibc_input_dev, ABS_MT_POSITION_Y, 0, TPD_RES_Y, 0, 0);
    input_set_abs_params(uibc_input_dev, ABS_X, 0, TPD_RES_X, 0, 0);
    input_set_abs_params(uibc_input_dev, ABS_Y, 0, TPD_RES_Y, 0, 0);

    input_abs_set_res(uibc_input_dev, ABS_X, TPD_RES_X);
    input_abs_set_res(uibc_input_dev, ABS_Y, TPD_RES_Y);

    uibc_input_dev->name = UIBC_KBD_NAME;
    uibc_input_dev->keycode = uibckbd->keymap;
    uibc_input_dev->keycodesize = sizeof(unsigned short);
    uibc_input_dev->id.bustype = BUS_HOST;

    return 0;
fail:
    input_free_device(uibc_input_dev);
    kfree(uibckbd);

    return -EINVAL;
}

static int uibc_kbd_dev_release(struct inode *inode, struct file *file) {
    xlog_printk(ANDROID_LOG_INFO, UIBC_TAG, "*** uibckeyboard uibc_kbd_dev_release ***\n");
    if(uibc_registered == 1) {
        input_unregister_device(uibc_input_dev);
        uibc_registered = 0;
    }
    return 0;
}


static struct file_operations uibc_kbd_dev_fops = {
    .owner		= THIS_MODULE,
    .unlocked_ioctl	= uibc_kbd_dev_ioctl,
    .open		= uibc_kbd_dev_open,
    .release		= uibc_kbd_dev_release
};

static struct miscdevice uibc_kbd_dev = {
    .minor	= MISC_DYNAMIC_MINOR,
    .name	= UIBC_KBD_NAME,
    .fops	= &uibc_kbd_dev_fops,
};


static int uibc_keyboard_probe(struct platform_device *pdev) {

    int i, err;

    xlog_printk(ANDROID_LOG_INFO, UIBC_TAG, "*** uibckeyboard probe ***\n");

    uibckbd = kzalloc(sizeof(struct uibckeyboard), GFP_KERNEL);
    uibc_input_dev = input_allocate_device();
    if (!uibckbd || !uibc_input_dev)
        goto fail;

    memcpy(uibckbd->keymap, uibc_keycode,
           sizeof(uibc_keycode));
    uibckbd->input = uibc_input_dev;
    __set_bit(EV_KEY, uibc_input_dev->evbit);
    platform_set_drvdata(pdev, uibckbd);

    uibc_input_dev->name = UIBC_KBD_NAME;
    uibc_input_dev->keycode = uibckbd->keymap;
    uibc_input_dev->keycodesize = sizeof(unsigned short);
    uibc_input_dev->keycodemax = ARRAY_SIZE(uibc_keycode);
    uibc_input_dev->id.bustype = BUS_HOST;
    uibc_input_dev->dev.parent = &pdev->dev;


    for (i = 0; i < ARRAY_SIZE(uibckbd->keymap); i++)
        __set_bit(uibckbd->keymap[i], uibc_input_dev->keybit);

    input_set_capability(uibc_input_dev, EV_MSC, MSC_SCAN);

    uibc_kbd_dev.parent = &pdev->dev;
    err = misc_register(&uibc_kbd_dev);
    if (err) {
        xlog_printk(ANDROID_LOG_ERROR, UIBC_TAG, "register device failed (%d)\n", err);
        return err;
    }

    return 0;

fail:
    platform_set_drvdata(pdev, NULL);
    input_free_device(uibc_input_dev);
    kfree(uibckbd);

    return -EINVAL;
}

static struct platform_driver uibc_keyboard_driver = {
    .probe = uibc_keyboard_probe,
    .driver = {
        .name = UIBC_KBD_NAME,
    },
};

static int uibc_keyboard_init(void) {
    xlog_printk(ANDROID_LOG_INFO, UIBC_TAG, "uibc_keyboard_init OK\n");

    return platform_driver_register(&uibc_keyboard_driver);
}


static void __exit uibc_keyboard_exit(void) {
}

module_init(uibc_keyboard_init);
module_exit(uibc_keyboard_exit);

MODULE_DESCRIPTION("hid keyboard Device");
MODULE_LICENSE("GPL");

