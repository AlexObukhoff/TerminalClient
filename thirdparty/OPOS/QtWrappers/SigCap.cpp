/****************************************************************************
**
** Metadata for OPOS generated by dumpcpp from type library
** OPOS\CommonCO\OPOSSigCap.ocx
**
****************************************************************************/

#define QAX_DUMPCPP_OPOS_NOINLINES
#include "OPOS\QtWrappers\SigCap.h"

using namespace OPOS;

static const uint qt_meta_data_OPOS__OPOSSigCap[] = {

 // content:
       1,       // revision
       0,       // classname
       6,    10, // classinfo
       35,    22, // methods
       34,    197, // properties
       0,    0, // enums/sets

 // classinfo: key, value
       17, 35, 
       54, 66, 
       78, 90, 
       107, 119, 
       135, 147, 
       163, 175, 

 // signals: signature, parameters, type, tag, flags
       191, 206, 213, 214, 5,
       215, 248, 274, 275, 5,
       276, 305, 361, 362, 5,
       363, 386, 391, 392, 5,
       393, 432, 454, 455, 5,
       456, 481, 486, 487, 5,
       488, 514, 529, 530, 5,

 // slots: signature, parameters, type, tag, flags
       531, 553, 562, 566, 9,
       567, 584, 590, 594, 9,
       595, 612, 620, 624, 9,
       625, 638, 639, 643, 9,
       644, 667, 668, 672, 9,
       673, 681, 682, 686, 9,
       687, 724, 749, 753, 9,
       754, 782, 804, 808, 9,
       809, 822, 823, 827, 9,
       828, 842, 853, 857, 9,
       858, 874, 875, 879, 9,
       880, 905, 922, 926, 9,
       927, 956, 974, 978, 9,
       979, 991, 998, 999, 9,
       1000, 1030, 1056, 1057, 9,
       1058, 1084, 1140, 1141, 9,
       1142, 1169, 1178, 1179, 9,
       1180, 1194, 1195, 1199, 9,
       1200, 1220, 1225, 1226, 9,
       1227, 1248, 1260, 1261, 9,
       1262, 1287, 1304, 1305, 9,
       1306, 1332, 1349, 1350, 9,
       1351, 1374, 1388, 1389, 9,
       1390, 1412, 1425, 1426, 9,
       1427, 1447, 1459, 1460, 9,
       1461, 1490, 1510, 1511, 9,
       1512, 1536, 1553, 1557, 9,
       1558, 1584, 1601, 1605, 9,

 // properties: name, type, flags
       1606, 1618, 0x01015003, 		 // bool AutoDisable
       1623, 1640, 0x02015003, 		 // int BinaryConversion
       1644, 1670, 0x01005001, 		 // bool CapCompareFirmwareVersion
       1675, 1686, 0x01005001, 		 // bool CapDisplay
       1691, 1709, 0x02005001, 		 // int CapPowerReporting
       1713, 1729, 0x01005001, 		 // bool CapRealTimeData
       1734, 1757, 0x01005001, 		 // bool CapStatisticsReporting
       1762, 1780, 0x01005001, 		 // bool CapUpdateFirmware
       1785, 1805, 0x01005001, 		 // bool CapUpdateStatistics
       1810, 1828, 0x01005001, 		 // bool CapUserTerminated
       1833, 1849, 0x0a005001, 		 // QString CheckHealthText
       1857, 1865, 0x01005001, 		 // bool Claimed
       1870, 1895, 0x0a005001, 		 // QString ControlObjectDescription
       1903, 1924, 0x02005001, 		 // int ControlObjectVersion
       1928, 1938, 0x02005001, 		 // int DataCount
       1942, 1959, 0x01015003, 		 // bool DataEventEnabled
       1964, 1982, 0x0a005001, 		 // QString DeviceDescription
       1990, 2004, 0x01015003, 		 // bool DeviceEnabled
       2009, 2020, 0x0a005001, 		 // QString DeviceName
       2028, 2041, 0x01015003, 		 // bool FreezeEvents
       2046, 2055, 0x02005001, 		 // int MaximumX
       2059, 2068, 0x02005001, 		 // int MaximumY
       2072, 2083, 0x02005001, 		 // int OpenResult
       2087, 2098, 0x0a005001, 		 // QString PointArray
       2106, 2118, 0x02015003, 		 // int PowerNotify
       2122, 2133, 0x02005001, 		 // int PowerState
       2137, 2145, 0x0a005001, 		 // QString RawData
       2153, 2173, 0x01015003, 		 // bool RealTimeDataEnabled
       2178, 2189, 0x02005001, 		 // int ResultCode
       2193, 2212, 0x02005001, 		 // int ResultCodeExtended
       2216, 2241, 0x0a005001, 		 // QString ServiceObjectDescription
       2249, 2270, 0x02005001, 		 // int ServiceObjectVersion
       2274, 2280, 0x02005001, 		 // int State
       2284, 2296, 0x02005001, 		 // int TotalPoints

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__OPOSSigCap() {
    static const char stringdata0[] = {
    "OPOS::OPOSSigCap\0"
    "Event Interface 5\0_IOPOSSigCapEvents\0Interface 0\0IOPOSSigCap\0Interface 1\0IOPOSSigCap_1_10\0Interface 2\0IOPOSSigCap_1_9\0Interface 3\0IOPOSSigCap_1_8\0Interface 4\0IOPOSSigCap_1_5\0"
    "DataEvent(int)\0Status\0\0\0DirectIOEvent(int,int&,QString&)\0"
    "EventNumber,pData,pString\0\0\0ErrorEvent(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0\0\0StatusUpdateEvent(int)\0Data\0\0\0exception(int,QString,QString,QString)\0code,source,disc,help\0"
    "\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "BeginCapture(QString)\0FormName\0int\0\0CheckHealth(int)\0Level\0int\0\0ClaimDevice(int)\0Timeout\0int\0\0ClearInput()\0\0int\0\0ClearInputProperties()\0"
    "\0int\0\0Close()\0\0int\0\0CompareFirmwareVersion(QString,int&)\0FirmwareFileName,pResult\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0int\0\0EndCapture()\0\0int\0\0Open(QString)\0DeviceName\0int\0\0ReleaseDevice()\0"
    "\0int\0\0ResetStatistics(QString)\0StatisticsBuffer\0int\0\0RetrieveStatistics(QString&)\0pStatisticsBuffer\0int\0\0SOData(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0EventNumber,pData,pString\0\0\0SOError(int,int,int,int&)\0"
    "ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetAutoDisable(bool)\0AutoDisable\0\0\0SetBinaryConversion(int)\0"
    "BinaryConversion\0\0\0SetDataEventEnabled(bool)\0DataEventEnabled\0\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0\0\0SetPowerNotify(int)\0PowerNotify\0\0\0SetRealTimeDataEnabled(bool)\0"
    "RealTimeDataEnabled\0\0\0UpdateFirmware(QString)\0FirmwareFileName\0int\0\0UpdateStatistics(QString)\0StatisticsBuffer\0int\0\0"
    "AutoDisable\0bool\0BinaryConversion\0int\0CapCompareFirmwareVersion\0bool\0CapDisplay\0bool\0"
    "CapPowerReporting\0int\0CapRealTimeData\0bool\0CapStatisticsReporting\0bool\0CapUpdateFirmware\0bool\0CapUpdateStatistics\0bool\0CapUserTerminated\0bool\0CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0"
    "QString\0ControlObjectVersion\0int\0DataCount\0int\0DataEventEnabled\0bool\0DeviceDescription\0QString\0DeviceEnabled\0bool\0DeviceName\0QString\0FreezeEvents\0bool\0MaximumX\0int\0MaximumY\0int\0OpenResult\0int\0PointArray\0"
    "QString\0PowerNotify\0int\0PowerState\0int\0RawData\0QString\0RealTimeDataEnabled\0bool\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0int\0State\0int\0TotalPoints\0int\0"
    ""
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject OPOSSigCap::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__OPOSSigCap(),
qt_meta_data_OPOS__OPOSSigCap }
};

void *OPOSSigCap::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__OPOSSigCap()))
        return static_cast<void*>(const_cast<OPOSSigCap*>(this));
    return QAxObject::qt_metacast(_clname);
}

