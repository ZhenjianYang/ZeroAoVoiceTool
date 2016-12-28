#ifndef __ZA_H__
#define __ZA_H__

#include "ZaData.h"

namespace Za {
	namespace Main {
		bool Init();
		bool End();

		bool CheckGameEnd();
		bool OpenGameProcess(Data::GameProcessOut& gpOut, const Data::GameProcessIn& gpIn);
		bool CloseGameProcess();

		bool LoadVoiceTables(Data::VoiceTableOut& vtblOut);
		bool LoadVoiceTablesAsyn(Data::VoiceTableOut& vtblOut, const Data::VoiceTableIn& vtblIn);
		bool LoadVoiceTablesAsynCancle(Data::VoiceTableOut& vtblOut, const Data::VoiceTableIn& vtblIn);

		bool SetVoicePlayConfig(const Data::PlayConfigIn& playConfigIn);

		bool MessageRecived(Data::MessageOut& msgOut, Data::MessageIn& msgIn);
	}
}

#endif //__ZA_H__

