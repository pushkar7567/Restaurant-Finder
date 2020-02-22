#include "restaurant.h"

/*
	Sets *ptr to the i'th restaurant. If this restaurant is already in the cache,
	it just copies it directly from the cache to *ptr. Otherwise, it fetches
	the block containing the i'th restaurant and stores it in the cache before
	setting *ptr to it.
*/
void getRestaurant(restaurant* ptr, int i, Sd2Card* card, RestCache* cache) {
	// calculate the block with the i'th restaurant
	uint32_t block = REST_START_BLOCK + i/8;

	// if this is not the cached block, read the block from the card
	if (block != cache->cachedBlock) {
		while (!card->readBlock(block, (uint8_t*) cache->block)) {
			Serial.print("readblock failed, try again");
		}
		cache->cachedBlock = block;
	}

	// either way, we have the correct block so just get the restaurant
	*ptr = cache->block[i%8];
}

// Swaps the two restaurants (which is why they are pass by reference).
void swap(RestDist& r1, RestDist& r2) {
	RestDist tmp = r1;
	r1 = r2;
	r2 = tmp;
}

// Insertion sort to sort the restaurants.
void insertionSort(RestDist restaurants[], int counter) {
	// Invariant: at the start of iteration i, the
	// array restaurants[0 .. i-1] is sorted.
	for (int i = 1; i < counter; ++i) {
		// Swap restaurant[i] back through the sorted list restaurants[0 .. i-1]
		// until it finds its place.
		for (int j = i; j > 0 && restaurants[j].dist < restaurants[j-1].dist; --j) {
			swap(restaurants[j-1], restaurants[j]);
		}
	}
}

// Computes the manhattan distance between two points (x1, y1) and (x2, y2).
int16_t manhattan(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
	return abs(x1-x2) + abs(y1-y2);
}

// Pivot function which decides the pivot for each recursive call
int pivot(RestDist restaurants[], int n, int pi){
	int lo=0;
	int hi=n-2;

	swap(restaurants[pi], restaurants[n-1]);

	while (lo<=hi){
		if (restaurants[lo].dist <= restaurants[n-1].dist){
			lo++;
		}

		else if (restaurants[hi].dist > restaurants[n-1].dist){
			hi--;
		}

		else {
			swap(restaurants[lo], restaurants[hi]);
		}
	}
	swap(restaurants[lo], restaurants[n-1]);
	return lo;

} 

// qsort implementation to sort restaurants
void quickSort(RestDist restaurants[], int n){
	if (n<=1){
		return;
	}
	int pivot_index = n/2;
	int new_pi = pivot(restaurants, n, pivot_index);

	quickSort(restaurants, new_pi);
	quickSort(restaurants+new_pi, n - new_pi);

}


/*
	Fetches all restaurants from the card, saves their RestDist information
	in restaurants[], and then sorts them based on their distance to the
	point on the map represented by the MapView. Uses two more arguments
	starNum to get restaurants with threshold star rating and sortNum for 
	the type of sorting
*/
void getAndSortRestaurants(const MapView& mv, RestDist restaurants[], Sd2Card* card, RestCache* cache, uint16_t starNum, uint16_t sortNum) {
	restaurant r;
	// time variables to time the sorting
	int time1;
	int time2;
	int time3;
	int time4;
	// counter variable to keep track of the total number of restaurants
	int counter = 0;

	if (sortNum == 0){
		for (int i = 0; i < NUM_RESTAURANTS; ++i) {
		getRestaurant(&r, i, card, cache);
		int restRating = max(floor((r.rating + 1)/2), 1);
			if (restRating >= starNum){
				restaurants[counter].index = i;
				restaurants[counter].dist = manhattan(lat_to_y(r.lat), lon_to_x(r.lon),
																		mv.mapY + mv.cursorY, mv.mapX + mv.cursorX);
				counter++;
			}	
		}

		time1 = millis();
		quickSort(restaurants, counter);
		time2 = millis();
		Serial.print("qsort ");
		Serial.print(counter);
		Serial.print(" restaurants: ");
		Serial.print(time2-time1);
		Serial.println(" ms");
	}

	else if (sortNum == 1){
		for (int i = 0; i < NUM_RESTAURANTS; ++i) {
		getRestaurant(&r, i, card, cache);
		int restRating = max(floor((r.rating + 1)/2), 1);
			if (restRating >= starNum){
				restaurants[counter].index = i;
				restaurants[counter].dist = manhattan(lat_to_y(r.lat), lon_to_x(r.lon),
																		mv.mapY + mv.cursorY, mv.mapX + mv.cursorX);
				counter++;
			}	
		}

		time1 = millis();
		insertionSort(restaurants, counter);
		time2 = millis();
		Serial.print("isort ");
		Serial.print(counter);
		Serial.print(" restaurants: ");
		Serial.print(time2-time1);
		Serial.println(" ms");
	}

	else if (sortNum == 2){
		for (int i = 0; i < NUM_RESTAURANTS; ++i) {
		getRestaurant(&r, i, card, cache);
		int restRating = max(floor((r.rating + 1)/2), 1);
			if (restRating >= starNum){
				restaurants[counter].index = i;
				restaurants[counter].dist = manhattan(lat_to_y(r.lat), lon_to_x(r.lon),																		mv.mapY + mv.cursorY, mv.mapX + mv.cursorX);
				counter++;
			}	
		}

		time1 = millis();
		quickSort(restaurants, counter);
		time2 = millis();
		
		Serial.print("qsort ");
		Serial.print(counter);
		Serial.print(" restaurants: ");
		Serial.print(time2-time1);
		Serial.println(" ms");

		counter = 0;

		for (int i = 0; i < NUM_RESTAURANTS; ++i) {
		getRestaurant(&r, i, card, cache);
		int restRating = max(floor((r.rating + 1)/2), 1);
			// Only store the restaurants if they are equal to or above the threshold rating
			if (restRating >= starNum){
				restaurants[counter].index = i;
				restaurants[counter].dist = manhattan(lat_to_y(r.lat), lon_to_x(r.lon),
																		mv.mapY + mv.cursorY, mv.mapX + mv.cursorX);
				counter++;
			}	
		}

		time3 = millis();
		insertionSort(restaurants, counter);
		time4 = millis();
		Serial.print("isort ");
		Serial.print(counter);
		Serial.print(" restaurants: ");
		Serial.print(time4-time3);
		Serial.println(" ms");
	}

	
}


