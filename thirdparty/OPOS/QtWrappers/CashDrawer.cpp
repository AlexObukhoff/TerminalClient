/****************************************************************************
**
** Metadata for OPOS generated by dumpcpp from type library
** OPOS\CommonCO\OPOSCashDrawer.ocx
**
****************************************************************************/

#define QAX_DUMPCPP_OPOS_NOINLINES
#include "OPOS\QtWrappers\CashDrawer.h"

using namespace OPOS;

static const uint qt_meta_data_OPOS__OPOSCashDrawer[] = {

 // content:
       1,       // revision
       0,       // classname
       5,    10, // classinfo
       28,    20, // methods
       25,    160, // properties
       0,    0, // enums/sets

 // classinfo: key, value
       21, 39, 
       62, 74, 
       90, 102, 
       122, 134, 
       154, 166, 

 // signals: signature, parameters, type, tag, flags
       186, 219, 245, 246, 5,
       247, 270, 275, 276, 5,
       277, 316, 338, 339, 5,
       340, 365, 370, 371, 5,
       372, 398, 413, 414, 5,

 // slots: signature, parameters, type, tag, flags
       415, 432, 438, 442, 9,
       443, 460, 468, 472, 9,
       473, 481, 482, 486, 9,
       487, 524, 549, 553, 9,
       554, 582, 604, 608, 9,
       609, 623, 634, 638, 9,
       639, 652, 653, 657, 9,
       658, 674, 675, 679, 9,
       680, 705, 722, 726, 9,
       727, 756, 774, 778, 9,
       779, 796, 803, 804, 9,
       805, 835, 861, 862, 9,
       863, 894, 950, 951, 9,
       952, 979, 988, 989, 9,
       990, 1004, 1005, 1009, 9,
       1010, 1030, 1035, 1036, 9,
       1037, 1062, 1079, 1080, 9,
       1081, 1104, 1118, 1119, 9,
       1120, 1142, 1155, 1156, 9,
       1157, 1177, 1189, 1190, 9,
       1191, 1215, 1232, 1236, 9,
       1237, 1263, 1280, 1284, 9,
       1285, 1321, 1370, 1374, 9,

 // properties: name, type, flags
       1375, 1392, 0x02015003, 		 // int BinaryConversion
       1396, 1422, 0x01005001, 		 // bool CapCompareFirmwareVersion
       1427, 1445, 0x02005001, 		 // int CapPowerReporting
       1449, 1472, 0x01005001, 		 // bool CapStatisticsReporting
       1477, 1487, 0x01005001, 		 // bool CapStatus
       1492, 1519, 0x01005001, 		 // bool CapStatusMultiDrawerDetect
       1524, 1542, 0x01005001, 		 // bool CapUpdateFirmware
       1547, 1567, 0x01005001, 		 // bool CapUpdateStatistics
       1572, 1588, 0x0a005001, 		 // QString CheckHealthText
       1596, 1604, 0x01005001, 		 // bool Claimed
       1609, 1634, 0x0a005001, 		 // QString ControlObjectDescription
       1642, 1663, 0x02005001, 		 // int ControlObjectVersion
       1667, 1685, 0x0a005001, 		 // QString DeviceDescription
       1693, 1707, 0x01015003, 		 // bool DeviceEnabled
       1712, 1723, 0x0a005001, 		 // QString DeviceName
       1731, 1744, 0x01005001, 		 // bool DrawerOpened
       1749, 1762, 0x01015003, 		 // bool FreezeEvents
       1767, 1778, 0x02005001, 		 // int OpenResult
       1782, 1794, 0x02015003, 		 // int PowerNotify
       1798, 1809, 0x02005001, 		 // int PowerState
       1813, 1824, 0x02005001, 		 // int ResultCode
       1828, 1847, 0x02005001, 		 // int ResultCodeExtended
       1851, 1876, 0x0a005001, 		 // QString ServiceObjectDescription
       1884, 1905, 0x02005001, 		 // int ServiceObjectVersion
       1909, 1915, 0x02005001, 		 // int State

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__OPOSCashDrawer() {
    static const char stringdata0[] = {
    "OPOS::OPOSCashDrawer\0"
    "Event Interface 4\0_IOPOSCashDrawerEvents\0Interface 0\0IOPOSCashDrawer\0Interface 1\0IOPOSCashDrawer_1_9\0Interface 2\0IOPOSCashDrawer_1_8\0Interface 3\0IOPOSCashDrawer_1_5\0"
    "DirectIOEvent(int,int&,QString&)\0EventNumber,pData,pString\0"
    "\0\0StatusUpdateEvent(int)\0Data\0\0\0exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "CheckHealth(int)\0Level\0int\0\0ClaimDevice(int)\0"
    "Timeout\0int\0\0Close()\0\0int\0\0CompareFirmwareVersion(QString,int&)\0FirmwareFileName,pResult\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0int\0\0Open(QString)\0DeviceName\0int\0\0OpenDrawer()\0\0int\0\0ReleaseDevice()\0"
    "\0int\0\0ResetStatistics(QString)\0StatisticsBuffer\0int\0\0RetrieveStatistics(QString&)\0pStatisticsBuffer\0int\0\0SODataDummy(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0EventNumber,pData,pString\0\0\0SOErrorDummy(int,int,int,int&)\0"
    "ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetBinaryConversion(int)\0BinaryConversion\0\0\0SetDeviceEnabled(bool)\0"
    "DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0\0\0SetPowerNotify(int)\0PowerNotify\0\0\0UpdateFirmware(QString)\0FirmwareFileName\0int\0\0UpdateStatistics(QString)\0StatisticsBuffer\0int\0\0WaitForDrawerClose(int,int,int,int)\0"
    "BeepTimeout,BeepFrequency,BeepDuration,BeepDelay\0int\0\0"
    "BinaryConversion\0int\0CapCompareFirmwareVersion\0bool\0CapPowerReporting\0int\0CapStatisticsReporting\0bool\0CapStatus\0bool\0CapStatusMultiDrawerDetect\0bool\0"
    "CapUpdateFirmware\0bool\0CapUpdateStatistics\0bool\0CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0QString\0ControlObjectVersion\0int\0DeviceDescription\0QString\0DeviceEnabled\0bool\0DeviceName\0QString\0"
    "DrawerOpened\0bool\0FreezeEvents\0bool\0OpenResult\0int\0PowerNotify\0int\0PowerState\0int\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0int\0State\0int\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject OPOSCashDrawer::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__OPOSCashDrawer(),
qt_meta_data_OPOS__OPOSCashDrawer }
};

void *OPOSCashDrawer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__OPOSCashDrawer()))
        return static_cast<void*>(const_cast<OPOSCashDrawer*>(this));
    return QAxObject::qt_metacast(_clname);
}

