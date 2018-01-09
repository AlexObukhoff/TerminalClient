/* @file Коды завершения приложения */

#pragma once

namespace ExitCode
{
	enum Enum
	{
		NoError = 0,         /// Выход без ошибок.
		NoRequiredParameter, /// Отсутствуют необходимые параметры (напр. командной строки).
		NoRequiredFile,      /// Отсутствуют необходимые файлы.
		InvalidParameter,    /// Некорректный параметр (напр. командной строки).
		Error                /// Какая-то ошибка. TODO: extend.
	};
}
