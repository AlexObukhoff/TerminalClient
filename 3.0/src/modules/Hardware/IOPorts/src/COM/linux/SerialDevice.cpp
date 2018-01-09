#include "SerialDevice.h"

#include <termios.h>

//-----------------------------------------------------------------------------
SerialDeviceSettings::SerialDeviceSettings()
    : baudRate(CBaudRateType::Baud115200)
    , dataBits(CDataBitsType::Data8)
    , parity(CParityType::None)
    , stopBits(CStopBitsType::Stop1)
    , flowControl(CFlowType::Hardware)
    , timeout(500)
{
}

//-----------------------------------------------------------------------------
SerialDevice::SerialDevice(const QString & aFilePath, QObject * aParent)
    : QIODevice(aParent)
    , m_file(aFilePath)
    , m_settings()
    , m_mutex(QMutex::Recursive)
{
    setOpenMode(QIODevice::NotOpen);
}

//-----------------------------------------------------------------------------
SerialDevice::~SerialDevice()
{
    if (isOpen())
    {
        close();
    }

    m_file.close();
}

//-----------------------------------------------------------------------------
bool SerialDevice::open(OpenMode aMode)
{
    QMutexLocker locker(&m_mutex);
    
    bool status = isOpen();
    if (!(status || aMode == NotOpen))
    {
        if (m_file.open(ReadWrite | Unbuffered))
        {
            QIODevice::open(aMode);
            status = isOpen();

            termios termiosConfig;
            tcgetattr(m_file.handle(), &termiosConfig);
            termiosConfig.c_cflag |= CREAD | CLOCAL;
            termiosConfig.c_lflag &= (~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG));
            termiosConfig.c_iflag &= (~(INPCK | IGNPAR | PARMRK | ISTRIP | ICRNL | IXANY));
            termiosConfig.c_oflag &= (~OPOST);
            termiosConfig.c_cc[VMIN] = 0;
            termiosConfig.c_cc[VINTR] = _POSIX_VDISABLE;
            termiosConfig.c_cc[VQUIT] = _POSIX_VDISABLE;
            termiosConfig.c_cc[VSTART] = _POSIX_VDISABLE;
            termiosConfig.c_cc[VSTOP] = _POSIX_VDISABLE;
            termiosConfig.c_cc[VSUSP] = _POSIX_VDISABLE;
            tcsetattr(m_file.handle(), TCSAFLUSH, &termiosConfig);
        }
    }

    return status;
}

//-----------------------------------------------------------------------------
bool SerialDevice::init()
{
    QMutexLocker locker(&m_mutex);

    bool response = true;
    response = response & doSetBaudRate(m_settings.baudRate);
    response = response & doSetDataBits(m_settings.dataBits);
    response = response & doSetParity(m_settings.parity);
    response = response & doSetStopBits(m_settings.stopBits);
    response = response & doSetFlowControl(m_settings.flowControl);
    response = response & doSetTimeout(m_settings.timeout);

    return response;
}

//-----------------------------------------------------------------------------
bool SerialDevice::isSequential() const
{
    return true;
}

//-----------------------------------------------------------------------------
void SerialDevice::close()
{
    QMutexLocker locker(&m_mutex);

    m_file.close();
    QIODevice::close();
}

//-----------------------------------------------------------------------------
void SerialDevice::flush()
{
    QMutexLocker locker(&m_mutex);

    if (isOpen())
    {
        m_file.flush();
    }
}

//-----------------------------------------------------------------------------
qint64 SerialDevice::size() const
{
    QMutexLocker locker(&m_mutex);
    
    int response = 0;
    if (ioctl(m_file.handle(), FIONREAD, &response) < 0)
    {
        response = 0;
    }

    return static_cast<qint64>(response);
}

//-----------------------------------------------------------------------------
bool SerialDevice::atEnd() const
{
    return !!size();
}

//-----------------------------------------------------------------------------
qint64 SerialDevice::bytesAvailable()
{
    QMutexLocker locker(&m_mutex);
    int response = 0;

    if (isOpen())
    {
        fd_set fileSet;
        FD_ZERO(&fileSet);
        FD_SET(m_file.handle(), &fileSet);

        // set timeout
        timeval timeout;
        memset(&timeout, 0, sizeof timeout);
        timeout.tv_sec = m_settings.timeout / 1000;
        timeout.tv_usec = (m_settings.timeout % 1000) * 1000;
        
        int n = select(m_file.handle() + 1, &fileSet, 0, &fileSet, &timeout);

        if (n == 0)
        {
            // by timeout
            response = -1;
        }
        else if (n == -1 || ioctl(m_file.handle(), FIONREAD, &response) == -1)
        {
            // some error
            response = -1;
        }
    }

    return static_cast<qint64>(response + QIODevice::bytesAvailable());
}