static const uint qt_meta_data_OPOS__IOPOSCashDrawer[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       26,    10, // methods
       26,    140, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       22, 61, 83, 84, 5,
       85, 110, 115, 116, 5,
       117, 143, 158, 159, 5,

 // slots: signature, parameters, type, tag, flags
       160, 177, 183, 187, 9,
       188, 205, 213, 217, 9,
       218, 226, 227, 231, 9,
       232, 269, 294, 298, 9,
       299, 327, 349, 353, 9,
       354, 368, 379, 383, 9,
       384, 397, 398, 402, 9,
       403, 419, 420, 424, 9,
       425, 450, 467, 471, 9,
       472, 501, 519, 523, 9,
       524, 541, 548, 549, 9,
       550, 580, 606, 607, 9,
       608, 639, 695, 696, 9,
       697, 724, 733, 734, 9,
       735, 749, 750, 754, 9,
       755, 775, 780, 781, 9,
       782, 807, 824, 825, 9,
       826, 849, 863, 864, 9,
       865, 887, 900, 901, 9,
       902, 922, 934, 935, 9,
       936, 960, 977, 981, 9,
       982, 1008, 1025, 1029, 9,
       1030, 1066, 1115, 1119, 9,

 // properties: name, type, flags
       1120, 1137, 0x02015003, 		 // int BinaryConversion
       1141, 1167, 0x01005001, 		 // bool CapCompareFirmwareVersion
       1172, 1190, 0x02005001, 		 // int CapPowerReporting
       1194, 1217, 0x01005001, 		 // bool CapStatisticsReporting
       1222, 1232, 0x01005001, 		 // bool CapStatus
       1237, 1264, 0x01005001, 		 // bool CapStatusMultiDrawerDetect
       1269, 1287, 0x01005001, 		 // bool CapUpdateFirmware
       1292, 1312, 0x01005001, 		 // bool CapUpdateStatistics
       1317, 1333, 0x0a005001, 		 // QString CheckHealthText
       1341, 1349, 0x01005001, 		 // bool Claimed
       1354, 1379, 0x0a005001, 		 // QString ControlObjectDescription
       1387, 1408, 0x02005001, 		 // int ControlObjectVersion
       1412, 1430, 0x0a005001, 		 // QString DeviceDescription
       1438, 1452, 0x01015003, 		 // bool DeviceEnabled
       1457, 1468, 0x0a005001, 		 // QString DeviceName
       1476, 1489, 0x01005001, 		 // bool DrawerOpened
       1494, 1507, 0x01015003, 		 // bool FreezeEvents
       1512, 1523, 0x02005001, 		 // int OpenResult
       1527, 1539, 0x02015003, 		 // int PowerNotify
       1543, 1554, 0x02005001, 		 // int PowerState
       1558, 1569, 0x02005001, 		 // int ResultCode
       1573, 1592, 0x02005001, 		 // int ResultCodeExtended
       1596, 1621, 0x0a005001, 		 // QString ServiceObjectDescription
       1629, 1650, 0x02005001, 		 // int ServiceObjectVersion
       1654, 1660, 0x02005001, 		 // int State
       1664, 1672, 0x0a055003, 		 // QString control

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__IOPOSCashDrawer() {
    static const char stringdata0[] = {
    "OPOS::IOPOSCashDrawer\0"
    "exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "CheckHealth(int)\0Level\0int\0\0ClaimDevice(int)\0Timeout\0int\0\0Close()\0"
    "\0int\0\0CompareFirmwareVersion(QString,int&)\0FirmwareFileName,pResult\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0int\0\0Open(QString)\0DeviceName\0int\0\0OpenDrawer()\0\0int\0\0ReleaseDevice()\0\0int\0\0ResetStatistics(QString)\0"
    "StatisticsBuffer\0int\0\0RetrieveStatistics(QString&)\0pStatisticsBuffer\0int\0\0SODataDummy(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0EventNumber,pData,pString\0\0\0SOErrorDummy(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0"
    "\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetBinaryConversion(int)\0BinaryConversion\0\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0"
    "\0\0SetPowerNotify(int)\0PowerNotify\0\0\0UpdateFirmware(QString)\0FirmwareFileName\0int\0\0UpdateStatistics(QString)\0StatisticsBuffer\0int\0\0WaitForDrawerClose(int,int,int,int)\0BeepTimeout,BeepFrequency,BeepDuration,BeepDelay\0"
    "int\0\0"
    "BinaryConversion\0int\0CapCompareFirmwareVersion\0bool\0CapPowerReporting\0int\0CapStatisticsReporting\0bool\0CapStatus\0bool\0CapStatusMultiDrawerDetect\0bool\0CapUpdateFirmware\0bool\0CapUpdateStatistics\0bool\0"
    "CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0QString\0ControlObjectVersion\0int\0DeviceDescription\0QString\0DeviceEnabled\0bool\0DeviceName\0QString\0DrawerOpened\0bool\0FreezeEvents\0bool\0OpenResult\0"
    "int\0PowerNotify\0int\0PowerState\0int\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0int\0State\0int\0control\0QString\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject IOPOSCashDrawer::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__IOPOSCashDrawer(),
qt_meta_data_OPOS__IOPOSCashDrawer }
};

void *IOPOSCashDrawer::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__IOPOSCashDrawer()))
        return static_cast<void*>(const_cast<IOPOSCashDrawer*>(this));
    return QAxObject::qt_metacast(_clname);
}

