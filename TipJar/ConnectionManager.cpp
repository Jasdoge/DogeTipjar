#include "ConnectionManager.h"

tm ConnectionManager::timeinfo;

void (*ConnectionManager::onDisconnect)();
bool ConnectionManager::connected = false;

void ConnectionManager::setup( void(*onDisconnectCallback)() ){
	
	onDisconnect = onDisconnectCallback;

}

void ConnectionManager::loop(){

	if( !connected )
		return;

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

	
	configTime(0, 0, "pool.ntp.org");
	if( !getLocalTime(&timeinfo) ){
		Serial.println("NTP not available");
		return false;
	}

	connected = true;
	Serial.println("Connected to the WiFi network");
	Serial.println(WiFi.localIP());
	return true;

}

time_t ConnectionManager::getTime(){

	time_t now;
	time(&now);
	return now;

}

void ConnectionManager::factoryReset(){

	WiFi.disconnect(true, true);
	/*
	delay(500);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //load the flash-saved configs
	esp_wifi_init(&cfg); //initiate and allocate wifi resources (does not matter if connection fails)
	delay(2000); //wait a bit
	if(esp_wifi_restore()!=ESP_OK){
		Serial.println("WiFi is not initialized by esp_wifi_init ");
	}else{
		Serial.println("WiFi Configurations Cleared!");
	}
	*/

}



ConnectionManager Connection;

