/*
 *自定义中文库
 */

#ifndef CHINESETXT_H
#define CHINESETXT_H

#include "u8x8.h"

#define U8G2_FONT_SECTION(name) U8X8_FONT_SECTION(name) 

extern const uint8_t u8g2_font_unifont_st16[] U8G2_FONT_SECTION("u8g2_font_unifont_st16");
extern const uint8_t u8g2_font_unifont_hwxk16[] U8G2_FONT_SECTION("u8g2_font_unifont_hwhp16");

#endif