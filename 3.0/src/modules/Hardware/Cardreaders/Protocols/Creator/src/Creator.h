/* @file Протокол Creator. */

#pragma once

#include "Hardware/Common/ProtocolBase.h"

//--------------------------------------------------------------------------------
class Creator: public ProtocolBase
{
public:
	Creator();

	/// Установить порт.
	void setPort(SDK::Driver::IIOPort * aPort);

	/// Выполнить команду протокола.
	TResult processCommand(const QByteArray & aCommandData, QByteArray & aUnpackedAnswer, bool aIOLogsDebugMode = false);

private:
	/// Послать пакет данных в порт.
	bool sendPacket(const QByteArray & aData);

	/// Получить пакет данных из порта.
	bool receivePacket(QByteArray & aData);

	/// Пересчитать базовый индекс пакета в прочитанных из USB порта данных и обрезать USB данные справа в пределах прочитанного пакета.
	bool checkUSBData(QByteArray & aData, int & aBase) const;

	/// Обрезать прочитанные данные, оставив только данные протокола.
	void trimUSBData(QByteArray & aData);

	/// Подсчет контрольной суммы пакета данных.
	char calcCRC(const QByteArray & aData);

	/// Упаковать команду.
	void packData(const QByteArray & aCommandPacket, QByteArray & aPacket);

	/// Прочитать ответный пакет данных от устройства.
	bool readAnswer(QByteArray & aData);

	/// Прочитать ответ.	
	TResult receiveAnswer(QByteArray & aAnswer);

	/// Проверить ответ.
	TResult checkAnswer(const QByteArray & aAnswer);

	/// Отправить ACK.
	bool sendACK();

	/// Отправить NAK.
	bool sendNAK();

	/// Печатать логи обмена с устройством в отладочном режиме.
	bool mIOLogsDebugMode;
};

//--------------------------------------------------------------------------------
