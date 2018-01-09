/* @file Загрузчик списков номиналов для пиновых провайдеров. */
#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QMutexLocker>
#include <QtCore/QMap>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Settings/Provider.h>
#include <SDK/PaymentProcessor/CyberPlat/RequestSender.h>
#include <Common/ILogable.h>

// Project
#include "PinCard.h"
#include "PaymentFactoryBase.h"

namespace PPSDK = SDK::PaymentProcessor;
using namespace PPSDK::CyberPlat;

//------------------------------------------------------------------------------
class PinLoader : public QObject, public ILogable
{
	Q_OBJECT

public:
	PinLoader(PaymentFactoryBase * aPaymentFactoryBase);
	~PinLoader(void);

	/// Возвращает уточнённое описание для провайдера, который должен проводиться с помощью этой фабрики платежей.
	/// Если фабрика не может уточнить описание (или оператор сейчас использовать нельзя), то она возвращает невалидную структуру.
	/// Пример: пиновая фабрика постепенно загружает списки номиналов карт и обновляет их в настройках провайдера
	/// при вызове этого метода.
	PPSDK::SProvider getProviderSpecification(const PPSDK::SProvider & aProvider);

	/// Возвращает список номиналов для пинового провайдера aProvider.
	QList<SPinCard> getPinCardList(qint64 aProvider);

private slots:
	/// Нить, обновляющая списки пинов начала работу.
	void onPinThreadStarted();

	/// Загрузка списка номиналов пинов.
	void onLoadPinList();

private:
	/// Загрузить из конфигурации всех пиновых провайдеров
	void findPinProviders();

	/// Создаёт класс ответа по классу запроса.
	Response * createResponse(const Request & aRequest, const QString & aResponseString);

	/// Обновляет кэш списка пинов
	void updatePinList(qint64 aProvider, const QList<SPinCard> & aCards);

private:
	PaymentFactoryBase * mPaymentFactoryBase;

	bool mIsStopping;
	QTimer mPinTimer;
	QThread mPinThread;
	QMutex mPinMutex;

	/// Список загруженных пинов для оператора
	struct SProviderPins
	{
		PPSDK::SProvider provider;
		QList<SPinCard> pins;
		QDateTime lastLoad;

		SProviderPins() 
		{
			init();
		}

		SProviderPins(const PPSDK::SProvider & aProvider) : provider(aProvider)
		{
			init();
		}

		SProviderPins(const PPSDK::SProvider & aProvider, const QList<SPinCard> & aPins) : provider(aProvider), pins(aPins)
		{ 
			init();
		}

		void init()
		{
			// Для Pin провайдеров всегда должен быть онлайн и включена проверка номера
			provider.processor.payOnline = true;
			provider.processor.skipCheck = false;
		}
	};

	/// Список пиновых операторов. Ключ - идентификатор оператора
	QMap<qint64, SProviderPins> mPinProviders;
};

//------------------------------------------------------------------------------
