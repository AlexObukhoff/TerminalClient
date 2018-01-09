/* @file Ошибки работы драйвера с валютой для устройства приема денег. */

#pragma once

// Modules
#include "Hardware/Common/Specifications.h"

// Project
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"

//--------------------------------------------------------------------------------
namespace ECurrencyError
{
	enum Enum
	{
		OK = 0,         /// Нет ошибок.
		Loading,        /// Не прогрузили номиналы из девайса.
		Config,         /// Неизвестная валюта конфига.
		Billset,        /// Не получили билл-сет девайса или получили, но пустой.
		Compatibility,  /// Код валюты билл-сета отличается от кода валюты в конфиге.
		NoAvailable,    /// Нет доступных номиналов.
		ParInhibitions  /// Не установлены запрещения номиналов.
	};

	class CSpecifications: public CSpecification<Enum, int>
	{
	public:
		CSpecifications()
		{
			append(Loading, BillAcceptorStatusCode::Error::ParTableLoading);
			append(NoAvailable, BillAcceptorStatusCode::Error::NoParsAvailable);
			append(ParInhibitions, BillAcceptorStatusCode::Warning::ParInhibitions);
			
			setDefault(BillAcceptorStatusCode::Error::WrongCurrency);
		}
	};

	static CSpecifications Specification;
}

//--------------------------------------------------------------------------------
