/*
   Copyright 2022 Pigmy-penguin

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

           http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <arch/amd64/timers/tsc.h>
#include <drivers/video/fb.h>
#include <kernel/console.h>
#include <kernel/string.h>
#include <kernel/types.h>

#include <drivers/char/serial.h>

static u8 font_bitmap[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x81, 0xa5, 0x81, 0x81, 0xbd,
    0x99, 0x81, 0x81, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0xff,
    0xdb, 0xff, 0xff, 0xc3, 0xe7, 0xff, 0xff, 0x7e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x6c, 0xee, 0xfe, 0xfe, 0xfe, 0x7c, 0x7c, 0x38, 0x38, 0x10,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x38, 0x7c, 0x7c, 0xfe,
    0x7c, 0x7c, 0x38, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x38,
    0x38, 0x10, 0xd6, 0xfe, 0xfe, 0xd6, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x38, 0x38, 0x7c, 0x7c, 0xfe, 0xfe, 0xd6, 0x10, 0x10,
    0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x3c,
    0x3c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
    0xe7, 0xc3, 0xc3, 0xc3, 0xc3, 0xe7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x3c, 0x24, 0x66, 0x42, 0x42, 0x42, 0x66, 0x24, 0x3c,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc3, 0xdb, 0x99, 0xbd, 0xbd,
    0xbd, 0x99, 0xdb, 0xc3, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x1e, 0x06,
    0x0a, 0x0a, 0x10, 0x70, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x44, 0x38, 0x10, 0x10, 0x7c, 0x10,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x18, 0x14, 0x14, 0x12, 0x12,
    0x10, 0x10, 0x70, 0xf0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x20, 0x30, 0x2c,
    0x22, 0x32, 0x2e, 0x22, 0x22, 0xe2, 0xc2, 0x02, 0x0e, 0x0c, 0x00, 0x00,
    0x00, 0x10, 0x92, 0xba, 0x44, 0x44, 0x44, 0xc6, 0x44, 0x44, 0x44, 0xba,
    0x92, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 0xf8, 0xfe,
    0xf8, 0xe0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x02, 0x0e, 0x3e, 0xfe, 0x3e, 0x0e, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x38, 0x7c, 0x54, 0x10, 0x10, 0x10, 0x10, 0x10, 0x54, 0x7c,
    0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x24, 0x24, 0x24, 0x24, 0x24, 0x24,
    0x24, 0x00, 0x00, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x92,
    0x92, 0x92, 0x92, 0x72, 0x12, 0x12, 0x12, 0x12, 0x12, 0x00, 0x00, 0x00,
    0x00, 0x38, 0x44, 0x40, 0x20, 0x58, 0x84, 0x82, 0x42, 0x34, 0x08, 0x04,
    0x44, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x7e, 0x7e, 0x7e, 0x7e, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x7c,
    0x54, 0x10, 0x10, 0x10, 0x10, 0x54, 0x7c, 0x38, 0x10, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x10, 0x38, 0x7c, 0x54, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x54, 0x7c, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x0c, 0xfe, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x60, 0xfe, 0x60, 0x30, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x24, 0x66, 0xff, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x38, 0x38, 0x7c, 0x7c, 0xfe, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7c, 0x7c, 0x38,
    0x38, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x10, 0x38, 0x38, 0x38, 0x10, 0x10, 0x10, 0x00, 0x00, 0x10,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x24, 0x24, 0x24, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x24,
    0x24, 0x7e, 0x24, 0x24, 0x24, 0x7e, 0x24, 0x24, 0x24, 0x00, 0x00, 0x00,
    0x10, 0x10, 0x3e, 0x42, 0x40, 0x40, 0x40, 0x3c, 0x02, 0x02, 0x02, 0x42,
    0x7c, 0x10, 0x10, 0x00, 0x00, 0x60, 0x90, 0x90, 0x92, 0x64, 0x08, 0x10,
    0x20, 0x4c, 0x92, 0x12, 0x12, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44,
    0x44, 0x28, 0x30, 0x50, 0x8a, 0x8a, 0x84, 0x8a, 0x72, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x08, 0x08, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x10, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00, 0x40, 0x20, 0x10,
    0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x10, 0x10, 0x20, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x7c,
    0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0x20, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x04,
    0x04, 0x08, 0x08, 0x10, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40, 0x00, 0x00,
    0x00, 0x00, 0x3c, 0x42, 0x46, 0x46, 0x4a, 0x4a, 0x52, 0x52, 0x62, 0x62,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x28, 0x48, 0x08, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x42,
    0x42, 0x02, 0x04, 0x08, 0x10, 0x20, 0x20, 0x42, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3c, 0x42, 0x42, 0x02, 0x04, 0x18, 0x04, 0x02, 0x42, 0x42,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x14, 0x14, 0x24, 0x44, 0x44,
    0x84, 0xfe, 0x04, 0x04, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x40,
    0x40, 0x40, 0x7c, 0x42, 0x02, 0x02, 0x02, 0x42, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x1c, 0x22, 0x40, 0x40, 0x7c, 0x42, 0x42, 0x42, 0x42, 0x42,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x42, 0x44, 0x04, 0x08, 0x08,
    0x08, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x42,
    0x42, 0x42, 0x24, 0x18, 0x24, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x3e, 0x02, 0x02, 0x02, 0x44,
    0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00,
    0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x20, 0x20, 0x00,
    0x00, 0x00, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x20, 0x10, 0x08, 0x04,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00,
    0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x20,
    0x10, 0x08, 0x04, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7c, 0x82, 0x82, 0x02, 0x04, 0x08, 0x10, 0x10, 0x00, 0x10,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x82, 0x9e, 0xa2, 0xa2, 0xa2,
    0xa2, 0xa6, 0x9a, 0x80, 0x40, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x70, 0x10,
    0x28, 0x28, 0x28, 0x44, 0x44, 0x7c, 0x82, 0x82, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfc, 0x42, 0x42, 0x42, 0x44, 0x78, 0x44, 0x42, 0x42, 0x42,
    0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x42, 0x82, 0x80, 0x80, 0x80,
    0x80, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x44,
    0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x44, 0xf8, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0x42, 0x40, 0x40, 0x44, 0x7c, 0x44, 0x40, 0x40, 0x42,
    0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x42, 0x40, 0x40, 0x44, 0x7c,
    0x44, 0x40, 0x40, 0x40, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x42,
    0x82, 0x80, 0x80, 0x9e, 0x82, 0x82, 0x82, 0x42, 0x3e, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xe6, 0x42, 0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x42,
    0xe6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x84, 0x84, 0x88, 0x70, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xce, 0x84, 0x88, 0x90, 0xa0, 0xd0, 0x88, 0x88, 0x84, 0x84,
    0xce, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x42, 0x42, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0xc6,
    0xaa, 0xaa, 0x92, 0x92, 0x82, 0x82, 0x82, 0x82, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xc6, 0x42, 0x62, 0x52, 0x52, 0x4a, 0x4a, 0x4a, 0x46, 0x42,
    0xe2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82,
    0x82, 0x82, 0x82, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x42,
    0x42, 0x42, 0x42, 0x7c, 0x40, 0x40, 0x40, 0x40, 0xe0, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 0xa2, 0x92, 0x4c,
    0x38, 0x04, 0x02, 0x00, 0x00, 0x00, 0xfc, 0x42, 0x42, 0x42, 0x42, 0x7c,
    0x44, 0x44, 0x42, 0x42, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x42,
    0x40, 0x40, 0x40, 0x3c, 0x02, 0x02, 0x02, 0x42, 0x7c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0x92, 0x92, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe6, 0x42, 0x42, 0x42, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x82,
    0x82, 0x44, 0x44, 0x44, 0x28, 0x28, 0x28, 0x10, 0x10, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xc6, 0x82, 0x82, 0x92, 0x92, 0x92, 0x54, 0x54, 0x54, 0x6c,
    0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x44, 0x44, 0x28, 0x28, 0x10,
    0x28, 0x28, 0x44, 0x44, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x82,
    0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x7e, 0x42, 0x04, 0x04, 0x08, 0x10, 0x10, 0x20, 0x20, 0x42,
    0x7e, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x00, 0x00, 0x00, 0x40, 0x40, 0x20,
    0x20, 0x10, 0x10, 0x08, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x00, 0x00,
    0x00, 0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x08, 0x78, 0x00, 0x00, 0x00, 0x10, 0x28, 0x44, 0x82, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x20, 0x20, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x84, 0x04,
    0x7c, 0x84, 0x84, 0x8c, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x40,
    0x40, 0x5c, 0x62, 0x42, 0x42, 0x42, 0x42, 0x62, 0xdc, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x42, 0x40, 0x40, 0x40, 0x42, 0x42,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x04, 0x04, 0x74, 0x8c, 0x84,
    0x84, 0x84, 0x84, 0x8c, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x3c, 0x42, 0x42, 0x7e, 0x40, 0x40, 0x42, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0e, 0x12, 0x10, 0x7e, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x76, 0x8c, 0x84,
    0x84, 0x84, 0x8c, 0x74, 0x04, 0x84, 0x78, 0x00, 0x00, 0x00, 0xc0, 0x40,
    0x40, 0x5c, 0x62, 0x42, 0x42, 0x42, 0x42, 0x42, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x18, 0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x1c, 0x04, 0x04,
    0x04, 0x04, 0x04, 0x04, 0x84, 0x88, 0x70, 0x00, 0x00, 0x00, 0xc0, 0x40,
    0x40, 0x4e, 0x44, 0x48, 0x50, 0x68, 0x44, 0x42, 0xc6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xac, 0xd2, 0x92,
    0x92, 0x92, 0x92, 0x92, 0xd6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xdc, 0x62, 0x42, 0x42, 0x42, 0x42, 0x42, 0xe6, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x62, 0x42,
    0x42, 0x42, 0x62, 0x5c, 0x40, 0x40, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x76, 0x8c, 0x84, 0x84, 0x84, 0x8c, 0x74, 0x04, 0x04, 0x0e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x6e, 0x32, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x42, 0x40,
    0x30, 0x0c, 0x02, 0x42, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
    0x20, 0xfc, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x1c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x42, 0x42, 0x42, 0x42, 0x42, 0x46,
    0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x82, 0x82,
    0x44, 0x44, 0x28, 0x28, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xc6, 0x82, 0x92, 0x92, 0x92, 0x54, 0x6c, 0x44, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xee, 0x44, 0x28, 0x10, 0x10, 0x28, 0x44,
    0xee, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x82, 0x82,
    0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7e, 0x42, 0x04, 0x08, 0x10, 0x20, 0x42, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x60, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x60, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x0c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x60, 0x00, 0x00,
    0x00, 0x62, 0x92, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x28, 0x28,
    0x44, 0x44, 0x82, 0x82, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x42,
    0x80, 0x80, 0x80, 0x80, 0x82, 0x82, 0x44, 0x38, 0x08, 0x04, 0x38, 0x00,
    0x00, 0x00, 0x6c, 0x00, 0x00, 0xc6, 0x42, 0x42, 0x42, 0x42, 0x42, 0x46,
    0x3a, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x3c, 0x42, 0x42,
    0x7e, 0x40, 0x40, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x10, 0x28, 0x44,
    0x00, 0x78, 0x84, 0x04, 0x7c, 0x84, 0x84, 0x8c, 0x76, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x6c, 0x00, 0x00, 0x78, 0x84, 0x04, 0x7c, 0x84, 0x84, 0x8c,
    0x76, 0x00, 0x00, 0x00, 0x00, 0x60, 0x18, 0x00, 0x00, 0x78, 0x84, 0x04,
    0x7c, 0x84, 0x84, 0x8c, 0x76, 0x00, 0x00, 0x00, 0x00, 0x30, 0x48, 0x30,
    0x00, 0x78, 0x84, 0x04, 0x7c, 0x84, 0x84, 0x8c, 0x76, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x42, 0x40, 0x40, 0x42, 0x42, 0x3c,
    0x08, 0x04, 0x38, 0x00, 0x00, 0x10, 0x28, 0x44, 0x00, 0x3c, 0x42, 0x42,
    0x7e, 0x40, 0x40, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x00,
    0x00, 0x3c, 0x42, 0x42, 0x7e, 0x40, 0x40, 0x42, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x30, 0x0c, 0x00, 0x00, 0x3c, 0x42, 0x42, 0x7e, 0x40, 0x40, 0x42,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x00, 0x00, 0x38, 0x08, 0x08,
    0x08, 0x08, 0x08, 0x08, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x10, 0x28, 0x44,
    0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x30, 0x0c, 0x00, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
    0x7e, 0x00, 0x00, 0x00, 0x6c, 0x00, 0x70, 0x10, 0x28, 0x28, 0x28, 0x44,
    0x44, 0x7c, 0x82, 0x82, 0xc6, 0x00, 0x00, 0x00, 0x30, 0x48, 0x30, 0x00,
    0x70, 0x28, 0x28, 0x44, 0x44, 0x7c, 0x82, 0x82, 0xc6, 0x00, 0x00, 0x00,
    0x0c, 0x30, 0x00, 0xfe, 0x42, 0x40, 0x44, 0x7c, 0x44, 0x40, 0x40, 0x42,
    0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x92, 0x12,
    0x7e, 0x90, 0x90, 0x92, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x1a,
    0x28, 0x28, 0x28, 0x4e, 0x48, 0x78, 0x88, 0x8a, 0xce, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x28, 0x44, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x00, 0x00, 0x3c, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0c, 0x00,
    0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x28, 0x44, 0x00, 0xc6, 0x42, 0x42, 0x42, 0x42, 0x42, 0x46,
    0x3a, 0x00, 0x00, 0x00, 0x00, 0x30, 0x0c, 0x00, 0x00, 0xc6, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x46, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x00,
    0x00, 0xc6, 0x82, 0x82, 0x44, 0x44, 0x28, 0x28, 0x10, 0x10, 0x60, 0x00,
    0x6c, 0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x44,
    0x38, 0x00, 0x00, 0x00, 0x6c, 0x00, 0xe6, 0x42, 0x42, 0x42, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x44, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
    0x10, 0x3e, 0x42, 0x40, 0x40, 0x40, 0x42, 0x42, 0x3c, 0x10, 0x10, 0x00,
    0x00, 0x00, 0x1c, 0x22, 0x20, 0x20, 0x7c, 0x20, 0x20, 0x20, 0x20, 0x62,
    0x9c, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6, 0x82, 0x44, 0x44, 0x28, 0xee,
    0x10, 0xfe, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x90,
    0x94, 0x94, 0x9e, 0x94, 0xe4, 0x84, 0x84, 0x84, 0x86, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x0e, 0x12, 0x10, 0x10, 0x7e, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x90, 0xe0, 0x00, 0x00, 0x18, 0x60, 0x00, 0x00, 0x78, 0x84, 0x04,
    0x7c, 0x84, 0x84, 0x8c, 0x76, 0x00, 0x00, 0x00, 0x00, 0x18, 0x60, 0x00,
    0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x7e, 0x00, 0x00, 0x00,
    0x00, 0x18, 0x60, 0x00, 0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42,
    0x3c, 0x00, 0x00, 0x00, 0x00, 0x18, 0x60, 0x00, 0x00, 0xc6, 0x42, 0x42,
    0x42, 0x42, 0x42, 0x46, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x32, 0x4c, 0x00,
    0x00, 0xdc, 0x62, 0x42, 0x42, 0x42, 0x42, 0x42, 0xe6, 0x00, 0x00, 0x00,
    0x32, 0x4c, 0x00, 0xc6, 0x42, 0x62, 0x52, 0x52, 0x4a, 0x4a, 0x46, 0x42,
    0xe2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x84, 0x04,
    0x7c, 0x84, 0x84, 0x8c, 0x76, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x3c, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x7e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x00, 0x10, 0x10, 0x20, 0x40, 0x80,
    0x82, 0x82, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xc0, 0x40, 0x40, 0x42, 0x44, 0xe8, 0x10, 0x20, 0x4c, 0x92, 0x12, 0x04,
    0x08, 0x10, 0x1e, 0x00, 0xc0, 0x40, 0x40, 0x40, 0x42, 0xe4, 0x08, 0x10,
    0x24, 0x4c, 0x94, 0x24, 0x3e, 0x04, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x10, 0x00, 0x00, 0x10, 0x10, 0x10, 0x38, 0x38, 0x38, 0x10, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x12, 0x24, 0x48, 0x90, 0x48, 0x24, 0x12, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x48, 0x24, 0x12,
    0x24, 0x48, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x22, 0x88, 0x88,
    0x22, 0x22, 0x88, 0x88, 0x22, 0x22, 0x88, 0x88, 0x22, 0x22, 0x88, 0x88,
    0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa, 0x55, 0x55, 0xaa, 0xaa,
    0x55, 0x55, 0xaa, 0xaa, 0xdb, 0xdb, 0x77, 0x77, 0xdb, 0xdb, 0xee, 0xee,
    0xdb, 0xdb, 0x77, 0x77, 0xdb, 0xdb, 0xee, 0xee, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xf0, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0xf0, 0x10, 0x10,
    0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0xf4, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x10, 0x10,
    0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x14, 0x14, 0x14, 0x14,
    0x14, 0xf4, 0x04, 0x04, 0xf4, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x04, 0x04,
    0xf4, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0xf4, 0x04, 0x04, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0xfc, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0xf0, 0x10, 0x10,
    0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xf0, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x17, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x17, 0x10, 0x10, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x10, 0x10, 0x17, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0xf7, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x00, 0xf7, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x17, 0x10, 0x10, 0x17, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x14, 0x14, 0x14,
    0x14, 0xf7, 0x00, 0x00, 0xf7, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x10, 0x10, 0x10, 0x10, 0x10, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xff, 0x00, 0x00, 0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x1f, 0x10, 0x10, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x10, 0x10, 0x1f, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1f, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x14, 0x14, 0x14, 0x14, 0xff, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
    0x10, 0x10, 0x10, 0x10, 0x10, 0xff, 0x10, 0x10, 0xff, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x1f, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0,
    0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
    0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
    0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x62, 0x92, 0x94, 0x88, 0x88, 0x94, 0x92, 0x62, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x58, 0x44, 0x42, 0x42, 0x42, 0x42,
    0x5c, 0x40, 0x40, 0x00, 0x00, 0x00, 0xfe, 0x42, 0x42, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7e, 0xa4, 0x24, 0x24, 0x24, 0x24, 0x44, 0x46, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xfe, 0x42, 0x20, 0x20, 0x10, 0x08, 0x10, 0x20, 0x20, 0x42,
    0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x88, 0x84,
    0x84, 0x84, 0x84, 0x84, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xcc, 0x44, 0x44, 0x44, 0x44, 0x44, 0x64, 0x5a, 0x40, 0x40, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x66, 0x98, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x7c, 0x10, 0x10, 0x38, 0x44, 0x44,
    0x44, 0x38, 0x10, 0x10, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x24,
    0x42, 0x42, 0x42, 0x7e, 0x42, 0x42, 0x42, 0x24, 0x18, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x38, 0x44, 0x82, 0x82, 0x82, 0x82, 0x82, 0x44, 0x44, 0x28,
    0xee, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x42, 0x20, 0x10, 0x3c, 0x42,
    0x42, 0x42, 0x42, 0x42, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x44, 0xaa, 0x92, 0x92, 0xaa, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x02, 0x02, 0x3c, 0x44, 0x8a, 0x92, 0x92, 0xa2, 0x44, 0x78,
    0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x20, 0x40, 0x40, 0x40, 0x7e,
    0x40, 0x40, 0x40, 0x20, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x24,
    0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0xfe, 0x10, 0x10,
    0x10, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x30,
    0x0c, 0x02, 0x0c, 0x30, 0xc0, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x06, 0x18, 0x60, 0x80, 0x60, 0x18, 0x06, 0x00, 0x00, 0xfe,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0a, 0x08, 0x08, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x20, 0x20, 0xa0, 0xc0, 0x00,
    0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x10,
    0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x62, 0x92, 0x8c, 0x00,
    0x62, 0x92, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x24, 0x24,
    0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x38, 0x38, 0x38, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x38,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x08,
    0x08, 0x08, 0x10, 0x90, 0x90, 0x50, 0x50, 0x20, 0x20, 0x00, 0x00, 0x00,
    0x58, 0x68, 0x48, 0x48, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x30, 0x48, 0x10, 0x20, 0x78, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};

extern u32 *fb;
extern u16 width;
extern u16 height;
extern u16 pitch;

static int x_pos;
static int y_pos;

static int fg_color = DEFAULT_FG_COLOR;
static int bg_color = DEFAULT_BG_COLOR;

static inline void set_color(u32 fg, u32 bg) {
  fg_color = fg;
  bg_color = bg;
}

// Scrolls the text on the screen up by one line.
static void scroll(void) {
  int screensiz = pitch * 16 * (height / 16) / sizeof(u32);
  int rowsiz = pitch * 16 / sizeof(u32);
  y_pos--;
  for (int i = 0; i < screensiz - rowsiz; i++) {
    fb[i] = fb[i + rowsiz];
  }
}

void init_console() {
  cls();

  puts("\nSlops version 0.0.0 (compiled on ");
  puts(__DATE__);
  puts(" at ");
  puts(__TIME__);
  puts(") \n\n");
}

void cls(void) {
  for (u32 i = 0; i < width * pitch / sizeof(u32); i++) {
    fb[i] = bg_color;
  }
  x_pos = 0;
  y_pos = 0;
}

int vsnprintk(char *buffer, u32 buffer_size, const char *fmt,
              __builtin_va_list vl) {
  char c;
  char buf[20];
  char *p = NULL;

  u32 buffer_index = 0;

  while ((c = *fmt++) != 0) {
    if (c == '%') {
      c = *fmt++;
      switch (c) {
      case 'p':
      case 'x':
        buf[0] = '0';
        buf[1] = 'x';
        itoa(buf + 2, c, __builtin_va_arg(vl, int));
        p = buf;
        goto string;
        break;
        // case 'l':
        //      *fmt++;
        //      c = 'd';
        //      // fallthrough
      case 'd':
      case 'u':
        itoa(buf, c, __builtin_va_arg(vl, int));
        p = buf;
        goto string;
        break;
      case 's':
        p = __builtin_va_arg(vl, char *);
        if (!p)
          p = "(null)";
      string:
        while (*p) {
          buffer[buffer_index++] = (*p++);
        }
        break;
      default:
        buffer[buffer_index++] = __builtin_va_arg(vl, int);
        break;
      }
    } else {
      buffer[buffer_index++] = c;
    }
    if (buffer_index >= buffer_size - 1) {
      break;
    }
  }

  buffer[buffer_index] = '\0';

  return buffer_index;
}

void printk(const char *fmt, ...) {
  char buffer[1024];

  __builtin_va_list vl;
  __builtin_va_start(vl, fmt);

  vsnprintk(buffer, sizeof(buffer), fmt, vl);

  __builtin_va_end(vl);
  char ts[17] = "![    0.000] ";
  u64 ms = tsc_get_ms();
  // Display timestamp
  // TODO: optimize timestamp display

  if (fmt[0] == '$')
    ;

  else if (ms == 0)
    puts(ts);

  else {
    // Count number of digits
    u64 seconds = ms / 1000;
    u64 milliseconds = ms - (seconds * 1000);
    int num = seconds;
    u8 count = 0;
    do {
      // Increment digit count
      count++;
      // Remove last digit of num
      num /= 10;
    } while (num != 0);

    int x = 2;
    for (int i = (5 - count); i != 0; i--) {
      ts[x] = ' ';
      x++;
    }

    itoa(&ts[x], 10, seconds);
    for (int i = count; i != 0; i--) {
      x++;
    }
    ts[x] = '.';
    x++;
    num = milliseconds;
    u8 count2 = 0;
    do {
      // Increment digit count
      count2++;
      // Remove last digit of num
      num /= 10;
    } while (num != 0);

    if (count2 == 2) {
      ts[x] = '0';
      x++;
    } else if (count2 == 1) {
      ts[x] = '0';
      x++;
      ts[x] = '0';
      x++;
    }
    itoa(&ts[x], 10, milliseconds);
    ts[11] = ']';
    ts[12] = ' ';
    puts(ts);
  }
  puts(buffer);
}

void puts(const char *str) {
  while (*str)
    putc(*str++);
}

// Writes a single character out to the screen.
void putc(char c) {
  switch (c) {
  case '\n':
    goto newline;
  case '^':
    set_color(0xf54263, DEFAULT_BG_COLOR);
    goto new_color;
  case '~':
    set_color(DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
    goto new_color;
  case '!':
    set_color(0x34eba8, DEFAULT_BG_COLOR);
    goto new_color;
  case '*':
    set_color(0xf8e1af, DEFAULT_BG_COLOR);
    goto new_color;
  case '$':
    set_color(DEFAULT_FG_COLOR, 0xf72202);
    goto new_color;
  }
  int x_pix = x_pos * 8;  // font width
  int y_pix = y_pos * 16; // font height
  u8 *glyph = &font_bitmap[c * 16];
  for (int i = 0; i < 16; i++) {
    for (int j = 7 /*8-1*/; j >= 0; j--) {
      if (glyph[i] & (1 << j))
        putpixel(x_pix++, y_pix, fg_color);
      else
        putpixel(x_pix++, y_pix, bg_color);
    }
    x_pix -= 8;
    y_pix++;
  }
  x_pos++;
  if (x_pos >= width / 8) {
  newline:
    y_pos++;
    x_pos = 0;
    putc(' ');
  }
new_color:
  if (y_pos >= height / 16 - 1) {
    scroll();
  }
}

char *get_timestamp(void) {
  // int count = 0;
  char *ts = "[    0.000000]\x00";
  u64 ms = tsc_get_ms();
  if (ms == 0)
    return ts;
  return ts;
}
