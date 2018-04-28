/* @file Константы протокола NPSTalk. */

#pragma once

/// Константы, команды и коды состояний устройств на протоколе NPSTalk.
namespace CNPSTalk
{
	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// После сброса валидатора.
		//const int ResetValidator = 3000;  // если будем делать валидаторы на NPSTalk - раскомментить.

		/// Дефолтный для ожидания ответа.
		const int Default = 500;

		/// Период переинициализации.
		const int ReInitialize = 5 * 1000;
	}

	/// Команды.
	namespace Command
	{
		const char TestConnection     = '\xA1';
		const char Reset              = '\xA2';
		const char GetModelVersion    = '\xA3';
		const char GetFirmwareVersion = '\xA4';
		const char GetNominalChannels = '\xA5';
		const char GetNominals        = '\xA6';
		const char Enable             = '\xA7';
		const char GetAcceptedCoins   = '\xA8';
		const char Disable            = '\xA9';
		const char GetStatus          = '\xAA';
	}
}

//--------------------------------------------------------------------------------
