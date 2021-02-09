#include "ConnectionManager.h"

void (*ConnectionManager::onDisconnect)();
bool ConnectionManager::connected = false;

//uint32_t tick = 0;

void ConnectionManager::setup( void(*onDisconnectCallback)() ){
	
	onDisconnect = onDisconnectCallback;

}

void ConnectionManager::loop(){

	if( !connected )
		return;

	/*
	if( millis()-tick > 1000 ){

		Serial.printf("Status %i\n", WiFi.status() == WL_CONNECTED);
		tick = millis();
	}
	*/

	if( WiFi.status() != WL_CONNECTED ){

		connected = false;
		onDisconnect();

	}

}

// Returns true/false
bool ConnectionManager::autoConnect( String ssid, String password, void(*connectionTick)(uint8_t tick) ){

	// Handle hardcoded
	#if defined(WIFI_SSID) && defined(WIFI_PASS)
	if( !ssid.length() ){
		ssid = WIFI_SSID;
		password = WIFI_PASS;
	}
	#endif

	Serial.printf("Connecting to %s with %s\n", ssid.c_str(), password.c_str());

	if( ssid.length() )
		WiFi.begin(ssid.c_str(), password.c_str());
	else
		WiFi.begin();

	uint8_t timeout = 0;
	while( WiFi.status() != WL_CONNECTED && ++timeout < 15 ){

		if( connectionTick )
			connectionTick(timeout);
		delay(1000);
		
	}
	
	if( WiFi.status() != WL_CONNECTED )
		return false;

	connected = true;
	Serial.println("Connected to the WiFi network");
	Serial.println(WiFi.localIP());
	return true;

}



ConnectionManager Connection;

