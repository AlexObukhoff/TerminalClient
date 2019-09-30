/* @file Данные ФН. */

#pragma once

// Modules
#include "Hardware/Common/Specifications.h"

// Project
#include "Hardware/FR/FRDataTypes.h"

//--------------------------------------------------------------------------------
namespace FS
{
	struct SData
	{
		int expiration;      /// Срок годности.
		EFFD::Enum FFD;      /// "Родная" версия ФФД.
		QString provider;    /// Поставщик.
		QString revision;    /// Ревизия.

		SData(): expiration(13), FFD(EFFD::F10) {}
		SData(int aExpiration, EFFD::Enum aFFD, const QString & aProvider, const QString & aRevision): expiration(aExpiration), FFD(aFFD), provider(aProvider), revision(aRevision) {}
	};

	class CData: public CSpecification<QString, SData>
	{
	public:
		CData();

		/// Содержит ли шаблон серийного номера.
		bool contains(const QString & aSerialNumber) const;

	protected:
		/// Добавить данные.
		void add(const QString & aSerialNumber, int aExpiration, EFFD::Enum aFFD, const char * aProvider, const char * aRevision = "");

		/// Получить данные по серийному номеру.
		virtual SData value(const QString & aSerialNumber) const;

		/// Получить шаблон серийного номера по серийному номеру.
		QStringList::const_iterator getSerialNumberTemplate(const QString & aSerialNumber) const;

		/// Шаблоны серийных номеров.
		QStringList mSerialNumberTemplates;
	};

	static CData Data;
}

//--------------------------------------------------------------------------------
