#ifndef FTM_CUST_LCD_H
#define FTM_CUST_LCD_H

#define CUST_LCD_AVAIL_WIDTH      720
#define CUST_LCD_AVAIL_HEIGHT     1280

#define DEFINE_TEST_IMAGE_FILENAMES(x)      \
const char *x[] = {                         \
    "lcd_test_00",                          \
    "lcd_test_01",                          \
    "lcd_test_02",                          \
    "lcd_test_03",                          \
    "lcd_test_04",                          \
    "lcd_test_05",                          \
    "lcd_test_06",                          \
    "lcd_test_07",                          \
}


//LINE<JIRA_ID><DATE20140214><wallpaper check for ms color>zenghaihui
#ifdef TINNO_MS_COLOR_SELECT
#define DEFINE_PHONE_COLOR_IMAGE_FILENAMES(y)      \
static const char *y[] = {                         \
    "default_wallpaper_01",                          \
    "default_wallpaper_02",                          \
    "default_wallpaper_03",                          \
    "default_wallpaper_04",                          \
    "default_wallpaper_05",                          \
    "default_wallpaper_06",                          \
    "default_wallpaper_error",                          \
}
#endif /* TINNO_MS_COLOR_SELECT */

#endif /* FTM_CUST_LCD_H */
