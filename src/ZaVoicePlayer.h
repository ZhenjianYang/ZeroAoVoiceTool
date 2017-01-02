#ifndef __ZAVOICEPLAYER_H__
#define __ZAVOICEPLAYER_H__

namespace Za {
	namespace VoicePlayer {
		//��out_filename�ǿ�ʱ�����ļ���д��out_filename
		bool PlayVoice(int voiceId, char *out_filename = nullptr);

		bool Init();
		bool End();

		void AddToWait(int voiceId);
		void ClearWait();
		int GetWaitingNum();
		bool PlayWait(char *out_filename = nullptr);
	};
}

#endif // !__ZAVOICEPLAYER_H__
