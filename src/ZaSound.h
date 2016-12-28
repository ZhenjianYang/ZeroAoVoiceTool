#ifndef __ZASOUND_H__
#define __ZASOUND_H__

namespace Za {
	namespace Sound {
		enum class Status
		{
			Playing = 0,
			Stop = 1
		};
		using StopCallBackType = int(*)(void*);

		extern const int VolumeMax;

		bool Init(int volume = VolumeMax);
		bool End();

		bool SetVolume(int volume);
		int GetVolume();

		Status GetStatus();

		bool Play(const char* soundFile);
		bool Stop();

		void SetStopCallBack(StopCallBackType stopCallBack = nullptr, void* param = nullptr);
	}
}

#endif // !__ZASOUND_H__
