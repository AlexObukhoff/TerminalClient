/* @file Интерфейс для объектов, работающих с конфигурацией */

#pragma once

//------------------------------------------------------------------------
class IConfigManager
{
public:
	virtual ~IConfigManager() {}

public:
	/// Конфигурация изменилась?
	virtual bool isConfigurationChanged() const = 0;

	/// Делаем текущую конфигурацию начальной
	virtual void resetConfiguration() = 0;
};

//------------------------------------------------------------------------