static const uint qt_meta_data_OPOS__IOPOSCashDrawer_1_9[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       26,    10, // methods
       26,    140, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       26, 65, 87, 88, 5,
       89, 114, 119, 120, 5,
       121, 147, 162, 163, 5,

 // slots: signature, parameters, type, tag, flags
       164, 181, 187, 191, 9,
       192, 209, 217, 221, 9,
       222, 230, 231, 235, 9,
       236, 273, 298, 302, 9,
       303, 331, 353, 357, 9,
       358, 372, 383, 387, 9,
       388, 401, 402, 406, 9,
       407, 423, 424, 428, 9,
       429, 454, 471, 475, 9,
       476, 505, 523, 527, 9,
       528, 545, 552, 553, 9,
       554, 584, 610, 611, 9,
       612, 643, 699, 700, 9,
       701, 728, 737, 738, 9,
       739, 753, 754, 758, 9,
       759, 779, 784, 785, 9,
       786, 811, 828, 829, 9,
       830, 853, 867, 868, 9,
       869, 891, 904, 905, 9,
       906, 926, 938, 939, 9,
       940, 964, 981, 985, 9,
       986, 1012, 1029, 1033, 9,
       1034, 1070, 1119, 1123, 9,

 // properties: name, type, flags
       1124, 1141, 0x02015003, 		 // int BinaryConversion
       1145, 1171, 0x01005001, 		 // bool CapCompareFirmwareVersion
       1176, 1194, 0x02005001, 		 // int CapPowerReporting
       1198, 1221, 0x01005001, 		 // bool CapStatisticsReporting
       1226, 1236, 0x01005001, 		 // bool CapStatus
       1241, 1268, 0x01005001, 		 // bool CapStatusMultiDrawerDetect
       1273, 1291, 0x01005001, 		 // bool CapUpdateFirmware
       1296, 1316, 0x01005001, 		 // bool CapUpdateStatistics
       1321, 1337, 0x0a005001, 		 // QString CheckHealthText
       1345, 1353, 0x01005001, 		 // bool Claimed
       1358, 1383, 0x0a005001, 		 // QString ControlObjectDescription
       1391, 1412, 0x02005001, 		 // int ControlObjectVersion
       1416, 1434, 0x0a005001, 		 // QString DeviceDescription
       1442, 1456, 0x01015003, 		 // bool DeviceEnabled
       1461, 1472, 0x0a005001, 		 // QString DeviceName
       1480, 1493, 0x01005001, 		 // bool DrawerOpened
       1498, 1511, 0x01015003, 		 // bool FreezeEvents
       1516, 1527, 0x02005001, 		 // int OpenResult
       1531, 1543, 0x02015003, 		 // int PowerNotify
       1547, 1558, 0x02005001, 		 // int PowerState
       1562, 1573, 0x02005001, 		 // int ResultCode
       1577, 1596, 0x02005001, 		 // int ResultCodeExtended
       1600, 1625, 0x0a005001, 		 // QString ServiceObjectDescription
       1633, 1654, 0x02005001, 		 // int ServiceObjectVersion
       1658, 1664, 0x02005001, 		 // int State
       1668, 1676, 0x0a055003, 		 // QString control

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__IOPOSCashDrawer_1_9() {
    static const char stringdata0[] = {
    "OPOS::IOPOSCashDrawer_1_9\0"
    "exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "CheckHealth(int)\0Level\0int\0\0ClaimDevice(int)\0Timeout\0int\0\0Close()\0"
    "\0int\0\0CompareFirmwareVersion(QString,int&)\0FirmwareFileName,pResult\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0int\0\0Open(QString)\0DeviceName\0int\0\0OpenDrawer()\0\0int\0\0ReleaseDevice()\0\0int\0\0ResetStatistics(QString)\0"
    "StatisticsBuffer\0int\0\0RetrieveStatistics(QString&)\0pStatisticsBuffer\0int\0\0SODataDummy(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0EventNumber,pData,pString\0\0\0SOErrorDummy(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0"
    "\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetBinaryConversion(int)\0BinaryConversion\0\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0"
    "\0\0SetPowerNotify(int)\0PowerNotify\0\0\0UpdateFirmware(QString)\0FirmwareFileName\0int\0\0UpdateStatistics(QString)\0StatisticsBuffer\0int\0\0WaitForDrawerClose(int,int,int,int)\0BeepTimeout,BeepFrequency,BeepDuration,BeepDelay\0"
    "int\0\0"
    "BinaryConversion\0int\0CapCompareFirmwareVersion\0bool\0CapPowerReporting\0int\0CapStatisticsReporting\0bool\0CapStatus\0bool\0CapStatusMultiDrawerDetect\0bool\0CapUpdateFirmware\0bool\0CapUpdateStatistics\0bool\0"
    "CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0QString\0ControlObjectVersion\0int\0DeviceDescription\0QString\0DeviceEnabled\0bool\0DeviceName\0QString\0DrawerOpened\0bool\0FreezeEvents\0bool\0OpenResult\0"
    "int\0PowerNotify\0int\0PowerState\0int\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0int\0State\0int\0control\0QString\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject IOPOSCashDrawer_1_9::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__IOPOSCashDrawer_1_9(),
qt_meta_data_OPOS__IOPOSCashDrawer_1_9 }
};

void *IOPOSCashDrawer_1_9::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__IOPOSCashDrawer_1_9()))
        return static_cast<void*>(const_cast<IOPOSCashDrawer_1_9*>(this));
    return QAxObject::qt_metacast(_clname);
}

