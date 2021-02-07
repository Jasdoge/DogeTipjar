#include "DisplayManager.h"
#include <WiFi.h>

// Initialization
uint8_t DisplayManager::MENU = DisplayManager::MENU_NONE;
TFT_eSPI DisplayManager::tft = TFT_eSPI();
uint32_t DisplayManager::lastTouch = 0;
uint16_t DisplayManager::wifi_buttons[5] = {0};
String DisplayManager::cache_SSID;				// Stores SSID when configuring
String DisplayManager::cache_passwd;			// Stores password when configuring
void (*DisplayManager::cache_wifiCallback)(String ssid, String passwd);
uint16_t DisplayManager::keypad_btns[KEYBOARD_LENGTH*2] = {0};	// X/Y 
bool DisplayManager::keyboardUpperCase = false;


void DisplayManager::setup(){

	// TFT loading
	tft.init();
	tft.setRotation(1);
	tft.loadFont("ComicSans");	
	tft.setTextDatum(MC_DATUM);
	tft.setTextColor(TFT_YELLOW);
	tft.fillScreen(0);
	tft.drawString("Loading", 160, 130);

	/*
	Serial.println("Drawing QR");
	drawQrCode();
	*/

}




void DisplayManager::loop(){

	const uint32_t ms = millis();

	uint16_t t_x = 0, t_y = 0; // To store the touch coordinates

	// Touchscreen handler 

  	// Pressed will be set true is there is a valid touch on the screen
  	bool pressed = tft.getTouch(&t_x, &t_y);
	if( pressed ){

		const uint32_t l = lastTouch;
		lastTouch = ms;
		if( l )
			return;

		t_y = 240-t_y;	// May need to comment this out

		Serial.printf("%ix, %iy\n", t_x, t_y);

		// WiFi SSID Listing
		if( MENU == MENU_SSID ){

			// Scan the buttons
			for( uint8_t i = 0; i < 5; ++i ){

				uint16_t y = wifi_buttons[i];
				if( !y )
					break;
				if( 
					t_x > WIFI_BUTTON_LEFT && t_x < WIFI_BUTTON_WIDTH+WIFI_BUTTON_LEFT &&
					t_y > y && t_y < y+WIFI_BUTTON_HEIGHT-WIFI_BUTTON_PADDING_BOTTOM
				){

					cache_SSID = WiFi.SSID(i);
					Serial.printf("CLICKED BUTTON: %i\n", i);
					Serial.println(cache_SSID);

					if( WiFi.encryptionType(i) == WIFI_AUTH_OPEN ){
						cache_wifiCallback(cache_SSID, "");
					}
					else{
						setScreenNetworkPassword();
					}

					break;

				}

			} 

		}

		// Password keypad scanning
		else if( MENU == MENU_PASSWD ){

			if( 
				t_x > KEYBOARD_DONE_BTN_POS_X && t_x < KEYBOARD_DONE_BTN_POS_X+KEYBOARD_DONE_BTN_WIDTH &&
				t_y > KEYBOARD_DONE_BTN_POS_Y && t_y < KEYBOARD_DONE_BTN_POS_Y+KEYBOARD_DONE_BTN_HEIGHT
			){
				
				tft.fillRect(KEYBOARD_DONE_BTN_POS_X, KEYBOARD_DONE_BTN_POS_Y, KEYBOARD_DONE_BTN_WIDTH, KEYBOARD_DONE_BTN_HEIGHT, 0xFFFF);
				delay(50);
				cache_wifiCallback(cache_SSID, cache_passwd);
				cache_SSID = cache_passwd = "";

				return;
			}

			for( uint8_t i = 0; i < KEYBOARD_LENGTH; ++i ){

				const uint16_t x = keypad_btns[i*2];
				const uint16_t y = keypad_btns[i*2+1];

				if( 
					t_x > x && t_x < x+KEYBOARD_BTN_WIDTH &&
					t_y > y && t_y < y+KEYBOARD_BTN_HEIGHT
				){

					tft.fillRect(x, y, KEYBOARD_BTN_WIDTH, KEYBOARD_BTN_HEIGHT, 0xFFFF);
					delay(50);

					const char *keymap = keyboardUpperCase ? KEYBOARD_MAP_UPPERCASE : KEYBOARD_MAP_LOWERCASE;
					char pushed = keymap[i];
					if( pushed == '^' ){
						keyboardUpperCase = !keyboardUpperCase;
					}
					else if( pushed == '<' && cache_passwd.length() ){
						cache_passwd = cache_passwd.substring(0, cache_passwd.length()-1);
						drawPasswordStars();
					}
					else{
						cache_passwd += String(pushed);
						drawPasswordStars();
					}
					drawKeyboard();

					break;

				}

			}


		}


	}
	else if( ms-lastTouch > 100 )
		lastTouch = 0;


}



// SCREENS

