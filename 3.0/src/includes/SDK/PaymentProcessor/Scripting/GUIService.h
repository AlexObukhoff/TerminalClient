/* @file Прокси класс для работы с графическим движком в скриптах. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

class ICore;
class IGUIService;

namespace Scripting {

//------------------------------------------------------------------------------
class GUIService : public QObject
{
	Q_OBJECT

	Q_PROPERTY(bool isDisabled READ isDisabled)
	Q_PROPERTY(int width READ getWidth CONSTANT)
	Q_PROPERTY(int height READ getHeight CONSTANT)
	Q_PROPERTY(QVariantMap ui READ getParametersUI CONSTANT)
	Q_PROPERTY(QVariantMap ad READ getParametersAd CONSTANT)
	Q_PROPERTY(QString topScene READ getTopScene CONSTANT)

public:
	GUIService(ICore * aCore);

public slots:
	/// Отображает виджет.
	bool show(const QString & aWidget, const QVariantMap & aParameters);

	/// Отображает popup-виджет (на уровнь выше текущего).
	bool showPopup(const QString & aWidget, const QVariantMap & aParameters);

	/// Скрывает текущий popup или модальный виджет.
	bool hidePopup(const QVariantMap & aParameters = QVariantMap());

	/// Оповещает виджет.
	void notify(const QString & aEvent, const QVariantMap & aParameters);

	void reset();

	void reload(const QVariantMap & aParams);

	QString getTopScene() const;

	// Когда надо из qml что-то передать в скрипты
	void notifyScenario(const QVariantMap & aParams) { emit notifyScriptEngine(aParams); }

signals:
	void topSceneChange();
	void skinReload(const QVariantMap & aParams);
	void notifyScriptEngine(const QVariantMap & aParams);

private:
	QVariantMap getParametersUI() const;
	QVariantMap getParametersAd() const;

private:
	bool isDisabled() const;
	int getWidth() const;
	int getHeight() const;

private:
	ICore * mCore;
	IGUIService * mGUIService;
	QString mTopWidgetName;
};

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
