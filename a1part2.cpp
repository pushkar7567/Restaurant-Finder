//------------------------------------------------------
// Names: Amro Amanuddein and Pushkar Sabharwal
// ID's: 1572498 (Amro) 1588927 (Pushkar)
// CMPUT275, Winter 2020
//
// Assignment 1 Part 2: Restaurant Finder
//------------------------------------------------------

#include <Arduino.h>
#include <MCUFRIEND_kbv.h>
#include <SPI.h>
#include <SD.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>
#include "lcd_image.h"
#include "yegmap.h"
#include "restaurant.h"

// SD_CS pin for SD card reader
#define SD_CS 10

// joystick pins
#define JOY_VERT_ANALOG  A9
#define JOY_HORIZ_ANALOG A8
#define JOY_SEL 53

// width/height of the display when rotated horizontally
#define TFT_WIDTH 480
#define TFT_HEIGHT 320

// layout of display for this assignment,
#define RATING_SIZE 60
#define DISP_WIDTH (TFT_WIDTH - RATING_SIZE)
#define DISP_HEIGHT TFT_HEIGHT

// constants for the joystick
#define JOY_DEADZONE 64
#define JOY_CENTRE 512
#define JOY_STEPS_PER_PIXEL 64

// touch screen pins, obtained from the documentaion
#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM  9  // can be a digital pin
#define XP  8  // can be a digital pin

// calibration data for the touch screen, obtained from documentation
// the minimum/maximum possible readings from the touch point
#define TS_MINX 100
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// thresholds to determine if there was a touch
#define MINPRESSURE  100
#define MAXPRESSURE 1000

// Cursor size. For best results, use an odd number.
#define CURSOR_SIZE 9

// number of restaurants to display
#define REST_DISP_NUM 21

#define RED     0xF800
#define GREEN   0x0F00

// ********** BEGIN GLOBAL VARIABLES ************
MCUFRIEND_kbv tft;
Sd2Card card;

// A multimeter reading says there are 300 ohms of resistance across the plate,
// so initialize with this to get more accurate readings.
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

// The currently selected restaurant, if we are in mode 1.
// It's arguable if this needs to be a global variable, but we'll let this
// be one of the "few" that are allowed. As long as we don't clutter the global
// space with too many variables.
int selectedRest;

// which mode are we in?
enum DisplayMode { MAP, MENU } displayMode;

// the current map view and the previous one from last cursor movement
MapView curView, preView;

// For sorting and displaying the restaurants, will hold the restaurant RestDist
// information for the most recent click in sorted order.
RestDist restaurants[NUM_RESTAURANTS];

lcd_image_t edmontonBig = { "yeg-big.lcd", MAPWIDTH, MAPHEIGHT };

// The cache of 8 restaurants for the getRestaurant function.
RestCache cache;
uint16_t screenNum=0;
uint16_t sortNum=0;
uint16_t starNum=1;
int maxScreen;

// ************ END GLOBAL VARIABLES ***************

// Forward declaration of functions to begin the modes. Setup uses one, so
// it seems natural to forward declare both (not really that important).
void beginMode0();
void beginMode1();

void setup() {
	init();

	Serial.begin(9600);

	// joystick button initialization
	pinMode(JOY_SEL, INPUT_PULLUP);

	// tft display initialization
	uint16_t ID = tft.readID();
	tft.begin(ID);
	tft.setRotation(1);
	tft.setTextWrap(false);

	// now initialize the SD card in both modes
  Serial.print("Initializing SD card...");

	// Initialize for reading through the FAT filesystem
	// (required for lcd_image drawing function).
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
    Serial.println("Is the card inserted properly?");
    while (true) {}
  }

	// Also initialize the SD card for raw reads.
  Serial.print("Initializing SPI communication for raw reads...");
  if (!card.init(SPI_HALF_SPEED, SD_CS)) {
    Serial.println("failed!");
    while (true) {}
  }

  Serial.println("OK!");

  // initial cursor position is the centre of the screen
  curView.cursorX = DISP_WIDTH/2;
  curView.cursorY = DISP_HEIGHT/2;

  // initial map position is the middle of Edmonton
  curView.mapX = ((MAPWIDTH / DISP_WIDTH)/2) * DISP_WIDTH;
  curView.mapY = ((MAPHEIGHT / DISP_HEIGHT)/2) * DISP_HEIGHT;

	// This ensures the first getRestaurant() will load the block as all blocks
	// will start at REST_START_BLOCK, which is 4000000.
	cache.cachedBlock = 0;
	// will draw the initial map screen and other stuff on the display

  beginMode0();
}

