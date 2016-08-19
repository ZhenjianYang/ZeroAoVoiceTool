#include "ZaRemote.h"
#include "ZaLog.h"
#include "ZaRemoteAsm.h"
#include "ZaConfig.h"

#include <Windows.h>

static unsigned _rAddZaData;
static unsigned _rSizeZaData;
const unsigned &rAddData = _rAddZaData;
const unsigned &rSizeData = _rSizeZaData;

static HWND hWnd = NULL;
static HANDLE hProcess = NULL;

static bool enableDebugPriv();

static int ZaFindGameWindow(int mode)
{
	const char* tbuf[] = { Z_DFT_WIN_TITLE, A_DFT_WIN_TITLE };
	if (mode == MODE_AO) tbuf[0] = tbuf[1];
	else if (mode == MODE_ZERO) tbuf[1] = tbuf[0];

	hWnd = NULL;
	int index = 0;
	while (!(hWnd = ::FindWindowA(NULL, tbuf[index])))
	{
		index = 1 - index;
		if(index == 0) Sleep(1000);
	}
	
	ZALOG("游戏标题为：%s", tbuf[index]);

	if (strcmp(tbuf[index], Z_DFT_WIN_TITLE) == 0) return MODE_ZERO;
	else return MODE_AO;
}
static bool ZaOpenProcess() {
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	return hProcess != NULL;
}

static const unsigned z_rOldJctoList[] = { Z_OLD_JCTO_LOADSCENA, Z_OLD_JCTO_LOADSCENA1, Z_OLD_JCTO_LOADBLOCK , Z_OLD_JCTO_SHOWTEXT };
static const unsigned z_rAddJcList[] = { Z_ADD_JC_LOADSCENA, Z_ADD_JC_LOADSCENA1, Z_ADD_JC_LOADBLOCK, Z_ADD_JC_SHOWTEXT };

static const unsigned a_rOldJctoList[] = { A_OLD_JCTO_LOADSCENA, A_OLD_JCTO_LOADSCENA1, A_OLD_JCTO_LOADBLOCK , A_OLD_JCTO_SHOWTEXT };
static const unsigned a_rAddJcList[] = { A_ADD_JC_LOADSCENA, A_ADD_JC_LOADSCENA1, A_ADD_JC_LOADBLOCK, A_ADD_JC_SHOWTEXT };

static const void* rNewCJList[] = { rNewLoadScena , rNewLoadScena1, rNewLoadBlock, rNewShowText };
const int numJc = sizeof(rNewCJList) / sizeof(*rNewCJList);
const int irNewShowText = numJc - 1;

static unsigned rAddNewJc[numJc];

#define MAX_REMOTE_DADA_SIZE 512
#define REMOTE_DATA_PACK 0x10
#define PACK(p, pack) (p = (p + (pack) - 1) / (pack) * (pack))

