/* @file Данные моделей ФР на протоколе СПАРК. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CSparkFR
{
	/// Системные флаги.
	namespace SystemFlags
	{
		struct SData
		{
			int number;
			QString mask;
			QString name;

			SData(): number(0) {}
			SData(int aNumber, const QString & aMask, const QString & aName) : number(aNumber), mask(aMask), name(aName) {}
		};

		class Data : public QList<SystemFlags::SData>
		{
		public:
			void add(int aNumber, const QString & aMask, const QString & aName)
			{
				append(SystemFlags::SData(aNumber, aMask, aName));
			}
		};
	}

	//--------------------------------------------------------------------------------
	/// Модели.
	namespace Models
	{
		/// Название модели по умолчанию.
		const char Default[] = "SPARK FR";

		/// Модель SPARK 110K.
		const int Id110K = 110;

		/// Данные регэкспа для парсинга ответа на идентификацию.
		const char RegExpData[] = "([0-9]+).*([0-9\\.]+)";

		/// Данные модели.
		struct SData
		{
			QString name;
			int lineSize;
			bool verified;
			SystemFlags::Data systemFlags;

			SData() : name(Default), lineSize(43), verified(false) {}
			SData(const QString & aName, int aLineSize, bool aVerified, const SystemFlags::Data & aSystemFlags):
				name(aName), lineSize(aLineSize), verified(aVerified), systemFlags(aSystemFlags) {}
		};

		class CData : public CSpecification<int, SData>
		{
		public:
			CData()
			{
				//add(100, "SPARK FR100K", 44);
				//add(800, "SPARK 800TK" , 43);

				SystemFlags::Data flags110;    // Приоритеты установки флагов не менять.
				flags110.add(28, "1x10xx01", "extra options");                                 /// Дополнительные опции (только 110)
				flags110.add(13, "x0000000", "format and protection of fiscal document");      /// Формат и защита полей фискального чека

				flags110.add( 3, "xxxxxx0x", "cashbox and EKLZ extra options in Z-report");    /// Денежный ящик и доп. инфо ЭКЛЗ на Z-отчете
				flags110.add( 4, "xx000000", "format of fiscal document");                     /// Формат полей фискального чека
				flags110.add( 7, "10xxxxxx", "format of fiscal document and Z-report");        /// Формат полей фискального чека и Z-отчета
				flags110.add( 9, "1xxxxxxx", "inhibition of payment instrument");              /// Запреты средств платежей
				flags110.add(10, "xxxxx0xx", "KKM working features");                          /// Особенности работы ККМ
				flags110.add(14, "00xxxxxx", "paper heating and extra options");               /// Прогрев бумаги м другие настройки
				flags110.add(18, "11xxxxxx", "barcode options");                               /// Настройки печати штрих-кодов
				flags110.add(19, "xx01xxxx", "taxes and other options");                       /// Дополнительные возможности по использованию нулевой ставки налогов
				flags110.add(21, "1x1xxxxx", "system options");                                /// Системные настройки
				flags110.add(22, "xxx00000", "separating line");                               /// Линия разделения заголовка чека
				flags110.add(24, "01000x11", "system options 2");                              /// Системные настройки 2 (только 110)
				flags110.add(25, "00001000", "backfeed");                                      /// Величина обратного движения бумаги после обрезки чека (только 110)
				flags110.add(26, "01xxxxxx", "X,Z-report and paper spending");                 /// X- и Z-отчеты и расход бумаги после отрезки чека (только 110)

				add(110, "SPARK 110K", 44, flags110);
			}

		private:
			void add(int aTag, const QString & aName, int aLineSize)
			{
				append(aTag, SData(aName, aLineSize, false, SystemFlags::Data()));
			}

			void add(int aTag, const QString & aName, int aLineSize, const SystemFlags::Data & aSystemFlags)
			{
				append(aTag, SData(aName, aLineSize, true, aSystemFlags));
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
