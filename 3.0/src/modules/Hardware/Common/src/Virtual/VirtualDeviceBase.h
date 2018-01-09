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
	/// Запрос статуса.
	virtual bool getStatus(TStatusCodes & aStatusCodes);

	/// Фильтр событий, для отслеживаения нажатия кнопок.
	virtual bool eventFilter(QObject * aWatched, QEvent * aEvent);

	/// Фильтровать нажатие кнопки(ок).
	virtual void filterKeyEvent(int /*aKey*/, const Qt::KeyboardModifiers & /*aModifiers*/) {};

	/// Мигнуть статус-кодом.
	void blinkStatusCode(int aStatusCode);

	/// Инвертировать статус-код.
	void changeStatusCode(int aStatusCode);

	/// Текущие статус-коды.
	TStatusCodes mStatusCodes;
};

//-----------------------------------------------------------------------------------------------------------