// Draw the map patch of edmonton over the preView position, then
// draw the red cursor at the curView position.
void moveCursor() {
	lcd_image_draw(&edmontonBig, &tft, preView.mapX + preView.cursorX - CURSOR_SIZE/2,
							 	 preView.mapY + preView.cursorY - CURSOR_SIZE/2,
							   preView.cursorX - CURSOR_SIZE/2, preView.cursorY - CURSOR_SIZE/2,
								 CURSOR_SIZE, CURSOR_SIZE);

	tft.fillRect(curView.cursorX - CURSOR_SIZE/2, curView.cursorY - CURSOR_SIZE/2,
							 CURSOR_SIZE, CURSOR_SIZE, TFT_RED);
}

// Set the mode to 0 and draw the map and cursor according to curView
void beginMode0() {
	// Black out the rating selector part (less relevant in Assignment 1, but
	// it is useful when you first start the program).
	tft.fillRect(DISP_WIDTH, 0, RATING_SIZE, DISP_HEIGHT, TFT_BLACK);

	// Draw the current part of Edmonton to the tft display.
	lcd_image_draw(&edmontonBig, &tft, curView.mapX, curView.mapY, 0, 0, DISP_WIDTH, DISP_HEIGHT);
	// just the initial draw of the cursor on the map
	moveCursor();

	tft.fillRect(420, 0, 60, 159, TFT_WHITE);
  	tft.fillRect(420, 161, 60, 159, TFT_WHITE);
	tft.drawRect(420, 0, 60, 159, RED);
  	tft.drawRect(420, 161, 60, 159, GREEN);
  	tft.setCursor(445, 90);
  	tft.setTextSize(2);
  	tft.println(starNum);
  	tft.setTextColor(TFT_BLACK);

  	// Sortnum represents type of sorting
  	if (sortNum == 0){
  		char array0[] = "QSORT";
  		tft.fillRect(422,165,55,120, TFT_WHITE);
  		tft.drawRect(420, 0, 60, 159, GREEN);
  		for (int i=0; i<6; ++i){  
  			tft.setCursor(445, 200+20*i);
   			tft.println(array0[i]);
  		}
  	}

  	else if (sortNum == 1){
  		char array1[] = "ISORT";
  		tft.fillRect(422,165,55,120, TFT_WHITE);
  		tft.drawRect(420, 161, 60, 159, GREEN);
  		for (int i=0; i<6; ++i){  
  			tft.setCursor(445, 200+20*i);
   			tft.println(array1[i]);
  		}
  	}

  	else if(sortNum == 2){
  		char array3[] = "BOTH";
  		tft.fillRect(422,165,55,120, TFT_WHITE);
  		tft.drawRect(420, 161, 60, 159, GREEN);
  		for (int i=0; i<5; ++i){  
  			tft.setCursor(445, 200+20*i);
   			tft.println(array3[i]);
  		}
  	}
		
	// starNum represnts the number of stars of restaurants		
  	tft.fillRect(420, 0, 60, 159, TFT_WHITE);
  	tft.drawRect(420, 0, 60, 159, RED);
  	tft.setCursor(445, 90);
  	tft.println(starNum);

	displayMode = MAP;
}

