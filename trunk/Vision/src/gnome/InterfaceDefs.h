#ifndef INTERFACE_DEFS__
#define INTERFACE_DEFS__

#include "PlatformDefines.h"
#include "Color.h"

struct key_info {
	uint32 modifiers;
	uint8 key_states[16];
};

enum color_which {
	B_PANEL_BACKGROUND_COLOR = 1,
	B_MENU_BACKGROUND_COLOR = 2,
	B_MENU_SELECTION_BACKGROUND_COLOR = 6,
	B_MENU_ITEM_TEXT_COLOR = 7,
	B_MENU_SELECTED_ITEM_TEXT_COLOR = 8,
	B_WINDOW_TAB_COLOR = 3,
	B_KEYBOARD_NAVIGATION_COLOR = 4,
	B_DESKTOP_COLOR = 5
};

enum alignment {
	B_LEFT,
	B_CENTER,
	B_RIGHT
};

// effects on standard gray level

const float B_LIGHTEN_MAX_TINT = 0.0F;		/* 216 --> 255.0 (255) */
const float B_LIGHTEN_2_TINT = 0.385F;	/* 216 --> 240.0 (240) */
const float B_LIGHTEN_1_TINT = 0.590F;	/* 216 --> 232.0 (232) */

const float B_NO_TINT = 1.0F;		/* 216 --> 216.0 (216) */

const float B_DARKEN_1_TINT = 1.147F;	/* 216 --> 184.2 (184) */
const float B_DARKEN_2_TINT = 1.295F;	/* 216 --> 152.3 (152) */
const float B_DARKEN_3_TINT = 1.407F;	/* 216 --> 128.1 (128) */
const float B_DARKEN_4_TINT = 1.555F;	/* 216 -->  96.1  (96) */
const float B_DARKEN_MAX_TINT = 2.0F;		/* 216 -->   0.0   (0) */

const float B_DISABLED_LABEL_TINT = B_DARKEN_3_TINT;
const float B_HIGHLIGHT_BACKGROUND_TINT	= B_DARKEN_2_TINT;
const float B_DISABLED_MARK_TINT = B_LIGHTEN_2_TINT;

status_t get_key_info (key_info *keyInfo);
status_t get_deskbar_frame(BRect *frame);
rgb_color ui_color(color_which which);
rgb_color tint_color(rgb_color color, float tint);


#endif
