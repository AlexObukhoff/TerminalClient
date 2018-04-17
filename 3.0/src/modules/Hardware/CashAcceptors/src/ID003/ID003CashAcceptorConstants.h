/* @file Константы купюроприемника на протоколе ID003. */

#pragma once

// Modules
#include "Hardware/Common/ASCII.h"
#include "Hardware/Common/DeviceCodeSpecification.h"
#include "Hardware/Common/WaitingData.h"

// Project
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"
#include "Hardware/CashAcceptors/CashAcceptorBaseConstants.h"

//--------------------------------------------------------------------------------
namespace CID003
{
	/// ACK.
	const char ACK = '\x50';

	/// Пауза при автопоиске для MFL.
	const int IdentificationPause = 2000;

	/// Режим связи с устройством - поллинг.
	char CommunicationMode = ASCII::NUL;

	/// Разрешить прием купюр любой стороной в любом направлении.
	char AllNoteDirections = ASCII::NUL;

	/// Высокий уровень контроля подлинности (ASCII::NUL - нижкий уровень, 2-й байт - по умолчанию)
	const QByteArray HighSecurityLevel = QByteArray(1, ASCII::Full) + ASCII::NUL;

	/// Размер блока данных 1 номинала в ответе на запрос номиналов.
	const int NominalSize = 4;

	/// Ожидание выхода из анабиоза, [мс].
	const SWaitingData AvailableWaiting = SWaitingData(CCashAcceptorsPollingInterval::Enabled, 2200);

	//--------------------------------------------------------------------------------
	/// Команды.
	namespace Commands
	{
		const char GetBillTable   = '\x8A';   /// Запрос таблицы номиналов.
		const char Reset          = '\x40';   /// Перезагрузка.
		const char StatusRequest  = '\x11';   /// Запрос статуса.
		const char Stack1         = '\x41';   /// Уложить в стекер. Статус VendValid выставляется, когда купюра пройдет рычаг соленоида стекера.
		const char Stack2         = '\x42';   /// Уложить в стекер. Статус VendValid выставляется, когда полностью зайдет в стекер (за этим следит сенсор).
		const char Return         = '\x43';   /// Выбросить купюру.
		const char VersionRequest = '\x88';   /// Запрос версии прошивки.
		const char SetEnables     = '\xC0';   /// Установить доступность номиналов.
		const char SetSecurities  = '\xC1';   /// Установить уровень контроля купюр.
		const char SetCommMode    = '\xC2';   /// Установить режим опроса валидатора.
		const char SetInhibits    = '\xC3';   /// Включить валидатор на прием купюр.
		const char SetDirections  = '\xC4';   /// Установить допустимые направления подачи купюры.

		namespace EAnswerType
		{
			enum Enum
			{
				Default,
				ACK,
				Echo,
				Answer
			};
		}

		class CData: public CSpecification < char, EAnswerType::Enum >
		{
		public:
			CData()
			{
				append(Reset,          EAnswerType::ACK);
				append(Return,         EAnswerType::ACK);
				append(Stack1,         EAnswerType::ACK);
				append(Stack2,         EAnswerType::ACK);
				append(SetInhibits,    EAnswerType::Echo);
				append(SetDirections,  EAnswerType::Echo);
				append(SetSecurities,  EAnswerType::Echo);
				append(SetCommMode,    EAnswerType::Echo);
				append(SetEnables,     EAnswerType::Echo);
				append(StatusRequest,  EAnswerType::Answer);
				append(GetBillTable,   EAnswerType::Answer);
				append(VersionRequest, EAnswerType::Answer);

				setDefault(EAnswerType::Default);
			}
		};

		static CData Data;
	}

