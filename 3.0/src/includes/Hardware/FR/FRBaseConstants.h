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

	/// Способ оплаты по умолчанию для платежей (не интернет-магазинов).
	const SDK::Driver::EPayOffSubjectMethodTypes::Enum PayOffSubjectMethodType = SDK::Driver::EPayOffSubjectMethodTypes::Full;

	/// Размеры ИНН.
	namespace INNSize
	{
		/// Для юридического лица.
		const int LegalPerson = 10;

		/// Для физического лица.
		const int NaturalPerson = 12;
	}

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

	/// Смержить данные (СНО, флаги агента).
	inline char joinData(const QList<char> & aData)
	{
		return std::accumulate(aData.begin(), aData.end(), ASCII::NUL, [] (char aResult, char aLocalData) -> char { return aResult | aLocalData; });
	}

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
	/// Признак способа расчета (1214).
	class CPayOffSubjectMethodTypes : public CDescription<char>
	{
	public:
		CPayOffSubjectMethodTypes()
		{
			using namespace SDK::Driver::EPayOffSubjectMethodTypes;

			append(Prepayment100,  "ПРЕДОПЛАТА 100%");
			append(Prepayment,     "ПРЕДОПЛАТА");
			append(PostPayment,    "АВАНС");
			append(Full,           "ПОЛНЫЙ РАСЧЕТ");
			append(Part,           "ЧАСТИЧНЫЙ РАСЧЕТ И КРЕДИТ");
			append(CreditTransfer, "ПЕРЕДАЧА В КРЕДИТ");
			append(CreditPayment,  "ОПЛАТА КРЕДИТА");
		}
	};

	static CPayOffSubjectMethodTypes PayOffSubjectMethodTypes;

	//--------------------------------------------------------------------------------
	/// Признак предмета расчета (1212).
	class CPayOffSubjectTypes : public CDescription<char>
	{
	public:
		CPayOffSubjectTypes()
		{
			using namespace SDK::Driver::EPayOffSubjectTypes;

			append(Unit,     "ТОВАР");
			append(Payment,  "ПЛАТЕЖ");
			append(AgentFee, "АГЕНТСКОЕ ВОЗНАГРАЖДЕНИЕ");
		}
	};

	static CPayOffSubjectTypes PayOffSubjectTypes;

	//--------------------------------------------------------------------------------
	/// Типы систем налогообложения (1062, 1055)
	class CTaxSystems : public CBitmapDescription<char>
	{
	public:
		CTaxSystems()
		{
			using namespace SDK::Driver::ETaxSystems;

			append(Main,                         "ОСН");
			append(SimplifiedIncome,             "УСН доход");
			append(SimplifiedIncomeMinusExpense, "УСН доход - расход");
			append(SingleImputedIncome,          "ЕНВД");
			append(SingleAgricultural,           "ЕСН");
			append(Patent,                       "Патент");
		}
	};

	static CTaxSystems TaxSystems;

	//--------------------------------------------------------------------------------
	/// Признаки платежного агента (1057, 1222).
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
	/// Признаки расчета (1054).
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
	/// Ставка НДС (1199).
	class CVATRates: public CDescription<char>
	{
	public:
		CVATRates::CVATRates()
		{
			append(1, "НДС 18%");
			append(2, "НДС 10%");
			append(3, "НДС 18/118");
			append(4, "НДС 10/110");
			append(5, "НДС 0%");
			append(6, "");
		}
	};

	static CVATRates VATRates;

	//--------------------------------------------------------------------------------
	const QString FDName            = QString::fromUtf8("КАССОВЫЙ ЧЕК");                /// ПФ тега 1000 (Наименование фискального документа).
	const QString LotteryMode       = QString::fromUtf8("ПРОВЕДЕНИЕ ЛОТЕРЕИ");          /// ПФ тега 1126 (Признак проведения лотереи).
	const QString GamblingMode      = QString::fromUtf8("ПРОВЕДЕНИЕ АЗАРТНОЙ ИГРЫ");    /// ПФ тега 1193 (Признак проведения азартных игр).
	const QString ExcisableUnitMode = QString::fromUtf8("ПОДАКЦИЗНЫЕ ТОВАРЫ");          /// ПФ тега 1207 (Признак торговли подакцизными товарами).

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
