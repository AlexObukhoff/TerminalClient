cmake_minimum_required(VERSION 2.6)

set(PROJECT SmsMessage)

set(SOURCES
	src/sms.cpp
	src/smspart.cpp
	src/smspdudecoder.cpp
	src/smspduencoder.cpp
)

set(HEADERS
	src/sms.h
	src/sms_p.h
	src/smspart.h
	src/smspart_p.h
	src/smspdudecoder.h
	src/smspdudecoder_p.h
	src/smspduencoder.h
)

project(${PROJECT})
find_package(Qt4 COMPONENTS QtCore REQUIRED)
include(UseQt4)
add_library(${PROJECT} STATIC ${SOURCES} ${HEADERS})
