/* @file Окно с локальным соединением. */

#pragma once

// Qt headers
#include <Common/QtHeadersBegin.h>
#include <QtNetwork/QNetworkProxy>
#include "ui_UnmanagedConnectionWindow.h"
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
class UnmanagedConnectionWindow : public QFrame, protected Ui::UnmanagedConnectionWindow
{
	Q_OBJECT

public:
	UnmanagedConnectionWindow(QWidget * aParent = 0);

	void initialize(const QNetworkProxy & aProxy);
	QNetworkProxy getUserSelection() const;

signals:
	/// Пользователь изменил данные формы.
	void userSelectionChanged();

	/// Пользователь выбрал тест соединения с заданными параметрами.
	void testConnection(QNetworkProxy aProxy);

private slots:
	void onTestConnection();
	void onTextChanged(const QString & aText);
	void onProxyTypeChanged(int aIndex);

private:
	/// Внутренний сигнал, изменилась отображаемая информация.
	void toggleProxy(bool aEnabled);
};

//------------------------------------------------------------------------
Q_DECLARE_METATYPE(QNetworkProxy::ProxyType)

//------------------------------------------------------------------------
