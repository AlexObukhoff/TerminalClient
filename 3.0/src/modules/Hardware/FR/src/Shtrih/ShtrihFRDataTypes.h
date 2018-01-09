/* @file Типы данных, использующихся в реализации функционала ФР Штрих. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPair>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"

// SDK
#include <SDK/Drivers/FR/FiscalDataTypes.h>

//--------------------------------------------------------------------------------
namespace CShtrihFR
{
	/// Структура для распарсивания данных по прошивкам.
	struct SSoftInfo
	{
		QString version;    /// Версия прошивки (формата x.y).
		ushort build;       /// Номер сборки.
		QDate date;         /// Дата.
	};

	/// Типы регистров ФР Штрих.
	namespace ERegisterType
	{
		enum Enum
		{
			Unknown,
			Money,
			Operational
		};
	}

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Дефолтный таймаут чтения.
		const int Default = 3500;
	}

	/// Команды.
	namespace Commands
	{
		struct SData
		{
			int timeout;
			bool password;

			SData(): timeout(Timeouts::Default), password(true) {}
			SData(int aTimeout, bool aPassword): timeout(aTimeout), password(aPassword) {}
		};

		class Data : public CSpecification<QByteArray, SData>
		{
		public:
			void add(const QByteArray & aCode, int aTimeout = Timeouts::Default, bool aPassword = true)
			{
				append(aCode, SData(aTimeout, aPassword));
			}

			void add(char aCode, int aTimeout = Timeouts::Default, bool aPassword = true)
			{
				add(QByteArray(1, aCode), aTimeout, aPassword);
			}
		};
	}

	/// Данные моделей.
	//TODO: докрутить наличие датчиков
	struct SModelData
	{
		QString id;
		QString name;
		bool verified;
		bool ejector;
		int ZReportQuantity;
		int feed;
		QDate date;
		int build;
		int linePrintingTimeout;

		SModelData(
			const QString & aId,
			const QString & aName,
			bool aVerified,
			bool aEjector,
			int aZReportQuantity,
			int aFeed,
			const QDate & aDate,
			int aBuild,
			int aLinePrintingTimeout):
				id(aId),
				name(aName),
				ejector(aEjector),
				ZReportQuantity(aZReportQuantity),
				verified(aVerified),
				feed(aFeed),
				date(aDate),
				build(aBuild),
				linePrintingTimeout(aLinePrintingTimeout)
		{}

		SModelData(): verified(false), ejector(false), ZReportQuantity(0), feed(0), date(QDate::currentDate()), build(0), linePrintingTimeout(0) {}
	};

	typedef QPair<uchar, ERegisterType::Enum> TRegisterId;

	/// Таблицы для программирования (некоторые, используемые).
	namespace Tables
	{
		const char Modes = 1;
	}

	/// Параметры ФР.
	namespace FRParameters
	{
		const int NA = -1;    /// Not Available, параметр недоступен.

		/// Полная отрезка.
		const char FullCutting = 0x01;

		/// Неполная отрезка.
		const char PartialCutting = 0x02;

		struct SData
		{
			int field;
			int table;
			QString description;

			SData() : field(NA), table(0) {}
			SData(int aField, int aTable, const QString & aDescription = "") : field(aField), table(aTable), description(aDescription) {}
		};

		typedef QList<SData> TData;
		typedef QList<int> TFields;
		typedef QList<int> TTables;

		/// Параметры системных таблиц.
		struct SFields
		{
			SData autoNulling;
			SData documentCapEnable;
			SData printOnControlTape;
			SData printOnChequeTapeOnly;
			SData printNotNullingSum;
			SData cutting;
			SData documentCapPlace;
			SData printOneAmount;
			SData feedBeforeCutting;
			SData weightSensorEnable;
			SData ZReportType;
			SData taxesCalculation;
			SData taxesPrinting;
			SData documentCapAmount;
			SData printDocumentCap;
			SData cuttingWithOpenedDocument;
			SData autoRetractingCheques;
			SData autoRetractingReports;
			SData presentationLength;
			SData leftReceiptTimeout;
			SData loop;

			SFields(const TFields & aFields)
			{
				SFields(aFields, TTables());

				TData data;

				for (int i = 0; i < aFields.size(); ++i)
				{
					data << SData(aFields[i], Tables::Modes);
				}

				add(data);
			}

			SFields(const TFields & aFields, const TTables & aTables)
			{
				TData data;
				int size = qMin(aFields.size(), aTables.size());

				for (int i = 0; i < size; ++i)
				{
					data << SData(aFields[i], aTables[i]);
				}

				add(data);
			}

			SFields() : mMaxNADescriptionSize(0) {}

			/// Получить максимальный размер описания отсутствующего в системных таблицах конкретного ФР поля.
			int getMaxNADescriptionSize() { return mMaxNADescriptionSize; }

		private:
			void add(const TData & aData)
			{
				#define ADD_SHTRIH_FIELD(aIndex, aName) if (aData.size() > aIndex) aName = aData[aIndex]; aName.description = #aName##" ("#aIndex##")"; \
					if ((aData.size() > aIndex) && (aName.field == NA)) mMaxNADescriptionSize = qMax(mMaxNADescriptionSize, aName.description.size());

				mMaxNADescriptionSize = 0;

				ADD_SHTRIH_FIELD( 0, autoNulling);                  // Автоматическое обнуление денежной наличности при закрытии смены
				ADD_SHTRIH_FIELD( 1, documentCapEnable);            // Печать рекламного текста
				ADD_SHTRIH_FIELD( 2, printOnControlTape);           // Печать текстовых строк на ленте операционного журнала
				ADD_SHTRIH_FIELD( 3, printOnChequeTapeOnly);        // Печать только на чековой ленте
				ADD_SHTRIH_FIELD( 4, printNotNullingSum);           // Печать необнуляемой суммы
				ADD_SHTRIH_FIELD( 5, cutting);                      // Отрезка чека после завершения печати
				ADD_SHTRIH_FIELD( 6, documentCapPlace);             // Печать заголовка чека
				ADD_SHTRIH_FIELD( 7, printOneAmount);               // Печать единичного количества
				ADD_SHTRIH_FIELD( 8, feedBeforeCutting);            // Промотка ленты перед отрезкой чека
				ADD_SHTRIH_FIELD( 9, weightSensorEnable);           // Использование весовых датчиков при контроле отсутствия бумаги
				ADD_SHTRIH_FIELD(10, ZReportType);                  // Тип суточного отчета
				ADD_SHTRIH_FIELD(11, taxesCalculation);             // Начисление налогов
				ADD_SHTRIH_FIELD(12, taxesPrinting);                // Печать налогов
				ADD_SHTRIH_FIELD(13, documentCapAmount);            // Количество строк рекламного текста
				ADD_SHTRIH_FIELD(14, printDocumentCap);             // Печать клише
				ADD_SHTRIH_FIELD(15, cuttingWithOpenedDocument);    // Отрезка при открытом чеке
				ADD_SHTRIH_FIELD(16, autoRetractingCheques);        // Выброс чеков
				ADD_SHTRIH_FIELD(17, autoRetractingReports);        // Выброс отчетов
				ADD_SHTRIH_FIELD(18, presentationLength);           // Длина презентации чеков
				ADD_SHTRIH_FIELD(19, leftReceiptTimeout);           // Таймаут ретракции
				ADD_SHTRIH_FIELD(20, loop);                         // Делать петлю при печати
			}

			/// Максимальный размер описания отсутствующего в системных таблицах конкретного ФР поля.
			int mMaxNADescriptionSize;
		};
	}
}

//--------------------------------------------------------------------------------
