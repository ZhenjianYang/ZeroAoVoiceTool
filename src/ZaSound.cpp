#include "ZaSound.h"

#include <SFML/Audio/Music.hpp>

static sf::Music music;

int ZaSoundInit(float volumn)
{
	ZaSoundStop();
	ZaSoundSetVolumn(volumn);
	return 0;
}

void ZaSoundSetVolumn(float volumn)
{
	music.setVolume(volumn);
}

float ZaSoundGetVolumn()
{
	return music.getVolume();
}

int ZaSoundStatus()
{
	switch (music.getStatus())
	{
	case music.Stopped:
		return ZASOUND_STATUS_STOP;
	case music.Playing:
		return ZASOUND_STATUS_PLAYING;
	case music.Paused:
	default:
		return ZASOUND_STATUS_PAUSE;
	}
}

bool ZaSoundPlay(const std::string& soundFile)
{
	music.stop();
	if (!music.openFromFile(soundFile)) return false;
	music.play();
	return true;
}

void ZaSoundStop()
{
	music.stop();
}
