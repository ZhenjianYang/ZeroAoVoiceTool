#include "ZaRemote.h"
#include "ZaLog.h"
#include "ZaRemoteAsm.h"
#include "ZaConfig.h"

#include <Windows.h>

static unsigned _rAddZaData;
static unsigned _rSizeZaData;
const unsigned &g_rAddData = _rAddZaData;
const unsigned &g_rSizeData = _rSizeZaData;

static HWND hWnd = NULL;
static HANDLE hProcess = NULL;

static const unsigned z_rOldJctoList[] = { Z_OLD_JCTO_LOADSCENA, Z_OLD_JCTO_LOADSCENA1, Z_OLD_JCTO_LOADBLOCK , Z_OLD_JCTO_SHOWTEXT };
static const unsigned z_rAddrJcList[] = { Z_ADD_JC_LOADSCENA, Z_ADD_JC_LOADSCENA1, Z_ADD_JC_LOADBLOCK, Z_ADD_JC_SHOWTEXT };
static const unsigned z_ptr_apipmsg = Z_PTR_API_PostMessageA;

static const unsigned a_rOldJctoList[] = { A_OLD_JCTO_LOADSCENA, A_OLD_JCTO_LOADSCENA1, A_OLD_JCTO_LOADBLOCK , A_OLD_JCTO_SHOWTEXT };
static const unsigned a_rAddrJcList[] = { A_ADD_JC_LOADSCENA, A_ADD_JC_LOADSCENA1, A_ADD_JC_LOADBLOCK, A_ADD_JC_SHOWTEXT };
static const unsigned a_ptr_apipmsg = A_PTR_API_PostMessageA;

static const unsigned msgId_add[] = { MSGID_ADDER_LOADSCENA, MSGID_ADDER_LOADSCENA1, MSGID_ADDER_LOADBLOCK, MSGID_ADDER_SHOWTEXT };

static const void* rNewCJList[] = { rNewLoadScena , rNewLoadScena1, rNewLoadBlock, rNewShowText };
static const void* rNewCJListMsg[] = { rNewLoadScenaMsg , rNewLoadScena1Msg, rNewLoadBlockMsg, rNewShowTextMsg };
const int numJc = sizeof(rNewCJList) / sizeof(*rNewCJList);
const int irNewShowText = numJc - 1;

static unsigned rAddrNewJc[numJc];

#define MAX_REMOTE_DADA_SIZE 512
#define REMOTE_DATA_PACK 0x10
#define PACK(p, pack) (p = (p + (pack) - 1) / (pack) * (pack))

static bool enableDebugPriv();

static int WaitGameStart(int mode)
{
	const char* tbuf[] = { Z_DFT_WIN_TITLE, A_DFT_WIN_TITLE };
	if (mode == MODE_AO) tbuf[0] = tbuf[1];
	else if (mode == MODE_ZERO) tbuf[1] = tbuf[0];

	hWnd = NULL;
	int index = -1;
	while ((index = ZaCheckGameStart(sizeof(tbuf) / sizeof(*tbuf), tbuf)) < 0);

	ZALOG("游戏标题为：%s", tbuf[index]);

	if (mode == MODE_AUTO) return index == 0 ? GAMEID_ZERO : GAMEID_AO;
	else if (mode == MODE_AO) return GAMEID_AO;
	else return GAMEID_ZERO;
}

int ZaCheckGameStart(int numTitles, const char* titles[]) {
	hWnd = NULL;
	int index;
	for (index = numTitles - 1; index >= 0; --index) {
		hWnd = ::FindWindowA(NULL, titles[index]);
		if (hWnd) break;
	}
	return index;
}

bool ZaCheckGameEnd() {
	DWORD code;
	if (!GetExitCodeProcess(hProcess, &code) || code != STILL_ACTIVE)
		return true;
	return false;
}

bool ZaOpenProcess() {
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
	return hProcess != NULL;
}

bool ZaInjectRemoteCode(int gameId, int hWnd_this /*= 0*/, unsigned bMsg /*= 0*/) {
	const unsigned ptr_apipmsg = gameId == GAMEID_AO ? a_ptr_apipmsg : z_ptr_apipmsg;
	const unsigned *rOldJctoList = gameId == GAMEID_AO ? a_rOldJctoList : z_rOldJctoList;
	const unsigned *rAddJcList = gameId == GAMEID_AO ? a_rAddrJcList : z_rAddrJcList;

	unsigned char buff[MAX_REMOTE_DADA_SIZE];
	memset(buff, INIT_CODE, sizeof(buff));

	unsigned p = sizeof(ZAData); PACK(p, REMOTE_DATA_PACK);
	for (int i = 0; i < numJc; ++i) {
		rAddrNewJc[i] = p;
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
	ZALOG_DEBUG("分配远程数据空间成功。地址：0x%08X, 大小：0x%04X", _rAddZaData, _rSizeZaData);

	//将占位指令修改为本来的指令
	for (int i = 0; i < numJc; ++i) {
		p = rAddrNewJc[i];
		for (;;) {
			if (*(unsigned *)(buff + p) == FAKE_RAZADATA) {
				*(unsigned *)(buff + p) = _rAddZaData; p += 4;
			}
			else if (*(unsigned *)(buff + p) == FAKE_MESSAGE_ID){
				*(unsigned *)(buff + p) = msgId_add[i] + bMsg; p += 4;
			}
			else if (*(unsigned *)(buff + p) == FAKE_HWND) {
				*(unsigned *)(buff + p) = hWnd_this; p += 4;
			}
			else if (*(unsigned *)(buff + p) == FAKE_PTR_API) {
				*(unsigned *)(buff + p) = ptr_apipmsg;
				buff[p - 1] = FAKE_CALLPTR_CODE2;
				buff[p - 2] = FAKE_CALLPTR_CODE1;
				p += 4;
			}
			else if (buff[p] == FAKE_CODE2
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
	((ZAData*)buff)->disableOriVoice = gameId == GAMEID_AO && g_zaConfig->Ao.DisableOriginalVoice;

	ZALOG_DEBUG("写入远程数据...");
	if (!ZaRemoteWrite(_rAddZaData, buff, _rSizeZaData)) {
		ZaRemoteFree(_rAddZaData, _rSizeZaData);
		ZALOG_ERROR("写入远程远程数据失败！");
		return false;
	}
	for (int i = 0; i < numJc; ++i) {
		rAddrNewJc[i] += _rAddZaData;
		unsigned ljmp = rAddrNewJc[i] - rAddJcList[i] - 5;
		if (!ZaRemoteWrite(rAddJcList[i] + 1, &ljmp, sizeof(ljmp))) {
			ZALOG_ERROR("写入远程远程数据失败！");
			return false;
		}
	}

	return true;
}

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
	int gameId = WaitGameStart(mode);
	ZALOG("游戏已启动！");

	ZALOG("访问游戏进程...");
	if (!ZaOpenProcess()) {
		ZaRemoteFinish();
		ZALOG_ERROR("访问游戏进程失败！");
		return GAMEID_INVALID;
	}
	ZALOG("访问游戏进程成功");

	ZALOG("写入远程代码...");
	if (!ZaInjectRemoteCode(gameId)) {
		ZALOG_ERROR("写入远程代码失败！");
		ZaRemoteFinish();
		return GAMEID_INVALID;
	}
	ZALOG("写入远程代码成功");

	return gameId;
}

void ZaRemoteFinish()
{
	CloseHandle(hProcess); 
	hProcess = NULL;
	hWnd = NULL;
}

#if !_DEBUG
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
#endif // if !_DEBUG
