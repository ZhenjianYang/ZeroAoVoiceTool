#include "ZaRemote.h"
#include "ZaRemoteAsm.h"
#include "ZaGameData.h"
#include "ZaData.h"
#include "ZaCommonMethod.h"
#include "ZaErrorMsg.h"
#include "ZaConst.h"

#include <Windows.h>

#include <vector>
#include <map>

#define MAX_TITLE_LENGTH 50
#define MAX_COMMENT_LEGNTH 200
#define MAX_REMOTE_DADA_SIZE 512

static const Za::Data::GameData* _curGameData = nullptr;
const Za::Data::GameData* const &Za::Remote::CurGameData = _curGameData;
static Za::Data::GameProcessIn _gameProcessIn;
const Za::Data::GameProcessIn* const &Za::Remote::CurGameProcessIn = &_gameProcessIn;

static unsigned _remoteDataAddr;
static unsigned _remoteDataSize;

static std::vector<Za::Data::GameData> _gameDataList;
static std::map<std::string, std::vector<int>> _mTitleIdx;

static bool _openProcess();
static bool _injectRemoteCode();
static bool _cleanRemoteData();

static HWND _hWnd = 0;
static HANDLE _hProcess = 0;

static int _disableOriVoice = 0;

bool Za::Remote::Init()
{
	if (Za::Data::GameData::GetFromFiles(_gameDataList, DATA_FILENAME, DATACSTM_FILENAME) == 0) {
		Za::Error::SetErrMsg("未能获取任何游戏数据信息！");
		return false;
	}
	_mTitleIdx.clear();
	for(int i = 0; i < (int)_gameDataList.size(); ++i) {
		if (_gameDataList[i].Enable) {
			_mTitleIdx[_gameDataList[i].Title].push_back(i);
		}
	}

#if !_DEBUG
	//对旧系统的支持
	OSVERSIONINFO OSVersionInfo;
	memset(&OSVersionInfo, 0, sizeof(OSVERSIONINFO));
	OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (!GetVersionEx(&OSVersionInfo) || OSVersionInfo.dwMajorVersion <= 5) {
		enableDebugPriv();
	}
#endif

	return true;
}

bool Za::Remote::End()
{
	_gameDataList.clear();
	_mTitleIdx.clear();
	return true;
}

bool Za::Remote::CheckGameEnd()
{
	DWORD code;
	if (!GetExitCodeProcess(_hProcess, &code) || code != STILL_ACTIVE)
		return true;
	return false;;
}

bool Za::Remote::OpenGameProcess(Data::GameProcessOut & gpOut, const Data::GameProcessIn & gpIn)
{
	_gameProcessIn = gpIn;
	static char buff_title[MAX_TITLE_LENGTH + 1];
	static char buff_comment[MAX_COMMENT_LEGNTH + 1];

	buff_title[0] = buff_comment[0] = '\0';

	if (!_openProcess()) {
		Za::Error::SetErrMsg();
		return false;
	}

	if (!_injectRemoteCode()) {
		Za::Error::SetErrMsg("写入远程代码失败！");
		return false;
	}

	CpyStrToArray(buff_title, _curGameData->Title.c_str());
	CpyStrToArray(buff_comment, _curGameData->Comment.c_str());

	gpOut.Title = buff_title;
	gpOut.Comment = buff_comment;
	gpOut.RemoteDataAddr = _remoteDataAddr;
	gpOut.RemoteDataSize = _remoteDataSize;

	return true;
}

bool Za::Remote::CloseGameProcess()
{
	bool ret = CheckGameEnd() || _cleanRemoteData();
	ret = ret && TRUE == CloseHandle(_hProcess);

	_hProcess = NULL;
	_hWnd = NULL;

	return ret;
}

bool Za::Remote::DisableOriVoice(int op)
{
	if (_disableOriVoice == op) return true;
	
	_disableOriVoice = op;
	if (!_hProcess) return true;

	return Za::Remote::RemoteWrite(_remoteDataAddr + OFF_disableOriVoice, &_disableOriVoice, sizeof(_disableOriVoice));
}

bool Za::Remote::RemoteRead(unsigned rAdd, void *buff, unsigned size) {
	return ReadProcessMemory(_hProcess, (LPVOID)rAdd, buff, size, NULL) == TRUE;
}

bool Za::Remote::RemoteWrite(unsigned rAdd, const void *buff, unsigned size) {
	return WriteProcessMemory(_hProcess, (LPVOID)rAdd, buff, size, NULL) == TRUE;
}

unsigned Za::Remote::RemoteAlloc(unsigned size) {
	return (unsigned)VirtualAllocEx(_hProcess, NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
}

bool Za::Remote::RemoteFree(unsigned rAdd, unsigned size) {
	return VirtualFreeEx(_hProcess, (LPVOID)rAdd, size, MEM_DECOMMIT) == TRUE;
}

#if !_DEBUG
static bool enableDebugPriv()
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

bool _openProcess()
{
	_hWnd = NULL;
	_hProcess = NULL;

	for (const auto& it : _mTitleIdx) {
		const std::string& title = it.first;
		const std::vector<int>& idxlist = it.second;

		_hWnd = ::FindWindowA(NULL, title.c_str());
		if (_hWnd) {
			DWORD pid;

			GetWindowThreadProcessId(_hWnd, &pid);
			_hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);

			if (_hProcess) {
				_hWnd = NULL;
				unsigned fv;

				for (auto idx : idxlist) {
					if (_gameDataList[idx].FeatureAddr == 0
						|| ReadProcessMemory(_hProcess, (LPVOID)_gameDataList[idx].FeatureAddr, &fv, sizeof(fv), NULL) == TRUE
						&& fv == _gameDataList[idx].FeatureValue){
						_curGameData = &_gameDataList[idx];
						return true;
					}
				}

				CloseHandle(_hProcess);
			}
		}
	}

	return false;
}

