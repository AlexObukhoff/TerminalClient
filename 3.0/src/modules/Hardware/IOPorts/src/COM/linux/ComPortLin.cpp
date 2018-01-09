#include "ComPortLin.h"

#include "SerialDevice.h"

//-----------------------------------------------------------------------------
ComPortLin::ComPortLin(const QString & aFilePath)
    : m_device(new SerialDevice(aFilePath))
    , m_deviceName(aFilePath)
{
    m_device->open();
}

//-----------------------------------------------------------------------------
ComPortLin::~ComPortLin()
{
    release();
}

//-----------------------------------------------------------------------------
bool ComPortLin::open()
{
    bool status = true;
    
    if (!m_device->isOpen())
    {
        status = m_device->open(SerialDevice::ReadWrite);

        if (status)
        {
            LOG_WRITE(CComPortLin::LogName, LogLevel::Normal, "Port " + m_deviceName + " is successfully opened");
        }
        else
        {
            LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "Port " + m_deviceName + ", ComPortLin::open(): failed because SerialDevice::open() failed");
        }

        m_isPortOpen = status;
    }
    
    return status;
}

//-----------------------------------------------------------------------------
bool ComPortLin::init()
{
    return m_device->init();
}

//-----------------------------------------------------------------------------
bool ComPortLin::release()
{
    if (m_device->isOpen())
    {
        m_device->close();
        LOG_WRITE(CComPortLin::LogName, LogLevel::Normal, "Port " + m_deviceName + " is successfully closed");
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
bool ComPortLin::clear()
{
    if (m_device->isOpen())
    {
        m_device->readAll();
        return true;
    }
    return false;
}

//-----------------------------------------------------------------------------
bool ComPortLin::readData(QByteArray & aData, unsigned int aMaxSize, bool aIsTimeOut, bool aIsFirstData)
{
    Q_UNUSED(aIsFirstData);
    
    const ulong oldTimeout = m_device->getTimeout();
    if (!aIsTimeOut)
    {
        m_device->setTimeout(0);
    }

    aData = m_device->read(aMaxSize);

    if (!aIsTimeOut)
    {
        m_device->setTimeout(oldTimeout);
    }

    return true;
}

//-----------------------------------------------------------------------------
int ComPortLin::writeData(const QByteArray& aData)
{
    return m_device->write(aData);
}

//-----------------------------------------------------------------------------
bool ComPortLin::setBaudRate(PortParameters::BaudRate::Enum aBaudRate)
{
    CBaudRateType::Enum baudRate = CBaudRateType::Baud115200;
    switch (aBaudRate)
    {
    case PortParameters::BaudRate::BR110:
        baudRate = CBaudRateType::Baud110;
        break;
    case PortParameters::BaudRate::BR300:
        baudRate = CBaudRateType::Baud300;
        break;
    case PortParameters::BaudRate::BR600:
        baudRate = CBaudRateType::Baud600;
        break;
    case PortParameters::BaudRate::BR1200:
        baudRate = CBaudRateType::Baud1200;
        break;
    case PortParameters::BaudRate::BR2400:
        baudRate = CBaudRateType::Baud2400;
        break;
    case PortParameters::BaudRate::BR4800:
        baudRate = CBaudRateType::Baud4800;
        break;
    case PortParameters::BaudRate::BR9600:
    case PortParameters::BaudRate::BR14400:
        baudRate = CBaudRateType::Baud9600;
        break;
    case PortParameters::BaudRate::BR19200:
        baudRate = CBaudRateType::Baud19200;
        break;
    case PortParameters::BaudRate::BR38400:
    case PortParameters::BaudRate::BR56000:
        baudRate = CBaudRateType::Baud38400;
        break;
    case PortParameters::BaudRate::BR57600:
        baudRate = CBaudRateType::Baud57600;
        break;
    case PortParameters::BaudRate::BR115200:
    case PortParameters::BaudRate::BR128000:
        baudRate = CBaudRateType::Baud115200;
        break;
    case PortParameters::BaudRate::BR256000:
        baudRate = CBaudRateType::Baud230400;
        break;
    default:
        LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "Port " + m_deviceName +
                  ", ComPortLin::setBaudRate(): invalud value");
        return false;
    };

    if (!m_device->setBaudRate(baudRate))
    {
        LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "Port " + m_deviceName +
                  ", ComPortLin::setBaudRate(): failed because SerialDevice::setBaudRate() failed");
        return false;
    }
    else
    {
        m_COMParameters.baudRate = aBaudRate;
        
        if (!m_registryParameters)
        {
            changeRegistryParameters(m_COMParameters);
        }
        else
        {
            QString root = CRegistry::Keys::Root;
            if (!m_registryParameters->setValue(root, CRegistry::Values::Ports::BaudRate, 
                                                PortParameters::BaudRate::toString(m_COMParameters.baudRate)))
            {
                saveLastError("Error set value : " + root + "/" + CRegistry::Values::Ports::BaudRate);
                LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "COM" + QString::number(m_COMParameters.portNumber) + 
                          ", ComPortLin::setBaundRate(): IRegistry::setValue() returned error");
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
bool ComPortLin::setStopBits(PortParameters::StopBits::Enum aStopBits)
{
    CStopBitsType::Enum stopBits = CStopBitsType::Stop1;
    switch (aStopBits)
    {
    case PortParameters::StopBits::One:
        stopBits = CStopBitsType::Stop1;
        break;
    case PortParameters::StopBits::Two:
        stopBits = CStopBitsType::Stop2;
        break;
    default:
        LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "Port " + m_deviceName +
                  ", ComPortLin::setStopBits(): invalud stop bits value");
        return false;
    };

    if (!m_device->setStopBits(stopBits))
    {
        LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "Port " + m_deviceName +
                  ", ComPortLin::setStopBits(): failted because SerialDevice::setStopBits() failed");
        return false;
    }
    else
    {
        m_COMParameters.stopBits = aStopBits;
        
        if (!m_registryParameters)
            changeRegistryParameters(m_COMParameters);
        else
        {
            QString root = CRegistry::Keys::Root;
            if (!m_registryParameters->setValue(root, CRegistry::Values::Ports::StopBits, 
                                                PortParameters::StopBits::toString(m_COMParameters.stopBits)))
            {
                saveLastError("Error set value : " + root + "/" + CRegistry::Values::Ports::StopBits);
                LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "COM" + QString::number(m_COMParameters.portNumber) + 
                          ", ComPortLin::setStopBits(): IRegistry::setValue() returned error");
            }
        }
    }
        
    return true;
}

