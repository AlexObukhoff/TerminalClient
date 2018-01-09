#include <QString>
#include <QtTest>
#include <QHostInfo>
#include <QCoreApplication>

#include "qntp/NtpClient.h"
#include "qntp/NtpReply.h"

const QString TimeFormat = "yyyy.MM.dd hh:mm:ss.zzz";

class Qntp_Test : public QObject
{
    Q_OBJECT
    
	int replyCount;
	NtpClient client;
	QStringList ntpHosts;

public:
    Qntp_Test();
    
private slots:
	void initTestCase();
    void testCase();

	void ntpReplyReceived(const QHostAddress &address, quint16 port, const NtpReply &reply);
};

Qntp_Test::Qntp_Test() :
	replyCount(0)
{
	connect (&client, SIGNAL(replyReceived(const QHostAddress &, quint16, const NtpReply &)), 
		this, SLOT(ntpReplyReceived(const QHostAddress &, quint16, const NtpReply &)));

	ntpHosts 
		<< "ntp.mobatime.ru";
/*		<< "0.ru.pool.ntp.org"
		<< "1.ru.pool.ntp.org"
		<< "2.ru.pool.ntp.org"
		<< "3.ru.pool.ntp.org";  */
}

void Qntp_Test::initTestCase()
{
	foreach (auto hostName, ntpHosts)
	{
		QString msg = QString("Get time from %1").arg(hostName);

		QHostInfo info = QHostInfo::fromName(hostName);
		
		QVERIFY2(!info.addresses().isEmpty(), "Error host name lookup.");

		QWARN ((msg + " - " +  info.addresses().first().toString()).toAscii().data());

		QVERIFY2(client.sendRequest(info.addresses().first(), 123), msg.toAscii().data());
	}
}


void Qntp_Test::testCase()
{
	int i = 0;
	while (replyCount != ntpHosts.size() && i++ < 50)
	{
		QTest::qWait(250);
	}

	if (replyCount != ntpHosts.size())
	{
		QFAIL("Not from all NTP servers receive reply");
	}
}

void Qntp_Test::ntpReplyReceived(const QHostAddress &address, quint16 port, const NtpReply &reply)
{
	QWARN (QString("Receive reply from %1: local time: %2, server time: %3, localClockOffset: %4 ms")
		.arg(address.toString())
		.arg(reply.destinationTime().toString(TimeFormat))
		.arg(reply.transmitTime().toString(TimeFormat))
		.arg(reply.localClockOffset())
		.toAscii().data());
	
	++replyCount;
}

QTEST_MAIN(Qntp_Test)

#include "qntp_test.moc"
