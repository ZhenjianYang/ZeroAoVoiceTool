#include "ZaSound.h"

#include <audiere.h>

static float _volume = 1.0f;
static audiere::AudioDevicePtr _device = nullptr;
audiere::OutputStreamPtr _soundStream = nullptr;
static StopCallBack _callBack = nullptr;
static void* _callBackParam = nullptr;

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

int ZaSoundInit(float volume)
{
	ZaSoundSetStopCallBack(nullptr);
	ZaSoundStop();

	ZaSoundSetVolumn(volume);
	
	if (!_device.get()) {
		_device = audiere::OpenDevice();
	}

	if (!_device.get())
		return 1;

	return 0;
}

int ZaSoundEnd()
{
	ZaSoundSetStopCallBack(nullptr);
	ZaSoundStop();
	_soundStream = nullptr;
	_device = nullptr;

	return 0;
}

void ZaSoundSetVolumn(float volumn)
{
	_volume = volumn;
}

float ZaSoundGetVolumn()
{
	return _volume;
}

int ZaSoundStatus()
{
	if (_soundStream && _soundStream->isPlaying())
		return ZASOUND_STATUS_PLAYING;
	else
		return ZASOUND_STATUS_STOP;
}

bool ZaSoundPlay(const char* soundFile)
{
	ZaSoundStop();
	_soundStream = audiere::OpenSound(_device, soundFile, true);
	if (!_soundStream) return false;

	_soundStream->setVolume(_volume);
	_soundStream->play();

	return true;
}

void ZaSoundStop()
{
	if(_soundStream)
		_soundStream->stop();
}

void ZaSoundSetStopCallBack(StopCallBack stopCallBack /*= nullptr*/, void* _param /*= nullptr*/)
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


