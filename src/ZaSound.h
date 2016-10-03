#ifndef __ZASOUND_H__
#define __ZASOUND_H__

#include "ZaConst.h"

#define ZASOUND_STATUS_PLAYING 0
#define ZASOUND_STATUS_STOP    1

typedef int(*StopCallBack)(void*);

int ZaSoundInit(int volumn = VOLUME_MAX);
int ZaSoundEnd();

void ZaSoundSetVolumn(int volumn);
float ZaSoundGetVolumn();

int ZaSoundStatus();

bool ZaSoundPlay(const char* soundFile);
void ZaSoundStop();

void ZaSoundSetStopCallBack(StopCallBack stopCallBack = nullptr, void* _param = nullptr);

#endif // !__ZASOUND_H__
