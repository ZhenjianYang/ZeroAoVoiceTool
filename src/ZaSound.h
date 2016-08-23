#ifndef __ZASOUND_H__
#define __ZASOUND_H__

#include "ZaConst.h"

#define ZASOUND_STATUS_PLAYING 0
#define ZASOUND_STATUS_STOP    1

typedef int(*StopCallBack)(void*);

int ZaSoundInit(float volumn);

void ZaSoundSetVolumn(float volumn);
float ZaSoundGetVolumn();

int ZaSoundStatus();

bool ZaSoundPlay(const char* soundFile);
void ZaSoundStop();

void ZaSoundSetStopCallBack(StopCallBack stopCallBack = nullptr, void* param = nullptr);

#endif // !__ZASOUND_H__