// Connecting screen
void DisplayManager::setScreenConnecting(){
	
	MENU = MENU_NONE;
	tft.setTextColor(TFT_YELLOW);
	tft.setTextSize(2);
	tft.fillScreen(TFT_BLACK);
	tft.drawString("Connecting", CENTER, 20);

}
void DisplayManager::setScreenConnectingTicks( uint8_t ticks ){
	
	tft.fillRect(0, 100, 320, 40, TFT_BLACK);
	uint16_t start = CENTER-ticks*6/2;
	for( uint8_t i = 0; i < ticks; ++i ){

		tft.fillRect(start+1+i*6, 120, 4, 4, 0xFFFF);

	}

}
	
void DisplayManager::setScreenConnectionFailed(){
	MENU = MENU_NONE;
	tft.fillScreen(TFT_BLACK);
	tft.drawString("Connection failed", CENTER, 20);
		
}


void DisplayManager::setScreenDogeReceived( uint32_t amount ){
	MENU = MENU_NONE;

	drawBmp("/doge_received.bmp");

	tft.loadFont("ComicSans");
	tft.setTextDatum(MC_DATUM);
	tft.setTextColor(TFT_YELLOW);
	
	tft.drawNumber(amount, 160, 130-40);
	tft.drawString("DOGE!", 160, 130+40);
	tft.unloadFont();

}

void DisplayManager::setScreenStats(){
	MENU = MENU_DEFAULT;
	drawBmp("/wallpaper.bmp");

}

void DisplayManager::setScreenQR(){
	MENU = MENU_DEFAULT;

	drawQrCode();

}

void DisplayManager::setScreenNetworkScan( void(*wifiCallback)(String ssid, String passwd) ){

	cache_wifiCallback = wifiCallback;

	MENU = MENU_SSID;
	const uint8_t x = CENTER;

	tft.setTextColor(TFT_YELLOW);
	tft.fillScreen(TFT_BLACK);
	
	tft.setTextFont(2);
	tft.setTextSize(2);
	tft.drawString("Scanning", x, 20);

	uint8_t n = WiFi.scanNetworks();
	tft.fillScreen(TFT_BLACK);

	if( !n ){
		tft.drawString("No networks found", x, 100);
		tft.drawString("Try restarting", x, 150);
		return;
	}
	

	tft.drawString("Pick a network", x, 20);

	tft.setTextSize(1.5);
	// max 5 networks
	for( uint8_t i = 0; i < 5; ++i ){

		// Reset to 0
		if( i >= n ){
			wifi_buttons[i] = 0;
			continue;
		}

		const uint8_t posY = WIFI_BUTTON_TOP + WIFI_BUTTON_HEIGHT*i;
		wifi_buttons[i] = posY;

		tft.fillRect(WIFI_BUTTON_LEFT, posY, WIFI_BUTTON_WIDTH, WIFI_BUTTON_HEIGHT-WIFI_BUTTON_PADDING_BOTTOM, BUTTON_COLOR);
		String name = WiFi.SSID(i) + " (" + String(100+WiFi.RSSI(i)) + "%)";
		tft.drawString(name.substring(0,30), x, posY+BUTTON_TEXT_PADDING_TOP);

	}

}
void DisplayManager::setScreenNetworkPassword(){

	MENU = MENU_PASSWD;

	tft.fillScreen(TFT_BLACK);

	tft.setTextFont(2);
	tft.setTextSize(2);
	tft.setTextColor(TFT_YELLOW);


	tft.drawString("Enter password", CENTER, 20);
	drawPasswordStars();
	drawKeyboard();

}







