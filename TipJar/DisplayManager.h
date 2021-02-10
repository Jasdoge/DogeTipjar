#ifndef __DisplayManager
#define __DisplayManager
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <qrcode.h>
#include "Configuration.h"
#include "LogManager.h"


class DisplayManager{

	public:
		
		// These are used by touchscreen to figure out where you clicked
		static const uint8_t MENU_NONE = 0;	// No touch events
		static const uint8_t MENU_SSID = 1;	// Pick a network
		static const uint8_t MENU_PASSWD = 2;	// Pick a password
		static const uint8_t MENU_DEFAULT = 3;	// Running as normal
		static const uint8_t MENU_RESCAN = 4;	// Tap to re-scan network
		static const uint8_t MENU_SETTINGS = 5;	// Setting screen
		static const uint8_t MENU_ADDRESS = 6;	// Set address
		static const uint8_t MENU_ADDR_CHANGE = 7;	// Address recently changed screen
		static uint8_t MENU;


		static const uint8_t EVT_TOUCH = 0;		// i = x/y coords, x being the leftmost 16 bits
		static const uint8_t EVT_ADDR = 1;		// s = address. Raised when address changes.
		static const uint8_t EVT_CONTINUE = 2;	// raised on boot screen, skips the wait time
		static const uint8_t EVT_VOLUME = 3;	// i = 1 for up, 0 for down
		static const uint8_t EVT_BUTTON_CLICK = 4;	// Raised whenever a button is clicked
		static const uint8_t EVT_KEYBOARD_CANCEL = 5;	// Clicking the cancel button on the keypad
		static const uint8_t EVT_RESET_TIPS = 6;		// Reset tips

		// Library classes
		static TFT_eSPI tft;


		// Events
		static void onTipReceived( uint32_t amount );



		// Menus
		static void setScreenConnecting();
			static void setScreenConnectingTicks( uint8_t ticks );	// Can be called each second while connecting to add a dot
		static void setScreenConnectionFailed();

		static void setScreenQR( String address );

		static void setScreenDogeReceived( uint32_t amount );

		static void setScreenNetworkScan( void(*wifiCallback)(String ssid, String passwd) );
		static void setScreenNetworkPassword();
		static void setScreenConnectingWebsocket();
		static void setScreenSaver();

		static bool settingsContinueButtonEnabled;

		static void setScreenSettings( uint64_t total_tips, String address, uint64_t address_last_changed );
		static void setScreenSettingsTicks( int8_t ticks );
		static void setScreenSettingsEnableContinue();	// Enables the "Continue" button
		static void setScreenSettingsVolume( uint8_t volume );

		static void setScreenAddress();

		static void setScreenAddressChanged( uint32_t ms_ago );




		static void setup( void(*onEventHandler)(uint8_t type, uint32_t i, String t) );
		static void loop();

	private:
		static String cache_SSID;				// Stores SSID when configuring
		static String cache_text;				// Text editor cache
		static void(*cache_wifiCallback)(String ssid, String passwd);
		static void(*onEvent)(uint8_t type, uint32_t i, String t);
		
		

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

		static const uint16_t SETTING_LINES_START_X = 10;
		static const uint16_t SETTING_LINES_START_Y = 50;
		static const uint16_t SETTING_LINES_HEIGHT = 20;

		static const uint16_t VOLUME_BUTTON_X = 110;
		static const uint16_t VOLUME_BUTTON_Y = SETTING_LINES_START_Y+SETTING_LINES_HEIGHT*2;
		static const uint16_t VOLUME_BUTTON_WIDTH = 18;

		static const uint16_t RESET_BUTTON_WIDTH = 50;


		static uint16_t keypad_btns[KEYBOARD_LENGTH*2];	// X/Y 
		static bool keyboardUpperCase;



		// Tools:
		static void drawBmp(const char *filename, int16_t x = 0, int16_t y = 0);
		static uint16_t read16(fs::File &f);
		static uint32_t read32(fs::File &f);
		static void drawQrCode( String address, const uint16_t xPos = 101, const uint16_t yPos = 19, const uint16_t color = 0, const uint8_t w = 7 );

		static void drawTextEditor( bool stars = false );
		static void drawKeyboard();
		
		static bool checkClick( uint16_t x, uint16_t y, uint16_t xMin, uint16_t xMax, uint16_t yMin, uint16_t yMax );

};


#endif
