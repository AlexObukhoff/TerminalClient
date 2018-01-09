cmake_minimum_required(VERSION 2.6)

set(PROJECT qntp)

set(SOURCES
	src/qntp/NtpClient.cpp
	src/qntp/NtpReply.cpp
)

set(HEADERS
	src/qntp/config.h
	src/qntp/NtpClient.h
	src/qntp/NtpPacket.h
	src/qntp/NtpReply.h
	src/qntp/NtpReply_p.h
	src/qntp/NtpTimestamp.h
	src/qntp/QNtp.h
)

project(${PROJECT})
find_package(Qt4 COMPONENTS QtCore QtNetwork REQUIRED)
include(UseQt4)
include_directories(include ${QT_QTNETWORK_INCLUDE_DIR})
moc_wrap_cpp(MOC_SOURCES ${HEADERS})
add_definitions("-DQNTP_EXPORT=")
add_library(${PROJECT} STATIC ${SOURCES} ${HEADERS} ${MOC_SOURCES})
