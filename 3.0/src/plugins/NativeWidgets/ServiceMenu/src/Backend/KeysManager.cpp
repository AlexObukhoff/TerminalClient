/* @file Менеджер для работы с сключами */

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>

// Modules
#include <Crypt/ICryptEngine.h>

// Project
#include "GUI/ServiceTags.h"
#include "KeysManager.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------
KeysManager::KeysManager(SDK::PaymentProcessor::ICore * aCore)
	: mCore(aCore),
	  mIsGenerated(false)
{
	mCryptService = mCore->getCryptService();
	mTerminalSettings = static_cast<PPSDK::TerminalSettings *>(mCore->getSettingsService()->
		getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
}

//------------------------------------------------------------------------
KeysManager::~KeysManager()
{
}

//------------------------------------------------------------------------
bool KeysManager::generateKey(QVariantMap & aKeysParam)
{
	QString login(aKeysParam[CServiceTags::Login].toString());
	QString password(aKeysParam[CServiceTags::Password].toString());
	QString url(mTerminalSettings->getKeygenURL());

	if (url.isEmpty())
	{
		aKeysParam[CServiceTags::Error] = "#url_is_empty";
		mIsGenerated = false;
	}

	EKeysUtilsError::Enum result = static_cast<EKeysUtilsError::Enum>(mCryptService->generateKey(0, login, password, url, mSD, mAP, mOP));

	aKeysParam[CServiceTags::Error] = errorToString(result);

	mIsGenerated = result ==  EKeysUtilsError::Ok;

	return mIsGenerated;
}

//------------------------------------------------------------------------
bool KeysManager::formatToken()
{
	auto status = tokenStatus();

	if (status.available)
	{
		if (!status.initialized)
		{ 
			return mCryptService->getCryptEngine()->initializeToken(CCrypt::ETypeEngine::RuToken);
		}

		return true;
	}

	return false;
}

//------------------------------------------------------------------------
CCrypt::TokenStatus KeysManager::tokenStatus() const
{
	return mCryptService->getCryptEngine()->getTokenStatus(CCrypt::ETypeEngine::RuToken);
}

//------------------------------------------------------------------------
bool KeysManager::saveKey()
{
	return mCryptService->saveKey();
}

//------------------------------------------------------------------------
bool KeysManager::isConfigurationChanged() const
{
	return mIsGenerated;
}

//------------------------------------------------------------------------
void KeysManager::resetConfiguration()
{
	mIsGenerated = false;
}

//------------------------------------------------------------------------
QString KeysManager::getSD() const
{
	return mSD;
}

//------------------------------------------------------------------------
QString KeysManager::getAP() const
{
	return mAP;
}

//------------------------------------------------------------------------
QString KeysManager::getOP() const
{
	return mOP;
}

//------------------------------------------------------------------------
QString KeysManager::errorToString(EKeysUtilsError::Enum aCode) const
{
	switch (aCode)
	{
		case EKeysUtilsError::Ok: return tr("#ok");
		case EKeysUtilsError::NetworkError: return tr("#network_error");
		case EKeysUtilsError::WrongPassword: return tr("#wrong_login_or_password");
		case EKeysUtilsError::WrongServerAnswer: return tr("#wrong_server_answer");
		case EKeysUtilsError::WrongLocalTime: return tr("#wrong_local_time");
		case EKeysUtilsError::UnknownServerError: return tr("#unknown_server_error");
		case EKeysUtilsError::KeyPairCreateError: return tr("#key_pair_create_error");
		case EKeysUtilsError::KeyExportError: return tr("#key_export_error");
	}

	return tr("#unknown_error");
}

//------------------------------------------------------------------------
