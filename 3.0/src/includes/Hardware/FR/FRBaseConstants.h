/* @file Базовый фискальный регистратор. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <Common/QtHeadersEnd.h>

// STL
#include <numeric>

// SDK
#include <SDK/Drivers/FR/FiscalFields.h>
#include <SDK/Drivers/FR/FiscalDataTypes.h>

// Modules
#include "Hardware/Common/ASCII.h"

// Project
#include "Hardware/FR/FRStatusesDescriptions.h"

//--------------------------------------------------------------------------------
/// Режим работы ФР
namespace EFRMode
{
	enum Enum
	{
		Fiscal,
		Printer
	};
}

//--------------------------------------------------------------------------------
/// Регион (для налоговых ставок).
namespace ERegion
{
	enum Enum
	{
		RF,
		KZ
	};
}

//--------------------------------------------------------------------------------
/// Типы ФР.
namespace EFRType
{
	enum Enum
	{
		EKLZ,       // Старый ФР с ЭКЛЗ
		NoEKLZ,     // Старый ФР без ЭКЛЗ (для ЕНВД)
		FS          // Онлайновый ФР с ФН
	};
}

//--------------------------------------------------------------------------------
/// Форматы ФФД.
namespace EFFD
{
	enum Enum
	{
		Unknown = -1,    // Неизвестная
		F10Beta,    // 1.0 Beta
		F10,        // 1.0
		F105,       // 1.05
		F11         // 1.1
	};
}

//--------------------------------------------------------------------------------
/// Общие константы ФР.
namespace CFR
{
	/// Актуальный ФФД.
	const EFFD::Enum ActualFFD = EFFD::F10;

	/// Формат представления даты.
	const char DateFormat[] = "ddMMyyyy";

	/// Формат представления даты-времени для парсинга даты bp TLV-параметра фискального тега.
	const char TLVDateTimeFormat[] = "yyyyMMddhhmm";

	/// Формат представления даты для вывода в лог.
	const char DateLogFormat[] = "dd.MM.yyyy";

	/// Формат представления даты для вывода в лог.
	const char TimeLogFormat[] = "hh:mm:ss";

	/// Формат представления даты для вывода в лог даты и времени.
	const char DateTimeLogFormat[] = "dd.MM.yyyy hh:mm:ss";

	/// Формат представления даты для вывода в лог даты и времени.
	const char DateTimeShortLogFormat[] = "dd.MM.yyyy hh:mm";

	/// Количество секунд в сутках.
	const int SecsInDay = 24 * 60 * 60;

	/// Количество миллисекунд в сутках.
	const int MSecsInDay = SecsInDay * 1000;

	/// Константные данные ФФД.
	struct SFFDData
	{
		int maxUnitNameSize;
		QString description;

		SFFDData() : maxUnitNameSize(0), description("unknown") {}
		SFFDData(int aMaxUnitNameSize, const QString & aDescription) : maxUnitNameSize(aMaxUnitNameSize), description(aDescription) {}
	};

	class CFFD : public CSpecification<EFFD::Enum, SFFDData>
	{
	public:
		CFFD()
		{
			add(EFFD::F10Beta,  64, "1.0 Beta");
			add(EFFD::F10,     100, "1.0");
			add(EFFD::F105,    128, "1.05");
			add(EFFD::F11,     128, "1.1");
		}

	private:
		void add(EFFD::Enum aFFD, int aMaxUnitNameSize, const QString & aDescription)
		{
			append(aFFD, SFFDData(aMaxUnitNameSize, aDescription));
		}
	};

	static CFFD FFD;

	/// Фискальные ошибки при выполнении X-отчета.
	const TStatusCodes XReportFiscalErrors = getFiscalStatusCodes(SDK::Driver::EWarningLevel::Error)
		<< DeviceStatusCode::Error::Unknown
		<< FRStatusCode::Error::EKLZ
		<< FRStatusCode::Error::FiscalMemory;

	/// Актуальные ставки НДС в России.
	const SDK::Driver::TVATs RFActualVATs = SDK::Driver::TVATs() << 18 << 10 << 0;

	/// Актуальные ставки НДС в Казахстане.
	const SDK::Driver::TVATs KZActualVATs = SDK::Driver::TVATs() << 12 << 0;

	/// Таймаут соединения с ОФД, [с].
	const int OFDConnectionTimeout = 3 * 60;

	/// Преобразование байт-массива данных в формат ФФД
	inline QString dataToString(const QByteArray & aData, int aBase, int aSize)
	{
		int index = -1;
		while ((aData.size() > ++index) && !aData[index]) {}
		int lastIndex = aData.indexOf(ASCII::NUL, index);
		QString data = QString(aData.mid(index, lastIndex - index)).simplified();

		if (aBase != 10)
		{
			bool OK;
			qulonglong result = data.toULongLong(&OK, aBase);

			return OK ? QString("%1").arg(result, aSize, 10, QChar(ASCII::Zero)) : "";
		}
		else if (QRegExp("^[0-9]+$").indexIn(data) == -1)
		{
			return "";
		}
		else if (data.size() <= aSize)
		{
			return data.rightJustified(aSize, QChar(ASCII::Zero));
		}

		index = data.indexOf(QRegExp("[1-9]"));

		return data.mid(index);
	}

	inline QString   serialToString(const QByteArray & aData, int aBase = 10) { return dataToString(aData, aBase, 16); }    /// Серийный номер ККТ (1013)
	inline QString FSSerialToString(const QByteArray & aData, int aBase = 10) { return dataToString(aData, aBase, 16); }    /// Серийный номер ФН (1041)
	inline QString      RNMToString(const QByteArray & aData, int aBase = 10) { return dataToString(aData, aBase, 16); }    /// РНМ (1037)
	inline QString      INNToString(const QByteArray & aData, int aBase = 10) { return dataToString(aData, aBase, 10); }    /// ИНН оператора перевода (1016)

	/// Размеры номеров
	const int FDSerialNumberSize =  4;    /// Чек в смене (1042)
	const int SessionNumberSize  =  4;    /// Смена (1038)
	const int FDNumberSize       =  4;    /// ФД (1040)
	const int FDSignSize         = 10;    /// ФП (1077)
	const int AutomaticNumber    = 20;    /// Номер автомата (1036)

	//--------------------------------------------------------------------------------
	/// Минимальный размер TLV-структуры, нет данных.
	const int MinTLVSize = 4;

	/// TLV-структура
	struct STLV
	{
		int field;
		QByteArray data;

		STLV(): field(0) {}
	};

	/// TLV-пакет
	typedef QMap<int, QByteArray> TTLVList;

	//--------------------------------------------------------------------------------
	/// Налоги.
	namespace Taxes
	{
		struct SData
		{
			int group;
			QString description;

			SData() : group(0) {}
			SData(int aGroup, const QString & aDescription) : group(aGroup), description(aDescription) {}
		};

		class Data : public CSpecification<SDK::Driver::TVAT, SData>
		{
		public:
			void add(SDK::Driver::TVAT aVAT, int aGroup, const char * aDescription)
			{
				append(aVAT, SData(aGroup, QString::fromUtf8(aDescription)));
			}
		};
	}

	//--------------------------------------------------------------------------------
	/// Типы оплаты
	struct SPayTypeData
	{
		char value;
		QString description;

		SPayTypeData() : value(0) {}
		SPayTypeData(int aValue, const QString & aDescription) : value(char(aValue)), description(aDescription) {}
	};

	class CPayTypeDescription : public CDescription<SDK::Driver::EPayTypes::Enum>
	{
	public:
		CPayTypeDescription()
		{
			using namespace SDK::Driver::EPayTypes;

			append(Cash,         QString::fromUtf8("НАЛИЧНЫМИ"));
			append(EMoney,       QString::fromUtf8("КАРТОЙ"));
			append(PostPayment,  QString::fromUtf8("АВАНС"));
			append(Credit,       QString::fromUtf8("КРЕДИТ"));
			append(CounterOffer, QString::fromUtf8("ВСТРЕЧНОЕ ПРЕДОСТАВЛЕНИЕ"));
		}
	};

	static CPayTypeDescription PayTypeDescription;

	class PayTypeData : public CSpecification<SDK::Driver::EPayTypes::Enum, SPayTypeData>
	{
	public:
		void add(SDK::Driver::EPayTypes::Enum aPayType, int aValue)
		{
			append(aPayType, SPayTypeData(aValue, PayTypeDescription[aPayType]));
		}
	};

	//--------------------------------------------------------------------------------
	/// Типы систем налогообложения
	class CTaxations : public CBitmapDescription<char>
	{
	public:
		CTaxations()
		{
			using namespace SDK::Driver::ETaxations;

			append(Main,                         "ОСН");
			append(SimplifiedIncome,             "УСН доход");
			append(SimplifiedIncomeMinusExpense, "УСН доход - расход");
			append(SingleImputedIncome,          "ЕНВД");
			append(SingleAgricultural,           "ЕСН");
			append(Patent,                       "Патент");
		}
	};

	static CTaxations Taxations;

	//--------------------------------------------------------------------------------
	/// Признаки платежного агента.
	class CAgentFlags : public CBitmapDescription<char>
	{
	public:
		CAgentFlags()
		{
			using namespace SDK::Driver::EAgentFlags;

			append(BankAgent,       "БАНК. ПЛ. АГЕНТ");
			append(BankSubagent,    "БАНК. ПЛ. СУБАГЕНТ");
			append(PaymentAgent,    "ПЛ. АГЕНТ");
			append(PaymentSubagent, "ПЛ. СУБАГЕНТ");
			append(Attorney,        "ПОВЕРЕННЫЙ");
			append(CommissionAgent, "КОМИССИОНЕР");
			append(Agent,           "АГЕНТ");
		}
	};

	static CAgentFlags AgentFlags;

	//--------------------------------------------------------------------------------
	/// Спецификация флагов ФН.
	class CFSFlagData : public CSpecification<char, int>
	{
	public:
		CFSFlagData()
		{
			append('\x01', FRStatusCode::Error::FSEnd);
			append('\x02', FRStatusCode::Warning::FSNearEnd);
			append('\x04', FRStatusCode::Error::FSMemoryEnd);
		//	append('\x08', FRStatusCode::Warning::OFDNoConnection);    // не работает либо имеет другое значение
			append('\x80', FRStatusCode::Error::FS);
		}
	};

	static CFSFlagData FSFlagData;

	//--------------------------------------------------------------------------------
	/// Признаки расчета.
	class CPayOffTypes : public CDescription<SDK::Driver::EPayOffTypes::Enum>
	{
	public:
		CPayOffTypes()
		{
			using namespace SDK::Driver::EPayOffTypes;

			append(Debit,      "ПРИХОД");
			append(DebitBack,  "ВОЗВРАТ ПРИХОДА");
			append(Credit,     "РАСХОД");
			append(CreditBack, "ВОЗВРАТ РАСХОДА");
		}
	};

	static CPayOffTypes PayOffTypes;

	//--------------------------------------------------------------------------------
	/// Наименование фискального чека.
	const QString CashFDName = QString::fromUtf8("КАССОВЫЙ ЧЕК");

	//--------------------------------------------------------------------------------
	/// Режимы работы.
	namespace OperationModes
	{
		struct SData
		{
			int field;
			QString description;

			SData(): field(0) {}
			SData(int aField, const QString & aDescription): field(aField), description(aDescription) {}
		};

		#define ADD_OPERATION_MODE(aName, aDescription) append(SDK::Driver::EOperationModes::aName, SData(SDK::Driver::FiscalFields::aName##Mode, QString::fromUtf8(aDescription)))

		class CData: public CSpecification<char, SData>
		{
		public:
			CData()
			{
				ADD_OPERATION_MODE(Encryption,     "ШФД");
				ADD_OPERATION_MODE(Autonomous,     "АВТОНОМН. РЕЖИМ");
				ADD_OPERATION_MODE(Automatic,      "АВТОМАТ. РЕЖИМ");
				ADD_OPERATION_MODE(ServiceArea,    "ККТ ДЛЯ УСЛУГ");
				ADD_OPERATION_MODE(FixedReporting, "АС БСО");
				ADD_OPERATION_MODE(Internet,       "ККТ ДЛЯ ИНТЕРНЕТ");
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
