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


		bool StartVoiceTables(Data::VoicePlayerOut& vpOut, const Data::VoicePlayerIn& vpIn);
		bool EndVoiceTables();

		bool SetVoicePlayConfig(const Data::PlayConfigIn& playConfigIn);

		bool MessageReceived(Data::MessageOut& msgOut, Data::MessageIn& msgIn);
	}
}

#endif //__ZA_H__

