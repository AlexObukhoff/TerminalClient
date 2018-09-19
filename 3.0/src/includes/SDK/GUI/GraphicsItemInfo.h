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

	inline bool operator==(const GraphicsItemInfo & aGraphicsItemInfo)
	{
		return
			name == aGraphicsItemInfo.name &&
			type == aGraphicsItemInfo.type &&
			directory == aGraphicsItemInfo.directory &&
			parameters == aGraphicsItemInfo.parameters &&
			context == aGraphicsItemInfo.context;
	}

	inline bool operator!=(const GraphicsItemInfo & aGraphicsItemInfo) { return !(*this == aGraphicsItemInfo); }
};

}} // namespace SDK::GUI

//---------------------------------------------------------------------------

