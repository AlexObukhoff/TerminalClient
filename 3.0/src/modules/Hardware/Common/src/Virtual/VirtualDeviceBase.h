/* @file Базовый класс виртуальных устройств. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QEvent>
#include <QtGui/QKeyEvent>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/DeviceBase.h"

//---------------------------------------------------------------------------------------------
template<class T>
class VirtualDeviceBase : public T
{
	SET_SERIES("Virtual")

public:
	VirtualDeviceBase();

	/// Подключает и инициализует устройство. Обертка для вызова функционала в рабочем потоке.
	virtual void initialize();

	/// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова updateParameters().
	virtual bool release();

protected:
	/// Завершение инициализации.
	virtual void finaliseInitialization();

	/// Получить статус.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Переинициализация в рамках фоновой логики пост-поллинга.
	virtual void reInitialize();

	/// Фильтр событий, для отслеживаения нажатия кнопок.
	virtual bool eventFilter(QObject * aWatched, QEvent * aEvent);

	/// Фильтровать нажатие кнопки(ок).
	virtual void filterKeyEvent(int /*aKey*/, const Qt::KeyboardModifiers & /*aModifiers*/) {};

	/// Мигнуть статус-кодом.
	void blinkStatusCode(int aStatusCode);

	/// Инвертировать статус-код.
	void changeStatusCode(int aStatusCode);

	/// Клавиша - модификатор?
	bool isKeyModifier(int aKey) const;

	/// Текущие статус-коды.
	TStatusCodes mStatusCodes;
};

//-----------------------------------------------------------------------------------------------------------
