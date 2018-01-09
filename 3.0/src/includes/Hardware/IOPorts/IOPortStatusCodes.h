/* @file Коды состояний портов общие. */

#pragma once

//--------------------------------------------------------------------------------
namespace IOPortStatusCode
{
	/// Предупреждения.
	namespace Warning
	{
		const int MismatchParameters = 100;    /// Параметры порта установлены неверно.
	}

	/// Ошибки.
	namespace Error
	{
		const int NotSet = 120;    /// Порт не настроен.
		const int Busy   = 121;    /// Порт занят.
		const int NotConfigured = 122;    /// Порт неверно сконфигурирован.
		const int NotConnected  = 123;    /// Порт не подключен.
	}
}

//-------------------------------------------------------------------------------- 