static const uint qt_meta_data_OPOS__IOPOSCashDrawer_1_8[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       24,    10, // methods
       24,    130, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       26, 65, 87, 88, 5,
       89, 114, 119, 120, 5,
       121, 147, 162, 163, 5,

 // slots: signature, parameters, type, tag, flags
       164, 181, 187, 191, 9,
       192, 209, 217, 221, 9,
       222, 230, 231, 235, 9,
       236, 264, 286, 290, 9,
       291, 305, 316, 320, 9,
       321, 334, 335, 339, 9,
       340, 356, 357, 361, 9,
       362, 387, 404, 408, 9,
       409, 438, 456, 460, 9,
       461, 478, 485, 486, 9,
       487, 517, 543, 544, 9,
       545, 576, 632, 633, 9,
       634, 661, 670, 671, 9,
       672, 686, 687, 691, 9,
       692, 712, 717, 718, 9,
       719, 744, 761, 762, 9,
       763, 786, 800, 801, 9,
       802, 824, 837, 838, 9,
       839, 859, 871, 872, 9,
       873, 899, 916, 920, 9,
       921, 957, 1006, 1010, 9,

 // properties: name, type, flags
       1011, 1028, 0x02015003, 		 // int BinaryConversion
       1032, 1050, 0x02005001, 		 // int CapPowerReporting
       1054, 1077, 0x01005001, 		 // bool CapStatisticsReporting
       1082, 1092, 0x01005001, 		 // bool CapStatus
       1097, 1124, 0x01005001, 		 // bool CapStatusMultiDrawerDetect
       1129, 1149, 0x01005001, 		 // bool CapUpdateStatistics
       1154, 1170, 0x0a005001, 		 // QString CheckHealthText
       1178, 1186, 0x01005001, 		 // bool Claimed
       1191, 1216, 0x0a005001, 		 // QString ControlObjectDescription
       1224, 1245, 0x02005001, 		 // int ControlObjectVersion
       1249, 1267, 0x0a005001, 		 // QString DeviceDescription
       1275, 1289, 0x01015003, 		 // bool DeviceEnabled
       1294, 1305, 0x0a005001, 		 // QString DeviceName
       1313, 1326, 0x01005001, 		 // bool DrawerOpened
       1331, 1344, 0x01015003, 		 // bool FreezeEvents
       1349, 1360, 0x02005001, 		 // int OpenResult
       1364, 1376, 0x02015003, 		 // int PowerNotify
       1380, 1391, 0x02005001, 		 // int PowerState
       1395, 1406, 0x02005001, 		 // int ResultCode
       1410, 1429, 0x02005001, 		 // int ResultCodeExtended
       1433, 1458, 0x0a005001, 		 // QString ServiceObjectDescription
       1466, 1487, 0x02005001, 		 // int ServiceObjectVersion
       1491, 1497, 0x02005001, 		 // int State
       1501, 1509, 0x0a055003, 		 // QString control

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__IOPOSCashDrawer_1_8() {
    static const char stringdata0[] = {
    "OPOS::IOPOSCashDrawer_1_8\0"
    "exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "CheckHealth(int)\0Level\0int\0\0ClaimDevice(int)\0Timeout\0int\0\0Close()\0"
    "\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0int\0\0Open(QString)\0DeviceName\0int\0\0OpenDrawer()\0\0int\0\0ReleaseDevice()\0\0int\0\0ResetStatistics(QString)\0StatisticsBuffer\0int\0\0RetrieveStatistics(QString&)\0"
    "pStatisticsBuffer\0int\0\0SODataDummy(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0EventNumber,pData,pString\0\0\0SOErrorDummy(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0\0\0SOOutputCompleteDummy(int)\0"
    "OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetBinaryConversion(int)\0BinaryConversion\0\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0\0\0SetPowerNotify(int)\0PowerNotify\0"
    "\0\0UpdateStatistics(QString)\0StatisticsBuffer\0int\0\0WaitForDrawerClose(int,int,int,int)\0BeepTimeout,BeepFrequency,BeepDuration,BeepDelay\0int\0\0"
    "BinaryConversion\0int\0CapPowerReporting\0int\0CapStatisticsReporting\0"
    "bool\0CapStatus\0bool\0CapStatusMultiDrawerDetect\0bool\0CapUpdateStatistics\0bool\0CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0QString\0ControlObjectVersion\0int\0DeviceDescription\0QString\0DeviceEnabled\0"
    "bool\0DeviceName\0QString\0DrawerOpened\0bool\0FreezeEvents\0bool\0OpenResult\0int\0PowerNotify\0int\0PowerState\0int\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0int\0"
    "State\0int\0control\0QString\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject IOPOSCashDrawer_1_8::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__IOPOSCashDrawer_1_8(),
qt_meta_data_OPOS__IOPOSCashDrawer_1_8 }
};

void *IOPOSCashDrawer_1_8::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__IOPOSCashDrawer_1_8()))
        return static_cast<void*>(const_cast<IOPOSCashDrawer_1_8*>(this));
    return QAxObject::qt_metacast(_clname);
}

