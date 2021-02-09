/*
	Edit Configuration.h
	Don't forget to edit your TFT_eSPI config in libraries

	Todo: 
		- Stats display
			- Total tips all time
			- Last tip received
		- Button sounds
		- Test audio
		- Settings page on boot
			- Ability to change address
			- Show when address was last changed
			- Show current address and last change on boot
			- Ability to change volume
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
*/
#include "Configuration.h"
#include "DisplayManager.h"
#include "WebsocketManager.h"
#include "SoundManager.h"
#include "ConnectionManager.h"
#include "LogManager.h"

void onWifiCredentials( String ssid, String pass );



uint32_t last_rec = 0;			// Timestamp when we last received a tip
uint32_t last_screensav = 0; 	// 
bool screensav_active = false;

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

void onWebsocketsSetup(){
	
	DisplayManager::setScreenQR();

}


void onWebsocketsDisconnect(){

	if( !ConnectionManager::connected )
		return;
	DisplayManager::setScreenConnectingWebsocket();
	WebsocketManager::reconnect( onWebsocketsSetup );

}



// Attempt a wifi connection
bool wifiConnect( String ssid = "", String pass = "" ){

	DisplayManager::setScreenConnecting();
	if( !ConnectionManager::autoConnect(ssid, pass, DisplayManager::setScreenConnectingTicks) ){

		DisplayManager::setScreenConnectionFailed();
		delay(3000);
		return false;

	}

	onWebsocketsDisconnect();	// reconnects to websockets
	
	
	return true;

}

// Callback for when wifi credentials have been entered into the device
void onWifiCredentials( String ssid = "", String pass = "" ){

	// All connected, business as usual
	if( !wifiConnect(ssid, pass) )
		DisplayManager::setScreenNetworkScan( onWifiCredentials );	// Recurse until you fix it >:[
	

}

void onTouch( uint16_t x, uint16_t y ){

	Serial.printf("onTouch %i, %i, %i\n", DisplayManager::MENU, x, y);
	// Retry connection, because it's more likely that we just lost the usual wifi
	if( DisplayManager::MENU == DisplayManager::MENU_RESCAN )
		onWifiCredentials();
	else if( DisplayManager::MENU == DisplayManager::MENU_DEFAULT )
		last_screensav = 0;
	

}


void setup(){
	
	Serial.begin(115200);
	delay(100);
	Serial.println("IT BEGINS");

	Serial.println("Setup display");
	DisplayManager::setup( onTouch );
	SoundManager::setup();

	ConnectionManager::setup(onDisconnect);

	// Setup SPIFFS
	if( !SPIFFS.begin() ){
		Serial.println("SPIFFS initialisation failed!");
		while (1) yield(); // Stay here twiddling thumbs waiting
	}
	Serial.println("\r\nSPIFFS available!");

	LogManager::setup();

	// Setup callbacks and such
	WebsocketManager::setup( onTipReceived, onWebsocketsDisconnect );
	
	onWifiCredentials();

}


void loop(){

	ConnectionManager::loop();
	WebsocketManager::loop();
	DisplayManager::loop();


	// Send test on serial to test doge tips received
	if( Serial.available() ){

		String input = Serial.readStringUntil('\n');
		if( input == "test" ){
			onTipReceived(1000);
		}

	}
	
	if( !connected() )
		return;

	// Handle tip display
	const uint32_t ms = millis();
	if( last_rec && ms - last_rec > TIP_TIME ){
		
		last_rec = 0;
		last_screensav = ms;
		screensav_active = false;
		DisplayManager::setScreenQR();

	}
	else if( last_rec )
		return;

	// Handle screensaver
	if( !screensav_active && ms - last_screensav > QR_TIME ){

		last_screensav = ms;
		screensav_active = true;
		DisplayManager::setScreenStats();
		return;

	}

	if( screensav_active && ms - last_screensav > SCREENSAVER_TIME ){

		last_screensav = ms;
		screensav_active = false;
		DisplayManager::setScreenQR();

	}


}


