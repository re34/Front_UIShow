/*******************************************************************************
 * Size: 22 px
 * Bpp: 2
 * Opts: 
 ******************************************************************************/

#include "lvgl.h"


#ifndef FONT_ATLANTICO_22
#define FONT_ATLANTICO_22 1
#endif

#if FONT_ATLANTICO_22

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0041 "A" */
    0xf, 0xf0, 0xf, 0xf0, 0xf, 0xf0, 0xf, 0xf0,
    0x1f, 0xf4, 0x1f, 0xf4, 0x2e, 0xf8, 0x2e, 0xb8,
    0x3e, 0xb8, 0x3e, 0xbc, 0x3d, 0xbc, 0x3e, 0xbc,
    0x3f, 0xfc, 0x7f, 0xfd, 0x7c, 0x3d, 0xbc, 0x3d,

    /* U+0044 "D" */
    0xbf, 0xe2, 0xff, 0xeb, 0xdf, 0xef, 0x3f, 0xbc,
    0xfe, 0xf3, 0xfb, 0xcf, 0xef, 0x3f, 0xbc, 0xfe,
    0xf3, 0xfb, 0xcf, 0xef, 0x3f, 0xbc, 0xfe, 0xf7,
    0xfb, 0xff, 0xef, 0xf8,

    /* U+0045 "E" */
    0xbf, 0xdb, 0xfd, 0xbd, 0xb, 0xc0, 0xbc, 0xb,
    0xc0, 0xbd, 0xb, 0xfc, 0xbf, 0xcb, 0xc0, 0xbc,
    0xb, 0xc0, 0xbc, 0xb, 0xd0, 0xbf, 0xdb, 0xfd,

    /* U+0046 "F" */
    0xbf, 0xeb, 0xfe, 0xbd, 0xb, 0xc0, 0xbc, 0xb,
    0xc0, 0xbd, 0xb, 0xfc, 0xbf, 0xcb, 0xc0, 0xbc,
    0xb, 0xc0, 0xbc, 0xb, 0xc0, 0xbc, 0xb, 0xc0,

    /* U+004C "L" */
    0xbc, 0xb, 0xc0, 0xbc, 0xb, 0xc0, 0xbc, 0xb,
    0xc0, 0xbc, 0xb, 0xc0, 0xbc, 0xb, 0xc0, 0xbc,
    0xb, 0xc0, 0xbc, 0xb, 0xd0, 0xbf, 0xdb, 0xfd,

    /* U+0052 "R" */
    0xbf, 0xe2, 0xff, 0xeb, 0xdf, 0xaf, 0x3e, 0xbc,
    0xfa, 0xf3, 0xeb, 0xdf, 0xaf, 0xfc, 0xbf, 0xf2,
    0xfb, 0xeb, 0xcf, 0xaf, 0x3e, 0xbc, 0xfa, 0xf3,
    0xeb, 0xcf, 0xaf, 0x3e,

    /* U+0053 "S" */
    0x1f, 0xe1, 0xff, 0xeb, 0xdf, 0xef, 0x3f, 0xbc,
    0xfe, 0xf0, 0xb, 0xf0, 0xb, 0xf0, 0xb, 0xf4,
    0xb, 0xf0, 0xf, 0xef, 0x3f, 0xbc, 0xfe, 0xf7,
    0xf7, 0xff, 0xc7, 0xf8,

    /* U+0059 "Y" */
    0x7c, 0x3d, 0x3c, 0x3c, 0x3d, 0x7c, 0x2d, 0xb8,
    0x1e, 0xb4, 0xf, 0xf0, 0xf, 0xf0, 0xb, 0xe0,
    0x7, 0xd0, 0x7, 0xd0, 0x7, 0xd0, 0x7, 0xd0,
    0x7, 0xd0, 0x7, 0xd0, 0x7, 0xd0, 0x7, 0xd0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 28, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 129, .box_w = 8, .box_h = 16, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 32, .adv_w = 117, .box_w = 7, .box_h = 16, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 94, .box_w = 6, .box_h = 16, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 96, .box_w = 6, .box_h = 16, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 95, .box_w = 6, .box_h = 16, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 132, .adv_w = 115, .box_w = 7, .box_h = 16, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 117, .box_w = 7, .box_h = 16, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 129, .box_w = 8, .box_h = 16, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x21, 0x24, 0x25, 0x26, 0x2c, 0x32, 0x33,
    0x39
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 58, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 9, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    2, 2,
    2, 3,
    2, 6,
    2, 8,
    2, 9,
    3, 2,
    3, 6,
    3, 7,
    3, 9,
    4, 8,
    4, 9,
    5, 2,
    5, 9,
    6, 2,
    6, 9,
    7, 2,
    7, 7,
    7, 9,
    8, 2,
    8, 9,
    9, 2,
    9, 3,
    9, 4,
    9, 8
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    -2, -2, -2, -4, -26, -2, 2, 2,
    -7, -2, -4, -18, -2, -4, -37, -2,
    2, -5, -4, -7, -23, -2, -2, -5
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 24,
    .glyph_ids_size = 0
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
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 2,
    .kern_classes = 0,
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
const lv_font_t font_Atlantico_22 = {
#else
lv_font_t font_Atlantico_22 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = 0,
    .underline_thickness = 0,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if FONT_ATLANTICO_22*/

