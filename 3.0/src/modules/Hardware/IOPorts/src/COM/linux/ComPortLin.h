#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

#include "Devices/Ports/ComPort/ComPortBase.h"

class SerialDevice;

//-----------------------------------------------------------------------------
namespace CComPortLin
{
const QString LogName = "ComPortLin";
};

//-----------------------------------------------------------------------------
class ComPortLin : public ComPortBase
{
public:
    ComPortLin(const QString & aFilePath);
	virtual ~ComPortLin();
    
	virtual bool open();
	virtual bool init();
	virtual bool release();
	virtual bool clear();
	virtual bool readData(QByteArray &aData, unsigned int aMaxSize, bool aIsTimeOut, bool aIsFirstData);
	virtual int writeData(const QByteArray& aData);
	virtual bool setBaudRate(PortParameters::BaudRate::Enum aBaudRate);
	virtual bool setStopBits(PortParameters::StopBits::Enum aStopBits);
	virtual bool setParity(PortParameters::Parity::Enum aParity);
	virtual bool setDTR(PortParameters::DTR::Enum aDTR);
	virtual bool setRTS(PortParameters::RTS::Enum aRTS);
	virtual void setTimeOut(int aMsecs);

private:
    QSharedPointer<SerialDevice> m_device;
    QString m_deviceName;
};

//-----------------------------------------------------------------------------
