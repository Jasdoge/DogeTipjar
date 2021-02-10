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
		static bool connected;

		static void onMessage( WebsocketsMessage message );
		static void onEvents(WebsocketsEvent event, String data);

		static void setup( void (*coinsReceivedCallback)( uint32_t amount ), void (*onDisconnectCallback)() );
		static bool reconnect( String address, void (*onConnectCallback)());
		static void loop();

	private:
		static String cache_address;

		static void (*onCoinsReceived)( uint32_t amount );
		static void (*onConnect)();
		static void (*onDisconnect)(); 
		

};


#endif