#define REMOTE_DATA_PACK 0x10
#define PACK(p, pack) (p = (p + (pack) - 1) / (pack) * (pack))

bool _injectRemoteCode()
{
	const unsigned PtrPostMessageA = _curGameData->PtrPostMessageA;
	const unsigned *AddrOpJcList = _curGameData->AddrOpJc;
	const unsigned *AddrFuncList = _curGameData->AddrFunc;
	const void* NewFuncList[] = { Za::Remote::rNewLoadScenaMsg , Za::Remote::rNewLoadScena1Msg, Za::Remote::rNewLoadBlockMsg, Za::Remote::rNewShowTextMsg };
	const int cnt = sizeof(NewFuncList) / sizeof(*NewFuncList);
	unsigned AddrNewFuncList[cnt];

	unsigned char buff[MAX_REMOTE_DADA_SIZE];
	memset(buff, INIT_CODE, sizeof(buff));

	unsigned p = sizeof(Za::Remote::RemoteData); PACK(p, REMOTE_DATA_PACK);
	for (int i = 0; i < cnt; ++i) {
		AddrNewFuncList[i] = p;
		unsigned char *t = (unsigned char *)NewFuncList[i];
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
	_remoteDataSize = p;

	_remoteDataAddr = Za::Remote::RemoteAlloc(_remoteDataSize);
	if (!_remoteDataAddr) {
		Za::Error::SetErrMsg("分配远程数据空间失败！");
		return false;
	}

	//将占位指令修改为本来的指令
	for (int i = 0; i < cnt; ++i) {
		p = AddrNewFuncList[i];
		for (;;) {
			if (*(unsigned *)(buff + p) == FAKE_RAZADATA) {
				*(unsigned *)(buff + p) = _remoteDataAddr; p += 4;
			}
			else if (*(unsigned *)(buff + p) == FAKE_MESSAGE_ID) {
				*(unsigned *)(buff + p) = _gameProcessIn.msgId; p += 4;
			}
			else if (*(unsigned *)(buff + p) == FAKE_HWND) {
				*(unsigned *)(buff + p) = _gameProcessIn.hMainWindow; p += 4;
			}
			else if (*(unsigned *)(buff + p) == FAKE_TYPE) {
				*(unsigned *)(buff + p) = i; p += 4;
			}
			else if (*(unsigned *)(buff + p) == FAKE_PTR_API) {
				*(unsigned *)(buff + p) = PtrPostMessageA;
				p += 4;
			}
			else if (buff[p] == FAKE_CODE2
				&& buff[p + 1] == FAKE_CODE2 && buff[p + 2] == FAKE_CODE2
				&& buff[p + 3] == FAKE_CODE2 && buff[p + 4] == FAKE_CODE2) {
				buff[p] = JMP_CODE; p++;
				*(unsigned *)(buff + p) = AddrFuncList[i] - (_remoteDataAddr + p + 4); p += 4;
				break;
			}
			else
				++p;
		}
	}
	memset(buff, 0, sizeof(Za::Remote::RemoteData));
	((Za::Remote::RemoteData*)buff)->disableOriVoice = _disableOriVoice;

	if (!Za::Remote::RemoteWrite(_remoteDataAddr, buff, _remoteDataSize)) {
		Za::Remote::RemoteFree(_remoteDataAddr, _remoteDataSize);
		Za::Error::SetErrMsg("写入远程数据失败！");
		return false;
	}
	for (int i = 0; i < cnt; ++i) {
		AddrNewFuncList[i] += _remoteDataAddr;
		unsigned ljmp = AddrNewFuncList[i] - AddrOpJcList[i] - 5;
		if (!Za::Remote::RemoteWrite(AddrOpJcList[i] + 1, &ljmp, sizeof(ljmp))) {
			Za::Error::SetErrMsg("写入远程数据失败！");
			return false;
		}
	}

	return true;
}

bool _cleanRemoteData()
{
	if (!_hProcess) return true;

	for (int i = 0; i < sizeof(_curGameData->AddrOpJc) / sizeof(*_curGameData->AddrOpJc); ++i) {
		unsigned ljmp = _curGameData->AddrFunc[i] - _curGameData->AddrOpJc[i] - 5;
		if (!Za::Remote::RemoteWrite(_curGameData->AddrOpJc[i] + 1, &ljmp, sizeof(ljmp))) {
			Za::Error::SetErrMsg("清理远程代码失败！");
			return false;
		}
	}

	::Sleep(5);
	return Za::Remote::RemoteFree(_remoteDataAddr, _remoteDataSize);
}
