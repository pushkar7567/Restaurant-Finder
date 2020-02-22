/*
	Contains the restaurant struct declaration, functions to read restaurants
	from the SD card, and functions to sort a restaurant around a given
	latitude/longitude
*/

#ifndef _READ_REST_H_
#define _READ_REST_H_

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "restaurant.h"
#include "yegmap.h"

#define REST_START_BLOCK 4000000
#define NUM_RESTAURANTS  1066

// The same restaurant struct we discussed in class.
struct restaurant {
  int32_t lat;
  int32_t lon;
  uint8_t rating;
  char name[55];
};

// Struct to hold a block of restaurants and the index of the block..
struct RestCache {
  uint32_t cachedBlock;
  restaurant block[8];
};

// Struct to hold the index and "distance to cursor" for a restaurant,
// for the purposes of loading into main memory for sorting.
struct RestDist {
  uint16_t index; // Index of restaurant from 0 to NUM_RESTAURANTS-1.
  uint16_t dist;  // Manhatten distance to cursor position.
};


// Get the i'th restaurant from the SD card and store at the pointer location.
// Assumes *card has been initialized for raw reads.
void getRestaurant(restaurant* ptr, int i, Sd2Card* card, RestCache* cache);

// Sort the restaurants around the cursor represented by the mapview.
// Will actually just sort the restDist array.
// Assumes *card has been initialized for raw reads.
void getAndSortRestaurants(const MapView& mv, RestDist restaurants[],
                           Sd2Card* card, RestCache* cache, uint16_t starNum, uint16_t sortNum);

#endif
