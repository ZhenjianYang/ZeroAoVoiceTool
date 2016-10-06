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
		typedef int(*StopCallBackType)(void*);

		static const int VolumnMax;

		static int Init(int volumn = VolumnMax);
		static int End();

		static void SetVolumn(int volumn);
		static int GetVolumn();

		static Status GetStatus();

		static bool Play(const char* soundFile);
		static void Stop();

		static void SetStopCallBack(StopCallBackType stopCallBack = nullptr, void* param = nullptr);

	private:
		virtual ~Sound() = 0;
	};
}

#endif // !__ZASOUND_H__
