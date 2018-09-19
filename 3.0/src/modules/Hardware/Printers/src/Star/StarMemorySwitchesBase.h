/* @file Движок мем-свичей принтеров STAR.*/

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// Common
#include <Common/ILogable.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/HardwareConstants.h"

// Project
#include "StarMemorySwitchTypes.h"

//--------------------------------------------------------------------------------
/// Константы принтера, используются по дефолту в случае пустого конфига.
namespace CSTAR
{
	/// Варианты значений комплексного параметра мем-свича.
	typedef QMap<QString, QStringList> TMSWParameters;

	/// Варианты значений линейного параметра мем-свича.
	typedef QMap<QString, QString> TMSWSimpleParameters;

	/// Группа моделей, для окторой предназначен данный параметр.
	typedef QSet<QString> TModels;

	/// Параметр мем-свича.
	struct SMSWParameter
	{
		int number;     /// Номер мем-свича.
		int index;     /// Индекс начального бита параметра мем-свича.
		int bitAmount;     /// Количество битов параметра мем-свича.
		QStringList dataTypes;    /// Типы данных в структурированном виде.
		TMSWParameters parameters;    /// Варианты значений параметра мем-свича.
		QString description;    /// Описание (б.ч. для логгирования).
		TModels models;

		SMSWParameter() : number(-1), index(-1), bitAmount(0) {}
		SMSWParameter(int aNumber, int aIndex, int aBitAmount, const QStringList & aDataTypes, const TMSWParameters & aParameters, const QString & aDescription, const TModels & aModels) :
			number(aNumber), index(aIndex), bitAmount(aBitAmount), dataTypes(aDataTypes), parameters(aParameters), description(aDescription), models(aModels) {}
	};

	typedef QMap<ESTARMemorySwitchTypes::Enum, SMSWParameter> TMSWData;
	
	struct SMemorySwitch
	{
		ushort value;
		bool valid;

		SMemorySwitch() : value(0), valid(false) {}
		bool operator==(const SMemorySwitch & aMemorySwitch) const { return value == aMemorySwitch.value; }
	};

	typedef QVector<SMemorySwitch> TMemorySwitches;
	typedef QList<ESTARMemorySwitchTypes::Enum> TMemorySwitchTypes;

	/// Описатель параметров мем-свичей.
	class BaseMemorySwitchUtils: public ILogable
	{
	public:
		/// Проверить возможность и обновить параметр memory-switch-а по настройкам из конфига.
		bool update(TMemorySwitches & aMemorySwitches);
		bool update(const TMemorySwitchTypes & aSTARMemorySwitchTypes, TMemorySwitches & aMemorySwitches, const QVariantMap & aConfiguration);

		/// Установить конфигурацию для выбранного параметра мем-свича.
		bool setConfiguration(const TMemorySwitches & aMemorySwitches);

		/// Установить конфигурацию.
		void setConfiguration(const QVariantMap & aConfiguration);

		/// Устанавить группу моделей, для которой будет выполняться логика работы с мем-свичами.
		void setModels(const TModels & aModels);

		/// Получить фактическое значение параметра по данным мем-свичей.
		bool getConfigParameter(ESTARMemorySwitchTypes::Enum aType, const TMemorySwitches & aMemorySwitches, QVariant & aValue);

	protected:
		/// Добавить данные составного параметра мем-свича.
		void add(ESTARMemorySwitchTypes::Enum aParameterType, int aNumber, int aIndex, const QStringList & aDataTypes, const TMSWParameters & aParameters, const QString & aDescription, const TModels & models = TModels());
		void add(ESTARMemorySwitchTypes::Enum aParameterType, int aNumber, int aIndex, const QString & aDataType, const TMSWParameters & aParameters, const QString & aDescription, const TModels & models = TModels());

		/// Заполнить данные параметра мем-свича.
		void fillExcept(TMSWParameters & aParameters, const TMSWSimpleParameters & aKnownParameters, const QString & aExceptValue = "");
		void fillExcept(TMSWParameters & aParameters, const QString & aValue, const QString & aKey, const QString & aExceptValue = "");

		/// Добавить данные булевского параметра мем-свича.
		void add(ESTARMemorySwitchTypes::Enum aParameterType, int aNumber, int aIndex, bool aInvert, const QString & aDataName, const QString & aDescription, const TModels & models = TModels());

		/// Получить из конфигурации значение параметра по ключу и выбранной конфигурации.
		bool getParameterValue(ESTARMemorySwitchTypes::Enum aParameterType, const QVariantMap & aConfiguration, QStringList & aValue);

		/// Проверить достаточность ключей конфига для работы с параметром мем-свича.
		bool hasConfigKeys(ESTARMemorySwitchTypes::Enum aParameterType, const QVariantMap & aConfiguration);

		/// Получить ключи конфига для заданной модели.
		CSTAR::TMemorySwitchTypes getMemorySwitchTypes();

		/// Проверить валидность параметра мем-свича.
		bool hasValidData(ESTARMemorySwitchTypes::Enum aParameterType, const TMemorySwitches & aMemorySwitches);

		/// Получить маску для параметра мем-свича.
		ushort getMask(ESTARMemorySwitchTypes::Enum aParameterType);

		/// Получить данные параметра мем-свича по его типу с учетом используемой группы моделей.
		SMSWParameter getMSWParameter(ESTARMemorySwitchTypes::Enum aParameterType);

		/// Данные мем-свичей.
		TMSWData mData;

		/// Настройки для обновления параметров мем-свичей.
		QVariantMap mConfiguration;

		/// Модель/группа моделей, для которой применяются операции с мем-свичами.
		TModels mModels;
	};

	struct SModelMSWData
	{
		QVariantMap configuration;
		TModels models;
	};

	/// Общие обязательные параметры для всех принтеров.
	class CMainSettingsBase : public CSpecification<ESTARMemorySwitchTypes::Enum, SModelMSWData>
	{
	protected:
		void add(ESTARMemorySwitchTypes::Enum aType, const QString & aName, const QVariant & aValue, const TModels & aModels = TModels())
		{
			QVariant value(aValue);
			
			if (value.type() == QVariant::Bool)
			{
				value = value.toBool() ? CHardwareSDK::Values::Use : CHardwareSDK::Values::NotUse;
			}

			QVariantMap configuration;
			configuration.insert(aName, value);

			if (mBuffer[aType].models != aModels)
			{
				SModelMSWData data = {configuration, aModels};
				mBuffer.insertMulti(aType, data);
			}
			else
			{
				mBuffer[aType].configuration.insert(aName, value);
			}
		}
	};
}

//--------------------------------------------------------------------------------
