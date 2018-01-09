#pragma once 

#if QT_VERSION >= 0x050000
	#include <QJsonDocument>
	#include <QJsonValue>
	#include <QJsonObject>
	#include <QJsonArray>
#else
	#include "json/qjsondocument.h"
	#include "json/qjsonvalue.h"
	#include "json/qjsonobject.h"
	#include "json/qjsonarray.h"
#endif
