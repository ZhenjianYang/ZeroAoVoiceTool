#include "Za.h"
#include "ZaSound.h"
#include "ZaRemote.h"
#include "ZaConst.h"
#include "ZaErrorMsg.h"

using namespace Za;

bool Za::Main::Init()
{
	bool ret = Za::Remote::Init() && Za::Sound::Init();
	return ret;;
}

bool Za::Main::End()
{
	bool ret = Za::Sound::End() && Za::Remote::End();
	return ret;
}

bool Za::Main::CheckGameEnd()
{
	return Za::Remote::CheckGameEnd();
}

bool Za::Main::OpenGameProcess(Data::GameProcessOut & gtOut, const Data::GameProcessIn & gtIn)
{
	return Za::Remote::OpenGameProcess(gtOut, gtIn);
}

bool Za::Main::CloseGameProcess()
{
	return Za::Remote::CloseGameProcess();
}

bool Za::Main::LoadVoiceTables(Data::VoiceTableOut & vtblOut)
{
	return false;
}

bool Za::Main::LoadVoiceTablesAsyn(Data::VoiceTableOut & vtblOut, const Data::VoiceTableIn & vtblIn)
{
	return false;
}

bool Za::Main::LoadVoiceTablesAsynCancle(Data::VoiceTableOut & vtblOut, const Data::VoiceTableIn & vtblIn)
{
	return false;
}

bool Za::Main::SetVoicePlayConfig(const Data::PlayConfigIn & playConfigIn)
{
	return Za::Remote::DisableOriVoice(playConfigIn.disableOriVoice)
		&& Za::Sound::SetVolume(playConfigIn.volume);
}

bool Za::Main::MessageRecived(Data::MessageOut & msgOut, Data::MessageIn & msgIn)
{
	return false;
}

