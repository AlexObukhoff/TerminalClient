TEMPLATE = lib
CONFIG  += staticlib
TARGET   = qntp
QT      -= gui
QT      += network

MOC_DIR = temp/moc

INCLUDEPATH += \
  include \

DESTDIR = $(THIRDPARTY_BIN_DIR)

CONFIG(debug, debug|release) {
  OBJECTS_DIR     = debug
  TARGET          = qntpd
}

CONFIG(release, debug|release) {
  OBJECTS_DIR     = release
}

DEFINES += QNTP_BUILDING

HEADERS += \
  src/qntp/NtpClient.h \
  src/qntp/config.h \
  src/qntp/NtpPacket.h \
  src/qntp/QNtp.h \
  src/qntp/NtpReply.h \
  src/qntp/NtpReply_p.h \
  src/qntp/NtpTimestamp.h \

SOURCES += \
  src/qntp/NtpClient.cpp \
  src/qntp/NtpReply.cpp \
