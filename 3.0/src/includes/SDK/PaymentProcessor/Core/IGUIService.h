/* @file Интерфейс, обеспечивающий взаимодействие с графикой. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/GUI/IAdSource.h>
#include <SDK/PaymentProcessor/Scripting/IBackendScenarioObject.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IGUIService
{
public:
	/// Отображает виджет.
	virtual bool show(const QString & aWidget, const QVariantMap & aParameters) = 0;

	/// Отображает popup-виджет (на уровнь выше текущего).
	virtual bool showPopup(const QString & aWidget, const QVariantMap & aParameters) = 0;

	/// Отображает модальный виджет.
	virtual QVariantMap showModal(const QString & aWidget, const QVariantMap & aParameters) = 0;

	/// Скрывает текущий popup или модальный виджет.
	virtual bool hidePopup(const QVariantMap & aParameters = QVariantMap()) = 0;

	/// Оповещает виджет.
	virtual void notify(const QString & aEvent, const QVariantMap & aParameters) = 0;

	/// Проверка состояния интерфейса.
	virtual bool isDisabled() const = 0;

	/// Удалает все объекты GraphicsItem, очищает сцену
	virtual void reset() = 0;

	/// Возвращает текущий размер экрана в пикселях.
	virtual QRect getScreenSize(int aIndex) const = 0;

	virtual QPixmap getScreenshot() = 0;

	/// Возвращаем мапу параметров секции aSection из файла interface.ini
	virtual QVariantMap getUiSettings(const QString & aSection) const = 0;

	/// Возвращает первого по списку поставщика рекламы
	virtual SDK::GUI::IAdSource * getAdSource() const = 0;

	/// Возвращает объект скриптового ядра
	virtual QObject * getBackendObject(const QString & aName) const = 0;
	
protected:
	virtual ~IGUIService() {}
};

//------------------------------------------------------------------------------
}} // SDK::PaymentProcessor
