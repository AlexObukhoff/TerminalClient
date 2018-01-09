/* @file Список шагов платежа. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
/// Возможные статусы платежа.
namespace EPaymentStatus
{
	enum Enum
	{
		DispensedChange = -4, /// Сдача, выданная вручную, по заявлению клиента
		LostChange = -3,   /// Неизрасходованная сдача
		Cheated = -2,      /// Попытка мошенничества с БД терминала
		Deleted = -1,      /// Платёж удалён
		Init = 0,          /// Платёж на этапе формирования
		ReadyForCheck = 3, /// Готов к проведению
		ProcessError = 5,  /// Предыдущая попытка проведения завершилась неудачей
		Completed = 6,     /// Завершён успешно
		Canceled = 7,      /// Отменён
		BadPayment = 8     /// Критическая ошибка
	};
};

//------------------------------------------------------------------------------
/// Список шагов платежа.
class EPaymentStep : public QObject
{
	Q_OBJECT
	Q_ENUMS(Enum)

public:
	enum Enum
	{
		DataCheck = 0, /// Проверка данных.
		Pay,           /// Создание транзакции платежа.
		Status,        /// Запрос статуса платежа.
		GetStep,       /// Запрос получения полей для следующего шага (multistage)
		User = 255     /// Произвольный запрос для нестандартного платежа.
	};
};

//------------------------------------------------------------------------------
/// Список типов мошенничества.
namespace EPaymentCheatedType
{
	enum Enum
	{
		CashAcceptor = 1, /// Манипуляции с купюрником.
		NotesCount,       /// Подозрительное кол-во купюр.
	};
};


//------------------------------------------------------------------------------
} // PaymentProcessor
} // SDK

