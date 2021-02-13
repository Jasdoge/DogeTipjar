#include "DisplayManager.h"
#include <WiFi.h>

// Initialization
uint8_t DisplayManager::MENU = DisplayManager::MENU_NONE;
TFT_eSPI DisplayManager::tft = TFT_eSPI();
uint32_t DisplayManager::lastTouch = 0;
uint16_t DisplayManager::wifi_buttons[5] = {0};
String DisplayManager::cache_SSID;				// Stores SSID when configuring
String DisplayManager::cache_text;			// Stores password when configuring
void (*DisplayManager::cache_wifiCallback)(String ssid, String passwd);
void (*DisplayManager::onEvent)(uint8_t event, uint32_t i, String t);
uint16_t DisplayManager::keypad_btns[KEYBOARD_LENGTH*2] = {0};	// X/Y 
bool DisplayManager::keyboardUpperCase = false;
bool DisplayManager::settingsContinueButtonEnabled = false;

void DisplayManager::setup( void(*onEventHandler)(uint8_t event, uint32_t i, String t) ){

	onEvent = onEventHandler;
	// TFT loading
	tft.init();
	tft.setRotation(1);
	tft.loadFont("ComicSans");	
	tft.setTextDatum(MC_DATUM);
	tft.setTextColor(TFT_YELLOW);
	tft.fillScreen(0);
	tft.drawString("Loading", 160, 130);


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

		onEvent(EVT_TOUCH, (t_x<<16)|t_y, "");

		Serial.printf("%ix, %iy\n", t_x, t_y);

		// WiFi SSID Listing
		if( MENU == MENU_SSID ){

			// Scan the buttons
			for( uint8_t i = 0; i < 5; ++i ){

				uint16_t y = wifi_buttons[i];
				if( !y )
					break;

				if(checkClick(
					t_x, t_y,
					WIFI_BUTTON_LEFT,
					WIFI_BUTTON_WIDTH+WIFI_BUTTON_LEFT,
					y, 
					y+WIFI_BUTTON_HEIGHT-WIFI_BUTTON_PADDING_BOTTOM
				)){

					cache_SSID = WiFi.SSID(i);
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
		else if( MENU == MENU_PASSWD || MENU == MENU_ADDRESS ){

			if(checkClick(
				t_x, t_y,
				KEYBOARD_DONE_BTN_POS_X,
				KEYBOARD_DONE_BTN_POS_X+KEYBOARD_DONE_BTN_WIDTH,
				KEYBOARD_DONE_BTN_POS_Y,
				KEYBOARD_DONE_BTN_POS_Y+KEYBOARD_DONE_BTN_HEIGHT
			)){

				if( MENU == MENU_PASSWD ){
					cache_wifiCallback(cache_SSID, cache_text);
					
				}
				else{
					onEvent(EVT_ADDR, 0, cache_text);
				}
				cache_SSID = cache_text = "";

				return;
			}

			if(checkClick(
				t_x, t_y,
				10,
				10+KEYBOARD_DONE_BTN_WIDTH,
				KEYBOARD_DONE_BTN_POS_Y,
				KEYBOARD_DONE_BTN_POS_Y+KEYBOARD_DONE_BTN_HEIGHT
			)){
				cache_SSID = cache_text = "";
				onEvent(EVT_KEYBOARD_CANCEL, 0, "");
				return;
			}

			for( uint8_t i = 0; i < KEYBOARD_LENGTH; ++i ){

				const uint16_t x = keypad_btns[i*2];
				const uint16_t y = keypad_btns[i*2+1];

				if(checkClick(
					t_x, t_y,
					x, x+KEYBOARD_BTN_WIDTH,
					y, y+KEYBOARD_BTN_HEIGHT
				)){

					const char *keymap = keyboardUpperCase ? KEYBOARD_MAP_UPPERCASE : KEYBOARD_MAP_LOWERCASE;
					char pushed = keymap[i];
					if( pushed == '^' ){
						keyboardUpperCase = !keyboardUpperCase;
					}
					else if( pushed == '<' && cache_text.length() ){
						cache_text = cache_text.substring(0, cache_text.length()-1);
						drawTextEditor( MENU == MENU_PASSWD );
					}
					else if( MENU != MENU_ADDRESS || cache_text.length() < 34 ){

						cache_text += String(pushed);
						drawTextEditor( MENU == MENU_PASSWD );

					}
					drawKeyboard();

					break;

				}

			}


		}

		else if( MENU == MENU_SETTINGS ){

			#ifndef HARD_WALLET
			// Address button clicked
			if(checkClick(
				t_x, t_y,
				CENTER-60,
				CENTER+60,
				SETTING_LINES_START_Y+SETTING_LINES_HEIGHT*4,
				SETTING_LINES_START_Y+SETTING_LINES_HEIGHT*4+WIFI_BUTTON_HEIGHT
			)){
				setScreenAddress();
			}
			#endif

			// Continue button
			if(
				settingsContinueButtonEnabled &&
				checkClick(
					t_x, t_y,
					CENTER-60,
					CENTER+60,
					SETTING_LINES_START_Y+SETTING_LINES_HEIGHT*4+WIFI_BUTTON_HEIGHT+5,
					SETTING_LINES_START_Y+SETTING_LINES_HEIGHT*4+WIFI_BUTTON_HEIGHT+5+WIFI_BUTTON_HEIGHT
				)
			){
				onEvent(EVT_CONTINUE, 0, "");
			}

			// Volume minus button
			if(checkClick(
				t_x, t_y,
				VOLUME_BUTTON_X,
				VOLUME_BUTTON_X+VOLUME_BUTTON_WIDTH,
				VOLUME_BUTTON_Y-8,
				VOLUME_BUTTON_Y-8+VOLUME_BUTTON_WIDTH
			)){
				onEvent(EVT_VOLUME, 0, "");
			}

			// Volume plus button
			if(checkClick(
				t_x, t_y,
				VOLUME_BUTTON_X+VOLUME_BUTTON_WIDTH+2,
				VOLUME_BUTTON_X+VOLUME_BUTTON_WIDTH+2+VOLUME_BUTTON_WIDTH,
				VOLUME_BUTTON_Y-8,
				VOLUME_BUTTON_Y-8+VOLUME_BUTTON_WIDTH
			)){
				onEvent(EVT_VOLUME, 1, "");
			}

			// Reset tips
			if(checkClick(
				t_x, t_y,
				SETTING_LINES_START_X,
				SETTING_LINES_START_X+RESET_BUTTON_WIDTH,
				SETTING_LINES_START_Y+SETTING_LINES_HEIGHT-8,
				SETTING_LINES_START_Y+SETTING_LINES_HEIGHT-8+VOLUME_BUTTON_WIDTH
			)){
				onEvent(EVT_RESET_TIPS, 0, "");
			}

		}

		

	}
	else if( ms-lastTouch > 100 )
		lastTouch = 0;


}


// Check if a click is within boundsand draws a rect and raises an event in that case
bool DisplayManager::checkClick( uint16_t x, uint16_t y, uint16_t xMin, uint16_t xMax, uint16_t yMin, uint16_t yMax ){

	if( x >= xMin && x <= xMax && y >= yMin && y <= yMax ){

		onEvent(EVT_BUTTON_CLICK, 0, "");
		tft.fillRect(xMin, yMin, xMax-xMin, yMax-yMin, 0xFFFF);
		delay(50);
		return true;

	}
	return false;

}




// SCREENS

// Connecting screen
void DisplayManager::setScreenConnecting(){
	
	MENU = MENU_NONE;
	tft.setTextColor(TFT_YELLOW);
	tft.setTextSize(2);
	tft.fillScreen(TFT_BLACK);
	tft.drawString("WiFi Connecting", CENTER, 20);

}
void DisplayManager::setScreenConnectingTicks( uint8_t ticks ){
	
	tft.fillRect(0, 100, 320, 40, TFT_BLACK);
	uint16_t start = CENTER-ticks*6/2;
	for( uint8_t i = 0; i < ticks; ++i ){

		tft.fillRect(start+1+i*6, 120, 4, 4, 0xFFFF);

	}

}

void DisplayManager::setScreenConnectingWebsocket(){
	MENU = MENU_NONE;

	tft.setTextColor(TFT_YELLOW);
	tft.setTextSize(2);
	tft.fillScreen(TFT_BLACK);
	tft.drawString("Connecting to dogechain", CENTER, 20);

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

void DisplayManager::setScreenSaver(){
	MENU = MENU_DEFAULT;
	drawBmp("/wallpaper.bmp");

}

void DisplayManager::setScreenQR( String address ){
	MENU = MENU_DEFAULT;

	drawQrCode(address);

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

	int8_t n = WiFi.scanNetworks();
	tft.fillScreen(TFT_BLACK);

	if( n < 1 ){
		MENU = MENU_RESCAN;
		tft.drawString("No networks found", x, 100);
		tft.setTextSize(2);
		tft.drawString("Tap to retry", x, 150);
		return;
	}
	
	Serial.printf("Found %i networks\n", n);

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
	drawTextEditor( MENU == MENU_PASSWD );
	drawKeyboard();

}


void DisplayManager::setScreenSettingsTicks( int8_t ticks ){

	// Timeout
	tft.setTextSize(2);
	tft.fillRect(260, 0, 60, 40, 0x0);
	if( ticks > -1 )
		tft.drawString(String(ticks).c_str(), 280, 20);

}
void DisplayManager::setScreenSettings( uint64_t total_tips, String address, uint64_t address_last_changed ){
	MENU = MENU_SETTINGS;

	settingsContinueButtonEnabled = false;

	tft.fillScreen(TFT_BLACK);
	
	tft.setTextFont(2);
	tft.setTextSize(2);
	tft.setTextColor(TFT_YELLOW);
	tft.drawString("Settings", CENTER, 20);

	tft.setTextSize(1);

	tft.setTextDatum(ML_DATUM);
	String out;

	out = "Addr: "+address;
	tft.drawString(out.c_str(), SETTING_LINES_START_X, SETTING_LINES_START_Y);

	out = "All time tips: "+String(LogManager::uint64_to_string(total_tips))+" DOGE";
	tft.drawString(out.c_str(), SETTING_LINES_START_X+60, SETTING_LINES_START_Y+SETTING_LINES_HEIGHT);

	tft.setTextDatum(MC_DATUM);

	// Reset button
	tft.fillRect(
		SETTING_LINES_START_X, 
		SETTING_LINES_START_Y+SETTING_LINES_HEIGHT-8, 
		RESET_BUTTON_WIDTH, 
		VOLUME_BUTTON_WIDTH, 
		BUTTON_COLOR
	);
	tft.drawString("RESET", SETTING_LINES_START_X+RESET_BUTTON_WIDTH/2, SETTING_LINES_START_Y+SETTING_LINES_HEIGHT-VOLUME_BUTTON_WIDTH/2+10);

	out = "";
	

	// Buttons
	#ifndef HARD_WALLET
	tft.fillRect(CENTER-60, SETTING_LINES_START_Y+SETTING_LINES_HEIGHT*4, 120, WIFI_BUTTON_HEIGHT, BUTTON_COLOR);
	tft.drawString("Change Address", CENTER, SETTING_LINES_START_Y+SETTING_LINES_HEIGHT*4+20);
	#endif
}
void DisplayManager::setScreenSettingsVolume( uint8_t volume ){

	tft.setTextSize(1);
	tft.setTextDatum(ML_DATUM);

	tft.fillRect(0, VOLUME_BUTTON_Y-6, VOLUME_BUTTON_X, VOLUME_BUTTON_WIDTH, 0);

	String out = "Volume: "+String(volume)+"/30";
	tft.drawString(out.c_str(), SETTING_LINES_START_X, VOLUME_BUTTON_Y);
	tft.setTextDatum(MC_DATUM);


	tft.fillRect(
		VOLUME_BUTTON_X, 
		VOLUME_BUTTON_Y-8, 
		VOLUME_BUTTON_WIDTH, 
		VOLUME_BUTTON_WIDTH, 
		BUTTON_COLOR
	);
	tft.drawString("-", VOLUME_BUTTON_X+VOLUME_BUTTON_WIDTH/2, VOLUME_BUTTON_Y-VOLUME_BUTTON_WIDTH/2+8);

	tft.fillRect(
		VOLUME_BUTTON_X+VOLUME_BUTTON_WIDTH+2, 
		VOLUME_BUTTON_Y-8, 
		VOLUME_BUTTON_WIDTH, 
		VOLUME_BUTTON_WIDTH, 
		BUTTON_COLOR
	);
	tft.drawString("+", VOLUME_BUTTON_X+VOLUME_BUTTON_WIDTH/2+VOLUME_BUTTON_WIDTH+2, VOLUME_BUTTON_Y-VOLUME_BUTTON_WIDTH/2+8);



}
void DisplayManager::setScreenSettingsEnableContinue(){

	settingsContinueButtonEnabled = true;
	tft.setTextSize(1);
	tft.fillRect(CENTER-60, SETTING_LINES_START_Y+SETTING_LINES_HEIGHT*4+WIFI_BUTTON_HEIGHT+5, 120, WIFI_BUTTON_HEIGHT, BUTTON_COLOR);
	tft.drawString("Continue", CENTER, SETTING_LINES_START_Y+SETTING_LINES_HEIGHT*4+20+WIFI_BUTTON_HEIGHT+5);

}


// Shows when the address was last changed
void DisplayManager::setScreenAddressChanged( uint32_t delta ){

	MENU = MENU_ADDR_CHANGE;
	tft.fillScreen(TFT_BLACK);
	tft.setTextFont(2);
	tft.setTextSize(1);

	tft.drawString("Last Address Change", CENTER, 50);
	
	String out;
	tft.setTextSize(2);

	if( delta < 3600 )
		out += String(delta/60)+" minutes ago";
	else if( delta < 3600*24 )
		out += String(delta/3600)+" hours ago";
	else
		out += String(delta/(3600*24))+" days ago";

	if( delta < 3600*24 )
		tft.setTextColor(TFT_RED);

	tft.drawString(out.c_str(), CENTER, 120);

	tft.setTextColor(TFT_YELLOW);

}

void DisplayManager::setScreenAddress(){
	MENU = MENU_ADDRESS;

	tft.fillScreen(TFT_BLACK);

	tft.setTextFont(2);
	tft.setTextSize(2);
	tft.setTextColor(TFT_YELLOW);


	tft.drawString("Enter wallet address", CENTER, 20);
	drawTextEditor( false );
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

void DisplayManager::drawQrCode( String address, const uint16_t xPos, const uint16_t yPos, const uint16_t color, const uint8_t w ){

	drawBmp("/qr.bmp");

	const uint8_t blocks = 29;	// 29 squares in each direction
	const uint8_t buffer = 3;

	QRCode qrcode;
	uint8_t qrcodeBytes[qrcode_getBufferSize(buffer)];
	qrcode_initText(&qrcode, qrcodeBytes, buffer, ECC_MEDIUM, (String("doge:")+address).c_str());

	for( uint8_t y = 0; y < qrcode.size; ++y ){

		const uint16_t startY = yPos+w*y;

		for( uint8_t x = 0; x < qrcode.size; ++x ){

			const uint16_t startX = xPos+w*x;

			if( qrcode_getModule(&qrcode, x, y) )
				tft.fillRect(startX, startY, w, w, color);

		}

	}

}


void DisplayManager::drawTextEditor( bool stars ){
	
	tft.fillRect(WIFI_BUTTON_LEFT, WIFI_BUTTON_TOP, WIFI_BUTTON_WIDTH, WIFI_BUTTON_HEIGHT-WIFI_BUTTON_PADDING_BOTTOM, BUTTON_COLOR);
	tft.setTextSize(1);
	tft.setTextColor(TFT_YELLOW);
	String output;
	if( stars ){
		for( uint8_t i = 0; i < cache_text.length(); ++i )
			output += "*";
	}
	else
		output = cache_text;
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
	
	// Cancel button
	tft.fillRect(
		10, 
		KEYBOARD_DONE_BTN_POS_Y, 
		KEYBOARD_DONE_BTN_WIDTH, 
		KEYBOARD_DONE_BTN_HEIGHT, 
		BUTTON_COLOR
	);
	tft.drawString("CANCEL", 10+KEYBOARD_DONE_BTN_WIDTH/2, KEYBOARD_DONE_BTN_POS_Y+BUTTON_TEXT_PADDING_TOP);
	

	
}

