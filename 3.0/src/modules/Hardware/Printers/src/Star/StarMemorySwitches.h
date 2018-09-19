/* @file Мем-свичи принтеров STAR.*/

#pragma once

#include "StarMemorySwitchesBase.h"
#include "ModelData.h"

//----------------------------------------------------------------------------
namespace CSTAR { namespace MemorySwitches
{
	/// Таймаут по умолчанию на получение ответа от устройства, [мс].
	const int MaxNumber = 0xA;

	/// Размер ответа на запрос мем-свича.
	const int AnswerSize = 10;

	/// Минимальный размер ответа на запрос мем-свича.
	const int MinAnswerSize = 8;

	/// Количество попыток запроса мем-свича.
	const int ReadingAttempts = 2;

	/// Количество мем-свичей.
	const int Amount = 11;

	/// Утилиты - предоставляют логику работы с мем-свичами.
	class Utils : public BaseMemorySwitchUtils
	{
	public:
		Utils()
		{
			#define STAR_PARAMETER(aHeaderName, aValues) QStringList aHeaderName = QStringList() << aValues;
			#define STAR_VALUE(aParameters, aValue, aKey) aParameters.insert(aKey, QStringList() << aValue);

			using namespace CHardware::Printer::Settings;
			using namespace CHardware::Printer::Values;
			using namespace CHardwareSDK::Values;
			using namespace CSTAR::Models;

			// LoopHoldEjector (LPE) - для принтеров с эжектором
			TMSWParameters LPEData;
			STAR_PARAMETER(LPEType, Loop   << Hold   << Ejector);
				STAR_VALUE(LPEData, Use    << Use    << Retract, "0000");
				STAR_VALUE(LPEData, Use    << Use    << Present, "0001");
				STAR_VALUE(LPEData, NotUse << Use    << Retract, "0010");
				STAR_VALUE(LPEData, NotUse << Use    << Present, "0011");
				STAR_VALUE(LPEData, NotUse << NotUse << Present, "0100");
			add(ESTARMemorySwitchTypes::LoopHoldEjector, 6, 0x0, LPEType, LPEData, "presenter mode", TModels() << TUP592 << TUP992 << UnknownEjector);

			//TODO: отключено - нет у некоторых принтеров. Если команда установки ASB где-то не работает - кастомизировать ASB-мем-свич и включить
			// ASB - автовыдача статуса при его изменении
			//add(ESTARMemorySwitchTypes::ASB, 7, 0xC, false, CHardware::AutomaticStatus, "automatic status emitting");

			// Presenter paper position status (ASB) - статус презентера включен в ответ на запрос статуса
			add(ESTARMemorySwitchTypes::ASBPresenter, 6, 0x4, false, CHardware::Printer::PresenterStatusEnable, "presenter status enable", TModels() << TUP592 << TUP992 << UnknownEjector);

			// 180 degree rotation print - печатаем задом наперед и с задержками на каждой строе.
			add(ESTARMemorySwitchTypes::VerticalMountMode, 2, 0xC, false, CHardware::Printer::VerticalMountMode, "180 degree rotation print");

			// Таймаут авторетракта
			TMSWParameters PCTData;
			fillExcept(PCTData, "0", "00000000");

			add(ESTARMemorySwitchTypes::AutoRetractionTimeout, 5, 0x0, CHardware::Printer::AutoRetractionTimeout, PCTData, "presenter capture timer");

			// BM function.
			add(ESTARMemorySwitchTypes::BlackMark, 1, 0x8, false, CHardware::Printer::BlackMark, "black mark");

			// Paper in presenter at power on.
			TMSWParameters POnRData592;
			fillExcept(POnRData592, Retract, "10");

			add(ESTARMemorySwitchTypes::PowerOnReaction, 6, 0x8, CHardware::Printer::PowerOnReaction, POnRData592, "paper in presenter at power on", TModels() << TUP592);

			TMSWParameters POnRData992;
			fillExcept(POnRData992, Cut, "1");

			add(ESTARMemorySwitchTypes::PowerOnReaction, 6, 0x8, CHardware::Printer::PowerOnReaction, POnRData992, "paper in presenter at power on", TModels() << TUP992);

			// Codepage.
			TMSWParameters codepageData;
			TMSWSimpleParameters knownParameters;
			knownParameters.insert("00001010", CHardware::Codepages::CP866);
			knownParameters.insert("00100010", CHardware::Codepages::Win1251);
			fillExcept(codepageData, knownParameters);

			add(ESTARMemorySwitchTypes::Codepage, 3, 0x8, CHardware::Codepage, codepageData, "codepage");

			// LineSpacing.
			TMSWParameters LSData;
				STAR_VALUE(LSData, "4", "0");
				STAR_VALUE(LSData, "3", "1");

			add(ESTARMemorySwitchTypes::LineSpacing, 3, 0x0, LineSpacing, LSData, "line spacing");
		}
	};

	/// Обязательные параметры для всех или некоторых принтеров на этапе идентификации.
	class CMainSettings : public CMainSettingsBase
	{
	public:
		CMainSettings()
		{
			using namespace CHardware::Printer;
			using namespace CSTAR::Models;

			add(ESTARMemorySwitchTypes::LoopHoldEjector, Settings::Loop, true);
			add(ESTARMemorySwitchTypes::LoopHoldEjector, Settings::Hold, true);
			add(ESTARMemorySwitchTypes::LoopHoldEjector, Settings::Ejector, Values::Retract);
			add(ESTARMemorySwitchTypes::ASBPresenter, PresenterStatusEnable, true);
			add(ESTARMemorySwitchTypes::AutoRetractionTimeout, AutoRetractionTimeout, 0);
			add(ESTARMemorySwitchTypes::BlackMark, BlackMark, false);
			add(ESTARMemorySwitchTypes::PowerOnReaction, PowerOnReaction, Values::Retract, TModels() << TUP592);
			add(ESTARMemorySwitchTypes::PowerOnReaction, PowerOnReaction, Values::Cut, TModels() << TUP992);
		}
	};
} }

//--------------------------------------------------------------------------------
