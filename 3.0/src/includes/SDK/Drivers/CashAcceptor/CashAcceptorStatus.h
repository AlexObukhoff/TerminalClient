/* @file Обобщенные статусы устройств приема денег. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/DeviceStatus.h>

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
/// Обобщенные статусы устройств приема денег. Передаются в пп и служат для внутренних нужд драйвера. Порядок не менять.
namespace ECashAcceptorStatus
{
	enum Enum
	{
		/// OK.
		OK = 1,             /// Хороший статус.
		Escrow,             /// Принял деньгу, и ждет дальнейшего решения: stack, return или hold. Выдает инфо о купюре.
		Stacked,            /// Уложил деньгу. Может выдать инфо о купюре.

		/// Ворнинги.
		Warning,            /// Находится в подозрительном состоянии.

		/// Ошибки.
		Error,              /// Ошибка, не связанная с механикой.
		MechanicFailure,    /// Ошибка в механике устройства.
		StackerFull,        /// Стекер полон.
		StackerOpen,        /// Стекер снят.

		/// Нужны для логики драйвера и, возможно, для учета статистики.
		Cheated,            /// Попытка мошеничества - ворнинг.
		Rejected = 101,     /// Выброс - OK.

		/// Нужны для внутреннего пользования в драйверах. Наверх не выдаются.
		Inhibit,            /// Выключен на прием денег.
		Disabled,           /// Отключен на прием денег.
		Enabled,            /// Включен на прием денег.
		BillOperation,      /// Занят, работает с деньгой.
		Busy,               /// Занят чем-то непонятным.
		OperationError,     /// Ошибка.
		Unknown             /// Дефолтный статус.
	};
}

}} // namespace SDK::Driver

Q_DECLARE_METATYPE(SDK::Driver::ECashAcceptorStatus::Enum);

//--------------------------------------------------------------------------------
