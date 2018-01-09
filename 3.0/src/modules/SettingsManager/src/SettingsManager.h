/* @file Интерфейс менеджера настроек. */

#pragma once

// boost
#pragma push_macro("foreach")
#undef foreach
#include <boost/property_tree/ptree.hpp>
#pragma pop_macro("foreach")

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <Common/QtHeadersEnd.h>

// Модули
#include "Common/ILogable.h"
#include <Common/PropertyTree.h>

/// Имя файла конфига - имя ветки дерева настроек.
typedef QMap<QString, QString> TConfigFileMap;

//---------------------------------------------------------------------------
struct SSettingsSource
{
	QString configFileName;
	QString adapterName;
	QString symlinkName;
	bool readOnly;
	
	SSettingsSource();
	explicit SSettingsSource(const QString & aFileName, const QString & aAdapterName, bool aReadOnly);
	explicit SSettingsSource(const QString & aFileName, const QString & aAdapterName, const char * aSymlinkName);

	bool isSymlink() const { return !symlinkName.isEmpty(); }

	/// Имена полей из настроек, которые были загружены из данного источника.
	QList<QString> fieldNames;
};

//---------------------------------------------------------------------------
class SettingsManager : public ILogable
{
public:
	SettingsManager(const QString & aConfigPath);
	~SettingsManager();

	/// Загружает все указанные конфиги.
	bool loadSettings(const QList<SSettingsSource> & aSettingSources);

	/// Сохраняем конфиги.
	bool saveSettings();

	/// Получить поддерево настроек.
	TPtree & getProperties(const QString & aAdapterName = QString());

	/// Производит сравнение дерева настроек другого менеджера настроек
	bool isEqual(const SettingsManager & aManager) const;

private:
	bool readXML(const QString & aFileName, TPtree & aTree);
	bool writeXML(const QString & aFileName, const TPtree & aTree);
	void writeXMLNode(class QXmlStreamWriter & aWriter, const TPtree & aNode);

	/// Данные ini-файлов попадают в секцию с именем файла.
	bool readINI(const QString & aFileName, TPtree & aTree);
	bool writeINI(const QString & aFileName, const TPtree & aTree);

	/// Делает резервную копию файла.
	void createBackup(const QString & aFilePath);

	TPtree mProperties;

	QString mConfigPath;
	QList<SSettingsSource> mSettingSources;
};

//---------------------------------------------------------------------------
