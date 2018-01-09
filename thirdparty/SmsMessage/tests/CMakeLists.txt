cmake_minimum_required(VERSION 2.6)

set(PROJECT SmsMessageTest)

project(${PROJECT})

set(SOURCES
	PduTest/tst_pdutest.cpp
	../src/sms.cpp
	../src/smspart.cpp
	../src/smspdudecoder.cpp
	../src/smspduencoder.cpp
)

set(HEADERS
	../src/sms.h
	../src/sms_p.h
	../src/smspart.h
	../src/smspart_p.h
	../src/smspdudecoder.h
	../src/smspdudecoder_p.h
	../src/smspduencoder.h
	PduTest/tst_pdutest.h
)

set(RESOURCES
	PduTest/resources.qrc
)

find_package (Qt4 REQUIRED QtCore QtTest)
include (${QT_USE_FILE})
include_directories(../src ../../../includes)
qt4_wrap_cpp (MOC_SOURCES PduTest/tst_pdutest.h)
qt4_add_resources (QRC_SOURCES ${RESOURCES})
ADD_EXECUTABLE( ${PROJECT} ${HEADERS} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES})
ADD_TEST( NAME ${PROJECT} COMMAND ${PROJECT} )
target_link_libraries (${PROJECT} ${QT_LIBRARIES})
