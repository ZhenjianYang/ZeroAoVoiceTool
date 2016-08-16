#ifndef __ZACONFIG_H__
#define __ZACONFIG_H__

#include <string>

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


struct ZaConfigData
{
	std::string ao_dir_voice;
	std::string ao_name_voice;
	std::string ao_ext_voice;

	std::string ao_dir_voiceTable;
	std::string ao_ext_voiceTable;

	int ao_disableOriginalVoice;

	std::string zero_dir_voice;
	std::string zero_name_voice;
	std::string zero_ext_voiceFile;

	std::string zero_dir_voiceTable;
	std::string zero_ext_voiceTable;

	int debuglog;
	int uselogfile;

	int removeFwdCtrlCh;
	int sleepTime;
	int mode;
};
extern const ZaConfigData &zaConfigData;

void ZaConfigLoad(const char* configFile);

#endif // !__ZACONFIG_H__
