/*
	Edit Configuration.h
	Don't forget to edit your TFT_eSPI config in libraries

	Todo: 
		- Button sounds
		- Test audio
		- Publish schematics
		- Make tutorial
		


	Libraries:
	https://github.com/Bodmer/TFT_eSPI
	https://github.com/ricmoo/QRCode
	https://github.com/bblanchon/ArduinoJson
	https://github.com/gilmaimon/ArduinoWebsockets
	https://github.com/PowerBroker2/DFPlayerMini_Fast


	Audio credits 
	https://freesound.org/people/Robinhood76/sounds/63814/
	https://freesound.org/people/clairinski/sounds/184372/
	https://freesound.org/people/Anthousai/sounds/336578/
	https://freesound.org/people/EminYILDIRIM/sounds/536108/
*/
#include "Configuration.h"
#include "DisplayManager.h"
#include "WebsocketManager.h"
#include "SoundManager.h"
#include "ConnectionManager.h"
#include "LogManager.h"

void onWifiCredentials( String ssid = "", String pass = "" );



uint32_t last_rec = 0;			// Timestamp when we last received a tip
uint32_t last_screensav = 0; 	// 
bool screensav_active = false;

// Info screen ticks
int8_t boot_ticks = 0;
uint32_t boot_time = 0;	// Time when we first showed the settings screen
bool initialized = false;
bool addrWarning = false;	// Warned about address changed

bool connected(){
	return WebsocketManager::connected && ConnectionManager::connected;
}

void onTipReceived( uint32_t amount ){

	last_rec = millis();

	uint8_t sound = SoundManager::SOUND_MONEY_SMALL;
	if( amount >= SOUND_BIG_THR )
		sound = SoundManager::SOUND_MONEY_LARGE;
	else if( amount >= SOUND_MED_THR )
		sound = SoundManager::SOUND_MONEY_MED;
	
	SoundManager::play(sound);
	DisplayManager::setScreenDogeReceived(amount);

	LogManager::addPaymentLog(amount);

}

void onDisconnect(){

	// Try to reconnect
	onWifiCredentials("", "");

}

// When everything has been initialized, run this
void onWebsocketsSetup(){
	
	DisplayManager::setScreenQR(LogManager::RECEIVING_ADDRESS);

	if( !initialized ){
		initialized = true;
		SoundManager::play(SoundManager::SOUND_MONEY_MED);
	}

}


void onWebsocketsDisconnect(){

	if( !ConnectionManager::connected )
		return;
	DisplayManager::setScreenConnectingWebsocket();
	WebsocketManager::reconnect( LogManager::RECEIVING_ADDRESS, onWebsocketsSetup );

}

void drawBootScreen( bool force = false ){

	// Set intro screen
	if( !boot_time || force ){

		boot_time = millis();
		boot_ticks = LogManager::RECEIVING_ADDRESS.length() && !force ? 5 : -1;

		DisplayManager::setScreenSettings( 
			LogManager::total_tips, 
			LogManager::RECEIVING_ADDRESS, 
			LogManager::address_changed 
		);
		DisplayManager::setScreenSettingsTicks(boot_ticks);
		if( LogManager::RECEIVING_ADDRESS.length() )
			DisplayManager::setScreenSettingsEnableContinue();
		DisplayManager::setScreenSettingsVolume(LogManager::volume);

	}else{
		onWifiCredentials();	// Setup wifi
	}

}

// Attempt a wifi connection
bool wifiConnect( String ssid = "", String pass = "" ){

	DisplayManager::setScreenConnecting();
	if( !ConnectionManager::autoConnect(ssid, pass, DisplayManager::setScreenConnectingTicks) ){

		DisplayManager::setScreenConnectionFailed();
		delay(3000);
		return false;

	}

	// Connected

	// We changed the address this session, so we need to note a unix time
	if( !LogManager::address_changed )
		LogManager::setAddress( LogManager::RECEIVING_ADDRESS, ConnectionManager::getTime() );
	
	#ifndef HARD_WALLET
	if( !addrWarning ){
		DisplayManager::setScreenAddressChanged( ConnectionManager::getTime()-LogManager::address_changed );
		delay(4000);
	}
	#endif
	onWebsocketsDisconnect();	// Connect to WS
	
	return true;

}

// Callback for when wifi credentials have been entered into the device
void onWifiCredentials( String ssid, String pass ){

	// All connected, business as usual
	if( !wifiConnect(ssid, pass) )
		DisplayManager::setScreenNetworkScan( onWifiCredentials );	// Recurse until you fix it >:[
	

}

