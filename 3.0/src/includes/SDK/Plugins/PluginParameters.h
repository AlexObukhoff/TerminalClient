/* @file Типы для опередения параметров плагина. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QVariantMap>
#include <QtCore/QVector>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Список имен параметров общих для любого плагина.
namespace Parameters {
	extern const char * Debug; /// Режим отладки
	extern const char * Singleton; /// Плагин - единоличник
}

//------------------------------------------------------------------------------
/// Описание параметра.
struct SPluginParameter
{
	/// Возможные типы.
	enum Type
	{
		Unknown = 0,
		Bool,
		Number,
		Text,
		Set,     /// Значение из списка.
		MultiSet /// Несколько значений из списка.
	};

	/// Конструктор.
	SPluginParameter(): type(Unknown) {}

	SPluginParameter(const QString & aName, Type aType, bool aRequired, const QString & aTitle, const QString & aDescription, const QVariant & aDefaultValue, const QVariantMap & aPossibleValues = QVariantMap(), bool aReadOnly = false) :
		type(aType), required(aRequired), name(aName), title(aTitle), description(aDescription), defaultValue(aDefaultValue), possibleValues(aPossibleValues), readOnly(aReadOnly)
	{
		if (aType == Type::Bool)
		{
			possibleValues.clear();
			possibleValues.insert("true",  true);
			possibleValues.insert("false", false);
		}
	}

	/// Специальный конструктор для типа Set.
	/// !! Если в сервисном меню необходимо останавливаться на позиции с незаполненным по умолччанию значением - надо ставить QVariant(), а не "" !!
	SPluginParameter(
		const QString & aName, bool aRequired,
		const QString & aTitle, const QString & aDescription,
		const QVariant & aDefaultValue, const QStringList & aPossibleValues, bool aReadOnly = false)
		: type(Set), name(aName), required(aRequired), title(aTitle), description(aDescription), defaultValue(aDefaultValue), readOnly(aReadOnly)
	{
		foreach (const QString & value, aPossibleValues)
		{
			possibleValues.insert(value, value);
		}
	}

	bool isValid()
	{
		return type && !name.isEmpty();
	}

	Type     type;         /// Тип.
	bool     required;     /// Определяет обязателен ли параметр.
	bool     readOnly;     /// Параметр только для чтения.
	QString  name;         /// Имя параметра (уникальный идентификатор в рамках приложения).
	QString  title;        /// Имя параметра для GUI (содержит локализуемое значение).
	QString  description;  /// Описание параметра для GUI (содержит локализуемое значение).
	QVariant defaultValue; /// Значение по умолчанию.

	/// Возможный набор значений (для типов Set и MultiSet).
	QVariantMap possibleValues;
};

typedef QVector<SPluginParameter> TParameterList;

/// Функция для поиска нужного параметра.
inline SPluginParameter findParameter(const QString & aName, const TParameterList & aParameters)
{
	foreach (const SPluginParameter & parameter, aParameters)
	{
		if (parameter.name == aName)
		{
			return parameter;
		}
	}

	return SPluginParameter();
}

//------------------------------------------------------------------------------
}} // namespace SDK::Plugin

Q_DECLARE_METATYPE(SDK::Plugin::SPluginParameter);
Q_DECLARE_METATYPE(SDK::Plugin::TParameterList);

//------------------------------------------------------------------------------
