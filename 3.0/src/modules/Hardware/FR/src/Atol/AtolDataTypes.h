/* @file Типы данных ФР на протоколе АТОЛ. */

#pragma once

// SDK
#include <SDK/Drivers/FR/FiscalDataTypes.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/ASCII.h"
#include "Hardware/Protocols/FR/FiscalChequeStates.h"

// Project
#include "Hardware/FR/FRBaseConstants.h"

//--------------------------------------------------------------------------------
namespace CAtolFR
{
	/// Структура для распарсивания данных по прошивкам.
	struct SSoftInfo
	{
		QString version;    /// Версия прошивки (формата x.y).
		ushort build;       /// Номер сборки.
		QString language;   /// Языковая таблица.

		SSoftInfo(): version("Unknown"), build(0) {}
	};

	struct SCommadData
	{
		bool error;     /// Код ошибки бывает в ответе
		bool status;    /// Команда содержится в запросе статуса
		bool prefix;    /// Префикс бывает в ответе
		int timeout;    /// Таймаут на ENQ запроса чтения от устройства

		SCommadData() : error(true), status(false), prefix(true), timeout(0) {}
		SCommadData(bool aError, bool aStatus, bool aPrefix, int aTimeout) : error(aError), status(aStatus), prefix(aPrefix), timeout(aTimeout) {}
	};

	/// Параметры.
	namespace FRParameters
	{
		struct SData
		{
			uchar table;
			ushort series;
			uchar field;

			SData() : table(0), series(0), field(0) {}
			SData(uchar aTable, ushort aSeries, uchar aField) : table(aTable), series(aSeries), field(aField) {}
		};

		typedef SData (*TData)(int aSeries);
	}

	/// Регистры
	namespace Registers
	{
		struct SData
		{
			char number;
			int size;

			SData() : number(ASCII::NUL), size(0) {}
			SData(char aNumber, int aSize) : number(aNumber), size(aSize) {}
		};

		class CData : public CSpecification<QString, SData>
		{
		public:
			void add(const QString & aDescription, char aNumber, int aSize)
			{
				append(aDescription, SData(aNumber, aSize));
			}
		};
	}

	/// Таймауты чтения, [мс].
	namespace Timeouts
	{
		/// Дефолтный для выполнения команды.
		const int CommandDefault = 3000;
	}

	class CommandData : public CSpecification<QByteArray, SCommadData>
	{
	public:
		CommandData()
		{
			setDefault(SCommadData(true, false, true, Timeouts::CommandDefault));
		}

		void add(char aCommand, bool aError = true, bool aStatus = false, bool aPrefix = true) { append(QByteArray(1, aCommand), SCommadData(aError, aStatus, aPrefix, Timeouts::CommandDefault)); }
		void add(char aCommand, int aTimeout, bool aStatus = false) { append(QByteArray(1, aCommand), SCommadData(true, aStatus, true, aTimeout)); }
	};

	/// Структура для статических данных моделей.
	struct SModelData
	{
		int modelNumber;
		int maxStringSize;
		QString name;
		bool canBeDP;    // все новые ФР могут иметь прошивку для DP. Но здесь классификация введена еще и для обозначения совокупности новых настроек этих ФР
		bool terminal;
		bool verified;
		EFRType::Enum FRType;
		int build;
		bool ejector;
		bool cutter;
		int feedingAmount;
		int ZBufferSize;
		bool lineSpacing;

		SModelData(
			int aModelNumber,
			int aMaxStringSize,
			const QString & aName,
			bool aCanBeDP,
			bool aTerminal,
			bool aVerified,
			EFRType::Enum aFRType,
			int aBuild,
			bool aEjector,
			bool aCutter,
			int aFeedingAmount,
			int aZBufferSize,
			bool aLineSpacing) :
				modelNumber(aModelNumber),
				maxStringSize(aMaxStringSize),
				name(aName),
				canBeDP(aCanBeDP),
				terminal(aTerminal),
				verified(aVerified),
				FRType(aFRType),
				build(aBuild),
				ejector(aEjector),
				cutter(aCutter),
				feedingAmount(aFeedingAmount),
				ZBufferSize(aZBufferSize),
				lineSpacing(aLineSpacing) {}

		SModelData() :
			modelNumber(0),
			maxStringSize(1000),
			canBeDP(true),
			terminal(false),
			verified(false),
			FRType(EFRType::FS),
			build(0),
			ejector(false),
			cutter(true),
			feedingAmount(0),
			ZBufferSize(0),
			lineSpacing(true) {}
	};
}

//--------------------------------------------------------------------------------
