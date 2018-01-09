/* @file Тесты для декодирования сообщений из формата PDU. */

#pragma once

#include <QtTest>
#include <QtCore/QObject>

class PduTest : public QObject
{
    Q_OBJECT
    
public:
    PduTest();
    
private slots:
	void initTestCase();
	void cleanupTestCase();
	void swapSemiOctets();
	void septetsToOctets();
	void septetsToOctets_data();
	void emptySms();
	void decodeSinglePart();
	void decodeSinglePart_data();
	void decodeMessage();
	void decodeMessage_data();
	void encode();

private:
    QFile mValidMessagesFile;
    QList<QByteArray> mValidMessages;
};

