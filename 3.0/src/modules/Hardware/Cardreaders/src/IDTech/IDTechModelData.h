/* @file Данные моделей кардридеров IDTech. */

#pragma once

// IDTech SDK
#pragma warning(push, 1)
#include "libIDT_Device.h"
#pragma warning(pop)

// Modules
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе ATOL.
namespace CIDTech
{
	namespace Models
	{
		/// Название кардридера IDTech по умолчанию.
		const char Default[] = "ViVOpay cardreader";

		//TODO: убрать
		const char Kiosk_III_IV[] = "IDTech ViVOpay Kiosk III/IV";

		namespace EModes
		{
			enum Enum
			{
				Unknown = 0,
				COM,
				USB,
				HID,
				BT,
				Keydoard
			};
		}

		struct SData
		{
			QString name;
			EModes::Enum mode;
			bool SRED;
		};

		class CData: public CDescription<IDT_DEVICE_TYPE>
		{
		public:
			CData()
			{
				/*
				AUGUSTA_HID,
				AUGUSTA_KB,     // keyboard mode
				AUGUSTA_S_HID,
				AUGUSTA_S_KB,
				AUGUSTA_S_TTK_HID,
				SPECTRUM_PRO,
				MINISMART_II,
				L100,
				UNIPAY,
				UNIPAY_I_V, // UniPay 1.5
				VP3300_AJ, // with Audio Jack
				KIOSK_III,
				KIOSK_III_S,
				VENDI,
				VP3300_USB,
				UNIPAY_I_V_TTK,
				VP3300_BT,    // Bluetooth
				VP8800,
				NEO2,
				MINISMART_II_COM,
				SPECTRUM_PRO_COM,
				KIOSK_III_COM,
				KIOSK_III_S_COM,
				NEO2_COM
				*/
			}
		};
	}
}

//--------------------------------------------------------------------------------
