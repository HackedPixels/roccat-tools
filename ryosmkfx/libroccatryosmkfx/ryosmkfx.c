/*
 * This file is part of roccat-tools.
 *
 * roccat-tools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Macro Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * roccat-tools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Macro Public License for more details.
 *
 * You should have received a copy of the GNU Macro Public License
 * along with roccat-tools. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ryosmkfx.h"

/* least significant byte is pwm value, lower nibble of higher byte is brightness */
static guint16 const rgb_to_hardware[256] = {
	/*   0 */	0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
	/*  16 */	0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
	/*  32 */	0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
	/*  48 */	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
	/*  64 */	0x0120, 0x040d, 0x050b, 0x0311, 0x0217, 0x040e, 0x050c, 0x0125, 0x040f, 0x0313, 0x060b, 0x050d, 0x070a, 0x0809, 0x0b07, 0x0a08,
	/*  80 */	0x0909, 0x0c07, 0x0b08, 0x0c08, 0x0e07, 0x0b09, 0x0a0a, 0x0d08, 0x0c09, 0x0b0a, 0x0d09, 0x090d, 0x0a0c, 0x0f08, 0x090e, 0x0a0d,
	/*  96 */	0x0810, 0x090f, 0x0713, 0x0811, 0x0a0e, 0x0b0d, 0x0910, 0x0812, 0x0a0f, 0x0b0e, 0x0c0d, 0x0911, 0x0813, 0x0a10, 0x0912, 0x061a,
	/* 112 */	0x0717, 0x0815, 0x0913, 0x0b10, 0x0c0f, 0x0d0e, 0x0a12, 0x0914, 0x0b11, 0x0c10, 0x0a13, 0x0915, 0x0818, 0x061f, 0x0916, 0x0c11,
	/* 128 */	0x0d10, 0x0b13, 0x0917, 0x0a15, 0x081a, 0x0622, 0x071e, 0x0a16, 0x081b, 0x0623, 0x071f, 0x0919, 0x081c, 0x0720, 0x091a, 0x0a18,
	/* 144 */	0x091b, 0x0a19, 0x0b17, 0x091c, 0x0a1a, 0x0b18, 0x091d, 0x0d15, 0x0a1b, 0x0b19, 0x0822, 0x0a1c, 0x0b1a, 0x0a1d, 0x0b1b, 0x0a1e,
	/* 160 */	0x0b1c, 0x0a1f, 0x072b, 0x0b1d, 0x0923, 0x0a20, 0x0924, 0x0d1a, 0x0829, 0x0b1f, 0x0e19, 0x0d1b, 0x0c1e, 0x0b21, 0x0c1f, 0x0b22,
	/* 176 */	0x0c20, 0x0b23, 0x082f, 0x0a27, 0x0b24, 0x0a28, 0x0b25, 0x092d, 0x0c23, 0x0739, 0x0a2a, 0x0b27, 0x0d22, 0x0b28, 0x0a2c, 0x0b29,
	/* 192 */	0x0a2d, 0x0932, 0x0b2a, 0x0c27, 0x0b2b, 0x0c28, 0x0b2c, 0x0c29, 0x0b2d, 0x0c2a, 0x0b2e, 0x0c2b, 0x0b2f, 0x0e26, 0x0c2c, 0x0b30,
	/* 208 */	0x0c2d, 0x0a36, 0x0c2e, 0x0b32, 0x0c2f, 0x0d2c, 0x0c30, 0x0d2d, 0x0b35, 0x0d2e, 0x0c32, 0x0d2f, 0x0c33, 0x0d30, 0x0c34, 0x0d31,
	/* 224 */	0x0e2e, 0x0d32, 0x0d33, 0x0d34, 0x0e31, 0x0d35, 0x0d36, 0x0c3b, 0x0d37, 0x0d38, 0x0c3d, 0x0d39, 0x0e36, 0x0c3f, 0x0d3b, 0x0d3c,
	/* 240 */	0x0d3d, 0x0e3a, 0x0d3f, 0x0e3b, 0x0f38, 0x0e3c, 0x0f39, 0x0e3d, 0x0f3a, 0x0e3e, 0x0f3b, 0x0e3f, 0x0f3c, 0x0f3d, 0x0f3e, 0x0f3f
};

