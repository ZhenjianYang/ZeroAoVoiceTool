#include "Za.h"
#include "ZaConst.h"
#include "ZaErrorMsg.h"
#include "ZaSound.h"
#include "ZaRemote.h"
#include "ZaVoicePlayer.h"
#include "ZaScenaAnalyzer.h"

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

bool Za::Main::StartVoiceTables(Data::VoicePlayerOut & vpOut, const Data::VoicePlayerIn & vpIn)
{
	return Za::ScenaAnalyzer::Init(vpOut, vpIn);
}

bool Za::Main::EndVoiceTables()
{
	return Za::ScenaAnalyzer::End();
}

bool Za::Main::SetVoicePlayConfig(const Data::PlayConfigIn & playConfigIn)
{
	return Za::Remote::DisableOriVoice(playConfigIn.disableOriVoice)
		&& Za::Sound::SetVolume(playConfigIn.volume);
}

bool Za::Main::MessageReceived(Data::MessageOut & msgOut, Data::MessageIn & msgIn)
{
	return Za::ScenaAnalyzer::MessageReceived(msgOut, msgIn);
}

