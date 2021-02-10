#ifndef __SoundManager
#define __SoundManager
#include <DFPlayerMini_Fast.h>
#include <Arduino.h>

class SoundManager{

	public:
		static const uint8_t SOUND_MONEY_LARGE = 1;		// Sound file index, starting from 1. This is the index it was copied to the card.
		static const uint8_t SOUND_MONEY_MED = 2;
		static const uint8_t SOUND_MONEY_SMALL = 3;
		static const uint8_t SOUND_CLICK = 4;

		static DFPlayerMini_Fast player;
	
		static void setup();
		static void play( const uint8_t sound );
		static void setVolume( uint8_t volume );

};





#endif
