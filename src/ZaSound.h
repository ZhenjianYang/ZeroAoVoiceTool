#ifndef __ZASOUND_H__
#define __ZASOUND_H__

namespace Za {
	class Sound {

	public:
		enum class Status
		{
			Playing = 0,
			Stop = 1
		};
		using StopCallBackType = int(*)(void*);

		static const int VolumeMax;

		static int Init(int volume = VolumeMax);
		static int End();

		static void SetVolume(int volume);
		static int GetVolume();

		static Status GetStatus();

		static bool Play(const char* soundFile);
		static void Stop();

		static void SetStopCallBack(StopCallBackType stopCallBack = nullptr, void* param = nullptr);

	private:
		virtual ~Sound() = 0;
	};
}

#endif // !__ZASOUND_H__