static const uint qt_meta_data_OPOS__IOPOSSigCap[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       31,    10, // methods
       35,    165, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       18, 57, 79, 80, 5,
       81, 106, 111, 112, 5,
       113, 139, 154, 155, 5,

 // slots: signature, parameters, type, tag, flags
       156, 178, 187, 191, 9,
       192, 209, 215, 219, 9,
       220, 237, 245, 249, 9,
       250, 263, 264, 268, 9,
       269, 292, 293, 297, 9,
       298, 306, 307, 311, 9,
       312, 349, 374, 378, 9,
       379, 407, 429, 433, 9,
       434, 447, 448, 452, 9,
       453, 467, 478, 482, 9,
       483, 499, 500, 504, 9,
       505, 530, 547, 551, 9,
       552, 581, 599, 603, 9,
       604, 616, 623, 624, 9,
       625, 655, 681, 682, 9,
       683, 709, 765, 766, 9,
       767, 794, 803, 804, 9,
       805, 819, 820, 824, 9,
       825, 845, 850, 851, 9,
       852, 873, 885, 886, 9,
       887, 912, 929, 930, 9,
       931, 957, 974, 975, 9,
       976, 999, 1013, 1014, 9,
       1015, 1037, 1050, 1051, 9,
       1052, 1072, 1084, 1085, 9,
       1086, 1115, 1135, 1136, 9,
       1137, 1161, 1178, 1182, 9,
       1183, 1209, 1226, 1230, 9,

 // properties: name, type, flags
       1231, 1243, 0x01015003, 		 // bool AutoDisable
       1248, 1265, 0x02015003, 		 // int BinaryConversion
       1269, 1295, 0x01005001, 		 // bool CapCompareFirmwareVersion
       1300, 1311, 0x01005001, 		 // bool CapDisplay
       1316, 1334, 0x02005001, 		 // int CapPowerReporting
       1338, 1354, 0x01005001, 		 // bool CapRealTimeData
       1359, 1382, 0x01005001, 		 // bool CapStatisticsReporting
       1387, 1405, 0x01005001, 		 // bool CapUpdateFirmware
       1410, 1430, 0x01005001, 		 // bool CapUpdateStatistics
       1435, 1453, 0x01005001, 		 // bool CapUserTerminated
       1458, 1474, 0x0a005001, 		 // QString CheckHealthText
       1482, 1490, 0x01005001, 		 // bool Claimed
       1495, 1520, 0x0a005001, 		 // QString ControlObjectDescription
       1528, 1549, 0x02005001, 		 // int ControlObjectVersion
       1553, 1563, 0x02005001, 		 // int DataCount
       1567, 1584, 0x01015003, 		 // bool DataEventEnabled
       1589, 1607, 0x0a005001, 		 // QString DeviceDescription
       1615, 1629, 0x01015003, 		 // bool DeviceEnabled
       1634, 1645, 0x0a005001, 		 // QString DeviceName
       1653, 1666, 0x01015003, 		 // bool FreezeEvents
       1671, 1680, 0x02005001, 		 // int MaximumX
       1684, 1693, 0x02005001, 		 // int MaximumY
       1697, 1708, 0x02005001, 		 // int OpenResult
       1712, 1723, 0x0a005001, 		 // QString PointArray
       1731, 1743, 0x02015003, 		 // int PowerNotify
       1747, 1758, 0x02005001, 		 // int PowerState
       1762, 1770, 0x0a005001, 		 // QString RawData
       1778, 1798, 0x01015003, 		 // bool RealTimeDataEnabled
       1803, 1814, 0x02005001, 		 // int ResultCode
       1818, 1837, 0x02005001, 		 // int ResultCodeExtended
       1841, 1866, 0x0a005001, 		 // QString ServiceObjectDescription
       1874, 1895, 0x02005001, 		 // int ServiceObjectVersion
       1899, 1905, 0x02005001, 		 // int State
       1909, 1921, 0x02005001, 		 // int TotalPoints
       1925, 1933, 0x0a055003, 		 // QString control

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__IOPOSSigCap() {
    static const char stringdata0[] = {
    "OPOS::IOPOSSigCap\0"
    "exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "BeginCapture(QString)\0FormName\0int\0\0CheckHealth(int)\0Level\0int\0"
    "\0ClaimDevice(int)\0Timeout\0int\0\0ClearInput()\0\0int\0\0ClearInputProperties()\0\0int\0\0Close()\0\0int\0\0CompareFirmwareVersion(QString,int&)\0FirmwareFileName,pResult\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0"
    "int\0\0EndCapture()\0\0int\0\0Open(QString)\0DeviceName\0int\0\0ReleaseDevice()\0\0int\0\0ResetStatistics(QString)\0StatisticsBuffer\0int\0\0RetrieveStatistics(QString&)\0pStatisticsBuffer\0int\0\0SOData(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0"
    "EventNumber,pData,pString\0\0\0SOError(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetAutoDisable(bool)\0"
    "AutoDisable\0\0\0SetBinaryConversion(int)\0BinaryConversion\0\0\0SetDataEventEnabled(bool)\0DataEventEnabled\0\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0\0\0SetPowerNotify(int)\0PowerNotify\0"
    "\0\0SetRealTimeDataEnabled(bool)\0RealTimeDataEnabled\0\0\0UpdateFirmware(QString)\0FirmwareFileName\0int\0\0UpdateStatistics(QString)\0StatisticsBuffer\0int\0\0"
    "AutoDisable\0bool\0BinaryConversion\0int\0CapCompareFirmwareVersion\0"
    "bool\0CapDisplay\0bool\0CapPowerReporting\0int\0CapRealTimeData\0bool\0CapStatisticsReporting\0bool\0CapUpdateFirmware\0bool\0CapUpdateStatistics\0bool\0CapUserTerminated\0bool\0CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0"
    "QString\0ControlObjectVersion\0int\0DataCount\0int\0DataEventEnabled\0bool\0DeviceDescription\0QString\0DeviceEnabled\0bool\0DeviceName\0QString\0FreezeEvents\0bool\0MaximumX\0int\0MaximumY\0int\0OpenResult\0int\0PointArray\0"
    "QString\0PowerNotify\0int\0PowerState\0int\0RawData\0QString\0RealTimeDataEnabled\0bool\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0int\0State\0int\0TotalPoints\0int\0"
    "control\0QString\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject IOPOSSigCap::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__IOPOSSigCap(),
qt_meta_data_OPOS__IOPOSSigCap }
};

void *IOPOSSigCap::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__IOPOSSigCap()))
        return static_cast<void*>(const_cast<IOPOSSigCap*>(this));
    return QAxObject::qt_metacast(_clname);
}

