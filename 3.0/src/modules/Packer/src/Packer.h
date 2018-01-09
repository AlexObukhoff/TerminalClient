/* @file Класс для архивации папок и файлов. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/ILogable.h>

//------------------------------------------------------------------------------
class Packer : public ILogable
{
	int mExitCode;

	QString mMessages;

public:
	typedef enum 
	{
		Zip,
		SevenZip
	} Format;

public:
	Packer(const QString & aToolPath, ILog * aLog);

	/// Задать расположение архиватора 7zip
	void setToolPath(const QString & aToolPath);

	/// Включить режим обновления архива
	void setUpdateMode(bool aUpdateMode);

	/// Установка уровня сжатия
	void setLevel(int aLevel);

	/// Установить формат архива
	void setFormat(Format aFormat);

	/// Установить максимальный таймаут ожидания выполнения архивации
	void setTimeout(int aTimeout);

	/// Установить рекурсивный режим архивации
	void setRecursive(bool aRecursive);

public:
	/// Архивирует буфер в пямяти в GZ формат
	static QByteArray compressToGZ(const QByteArray & aInBuffer, const QString & aFileName, int aLevel = 9);

	/// Архивирует файлы в один том. В случае успеха возвращает имя сформированного архива.
	QString compress(const QString & aTargetName, const QString & aSourceDir, const QStringList & aSearchMasks, const QStringList & aExcludeWildcard);

	/// Архивириут файлы в несколько томов, размер которых ограничивается aMaxPartSize в байтах.
	QStringList compress(const QString & aTargetName, const QString & aSourceDir, const QStringList & aSearchMasks, const QStringList & aExcludeWildcard, int aMaxPartSize);

	/// Протестировать архив
	bool test(const QString & aTargetName);

	/// Распаковать определенные файлы из архива в определенную папку
	bool extract(const QString & aSourceName, const QString & aDestinationDir, bool aSkipExisting, const QStringList & aExtractFiles = QStringList());

	/// Экстренно прервать процесс архиватора, может быть вызван только из соседнего потока
	void terminate();

public:
	/// Возвращает exit код последней операции
	int exitCode() const;

	/// Возвращает текст описание кода возврата
	QString exitCodeDescription() const;

	/// Возвращает текст полученный от архиватора
	const QString & messages() const;

private:
	QString mToolPath;
	bool mUpdateMode;
	Format mFormat;
	int mLevel;
	bool mRecursive;
	int mTimeout;
	QProcess mZipProcess;
};

//------------------------------------------------------------------------------