// Print the i'th restaurant in the sorted list.
// Assumes 0 <= i < 21 for part 1.
void printRestaurant(int i) {
	restaurant r;
	tft.setTextSize(1);

	// get the i'th restaurant
	getRestaurant(&r, restaurants[i].index, &card, &cache);

	// Set its colour based on whether or not it is the selected restaurant.
	if (i%30 != selectedRest) {
		tft.setTextColor(TFT_WHITE, TFT_BLACK);
	}
	else {
		tft.setTextColor(TFT_BLACK, TFT_WHITE);
	}
	tft.setCursor(0, (i%30)*10);
	tft.print(r.name);
}

// Begin mode 1 by sorting the restaurants around the cursor
// and then displaying the list.
void beginMode1() {
	tft.setCursor(0, 0);
	tft.fillScreen(TFT_BLACK);
	tft.setTextSize(2);

	// Get the RestDist information for this cursor position and sort it.
	getAndSortRestaurants(curView, restaurants, &card, &cache, starNum, sortNum);

	// Initially have the closest restaurant highlighted.
	selectedRest = 0;

	// Print the list of restaurants.
	for (int i = 0; i < 30; ++i) {
		printRestaurant(i);
	}

	displayMode = 1;

}

// Checks if the edge was nudged and scrolls the map if it was.
void checkRedrawMap() {
  // A flag to indicate if we scrolled.
	bool scroll = false;

	// If we nudged the left or right edge, shift the map over.
	if (curView.cursorX == DISP_WIDTH-CURSOR_SIZE/2-1 && curView.mapX != MAPWIDTH - DISP_WIDTH) {
		curView.mapX += DISP_WIDTH;
		curView.cursorX = DISP_WIDTH/2;
		scroll = true;
	}
	else if (curView.cursorX == CURSOR_SIZE/2 && curView.mapX != 0) {
		 curView.mapX -= DISP_WIDTH;
		 curView.cursorX = DISP_WIDTH/2;
		 scroll = true;
	}

	// If we nudged the top or bottom edge, shift the map up or down.
	if (curView.cursorY == DISP_HEIGHT-CURSOR_SIZE/2-1 && curView.mapY != MAPHEIGHT - DISP_HEIGHT) {
		curView.mapY += DISP_HEIGHT;
		curView.cursorY = DISP_HEIGHT/2;
		scroll = true;
	}
	else if (curView.cursorY == CURSOR_SIZE/2 && curView.mapY != 0) {
		curView.mapY -= DISP_HEIGHT;
		curView.cursorY = DISP_HEIGHT/2;
		scroll = true;
	}

	// If we nudged the edge, recalculate and draw the new rectangular portion of Edmonton to display.
	if (scroll) {
		// Make sure we didn't scroll outside of the map.
		curView.mapX = constrain(curView.mapX, 0, MAPWIDTH - DISP_WIDTH);
		curView.mapY = constrain(curView.mapY, 0, MAPHEIGHT - DISP_HEIGHT);

		lcd_image_draw(&edmontonBig, &tft, curView.mapX, curView.mapY, 0, 0, DISP_WIDTH, DISP_HEIGHT);
	}
}

