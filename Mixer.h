#pragma once
#ifndef SDL_mixer
#include <SDL_mixer.h>
#endif
#ifndef vector
#include <vector>
#endif

class Mixer {
public:

	//Declare sounds in a header (maybe here) -> define them in that headers respective file -> load em in the load media funcion in the mixer file
	//how to declare a sound
	//static Mix_Chunk* sTestSound;
	//how to declare music
	//static Mix_Music* sTestMusic;


	int changeVolume(int volume);

	void playSound(Mix_Chunk* sound);

	bool loadSoundMedia();

	//TEMP AS F
	static std::vector<Mix_Chunk*> sMove;
};
