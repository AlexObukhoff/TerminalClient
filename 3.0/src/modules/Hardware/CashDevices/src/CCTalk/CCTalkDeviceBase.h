/* @file Базовый класс устройства на протоколе ccTalk. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Protocols/CashAcceptor/CCTalk.h"

// Project
#include "CCTalkModelData.h"
#include "Hardware/CashDevices/CCTalkDeviceConstants.h"

//--------------------------------------------------------------------------------
template <class T>
class CCTalkDeviceBase : public T
{
	SET_SERIES("ccTalk")

public:
	CCTalkDeviceBase();

	/// Получить поддерживаемые тпы протоколов.
	static QStringList getProtocolTypes();

protected:
	/// Запросить и сохранить параметры устройства.
	virtual void processDeviceData();

	/// Попытка самоидентификации.
	virtual bool isConnected();

	/// Попытка самоидентификации.
	bool checkConnection();

	/// Распарсить данные прошивки.
	virtual double parseFWVersion(const QByteArray & aAnswer);

	/// Выполнить команду.
	virtual TResult execCommand(const QByteArray & aCommand, const QByteArray & aCommandData, QByteArray * aAnswer = nullptr);

	/// Распарсить дату.
	QDate parseDate(const QByteArray & aData);

	/// Данные всех моделей.
	typedef QSharedPointer<CCCTalk::CModelDataBase> PAllModelData;
	PAllModelData mAllModelData;

	/// Базовый год (для парсинга дат).
	int mBaseYear;

	/// Протокол.
	CCTalkCAProtocol mProtocol;

	/// Индекс события.
	int mEventIndex;

	// TODO: в базу.
	/// Последние девайс-коды устройства.
	TDeviceCodes mCodes;

	/// Номер прошивки.
	double mFWVersion;

	/// Модели данной реализации.
	QStringList mModels;

	/// Данные модели.
	CCCTalk::SModelData mModelData;

	/// Адрес устройства.
	uchar mAddress;

	/// Поддерживаемые тпы протоколов.
	QStringList mProtocolTypes;

	/// Данные ошибок.
	typedef QSharedPointer<CCCTalk::ErrorDataBase> PErrorData;
	PErrorData mErrorData;
};

//--------------------------------------------------------------------------------