// Process joystick and touchscreen input when in mode 0.
void scrollingMap() {
  int v = analogRead(JOY_VERT_ANALOG);
  int h = analogRead(JOY_HORIZ_ANALOG);
  int invSelect = digitalRead(JOY_SEL);

	// A flag to indicate if the cursor moved or not.
	bool cursorMove = false;

  // If there was vertical movement, then move the cursor.
  if (abs(v - JOY_CENTRE) > JOY_DEADZONE) {
    // First move the cursor.
    int delta = (v - JOY_CENTRE) / JOY_STEPS_PER_PIXEL;
		// Clamp it so it doesn't go outside of the screen.
    curView.cursorY = constrain(curView.cursorY + delta, CURSOR_SIZE/2, DISP_HEIGHT-CURSOR_SIZE/2-1);
		// And now see if it actually moved.
		cursorMove |= (curView.cursorY != preView.cursorY);
  }

	// If there was horizontal movement, then move the cursor.
  if (abs(h - JOY_CENTRE) > JOY_DEADZONE) {
    // Ideas are the same as the previous if statement.
    int delta = -(h - JOY_CENTRE) / JOY_STEPS_PER_PIXEL;
    curView.cursorX = constrain(curView.cursorX + delta, CURSOR_SIZE/2, DISP_WIDTH-CURSOR_SIZE/2-1);
		cursorMove |= (curView.cursorX != preView.cursorX);
  }

	// If the cursor actually moved.
	if (cursorMove) {
		// Check if the map edge was nudged, and move it if so.
		checkRedrawMap();

		preView.mapX = curView.mapX;
		preView.mapY = curView.mapY;

		// Now draw the cursor's new position.
		moveCursor();
	}

	preView = curView;

	// Did we click the joystick?
  if(invSelect == LOW){
		beginMode1();
    displayMode = MENU;
    Serial.println(displayMode);
    Serial.println("MODE changed.");

		// Just to make sure the restaurant is not selected by accident
		// because the button was pressed too long.
		while (digitalRead(JOY_SEL) == LOW) { delay(10); }
  }

	// Check for touchscreen press and draws dots for each restaurant
	TSPoint touch = ts.getPoint();

	// Necessary to resume TFT display functions
  pinMode(YP, OUTPUT);
  pinMode(XM, OUTPUT);


	// If there was an actual touch, draw the dots
	if (touch.z >= MINPRESSURE && touch.z <= MAXPRESSURE) {
		restaurant r;
		int ptx = map(touch.y, TS_MINX, TS_MAXX, 0, tft.width());
        int pty = map(touch.x, TS_MINY, TS_MAXY, 0, tft.height());

        if (((ptx > 60) && (pty < 160))){
		// just iterate through all restaurants on the card
			for (int i = 0; i < NUM_RESTAURANTS; ++i) {
				getRestaurant(&r, i, &card, &cache);
				if (r.rating >= starNum){
					int16_t rest_x_tft = lon_to_x(r.lon)-curView.mapX, rest_y_tft = lat_to_y(r.lat)-curView.mapY;
					// only draw if entire radius-3 circle will be in the map display
					if (rest_x_tft >= 3 && rest_x_tft < DISP_WIDTH-3 &&  rest_y_tft >= 3 && rest_y_tft < DISP_HEIGHT-3) {
						tft.fillCircle(rest_x_tft, rest_y_tft, 3, TFT_BLUE);
					}
				}	
			}
		}

		// Increment starNum or sortNum which toucn of button
		else if ((ptx < 60) && (pty < 160)){
			sortNum+=1;
			sortNum = (sortNum%3);
			delay(100);
  			tft.setCursor(445, 90);
  			tft.println(starNum);

  			if (sortNum == 0){
  				char array0[] = "QSORT";
  				tft.fillRect(422,165,55,120, TFT_WHITE);
  				tft.drawRect(420, 161, 60, 159, GREEN);
  				for (int i=0; i<6; ++i){  
  					tft.setCursor(445, 200+20*i);
   					tft.println(array0[i]);
  				}
  			}

  			else if (sortNum == 1){
  				char array1[] = "ISORT";
  				tft.fillRect(422,165,55,120, TFT_WHITE);
  				tft.drawRect(420, 161, 60, 159, GREEN);
  				for (int i=0; i<6; ++i){  
  					tft.setCursor(445, 200+20*i);
   					tft.println(array1[i]);
  				}
  			}

  			else if(sortNum == 2){
  				char array3[] = "BOTH";
  				tft.fillRect(422,165,55,120, TFT_WHITE);
  				tft.drawRect(420, 161, 60, 159, GREEN);
  				for (int i=0; i<4; ++i){  
  					tft.setCursor(445, 200+20*i);
   					tft.println(array3[i]);
  				}
  			}		

		}

		else if ((ptx < 60) && (pty > 160)){
			starNum+=1;
			starNum= starNum%6;
			delay(100);
			if (starNum==0){
				starNum+=1;
			}
			
  			tft.fillRect(420, 0, 60, 159, TFT_WHITE);
  			tft.drawRect(420, 0, 60, 159, RED);
  			tft.setCursor(445, 90);
  			tft.println(starNum);
		}
	}
}

