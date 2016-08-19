#ifndef __ZASOUND_H__
#define __ZASOUND_H__

#include <string>

#define ZASOUND_STATUS_PLAYING 0
#define ZASOUND_STATUS_STOP    1
#define ZASOUND_STATUS_PAUSE   2

int ZaSoundInit(float volumn);

void ZaSoundSetVolumn(float volumn);
float ZaSoundGetVolumn();

int ZaSoundStatus();

bool ZaSoundPlay(const std::string& soundFile);
void ZaSoundStop();

#endif // !__ZASOUND_H__
