#include "ZaSound.h"

#include "ZaConst.h"
#include <audiere.h>

const int Za::Sound::VolumeMax = VOLUME_MAX;

static int _volume;
static Za::Sound::StopCallBackType _callBack;
static void* _callBackParam;

static audiere::AudioDevicePtr _device = nullptr;
static audiere::OutputStreamPtr _soundStream = nullptr;

class AdrStopCallBack : public audiere::RefImplementation<audiere::StopCallback> {
public:
	void ADR_CALL streamStopped(audiere::StopEvent* event);
};
static audiere::StopCallbackPtr _adrStopCallBack = new AdrStopCallBack;

void ADR_CALL AdrStopCallBack::streamStopped(audiere::StopEvent * event)
{
	if (_callBack) {
		_callBack(_callBackParam);
	}
}

int Za::Sound::Init(int volume)
{
	Za::Sound::SetStopCallBack(nullptr);
	Za::Sound::Stop();

	Za::Sound::SetVolume(volume);
	
	if (!_device.get()) {
		_device = audiere::OpenDevice();
	}

	if (!_device.get())
		return 1;

	return 0;
}

int Za::Sound::End()
{
	Za::Sound::SetStopCallBack(nullptr);
	Za::Sound::Stop();
	_soundStream = nullptr;
	_device = nullptr;

	return 0;
}

void Za::Sound::SetVolume(int volume)
{
	_volume = volume;
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

void Za::Sound::Stop()
{
	if(_soundStream)
		_soundStream->stop();
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


