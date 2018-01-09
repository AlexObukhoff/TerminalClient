/* @file Данные моделей устройств на протоколе ID003. */

#pragma once

#include "Hardware/CashAcceptors/ModelData.h"

//--------------------------------------------------------------------------------
namespace CID003
{
	/// Модели
	namespace Models
	{
		const char GPTAurora[] = "GPT Aurora";
		const char JCMIPRO[] = "JCM IPRO";
		const char JCMUBA[] = "JCM UBA";
		const char CashcodeMVU[] = "Cashcode MVU";
		const char JCMVega[] = "JCM Vega";
	}

	/// Подтип протокола. Разделение чисто символическое, связано с лицензированием пользования протокола ID003.
	/// По подтипу можно сделать вывод о фирме, которая его реализовала в девайсе.
	namespace ProtocolData
	{
		const char GPTAurora[] = "ID003 GPT"; /// Id GPT на ID003.

		/// Подтип протокола. Разделение чисто символическое, связано с лицензированием пользования протоколом ID003.
		/// По подтипу можно сделать вывод о фирме, которая его реализовала в девайсе.
		namespace Alias
		{
			const char ID003[]    = "ID003";       /// ID003 (JCM).
			const char ID003Ext[] = "External";    /// ID003 (JCM, расширенная идентификация).
			const char BDP[]      = "BDP";         /// BDP (Cashcode).
			const char OP003[]    = "OP003";       /// OP003 (КБ СП).
		}

		/// Лексема ответа на запрос идентификации.
		const char Lexeme[] = "[0-9A-Za-z_\\.]";
		const QString IdLexeme = QString("(%1*)").arg(Lexeme);

		/// Класс для перебора регулярок для парсинга ответа на запрос идентификации.
		class CIdentification : public CDescription<QString>
		{
		public :
			CIdentification();
		};
	}

	/// Количество данных о конструктиве в ответе на идентификацию для новых моделей JCM (IPRO и Vega)
	const int NewJCMModelDataCount = 3;

	//--------------------------------------------------------------------------------
	class BaseModelData : public CSpecification<QString, SBaseModelData>
	{
	public:
		void add(const QString & aId, const QString & aName, bool aVerified = false)
		{
			append(aId, SBaseModelData(aName, aVerified));
		}
	};

	//--------------------------------------------------------------------------------
	struct SModelData : public SBaseModelData
	{
		QString type;
		BaseModelData models;

		SModelData() {}
		SModelData(const QString & aName, bool aVerified) : SBaseModelData(aName, aVerified) {}
		SModelData(const QString & aDefaultName, const QString & aType);
	};

	class ModelData : public CSpecification<char, SModelData>
	{
	public :
		ModelData();
		SBaseModelData getData(char aCode, const QString & aNumber, const QString & aStackerType);

	private:
		void add(char aId, const QString & aName, bool aVerified = false);
	};
}

//--------------------------------------------------------------------------------
