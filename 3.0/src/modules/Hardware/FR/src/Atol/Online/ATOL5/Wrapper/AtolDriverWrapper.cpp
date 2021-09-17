/* @file Обертка над оберткой АТОЛа над драйвером библиотеки fptr. */

// STL
#include <time.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDate>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/CodecDescriptions.h"

// Project
#include "AtolDriverWrapper.h"

//--------------------------------------------------------------------------------
template bool AtolDriverWrapper::checkSetting(const std::wstring & aKey, const QString & aValue);
template bool AtolDriverWrapper::checkSetting(const std::wstring & aKey, const int & aValue);
template bool AtolDriverWrapper::checkSetting(const std::wstring & aKey, const bool & aValue);

//--------------------------------------------------------------------------------
AtolDriverWrapper::AtolDriverWrapper(): mFRMethodParameters(2)
{
}

//--------------------------------------------------------------------------------
bool AtolDriverWrapper::initialize(const QString & aLibraryPath)
{
	QString libraryPath = cleanPath(aLibraryPath, true);

	if (!checkPath(libraryPath, "library"))
	{
		return false;
	}

	try
	{
		if (mDriver.isNull())
		{
			mDriver = PDriver(new Atol::Fptr::Fptr(libraryPath.toStdWString()));
		}
	}
	catch (std::exception & e)
	{
		toLog(LogLevel::Error, QString("Failed to initialize fptr library due to error: ") + e.what());
	}

	if (mDriver.isNull())
	{
		toLog(LogLevel::Error, "Failed to initialize fptr library due driver handle is null");
		return false;
	}

	return true;
}

//--------------------------------------------------------------------------------
bool AtolDriverWrapper::initializeLogData(const QString & aUserPath, const QString & aLogPath, bool aDebug)
{
	QString userPath = cleanPath(aUserPath, true);

	if (!checkPath(userPath, "user", CAtol5Logs::LPTFileName))
	{
		return false;
	}

	// LPT == log properties template
	QFile LPT(userPath + CAtol5Logs::LPTFileName);

	if (!LPT.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		toLog(LogLevel::Error, QString("Failed to initialize due to cannot open %1").arg(CAtol5Logs::LPTFileName));
		return false;
	}

	QTextStream LPTStream(&LPT);
	LPTStream.setCodec(CodecByName[CHardware::Codepages::UTF8]);
	QString LPTData = LPTStream.readAll();
	LPT.close();

	QString logPath = cleanPath(aLogPath, false);

	if (!checkPath(logPath, "log"))
	{
		return false;
	}

	QString dateData = QDate::currentDate().toString(CAtol5Logs::FileDateFormat);

	LPTData.replace(CAtol5Logs::Tags::Type, CAtol5Logs::Types::Info);
	LPTData.replace(CAtol5Logs::Tags::Path, logPath);
	LPTData.replace(CAtol5Logs::Tags::Date, dateData);

	// LP == log properties
	QString LPFileName = userPath + CAtol5Logs::LPFileName;

	if (!qputenv(CAtol5Logs::DTOLogEV, LPFileName.replace("/", "\\").toLatin1()))
	{
		toLog(LogLevel::Error, QString("Failed to set system environment variable %1").arg(CAtol5Logs::DTOLogEV));
		return false;
	}

	QFile LP(LPFileName);

	if (!LP.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		toLog(LogLevel::Error, QString("Failed to initialize due to cannot open %1").arg(CAtol5Logs::LPFileName));
		return false;
	}

	LPTStream.setDevice(&LP);
	LPTStream << LPTData;
	LP.close();

	return true;
}

//--------------------------------------------------------------------------------
void AtolDriverWrapper::release()
{
	if (!mDriver.isNull())
	{
		mDriver.clear();
	}
}

//--------------------------------------------------------------------------------
Atol::Fptr::Fptr * const AtolDriverWrapper::operator()(bool aEnqueue)
{
	if (aEnqueue)
	{
		mFRMethodParameters.enqueue(CAtol5OnlineFR::TMethodParameters());
	}

	return mDriver.data();
}

