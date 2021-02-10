#include "WebsocketManager.h"

WebsocketsClient WebsocketManager::client = WebsocketsClient();
void (*WebsocketManager::onCoinsReceived)( uint32_t amount );
void (*WebsocketManager::onConnect)();
bool WebsocketManager::connected = false;
void (*WebsocketManager::onDisconnect)(); 
String WebsocketManager::cache_address;

void WebsocketManager::setup( void (*coinsReceivedCallback)( uint32_t amount ), void (*onDisconnectCallback)() ){

	onCoinsReceived = coinsReceivedCallback;
	onDisconnect = onDisconnectCallback;

	client.onMessage(onMessage);
	client.onEvent(onEvents);
	client.setInsecure();

}

bool WebsocketManager::reconnect( String address, void (*onConnectCallback)() ){

	connected = false;
	onConnect = onConnectCallback;
	cache_address = address;

	Serial.println("Connection to dogechain");
	bool connected = client.connect("wss://ws.dogechain.info/inv");
	Serial.printf("Connection status %i\n", connected);
	if( !connected ){
		Serial.println("Failed to connect");
		return false;
	}

	client.send("{\"op\":\"addr_sub\",\"addr\":\""+cache_address+"\"}");
	return true;

}


void WebsocketManager::loop(){
	client.poll();
}

void WebsocketManager::onMessage( WebsocketsMessage message ){
	
	DynamicJsonDocument doc(4086);
	deserializeJson(doc, message.data());
	JsonObject data = doc.as<JsonObject>();
	
	Serial.print("Got message: ");
	Serial.println(message.data());

	if( data["op"].isNull() )
		return;

	if( !data["msg"].isNull() && data["msg"] == "subscribed" ){
		connected = true;
		onConnect();
		return;
	}

	if( !data["x"].isNull() && data["op"] == "utx" && !data["x"]["outputs"].isNull() ){

		const JsonObject x = data["x"];
		const String txid = x["hash"];
		JsonArray outputs = x["outputs"];
		uint64_t received = 0;

		for( JsonObject v : outputs ){

			String add = v["addr"];
			if( add == cache_address )
				received += v["value"].as<uint64_t>();

		}

		if( received ){

			double total = (double)received/100000000;

			onCoinsReceived(round(total));
			Serial.println("New transaction discovered: "+txid+" >> "+String(total)+" DOGE");

		}

	}
	

}

void WebsocketManager::onEvents(WebsocketsEvent event, String data) {

	if(event == WebsocketsEvent::ConnectionOpened) {
		Serial.println("Connnection Opened");
	}
	else if(event == WebsocketsEvent::ConnectionClosed) {
		Serial.println("Connnection Closed");
		connected = false;
		onDisconnect();
	}
	else if(event == WebsocketsEvent::GotPing) {
		Serial.println("Got a Ping!");
	}
	else if(event == WebsocketsEvent::GotPong) {
		Serial.println("Got a Pong!");
	}

}





