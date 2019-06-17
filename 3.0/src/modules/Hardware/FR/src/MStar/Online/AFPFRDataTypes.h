/* @file Типы данных, использующихся в реализации функционала ФР AFP. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDate>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
namespace CAFPFR
{
	/// Типы данных в запросах и ответах. Используются для парсинга ответов.
	namespace EAnswerTypes
	{
		enum Enum
		{
			Unknown,    /// Не определен.
			String,     /// Строка.
			FString,    /// Непустая строка.
			Date,       /// Дата.
			Time,       /// Время.
			Int,        /// Целое. Пустое == 0.
			FInt,       /// Непустое целое.
			Double      /// Дробное.
		};
	}

	/// Данные для взаимодействия с протоколом.
	typedef QList<QVariant> TData;

	/// Типы данных ответа для взаимодействия с протоколом.
	typedef QList<EAnswerTypes::Enum> TAnswerTypes;

	//------------------------------------------------------------------------------------------------
	/// Данные ФР.
	namespace FRInfo
	{
		struct SData
		{
			int index;
			TAnswerTypes answerTypes;
			QString name;

			SData(): index(0) {}
			SData(int aIndex, EAnswerTypes::Enum aAnswerType, const QString & aName): index(aIndex), answerTypes(TAnswerTypes() << aAnswerType), name(aName) {}
			SData(int aIndex, const TAnswerTypes & aAnswerTypes, const QString & aName): index(aIndex), answerTypes(aAnswerTypes), name(aName) {}
		};
	}

	//------------------------------------------------------------------------------------------------
	/// Параметры.
	namespace FRParameters
	{
		const int NoBit = -1;    /// Не битовое поле.

		struct SData
		{
			int number;
			EAnswerTypes::Enum answerType;
			QString name;
			int bit;
			int index;

			SData() : number(0), answerType(EAnswerTypes::Unknown), bit(NoBit), index(0) {}
			SData(int aNumber, EAnswerTypes::Enum aAnswerType, const QString & aName, int aBit = NoBit, int aIndex = 0) :
				number(aNumber), answerType(aAnswerType), name(aName), bit(aBit), index(aIndex) {}

			QString log() const
			{
				QString bitLog = (bit == NoBit) ? "" : QString("-%1").arg(bit);

				return QString("field %1-%2%3 (%4)").arg(number).arg(index).arg(bitLog).arg(name);
			}
		};

		typedef SData (*TData)(int aSeries);
	}

	//------------------------------------------------------------------------------------------------
	/// Данные моделей.
	namespace Models
	{
		/// Модели по умолчанию.
		const char Default[] = "Multisoft AFP online FR";

		struct SData
		{
			QString name;
			QString firmware;
			bool verified;

			SData(): name(Default), verified(false) {}
			SData(const QString & aName, const QString & aFirmware, bool aVerified): name(aName), firmware(aFirmware), verified(aVerified) {}
		};
	}

	/// Таймауты, [мс].
	namespace Timeouts
	{
		/// Дефолтный чтения ответа.
		const int Default = 1000;
	}

	//------------------------------------------------------------------------------------------------
	/// Данные запросов.
	namespace Requests
	{
		struct SData
		{
			TAnswerTypes answerTypes;
			int timeout;

			SData(): timeout(Timeouts::Default) {}
			SData(const TAnswerTypes & aAnswerTypes, int aTimeout): answerTypes(aAnswerTypes), timeout(aTimeout) {}
		};

		class CDataBase : public CSpecification<char, SData>
		{
		protected:
			void add(char aCommand, const TAnswerTypes & aTypes, int aTimeout = Timeouts::Default)
			{
				append(aCommand, SData(aTypes, aTimeout));
			}

			void add(char aCommand, int aTimeout = Timeouts::Default)
			{
				append(aCommand, SData(TAnswerTypes(), aTimeout));
			}
		};
	}
}

//--------------------------------------------------------------------------------
