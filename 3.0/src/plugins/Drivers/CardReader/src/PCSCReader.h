/* @file Реализация интерфейса работы с PC/SC API. */
#pragma once

// windows
#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#pragma comment (lib, "Winscard.lib")
#endif

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

// Common
#include <Common/ILogable.h>

// Modules
#include "Hardware/Common/BaseStatusTypes.h"

//--------------------------------------------------------------------------------
class PCSCReader : public QObject, public ILogable
{
public:
	PCSCReader();
	~PCSCReader();

	/// Сброс карты по питанию.
	virtual bool reset(QByteArray & aAnswer);

	/// Получить список кардридеров
	QStringList getReaderList();

	/// Подключиться к ридеру
	bool connect(const QString & aReaderName);

	/// Проверка подключены ли мы к ридеру
	bool isConnected() const;

	/// Обменяться с картой или ридером пакетами APDU
	bool communicate(const QByteArray & aRequest, QByteArray & aResponse);

	/// Отключится от ридера
	void disconnect(bool aEject);

	/// Получить последние статус-коды
	TStatusCodes getStatusCodes();

private:
	/// Обработка результата выполнения функции работы с ридером/картой/SAM-модулем
	bool handleResult(const QString & aFunctionName, HRESULT aResultCode);

	/// Контекст карты
	SCARDCONTEXT mContext;

	/// Хендл карты
	SCARDHANDLE mCard;

	/// Используемый протокол
	unsigned mActiveProtocol;

	/// Заголовок протокольного запроса к ридеру
	SCARD_IO_REQUEST mPioSendPci;

	/// Статус-коды результатов операций
	TStatusCodes mStatusCodes;
};

//--------------------------------------------------------------------------------