static const uint qt_meta_data_OPOS__IOPOSSigCap_1_10[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       31,    10, // methods
       35,    165, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       23, 62, 84, 85, 5,
       86, 111, 116, 117, 5,
       118, 144, 159, 160, 5,

 // slots: signature, parameters, type, tag, flags
       161, 183, 192, 196, 9,
       197, 214, 220, 224, 9,
       225, 242, 250, 254, 9,
       255, 268, 269, 273, 9,
       274, 297, 298, 302, 9,
       303, 311, 312, 316, 9,
       317, 354, 379, 383, 9,
       384, 412, 434, 438, 9,
       439, 452, 453, 457, 9,
       458, 472, 483, 487, 9,
       488, 504, 505, 509, 9,
       510, 535, 552, 556, 9,
       557, 586, 604, 608, 9,
       609, 621, 628, 629, 9,
       630, 660, 686, 687, 9,
       688, 714, 770, 771, 9,
       772, 799, 808, 809, 9,
       810, 824, 825, 829, 9,
       830, 850, 855, 856, 9,
       857, 878, 890, 891, 9,
       892, 917, 934, 935, 9,
       936, 962, 979, 980, 9,
       981, 1004, 1018, 1019, 9,
       1020, 1042, 1055, 1056, 9,
       1057, 1077, 1089, 1090, 9,
       1091, 1120, 1140, 1141, 9,
       1142, 1166, 1183, 1187, 9,
       1188, 1214, 1231, 1235, 9,

 // properties: name, type, flags
       1236, 1248, 0x01015003, 		 // bool AutoDisable
       1253, 1270, 0x02015003, 		 // int BinaryConversion
       1274, 1300, 0x01005001, 		 // bool CapCompareFirmwareVersion
       1305, 1316, 0x01005001, 		 // bool CapDisplay
       1321, 1339, 0x02005001, 		 // int CapPowerReporting
       1343, 1359, 0x01005001, 		 // bool CapRealTimeData
       1364, 1387, 0x01005001, 		 // bool CapStatisticsReporting
       1392, 1410, 0x01005001, 		 // bool CapUpdateFirmware
       1415, 1435, 0x01005001, 		 // bool CapUpdateStatistics
       1440, 1458, 0x01005001, 		 // bool CapUserTerminated
       1463, 1479, 0x0a005001, 		 // QString CheckHealthText
       1487, 1495, 0x01005001, 		 // bool Claimed
       1500, 1525, 0x0a005001, 		 // QString ControlObjectDescription
       1533, 1554, 0x02005001, 		 // int ControlObjectVersion
       1558, 1568, 0x02005001, 		 // int DataCount
       1572, 1589, 0x01015003, 		 // bool DataEventEnabled
       1594, 1612, 0x0a005001, 		 // QString DeviceDescription
       1620, 1634, 0x01015003, 		 // bool DeviceEnabled
       1639, 1650, 0x0a005001, 		 // QString DeviceName
       1658, 1671, 0x01015003, 		 // bool FreezeEvents
       1676, 1685, 0x02005001, 		 // int MaximumX
       1689, 1698, 0x02005001, 		 // int MaximumY
       1702, 1713, 0x02005001, 		 // int OpenResult
       1717, 1728, 0x0a005001, 		 // QString PointArray
       1736, 1748, 0x02015003, 		 // int PowerNotify
       1752, 1763, 0x02005001, 		 // int PowerState
       1767, 1775, 0x0a005001, 		 // QString RawData
       1783, 1803, 0x01015003, 		 // bool RealTimeDataEnabled
       1808, 1819, 0x02005001, 		 // int ResultCode
       1823, 1842, 0x02005001, 		 // int ResultCodeExtended
       1846, 1871, 0x0a005001, 		 // QString ServiceObjectDescription
       1879, 1900, 0x02005001, 		 // int ServiceObjectVersion
       1904, 1910, 0x02005001, 		 // int State
       1914, 1926, 0x02005001, 		 // int TotalPoints
       1930, 1938, 0x0a055003, 		 // QString control

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__IOPOSSigCap_1_10() {
    static const char stringdata0[] = {
    "OPOS::IOPOSSigCap_1_10\0"
    "exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "BeginCapture(QString)\0FormName\0int\0\0CheckHealth(int)\0Level\0int\0"
    "\0ClaimDevice(int)\0Timeout\0int\0\0ClearInput()\0\0int\0\0ClearInputProperties()\0\0int\0\0Close()\0\0int\0\0CompareFirmwareVersion(QString,int&)\0FirmwareFileName,pResult\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0"
    "int\0\0EndCapture()\0\0int\0\0Open(QString)\0DeviceName\0int\0\0ReleaseDevice()\0\0int\0\0ResetStatistics(QString)\0StatisticsBuffer\0int\0\0RetrieveStatistics(QString&)\0pStatisticsBuffer\0int\0\0SOData(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0"
    "EventNumber,pData,pString\0\0\0SOError(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetAutoDisable(bool)\0"
    "AutoDisable\0\0\0SetBinaryConversion(int)\0BinaryConversion\0\0\0SetDataEventEnabled(bool)\0DataEventEnabled\0\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0\0\0SetPowerNotify(int)\0PowerNotify\0"
    "\0\0SetRealTimeDataEnabled(bool)\0RealTimeDataEnabled\0\0\0UpdateFirmware(QString)\0FirmwareFileName\0int\0\0UpdateStatistics(QString)\0StatisticsBuffer\0int\0\0"
    "AutoDisable\0bool\0BinaryConversion\0int\0CapCompareFirmwareVersion\0"
    "bool\0CapDisplay\0bool\0CapPowerReporting\0int\0CapRealTimeData\0bool\0CapStatisticsReporting\0bool\0CapUpdateFirmware\0bool\0CapUpdateStatistics\0bool\0CapUserTerminated\0bool\0CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0"
    "QString\0ControlObjectVersion\0int\0DataCount\0int\0DataEventEnabled\0bool\0DeviceDescription\0QString\0DeviceEnabled\0bool\0DeviceName\0QString\0FreezeEvents\0bool\0MaximumX\0int\0MaximumY\0int\0OpenResult\0int\0PointArray\0"
    "QString\0PowerNotify\0int\0PowerState\0int\0RawData\0QString\0RealTimeDataEnabled\0bool\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0int\0State\0int\0TotalPoints\0int\0"
    "control\0QString\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject IOPOSSigCap_1_10::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__IOPOSSigCap_1_10(),
qt_meta_data_OPOS__IOPOSSigCap_1_10 }
};

void *IOPOSSigCap_1_10::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__IOPOSSigCap_1_10()))
        return static_cast<void*>(const_cast<IOPOSSigCap_1_10*>(this));
    return QAxObject::qt_metacast(_clname);
}