	//--------------------------------------------------------------------------------
	/// Спецификация статусов.
	class DeviceCodeSpecification : public CommonDeviceCodeSpecification
	{
	public:
		DeviceCodeSpecification()
		{
			/// Состояния.
			addStatus('\x11', BillAcceptorStatusCode::Normal::Enabled);
			addStatus('\x12', BillAcceptorStatusCode::BillOperation::Accepting);
			addStatus('\x13', BillAcceptorStatusCode::BillOperation::Escrow);
			addStatus('\x14', BillAcceptorStatusCode::BillOperation::Stacking);
			addStatus('\x15', BillAcceptorStatusCode::BillOperation::Stacked, "Vend valid");
			addStatus('\x16', BillAcceptorStatusCode::BillOperation::Stacked);
			addStatus('\x18', BillAcceptorStatusCode::Busy::Returning);
			addStatus('\x1A', BillAcceptorStatusCode::Normal::Disabled);
			addStatus('\x1B', DeviceStatusCode::OK::Initialization);
			addStatus('\x40', DeviceStatusCode::OK::Initialization, "Power up");
			addStatus('\x41', DeviceStatusCode::OK::Initialization, "Power up with bill in acceptor");
			addStatus('\x42', DeviceStatusCode::OK::Initialization, "Power up with bill in stacker");
			addStatus('\x43', BillAcceptorStatusCode::MechanicFailure::StackerFull);
			addStatus('\x44', BillAcceptorStatusCode::MechanicFailure::StackerOpen);
			addStatus('\x45', BillAcceptorStatusCode::MechanicFailure::JammedInValidator);
			addStatus('\x46', BillAcceptorStatusCode::MechanicFailure::JammedInStacker);
			addStatus('\x47', BillAcceptorStatusCode::Busy::Pause);
			addStatus('\x48', BillAcceptorStatusCode::Warning::Cheated);

			/// Режекты.
			setExtraCodeDefault('\x17', BillAcceptorStatusCode::Reject::Unknown);
			addStatus('\x71', BillAcceptorStatusCode::Reject::Insertion);
			addStatus('\x72', BillAcceptorStatusCode::Reject::MagneticSensor);
			addStatus('\x73', BillAcceptorStatusCode::Reject::Operation);
			addStatus('\x74', BillAcceptorStatusCode::Reject::DataProcessing);
			addStatus('\x75', BillAcceptorStatusCode::Reject::Transport);
			addStatus('\x76', BillAcceptorStatusCode::Reject::Identification);
			addStatus('\x77', BillAcceptorStatusCode::Reject::OpticalSensor, "Reject by photo pattern");
			addStatus('\x78', BillAcceptorStatusCode::Reject::OpticalSensor, "Reject by photo level");
			addStatus('\x79', BillAcceptorStatusCode::Reject::InhibitNote);
			addStatus('\x7B', BillAcceptorStatusCode::Reject::Operation);
			addStatus('\x7C', BillAcceptorStatusCode::Reject::Operation);
			addStatus('\x7D', BillAcceptorStatusCode::Reject::Length);
			addStatus('\x7E', BillAcceptorStatusCode::Reject::OpticalSensor, "Reject by color pattern");

			/// Ошибки.
			setExtraCodeDefault('\x49', DeviceStatusCode::Error::MechanicFailure);
			addStatus('\xA2', BillAcceptorStatusCode::MechanicFailure::StackerMotor);
			addStatus('\xA5', BillAcceptorStatusCode::MechanicFailure::TransportMotor, "Transport motor speed error");
			addStatus('\xA6', BillAcceptorStatusCode::MechanicFailure::TransportMotor);
			addStatus('\xA7', BillAcceptorStatusCode::MechanicFailure::AligningMotor);
			addStatus('\xAB', BillAcceptorStatusCode::Busy::Unknown, "Cashbox not ready");     //TODO: ворнинг?
			addStatus('\xAC', BillAcceptorStatusCode::SensorError::Optical);
			addStatus('\xAD', BillAcceptorStatusCode::SensorError::Magnetic);
			addStatus('\xAF', BillAcceptorStatusCode::MechanicFailure::HeadRemoved);
			addStatus('\xB0', BillAcceptorStatusCode::Error::Firmware, "Boot ROM fairure");
			addStatus('\xB1', BillAcceptorStatusCode::Error::Firmware, "External ROM fairure");
			addStatus('\xB2', BillAcceptorStatusCode::Error::Firmware, "ROM fairure");
			addStatus('\xB3', BillAcceptorStatusCode::Error::Firmware, "External ROM writing fairure");
		}

		//TODO: при рефакторинге - попробовать завязать на статус-код
		bool isNeedACK(const QByteArray & aBuffer)
		{
			return !aBuffer.isEmpty() && ((aBuffer[0] == '\x15') || (aBuffer[0] == '\x40'));
		}
	};
}

//--------------------------------------------------------------------------------
