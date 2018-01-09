/* @file Класс для перевода строк внутри QML модулей. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <Common/QtHeadersEnd.h>

//------------------------------------------------------------------------------
class Translator : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString language READ getLanguage NOTIFY languageChanged);
	Q_PROPERTY(QString defaultLanguage READ getDefaultLanguage CONSTANT FINAL);

public:
	Translator(const QString & aInterfacePath);

public slots:
	/// Перевести строку
	QString tr(const QString & aString);

	/// Установить язык интерфейса.
	void setLanguage(const QString & aLanguage);

	/// Получить текущий язык интерфейса.
	QString getLanguage() const;

	/// Получить язык по умолчанию
	QString getDefaultLanguage() const;

	/// Получить список доступных языков.
	QStringList getLanguageList() const;

signals:
	void languageChanged();

private:
	QString mInterfacePath;

	// Список всех поддерживаеиых языков <наименование, локализованное название языка>.
	QMap<QString, QString> mLanguages;

	// Набор трансляторов для каждого модуля.
	QMap<QString, QTranslator *> mTranslators;

	QString mCurrentLanguage;
	QString mDefaultLanguage;
};

//------------------------------------------------------------------------------
