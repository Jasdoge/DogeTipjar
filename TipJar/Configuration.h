#ifndef __Configuration
#define __Configuration
const String RECEIVING_ADDRESS = "D9WMuyMgLY2CMAEndsrDEKh4URwWPBdnSk"; 

// Uncomment these if you want to hardcode the wifi credentials. Useful if your password or SSID contains special characters.
// #define WIFI_SSID ""
// #define WIFI_PASS ""

const uint32_t TIP_TIME = 30000;	// Show tip for 30 sec when it arrives
const uint32_t QR_TIME = 30000;		// Show QR code splash for 30 sec
const uint32_t SCREENSAVER_TIME = 5000;	// Show screensaver for 5 sec to prevent burn-in

const uint8_t SOUND_VOLUME = 20;			// Between 1 and 30. Do be aware that the DFplayer usually crashes when going max volume. 

const uint32_t SOUND_BIG_THR = 1000;		// Above this value of doge received, play the big sound
const uint32_t SOUND_MED_THR = 100;			// Above this value received, play the med sound


#endif
