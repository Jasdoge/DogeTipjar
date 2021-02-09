#include "LogManager.h"

uint64_t LogManager::total_tips = 0;
uint64_t LogManager::last_tip = 0;



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

	

};

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



char *LogManager::uint64_to_string(uint64_t input){

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