//-----------------------------------------------------------------------------
void SerialDevice::ungetChar(char aCh)
{
    Q_UNUSED(aCh);
}

//-----------------------------------------------------------------------------
qint64 SerialDevice::readLine(char * aData, qint64 aMaxSize)
{
    QMutexLocker locker(&m_mutex);
    
    qint64 bytes = bytesAvailable();
    char * pData = aData;

    if (aMaxSize < 2)
        return -1;

    while (pData < (aData + bytes) && --aMaxSize)
    {
        doReadData(pData, 1);
        if (*pData++ == '\n')
        {
            break;
        }
    }

    *pData = '\0';

    return pData - aData;
}

//-----------------------------------------------------------------------------
qint64 SerialDevice::doReadData(char * aData, qint64 aMaxSize)
{
    return m_file.read(aData, aMaxSize);
}

//-----------------------------------------------------------------------------
qint64 SerialDevice::readData(char * aData, qint64 aMaxSize)
{
    QMutexLocker locker(&m_mutex);
    return doReadData(aData, aMaxSize);
}

//-----------------------------------------------------------------------------
qint64 SerialDevice::writeData(const char * aData, qint64 aMaxSize)
{
    qint64 response = 0;
    do {
        QMutexLocker locker(&m_mutex);
        response = m_file.write(aData, aMaxSize);
    } while (0);
    
    flush();
    return response;
}

//-----------------------------------------------------------------------------
bool SerialDevice::doSetBaudRate(CBaudRateType::Enum aBaudRate)
{
    m_settings.baudRate = aBaudRate;
    
    if (!isOpen())
    {
        return false;
    }
    
    termios termiosConfig;
    tcgetattr(m_file.handle(), &termiosConfig);
        
    speed_t speed = 0;
    switch (aBaudRate)
    {
    case CBaudRateType::Baud50:
        speed = B50; break;
    case CBaudRateType::Baud75:
        speed = B75; break;
    case CBaudRateType::Baud110:
        speed = B110; break;
    case CBaudRateType::Baud134:
        speed = B134; break;
    case CBaudRateType::Baud150:
        speed = B150; break;
    case CBaudRateType::Baud200:
        speed = B200; break;
    case CBaudRateType::Baud300:
        speed = B300; break;
    case CBaudRateType::Baud600:
        speed = B600; break;
    case CBaudRateType::Baud1200:
        speed = B1200; break;
    case CBaudRateType::Baud1800:
        speed = B1800; break;
    case CBaudRateType::Baud2400:
        speed = B2400; break;
    case CBaudRateType::Baud4800:
        speed = B4800; break;
    case CBaudRateType::Baud9600:
        speed = B9600; break;
    case CBaudRateType::Baud19200:
        speed = B19200; break;
    case CBaudRateType::Baud38400:
        speed = B38400; break;
    case CBaudRateType::Baud57600:
        speed = B57600; break;
    // case CBaudRateType::Baud76800:
    //     speed = B76800; break;
    case CBaudRateType::Baud115200:
        speed = B115200; break;
    case CBaudRateType::Baud230400:
        speed = B230400; break;
    case CBaudRateType::Baud460800:
        speed = B460800; break;
    case CBaudRateType::Baud500000:
        speed = B500000; break;
    case CBaudRateType::Baud576000:
        speed = B576000; break;
    case CBaudRateType::Baud921600:
        speed = B921600; break;
    case CBaudRateType::Baud1000000:
        speed = B1000000; break;
    case CBaudRateType::Baud1152000:
        speed = B1152000; break;
    case CBaudRateType::Baud1500000:
        speed = B1500000; break;
    case CBaudRateType::Baud2000000:
        speed = B2000000; break;
    case CBaudRateType::Baud2500000:
        speed = B2500000; break;
    case CBaudRateType::Baud3000000:
        speed = B3000000; break;
    case CBaudRateType::Baud3500000:
        speed = B3500000; break;
    case CBaudRateType::Baud4000000:
        speed = B4000000; break;
    };

    cfsetispeed(&termiosConfig, speed);
    cfsetospeed(&termiosConfig, speed);
    tcsetattr(m_file.handle(), TCSAFLUSH, &termiosConfig);

    return true;
}

