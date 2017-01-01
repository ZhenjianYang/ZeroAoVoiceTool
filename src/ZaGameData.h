#ifndef __ZA_GAME_DATA_H__
#define __ZA_GAME_DATA_H__

#include <string>
#include <vector>

#define STR_Name "Name"
#define STR_Base "Base"
#define STR_Enable "Enable"
#define STR_Title "Title"
#define STR_Comment "Comment"
#define STR_FeatureAddr "FeatureAddr"
#define STR_FeatureValue "FeatureValue"
#define STR_OpAddrJcLoadScena "OpAddrJcLoadScena"
#define STR_AddrLoadScena "AddrLoadScena"
#define STR_OpAddrJcLoadScena1 "OpAddrJcLoadScena1"
#define STR_AddrLoadScena1 "AddrLoadScena1"
#define STR_OpAddrJcLoadBlock "OpAddrJcLoadBlock"
#define STR_AddrLoadBlock "AddrLoadBlock"
#define STR_OpAddrJcShowText "OpAddrJcShowText"
#define STR_AddrShowText "AddrShowText"
#define STR_PtrPostMessageA "PtrPostMessageA"
#define STR_VoiceIdLegnth "VoiceIdLegnth"
#define STR_VoiceFileName "VoiceFileName"
#define STR_VoiceFileDir "VoiceFileDir"
#define STR_VoiceTablesDir "VoiceTablesDir"
#define STR_VoiceTablesDirEx "VoiceTablesDirEx"

namespace Za {
	namespace Data {
		class GameData
		{
		public:
			static int GetFromFiles(std::vector<GameData> &dataList, const char* dataFN, const char* dataCustomizedFN = nullptr);

		public:
			std::string Name;
			std::string Base;

			int Enable = 0;

			std::string Title;
			std::string Comment;

			unsigned FeatureAddr = 0;
			unsigned FeatureValue = 0;

			unsigned AddrOpJc[4] = { 0 };
			unsigned AddrFunc[4] = { 0 };

			unsigned PtrPostMessageA = 0;

			int VoiceIdLegnth = 0;
			std::string VoiceFileName;
			std::string VoiceFileDir;
			std::string VoiceTablesDir;
			std::string VoiceTablesDirEx;
		};
	}
}

#endif // !__ZA_GAME_DATA_H__
