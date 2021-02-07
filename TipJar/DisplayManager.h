#ifndef __DisplayManager
#define __DisplayManager
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <qrcode.h>
#include "Configuration.h"


class DisplayManager{

	public:
		
		// These are used by touchscreen to figure out where you clicked
		static const uint8_t MENU_NONE = 0;	// No touch events
		static const uint8_t MENU_SSID = 1;	// Pick a network
		static const uint8_t MENU_PASSWD = 2;	// Pick a password
		static const uint8_t MENU_DEFAULT = 3;	// Running as normal
		static uint8_t MENU;

		// Library classes
		static TFT_eSPI tft;


		// Events
		static void onTipReceived( uint32_t amount );



		// Menus
		static void setScreenConnecting();
			static void setScreenConnectingTicks( uint8_t ticks );	// Can be called each second while connecting to add a dot
		static void setScreenConnectionFailed();

		static void setScreenQR();

		static void setScreenDogeReceived( uint32_t amount );

		static void setScreenNetworkScan( void(*wifiCallback)(String ssid, String passwd) );
		static void setScreenNetworkPassword();
			
		static void setScreenStats();
		





		static void setup();
		static void loop();

	private:
		static String cache_SSID;				// Stores SSID when configuring
		static String cache_passwd;			// Stores password when configuring
		static void(*cache_wifiCallback)(String ssid, String passwd);


		static uint32_t lastTouch;
		static uint16_t wifi_buttons[5];
		static const uint16_t WIFI_BUTTON_WIDTH = 300;
		static const uint8_t WIFI_BUTTON_HEIGHT = 40;
		static const uint8_t WIFI_BUTTON_PADDING_BOTTOM = 4;
		static const uint8_t WIFI_BUTTON_LEFT = 10;
		static const uint8_t WIFI_BUTTON_TOP = 40;
		static const uint8_t BUTTON_TEXT_PADDING_TOP = 16;
		static const uint16_t BUTTON_COLOR = 0x5AE0;
		static const uint8_t CENTER = 160;

		static const uint8_t KEYBOARD_OFFS_Y = 80;
		static const uint8_t KEYBOARD_BTN_WIDTH = 28;
		static const uint8_t KEYBOARD_BTN_HEIGHT = 20;
		static const uint8_t KEYBOARD_BTN_MARGIN = 4;

		static const uint8_t KEYBOARD_LENGTH = 44;
		static const uint8_t KEYBOARD_ROWS = 5;
		static constexpr char *KEYBOARD_MAP_UPPERCASE = "1234567890\nQWERTYUIOP\nASDFGHJKL\nZXCVBNM \n^<\n";
		static constexpr char *KEYBOARD_MAP_LOWERCASE = "1234567890\nqwertyuiop\nasdfghjkl\nzxcvbnm \n^<\n";

		static const uint16_t KEYBOARD_DONE_BTN_POS_X = 260;
		static const uint16_t KEYBOARD_DONE_BTN_POS_Y = 190;
		static const uint16_t KEYBOARD_DONE_BTN_WIDTH = 60;
		static const uint16_t KEYBOARD_DONE_BTN_HEIGHT = WIFI_BUTTON_HEIGHT;

		static uint16_t keypad_btns[KEYBOARD_LENGTH*2];	// X/Y 
		static bool keyboardUpperCase;



		// Tools:
		static void drawBmp(const char *filename, int16_t x = 0, int16_t y = 0);
		static uint16_t read16(fs::File &f);
		static uint32_t read32(fs::File &f);
		static void drawQrCode( const uint16_t xPos = 101, const uint16_t yPos = 19, const uint16_t color = 0, const uint8_t w = 7 );

		static void drawPasswordStars();
		static void drawKeyboard();
		

};


#endif
