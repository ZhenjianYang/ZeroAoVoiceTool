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

#define STR_Volume "Volume"

#define STR_OpenDebugLog "OpenDebugLog"
#define STR_UseLogFile "UseLogFile"

#define STR_AutoStart "AutoStart"

#define CH_Equal '='

namespace Za {
	class Config {
	public:
		class ConfigData
		{
			friend class Config;
		public:
			struct GeneralConfig
			{
				int OpenDebugLog;
				int UseLogFile;
				int AutoStart;

				int RemoveFwdCtrlCh;
				int SleepTime;
				int Mode;
			};

			struct GameConfig
			{
				std::string VoiceDir;
				std::string VoiceName;
				std::vector<std::string> VoiceExt;

				std::string VoiceTableDir;
				std::string VoiceTableExt;

				int DisableOriginalVoice;
				int VoiceIdLength;
				int Volume;
			};

			const int ActiveGameID = m_ActiveGameID;
			const GameConfig* const &ActiveGame = m_pActiveGame;

			const GeneralConfig* const General = &m_general;
			const GameConfig* const Ao = &m_ao;
			const GameConfig* const Zero = &m_zero;

		private:
			int m_ActiveGameID;
			GameConfig* m_pActiveGame;
			GameConfig m_ao;
			GameConfig m_zero;
			GeneralConfig m_general;
		};

		static const ConfigData* const MainConfig;

		static bool LoadFromFile(const char* configFile, ConfigData* pConfigDo = nullptr);
		static bool SaveToFile(const char* configFile, ConfigData* pConfigDo = nullptr);
		static void SetActiveGame(int gameID, ConfigData* pConfigDo = nullptr);
		static void LoadDefault(ConfigData* pConfigDo = nullptr);
		//将ao或zero的配置项复制给pConfigDo
		static void Set(const ConfigData::GameConfig& gameConfig, int gameId, ConfigData* pConfigDo = nullptr);
		//将general的配置项复制给pConfigDo
		static void Set(const ConfigData::GeneralConfig& generalConfig, ConfigData* pConfigDo = nullptr);
	};
}

#endif // !__ZACONFIG_H__
