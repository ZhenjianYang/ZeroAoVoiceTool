#ifndef __ZAREMOTE_H__
#define __ZAREMOTE_H__

struct ZAData
{
	unsigned aScena;
	unsigned aScena1;
	unsigned aScena2;
	unsigned cScena;

	unsigned aCurBlock;
	unsigned cBlock;

	unsigned aCurText;
	unsigned aFirstText;
	unsigned cText;
	unsigned flag;

	unsigned disableOriVoice;
};

#define OFFZAD_aScena			0x00
#define OFFZAD_aScena1			0x04
#define OFFZAD_aScena2			0x08
#define OFFZAD_cScena			0x0C

#define OFFZAD_aCurBlock		0x10
#define OFFZAD_cBlock			0x14

#define OFFZAD_aCurText			0x18
#define OFFZAD_aFirstText		0x1C
#define OFFZAD_cText			0x20
#define OFFZAD_flag				0x24

#define OFFZAD_disableOriVoice	0x28

extern const unsigned &g_rAddData;
extern const unsigned &g_rSizeData;

//�ȴ���Ϸ����
//����ֵ��
//      GAMEID_AO:   ����Ϊ��֮�켣
//      GAMEID_ZERO: ����Ϊ��֮�켣
//      ����:      ʧ��
int ZaRemoteWaitGameStart(int mode);

//��ʼ��������Ϸ���̲�д��Զ�̴���
//��Ҫ��������ķ�����ȡ gameID
//hWnd_this : �����ڵľ������0ʱΪ������Ϣ��ģʽ
int ZaRemoteInit(int hWnd_this = 0, unsigned bMsg = 0);

void ZaRemoteEnd();

//�����Ϸ�Ƿ�����
//titles : ��Ҫ������Ϸ�����б�
//����: 
//        >=0 ����������Ϸ�����ڱ����б��еı��
//        <0  ��Ϸδ���� 
int ZaCheckGameStart(int numTitles, const char* titles[]);

//�����Ϸ�Ƿ��ѽ���
//����:
//        true  ��Ϸ�ѽ���
//        false ��Ϸδ����
bool ZaCheckGameEnd();

bool ZaRemoteRead(unsigned rAdd, void *buff, unsigned size);
bool ZaRemoteWrite(unsigned rAdd, const void *buff, unsigned size);
unsigned ZaRemoteAlloc(unsigned size);
bool ZaRemoteFree(unsigned rAdd, unsigned size);

#endif // !__ZAREMOTE_H__