static const uint qt_meta_data_OPOS__IOPOSSigCap_1_9[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       30,    10, // methods
       35,    160, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       22, 61, 83, 84, 5,
       85, 110, 115, 116, 5,
       117, 143, 158, 159, 5,

 // slots: signature, parameters, type, tag, flags
       160, 182, 191, 195, 9,
       196, 213, 219, 223, 9,
       224, 241, 249, 253, 9,
       254, 267, 268, 272, 9,
       273, 281, 282, 286, 9,
       287, 324, 349, 353, 9,
       354, 382, 404, 408, 9,
       409, 422, 423, 427, 9,
       428, 442, 453, 457, 9,
       458, 474, 475, 479, 9,
       480, 505, 522, 526, 9,
       527, 556, 574, 578, 9,
       579, 591, 598, 599, 9,
       600, 630, 656, 657, 9,
       658, 684, 740, 741, 9,
       742, 769, 778, 779, 9,
       780, 794, 795, 799, 9,
       800, 820, 825, 826, 9,
       827, 848, 860, 861, 9,
       862, 887, 904, 905, 9,
       906, 932, 949, 950, 9,
       951, 974, 988, 989, 9,
       990, 1012, 1025, 1026, 9,
       1027, 1047, 1059, 1060, 9,
       1061, 1090, 1110, 1111, 9,
       1112, 1136, 1153, 1157, 9,
       1158, 1184, 1201, 1205, 9,

 // properties: name, type, flags
       1206, 1218, 0x01015003, 		 // bool AutoDisable
       1223, 1240, 0x02015003, 		 // int BinaryConversion
       1244, 1270, 0x01005001, 		 // bool CapCompareFirmwareVersion
       1275, 1286, 0x01005001, 		 // bool CapDisplay
       1291, 1309, 0x02005001, 		 // int CapPowerReporting
       1313, 1329, 0x01005001, 		 // bool CapRealTimeData
       1334, 1357, 0x01005001, 		 // bool CapStatisticsReporting
       1362, 1380, 0x01005001, 		 // bool CapUpdateFirmware
       1385, 1405, 0x01005001, 		 // bool CapUpdateStatistics
       1410, 1428, 0x01005001, 		 // bool CapUserTerminated
       1433, 1449, 0x0a005001, 		 // QString CheckHealthText
       1457, 1465, 0x01005001, 		 // bool Claimed
       1470, 1495, 0x0a005001, 		 // QString ControlObjectDescription
       1503, 1524, 0x02005001, 		 // int ControlObjectVersion
       1528, 1538, 0x02005001, 		 // int DataCount
       1542, 1559, 0x01015003, 		 // bool DataEventEnabled
       1564, 1582, 0x0a005001, 		 // QString DeviceDescription
       1590, 1604, 0x01015003, 		 // bool DeviceEnabled
       1609, 1620, 0x0a005001, 		 // QString DeviceName
       1628, 1641, 0x01015003, 		 // bool FreezeEvents
       1646, 1655, 0x02005001, 		 // int MaximumX
       1659, 1668, 0x02005001, 		 // int MaximumY
       1672, 1683, 0x02005001, 		 // int OpenResult
       1687, 1698, 0x0a005001, 		 // QString PointArray
       1706, 1718, 0x02015003, 		 // int PowerNotify
       1722, 1733, 0x02005001, 		 // int PowerState
       1737, 1745, 0x0a005001, 		 // QString RawData
       1753, 1773, 0x01015003, 		 // bool RealTimeDataEnabled
       1778, 1789, 0x02005001, 		 // int ResultCode
       1793, 1812, 0x02005001, 		 // int ResultCodeExtended
       1816, 1841, 0x0a005001, 		 // QString ServiceObjectDescription
       1849, 1870, 0x02005001, 		 // int ServiceObjectVersion
       1874, 1880, 0x02005001, 		 // int State
       1884, 1896, 0x02005001, 		 // int TotalPoints
       1900, 1908, 0x0a055003, 		 // QString control

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__IOPOSSigCap_1_9() {
    static const char stringdata0[] = {
    "OPOS::IOPOSSigCap_1_9\0"
    "exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "BeginCapture(QString)\0FormName\0int\0\0CheckHealth(int)\0Level\0int\0"
    "\0ClaimDevice(int)\0Timeout\0int\0\0ClearInput()\0\0int\0\0Close()\0\0int\0\0CompareFirmwareVersion(QString,int&)\0FirmwareFileName,pResult\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0int\0\0EndCapture()\0\0int\0"
    "\0Open(QString)\0DeviceName\0int\0\0ReleaseDevice()\0\0int\0\0ResetStatistics(QString)\0StatisticsBuffer\0int\0\0RetrieveStatistics(QString&)\0pStatisticsBuffer\0int\0\0SOData(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0"
    "EventNumber,pData,pString\0\0\0SOError(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetAutoDisable(bool)\0"
    "AutoDisable\0\0\0SetBinaryConversion(int)\0BinaryConversion\0\0\0SetDataEventEnabled(bool)\0DataEventEnabled\0\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0\0\0SetPowerNotify(int)\0PowerNotify\0"
    "\0\0SetRealTimeDataEnabled(bool)\0RealTimeDataEnabled\0\0\0UpdateFirmware(QString)\0FirmwareFileName\0int\0\0UpdateStatistics(QString)\0StatisticsBuffer\0int\0\0"
    "AutoDisable\0bool\0BinaryConversion\0int\0CapCompareFirmwareVersion\0"
    "bool\0CapDisplay\0bool\0CapPowerReporting\0int\0CapRealTimeData\0bool\0CapStatisticsReporting\0bool\0CapUpdateFirmware\0bool\0CapUpdateStatistics\0bool\0CapUserTerminated\0bool\0CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0"
    "QString\0ControlObjectVersion\0int\0DataCount\0int\0DataEventEnabled\0bool\0DeviceDescription\0QString\0DeviceEnabled\0bool\0DeviceName\0QString\0FreezeEvents\0bool\0MaximumX\0int\0MaximumY\0int\0OpenResult\0int\0PointArray\0"
    "QString\0PowerNotify\0int\0PowerState\0int\0RawData\0QString\0RealTimeDataEnabled\0bool\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0int\0State\0int\0TotalPoints\0int\0"
    "control\0QString\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject IOPOSSigCap_1_9::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__IOPOSSigCap_1_9(),
qt_meta_data_OPOS__IOPOSSigCap_1_9 }
};

void *IOPOSSigCap_1_9::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__IOPOSSigCap_1_9()))
        return static_cast<void*>(const_cast<IOPOSSigCap_1_9*>(this));
    return QAxObject::qt_metacast(_clname);
}

