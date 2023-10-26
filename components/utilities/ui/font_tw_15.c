/*******************************************************************************
 * Size: 15 px
 * Bpp: 2
 * Opts: 
 ******************************************************************************/

#include "lvgl.h"

#ifndef FONT_TW_15
#define FONT_TW_15 1
#endif

#if FONT_TW_15

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0028 "(" */
    0x1, 0x1, 0x80, 0xc0, 0xa0, 0x30, 0x1c, 0xb,
    0x2, 0xc0, 0xb0, 0x1c, 0x3, 0x40, 0xa0, 0xc,
    0x1, 0x80, 0x10,

    /* U+0029 ")" */
    0x10, 0x9, 0x0, 0xc0, 0x28, 0x7, 0x0, 0xd0,
    0x38, 0xe, 0x3, 0x80, 0xd0, 0x70, 0x28, 0xc,
    0x9, 0x1, 0x0,

    /* U+0041 "A" */
    0x0, 0x74, 0x0, 0x0, 0xb8, 0x0, 0x0, 0xbc,
    0x0, 0x1, 0x6d, 0x0, 0x2, 0x1e, 0x0, 0x2,
    0xf, 0x0, 0x5, 0xb, 0x40, 0xa, 0xab, 0xc0,
    0xc, 0x3, 0xc0, 0x8, 0x2, 0xd0, 0x79, 0x7,
    0xf8,

    /* U+0043 "C" */
    0x0, 0xaa, 0x90, 0xb8, 0xa, 0x1e, 0x0, 0x63,
    0xd0, 0x0, 0x3c, 0x0, 0x3, 0xc0, 0x0, 0x3c,
    0x0, 0x3, 0xc0, 0x0, 0x1e, 0x0, 0x30, 0xb4,
    0x7, 0x0, 0xaa, 0x90,

    /* U+0054 "T" */
    0xba, 0xfa, 0xb2, 0x83, 0xd0, 0xc9, 0xf, 0x3,
    0x0, 0x3c, 0x0, 0x0, 0xf0, 0x0, 0x3, 0xc0,
    0x0, 0xf, 0x0, 0x0, 0x3c, 0x0, 0x0, 0xf0,
    0x0, 0x3, 0xd0, 0x0, 0x6f, 0x90, 0x0,

    /* U+0061 "a" */
    0x9, 0xb4, 0xf, 0xf, 0x2, 0x42, 0xc0, 0x5,
    0xf0, 0x1c, 0x2c, 0xe, 0xb, 0x7, 0xc3, 0xd0,
    0xbd, 0x3c,

    /* U+0065 "e" */
    0x6, 0xb8, 0x1c, 0xe, 0x3c, 0xf, 0x7e, 0xae,
    0x7c, 0x0, 0x3c, 0x0, 0x2f, 0x1, 0x7, 0xf4,

    /* U+006D "m" */
    0x1a, 0x7e, 0x1f, 0x80, 0xbd, 0x2e, 0x4b, 0x41,
    0xe0, 0x78, 0x1e, 0x7, 0x81, 0xe0, 0x78, 0x1e,
    0x7, 0x81, 0xe0, 0x78, 0x1e, 0x7, 0x81, 0xe0,
    0x78, 0x1e, 0xb, 0xd2, 0xf4, 0xbd,

    /* U+006E "n" */
    0x1a, 0x3e, 0x2, 0xf4, 0xb8, 0x1e, 0x3, 0x81,
    0xe0, 0x3c, 0x1e, 0x3, 0xc1, 0xe0, 0x3c, 0x1e,
    0x3, 0xc2, 0xf4, 0xbd,

    /* U+0070 "p" */
    0x1a, 0x7e, 0x2, 0xf0, 0x78, 0x1e, 0x3, 0xc1,
    0xe0, 0x2d, 0x1e, 0x2, 0xd1, 0xe0, 0x3c, 0x1f,
    0xb, 0x81, 0xeb, 0xe0, 0x1e, 0x0, 0x1, 0xe0,
    0x0, 0x1e, 0x0, 0x2, 0xf8, 0x0,

    /* U+0072 "r" */
    0x1a, 0x7c, 0x2e, 0xac, 0x1f, 0x0, 0x1e, 0x0,
    0x1e, 0x0, 0x1e, 0x0, 0x1e, 0x0, 0x2f, 0x80,

    /* U+0074 "t" */
    0x5, 0x0, 0xe0, 0x1d, 0x7, 0xf9, 0x2d, 0x2,
    0xd0, 0x2d, 0x2, 0xd0, 0x2d, 0x1, 0xe0, 0xb,
    0xc0,

    /* U+0075 "u" */
    0x0, 0x0, 0x7, 0xe0, 0xb8, 0x1d, 0x7, 0x81,
    0xd0, 0x78, 0x2d, 0x7, 0x82, 0xd0, 0x78, 0x1d,
    0x7, 0x81, 0xf5, 0xb8, 0xb, 0xd7, 0xd0,

    /* U+2103 "℃" */
    0x18, 0x0, 0x19, 0x24, 0x80, 0xb4, 0xb8, 0x8,
    0x74, 0x1e, 0x5, 0x3c, 0x2, 0x2a, 0x1e, 0x0,
    0x0, 0x7, 0x80, 0x0, 0x1, 0xe0, 0x0, 0x0,
    0x78, 0x0, 0x0, 0xf, 0x0, 0x40, 0x2, 0xc0,
    0x30, 0x0, 0x38, 0x1c, 0x0, 0x2, 0xa9, 0x0,
    0x0, 0x0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 60, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 96, .box_w = 5, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 19, .adv_w = 96, .box_w = 5, .box_h = 15, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 38, .adv_w = 179, .box_w = 12, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 168, .box_w = 10, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 166, .box_w = 11, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 130, .adv_w = 140, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 148, .adv_w = 138, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 242, .box_w = 15, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 194, .adv_w = 164, .box_w = 10, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 214, .adv_w = 160, .box_w = 10, .box_h = 12, .ofs_x = 0, .ofs_y = -4},
    {.bitmap_index = 244, .adv_w = 119, .box_w = 8, .box_h = 8, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 260, .adv_w = 93, .box_w = 6, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 161, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 240, .box_w = 13, .box_h = 13, .ofs_x = 1, .ofs_y = -1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x8, 0x9, 0x21, 0x23, 0x34, 0x41, 0x45,
    0x4d, 0x4e, 0x50, 0x52, 0x54, 0x55, 0x20e3
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 8420, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 15, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Map glyph_ids to kern left classes*/
static const uint8_t kern_left_class_mapping[] =
{
    0, 0, 1, 2, 3, 4, 5, 6,
    7, 8, 8, 9, 10, 11, 12, 0
};

/*Map glyph_ids to kern right classes*/
static const uint8_t kern_right_class_mapping[] =
{
    0, 0, 1, 2, 3, 4, 5, 6,
    7, 8, 8, 9, 8, 10, 11, 0
};

/*Kern values between classes*/
static const int8_t kern_class_values[] =
{
    -17, 3, -7, -5, 0, -3, -7, -3,
    0, -3, -6, -2, -17, -1, 0, -5,
    0, 0, 0, 0, 0, 0, 0, -7,
    1, -7, -18, -1, -5, 0, -7, -6,
    -8, 0, 2, -1, -1, 0, 0, 0,
    0, -3, 0, -4, -5, 0, -19, -3,
    1, -14, -19, -9, -16, -2, -12, 0,
    -3, 1, -4, -16, 0, 0, 0, -5,
    0, -2, 0, -5, -5, 0, -14, -1,
    0, 0, -2, 0, 0, 0, -6, 0,
    -2, -17, -1, 0, 0, -4, -1, -1,
    0, -6, -12, 0, -16, -1, 1, 0,
    -2, -1, -1, 0, -8, -17, 0, -2,
    -5, -4, 0, 0, 2, 0, 0, 0,
    2, 0, -3, 0, 0, 0, -1, 0,
    -1, 0, -2, 0, -2, -13, 0, -1,
    0, -2, 0, 0
};


/*Collect the kern class' data in one place*/
static const lv_font_fmt_txt_kern_classes_t kern_classes =
{
    .class_pair_values   = kern_class_values,
    .left_class_mapping  = kern_left_class_mapping,
    .right_class_mapping = kern_right_class_mapping,
    .left_class_cnt      = 12,
    .right_class_cnt     = 11,
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_classes,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 2,
    .kern_classes = 1,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t font_tw_15 = {
#else
lv_font_t font_tw_15 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 4,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if FONT_TW_15*/

