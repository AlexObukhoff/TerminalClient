/* @file Окно настроек оборудования. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include "ui_HardwareServiceWindow.h"
#include <Common/QtHeadersEnd.h>

// Проект
#include "IServiceWindow.h"

class HardwareWindow;
class DeviceSlot;
class EditorPane;

//------------------------------------------------------------------------
class HardwareServiceWindow : public QFrame, public ServiceWindowBase, protected Ui::HardwareServiceWindow
{
	Q_OBJECT

public:
	HardwareServiceWindow(ServiceMenuBackend * aBackend, QWidget * aParent = 0);

	virtual bool initialize();
	virtual bool shutdown();
	virtual bool activate();
	virtual bool deactivate();

private slots:
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
	
	/// Отменяем поиск устройств
	void onClicked(const QVariantMap & aParameters);

private:
	HardwareWindow * mWindow;
	bool mShown;
	bool mFinish;
};

//------------------------------------------------------------------------