//--------------------------------------------------------------------------------
bool AtolDriverWrapper::isInitialized()
{
	return !mDriver.isNull();
}

//--------------------------------------------------------------------------------
bool AtolDriverWrapper::isReady()
{
	return !mDriver.isNull() && mDriver->isOpened();
}

//--------------------------------------------------------------------------------
bool AtolDriverWrapper::connect()
{
	if (!isInitialized())
	{
		return false;
	}
	else if (mDriver->isOpened())
	{
		toLog(LogLevel::Debug, "Device is already opened");
		return true;
	}

	bool result = 0;

	try
	{
		result = !mDriver->open();
	}
	catch (std::exception & e)
	{
		toLog(LogLevel::Error, QString("Failed to connect to cash register due to error: ") + e.what());
		return false;
	}

	if (!result)
	{
		return logLastError("open the device");
	}
	else if (!mDriver->isOpened())
	{
		toLog(LogLevel::Error, "Connection is OK, but device is not opened");
		return false;
	}

	toLog(LogLevel::Normal, "ATOL5 driver is opened");

	return true;
}

//--------------------------------------------------------------------------------
bool AtolDriverWrapper::disconnect()
{
	if (!isInitialized())
	{
		return true;
	}
	else if (!mDriver->isOpened())
	{
		toLog(LogLevel::Debug, "Device is already closed");
		return true;
	}

	bool result = 0;

	try
	{
		result = !mDriver->close();
	}
	catch (std::exception & e)
	{
		toLog(LogLevel::Error, QString("Failed to disconnect cash register due to error: ") + e.what());
		return false;
	}

	if (!result)
	{
		return logLastError("close the device");
	}
	else if (mDriver->isOpened())
	{
		toLog(LogLevel::Error, "Disconnection is OK, but the device is still opened");
		return false;
	}

	toLog(LogLevel::Normal, "ATOL5 driver is closed");

	return true;
}