// TOOLS
// Graphics
// Use hardware SPI
void DisplayManager::drawBmp(const char *filename, int16_t x, int16_t y) {

	if( (x >= tft.width()) || (y >= tft.height()) ){
		Serial.println("Out of bounds error");
		return;
	}
	fs::File bmpFS;

	// Open requested file on SD card
	bmpFS = SPIFFS.open(filename, "r");

	if( !bmpFS ){
		Serial.println("File not found");
		return;
	}

	uint32_t seekOffset;
	uint16_t w, h, row, col;
	uint8_t  r, g, b;

	uint32_t startTime = millis();

	if( read16(bmpFS) == 0x4D42 ){
		read32(bmpFS);
		read32(bmpFS);
		seekOffset = read32(bmpFS);
		read32(bmpFS);
		w = read32(bmpFS);
		h = read32(bmpFS);

		if( (read16(bmpFS) == 1) && (read16(bmpFS) == 24) && (read32(bmpFS) == 0) ){
				
			y += h - 1;

			bool oldSwapBytes = tft.getSwapBytes();
			tft.setSwapBytes(true);
			bmpFS.seek(seekOffset);

			uint16_t padding = (4 - ((w * 3) & 3)) & 3;
			uint8_t lineBuffer[w * 3 + padding];

			for (row = 0; row < h; row++) {
				
				bmpFS.read(lineBuffer, sizeof(lineBuffer));
				uint8_t*  bptr = lineBuffer;
				uint16_t* tptr = (uint16_t*)lineBuffer;
				// Convert 24 to 16 bit colours
				for (uint16_t col = 0; col < w; col++)
				{
				b = *bptr++;
				g = *bptr++;
				r = *bptr++;
				*tptr++ = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
				}

				// Push the pixel row to screen, pushImage will crop the line if needed
				// y is decremented as the BMP image is drawn bottom up
				tft.pushImage(x, y--, w, 1, (uint16_t*)lineBuffer);
			}
			tft.setSwapBytes(oldSwapBytes);
			Serial.print("Loaded in "); Serial.print(millis() - startTime);
			Serial.println(" ms");
		}
		else 
			Serial.println("BMP format not recognized.");
	}
	else
		Serial.println("Unable to read16");

	bmpFS.close();
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t DisplayManager::read16(fs::File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t DisplayManager::read32(fs::File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void DisplayManager::drawQrCode( const uint16_t xPos, const uint16_t yPos, const uint16_t color, const uint8_t w ){

	drawBmp("/qr.bmp");

	const uint8_t blocks = 29;	// 29 squares in each direction
	const uint8_t buffer = 3;

	QRCode qrcode;
	uint8_t qrcodeBytes[qrcode_getBufferSize(buffer)];
	qrcode_initText(&qrcode, qrcodeBytes, buffer, ECC_MEDIUM, (String("doge:")+RECEIVING_ADDRESS).c_str());

	for( uint8_t y = 0; y < qrcode.size; ++y ){

		const uint16_t startY = yPos+w*y;

		for( uint8_t x = 0; x < qrcode.size; ++x ){

			const uint16_t startX = xPos+w*x;

			if( qrcode_getModule(&qrcode, x, y) )
				tft.fillRect(startX, startY, w, w, color);

		}

	}

}


void DisplayManager::drawPasswordStars(){
	
	tft.fillRect(WIFI_BUTTON_LEFT, WIFI_BUTTON_TOP, WIFI_BUTTON_WIDTH, WIFI_BUTTON_HEIGHT-WIFI_BUTTON_PADDING_BOTTOM, BUTTON_COLOR);
	tft.setTextSize(1);
	tft.setTextColor(TFT_YELLOW);
	String output;
	for( uint8_t i = 0; i < cache_passwd.length(); ++i )
		output += "*";
	tft.drawString(output.c_str(), CENTER, WIFI_BUTTON_TOP+20);

}

void DisplayManager::drawKeyboard(){

	const char *keymap = keyboardUpperCase ? KEYBOARD_MAP_UPPERCASE : KEYBOARD_MAP_LOWERCASE;
	
	tft.setTextColor(TFT_YELLOW);
	tft.setTextSize(1);

	uint16_t rowSize[KEYBOARD_ROWS];
	uint16_t size = 0;
	uint8_t row = 0;
	for( uint8_t i = 0; i < KEYBOARD_LENGTH; ++i ){

		if( keymap[i] == '\n' ){
			
			rowSize[row] = size;
			//Serial.printf("Row: %i = %i\n", row, size);
			++row;
			size = 0;

		}
		else
			size = size+KEYBOARD_BTN_WIDTH+KEYBOARD_BTN_MARGIN;

	}
	
	uint8_t y = KEYBOARD_OFFS_Y;
	uint8_t x = 0;
	row = 0;
	for( uint8_t i = 0; i < KEYBOARD_LENGTH; ++i ){

		if( keymap[i] == '\n' ){
			y += KEYBOARD_BTN_HEIGHT+KEYBOARD_BTN_MARGIN;
			++row;
			x = 0;
			continue;
		}

		
		const uint16_t xpos = KEYBOARD_BTN_MARGIN/2+CENTER-rowSize[row]/2+x*(KEYBOARD_BTN_WIDTH+KEYBOARD_BTN_MARGIN);
		tft.fillRect(
			xpos,
			y,
			KEYBOARD_BTN_WIDTH,
			KEYBOARD_BTN_HEIGHT,
			BUTTON_COLOR
		);

		keypad_btns[i*2] = xpos;
		keypad_btns[i*2+1] = y;
		
		
		String letter = String(keymap[i]);
		//Serial.printf("Char %s x%i, y%i, center %i, rowsize %i, xSet %i\n", letter.c_str(), xpos, y, CENTER, rowSize[row], x);
		tft.drawString(
			letter.c_str(),
			xpos+KEYBOARD_BTN_WIDTH/2,
			y+8
		);
		

		++x;

	}

	// Done button
	tft.fillRect(
		KEYBOARD_DONE_BTN_POS_X, 
		KEYBOARD_DONE_BTN_POS_Y, 
		KEYBOARD_DONE_BTN_WIDTH, 
		KEYBOARD_DONE_BTN_HEIGHT, 
		BUTTON_COLOR
	);
	tft.drawString("DONE", KEYBOARD_DONE_BTN_POS_X+KEYBOARD_DONE_BTN_WIDTH/2, KEYBOARD_DONE_BTN_POS_Y+BUTTON_TEXT_PADDING_TOP);
	
	
}

