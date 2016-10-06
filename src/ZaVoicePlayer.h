#ifndef __ZAVOICEPLAYER_H__
#define __ZAVOICEPLAYER_H__

namespace Za {

	class VoicePlayer {
	public:
		//语音播放循环初始化
		static int Init(void* data = nullptr);

		static int End();

		//进行一次语音播放循环
		static int LoopOne();

		//当out_filename非空时，将文件名写到out_filename
		static bool PlayVoice(int voiceId, char *out_filename = nullptr);

		static void AddToWait(int voiceId);
		static void ClearWait();
		static int GetWaitingNum();
		static int PlayWait();

	private:
		virtual ~VoicePlayer() = 0;
	};
}

#endif // !__ZAVOICEPLAYER_H__
