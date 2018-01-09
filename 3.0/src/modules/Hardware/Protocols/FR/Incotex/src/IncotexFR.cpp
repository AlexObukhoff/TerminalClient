/* @file Протокол ФР МСтар. */

// Modules
#include "Hardware/Protocols/Common/ProtocolUtils.h"

// Project
#include "IncotexFR.h"

using namespace SDK::Driver::IOPort::COM;

//--------------------------------------------------------------------------------
IncotexFR::IncotexFR()
{
	// данные порта
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR115200);  // preferable for work
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR9600);    // default
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR4800);    // default after resetting to zero
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR57600);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR38400);
	mPortParameters[EParameters::BaudRate].append(EBaudRate::BR19200);

	mProtocolName = ProtocolNames::FR::Incotex;
}

//--------------------------------------------------------------------------------
const uchar IncotexFR::calcCRC(const QByteArray & aData)
{
	if (!aData.size())
	{
		return 0;
	}

	uchar sum = 0;

	for (int i = 0; i < aData.size(); ++i)
	{
		sum += uchar(aData[i]);
	}

	return sum;
}

//--------------------------------------------------------------------------------
bool IncotexFR::packData(FRProtocolCommands::Enum aCommand, const QVariantMap & aCommandData, QByteArray & aPacket)
{
	// Складываем в пакет код сообщения (команды) и в другой пакет данные команды
	QByteArray localCommandData;

	if (!getCommandPacket(aCommand, aPacket, localCommandData, aCommandData))
	{
		toLog(LogLevel::Error, "Incotex: Failed to get command packet");
		return false;
	}

	if (!isESCCommand(aCommand))
	{
		// Пароль передачи данных
		aPacket.append(CIncotexFR::Constants::ModelInfoPassword);

		// Разделитель (фиксированный)
		aPacket.append(CIncotexFR::Constants::Separator);

		// Данные команды
		aPacket.append(localCommandData);

		// Контрольная сумма
		QByteArray CRC8;
		UChar2ASCIIBuffer(CRC8, calcCRC(aPacket));
		aPacket.append(CRC8);

		// Стартовый байт
		aPacket.insert(0, CIncotexFR::Constants::Prefix);

		// Конечный байт
		aPacket.append(CIncotexFR::Constants::Postfix);
	}
	else
	{
		// Данные команды
		aPacket.append(localCommandData);
	}

	return true;
}

//--------------------------------------------------------------------------------
bool IncotexFR::unpackData(const QByteArray & aPacket, QByteArray & aData)
{
	// проверяем длину сообщения
	if (aPacket.size() < CIncotexFR::Constants::MinAnswerSize)
	{
		toLog(LogLevel::Error, QString("Incotex: Length of the packet is too small, %1 < %2")
			.arg(aPacket.size())
			.arg(CIncotexFR::Constants::MinAnswerSize));
		return false;
	}

	// проверяем первый байт
	if (!aPacket.startsWith(CIncotexFR::Constants::Prefix))
	{
		toLog(LogLevel::Error, "Incotex: Invalid first byte (prefix)");
		return false;
	}

	// проверяем последний байт
	if (!aPacket.endsWith(CIncotexFR::Constants::Postfix))
	{
		toLog(LogLevel::Error, "Incotex: Invalid last byte (postfix)");
		return false;
	}

	// обрезаем пакет, удаляем префикс, постфикс и CRC
	QByteArray trimmedAnswer = aPacket.mid(CIncotexFR::Positions::Command, aPacket.size() -
	    (CIncotexFR::Positions::CRC +
	     CIncotexFR::Positions::Command));

	// проверяем контрольную сумму
	uchar localCRC = calcCRC(trimmedAnswer);
	QByteArray CRCBuffer = aPacket.right(CIncotexFR::Positions::CRC).left(CIncotexFR::Positions::CRCsize);
	uchar CRC;

	if (!ASCIIBuffer2UChar(CRCBuffer, CRC))
	{
		toLog(LogLevel::Error, "Incotex: Invalid CRC:{" + CRCBuffer.toHex() + "}");
		return false;
	}

	if (localCRC != CRC)
	{
		toLog(LogLevel::Error, "Incotex: Invalid CRC of the answer message.");
		return false;
	}

	//чистим ответ от NUL-символов, не являющихся разделителями (00 00 -> 00)
	QByteArray purgedData = aPacket;
	int purgedDataSize;
	QByteArray separator = QByteArray(1, CIncotexFR::Constants::Separator);

	do
	{
		purgedDataSize = purgedData.size();
		purgedData = purgedData.replace(separator + separator, separator);
	}
	while (purgedDataSize != purgedData.size());

	// записываем данные
	aData = purgedData.right(purgedData.size() - CIncotexFR::Positions::Command);

	return true;
}

