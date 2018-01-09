/* @file Абстрактный запрос к серверу. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {
namespace CyberPlat {

//---------------------------------------------------------------------------
class Request
{
	Q_DISABLE_COPY(Request)

public:
	Request();
	virtual ~Request();

	/// Добавление параметра в запрос (в inputmessage)
	void addParameter(const QString & aName, const QVariant & aValue, const QVariant & aLogValue = QVariant());

	/// Добавление параметра в запрос (рядом с inputmessage)
	void addRawParameter(const QString & aName, const QVariant & aValue, const QVariant & aLogValue = QVariant());

	/// Удаление параметра из запроса.
	void removeParameter(const QString & aName, bool aRawParameter = false);

	/// Получение значение параметра из запроса. Если параметр отсутсвует возвращается пустой QVariant.
	virtual QVariant getParameter(const QString & aName, bool aRawParameter = false) const;

	/// Очищает все данные запроса.
	virtual void clear();

	/// Возвращает полный список параметров запроса.
	virtual const QVariantMap & getParameters(bool aRawParameter = false) const;

	/// Возвращает true, если запрос был софрмирован успешно.
	virtual bool isOk() const;

	/// Если isOk вернул false, то с помощью метода isCriticalError можно узнать,
	/// временная ли это ошибка или попытки сгенерировать запрос можно прекратить.
	virtual bool isCriticalError() const;

	/// Возвращает содержание запроса в формате принимающей стороны.
	virtual QString toString() const;

	/// Возвращает строку с полями запроса для логирования.
	virtual QString toLogString() const;

protected:
	bool mIsOk;
	bool mIsCriticalError;

	// Параметры, добавляемые методом addParam
	QVariantMap mParameters;
	QVariantMap mLogParameters;
	QVariantMap mRawParameters;
	QVariantMap mRawLogParameters;
};

//------------------------------------------------------------------------------
}}} // SDK::PaymentProcessor::CyberPlat