static const uint qt_meta_data_OPOS__IOPOSSigCap_1_8[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       28,    10, // methods
       33,    150, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       22, 61, 83, 84, 5,
       85, 110, 115, 116, 5,
       117, 143, 158, 159, 5,

 // slots: signature, parameters, type, tag, flags
       160, 182, 191, 195, 9,
       196, 213, 219, 223, 9,
       224, 241, 249, 253, 9,
       254, 267, 268, 272, 9,
       273, 281, 282, 286, 9,
       287, 315, 337, 341, 9,
       342, 355, 356, 360, 9,
       361, 375, 386, 390, 9,
       391, 407, 408, 412, 9,
       413, 438, 455, 459, 9,
       460, 489, 507, 511, 9,
       512, 524, 531, 532, 9,
       533, 563, 589, 590, 9,
       591, 617, 673, 674, 9,
       675, 702, 711, 712, 9,
       713, 727, 728, 732, 9,
       733, 753, 758, 759, 9,
       760, 781, 793, 794, 9,
       795, 820, 837, 838, 9,
       839, 865, 882, 883, 9,
       884, 907, 921, 922, 9,
       923, 945, 958, 959, 9,
       960, 980, 992, 993, 9,
       994, 1023, 1043, 1044, 9,
       1045, 1071, 1088, 1092, 9,

 // properties: name, type, flags
       1093, 1105, 0x01015003, 		 // bool AutoDisable
       1110, 1127, 0x02015003, 		 // int BinaryConversion
       1131, 1142, 0x01005001, 		 // bool CapDisplay
       1147, 1165, 0x02005001, 		 // int CapPowerReporting
       1169, 1185, 0x01005001, 		 // bool CapRealTimeData
       1190, 1213, 0x01005001, 		 // bool CapStatisticsReporting
       1218, 1238, 0x01005001, 		 // bool CapUpdateStatistics
       1243, 1261, 0x01005001, 		 // bool CapUserTerminated
       1266, 1282, 0x0a005001, 		 // QString CheckHealthText
       1290, 1298, 0x01005001, 		 // bool Claimed
       1303, 1328, 0x0a005001, 		 // QString ControlObjectDescription
       1336, 1357, 0x02005001, 		 // int ControlObjectVersion
       1361, 1371, 0x02005001, 		 // int DataCount
       1375, 1392, 0x01015003, 		 // bool DataEventEnabled
       1397, 1415, 0x0a005001, 		 // QString DeviceDescription
       1423, 1437, 0x01015003, 		 // bool DeviceEnabled
       1442, 1453, 0x0a005001, 		 // QString DeviceName
       1461, 1474, 0x01015003, 		 // bool FreezeEvents
       1479, 1488, 0x02005001, 		 // int MaximumX
       1492, 1501, 0x02005001, 		 // int MaximumY
       1505, 1516, 0x02005001, 		 // int OpenResult
       1520, 1531, 0x0a005001, 		 // QString PointArray
       1539, 1551, 0x02015003, 		 // int PowerNotify
       1555, 1566, 0x02005001, 		 // int PowerState
       1570, 1578, 0x0a005001, 		 // QString RawData
       1586, 1606, 0x01015003, 		 // bool RealTimeDataEnabled
       1611, 1622, 0x02005001, 		 // int ResultCode
       1626, 1645, 0x02005001, 		 // int ResultCodeExtended
       1649, 1674, 0x0a005001, 		 // QString ServiceObjectDescription
       1682, 1703, 0x02005001, 		 // int ServiceObjectVersion
       1707, 1713, 0x02005001, 		 // int State
       1717, 1729, 0x02005001, 		 // int TotalPoints
       1733, 1741, 0x0a055003, 		 // QString control

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__IOPOSSigCap_1_8() {
    static const char stringdata0[] = {
    "OPOS::IOPOSSigCap_1_8\0"
    "exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "BeginCapture(QString)\0FormName\0int\0\0CheckHealth(int)\0Level\0int\0"
    "\0ClaimDevice(int)\0Timeout\0int\0\0ClearInput()\0\0int\0\0Close()\0\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0int\0\0EndCapture()\0\0int\0\0Open(QString)\0DeviceName\0int\0\0ReleaseDevice()\0\0int\0\0ResetStatistics(QString)\0"
    "StatisticsBuffer\0int\0\0RetrieveStatistics(QString&)\0pStatisticsBuffer\0int\0\0SOData(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0EventNumber,pData,pString\0\0\0SOError(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0"
    "\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetAutoDisable(bool)\0AutoDisable\0\0\0SetBinaryConversion(int)\0BinaryConversion\0\0\0SetDataEventEnabled(bool)\0DataEventEnabled\0"
    "\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0\0\0SetPowerNotify(int)\0PowerNotify\0\0\0SetRealTimeDataEnabled(bool)\0RealTimeDataEnabled\0\0\0UpdateStatistics(QString)\0StatisticsBuffer\0"
    "int\0\0"
    "AutoDisable\0bool\0BinaryConversion\0int\0CapDisplay\0bool\0CapPowerReporting\0int\0CapRealTimeData\0bool\0CapStatisticsReporting\0bool\0CapUpdateStatistics\0bool\0CapUserTerminated\0bool\0CheckHealthText\0QString\0"
    "Claimed\0bool\0ControlObjectDescription\0QString\0ControlObjectVersion\0int\0DataCount\0int\0DataEventEnabled\0bool\0DeviceDescription\0QString\0DeviceEnabled\0bool\0DeviceName\0QString\0FreezeEvents\0bool\0MaximumX\0int\0"
    "MaximumY\0int\0OpenResult\0int\0PointArray\0QString\0PowerNotify\0int\0PowerState\0int\0RawData\0QString\0RealTimeDataEnabled\0bool\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0"
    "int\0State\0int\0TotalPoints\0int\0control\0QString\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject IOPOSSigCap_1_8::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__IOPOSSigCap_1_8(),
qt_meta_data_OPOS__IOPOSSigCap_1_8 }
};

void *IOPOSSigCap_1_8::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__IOPOSSigCap_1_8()))
        return static_cast<void*>(const_cast<IOPOSSigCap_1_8*>(this));
    return QAxObject::qt_metacast(_clname);
}