//--------------------------------------------------------------------------------
bool IncotexFR::processCommand(SDK::Driver::IIOPort * aPort,
                             FRProtocolCommands::Enum aCommand,
                             const QVariantMap & aCommandData,
                             SUnpackedDataBase * aUnpackedData)
{
	return localProcessCommand(aPort, aCommand, aCommandData, static_cast<CIncotexFR::SUnpackedData *>(aUnpackedData));
}

//--------------------------------------------------------------------------------
bool IncotexFR::processAnswer(SDK::Driver::IIOPort * aPort,
                            FRProtocolCommands::Enum aCommand,
                            const QVariantMap & aCommandData,
                            QByteArray & aAnswerData,
                            CIncotexFR::SUnpackedData * aUnpackedData)
{
	bool processSuccess = false;

	//если результат успешный и есть ответ и он может содержать код ошибки или режима
	if (aAnswerData.size() > 1)
	{
		CIncotexFR::SUnpackedData data;
		CIncotexFR::SUnpackedData & unpackedData = aUnpackedData ? *aUnpackedData : data;

		// проверяем на ошибку
		if (parseAnswer(aAnswerData, unpackedData))
		{
			processSuccess = !unpackedData.commandResult;

			//если есть ошибка - печатаем в лог ответ
			if (!processSuccess)
			{
				toLog(LogLevel::Error, QString("Incotex: Error: %1").arg(CIncotexFR::Errors::Descriptions[unpackedData.commandResult]));
			}

			QVariantMap commandData;

			switch (unpackedData.commandResult)
			{
				case CIncotexFR::Errors::Codes::CashierNotRegistered :
				{
					if (localProcessCommand(aPort, FRProtocolCommands::CashierRegistration, commandData))
					{
						return localProcessCommand(aPort, aCommand, aCommandData, aUnpackedData);
					}

					break;
				}
				//--------------------------------------------------------------------------------
				case CIncotexFR::Errors::Codes::NeedCloseSession :
				{
					toLog(LogLevel::Normal, "Incotex: Begin processing auto-ZReport command");

					if (localProcessCommand(aPort, FRProtocolCommands::ZReport, commandData) &&
					    localProcessCommand(aPort, FRProtocolCommands::CashierRegistration, commandData))
					{
						return localProcessCommand(aPort, aCommand, aCommandData, aUnpackedData);
					}

					break;
				}
			}
		}
	}

	return processSuccess;
}

//--------------------------------------------------------------------------------
bool IncotexFR::localProcessCommand(SDK::Driver::IIOPort * aPort,
                                  FRProtocolCommands::Enum aCommand,
                                  const QVariantMap & aCommandData,
                                  CIncotexFR::SUnpackedData * aUnpackedData)
{
	QByteArray packet;

	if (!packData(aCommand, aCommandData, packet))
	{
		toLog(LogLevel::Error, "Incotex: Failed to pack data");
		return false;
	}

	toLog(LogLevel::Normal, QString("Incotex: >> {%1}").arg(packet.toHex().data()));
	QByteArray answer;

	if (!execCommand(aPort, packet, answer))
	{
		toLog(LogLevel::Error, "Incotex: Failed to execute command");
		return false;
	}

	QByteArray unpacketAnswer;

	if (!isESCCommand(packet) && !unpackData(answer, unpacketAnswer))
	{
		toLog(LogLevel::Error, "Incotex: Failed to unpack data");
		return false;
	}

	return isESCCommand(aCommand) ? true : processAnswer(aPort, aCommand, aCommandData, unpacketAnswer, aUnpackedData);
}