//--------------------------------------------------------------------------------
template <class T>
void AtolDriverWrapper::setSetting(const std::wstring & aKey, const T & /*aValue*/)
{
	toLog(LogLevel::Error, QString("Unknown type for setting single driver parameter %1").arg(QString::fromStdWString(aKey)));
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setSetting<QString>(const std::wstring & aKey, const QString & aValue)
{
	if (!mDriver.isNull())
	{
		mDriver->setSingleSetting(aKey, aValue.toStdWString().data());
	}
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setSetting<int>(const std::wstring & aKey, const int & aValue)
{
	if (!mDriver.isNull())
	{
		mDriver->setSingleSetting(aKey, std::to_wstring((long long)aValue));
	}
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setSetting<bool>(const std::wstring & aKey, const bool & aValue)
{
	if (!mDriver.isNull())
	{
		mDriver->setSingleSetting(aKey, std::to_wstring((long long)aValue));
	}
}

//--------------------------------------------------------------------------------
template <class T>
T AtolDriverWrapper::getSetting(const std::wstring & aKey)
{
	toLog(LogLevel::Error, QString("Unknown type for getting single driver parameter %1").arg(QString::fromStdWString(aKey)));
}

//--------------------------------------------------------------------------------
template <>
QString AtolDriverWrapper::getSetting<QString>(const std::wstring & aKey)
{
	if (mDriver.isNull())
	{
		return false;
	}

	return QString::fromStdWString(mDriver->getSingleSetting(aKey));
}

//--------------------------------------------------------------------------------
template <>
int AtolDriverWrapper::getSetting<int>(const std::wstring & aKey)
{
	QString data = getSetting<QString>(aKey).simplified();
	bool OK;
	int result = data.toInt(&OK);

	if (!OK)
	{
		toLog(LogLevel::Error, QString("Failed to parse single setting %1 for key %2").arg(data).arg(QString::fromStdWString(aKey)));
		return 0;
	}

	return result;
}

//--------------------------------------------------------------------------------
template <>
bool AtolDriverWrapper::getSetting<bool>(const std::wstring & aKey)
{
	return bool(getSetting<int>(aKey));
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolDriverWrapper::checkSetting(const std::wstring & aKey, const T & aValue)
{
	T value = getSetting<T>(aKey);
	bool result = value != aValue;

	if (result)
	{
		setSetting<T>(aKey, aValue);
	}

	return result;
}

//--------------------------------------------------------------------------------
template <class T>
void AtolDriverWrapper::setDriverParameter(int aKey, const T & /*aValue*/)
{
	toLog(LogLevel::Error, QString("Unknown type for setting driver parameter %1").arg(aKey));
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setDriverParameter<int>(int aKey, const int & aValue)
{
	mFRMethodParameters.add(aKey, aValue);

	if (isReady())
	{
		mDriver->setParam(aKey, aValue);
	}
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setDriverParameter<uint>(int aKey, const uint & aValue)
{
	mFRMethodParameters.add(aKey, aValue);

	if (isReady())
	{
		mDriver->setParam(aKey, aValue);
	}
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setDriverParameter<bool>(int aKey, const bool & aValue)
{
	mFRMethodParameters.add(aKey, aValue);

	if (isReady())
	{
		mDriver->setParam(aKey, aValue);
	}
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setDriverParameter<double>(int aKey, const double & aValue)
{
	mFRMethodParameters.add(aKey, aValue);

	if (isReady())
	{
		mDriver->setParam(aKey, aValue);
	}
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setDriverParameter<QString>(int aKey, const QString & aValue)
{
	mFRMethodParameters.add(aKey, aValue);

	if (isReady())
	{
		mDriver->setParam(aKey, aValue.toStdWString());
	}
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setDriverParameter<QByteArray>(int aKey, const QByteArray & aValue)
{
	mFRMethodParameters.add(aKey, aValue);

	if (isReady())
	{
		mDriver->setParam(aKey, (uchar *) aValue.data(), aValue.size());
	}
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setDriverParameter<QDateTime>(int aKey, const QDateTime & aValue)
{
	mFRMethodParameters.add(aKey, aValue);

	if (!isReady())
	{
		return;
	}

	QDate date = aValue.date();
	QTime time = aValue.time();

	std::tm data;
	data.tm_year = date.year()  - 1900;
	data.tm_mon  = date.month() - 1;
	data.tm_mday = date.day();

	data.tm_hour = time.hour();
	data.tm_min  = time.minute();
	data.tm_sec  = time.second();

	mDriver->setParam(aKey, data);
}

//--------------------------------------------------------------------------------
template <>
void AtolDriverWrapper::setDriverParameter(int aKey, const QVariant & aValue)
{
	mFRMethodParameters.add(aKey, aValue);

	if (!aValue.isValid())
	{
		toLog(LogLevel::Error, QString("Failed to set driver parameter %1 due to it is not valid").arg(aKey));
		return;
	}

	QVariant::Type type = aValue.type();

	     if  (type == QVariant::String)     setDriverParameter<QString>   (aKey, aValue.toString());
	else if  (type == QVariant::ByteArray)  setDriverParameter<QByteArray>(aKey, aValue.toByteArray());
	else if  (type == QVariant::DateTime)   setDriverParameter<QDateTime> (aKey, aValue.toDateTime());
	else if  (type == QVariant::Double)     setDriverParameter<double>    (aKey, aValue.toDouble());
	else if  (type == QVariant::Bool)       setDriverParameter<bool>      (aKey, aValue.toBool());
	else if ((type == QVariant::Int) ||
	         (type == QVariant::LongLong))  setDriverParameter<int>       (aKey, aValue.toInt());
	else if ((type == QVariant::UInt) ||
	         (type == QVariant::ULongLong)) setDriverParameter<uint>      (aKey, aValue.toUInt());
	else
	{
		toLog(LogLevel::Error, QString("Failed to set driver parameter %1 due to unknown type %2").arg(aKey).arg(type));
	}
}

//--------------------------------------------------------------------------------
template <class T>
T AtolDriverWrapper::getDriverParameter(int aKey)
{
	toLog(LogLevel::Error, QString("Unknown type for getting driver parameter %1").arg(aKey));
	return T();
}

//--------------------------------------------------------------------------------
template <>
uint AtolDriverWrapper::getDriverParameter<uint>(int aKey)
{
	return !isReady() ? 0 : mDriver->getParamInt(aKey);
}

//--------------------------------------------------------------------------------
template <>
int AtolDriverWrapper::getDriverParameter<int>(int aKey)
{
	return !isReady() ? 0 : int(mDriver->getParamInt(aKey));
}

//--------------------------------------------------------------------------------
template <>
bool AtolDriverWrapper::getDriverParameter<bool>(int aKey)
{
	return !isReady() ? false : mDriver->getParamBool(aKey);
}

//--------------------------------------------------------------------------------
template <>
double AtolDriverWrapper::getDriverParameter<double>(int aKey)
{
	return !isReady() ? 0 : mDriver->getParamDouble(aKey);
}

//--------------------------------------------------------------------------------
template <>
QString AtolDriverWrapper::getDriverParameter<QString>(int aKey)
{
	return !isReady() ? "" : QString::fromStdWString(mDriver->getParamString(aKey));
}

//--------------------------------------------------------------------------------
template <>
QByteArray AtolDriverWrapper::getDriverParameter<QByteArray>(int aKey)
{
	if (!isReady())
	{
		return "";
	}

	auto data = mDriver->getParamByteArray(aKey);

	return QByteArray((const char *) data.data(), data.size());
}

//--------------------------------------------------------------------------------
template <>
QDateTime AtolDriverWrapper::getDriverParameter<QDateTime>(int aKey)
{
	if (!isReady())
	{
		return QDateTime();
	}

	std::tm data = mDriver->getParamDateTime(aKey);

	return QDateTime(QDate(1900 + data.tm_year, 1 + data.tm_mon, data.tm_mday), QTime(data.tm_hour, data.tm_min, data.tm_sec));
}

//--------------------------------------------------------------------------------
void AtolDriverWrapper::setFRMethodParameters(const CAtol5OnlineFR::TMethodParameters & aParameters)
{
	for (auto it = aParameters.begin(); it != aParameters.end(); ++it)
	{
		setDriverParameter<QVariant>(it.key(), it.value());
	}
}

//--------------------------------------------------------------------------------
CAtol5OnlineFR::TMethodParameters AtolDriverWrapper::getFRMethodParameters()
{
	return mFRMethodParameters.head();
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolDriverWrapper::setFRParameter(int aKey, const T & aValue)
{
	toLog(LogLevel::Error, QString("Unknown type for setting parameter %1").arg(aKey));
	return false;
}

//--------------------------------------------------------------------------------
template <>
bool AtolDriverWrapper::setFRParameter<QString>(int aKey, const QString & aValue)
{
	QString log = QString("set parameter %1 = %2").arg(aKey).arg(aValue);

	if (!isReady())
	{
		toLog(LogLevel::Error, "Failed to " + log + " due to driver is not ready");
		return false;
	}

	mDriver->setParam(LIBFPTR_PARAM_SETTING_ID, aKey);
	mDriver->setParam(LIBFPTR_PARAM_SETTING_VALUE, aValue.toStdWString());

	return !mDriver->writeDeviceSetting() || logLastError(log);
}

//--------------------------------------------------------------------------------
template <>
bool AtolDriverWrapper::setFRParameter<int>(int aKey, const int & aValue)
{
	QString log = QString("set parameter %1 = %2").arg(aKey).arg(aValue);

	if (!isReady())
	{
		toLog(LogLevel::Error, "Failed to " + log + " due to driver is not ready");
		return false;
	}

	mDriver->setParam(LIBFPTR_PARAM_SETTING_ID, aKey);
	mDriver->setParam(LIBFPTR_PARAM_SETTING_VALUE, aValue);

	return !mDriver->writeDeviceSetting() || logLastError(log);
}

//--------------------------------------------------------------------------------
template <>
bool AtolDriverWrapper::setFRParameter<bool>(int aKey, const bool & aValue)
{
	return setFRParameter<int>(aKey, int(aValue));
}

//--------------------------------------------------------------------------------
template <class T>
bool AtolDriverWrapper::getFRParameter(int aKey, T & aValue)
{
	toLog(LogLevel::Error, QString("Unknown type for getting parameter %1").arg(aKey));
	return false;
}

//--------------------------------------------------------------------------------
template <>
bool AtolDriverWrapper::getFRParameter<QString>(int aKey, QString & aValue)
{
	QString log = QString("get parameter %1").arg(aKey);

	if (!isReady())
	{
		toLog(LogLevel::Error, "Failed to " + log + " due to driver is not ready");
		return false;
	}

	mDriver->setParam(LIBFPTR_PARAM_SETTING_ID, aKey);

	if (mDriver->readDeviceSetting())
	{
		return logLastError(log);
	}

	aValue = QString::fromStdWString(mDriver->getParamString(LIBFPTR_PARAM_SETTING_VALUE));

	return true;
}

//--------------------------------------------------------------------------------
template <>
bool AtolDriverWrapper::getFRParameter<int>(int aKey, int & aValue)
{
	QString log = QString("get parameter %1").arg(aKey);

	if (!isReady())
	{
		toLog(LogLevel::Error, "Failed to " + log + " due to driver is not ready");
		return false;
	}

	mDriver->setParam(LIBFPTR_PARAM_SETTING_ID, aKey);

	if (mDriver->readDeviceSetting())
	{
		return logLastError(log);
	}

	aValue = int(mDriver->getParamInt(LIBFPTR_PARAM_SETTING_VALUE));

	return true;
}

//--------------------------------------------------------------------------------
bool AtolDriverWrapper::logLastError(const QString & aLog)
{
	QString log = "Failed to " + aLog;

	if (isInitialized())
	{
		log += QString(" due to error %1 (%2)").arg(mDriver->errorCode()).arg(QString::fromStdWString(mDriver->errorDescription()));
	}

	toLog(LogLevel::Error, log);

	return false;
}

//--------------------------------------------------------------------------------
QString AtolDriverWrapper::cleanPath(const QString & aPath, bool aSeparator) const
{
	QString path = QDir::cleanPath(QDir::fromNativeSeparators(aPath.simplified()));

	if (path.isEmpty() || (path.endsWith("/") == aSeparator))
	{
		return path;
	}
	else if (aSeparator)
	{
		return path + "/";
	}

	path.chop(1);

	return path;
};

//--------------------------------------------------------------------------------
bool AtolDriverWrapper::checkPath(const QString & aPath, const QString & aLog, const QString & aFileName) const
{
	if (aPath.isEmpty())
	{
		toLog(LogLevel::Error, QString("Failed to initialize due to %1 path is empty").arg(aLog));
		return false;
	}

	if (!QDir(aPath).exists())
	{
		toLog(LogLevel::Error, QString("Failed to initialize due to no %1 path %2").arg(aLog).arg(aPath));
		return false;
	}

	QString path = aPath + aFileName;
	QFile file(path);

	if (!aFileName.isEmpty() && !file.exists())
	{
		toLog(LogLevel::Error, QString("Failed to initialize due to no %1 on %2 path %3").arg(aFileName).arg(aLog).arg(aPath));
		return false;
	}

	return true;
};

//--------------------------------------------------------------------------------