static const uint qt_meta_data_OPOS__IOPOSCashDrawer_1_5[] = {

 // content:
       1,       // revision
       0,       // classname
       0,    0, // classinfo
       21,    10, // methods
       22,    115, // properties
       0,    0, // enums/sets

 // signals: signature, parameters, type, tag, flags
       26, 65, 87, 88, 5,
       89, 114, 119, 120, 5,
       121, 147, 162, 163, 5,

 // slots: signature, parameters, type, tag, flags
       164, 181, 187, 191, 9,
       192, 209, 217, 221, 9,
       222, 230, 231, 235, 9,
       236, 264, 286, 290, 9,
       291, 305, 316, 320, 9,
       321, 334, 335, 339, 9,
       340, 356, 357, 361, 9,
       362, 379, 386, 387, 9,
       388, 418, 444, 445, 9,
       446, 477, 533, 534, 9,
       535, 562, 571, 572, 9,
       573, 587, 588, 592, 9,
       593, 613, 618, 619, 9,
       620, 645, 662, 663, 9,
       664, 687, 701, 702, 9,
       703, 725, 738, 739, 9,
       740, 760, 772, 773, 9,
       774, 810, 859, 863, 9,

 // properties: name, type, flags
       864, 881, 0x02015003, 		 // int BinaryConversion
       885, 903, 0x02005001, 		 // int CapPowerReporting
       907, 917, 0x01005001, 		 // bool CapStatus
       922, 949, 0x01005001, 		 // bool CapStatusMultiDrawerDetect
       954, 970, 0x0a005001, 		 // QString CheckHealthText
       978, 986, 0x01005001, 		 // bool Claimed
       991, 1016, 0x0a005001, 		 // QString ControlObjectDescription
       1024, 1045, 0x02005001, 		 // int ControlObjectVersion
       1049, 1067, 0x0a005001, 		 // QString DeviceDescription
       1075, 1089, 0x01015003, 		 // bool DeviceEnabled
       1094, 1105, 0x0a005001, 		 // QString DeviceName
       1113, 1126, 0x01005001, 		 // bool DrawerOpened
       1131, 1144, 0x01015003, 		 // bool FreezeEvents
       1149, 1160, 0x02005001, 		 // int OpenResult
       1164, 1176, 0x02015003, 		 // int PowerNotify
       1180, 1191, 0x02005001, 		 // int PowerState
       1195, 1206, 0x02005001, 		 // int ResultCode
       1210, 1229, 0x02005001, 		 // int ResultCodeExtended
       1233, 1258, 0x0a005001, 		 // QString ServiceObjectDescription
       1266, 1287, 0x02005001, 		 // int ServiceObjectVersion
       1291, 1297, 0x02005001, 		 // int State
       1301, 1309, 0x0a055003, 		 // QString control

        0        // eod
};

