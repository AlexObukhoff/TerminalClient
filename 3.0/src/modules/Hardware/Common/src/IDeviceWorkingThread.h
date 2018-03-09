/* @file ��������� ��������� ������ ����������. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/WarningLevel.h>

//--------------------------------------------------------------------------------
class IDeviceWorkingThread : public QThread
{
	Q_OBJECT

signals:
	/// ��������� ���������.
	void status(SDK::Driver::EWarningLevel::Enum, const QString &, int);

	/// ��������� �������������.
	void initialized();

public slots:
	/// �������������.
	virtual void initialize() = 0;

	/// �������������.	
	virtual bool checkExistence() = 0;
};

//--------------------------------------------------------------------------------
