#ifndef __ZA_H__
#define __ZA_H__

#include "ZaData.h"

namespace Za
{
	class Main
	{
	public:
		static bool Init();
		static bool End();

		static bool CheckGameStart(Data::GameOut& gameOut);
		static bool CheckGameEnd();

		static bool OpenGameThread(const Data::ThreadIn& threadIn);
		static bool CloseGameThread();

		static bool LoadVoiceTables(Data::VoiceTableOut& vtblOut);
		static bool LoadVoiceTablesAsyn(Data::VoiceTableOut& vtblOut, const Data::VoiceTableIn& vtblIn);
		static bool LoadVoiceTablesAsynCancle(Data::VoiceTableOut& vtblOut, const Data::VoiceTableIn& vtblIn);

		static bool SetVoicePlayConfig(const Data::PlayConfigIn& playConfigIn);

		static bool MessageRecived(Data::MessageOut& msgOut, Data::MessageIn& msgIn);

		static const char* LastErr();
	private:
		virtual ~Main() = 0;
	};
}

#endif //__ZA_H__

