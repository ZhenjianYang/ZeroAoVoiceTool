#include "ZaSound.h"

#include <Windows.h>

#pragma comment(lib, "winmm.lib") 

bool ZaSoundPlay(const char * soundFile, bool wait /*= false*/)
{
	DWORD param = SND_FILENAME | SND_ASYNC | SND_NODEFAULT;
	if (wait) param |= SND_NOSTOP;
	return PlaySound(soundFile, NULL, param);
}