static const char *qt_meta_stringdata_OPOS__IOPOSCashDrawer_1_5() {
    static const char stringdata0[] = {
    "OPOS::IOPOSCashDrawer_1_5\0"
    "exception(int,QString,QString,QString)\0code,source,disc,help\0\0\0propertyChanged(QString)\0name\0\0\0signal(QString,int,void*)\0name,argc,argv\0\0\0"
    "CheckHealth(int)\0Level\0int\0\0ClaimDevice(int)\0Timeout\0int\0\0Close()\0"
    "\0int\0\0DirectIO(int,int&,QString&)\0Command,pData,pString\0int\0\0Open(QString)\0DeviceName\0int\0\0OpenDrawer()\0\0int\0\0ReleaseDevice()\0\0int\0\0SODataDummy(int)\0Status\0\0\0SODirectIO(int,int&,QString&)\0EventNumber,pData,pString\0"
    "\0\0SOErrorDummy(int,int,int,int&)\0ResultCode,ResultCodeExtended,ErrorLocus,pErrorResponse\0\0\0SOOutputCompleteDummy(int)\0OutputID\0\0\0SOProcessID()\0\0int\0\0SOStatusUpdate(int)\0Data\0\0\0SetBinaryConversion(int)\0"
    "BinaryConversion\0\0\0SetDeviceEnabled(bool)\0DeviceEnabled\0\0\0SetFreezeEvents(bool)\0FreezeEvents\0\0\0SetPowerNotify(int)\0PowerNotify\0\0\0WaitForDrawerClose(int,int,int,int)\0BeepTimeout,BeepFrequency,BeepDuration,BeepDelay\0"
    "int\0\0"
    "BinaryConversion\0int\0CapPowerReporting\0int\0CapStatus\0bool\0CapStatusMultiDrawerDetect\0bool\0CheckHealthText\0QString\0Claimed\0bool\0ControlObjectDescription\0QString\0ControlObjectVersion\0int\0DeviceDescription\0"
    "QString\0DeviceEnabled\0bool\0DeviceName\0QString\0DrawerOpened\0bool\0FreezeEvents\0bool\0OpenResult\0int\0PowerNotify\0int\0PowerState\0int\0ResultCode\0int\0ResultCodeExtended\0int\0ServiceObjectDescription\0QString\0ServiceObjectVersion\0"
    "int\0State\0int\0control\0QString\0"
    };
    static char data[sizeof(stringdata0) + 0];
    if (!data[0]) {
        int index = 0;
        memcpy(data + index, stringdata0, sizeof(stringdata0) - 1);
        index += sizeof(stringdata0) - 1;
    }

    return data;
};

const QMetaObject IOPOSCashDrawer_1_5::staticMetaObject = {
{ &QObject::staticMetaObject,
qt_meta_stringdata_OPOS__IOPOSCashDrawer_1_5(),
qt_meta_data_OPOS__IOPOSCashDrawer_1_5 }
};

void *IOPOSCashDrawer_1_5::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OPOS__IOPOSCashDrawer_1_5()))
        return static_cast<void*>(const_cast<IOPOSCashDrawer_1_5*>(this));
    return QAxObject::qt_metacast(_clname);
}

