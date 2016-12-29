#ifndef __ZASCENAANALYZER_H__
#define __ZASCENAANALYZER_H__

#include "ZaData.h"

namespace Za {
	namespace ScenaAnalyzer {
		bool Init(Data::VoicePlayerOut& vpOut, const Data::VoicePlayerIn& vpIn);
		bool End();
		bool MessageRecived(Data::MessageOut & msgOut, Data::MessageIn & msgIn);

		bool DLoadScena(unsigned raScena, const char* &out_scenaName);
		bool DLoadScena1(unsigned raScena1, const char* &out_scenaName);
		bool DLoadBlock(unsigned raBlock, const char* &out_scenaName);
		bool DShowText(unsigned raText,
			int & out_voiceID, const char* &cnText, const char* &jpText, bool & out_wait);
	}
}

#endif