//--------------------------------------------------------------------------------
bool IncotexFR::execCommand(SDK::Driver::IIOPort * aPort, const QByteArray & aCommandPacket, QByteArray & aAnswerData)
{
	if (!aPort->write(aCommandPacket))
	{
		return false;
	}

	if (!isESCCommand(aCommandPacket))
	{
		return getAnswer(aPort, aAnswerData);
	}

	QByteArray data;
	bool result = aPort->read(data);

	if (!data.isEmpty())
	{
		toLog(LogLevel::Normal, QString("Incotex: << {%1}").arg(data.toHex().data()));
	}

	return result;
}

//--------------------------------------------------------------------------------
bool IncotexFR::getCommandPacket(FRProtocolCommands::Enum aCommand,
                               QByteArray & aPacket,
                               QByteArray & aLocalCommandData,
                               const QVariantMap & aCommandData)
{
	switch (aCommand)
	{
		case FRProtocolCommands::Identification :
		{
			aPacket.append(CIncotexFR::Commands::GetModelInfo);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::GetStatus :
		case FRProtocolCommands::GetCodeStatus :
		{
			aPacket.append(CIncotexFR::Commands::GetStatus);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::GetFRParameters :
		{
			Q_ASSERT(aCommandData.contains(CHardware::StatusType));

			aPacket.append(CIncotexFR::Commands::GetFRParameters);

			//Тип параметра
			UShort2ASCIIBuffer(aLocalCommandData, ushort(aCommandData[CHardware::StatusType].toUInt()), 10, CIncotexFR::Constants::Length::FRParameter);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::CashierRegistration :
		{
			aPacket.append(CIncotexFR::Commands::CashierRegistration);

			//Номер кассира
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::Constants::CashierNumber);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			//Фамилия кассира
			aLocalCommandData.append(QByteArray(CIncotexFR::Constants::Length::Cashier, ASCII::Space));
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::XReport :
		{
			aPacket.append(CIncotexFR::Commands::Report);

			//Тип операции
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::Constants::XReport, 10,
			                                     CIncotexFR::Constants::Length::Report);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			//Флаги
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::FiscalData::Flags::ShortReport);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			//Номер кассира
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::Constants::CashierNumber);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::ZReport :
		{
			aPacket.append(CIncotexFR::Commands::Report);

			//Тип операции
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::Constants::ZReport, 10,
			                                     CIncotexFR::Constants::Length::Report);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			//Флаги
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::FiscalData::Flags::ShortReport);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			//Номер кассира
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::Constants::CashierNumber);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::Sale :
		{
			Q_ASSERT(aCommandData.contains(CHardware::FiscalPrinter::Amount));

			aPacket.append(CIncotexFR::Commands::Sale);

			//Тип операции
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::FiscalData::Sale, 10,
			                                     CIncotexFR::Constants::Length::Operation);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			//Флаги документа
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::FiscalData::Flags::CloseDocument);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			//************************************ РЕКВИЗИТЫ ************************************

			//позиции по X и Y
			char x = 0;
			char y = -1;

			QByteArray emptyData(CIncotexFR::Constants::Length::ElementData::Default, ASCII::NUL);

			// строка шапки 1
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentCap1, x, ++y, emptyData);

			// строка шапки 2
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentCap2, x, ++y, emptyData);

			// строка шапки 3
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentCap3, x, ++y, emptyData);

			// строка шапки 4
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentCap4, x, ++y, emptyData);

			// номер ККМ
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::KKMNumber, x, ++y, emptyData);

			// ИНН владельца терминала
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::INN, x + 15, y, emptyData);

			// номер документа
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentNumber,  x, ++y, emptyData);

			// номер чека
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::ReceiptNumber, x + 15, y, emptyData);

			// номер кассира
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::CashierNumber, x + 30, y, emptyData);

			// дата и время
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DateTime, x, ++y, emptyData);

			// цена услуги
			QByteArray sum = QByteArray::number(int(
				aCommandData[CHardware::FiscalPrinter::Amount].toDouble() * 100));
			sum.insert(sum.size() - 2, ASCII::Dot);

			QByteArray priceData = emptyData;
			priceData.replace(CIncotexFR::Positions::PaidSum, sum.size(), sum);
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::Price, x, ++y, priceData,
				CIncotexFR::FiscalData::Flags::DefaultFont |
				CIncotexFR::FiscalData::Flags::SaleForMoney);

			// итоговая сумма
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::TotalSum, x, ++y, emptyData);

			// уплаченная сумма
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::PaidSum, x, ++y,
				sum.leftJustified(CIncotexFR::Constants::Length::ElementData::Default, ASCII::NUL));

			// сумма сдачи
			int elementAmount =
				addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::OddSum, x, ++y, emptyData);

			//количество передаваемых реквизитов
			QByteArray amountData;
			UChar2ASCIIBuffer(amountData, uchar(elementAmount), 10, CIncotexFR::Constants::Length::ElementsQuantity);
			aLocalCommandData.insert(CIncotexFR::Positions::ElementAmount, amountData);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::Encashment :
		{
			Q_ASSERT(aCommandData.contains(CHardware::FiscalPrinter::Amount));
			Q_ASSERT(aCommandData.contains(CHardware::Printer::Receipt));

			aPacket.append(CIncotexFR::Commands::Sale);

			//Тип операции
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::FiscalData::Encashment, 10,
			                                     CIncotexFR::Constants::Length::Operation);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			//Флаги документа
			UChar2ASCIIBuffer(aLocalCommandData, CIncotexFR::FiscalData::Flags::CloseDocument);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			//************************************ РЕКВИЗИТЫ ************************************

			//позиции по X и Y
			char x = 0;
			char y = -1;

			//получаем строки терминального чека из реестра, который мы запомнили при вызове функции печати чека в классе принтера
			typedef QList<QByteArray> TReceiptBuffer;
			TReceiptBuffer * receipt = dynamic_cast<TReceiptBuffer *>(aCommandData[CHardware::Printer::Receipt].value<TReceiptBuffer *>());

			Q_ASSERT(receipt != 0);

			// нефискальная часть
			for (int i = 0; i < receipt->size(); ++i)
			{
				int start = 0;

				do
				{
					addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::Free, x, ++y,
						receipt->value(i).mid(start, CIncotexFR::Constants::Length::ElementData::Encashment).leftJustified(CIncotexFR::Constants::Length::ElementData::Default, ASCII::NUL));
					start += CIncotexFR::Constants::Length::ElementData::Encashment;
				}
				while (start < receipt->value(i).size());
			}

			//********************************** ФИСКАЛЬНАЯ ЧАСТЬ ***********************************

			QByteArray emptyData(CIncotexFR::Constants::Length::ElementData::Default, ASCII::NUL);

			// строка шапки 1
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentCap1, x, ++y, emptyData);

			// строка шапки 2
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentCap2, x, ++y, emptyData);

			// строка шапки 3
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentCap3, x, ++y, emptyData);

			// строка шапки 4
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentCap4, x, ++y, emptyData);

			// номер ККМ
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::KKMNumber, x, ++y, emptyData);

			// ИНН владельца терминала
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::INN, x + 15, y, emptyData);

			// номер документа
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DocumentNumber,  x, ++y, emptyData);

			// номер чека
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::ReceiptNumber, x + 15, y, emptyData);

			// номер кассира
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::CashierNumber, x + 30, y, emptyData);

			// дата и время
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::DateTime, x, ++y, emptyData);

			// цена услуги
			QByteArray sum = QByteArray::number(aCommandData[CHardware::FiscalPrinter::Amount].toDouble(), 'f', 2).right(11);
			QByteArray priceData = emptyData;
			priceData.replace(CIncotexFR::Positions::PaidSum, sum.size(), sum);
			addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::Price, x, ++y, priceData,
				CIncotexFR::FiscalData::Flags::DefaultFont |
				CIncotexFR::FiscalData::Flags::SaleForMoney);

			// итоговая сумма
			int elementAmount = addFiscalElement(aLocalCommandData, CIncotexFR::FiscalData::Elements::TotalSum, x, ++y, emptyData);

			//количество передаваемых реквизитов
			QByteArray amountData;
			UChar2ASCIIBuffer(amountData, uchar(elementAmount), 10, CIncotexFR::Constants::Length::ElementsQuantity);
			aLocalCommandData.insert(CIncotexFR::Positions::ElementAmount, amountData);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::GetFRRegisters :
		{
			Q_ASSERT(aCommandData.contains(CHardware::Register));

			aPacket.append(CIncotexFR::Commands::GetRegister);

			//Номер кассира
			UShort2ASCIIBuffer(aLocalCommandData, ushort(aCommandData[CHardware::Register].toUInt()), 10, 4);
			aLocalCommandData.append(CIncotexFR::Constants::Separator);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::SetPrinterMode :
		{
			aPacket.append(CIncotexFR::Commands::SetPrinterMode);

			break;
		}
		//---------------------------------------------------------------
		// Команды нефискального режима
		case FRProtocolCommands::SetFiscalMode :
		{
			aPacket.push_back(CIncotexFR::Commands::SetFiscalMode);

			break;
		}
		//---------------------------------------------------------------
		case FRProtocolCommands::PrintString :
		{
			Q_ASSERT(aCommandData.contains(CHardware::Printer::ByteString));

			aPacket.push_back(aCommandData[CHardware::Printer::ByteString].toByteArray());

			break;
		}
		default:
		{
			toLog(LogLevel::Error, QString("Incotex: The commmand %1 is not implemented").arg(FRProtocolCommands::Description[aCommand]));
			return false;
		}
	}

	return true;
}

