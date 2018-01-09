/* @file Константы AT GSM модема. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/DeviceDataConstants.h"

// Project
#include "ATData.h"

//--------------------------------------------------------------------------------
namespace CATGSMModem
{
	/// Таймауты, [мс]
	namespace Timeouts
	{
		const int ResetConnection = 3000;    /// Переустановка соединения
		const int Connection = 60000;   /// Установка связи с GSM сетью
		const int Default    = 200;     /// По умолчанию для команд
		const int CellInfo   = 300;     /// Инфо о статусе регистрации в сети
		const int SMS        = 5000;    /// Забирание SMS
		const int Config     = 300;     /// Получение конфигурации

		namespace SimCom
		{
			const int Config = 1000;    /// Получение конфигурации
			const int USSD   = 5000;    /// USSD-запрос
		}

		namespace Siemens
		{
			const int CellInfo = 1000;    /// Инфо о статусе регистрации в сети
		}
	}

	/// Задержки, [мс]
	namespace Pauses
	{
		const int AnswerAttempt  = 25;      /// Между попытками получения ответа на команду
		const int Answer         = 100;     /// После получения/неполучения ответа на команду
		const int BalanceAttempt = 1000;    /// Между попытками запроса баланса.
		const int BalanceData    = 100;     /// При чтении данных при запросе баланса.
		const int Message        = 3000;    /// Перед приходом уведомления об отправке сообщения.

		namespace ZTE
		{
			const int USSDAttempt = 100;    /// Между попытками USSD-запроса.
		}
	}

	/// Имя по умолчанию.
	const char DefaultName[] = "GSM AT compatible modem";

	/// Число попыток getBalance.
	const int BalanceAttempts = 10;

	/// Параметры запроса данных SIM-карты
	struct SSIMRequestInfo
	{
		QString name;
		QString regexpData;
		bool swapCharPair;

		SSIMRequestInfo(): swapCharPair(false) {}
		SSIMRequestInfo(const QString & aName, const QString & aRegexpData, bool aSwapCharPair): name(aName), regexpData(aRegexpData), swapCharPair(aSwapCharPair) {}
	};

	class CSIMRequestInfo: public CSpecification<QByteArray, SSIMRequestInfo>
	{
	public:
		CSIMRequestInfo()
		{
			add(AT::Commands::IMEI, CDeviceData::Modems::IMEI);
			add(AT::Commands::IMSI, CDeviceData::Modems::IMSI);
			add(AT::Commands::CNUM, CDeviceData::Modems::SIMNumber);
			add(AT::Commands::Siemens::SIMID, CDeviceData::Modems::SIMId);
			add(AT::Commands::Huawei::SIMID,  CDeviceData::Modems::SIMId, "\\d+", true);
			add(AT::Commands::ZTE::SIMID,     CDeviceData::Modems::SIMId, "\\w{6,24}", true);
		}

		QStringList getDeviceDataKeys()
		{
			QStringList result;

			foreach (const SSIMRequestInfo & aInfo, mBuffer)
			{
				result << aInfo.name;
			}

			result << CDeviceData::Revision;
			result.removeDuplicates();
			result.sort();

			return result;
		}

	private:
		void add(const QByteArray & aCommand, const QString & aName, const QString & aRegexpData = "\\d+", bool aSwapCharPair = false)
		{
			append(aCommand, SSIMRequestInfo(aName, aRegexpData, aSwapCharPair));
		}
	};

	static CSIMRequestInfo SIMRequestInfo;
}

//--------------------------------------------------------------------------------
