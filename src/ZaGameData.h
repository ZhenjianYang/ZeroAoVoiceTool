#ifndef __ZA_GAME_DATA_H__
#define __ZA_GAME_DATA_H__

#include <string>
#include <vector>

#define STR_Enbale "Enable"
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

namespace Za {
	class GameData
	{
	public:
		static int AddFromFile(std::vector<GameData> &dataList, const char* dataFileName);

	public:
		bool Enable;

		std::string Title;
		std::string Comment;

		unsigned FeatureAddr;
		unsigned FeatureValue;

		unsigned OpAddrJcLoadScena;
		unsigned AddrLoadScena;

		unsigned OpAddrJcLoadScena1;
		unsigned AddrLoadScena1;

		unsigned OpAddrJcLoadBlock;
		unsigned AddrLoadBlock;

		unsigned OpAddrJcShowText;
		unsigned AddrShowText;

		unsigned PtrPostMessageA;

		int VoiceIdLegnth;
		std::string VoiceFileName;
		std::string VoiceFileDir;
		std::string VoiceTablesDir;
	};
}

#endif // !__ZA_GAME_DATA_H__
