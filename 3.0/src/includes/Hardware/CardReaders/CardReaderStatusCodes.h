/* @file Коды состояний кардиридеров. */

#pragma once

#include "Hardware/Common/BaseStatus.h"

//--------------------------------------------------------------------------------
namespace CardReaderStatusCode
{
	/// Предупреждения.
	namespace Warning
	{
		const int Fan = 520;    /// Вентилятор.
		const int Forgotten = 521;    /// Карта забыта.
		const int NeedReloading = 523;    /// Карта не была прочитана, извлеките и повторно вставьте исправную карту.
	}

	/// Ошибки при работе со смарт-картой (предупреждение).
	namespace SCOperarionError
	{
		const int Unknown     = 540;    /// Неизвестная.
		const int Sertificate = 541;    /// Сертификат.
		const int Security    = 542;    /// Ошибка безопасности.
		const int Memory      = 543;    /// Память.
	}

	/// Выброс. предупреждение, но если долго висит - ошибка.
	namespace Reject
	{
		const int Unknown  = 560;    /// Причина неизвестна.
		const int Length   = 561;    /// Длина карты.
		const int NextCard = 562;    /// Попытка вставки 2-х карт.
		const int BadChip  = 563;    /// Поврежден чип карты.
	}

	/// Ошибки.
	namespace Error
	{
		const int Shutter = 580;    /// Блокировочное устройство.
		const int Sensors = 581;    /// Датчики местоположения карты.
		const int SAM     = 582;    /// SAM-модуль.
	}
}

//--------------------------------------------------------------------------------
