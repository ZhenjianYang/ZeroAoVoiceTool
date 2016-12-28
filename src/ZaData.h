#ifndef __ZADATA_H__
#define __ZADATA_H__

namespace Za
{
	class Data
	{
	public:
		struct GameOut
		{
			const char* Title;
			const char* Comment;
		};

		struct ThreadIn
		{
			int hMainWindow;
			int msgId;
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
	private:
		virtual ~Data() = 0;
	};
}

#endif //__ZA_H__

