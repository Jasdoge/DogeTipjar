#ifndef __WebsocketManager
#define __WebsocketManager
#define ARDUINOJSON_USE_LONG_LONG 1
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>
using namespace websockets;
#include "Configuration.h"


class WebsocketManager{

	public:
		static WebsocketsClient client;


		static void onMessage( WebsocketsMessage message );
		static void onEvents(WebsocketsEvent event, String data);

		static void setup( void (*coinsReceivedCallback)( uint32_t amount ) );
		static bool reconnect();
		static void loop();

	private:
		static void (*onCoinsReceived)( uint32_t amount );
	

};


#endif
