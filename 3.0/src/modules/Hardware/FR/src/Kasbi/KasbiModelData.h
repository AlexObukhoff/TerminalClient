/* @file Данные моделей ФР на протоколе Касби. */

#pragma once

#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
namespace CKasbiFR
{
	/// Модели.
	namespace Models
	{
		/// Название модели по умолчанию.
		const char Default[] = "Kasbi FR";

		/// Данные модели.
		struct SData
		{
			QString name;
			bool verified;

			SData() : name(Default), verified(false) {}
			SData(const QString & aName, bool aVerified): name(aName), verified(aVerified) {}
		};

		class CData : public CSpecification<QString, SData>
		{
		public:
			CData()
			{
				add("Terminal-FA", "Kasbi Terminal-FA");
			}

		private:
			void add(const QString & aId, const QString & aName, bool aVerified = true)
			{
				append(aId, SData(aName, aVerified));
			}
		};

		static CData Data;
	}
}

//--------------------------------------------------------------------------------