guint16 ryosmkfx_color_to_hardware(guint8 color) {
	return rgb_to_hardware[color];
}

static guint8 const level_to_rgb[1009] =  {
	/*    0 */	  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
	/*   20 */	 20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
	/*   40 */	 40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
	/*   60 */	 60,  61,  62,  63,  64,  65,  66,  66,  67,  68,  69,  69,  70,  70,  71,  72,  73,  74,  75,  75,
	/*   80 */   76,  77,  77,  78,  78,  78,  78,  79,  79,  79,  80,  81,  81,  81,  82,  82,  82,  82,  82,  82,
	/*  100 */	 82,  83,  83,  83,  83,  84,  84,  85,  85,  85,  86,  86,  87,  87,  87,  88,  88,  88,  88,  89,
	/*  120 */	 89,  89,  89,  89,  90,  90,  90,  90,  90,  91,  91,  91,  92,  92,  93,  93,  93,  93,  94,  94,
	/*  140 */	 94,  94,  95,  95,  96,  96,  96,  96,  97,  97,  97,  97,  98,  99, 100, 100, 101, 101, 101, 102,
	/*  160 */	102, 102, 103, 103, 104, 104, 104, 105, 105, 106, 107, 108, 108, 108, 109, 109, 109, 109, 109, 110,
	/*  180 */	110, 110, 111, 111, 112, 112, 112, 113, 113, 113, 114, 114, 115, 115, 116, 116, 117, 117, 118, 118,
	/*  200 */	119, 119, 119, 120, 120, 120, 120, 121, 121, 122, 123, 123, 123, 123, 124, 124, 124, 125, 125, 126,
	/*  220 */	126, 127, 127, 128, 128, 128, 128, 129, 129, 129, 130, 131, 131, 132, 132, 132, 132, 133, 133, 133,
	/*  240 */	134, 134, 135, 136, 136, 137, 137, 138, 138, 138, 139, 139, 140, 140, 140, 141, 141, 141, 141, 142,
	/*  260 */	142, 142, 142, 143, 143, 143, 143, 143, 144, 144, 144, 144, 144, 145, 145, 145, 146, 146, 146, 147,
	/*  280 */	147, 147, 147, 147, 148, 148, 148, 148, 149, 149, 150, 150, 150, 151, 151, 151, 152, 152, 152, 153,
	/*  300 */	153, 153, 153, 153, 154, 154, 154, 154, 155, 155, 155, 156, 156, 156, 156, 156, 157, 157, 157, 157,
	/*  320 */	157, 157, 158, 158, 158, 158, 158, 158, 159, 159, 159, 159, 159, 159, 160, 160, 160, 160, 160, 161,
	/*  340 */	161, 161, 161, 162, 162, 162, 162, 163, 163, 163, 164, 164, 165, 165, 165, 165, 165, 166, 166, 166,
	/*  360 */	166, 166, 166, 167, 167, 167, 167, 168, 168, 168, 168, 169, 169, 169, 170, 170, 170, 171, 171, 171,
	/*  380 */	171, 171, 171, 171, 171, 172, 172, 172, 172, 172, 172, 172, 172, 172, 173, 173, 173, 173, 173, 173,
	/*  400 */	174, 174, 174, 174, 174, 174, 175, 175, 175, 175, 175, 175, 175, 176, 176, 176, 176, 176, 176, 177,
	/*  420 */	177, 177, 178, 178, 178, 178, 178, 179, 179, 179, 179, 180, 180, 180, 180, 180, 180, 181, 181, 181,
	/*  440 */	181, 181, 181, 182, 182, 182, 182, 182, 183, 183, 183, 183, 183, 184, 184, 184, 185, 185, 185, 185,
	/*  460 */	186, 186, 186, 186, 186, 186, 187, 187, 187, 187, 187, 187, 187, 188, 188, 188, 188, 188, 188, 189,
	/*  480 */	189, 189, 189, 190, 190, 190, 190, 190, 190, 191, 191, 191, 191, 191, 192, 192, 192, 192, 193, 193,
	/*  500 */	193, 193, 193, 194, 194, 194, 195, 195, 195, 195, 195, 195, 196, 196, 196, 196, 196, 196, 196, 197,
	/*  520 */	197, 197, 197, 197, 197, 198, 198, 198, 198, 198, 198, 199, 199, 199, 199, 199, 199, 200, 200, 200,
	/*  540 */	200, 200, 200, 200, 201, 201, 201, 201, 201, 201, 202, 202, 202, 202, 202, 202, 203, 203, 203, 203,
	/*  560 */	203, 203, 204, 204, 204, 204, 204, 204, 205, 205, 205, 205, 206, 206, 206, 207, 207, 207, 207, 207,
	/*  580 */	207, 208, 208, 208, 208, 208, 208, 208, 208, 208, 209, 209, 209, 209, 209, 209, 209, 210, 210, 210,
	/*  600 */	211, 211, 211, 211, 211, 211, 212, 212, 212, 212, 212, 212, 212, 212, 213, 213, 213, 213, 213, 213,
	/*  620 */	213, 214, 214, 214, 214, 214, 214, 214, 215, 215, 215, 215, 215, 215, 216, 216, 216, 216, 216, 216,
	/*  640 */	216, 217, 217, 217, 217, 217, 217, 217, 218, 218, 218, 218, 218, 218, 218, 219, 219, 219, 219, 219,
	/*  660 */	219, 220, 220, 220, 220, 220, 220, 220, 221, 221, 221, 221, 221, 221, 221, 222, 222, 222, 222, 222,
	/*  680 */	222, 222, 223, 223, 223, 223, 223, 223, 223, 224, 224, 224, 224, 224, 224, 224, 225, 225, 225, 225,
	/*  700 */	225, 225, 225, 225, 225, 225, 225, 225, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226, 226,
	/*  720 */	226, 226, 227, 227, 227, 227, 227, 227, 227, 227, 227, 227, 228, 228, 228, 228, 228, 228, 228, 229,
	/*  740 */	229, 229, 229, 229, 229, 229, 229, 229, 229, 229, 230, 230, 230, 230, 230, 230, 230, 230, 230, 230,
	/*  760 */	230, 230, 231, 231, 231, 231, 231, 231, 231, 232, 232, 232, 232, 232, 232, 232, 232, 232, 233, 233,
	/*  780 */	233, 233, 233, 233, 233, 233, 233, 233, 233, 234, 234, 234, 234, 234, 234, 234, 235, 235, 235, 235,
	/*  800 */	235, 235, 235, 235, 235, 236, 236, 236, 236, 236, 236, 236, 236, 236, 236, 237, 237, 237, 237, 237,
	/*  820 */	237, 237, 237, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 238, 239, 239, 239, 239, 239, 239,
	/*  840 */	239, 239, 239, 239, 239, 239, 239, 239, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240, 240,
	/*  860 */	240, 240, 240, 241, 241, 241, 241, 241, 241, 241, 241, 241, 241, 241, 241, 241, 241, 242, 242, 242,
	/*  880 */	242, 242, 242, 242, 243, 243, 243, 243, 243, 243, 243, 244, 244, 244, 244, 244, 244, 244, 244, 245,
	/*  900 */	245, 245, 245, 245, 245, 245, 245, 246, 246, 246, 246, 246, 246, 246, 247, 247, 247, 247, 247, 247,
	/*  920 */	247, 247, 248, 248, 248, 248, 248, 248, 248, 248, 249, 249, 249, 249, 249, 249, 249, 249, 250, 250,
	/*  940 */	250, 250, 250, 250, 250, 251, 251, 251, 251, 251, 251, 251, 251, 252, 252, 252, 252, 252, 252, 252,
	/*  960 */	252, 252, 252, 252, 252, 252, 252, 252, 252, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253, 253,
	/*  980 */	253, 253, 253, 253, 253, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254, 254,
	/* 1000 */	254, 255, 255, 255, 255, 255, 255, 255, 255
};

guint8 ryosmkfx_hardware_to_color(guint16 hardware) {
	guint8 pwm = ryosmkfx_hardware_color_get_pwm(hardware);
	guint8 brightness = ryosmkfx_hardware_color_get_brightness(hardware);
	guint level = pwm * (brightness + 1);
	
	return level_to_rgb[level];
}
