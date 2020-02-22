#include "yegmap.h"

// These will convert between pixel coordiantes on the yeg-big.lcd map
// and geographic coordinates. They are from the assignment description.

int32_t x_to_lon(int16_t x) {
  return map(x, 0, MAPWIDTH, LONWEST, LONEAST);
}

int32_t y_to_lat(int16_t y) {
  return map(y, 0, MAPHEIGHT, LATNORTH, LATSOUTH);
}

int16_t lon_to_x(int32_t lon) {
  return map(lon, LONWEST, LONEAST, 0, MAPWIDTH);
}

int16_t lat_to_y(int32_t lat) {
  return map(lat, LATNORTH, LATSOUTH, 0, MAPHEIGHT);
}
