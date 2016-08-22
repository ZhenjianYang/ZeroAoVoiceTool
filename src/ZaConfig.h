#ifndef __ZACONFIG_H__
#define __ZACONFIG_H__

#include <string>
#include <vector>

#define STR_general		"[General]"
#define STR_Ao			"[Ao]"
#define STR_Zero		"[Zero]"

#define STR_VoiceFileDirectory		"VoiceFileDirectory"
#define STR_VoiceFileExtension		"VoiceFileExtension"

#define STR_VoiceTableFileDirectory "VoiceTableFileDirectory"
#define STR_VoiceTableFileExtension "VoiceTableFileExtension"

#define STR_DisableOriginalVoice	"DisableOriginalVoice"

#define STR_OpenDebugLog "OpenDebugLog"
#define STR_UseLogFile "UseLogFile"

#define STR_Volume "Volume"

struct ZaConfigGame
{
	std::string VoiceDir;
	std::string VoiceName;
	std::vector<std::string> VoiceExt;

	std::string VtblDir;
	std::string VtblExt;

	int DisableOriginalVoice;
	int VoiceIdLength;
	int Volume;
};

struct ZaConfigGeneral
{
	int OpenDebugLog;
	int UseLogFile;

	int RemoveFwdCtrlCh;
	int SleepTime;
	int Mode;
};

struct ZaConfig
{
	int ActiveGameID;
	const ZaConfigGame* ActiveGame;

	ZaConfigGeneral General;
	ZaConfigGame Ao;
	ZaConfigGame Zero;
};
extern const ZaConfig* const &g_zaConfig;

void ZaConfigLoad(const char* configFile);
void ZaConfigSetActive(int gameID);
void ZaConfigSetDefault();

#endif // !__ZACONFIG_H__
