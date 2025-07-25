/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2021 Tencent. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>
namespace pag {
// clang-format off

// https://github.com/harfbuzz/harfbuzz/blob/f1f2be776bcd994fa9262622e1a7098a066e5cf7/src/hb-unicode-emoji-table.hh#L26-L74
static const uint8_t
_hb_emoji_u8[464] =
{
   16, 17, 17, 17, 50, 20, 21, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
   17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,118,152,
    0,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    2,  0,  3,  4,  0,  0,  5,  6,  0,  7,  0,  8,  9, 10, 11, 12,
    0,  0, 13,  0,  0,  0, 14,  0, 15,  0,  0,  0,  0, 16,  0,  0,
   17, 17, 18, 19, 20, 17, 17, 21, 17, 17, 22, 17, 23, 17, 24, 25,
   26, 27, 28, 17, 17, 17,  0,  0, 17, 17, 17, 17, 17, 17, 17, 29,
    0,  0,  0,  0,  0,  1,  0,  0,  0,  2,  3,  0,  0,  4,  0,  0,
    5,  6,  0,  0,  7,  8,  0,  0,  8,  0,  9, 10,  0,  0, 11,  0,
    0, 12, 13, 14, 15, 16, 16, 16, 17, 16, 16, 16, 18, 19, 20, 21,
   22, 23,  0,  0,  0, 24,  0,  0, 25,  0, 26,  0,  0, 27,  0,  0,
   28,  0,  0,  0, 16, 16, 16, 16, 29,  9,  0, 30, 31, 32, 16, 33,
   34, 35, 36, 16, 16, 16, 16, 37, 16, 38, 39, 16, 16, 16, 40,  0,
    0,  0,  0, 41,  0,  0, 42, 16, 43,  0, 44,  0, 45, 46, 16, 16,
   47, 48, 49, 16, 16, 16, 16, 38,  0,  0,  0,  0,  0, 66,  0,  0,
    0,  0,  0, 16,  0,  2,  0,  0,  4,  0,  0,  2,  0,  0,240,  3,
    0,  6,  0,  0,  0,  0,  0, 12,  0,  1,  0,  0,  0,128,  0,  0,
    0,254, 15,  7,  4,  0,  0,  0,  0, 12, 64,  0,  1,  0,  0,  0,
    0,  0,  0,120,191,255,247,255,255,255,255,255, 63,  0,255,255,
   63,255, 87, 32,  2,  1, 24,  0,144, 80,184,  0,248,  0,  0,  0,
    0,  0,224,  0,  2,  0,  1,128,  0,  0, 48,  0,224,  0,  0, 24,
    0,  0, 33,  0,  0,  0,  1, 32,  0,  0,128,  2,  0,224,  0,  0,
    0,240,  3,192,  0, 64,254,  7,  0,224,255,255, 63,  0,  0,  0,
  254,255,  0,  4,  0,128,252,247,  0,254,255,255,255,255,255,  7,
  255,255,255, 63,192,255,255,255,255,255,  0,  0,  0,  0,240,255,
    0,  0,224,255,  0,240,  0,  0,  0,255,  0,252,  0,255,  0,  0,
    0,192,255,255,  0,240,255,255,255,255,255,247,191,255,255,255,
};

static inline unsigned
_hb_emoji_b4 (const uint8_t* a, unsigned i)
{
  return (a[i>>1]>>((i&1u)<<2))&15u;
}
static inline unsigned
_hb_emoji_b1 (const uint8_t* a, unsigned i)
{
  return (a[i>>3]>>((i&7u)<<0))&1u;
}
static inline uint_fast8_t
_hb_emoji_is_Extended_Pictographic (unsigned u)
{
  return u<131070u?_hb_emoji_b1(264+_hb_emoji_u8,((_hb_emoji_u8[144+(((_hb_emoji_u8[64+(((_hb_emoji_b4(_hb_emoji_u8,u>>5>>2>>3))<<3)+((u>>5>>2)&7u))])<<2)+((u>>5)&3u))])<<5)+((u)&31u)):0;
}
// clang-format on

static inline bool EmojiIsExtendedPictographic(uint32_t u) {
  return _hb_emoji_is_Extended_Pictographic(u);
}
}  // namespace pag
