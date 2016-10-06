#ifndef __ZAVOICEPLAYER_H__
#define __ZAVOICEPLAYER_H__

namespace Za {

	class VoicePlayer {
	public:
		//��������ѭ����ʼ��
		static int Init(void* data = nullptr);

		static int End();

		//����һ����������ѭ��
		static int LoopOne();

		//��out_filename�ǿ�ʱ�����ļ���д��out_filename
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
