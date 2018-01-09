#pragma once

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#include <QtCore/QIODevice>
#include <QtCore/QFile>
#include <QtCore/QMutex>

//-----------------------------------------------------------------------------
namespace CBaudRateType { enum Enum {
    Baud50,
    Baud75,
    Baud110,
    Baud134,
    Baud150,
    Baud200,
    Baud300,
    Baud600,
    Baud1200,
    Baud1800,
    Baud2400,
    Baud4800,
    Baud9600,
    Baud19200,
    Baud38400,
    Baud57600,
    // Baud76800,
    Baud115200,
    Baud230400,
    Baud460800,
    Baud500000,
    Baud576000,
    Baud921600,
    Baud1000000,
    Baud1152000,
    Baud1500000,
    Baud2000000,
    Baud2500000,
    Baud3000000,
    Baud3500000,
    Baud4000000
}; } // BaudRateType

namespace CDataBitsType { enum Enum {
    Data5,
    Data6,
    Data7,
    Data8
}; } // DataBitsType

//-----------------------------------------------------------------------------
namespace CParityType { enum Enum {
    None,
    Odd,
    Even,
    Space
}; } // ParityType

//-----------------------------------------------------------------------------
namespace CStopBitsType { enum Enum {
    Stop1,
    Stop2
}; } //CStopBitsType

namespace CFlowType { enum Enum {
    Off,
    Hardware,
    XOnXOff
}; } // CFlowType

//-----------------------------------------------------------------------------
struct SerialDeviceSettings
{
    SerialDeviceSettings();
    
    CBaudRateType::Enum baudRate;
    CDataBitsType::Enum dataBits;
    CParityType::Enum parity;
    CStopBitsType::Enum stopBits;
    CFlowType::Enum flowControl;
    ulong timeout;
};

//-----------------------------------------------------------------------------
class SerialDevice : public QIODevice
{
public:
    explicit SerialDevice(const QString & aFilePath, QObject * aParent = 0);
    ~SerialDevice();

    virtual bool init(); 
    
    virtual bool open(OpenMode aMode = 0);
    virtual bool isSequential() const;
    virtual void close();
    virtual void flush();
    virtual qint64 size() const;
    virtual bool atEnd() const;
    virtual qint64 bytesAvailable();
    virtual void ungetChar(char aCh);
    virtual qint64 readLine(char * aData, qint64 aMaxSize);

    virtual bool setBaudRate(CBaudRateType::Enum aBaudRate);
    virtual CBaudRateType::Enum getBaudRate() const;
    virtual bool setDataBits(CDataBitsType::Enum aDataBits);
    virtual CDataBitsType::Enum getDataBits() const;
    virtual bool setParity(CParityType::Enum aParity);
    virtual CParityType::Enum getParity() const;
    virtual bool setStopBits(CStopBitsType::Enum aStopBits);
    virtual CStopBitsType::Enum getStopBits() const;
    virtual bool setFlowControl(CFlowType::Enum aFlowControl);
    virtual CFlowType::Enum getFlowControl() const;
    virtual bool setTimeout(ulong aMsecs);
    virtual ulong getTimeout() const;

    virtual void setDtr(bool aSet = true);
    virtual void setRts(bool aSet = true);

protected:
    virtual qint64 readData(char * aData, qint64 aMaxSize);
    virtual qint64 writeData(const char * aData, qint64 aMaxSize);

    virtual qint64 doReadData(char * aData, qint64 aMaxSize);
    virtual bool doSetBaudRate(CBaudRateType::Enum aBaudRate);
    virtual bool doSetDataBits(CDataBitsType::Enum aDataBits);
    virtual bool doSetParity(CParityType::Enum aParity);
    virtual bool doSetStopBits(CStopBitsType::Enum aStopBits);
    virtual bool doSetFlowControl(CFlowType::Enum aFlowControl);
    virtual bool doSetTimeout(ulong aMsecs);

    
private:
    SerialDevice(const SerialDevice &);
    SerialDevice & operator= (const SerialDevice &);
    
private:
    QFile m_file;
    SerialDeviceSettings m_settings;
    mutable QMutex m_mutex;
};

//-----------------------------------------------------------------------------
