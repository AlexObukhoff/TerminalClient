#pragma once

#define WATCHSERVICE_TCP

//---------------------------------------------------------------------------
namespace CWatchService
{
	const QString ServiceName  = "Cyberplat Watch Service";

	const QString Name         = "watch_service";

#ifdef WATCHSERVICE_TCP
	const QString MessageQueue = "12346";
#else
	const QString MessageQueue = "WatchServiceMessageQueue";
#endif // WATCHSERVICE_TCP

	// Рекомендуемый временной интервал для пинга сторожевого сервиса
	const int PingInterval     = 5 * 1000; // msec

	// Время, после которого в случае отсутсвия пингов, сторожевой сервис закроет клиента.
	const int MaxPingInterval  = 60 * 1000; // msec

	namespace Commands
	{
		const QString Ping             = "ping";
		const QString ScreenActivity   = "screen_activity";
		const QString Close            = "close";
		const QString Exit             = "exit";
		const QString StartModule      = "start_module";
		const QString CloseModule      = "close_module";
		const QString Restart          = "restart";
		const QString Reboot           = "reboot";
		const QString Shutdown         = "shutdown";
		const QString ShowSplashScreen = "show_splash_screen";
		const QString HideSplashScreen = "hide_splash_screen";
		const QString SetState         = "set_state";
		const QString ResetState       = "reset_state";
		const QString CloseLogs        = "close_logs";
	}

	namespace Notification
	{
		const QString ModuleClosed     = "module_closed";
	}

	namespace Modules
	{
		const QString WatchService           = "guard";
		const QString PaymentProcessor       = "client";
		const QString Updater                = "updater";
		const QString WatchServiceController = "tray";
	}

	namespace Fields
	{
		const QString Module = "module";
		const QString Target = "target";
		const QString Sender = "sender";
		const QString Type   = "type";
		const QString Params = "params";
	}

	namespace States
	{
		enum Enum
		{
			Blocked = 0,      /// Модуль заблокирован
			ServiceOperation, /// Сервисная операция
			ValidatorFailure, /// Ошибка купюроприёмника
			PrinterFailure,   /// Ошибка принтера
			UpdateInProgress, /// Закачка/установка обновления
			NetworkFailure,   /// Ошибка сети
			CryptFailure      /// Ошибка модуля шифрования
		};
	}
}

//---------------------------------------------------------------------------
