/* @file Описатель графического объекта. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <Common/QtHeadersEnd.h>

namespace SDK { 
namespace GUI {

//---------------------------------------------------------------------------
/// Описатель графического объекта. Загружается из файла description.ini в каталоге объекта.
struct GraphicsItemInfo
{
	QString name;                      /// Уникальное название графического элемента.
	QString type;                      /// Тип нужного графического движка.
	QString directory;                 /// Директория с контентом элемента.

	QMap<QString, QString> parameters; /// Специфические параметры для движка.
	QVariantMap context;               /// Специфические параметры для виджета.
};

}} // namespace SDK::GUI

//---------------------------------------------------------------------------

