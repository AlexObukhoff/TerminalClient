/* @file Коды состояний монитора состояний компьютера. */

#pragma once

#include "Hardware/Common/BaseStatus.h"

//--------------------------------------------------------------------------------
namespace HealthStatusCode
{
	/// Предупреждения.
	namespace Warning
	{
		const int HDD0NearEnd     = 310;    /// На HDD0 не хватает места.
		const int HDD0Overheating = 311;    /// HDD0 перегрет.
		const int HDD0NearDead    = 312;    /// HDD0 скоро выйдет из строя.
		const int HDD0Worsened    = 313;    /// Состояние HDD0 ухудшилось.

		const int HDD1NearEnd     = 314;    /// На HDD1 не хватает места.
		const int HDD1Overheating = 315;    /// HDD1 перегрет.
		const int HDD1NearDead    = 316;    /// HDD1 скоро выйдет из строя.
		const int HDD1Worsened    = 317;    /// Состояние HDD1 ухудшилось.

		const int HDD2NearEnd     = 318;    /// На HDD2 не хватает места.
		const int HDD2Overheating = 319;    /// HDD2 перегрет.
		const int HDD2NearDead    = 320;    /// HDD2 скоро выйдет из строя.
		const int HDD2Worsened    = 321;    /// Состояние HDD2 ухудшилось.

		const int HDD3NearEnd     = 322;    /// На HDD3 не хватает места.
		const int HDD3Overheating = 323;    /// HDD3 перегрет.
		const int HDD3NearDead    = 324;    /// HDD3 скоро выйдет из строя.
		const int HDD3Worsened    = 325;    /// Состояние HDD3 ухудшилось.

		const int SRNearEnd          = 326;    /// Процессору платежей не хватает системных ресурсов.
		const int AllSRNearEnd       = 327;    /// Не хватает системных ресурсов.
		const int MemoryNearEnd      = 328;    /// Не хватает памяти.
		const int CPUOverheating     = 329;    /// Процессор почти перегрет.
		const int OSVersion          = 330;    /// ОС не поддерживается.
		const int NeedChangeTimezone = 331;    /// Необходимо сменить временную зону.
		const int NeedConfigTimezone = 332;    /// Необходимо сконфигурировать временную зону.
		const int Antivirus          = 333;    /// Антивирус отключен.
		const int Firewall           = 334;    /// Сетевой экран отключен.
	}

	/// Ошибки.
	namespace Error
	{
		const int HDD0End         = 350;    /// На HDD0 критически не хватает места.
		const int HDD0Overheated  = 351;    /// HDD0 критически перегрет.
		const int HDD0Dead        = 352;    /// HDD0 выходит из строя.

		const int HDD1End         = 353;    /// На HDD1 критически не хватает места.
		const int HDD1Overheated  = 354;    /// HDD1 критически перегрет.
		const int HDD1Dead        = 355;    /// HDD1 выходит из строя.

		const int HDD2End         = 356;    /// На HDD2 критически не хватает места.
		const int HDD2Overheated  = 357;    /// HDD2 критически перегрет.
		const int HDD2Dead        = 358;    /// HDD2 выходит из строя.

		const int HDD3End         = 359;    /// На HDD3 критически не хватает места.
		const int HDD3Overheated  = 360;    /// HDD3 критически перегрет.
		const int HDD3Dead        = 361;    /// HDD3 выходит из строя.

		const int SREnd           = 362;    /// Процессору платежей критически не хватает системных ресурсов.
		const int AllSREnd        = 363;    /// Критически не хватает системных ресурсов.
		const int MemoryEnd       = 364;    /// Критически не хватает памяти.
		const int CPUOverheated   = 365;    /// Процессор перегрет.
	}
}

//--------------------------------------------------------------------------------
