TEMPLATE = lib
TARGET = utils
QT += declarative
CONFIG += qt plugin

TARGET = $$qtLibraryTarget($$TARGET)

# Input
SOURCES += \
	../src/LogoProvider.cpp \
	../src/Utils.cpp \
	../src/UtilsPlugin.cpp \
    ../src/Translator.cpp \
	../src/GroupModel.cpp \
	../src/ProviderListFilter.cpp \
	../src/ProviderListModel.cpp

HEADERS += \
	../src/LogoProvider.h \
	../src/Utils.h \
	../src/UtilsPlugin.h \
    ../src/Log.h \
    ../src/Translator.h \
	../src/GroupModel.h \
	../src/ProviderListFilter.h \
	../src/ProviderListModel.h

OTHER_FILES = ../src/qmldir

INCLUDEPATH += $(TC_INCLUDE_DIR)

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
	copy_qmldir.target = $$OUT_PWD/qmldir
	copy_qmldir.depends = ../src/qmldir
	copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
	QMAKE_EXTRA_TARGETS += copy_qmldir
	PRE_TARGETDEPS += $$copy_qmldir.target
}
