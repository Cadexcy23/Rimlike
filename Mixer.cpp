#include <SDL_mixer.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include "Mixer.h"

//TEMP AS F
std::vector<Mix_Chunk*> Mixer::sMove;

//Declare sounds in a header -> define them in that headers respective file (maybe here) -> load em in the load media funcion in the mixer file
//how to load a sound
//Mix_Chunk* Class::sSoundTest = NULL;
//how to load music
//Mix_Music* Class::sMusicTest = NULL;



int Mixer::changeVolume(int volume)
{
	return Mix_Volume(-1, volume);
}

void Mixer::playSound(Mix_Chunk* sound)
{
	Mix_PlayChannel(-1, sound, 0);
}

bool Mixer::loadSoundMedia()
{
	bool success = true;

	//Declare sounds in a header -> define them in that headers respective file -> load em in here
	//how to load a sound
	//Class::sTestSound = Mix_LoadWAV("Resource/sound/soundFX/testSound.wav");
	//how to load music
	//Class::sTestMusic = Mix_LoadMUS("Resource/sound/music/sestMusic.wav");

	//seperate by class for good organization    maybe if i lived in the fucking stone age loser
	//write load functions here \/

	
	//turn down the m Fing volume
	Mix_Volume(-1, 0);

	//TEMP AS F
	sMove.push_back(Mix_LoadWAV("Resource/entities/gotIt.wav"));
	sMove.push_back(Mix_LoadWAV("Resource/entities/imGoin.wav"));
	sMove.push_back(Mix_LoadWAV("Resource/entities/mhmm.wav"));
	sMove.push_back(Mix_LoadWAV("Resource/entities/okay.wav"));
	sMove.push_back(Mix_LoadWAV("Resource/entities/yep.wav"));

	return success;
}