//-----------------------------------------------------------------------------
bool SerialDevice::doSetDataBits(CDataBitsType::Enum aDataBits)
{
    const bool status =
        (m_settings.stopBits == CStopBitsType::Stop2
         && aDataBits == CDataBitsType::Data5) ||
        (m_settings.parity == CParityType::Space
         && aDataBits == CDataBitsType::Data8);
    if (status)
    {
        return false;
    }
    else
    {
        m_settings.dataBits = aDataBits;
    }

    if (!isOpen())
    {
        return false;
    }

    unsigned flag = 0;
    switch (aDataBits)
    {
    case CDataBitsType::Data5:
        flag = CS5; break;
    case CDataBitsType::Data6:
        flag = CS6; break;
    case CDataBitsType::Data7:
        flag = CS7; break;
    case CDataBitsType::Data8:
        flag = CS8; break;
    };

    termios termiosConfig;
    tcgetattr(m_file.handle(), &termiosConfig);
    termiosConfig.c_cflag &= (~CSIZE);
    termiosConfig.c_cflag |= flag;
    tcsetattr(m_file.handle(), TCSAFLUSH, &termiosConfig);

    return true;
}

//-----------------------------------------------------------------------------
bool SerialDevice::doSetParity(CParityType::Enum aParity)
{
    const bool status =
        (aParity == CParityType::Space
         && m_settings.dataBits == CDataBitsType::Data8);
    
    if (status)
    {
        return false;
    }
    else
    {
        m_settings.parity = aParity;
    }

    if (!isOpen())
    {
        return false;
    }
        
    termios termiosConfig;
    tcgetattr(m_file.handle(), &termiosConfig);
    
    switch (aParity)
    {
    case CParityType::Space:
        termiosConfig.c_cflag &= (~PARENB | CSIZE);
        switch (m_settings.dataBits)
        {
        case CDataBitsType::Data5:
            m_settings.dataBits = CDataBitsType::Data6;
            termiosConfig.c_cflag |= CS6;
            break;
        case CDataBitsType::Data6:
            m_settings.dataBits = CDataBitsType::Data7;
            termiosConfig.c_cflag |= CS7;
            break;
        case CDataBitsType::Data7:
            m_settings.dataBits = CDataBitsType::Data8;
            termiosConfig.c_cflag |= CS8;
            break;
        case CDataBitsType::Data8:
            break;
        };
        break;
    case CParityType::None:
        termiosConfig.c_cflag &= (~PARENB);
        break;
    case CParityType::Even:
        termiosConfig.c_cflag &= (~PARODD);
        termiosConfig.c_cflag |= PARENB;
        break;
    case CParityType::Odd:
        termiosConfig.c_cflag |= (PARENB | PARODD);
        break;
    };

    tcsetattr(m_file.handle(), TCSAFLUSH, &termiosConfig);

    return true;
}

//-----------------------------------------------------------------------------
bool SerialDevice::doSetStopBits(CStopBitsType::Enum aStopBits)
{
    const bool status =
        (m_settings.dataBits == CDataBitsType::Data5
         && aStopBits == CStopBitsType::Stop2);
    
    if (status)
    {
        return false;
    }
    else
    {
        m_settings.stopBits = aStopBits;
    }

    if (!isOpen())
    {
        return false;
    }

    termios termiosConfig;
    tcgetattr(m_file.handle(), &termiosConfig);

    switch (aStopBits)
    {
    case CStopBitsType::Stop1:
        termiosConfig.c_cflag &= (~CSTOPB);
        break;
    case CStopBitsType::Stop2:
        termiosConfig.c_cflag |= CSTOPB;
        break;
    };

    tcsetattr(m_file.handle(), TCSAFLUSH, &termiosConfig);

    return true;
}

//-----------------------------------------------------------------------------
bool SerialDevice::doSetFlowControl(CFlowType::Enum aFlowControl)
{
    m_settings.flowControl = aFlowControl;

    if (!isOpen())
    {
        return false;
    }

    termios termiosConfig;
    tcgetattr(m_file.handle(), &termiosConfig);
    
    switch (aFlowControl)
    {
    case CFlowType::Off:
        termiosConfig.c_cflag &= (~CRTSCTS);
        termiosConfig.c_iflag &= (~(IXON | IXOFF | IXANY));
        break;
    case CFlowType::XOnXOff:
        termiosConfig.c_cflag &= (~CRTSCTS);
        termiosConfig.c_iflag |= (IXON|IXOFF|IXANY);
        break;
    case CFlowType::Hardware:
        termiosConfig.c_cflag |= CRTSCTS;
        termiosConfig.c_iflag &= (~(IXON|IXOFF|IXANY));
        break;
    }

    tcsetattr(m_file.handle(), TCSAFLUSH, &termiosConfig);

    return true;
}