//--------------------------------------------------------------------------------
bool IncotexFR::getAnswer(SDK::Driver::IIOPort * aPort, QByteArray & aAnswer)
{
	// Запускаем таймер максимального ожидания
	QTime clockTimer;
	clockTimer.start();

	QByteArray data;
	QByteArray answerData;
	int elapsedTime = 0;
	int elapsedTimeMark = clockTimer.elapsed();
	bool isACK;
	bool isCAN;
	bool isStopRead;

	auto adjust = [] (QByteArray & aAnswer) { int position = aAnswer.indexOf(CIncotexFR::Constants::Prefix);
		if (position != -1) aAnswer = aAnswer.mid(position, aAnswer.indexOf(CIncotexFR::Constants::Postfix, position) - position + 1); };

	do
	{
		data.clear();
		isACK = false;
		isCAN = false;
		isStopRead = false;

		if (!aPort->read(data))
		{
			return false;
		}

		elapsedTime = clockTimer.elapsed();

		if (!data.isEmpty())
		{
			adjust(data);

			if (data.size() == 1)
			{
				isACK = aAnswer.isEmpty() && (data[0] == ASCII::ACK);
				isCAN = aAnswer.isEmpty() && (data[0] == CIncotexFR::Constants::CAN);
			}

			elapsedTimeMark = elapsedTime;

			//если получили не ACK - складываем, иначе - если ESC-команда - выходим
			if (!isACK)
			{
				aAnswer.push_back(data);
			}
		}

		isStopRead = (elapsedTime > CIncotexFR::Timeouts::MaxAnswer) ||
		             ((elapsedTime - elapsedTimeMark) > CIncotexFR::Timeouts::MaxWork) ||
		             data.endsWith(CIncotexFR::Constants::Postfix);

		if (!isStopRead)
		{
			SleepHelper::msleep(CIncotexFR::Timeouts::Default);
		}
	}
	while (!isStopRead);
	/* выходим, если:
	1. очень долго принимаем ответ, при этом что-то в ответе может что-то быть... а может и нет
	2. ККМ ничего не отвечает более 1 секунды
	3. получили CAN
	*/

	toLog(LogLevel::Normal, QString("Incotex: << {%1}").arg(aAnswer.toHex().data()));
	adjust(aAnswer);

	return !aAnswer.isEmpty();
}

