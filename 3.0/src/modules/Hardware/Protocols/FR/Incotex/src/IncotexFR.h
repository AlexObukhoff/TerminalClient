/* @file Протокол ФР МСтар. */

#pragma once

// Modules
#include "Hardware/Common/ProtocolBase.h"

// Project
#include "IncotexFRConstants.h"

//--------------------------------------------------------------------------------
/// Класс протокола Incotex.
class IncotexFR : public ProtocolBase
{
	friend class MStarPrinters;

public:
	IncotexFR();

protected:
	/// Выполнить команду протокола.
	virtual bool processCommand(SDK::Driver::IIOPort * aPort,
	                            FRProtocolCommands::Enum aCommand,
	                            const QVariantMap & aCommandData,
	                            SUnpackedDataBase * aUnpackedData = 0);

private:
	/// Местный processCommand, нужен, чтобы не дублировать вызовы и не создать дедлок.
	bool localProcessCommand(SDK::Driver::IIOPort * aPort,
	                         FRProtocolCommands::Enum aCommand,
	                         const QVariantMap & aCommandData,
	                         CIncotexFR::SUnpackedData * aUnpackedData = 0);

	/// Обработка ответа на предыдущей команды. Автоисправление некоторых ошибок.
	bool processAnswer(SDK::Driver::IIOPort * aPort,
	                   FRProtocolCommands::Enum aCommand,
	                   const QVariantMap & aCommandData,
	                   QByteArray & aAnswerData,
	                   CIncotexFR::SUnpackedData * aUnpackedData = 0);

	/// Подсчет контрольной суммы пакета данных.
	const uchar calcCRC(const QByteArray & aData);

	/// Получение пакета с сформированной командой и её данными.
	bool getCommandPacket(FRProtocolCommands::Enum aCommand,
	                      QByteArray & aPacket,
	                      QByteArray & aLocalCommandData,
	                      const QVariantMap & aCommandData);

	/// Исполнить команду.
	bool execCommand(SDK::Driver::IIOPort * aPort, const QByteArray & aCommandPacket, QByteArray & aAnswerData);

	/// Упаковка команды и данных в пакет.
	bool packData(FRProtocolCommands::Enum aCommand, const QVariantMap & aCommandData, QByteArray & aPacket);

	/// Распаковка пришедших из порта данных.
	bool unpackData(const QByteArray & aPacket, QByteArray & aData);

	/// Получить пакет данных из порта.
	bool getAnswer(SDK::Driver::IIOPort * aPort, QByteArray & aData);

	/// Парсинг ответного пакета принтера.
	bool parseAnswer(const QByteArray & aUnpackedData, CIncotexFR::SUnpackedData & aAnswer);

	/// Является ли данная команда ESC-командой.
	/// Если 0-й байт не равен начальному символу обычного пакета FirstPrefix, возвращается true.
	const bool isESCCommand(const QByteArray & aPacket);
	const bool isESCCommand(FRProtocolCommands::Enum aCommand);

	/// Конвертирует из QByteArray в ASCII-формате в ushort.
	/// Например "4567"(== 34 35 36 37) -> 4567.
	const bool ASCIIBuffer2UShort(const QByteArray & aData, ushort & aNumber, bool isRevert = false);

	/// Конвертирует из ushort в QByteArray в ASCII-формате.
	/// Например 0x0538 -> 30 35 33 38(== "0538").
	void UShort2ASCIIBuffer(QByteArray & aData, ushort aNumber, int aBase = 16, int size = 0);

	/// Конвертирует из QByteArray в ASCII-формате в uchar.
	/// Например "45"(== 34 35) -> 45.
	const bool ASCIIBuffer2UChar(const QByteArray & aData, uchar & aNumber);

	/// Конвертирует из uchar в QByteArray в ASCII-формате.
	///  Например 0x05 -> 30 35 (== "05").
	void UChar2ASCIIBuffer(QByteArray & aData, uchar aNumber, int aBase = 16, int aSize = 0);

	/// Уствновить параметры ФР.
	const bool setFRParameters(SDK::Driver::IIOPort * aPort);

	/// Программировать таблицу режимов работы ФР.
	const bool programmFRParameter(SDK::Driver::IIOPort * aPort, uchar aNumber, uchar aValue);

	/// Поместить реквизит в буфер данных для фискального документа.
	/// Возвращает: количество реквизитов.
	const int addFiscalElement(QByteArray & aLocalData, char elementNumber, uchar aX, uchar aY,
		const QByteArray & aData, ushort aFlags = CIncotexFR::FiscalData::Flags::DefaultFont);

	/// Выполнить команду запроса статусов.
	bool getStatus(SDK::Driver::IIOPort * aPort, CIncotexFR::SUnpackedData & aUnpacketAnswer);

	/// Конвертировать буфер ASCII-символов в буфер 16-ричных символов.
	void convertASCIITo16(QByteArray & aBuffer);
};

//--------------------------------------------------------------------------------