static const uint qt_meta_data_OPOS__IOPOSSigCap_1_5[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       25,    10, // methods
       31,    135, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       22, 61, 83, 84, 5,
       85, 110, 115, 116, 5,
       117, 143, 158, 159, 5,

 // slots: signature, parameters, type, tag, flags
       160, 182, 191, 195, 9,
       196, 213, 219, 223, 9,
       224, 241, 249, 253, 9,
       254, 267, 268, 272, 9,
       273, 281, 282, 286, 9,
       287, 315, 337, 341, 9,
       342, 355, 356, 360, 9,
       361, 375, 386, 390, 9,
       391, 407, 408, 412, 9,
       413, 425, 432, 433, 9,
       434, 464, 490, 491, 9,
       492, 518, 574, 575, 9,
       576, 603, 612, 613, 9,
       614, 628, 629, 633, 9,
       634, 654, 659, 660, 9,
       661, 682, 694, 695, 9,
       696, 721, 738, 739, 9,
       740, 766, 783, 784, 9,
       785, 808, 822, 823, 9,
       824, 846, 859, 860, 9,
       861, 881, 893, 894, 9,
       895, 924, 944, 945, 9,

 // properties: name, type, flags
       946, 958, 0x01015003, 		 // bool AutoDisable
       963, 980, 0x02015003, 		 // int BinaryConversion
       984, 995, 0x01005001, 		 // bool CapDisplay
       1000, 1018, 0x02005001, 		 // int CapPowerReporting
       1022, 1038, 0x01005001, 		 // bool CapRealTimeData
       1043, 1061, 0x01005001, 		 // bool CapUserTerminated
       1066, 1082, 0x0a005001, 		 // QString CheckHealthText
       1090, 1098, 0x01005001, 		 // bool Claimed
       1103, 1128, 0x0a005001, 		 // QString ControlObjectDescription
       1136, 1157, 0x02005001, 		 // int ControlObjectVersion
       1161, 1171, 0x02005001, 		 // int DataCount
       1175, 1192, 0x01015003, 		 // bool DataEventEnabled
       1197, 1215, 0x0a005001, 		 // QString DeviceDescription
       1223, 1237, 0x01015003, 		 // bool DeviceEnabled
       1242, 1253, 0x0a005001, 		 // QString DeviceName
       1261, 1274, 0x01015003, 		 // bool FreezeEvents
       1279, 1288, 0x02005001, 		 // int MaximumX
       1292, 1301, 0x02005001, 		 // int MaximumY
       1305, 1316, 0x02005001, 		 // int OpenResult
       1320, 1331, 0x0a005001, 		 // QString PointArray
       1339, 1351, 0x02015003, 		 // int PowerNotify
       1355, 1366, 0x02005001, 		 // int PowerState
       1370, 1378, 0x0a005001, 		 // QString RawData
       1386, 1406, 0x01015003, 		 // bool RealTimeDataEnabled
       1411, 1422, 0x02005001, 		 // int ResultCode
       1426, 1445, 0x02005001, 		 // int ResultCodeExtended
       1449, 1474, 0x0a005001, 		 // QString ServiceObjectDescription
       1482, 1503, 0x02005001, 		 // int ServiceObjectVersion
       1507, 1513, 0x02005001, 		 // int State
       1517, 1529, 0x02005001, 		 // int TotalPoints
       1533, 1541, 0x0a055003, 		 // QString control

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__IOPOSSigCap_1_5() {
    static const char stringdata0[] = {
    "OPOS::IOPOSSigCap_1_5\0"
    "exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "BeginCapture(QString)\0FormName\0int\0\0CheckHealth(int)\0Level\0int\0"
    "\0ClaimDevice(int)\0Timeout\0int\0\0ClearInput()\0\0int\0\0Close()\0\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0int\0\0EndCapture()\0\0int\0\0Open(QString)\0DeviceName\0int\0\0ReleaseDevice()\0\0int\0\0SOData(int)\0"
    "Status\0\0\0SODirectIO(int,int&,QString&)\0EventNumber,pData,pString\0\0\0SOError(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0"
    "\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetAutoDisable(bool)\0AutoDisable\0\0\0SetBinaryConversion(int)\0BinaryConversion\0\0\0SetDataEventEnabled(bool)\0DataEventEnabled\0\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0"
    "FreezeEvents\0\0\0SetPowerNotify(int)\0PowerNotify\0\0\0SetRealTimeDataEnabled(bool)\0RealTimeDataEnabled\0\0\0"
    "AutoDisable\0bool\0BinaryConversion\0int\0CapDisplay\0bool\0CapPowerReporting\0int\0CapRealTimeData\0bool\0CapUserTerminated\0"
    "bool\0CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0QString\0ControlObjectVersion\0int\0DataCount\0int\0DataEventEnabled\0bool\0DeviceDescription\0QString\0DeviceEnabled\0bool\0DeviceName\0QString\0FreezeEvents\0"
    "bool\0MaximumX\0int\0MaximumY\0int\0OpenResult\0int\0PointArray\0QString\0PowerNotify\0int\0PowerState\0int\0RawData\0QString\0RealTimeDataEnabled\0bool\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0"
    "ServiceObjectVersion\0int\0State\0int\0TotalPoints\0int\0control\0QString\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject IOPOSSigCap_1_5::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__IOPOSSigCap_1_5(),
qt_meta_data_OPOS__IOPOSSigCap_1_5 }
};

void *IOPOSSigCap_1_5::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__IOPOSSigCap_1_5()))
        return static_cast<void*>(const_cast<IOPOSSigCap_1_5*>(this));
    return QAxObject::qt_metacast(_clname);
}