// Process joystick movement when in mode 1.
void scrollingMenu() {
	int oldRest = selectedRest;

	int v = analogRead(JOY_VERT_ANALOG);

	// maxscreen represents the max number of screens for given number of restaurants
	if (starNum == 1){
		maxScreen = 35;
	}

	else if (starNum == 2){
		maxScreen = 34;
	}

	else if (starNum == 3){
		maxScreen = 31;
	}

	else if (starNum == 4){
		maxScreen = 21;
	}

	else if (starNum == 5){
		maxScreen = 5;
	}

	// if the joystick was pushed up or down, change restaurants accordingly.
	if (v > JOY_CENTRE + JOY_DEADZONE) {
		++selectedRest;
	}
	else if (v < JOY_CENTRE - JOY_DEADZONE) {
		--selectedRest;
	}

	// Going to next page or previous restaurant page if conditions satisfied
	if (selectedRest==30 && (v > JOY_CENTRE + JOY_DEADZONE)){
		if (screenNum!=maxScreen)	
			screenNum += 1;
			tft.fillScreen(TFT_BLACK);
			for (int i=0; i<30; i++){
				printRestaurant(screenNum*30 + i);
					
			}
			selectedRest=0;
	}

	else if (selectedRest==-1 && (v < JOY_CENTRE - JOY_DEADZONE)){
		if (screenNum!=0){	
			screenNum -= 1;
			tft.fillScreen(TFT_BLACK);
			for (int i=0; i<30; i++){
				printRestaurant(screenNum*30 + i);
			}
			selectedRest=29;
		}	
	}

	selectedRest = constrain(selectedRest, 0, NUM_RESTAURANTS-1);

	// If we picked a new restaurant, update the way it and the previously
	// selected restaurant are displayed.
	if (oldRest != selectedRest) {
		printRestaurant(oldRest+(screenNum*30));
		printRestaurant(selectedRest+(screenNum*30));
		delay(50); // so we don't scroll too fast
	}

	// If we clicked on a restaurant.
	if (digitalRead(JOY_SEL) == LOW) {
		restaurant r;
		getRestaurant(&r, restaurants[selectedRest+(screenNum*30)].index, &card, &cache);
		// Calculate the new map view.

		// Center the map view at the restaurant, constraining against the edge of
		// the map if necessary.
		curView.mapX = constrain(lon_to_x(r.lon)-DISP_WIDTH/2, 0, MAPWIDTH-DISP_WIDTH);
		curView.mapY = constrain(lat_to_y(r.lat)-DISP_HEIGHT/2, 0, MAPHEIGHT-DISP_HEIGHT);

		// Draw the cursor, clamping to an edge of the map if needed.
		curView.cursorX = constrain(lon_to_x(r.lon) - curView.mapX, CURSOR_SIZE/2, DISP_WIDTH-CURSOR_SIZE/2-1);
		curView.cursorY = constrain(lat_to_y(r.lat) - curView.mapY, CURSOR_SIZE/2, DISP_HEIGHT-CURSOR_SIZE/2-1);

		preView = curView;

		beginMode0();

		// Ensures a long click of the joystick will not register twice.
		while (digitalRead(JOY_SEL) == LOW) { delay(10); }
	}
}

int main() {
	setup();
	// All the implementation work is done now, just have a loop that processes
	// the appropriate function!
	while (true) {
		if (displayMode == MAP) {
			scrollingMap();
		}
		else {
			scrollingMenu();
		}
	}

	Serial.end();
	return 0;
}