//-----------------------------------------------------------------------------
bool ComPortLin::setParity(PortParameters::Parity::Enum aParity)
{
    CParityType::Enum parity = CParityType::None;
    switch (aParity)
    {
    case PortParameters::Parity::PEven:
        parity = CParityType::Even;
        break;
    case PortParameters::Parity::PNo:
        parity = CParityType::None;
        break;
    case PortParameters::Parity::POdd:
        parity = CParityType::Odd;
        break;
    case PortParameters::Parity::PSpace:
        parity = CParityType::Space;
        break;
    default:
        LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "Port " + m_deviceName +
                  ", ComPortLin::setParity(): failed - invalid parity value");
        return false;
    }
    
    if (!m_device->setParity(parity))
    {
        LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "Port " + m_deviceName +
                  ", ComPortLin::setParity(): failted because SerialDevice::setParity() failed");
        return false;
    }
    else
    {
        m_COMParameters.parity = aParity;

        if (!m_registryParameters)
        {
            changeRegistryParameters(m_COMParameters);
        }
        else
        {
            QString root = CRegistry::Keys::Root;
            if (!m_registryParameters->setValue(root, CRegistry::Values::Ports::Parity, 
                                                PortParameters::Parity::toString(m_COMParameters.parity)))
            {
                saveLastError("Error set value : " + root + "/" + CRegistry::Values::Ports::Parity);
                LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "COM" + QString::number(m_COMParameters.portNumber) + 
                          ", ComPortLin::setParity(): IRegistry::setValue() returned error");
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
bool ComPortLin::setDTR(PortParameters::DTR::Enum aDTR)
{
    m_device->setDtr(!(aDTR == PortParameters::DTR::Disable));
    m_COMParameters.dtrControl = aDTR;

    if (!m_registryParameters)
    {
		changeRegistryParameters(m_COMParameters);
    }
	else
	{
		QString root = CRegistry::Keys::Root;
		if (!m_registryParameters->setValue(root, CRegistry::Values::Ports::DtrControl,
                                            PortParameters::DTR::toString(m_COMParameters.dtrControl)))
		{
			saveLastError("Error set value : " + root + "/" + CRegistry::Values::Ports::DtrControl);
			LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "COM" + QString::number(m_COMParameters.portNumber) + 
                      ", ComPortLin::setDTR(): IRegistry::setValue() returned error");
		}
	}
    
    return true;
}

//-----------------------------------------------------------------------------
bool ComPortLin::setRTS(PortParameters::RTS::Enum aRTS)   
{
    m_device->setRts(!(aRTS == PortParameters::RTS::Disable));
    m_COMParameters.rtsControl = aRTS;

    if (!m_registryParameters)
    {
		changeRegistryParameters(m_COMParameters);
    }
	else
	{
		QString root = CRegistry::Keys::Root;
		if (!m_registryParameters->setValue(root, CRegistry::Values::Ports::RtsControl, 
                                            PortParameters::RTS::toString(m_COMParameters.rtsControl)))
		{
			saveLastError("Error set value : " + root + "/" + CRegistry::Values::Ports::RtsControl);
			LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "COM" + QString::number(m_COMParameters.portNumber) + 
                      ", ComPortLin::initRTS(): IRegistry::setValue() returned error");
		}
	}
    
    return true;
}

//-----------------------------------------------------------------------------
void ComPortLin::setTimeOut(int aMsecs)
{
    if (!m_device->setTimeout(aMsecs))
    {
        LOG_WRITE(CComPortLin::LogName, LogLevel::Error, "Port " + m_deviceName +
                  ", ComPortLin::setTimeOut(): failted because SerialDevice::setTimeout() failed");
    }
}

//-----------------------------------------------------------------------------
