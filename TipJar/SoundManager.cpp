#include "SoundManager.h"

DFPlayerMini_Fast SoundManager::player = DFPlayerMini_Fast();

void SoundManager::setup(){
	
	Serial2.begin(9600, SERIAL_8N1, 16, 17);
	player.begin(Serial2);
	player.volume(20);

}

void SoundManager::play( const uint8_t sound ){

	player.play(sound);

}



