#ifndef __ZADATA_H__
#define __ZADATA_H__

namespace Za
{
	namespace Data
	{
		struct GameProcessIn
		{
			int hMainWindow;
			int msgId;
		};
		struct GameProcessOut
		{
			const char* Title;
			const char* Comment;

			unsigned RemoteDataAddr;
			unsigned RemoteDataSize;
		};

		struct VoiceTableIn
		{
			using CallBackType = int(*) (void*);
			CallBackType callBack = nullptr;
		};
		struct VoiceTableOut
		{
			int Count;
			bool Finished;
		};

		struct MessageIn
		{
			int wparam;
			int lparam;
		};
		struct MessageOut
		{
			int Type;
			const char* VoiceFileName;
			const char* CnText;
			const char* JpText;
		};

		struct PlayConfigIn
		{
			int volume;
			bool disableOriVoice;
		};
	};
}

#endif //__ZA_H__

