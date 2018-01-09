/* @file Плагин абстрактной фабрики. */

#pragma once

#include <SDK/Plugins/IPlugin.h>

namespace SDK {
namespace Plugin {

//------------------------------------------------------------------------------
/// Реализует создание группы классов с базой T.
template <typename T> class IFactory : public IPlugin
{
public:
	/// Возвращает список имен классов, которые создает фабрика.
	virtual QStringList getClassNames() const = 0;

	/// Создает класс c заданным именем.
	virtual T * create(const QString & aClassName) const = 0;
};

//------------------------------------------------------------------------------
}}