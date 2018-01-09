/* @file Загрузчик списков номиналов для пиновых провайдеров. */

// boost
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

// Qt 
#include <Common/QtHeadersBegin.h>
#include <QtCore/QScopedPointer>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Project
#include "PinGetCardListRequest.h"
#include "PinGetCardListResponse.h"
#include "PinPayment.h"
#include "PinLoader.h"

//------------------------------------------------------------------------------
namespace CPinLoader
{
	const char ThreadName[] = "PinLoaderThread";
	const char GetCardsRequestName[] = "GETCARDS";

	const int FirstLoadPinTimeout = 2 * 60;   // таймаут первой попытки загрузки пинов (сек)
	const int ErrorRetryTimeout = 5 * 60;    // таймаут повторной попытки загрузки пинов в случае ошибок (сек)
	const int NextLoadTimeout = 12 * 60 * 60; // таймаут повторной попытки загрузки пинов в случае успеха (сек)

	const int PinsExpirationTime = 24 * 60 * 60; // время в секундах, до следующей попытки загрузки успешно загруженного списка пинов
}

namespace CProcessorType
{
	const QString CybeplatPin = "cyberplat_pin";
}

//------------------------------------------------------------------------------
PinLoader::PinLoader(PaymentFactoryBase * aPaymentFactoryBase) : 
	QObject(aPaymentFactoryBase),
	mPaymentFactoryBase(aPaymentFactoryBase),
	mIsStopping(false)
{
	ILogable::setLog(aPaymentFactoryBase->getLog("PinLoader"));

	mPinThread.setObjectName(CPinLoader::ThreadName);

	mPinTimer.moveToThread(&mPinThread);

	connect(&mPinThread, SIGNAL(started()), SLOT(onPinThreadStarted()), Qt::DirectConnection);
	connect(&mPinThread, SIGNAL(finished()), &mPinTimer, SLOT(stop()), Qt::DirectConnection);

	connect(&mPinTimer, SIGNAL(timeout()), SLOT(onLoadPinList()), Qt::DirectConnection);

	mIsStopping = false;
	mPinThread.start();
}

//------------------------------------------------------------------------------
PinLoader::~PinLoader(void)
{
	mIsStopping = true;

	mPinThread.quit();
	if (!mPinThread.wait(3000))
	{
		toLog(LogLevel::Error, "Terminate PinLoader thread.");
		mPinThread.terminate();
	}
}

//------------------------------------------------------------------------------
PPSDK::SProvider PinLoader::getProviderSpecification(const PPSDK::SProvider & aProvider)
{
	QMutexLocker lock(&mPinMutex);

	if (mPinProviders.contains(aProvider.id) && mPinProviders[aProvider.id].lastLoad.isValid())
	{
		// Возвращаем описание с заполненными номиналами. 
		return mPinProviders[aProvider.id].provider;
	}

	return PPSDK::SProvider();
}

//------------------------------------------------------------------------------
void PinLoader::onPinThreadStarted()
{
	mPinTimer.start(CPinLoader::FirstLoadPinTimeout * 1000);
}

//------------------------------------------------------------------------------
Response * PinLoader::createResponse(const SDK::PaymentProcessor::CyberPlat::Request & aRequest, const QString & aResponseString)
{
	return new PinGetCardListResponse(aRequest, aResponseString);
}

//------------------------------------------------------------------------------
void PinLoader::findPinProviders()
{
	PPSDK::ISettingsService * settingsService = mPaymentFactoryBase->getCore()->getSettingsService();
	if (!settingsService)
	{
		toLog(LogLevel::Error, "Failed to get settings service.");
		return;
	}

	if (mPinProviders.isEmpty())
	{
		toLog(LogLevel::Normal, "Updating list...");

		PPSDK::DealerSettings * dealerSettings = dynamic_cast<PPSDK::DealerSettings *>(settingsService->getAdapter(PPSDK::CAdapterNames::DealerAdapter));
		if (!dealerSettings)
		{
			toLog(LogLevel::Error, "Failed to get dealer settings.");
			return;
		}

		foreach (qint64 providerId, dealerSettings->getProviders(CProcessorType::CybeplatPin))
		{
			PPSDK::SProvider provider = dealerSettings->getProvider(providerId);
			
			if (provider.processor.requests.contains(CPinLoader::GetCardsRequestName))
			{
				QMutexLocker lock(&mPinMutex);

				mPinProviders.insert(provider.id, SProviderPins(provider));
			}
		}
	}
}

