/*
	Contains constants and struct definitions relevant to the YEG map for
	this assignment.
*/

#ifndef _YEG_MAP_H_
#define _YEG_MAP_H_

#include "lcd_image.h"

struct MapView {
	int16_t cursorX, cursorY; // cursor pixel position on the screen
	int16_t mapX, mapY;       // upper-left pixel of the .lcd to display
};

#define MAPWIDTH  2048
#define MAPHEIGHT 2048
#define LATNORTH  5361858l
#define LATSOUTH  5340953l
#define LONWEST   -11368652l
#define LONEAST   -11333496l

// Conversion routines to convert between x/y and lon/lat coordinates
int32_t x_to_lon(int16_t x);
int32_t y_to_lat(int16_t y);
int16_t lon_to_x(int32_t lon);
int16_t lat_to_y(int32_t lat);

#endif
