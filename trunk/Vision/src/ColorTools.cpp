/*******************************************************************************
/
/	File:			ColorTools.cpp
/
/   Description:    Additional experimental color manipulation functions.
/
/	Copyright 2000, Be Incorporated, All Rights Reserved
/
*******************************************************************************/

#include "ColorTools.h"

#if B_BEOS_VERSION <= B_BEOS_VERSION_MAUI

namespace BExperimental {

#if DEBUG
#define DB_INLINE
#else
#define DB_INLINE inline
#endif

static DB_INLINE void mix_color_func(rgb_color* target, const rgb_color other, uint8 amount)
{
	target->red = (uint8)( ((int16(other.red)-int16(target->red))*amount)/255
								+ target->red );
	target->green = (uint8)( ((int16(other.green)-int16(target->green))*amount)/255
								+ target->green );
	target->blue = (uint8)( ((int16(other.blue)-int16(target->blue))*amount)/255
								+ target->blue );
	target->alpha = (uint8)( ((int16(other.alpha)-int16(target->alpha))*amount)/255
								+ target->alpha );
}

static DB_INLINE void blend_color_func(rgb_color* target, const rgb_color other, uint8 amount)
{
	const uint8 alphaMix = (uint8)( ((int16(other.alpha)-int16(255-target->alpha))*amount)/255
									+ (255-target->alpha) );
	target->red = (uint8)( ((int16(other.red)-int16(target->red))*alphaMix)/255
								+ target->red );
	target->green = (uint8)( ((int16(other.green)-int16(target->green))*alphaMix)/255
								+ target->green );
	target->blue = (uint8)( ((int16(other.blue)-int16(target->blue))*alphaMix)/255
								+ target->blue );
	target->alpha = (uint8)( ((int16(other.alpha)-int16(target->alpha))*amount)/255
								+ target->alpha );
}

static DB_INLINE void disable_color_func(rgb_color* target, const rgb_color background)
{
	blend_color_func(target, background, 255-70);
}

// --------------------------------------------------------------------------

rgb_color mix_color(rgb_color color1, rgb_color color2, uint8 amount)
{
	mix_color_func(&color1, color2, amount);
	return color1;
}

rgb_color blend_color(rgb_color color1, rgb_color color2, uint8 amount)
{
	blend_color_func(&color1, color2, amount);
	return color1;
}

rgb_color disable_color(rgb_color color, rgb_color background)
{
	disable_color_func(&color, background);
	return color;
}

}

#endif
