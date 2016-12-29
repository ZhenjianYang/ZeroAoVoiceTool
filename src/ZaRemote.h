#ifndef __ZAREMOTE_H__
#define __ZAREMOTE_H__

#include "ZaData.h"
#include "ZaGameData.h"

namespace Za {
	namespace Remote {
		extern const Za::Data::GameData* const &CurGameData;
		extern const Za::Data::GameProcessIn* const &CurGameProcessIn;

		bool Init();
		bool End();

		bool CheckGameEnd();
		bool OpenGameProcess(Data::GameProcessOut& gpOut, const Data::GameProcessIn& gpIn);
		bool CloseGameProcess();

		bool DisableOriVoice(bool op = true);

		bool RemoteRead(unsigned rAdd, void *buff, unsigned size);
		bool RemoteWrite(unsigned rAdd, const void *buff, unsigned size);
		unsigned RemoteAlloc(unsigned size);
		bool RemoteFree(unsigned rAdd, unsigned size);
	}
}

#endif // !__ZAREMOTE_H__
