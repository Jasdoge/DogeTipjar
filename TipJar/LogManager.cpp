#include "LogManager.h"

uint64_t LogManager::total_tips = 0;
uint64_t LogManager::last_tip = 0;
uint8_t LogManager::volume = 20;
uint64_t LogManager::address_changed = 0;
String LogManager::RECEIVING_ADDRESS = "";

void LogManager::setup(){

	File total = SPIFFS.open("/total.txt", FILE_READ);
	if( total ){
		
		String content = total.readString();
		total_tips = atoll(content.c_str());
		Serial.printf("Total: %s\n", uint64_to_string(total_tips));

		total.close();

	}
	
	total = SPIFFS.open("/last.txt");
	if( total ){
		
		String content = total.readString();
		last_tip = atoll(content.c_str());
		Serial.printf("Last: %s\n", uint64_to_string(last_tip));
		total.close();

	}

	total = SPIFFS.open("/volume.txt");
	if( total ){
		
		String content = total.readString();
		volume = content.toInt();
		Serial.printf("Volume: %i\n", volume);
		total.close();

	}

	total = SPIFFS.open("/address.txt");
	if( total ){
		
		RECEIVING_ADDRESS = total.readString();
		Serial.printf("Address: %s\n", RECEIVING_ADDRESS.c_str());
		total.close();

	}

	total = SPIFFS.open("/address_changed.txt");
	if( total ){
		
		String content = total.readString();
		Serial.printf("Content: %s\n", content.c_str());
		address_changed = atoll(content.c_str());
		Serial.printf("Addr changed: %s\n", uint64_to_string(address_changed));
		total.close();

	}

	

};


void LogManager::setVolume( uint8_t vol ){

	if( vol > 30 )
		vol = 30;

	volume = vol;
	File f = SPIFFS.open("/volume.txt", FILE_WRITE);
	f.print(String(volume).c_str());
	f.close();
	Serial.printf("Saving volume %s\n", String(volume).c_str());

}

void LogManager::addPaymentLog( uint32_t amount ){

	total_tips += amount;
	last_tip = amount;

	char *text = uint64_to_string(total_tips);
	File f = SPIFFS.open("/total.txt", FILE_WRITE);
	f.print(text);
	f.close();
	Serial.printf("Logging total %s\n", text);

	char *text2 = uint64_to_string(last_tip);
	f = SPIFFS.open("/last.txt", FILE_WRITE);
	f.print(text2);
	f.close();
	Serial.printf("Logging last %s\n", text2);

	


};

void LogManager::setAddress( String address, uint64_t time ){

	RECEIVING_ADDRESS = address;
	File f = SPIFFS.open("/address.txt", FILE_WRITE);
	f.print(address.c_str());
	f.close();
	Serial.println("Saved address");

	address_changed = time;
	char *text2 = uint64_to_string(time);
	f = SPIFFS.open("/address_changed.txt", FILE_WRITE);
	f.print(text2);
	f.close();
	Serial.printf("Last change %s\n", text2);

}

void LogManager::resetTips(){

	total_tips = 0;
	last_tip = 0;

	SPIFFS.remove("/total.txt");
	SPIFFS.remove("/last.txt");

}

char *LogManager::uint64_to_string(uint64_t input){

	if( !input )
		return "0";

    static char result[21] = "";
    // Clear result from any leftover digits from previous function call.
    memset(&result[0], 0, sizeof(result));
    // temp is used as a temporary result storage to prevent sprintf bugs.
    char temp[21] = "";
    char c;
    uint8_t base = 10;

    while( input ){

        int num = input % base;
        input /= base;
        c = '0' + num;

        sprintf(temp, "%c%s", c, result);
        strcpy(result, temp);

    } 
    return result;
}

uint64_t LogManager::atoll(const char* ptr){
	uint64_t result = 0;
	while (*ptr && isdigit(*ptr)) {
			result *= 10;
			result += *ptr++ - '0';
	}
	return result;
}
