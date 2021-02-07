/*
	Edit Configuration.h
	Don't forget to edit your TFT_eSPI config in libraries

	Todo: 
		- Automatic reconnect
			- Wifi
			- Websocket
		- Show when you have been disconnected
		- Error messages
		- Stats display
			- Total tips today?
			- Total tips all time?
		- Pushing on the display in normal mode takes it back to QR code
		- Button sounds
		- Test audio
		- Settings page?
			- Ability to change address?
			- Ability to change volume?
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




uint32_t last_rec = 0;			// Timestamp when we last received a tip
uint32_t last_screensav = 0; 	// 
bool screensav_active = false;
bool connected = true;


void onTipReceived( uint32_t amount ){

	last_rec = millis();

	uint8_t sound = SoundManager::SOUND_MONEY_SMALL;
	if( amount >= SOUND_BIG_THR )
		sound = SoundManager::SOUND_MONEY_LARGE;
	else if( amount >= SOUND_MED_THR )
		sound = SoundManager::SOUND_MONEY_MED;
	
	SoundManager::play(sound);
	DisplayManager::setScreenDogeReceived(amount);

}



// Attempt a wifi connection
bool wifiConnect( String ssid = "", String pass = "" ){

	connected = false;

	DisplayManager::setScreenConnecting();
	if( !ConnectionManager::autoConnect(ssid, pass, DisplayManager::setScreenConnectingTicks) ){

		DisplayManager::setScreenConnectionFailed();
		delay(3000);
		Serial.println("Todo: re-scan networks"); 
		//scanNetworks();
		return false;

	}

	WebsocketManager::reconnect();
	connected = true;

	return true;

}

// Callback for when wifi credentials have been entered into the device
void onWifiCredentials( String ssid = "", String pass = "" ){

	// All connected, business as usual
	if( wifiConnect(ssid, pass) )
		DisplayManager::setScreenQR();
	else
		DisplayManager::setScreenNetworkScan( onWifiCredentials );	// Recurse until you fix it >:[
	

}


void setup(){
	
	Serial.begin(115200);
	delay(100);
	Serial.println("IT BEGINS");

	Serial.println("Setup display");
	DisplayManager::setup();
	SoundManager::setup();

	// Setup SPIFFS
	if( !SPIFFS.begin() ){
		Serial.println("SPIFFS initialisation failed!");
		while (1) yield(); // Stay here twiddling thumbs waiting
	}
	Serial.println("\r\nSPIFFS available!");

	// Setup callbacks and such
	WebsocketManager::setup( onTipReceived );
	
	onWifiCredentials();

}


void loop(){

	WebsocketManager::loop();
	DisplayManager::loop();


	// Send test on serial to test doge tips received
	if( Serial.available() ){

		String input = Serial.readStringUntil('\n');
		if( input == "test" ){
			onTipReceived(1000);
		}

	}
	
	if( !connected )
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