//------------------------------------------------------------------------------
void PinLoader::onLoadPinList()
{
	if (mPinProviders.isEmpty())
	{
		findPinProviders();
	}

	if (mPinProviders.isEmpty())
	{
		toLog(LogLevel::Normal, "No providers.");

		mPinThread.quit();

		return;
	}

	PPSDK::ISettingsService * settingsService = mPaymentFactoryBase->getCore()->getSettingsService();
	if (!settingsService)
	{
		toLog(LogLevel::Error, "Failed to get settings service.");
		return;
	}

	PPSDK::TerminalSettings * terminalSettings = dynamic_cast<PPSDK::TerminalSettings *>(settingsService->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
	if (!terminalSettings)
	{
		toLog(LogLevel::Error, "Failed to get terminal settings.");
		return;
	}

	SDK::PaymentProcessor::CyberPlat::RequestSender http(mPaymentFactoryBase->getNetworkTaskManager(), mPaymentFactoryBase->getCryptEngine());
	http.setResponseCreator(boost::bind(&PinLoader::createResponse, this, _1, _2));

	int failedCount = 0;

	foreach (qint64 id, mPinProviders.keys())
	{
		if (mIsStopping)
		{
			toLog(LogLevel::Normal, "Updating providers stopped.");
			break;
		}

		if (mPinProviders[id].lastLoad.isValid() && 
			mPinProviders[id].lastLoad.addSecs(CPinLoader::PinsExpirationTime) > QDateTime::currentDateTime())
		{
			// пропускаем т.к. еще не закончилось время действия загруженных пинов
			continue;
		}

		PPSDK::SProvider & provider = mPinProviders[id].provider;

		toLog(LogLevel::Normal, QString("Updating provider %1 (%2).").arg(provider.id).arg(provider.name));

		PinGetCardListRequest request(terminalSettings->getKeys()[provider.processor.keyPair]);

		toLog(LogLevel::Normal, QString("Sending request to url: %1. Fields: %2.")
			.arg(provider.processor.requests[CPinLoader::GetCardsRequestName].url)
			.arg(request.toLogString()));

		SDK::PaymentProcessor::CyberPlat::RequestSender::ESendError error;

		http.setCryptKeyPair(provider.processor.keyPair);

		QScopedPointer<Response> response(http.post(provider.processor.requests[CPinLoader::GetCardsRequestName].url, request, RequestSender::Solid, error));

		if (!response)
		{
			toLog(LogLevel::Error, QString("Failed to retrieve pin card list. Network error: %1.")
				.arg(SDK::PaymentProcessor::CyberPlat::RequestSender::translateError(error)));
			
			++failedCount;
			
			continue;
		}

		toLog(LogLevel::Normal, QString("Response received. Fields: %1.").arg(response->toLogString()));

		if (!response->isOk())
		{
			toLog(LogLevel::Error, QString("Server response error: %1").arg(response->getError()));

			++failedCount;

			continue;
		}

		updatePinList(id, dynamic_cast<PinGetCardListResponse *>(response.data())->getCards());
	}

	if (failedCount)
	{
		toLog(LogLevel::Error, QString("Pin nominals update failed for %1 in %2 providers. Retry after %3 min.")
			.arg(failedCount).arg(mPinProviders.size()).arg(CPinLoader::ErrorRetryTimeout / 60.));
	}
	else
	{
		toLog(LogLevel::Normal, "All pin nominals successfully updated.");
	}

	// меняем таймаут в зависимости от результата.
	mPinTimer.start((failedCount ? CPinLoader::ErrorRetryTimeout : CPinLoader::NextLoadTimeout) * 1000);
}

//------------------------------------------------------------------------------
void PinLoader::updatePinList(qint64 aProvider, const QList<SPinCard> & aCards)
{
	QMutexLocker lock(&mPinMutex);

	mPinProviders[aProvider].pins = aCards;
	mPinProviders[aProvider].lastLoad = QDateTime::currentDateTime();

	PPSDK::SProvider & provider = mPinProviders[aProvider].provider;

	BOOST_FOREACH (PPSDK::SProviderField & field, provider.fields)
	{
		if (field.id == CPin::UIFieldName)
		{
			field.enumItems.clear();

			foreach (const SPinCard & card, mPinProviders[aProvider].pins)
			{
				PPSDK::SProviderField::SEnumItem item;
				item.title = card.name;
				item.value = card.id;

				field.enumItems << item;
			}
		
			toLog(LogLevel::Normal, QString("Pin card list was successfully updated operator: %1 (%2).").arg(provider.id).arg(provider.name));
		}
	}
}

//------------------------------------------------------------------------------
QList<SPinCard> PinLoader::getPinCardList(qint64 aProvider)
{
	QMutexLocker lock(&mPinMutex);

	if (mPinProviders.contains(aProvider) && mPinProviders[aProvider].lastLoad.isValid())
	{
		// Возвращаем описание с заполненными номиналами. 
		return mPinProviders[aProvider].pins;
	}

	return QList<SPinCard>();
}

//------------------------------------------------------------------------------
