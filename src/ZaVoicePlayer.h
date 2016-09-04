#ifndef __ZAVOICEPLAYER_H__
#define __ZAVOICEPLAYER_H__

//��������ѭ����ʼ��
int ZaVoicePlayerInit(void* data = 0);
//��ֹ��������ѭ�����Ի�(�Ϸ�������һ�̵߳���ʱʹ��)
int ZaVoicePlayerEnd();

//����һ����������ѭ��
int ZaVoicePlayerLoopOne();

bool ZaPlayVoice(int voiceID, char *out_filename);

void ZaAddToWait(int voiceId);
void ZaClearWait();
int ZaWaitingNum();
int ZaPlayWait();


#endif // !__ZAVOICEPLAYER_H__
