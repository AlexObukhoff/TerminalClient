/* @file Протокол CCNet. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"
#include "Hardware/Protocols/CashAcceptor/CCNetDataTypes.h"

//--------------------------------------------------------------------------------
class CCNetProtocol : public ProtocolBase
{
public:
	CCNetProtocol();

	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aAnswerData, const CCCNet::Commands::SData & aData);

	/// Установить адрес устройства.
	void setAddress(char aAddress);

	/// Изменить параметры порта.
	void changePortParameters(SDK::Driver::TPortParameters aParameters);

	/// Получить ответ.
	TResult getAnswer(QByteArray & aAnswerData, const CCCNet::Commands::SData & aData);

protected:
	/// Подсчет контрольной суммы пакета данных.
	ushort calcCRC16(const QByteArray & aData);

	/// Прочитать пакеты данных из порта.
	typedef QList<QByteArray> TAnswers;
	bool readAnswers(TAnswers & aAnswers, int aTimeout);

	/// Упаковка команды с данными.
	void pack(QByteArray & aCommandData);

	/// Проверка пришедших из порта данных.
	QString check(const QByteArray & aAnswer);

	/// Отсылка ACK в порт.
	bool sendACK();

	/// Адрес устройства.
	char mAddress;

	/// Измененные для командной установки параметры порта.
	SDK::Driver::TPortParameters mPortParameters;
};

//--------------------------------------------------------------------------------
