#include "WebsocketManager.h"

WebsocketsClient WebsocketManager::client = WebsocketsClient();
void (*WebsocketManager::onCoinsReceived)( uint32_t amount );

void WebsocketManager::setup( void (*coinsReceivedCallback)( uint32_t amount ) ){

	onCoinsReceived = coinsReceivedCallback;

	client.onMessage(onMessage);
	client.onEvent(onEvents);
	client.setInsecure();

}

bool WebsocketManager::reconnect(){

	bool connected = client.connect("wss://ws.dogechain.info/inv");
	if( !connected ){
		Serial.println("Failed to connect");
		return false;
	}

	client.send("{\"op\":\"addr_sub\",\"addr\":\""+RECEIVING_ADDRESS+"\"}");
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

	if( !data["op"].isNull() && !data["x"].isNull() && data["op"] == "utx" && !data["x"]["outputs"].isNull() ){

		const JsonObject x = data["x"];
		const String txid = x["hash"];
		JsonArray outputs = x["outputs"];
		uint64_t received = 0;

		for( JsonObject v : outputs ){

			String add = v["addr"];
			if( add == RECEIVING_ADDRESS )
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
	} else if(event == WebsocketsEvent::ConnectionClosed) {
		Serial.println("Connnection Closed");
	} else if(event == WebsocketsEvent::GotPing) {
		Serial.println("Got a Ping!");
	} else if(event == WebsocketsEvent::GotPong) {
		Serial.println("Got a Pong!");
	}

}





