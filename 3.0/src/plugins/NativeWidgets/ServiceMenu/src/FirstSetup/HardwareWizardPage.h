/* @file Окно найстройки железа. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// Проект
#include "WizardPage.h"

class EditorPane;
class DeviceSlot;
class HardwareWindow;

//----------------------------------------------------------------------------
class HardwareWizardPage : public WizardPageBase
{
	Q_OBJECT

public:
	HardwareWizardPage(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();

	virtual bool activate();
	virtual bool deactivate();

protected slots:
	/// Показываем диалог ожидания поиска устройств.
	void onDetectionStarted();

	/// Убираем диалог ожидания поиска устройств.
	void onDetectionFinished();

	/// Показываем диалог ожидания применения конфигурации.
	void onApplyingStarted();

	/// Убираем диалог ожидания применения конфигурации.
	void onApplyingFinished();

	/// Вызов редактора слота.
	void onEditSlot(DeviceSlot * aSlot, EditorPane * aPane);
	
	/// Удаление слота.
	void onRemoveSlot(DeviceSlot * aSlot);

	/// Редактор закончил работу.
	void onEditFinished();

	/// Изменился текущая форма (список устройств/выбор типа устройтсва/редактирование параметров).
	void onCurrentFormChanged(int aIndex);

	/// Отменяем поиск устройств
	void onClicked(const QVariantMap & aParameters);

private:
	HardwareWindow * mHardwareWindow;
	QWidget * mEditorWindow;
};

//----------------------------------------------------------------------------
