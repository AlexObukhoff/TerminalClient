﻿/* @file Набор вспомогательных функций для qml. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextCodec>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtCore/QSharedPointer>
#include <QtGui/QFont>
#include <Common/QtHeadersEnd.h>

#include "Skin.h"
#include "Translator.h"
#include "GroupModel.h"
#include "ProviderListModel.h"
#include "ProviderListFilter.h"

class QQmlEngine;

//------------------------------------------------------------------------------
class Utils : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QObject * locale READ getTranslator NOTIFY updateTranslator);
	Q_PROPERTY(QObject * GroupModel READ getGroupModel NOTIFY updateGroupModel);
	Q_PROPERTY(QObject * RootGroupModel READ getRootGroupModel NOTIFY updateRootGroupModel);
	Q_PROPERTY(QObject * ProviderList READ getProviderList NOTIFY updateProviderList);

public:
	Utils(QQmlEngine * aEngine, const QString & aInterfacePath, const QString & aUserPath);

public slots:
	/// Генерирует системное событие нажатия клавиши.
	void generateKeyEvent(int aKey, int aModifiers, const QString & aText = "") const;

	/// Извлекает значение введённое пользоваелем из строки с маской.
	QString stripMask(const QString & aSource, const QString & aMask) const;

	/// Возвращает позицию курсора после применения маски к строке.
	int getCursorPosition(const QString & aMask, const QString & aBefore, const QString & aAfter) const;

	/// Формирует строку с помощью маски вида [x][y][z]..., где x, y, z - индекс символа во входной строке.
	QString format(const QString & aSource, const QString & aFormat) const;

	/// Загружает содержимое файла.
	QString readFile(const QString & aPath) const;

	/// Проверяет файл на существование
	bool fileExist(const QString & aPath) const;

	/// Заменяет все вхождения [tag][/tag] на <tag></tag>
	QString toHtml(const QString & aSource) const;

	/// Удаляем все вхождения [tag][/tag]
	QString toPlain(const QString & aSource) const;

	/// Перекодировать из base64 в utf8
	QString fromBase64(const QString & aSource) const;

	/// Перекодировать из urlenc(win-1251) в utf8
	QString fromUrlEncoding(const QString & aSource) const;

	/// Проигрывание звука
	void playSound(const QString & aFileName) const;

	/// Клик через очередь событий
	void click();

	QVariantMap str2json(const QString & aString) const;

	QString json2str(const QObject * aJSON) const { Q_UNUSED(aJSON); return QString(); }

public:
	typedef QMap<qint64, QString> TSkinConfig;

public:
	/// Возвращает транслятор.
	QObject * getTranslator();

	/// Возвращает модель групп.
	QObject * getGroupModel();

	/// Возвращает модель корневых групп.
	QObject * getRootGroupModel();

	/// Возвращает модель с фильтрованным списком провайдеров.
	QObject * getProviderList();

	// Возвращает пары id оператора - имя скина
	TSkinConfig getSkinConfig() const;

signals:
	void updateTranslator();
	void updateGroupModel();
	void updateRootGroupModel();
	void updateProviderList();

	void clicked();

private slots:
	void onClicked();

private:
	/// Загрузка конфигурации
	void loadConfiguration();

	/// Загрузить статистику из ядра
	QMap<qint64, quint32> getStatistic();

private:
	QQmlEngine * mEngine;
	mutable QMap<QString, QTextCodec *> mCodecCache;

	QSharedPointer<Translator> mTranslator;
	QSharedPointer<GroupModel> mGroupModel; /// Модель иконок внутри корневых групп
	QSharedPointer<GroupModel> mRootGroupModel; /// Модель иконок корневых групп
	QSharedPointer<ProviderListModel> mProviderListModel; /// Сквозной список провайдеров для поиска
	QSharedPointer<ProviderListFilter> mProviderListFilter; /// Фильтр для поиска провайдеров

	QString mInterfacePath;
	QString mUserPath;
	TSkinConfig mSkinConfig;

	bool mUseCommonSounds;
	bool mUseNarratorSounds;
	bool mUseAutoOrderProviders;
};

//------------------------------------------------------------------------------
