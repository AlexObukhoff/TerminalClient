/* @file Новый стиль QLineEdit для нужного поведения виртуальной клавиатуры */

#pragma once

// Qt
#include "Common/QtHeadersBegin.h"
#include <QtGui/QProxyStyle>
#include "Common/QtHeadersEnd.h"

//------------------------------------------------------------------------
class SIPStyle: public QProxyStyle
{
public:
	virtual int styleHint(StyleHint hint, const QStyleOption *option = 0,
	                      const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const
	{
		return hint == SH_RequestSoftwareInputPanel ?
			RSIP_OnMouseClick :
			QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};

//------------------------------------------------------------------------