static bool ZaInjectRemoteCode(int mode) {

	const unsigned *rOldJctoList = mode == MODE_AO ? a_rOldJctoList : z_rOldJctoList;
	const unsigned *rAddJcList = mode == MODE_AO ? a_rAddJcList : z_rAddJcList;

	unsigned char buff[MAX_REMOTE_DADA_SIZE];
	memset(buff, INIT_CODE, sizeof(buff));

	unsigned p = sizeof(ZAData); PACK(p, REMOTE_DATA_PACK);
	for (int i = 0; i < numJc; ++i) {
		rAddNewJc[i] = p;
		unsigned char *t = (unsigned char *)rNewCJList[i];
		if (*t == JMP_CODE) t += *(unsigned *)(t + 1) + 5;//debug版对策

		for (;;) {
			if (*t == FAKE_CODE2
				&& *(t + 1) == FAKE_CODE2 && *(t + 2) == FAKE_CODE2
				&& *(t + 3) == FAKE_CODE2 && *(t + 4) == FAKE_CODE2) {
				for (int j = 1; j <= 5; ++j) { buff[p] = FAKE_CODE2; p++; t++; }
				break;
			}
			else {
				buff[p] = *t;
				p++; t++;
			}
		}
		PACK(p, REMOTE_DATA_PACK);
	}
	_rSizeZaData = p;

	ZALOG_DEBUG("分配远程数据空间...");
	_rAddZaData = ZaRemoteAlloc(_rSizeZaData);
	if (!_rAddZaData) {
		ZALOG_ERROR("分配远程数据空间失败！");
		return false;
	}
	ZALOG_DEBUG("分配远程数据空间成功。地址：0x%08X, 大小：0x%04X", _rAddZaData,  _rSizeZaData);
	
	//将占位指令修改为本来的指令
	for (int i = 0; i < numJc; ++i) {
		p = rAddNewJc[i];
		for (;;) {
			if (buff[p] == FAKE_CODE1
				&& buff[p + 1] == FAKE_CODE1 && buff[p + 2] == FAKE_CODE1
				&& buff[p + 3] == FAKE_CODE1 && buff[p + 4] == FAKE_CODE1) {
				buff[p] = MOVEBX_CODE; p++;
				*(unsigned *)(buff + p) = _rAddZaData; p += 4;

				if (i == irNewShowText &&
					(mode == MODE_AO && zaConfigData.Ao.DisableOriginalVoice)) {
					for (int j = 0; j < 2; ++j)
						buff[p++] = CODE_NOP;
				}
				else
					p += 2;
			}
			else if(buff[p] == FAKE_CODE2
				&& buff[p + 1] == FAKE_CODE2 && buff[p + 2] == FAKE_CODE2
				&& buff[p + 3] == FAKE_CODE2 && buff[p + 4] == FAKE_CODE2) {
				buff[p] = JMP_CODE; p++;
				*(unsigned *)(buff + p) = rOldJctoList[i] - (_rAddZaData + p + 4); p += 4;
				break;
			}
			else
				++p;
		}
	}
	memset(buff, 0, sizeof(ZAData));

	ZALOG_DEBUG("写入远程数据...");
	if (!ZaRemoteWrite(_rAddZaData, buff, _rSizeZaData)) {
		ZaRemoteFree(_rAddZaData, _rSizeZaData);
		ZALOG_ERROR("写入远程远程数据失败！");
		return false;
	}
	for (int i = 0; i < numJc; ++i) {
		rAddNewJc[i] += _rAddZaData;
		unsigned ljmp = rAddNewJc[i] - rAddJcList[i] - 5;
		if (!ZaRemoteWrite(rAddJcList[i] + 1, &ljmp, sizeof(ljmp))) {
			ZALOG_ERROR("写入远程远程数据失败！");
			return false;
		}
	}

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////

bool ZaRemoteRead(unsigned rAdd, void *buff, unsigned size) {
	return ReadProcessMemory(hProcess, (LPVOID)rAdd, buff, size, NULL);
}

bool ZaRemoteWrite(unsigned rAdd, const void *buff, unsigned size) {
	return WriteProcessMemory(hProcess, (LPVOID)rAdd, buff, size, NULL);
}

unsigned ZaRemoteAlloc(unsigned size) {
	return (unsigned)VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

bool ZaRemoteFree(unsigned rAdd, unsigned size) {
	return VirtualFreeEx(hProcess, (LPVOID)rAdd, size, MEM_DECOMMIT);
}

int ZaRemoteInit(int mode)
{
#if !_DEBUG
	//对旧系统的支持
	OSVERSIONINFO OSVersionInfo;
	memset(&OSVersionInfo, 0, sizeof(OSVERSIONINFO));
	OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&OSVersionInfo) || OSVersionInfo.dwMajorVersion <= 5) {
		enableDebugPriv();
	}
#endif

	ZALOG("等待游戏运行...");
	mode = ZaFindGameWindow(mode);
	ZALOG("游戏已启动！");

	ZALOG("访问游戏进程...");
	if (!ZaOpenProcess()) {
		ZaRemoteFinish();
		ZALOG_ERROR("访问游戏进程失败！");
		return MODE_NONE;
	}
	ZALOG("访问游戏进程成功");

	ZALOG("写入远程代码...");
	if (!ZaInjectRemoteCode(mode)) {
		ZALOG_ERROR("写入远程代码失败！");
		ZaRemoteFinish();
		return MODE_NONE;
	}
	ZALOG("写入远程代码成功");

	return mode;
}

void ZaRemoteFinish()
{
	CloseHandle(hProcess); 
	hProcess = NULL;
	hWnd = NULL;
}

bool enableDebugPriv()
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tkp;

	if (!OpenProcessToken(GetCurrentProcess(),
		TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		return false;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sedebugnameValue))
	{
		CloseHandle(hToken);
		return false;
	}
	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = sedebugnameValue;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		CloseHandle(hToken);
		return false;
	}
	return true;
}
