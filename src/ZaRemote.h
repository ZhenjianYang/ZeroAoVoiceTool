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

//初始化，寻找游戏窗口、打开游戏进程并写入远程代码
//返回值：
//      GAMEID_AO:   窗口为碧之轨迹
//      GAMEID_ZERO: 窗口为零之轨迹
//      其他:      失败
int ZaRemoteInit(int mode);

void ZaRemoteFinish();

//检查游戏是否启动
//titles : 需要检查的游戏标题列表
//返回: 
//        >=0 已启动的游戏标题在标题列表中的编号
//        <0  游戏未启动 
int ZaCheckGameStart(int numTitles, const char* titles[]);

//检查游戏是否已结束
//返回:
//        true  游戏已结束
//        false 游戏未结束
bool ZaCheckGameEnd();

bool ZaOpenProcess();
//注入代码，若hWnd_this != 0，则为消息模式
bool ZaInjectRemoteCode(int gameId, int hWnd_this = 0, unsigned bMsg = 0);

bool ZaRemoteRead(unsigned rAdd, void *buff, unsigned size);
bool ZaRemoteWrite(unsigned rAdd, const void *buff, unsigned size);
unsigned ZaRemoteAlloc(unsigned size);
bool ZaRemoteFree(unsigned rAdd, unsigned size);

#endif // !__ZAREMOTE_H__
