#include "ZaSound.h"

#include "ZaConst.h"
#include "ZaErrorMsg.h"

#include <audiere.h>

const int Za::Sound::VolumeMax = MAX_VOLUME;

static int _volume = Za::Sound::VolumeMax;
static Za::Sound::StopCallBackType _callBack = nullptr;
static void* _callBackParam = nullptr;;

static audiere::AudioDevicePtr _device = nullptr;
static audiere::OutputStreamPtr _soundStream = nullptr;

class AdrStopCallBack : public audiere::RefImplementation<audiere::StopCallback> {
public:
	void ADR_CALL streamStopped(audiere::StopEvent* event);
};
static audiere::StopCallbackPtr _adrStopCallBack = nullptr;

void ADR_CALL AdrStopCallBack::streamStopped(audiere::StopEvent * event)
{
	if (_callBack) {
		_callBack(_callBackParam);
	}
}

bool Za::Sound::Init(int volume)
{
	if (_device.get()) {
		return true;
	}

	_device = audiere::OpenDevice();
	if (!_device.get()) {
		Za::Error::SetErrMsg("获取音频设备失败！");
		return false;
	}

	_adrStopCallBack = new AdrStopCallBack;
	Za::Sound::SetVolume(volume);
	
	return true;
}

bool Za::Sound::End()
{
	Za::Sound::SetStopCallBack(nullptr);
	Za::Sound::Stop();
	_soundStream = nullptr;
	_device = nullptr;
	_adrStopCallBack = nullptr;

	return true;
}

bool Za::Sound::SetVolume(int volume)
{
	_volume = volume;
	if (_soundStream.get())
		_soundStream->setVolume(float(_volume) / VolumeMax);
	return true;
}

int Za::Sound::GetVolume()
{
	return _volume;
}

Za::Sound::Status Za::Sound::GetStatus()
{
	if (_soundStream && _soundStream->isPlaying())
		return Za::Sound::Status::Playing;
	else
		return Za::Sound::Status::Stop;
}

bool Za::Sound::Play(const char* soundFile)
{
	Za::Sound::Stop();
	_soundStream = audiere::OpenSound(_device, soundFile, true);
	if (!_soundStream) return false;

	_soundStream->setVolume((float)_volume / VolumeMax);
	_soundStream->play();

	return true;
}

bool Za::Sound::Stop()
{
	if(_soundStream)
		_soundStream->stop();
	return true;
}

void Za::Sound::SetStopCallBack(StopCallBackType stopCallBack /*= nullptr*/, void* _param /*= nullptr*/)
{
	if (!_device.get()) return;

	_callBackParam = _param;
	
	if (stopCallBack == nullptr) {
		_callBack = nullptr;
		_device->clearCallbacks();
	}
	else {
		if(_callBack == nullptr)
			_device->registerCallback(_adrStopCallBack.get());
		_callBack = stopCallBack;
	}
}