//-----------------------------------------------------------------------------
bool SerialDevice::doSetTimeout(ulong aMsecs)
{
    m_settings.timeout = aMsecs;
    
    if (!isOpen())
    {
        return false;
    }
    
    termios termiosConfig;
    tcgetattr(m_file.handle(), &termiosConfig);
    termiosConfig.c_cc[VTIME] = aMsecs / 1000;
    tcsetattr(m_file.handle(), TCSAFLUSH, &termiosConfig);
    
    return true;
}

//-----------------------------------------------------------------------------
bool SerialDevice::setBaudRate(CBaudRateType::Enum aBaudRate)
{
    QMutexLocker locker(&m_mutex);

    bool response = true;
    if (m_settings.baudRate != aBaudRate)
    {
        response = doSetBaudRate(aBaudRate);
    }

    return response;
}

//-----------------------------------------------------------------------------
CBaudRateType::Enum SerialDevice::getBaudRate() const
{
    return m_settings.baudRate;
}

//-----------------------------------------------------------------------------
bool SerialDevice::setDataBits(CDataBitsType::Enum aDataBits)
{
    QMutexLocker locker(&m_mutex);

    bool response = true;
    if (m_settings.dataBits != aDataBits)
    {
        response = doSetDataBits(aDataBits);
    }

    return response;
}

//-----------------------------------------------------------------------------
CDataBitsType::Enum SerialDevice::getDataBits() const
{
    return m_settings.dataBits;
}

//-----------------------------------------------------------------------------
bool SerialDevice::setParity(CParityType::Enum aParity)
{
    QMutexLocker locker(&m_mutex);

    bool response = true;
    if (m_settings.parity != aParity)
    {
        response = doSetParity(aParity);
    }

    return response;
}

//-----------------------------------------------------------------------------
CParityType::Enum SerialDevice::getParity() const
{
    return m_settings.parity;
}

//-----------------------------------------------------------------------------
bool SerialDevice::setStopBits(CStopBitsType::Enum aStopBits)
{
    QMutexLocker locker(&m_mutex);

    bool response = true;
    if (m_settings.stopBits != aStopBits)
    {
        response = doSetStopBits(aStopBits);
    }

    return response;
}

//-----------------------------------------------------------------------------
CStopBitsType::Enum SerialDevice::getStopBits() const
{
    return m_settings.stopBits;
}

//-----------------------------------------------------------------------------
bool SerialDevice::setFlowControl(CFlowType::Enum aFlowControl)
{
    QMutexLocker locker(&m_mutex);

    bool response = true;
    if (m_settings.flowControl != aFlowControl)
    {
        response = doSetFlowControl(aFlowControl);
    }
    
    return response;
}

//-----------------------------------------------------------------------------
CFlowType::Enum SerialDevice::getFlowControl() const
{
    return m_settings.flowControl;
}

//-----------------------------------------------------------------------------
ulong SerialDevice::getTimeout() const
{
    return m_settings.timeout;
}

//-----------------------------------------------------------------------------
bool SerialDevice::setTimeout(ulong aMsecs)
{
    QMutexLocker locker(&m_mutex);

    bool response = true;
    if (m_settings.timeout != aMsecs)
    {
        response = doSetTimeout(aMsecs);
    }

    return response;
}

//-----------------------------------------------------------------------------
void SerialDevice::setDtr(bool aSet)
{
    QMutexLocker locker(&m_mutex);

    if (isOpen())
    {
        int status = 0;
        ioctl(m_file.handle(), TIOCMGET, &status);
        aSet ? status |= TIOCM_DTR : status &= ~TIOCM_DTR;
        ioctl(m_file.handle(), TIOCMSET, &status);
    }
}

//-----------------------------------------------------------------------------
void SerialDevice::setRts(bool aSet)
{
    QMutexLocker locker(&m_mutex);

    if (isOpen())
    {
        int status = 0;
        ioctl(m_file.handle(), TIOCMGET, &status);
        aSet ? status |= TIOCM_RTS : status &= ~TIOCM_RTS;
        ioctl(m_file.handle(), TIOCMSET, &status);
    }
}

//-----------------------------------------------------------------------------
