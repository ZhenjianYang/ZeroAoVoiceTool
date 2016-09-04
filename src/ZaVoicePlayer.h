#ifndef __ZAVOICEPLAYER_H__
#define __ZAVOICEPLAYER_H__

//语音播放循环初始化
int ZaVoicePlayerInit();
//结束语音播放循环
int ZaVoicePlayerEnd();

//进行一次语音播放循环
int ZaVoicePlayerLoopOne();

bool ZaPlayVoice(int voiceID, char *out_filename);

void ZaAddToWait(int voiceId);
void ZaClearWait();
int ZaWaitingNum();
int ZaPlayWait();


#endif // !__ZAVOICEPLAYER_H__