void onDisplayEvent( uint8_t evt, uint32_t i = 0, String s = "" ){

	if( evt == DisplayManager::EVT_TOUCH ){

		uint16_t x = i>>16;
		uint16_t y = i&0xFFFF;
		Serial.printf("onTouch %i, %i, %i\n", DisplayManager::MENU, x, y);
		// Retry connection, because it's more likely that we just lost the usual wifi
		if( DisplayManager::MENU == DisplayManager::MENU_RESCAN )
			onWifiCredentials();
		// Go to next screen (only works after the first cycle)
		else if( DisplayManager::MENU == DisplayManager::MENU_DEFAULT )
			last_screensav = 0;
		// Halt the start screen countdown
		else if( boot_ticks > -1 && DisplayManager::MENU == DisplayManager::MENU_SETTINGS ){

			boot_ticks = -1;
			DisplayManager::setScreenSettingsTicks(boot_ticks);

		}

	}
	else if( evt == DisplayManager::EVT_ADDR ){

		// This is done before the time is synced, so write 0
		LogManager::setAddress( s, 0 );
		drawBootScreen( true );

	}
	else if( evt == DisplayManager::EVT_CONTINUE )
		drawBootScreen();
	else if( evt == DisplayManager::EVT_KEYBOARD_CANCEL ){

		// Go back to scan
		if( DisplayManager::MENU == DisplayManager::MENU_PASSWD )
			DisplayManager::setScreenNetworkScan( onWifiCredentials );
		else
			drawBootScreen(true);

	}

	else if( evt == DisplayManager::EVT_VOLUME ){
		
		uint8_t vol = LogManager::volume;
		if( i )
			++vol;
		else if( vol )
			--vol;

		LogManager::setVolume(vol);
		SoundManager::setVolume(LogManager::volume);	// Doing it this way lets logmanager set limits
		DisplayManager::setScreenSettingsVolume(LogManager::volume);

	}
	else if( evt == DisplayManager::EVT_BUTTON_CLICK ){

		SoundManager::play(SoundManager::SOUND_CLICK);

	}

	else if( evt == DisplayManager::EVT_RESET_TIPS ){

		LogManager::resetTips();
		drawBootScreen(true);

	}
		

}


void setup(){
	
	Serial.begin(115200);
	delay(100);
	Serial.println("IT BEGINS");

	Serial.println("Setup display");
	DisplayManager::setup( onDisplayEvent );
	SoundManager::setup();

	ConnectionManager::setup(onDisconnect);

	// Setup SPIFFS
	if( !SPIFFS.begin() ){
		Serial.println("SPIFFS initialisation failed!");
		while (1) yield(); // Stay here twiddling thumbs waiting
	}
	Serial.println("\r\nSPIFFS available!");

	LogManager::setup();

	SoundManager::setVolume(LogManager::volume);

	// Setup callbacks and such
	WebsocketManager::setup( onTipReceived, onWebsocketsDisconnect );
	
	drawBootScreen();

}


void loop(){


	ConnectionManager::loop();
	WebsocketManager::loop();
	DisplayManager::loop();

	const uint32_t ms = millis();

	// Send test on serial to test doge tips received
	if( Serial.available() ){

		String input = Serial.readStringUntil('\n');
		if( input == "test" ){
			onTipReceived(1000);
		}

	}

	// In boot menu
	if( boot_ticks > -1 ){

		if( ms-boot_time > 1000 ){
			boot_time = ms;
			--boot_ticks;
			if( boot_ticks == -1 ){
				drawBootScreen();	// Since boot_time is nonzero, this initiates
			}
			else
				DisplayManager::setScreenSettingsTicks(boot_ticks);
		}

		return;
	}
	
	if( !connected() )
		return;

	// Handle tip display
	if( last_rec && ms - last_rec > TIP_TIME ){
		
		last_rec = 0;
		last_screensav = ms;
		screensav_active = false;
		DisplayManager::setScreenQR(LogManager::RECEIVING_ADDRESS);

	}
	else if( last_rec )
		return;

	// Handle screensaver
	if( !screensav_active && ms - last_screensav > QR_TIME ){

		last_screensav = ms;
		screensav_active = true;
		DisplayManager::setScreenSaver();
		return;

	}

	if( screensav_active && ms - last_screensav > SCREENSAVER_TIME ){

		last_screensav = ms;
		screensav_active = false;
		DisplayManager::setScreenQR(LogManager::RECEIVING_ADDRESS);

	}


}