//--------------------------------------------------------------------------------
bool IncotexFR::parseAnswer(const QByteArray & aUnpackedData, CIncotexFR::SUnpackedData & aAnswer)
{
	bool result = true;

	QList<QByteArray> answerData = aUnpackedData.split(CIncotexFR::Constants::Separator);

	aAnswer.command = answerData[CIncotexFR::Answer::Sections::Command][0];
	aAnswer.commandResult = answerData[CIncotexFR::Answer::Sections::CommandResult].toUShort(0, 16);

	//парсим статус ККМ
	QByteArray FRStatus = answerData[CIncotexFR::Answer::Sections::FRStatus];
	convertASCIITo16(FRStatus);
	aAnswer.FiscalMemoryNearEnd = ProtocolUtils::getBit(FRStatus, CIncotexFR::Answer::Errors::FiscalMemoryNearEnd, true);
	aAnswer.FiscalMemoryEnd     = ProtocolUtils::getBit(FRStatus, CIncotexFR::Answer::Errors::FiscalMemoryEnd, true);

	//парсим статус принтера
	QByteArray PrinterStatus = answerData[CIncotexFR::Answer::Sections::PrinterStatus];
	convertASCIITo16(PrinterStatus);
	aAnswer.TechnoMode   = !ProtocolUtils::getBit(PrinterStatus, CIncotexFR::Answer::Errors::TechnoMode);
	aAnswer.PrinterError = !ProtocolUtils::getBit(PrinterStatus, CIncotexFR::Answer::Errors::PrinterError);
	aAnswer.CutterOff    =  ProtocolUtils::getBit(PrinterStatus, CIncotexFR::Answer::Errors::CutterOff);
	aAnswer.PaperEnd     =  ProtocolUtils::getBit(PrinterStatus, CIncotexFR::Answer::Errors::PaperEnd);
	aAnswer.OfflineError =  ProtocolUtils::getBit(PrinterStatus, CIncotexFR::Answer::Errors::OfflineError);

	if (aAnswer.commandResult)
	{
		return true;
	}

	QString errorLog = QString("Incotex: Wrong sections in answer : %1, need min ")
		.arg(answerData.size());

	//статусы ЭКЛЗ мы получить не можем
	switch (aAnswer.command)
	{
		case CIncotexFR::Commands::GetModelInfo :
		{
			if (answerData.size() < (CIncotexFR::Answer::Sections::SoftVersion + 1))
			{
				toLog(LogLevel::Error, errorLog + QString::number(CIncotexFR::Answer::Sections::SoftVersion + 1));
				return false;
			}

			aAnswer.modelName   = answerData[CIncotexFR::Answer::Sections::ModelName];
			aAnswer.vendorName  = answerData[CIncotexFR::Answer::Sections::VendorName];
			aAnswer.softVersion = answerData[CIncotexFR::Answer::Sections::SoftVersion];

			break;
		}
		case CIncotexFR::Commands::GetFRParameters :
		{
			if (answerData.size() < (CIncotexFR::Answer::Sections::StatusEKLZ + 1))
			{
				toLog(LogLevel::Error, errorLog + QString::number(CIncotexFR::Answer::Sections::StatusEKLZ));
				return false;
			}

			int ParameterFR = answerData[CIncotexFR::Answer::Sections::ParameterFR].toInt();
			QByteArray StatusEKLZ = answerData[CIncotexFR::Answer::Sections::StatusEKLZ];
			convertASCIITo16(StatusEKLZ);

			if (ParameterFR == CIncotexFR::Constants::EKLZStatusRequest)
			{
				aAnswer.EKLZError   = ProtocolUtils::getBit(StatusEKLZ, CIncotexFR::Answer::Errors::EKLZError, true);
				aAnswer.EKLZNearEnd = ProtocolUtils::getBit(StatusEKLZ, CIncotexFR::Answer::Errors::EKLZNearEnd, true);
			}

			break;
		}
		case CIncotexFR::Commands::GetRegister :
		{
			if (answerData.size() < (CIncotexFR::Answer::Sections::Register + 1))
			{
				toLog(LogLevel::Error, errorLog + QString::number(CIncotexFR::Answer::Sections::Register));
				return false;
			}

			bool convertCorrect;
			aAnswer.Register = answerData[CIncotexFR::Answer::Sections::Register].toDouble(&convertCorrect);

			if (!convertCorrect)
			{
				toLog(LogLevel::Error, "Incotex: wrong money register format: " + answerData[CIncotexFR::Answer::Sections::Register]);
				return false;
			}

			break;
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
const bool IncotexFR::setFRParameters(SDK::Driver::IIOPort * /*aPort*/)
{
	//TODO: реализовать
	return true;
}

//--------------------------------------------------------------------------------
const bool IncotexFR::programmFRParameter(SDK::Driver::IIOPort * /*aPort*/, uchar /*aNumber*/, uchar /*aValue*/)
{
	//TODO: реализовать
	return true;
}

//--------------------------------------------------------------------------------
const bool IncotexFR::isESCCommand(const QByteArray & aPacket)
{
	return aPacket.isEmpty() ? false : ((aPacket[0] != CIncotexFR::Constants::Prefix) && (aPacket != CIncotexFR::Commands::SetFiscalMode));
}

//--------------------------------------------------------------------------------
const bool IncotexFR::isESCCommand(FRProtocolCommands::Enum aCommand)
{
	return (aCommand == FRProtocolCommands::PrintString) ||
	       (aCommand == FRProtocolCommands::SetFiscalMode);
}

//--------------------------------------------------------------------------------
const bool IncotexFR::ASCIIBuffer2UShort(const QByteArray & aData, ushort & aNumber, bool isRevert)
{
	bool result = false;

	if (aData.size() == 2 * sizeof(aNumber))
	{
		ushort dcData = QString(aData).toUShort(&result, 16);

		if (result)
		{
			aNumber = isRevert ? qToBigEndian(dcData) : dcData;
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
void IncotexFR::UShort2ASCIIBuffer(QByteArray & aData, ushort aNumber, int aBase, int aSize)
{
	QString filledNumber = QString::number(aNumber, aBase).rightJustified(2 * sizeof(aNumber), ASCII::Zero, true);

	if (aSize > 0)
	{
		if (aSize < filledNumber.size())
		{
			filledNumber = filledNumber.mid(filledNumber.size() - aSize);
		}
		else
		{
			filledNumber = filledNumber.leftJustified(aSize, ASCII::Space, true);
		}
	}

	aData.append(filledNumber);
}

//--------------------------------------------------------------------------------
const bool IncotexFR::ASCIIBuffer2UChar(const QByteArray & aData, uchar & aNumber)
{
	bool result = false;

	if (aData.size() == 2 * sizeof(aNumber))
	{
		ushort dcData = QString(aData).toUShort(&result, 16);

		if (result)
		{
			aNumber = uchar(dcData);
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
void IncotexFR::UChar2ASCIIBuffer(QByteArray & aData, uchar aNumber, int aBase, int aSize)
{
	QByteArray buffer;
	buffer.append(QString::number(aNumber, aBase).rightJustified(2 * sizeof(aNumber), ASCII::Zero, true));

	if (aSize)
	{
		buffer = (buffer.size() >= aSize) ?
			buffer.mid(buffer.size() - aSize) : buffer.leftJustified(aSize, ASCII::Space, true);
	}

	aData.append(buffer);
}

//--------------------------------------------------------------------------------
const int IncotexFR::addFiscalElement(QByteArray & aLocalData, char elementNumber, uchar aX, uchar aY,
	const QByteArray & aData, ushort aFlags)
{
	UChar2ASCIIBuffer(aLocalData, elementNumber, 10);
	aLocalData.append(CIncotexFR::Constants::Separator);

	UShort2ASCIIBuffer(aLocalData, aFlags, 16);
	aLocalData.append(CIncotexFR::Constants::Separator);

	UChar2ASCIIBuffer(aLocalData, aX, 10, CIncotexFR::Constants::Length::X);
	aLocalData.append(CIncotexFR::Constants::Separator);

	UChar2ASCIIBuffer(aLocalData, aY, 10, CIncotexFR::Constants::Length::Y);
	aLocalData.append(CIncotexFR::Constants::Separator);

	aLocalData.append(QByteArray(CIncotexFR::Constants::Length::Reserved, ASCII::NUL));

	aLocalData.append(aData);

	return aLocalData.size() / CIncotexFR::Constants::Length::Element;
}

//--------------------------------------------------------------------------------
bool IncotexFR::getStatus(SDK::Driver::IIOPort * aPort, CIncotexFR::SUnpackedData & aUnpacketAnswer)
{
	QVariantMap commandData;

	if (localProcessCommand(aPort, FRProtocolCommands::GetStatus, commandData, &aUnpacketAnswer))
	{
		commandData.insert(CHardware::StatusType, CIncotexFR::Constants::EKLZStatusRequest);

		return localProcessCommand(aPort, FRProtocolCommands::GetFRParameters, commandData, &aUnpacketAnswer);
	}

	return false;
}

//--------------------------------------------------------------------------------
void IncotexFR::convertASCIITo16(QByteArray & aBuffer)
{
	int size = aBuffer.size();

	for (int i = 0; i < size / 2; ++i)
	{
		aBuffer.push_back(uchar(aBuffer.left(2).toUShort(0, 16)));
		aBuffer.remove(0, 2);
	}
}

//--------------------------------------------------------------------------------
