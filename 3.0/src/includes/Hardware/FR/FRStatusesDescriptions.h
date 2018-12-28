/* @file Описатель кодов состояний фискальных регистраторов. */

#pragma once

// Modules
#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Printers/PrinterStatusesDescriptions.h"

// Project
#include "Hardware/FR/FRStatusCodes.h"

#define ADD_FR_STATUS(aStatusCode, aTranslation, aWarningLevel) ADD_BASE_STATUS(aStatusCode, aTranslation, aWarningLevel); mFiscal.insert(aWarningLevel::aStatusCode)
#define ADD_FR_WARNING(aStatusCode, aTranslation) ADD_FR_STATUS(aStatusCode, aTranslation, Warning)
  #define ADD_FR_ERROR(aStatusCode, aTranslation) ADD_FR_STATUS(aStatusCode, aTranslation, Error)

//--------------------------------------------------------------------------------
namespace FRStatusCode
{
	class CSpecifications: public PrinterStatusCode::CSpecifications
	{
	public:
		CSpecifications(): mIsFiscal(true)
		{
			/// Предупреждения.
			ADD_FR_WARNING(EKLZNearEnd,                QCoreApplication::translate("FRStatuses", "#EKLZ_near_end"));
			ADD_FR_WARNING(FiscalMemoryNearEnd,        QCoreApplication::translate("FRStatuses", "#FM_near_end"));
			ADD_FR_WARNING(NotFiscalized,              QCoreApplication::translate("FRStatuses", "#FR_is_not_fiscalized"));
			ADD_FR_WARNING(ZBufferFull,                QCoreApplication::translate("FRStatuses", "#z_buffer_full"));
			ADD_FR_WARNING(OFDNoConnection,            QCoreApplication::translate("FRStatuses", "#ofd_no_connection"));
			ADD_FR_WARNING(FSNearEnd,                  QCoreApplication::translate("FRStatuses", "#fs_near_end"));
			ADD_FR_WARNING(OFDData,                    QCoreApplication::translate("FRStatuses", "#ofd_data"));
			ADD_FR_WARNING(FFDMismatch,                QCoreApplication::translate("FRStatuses", "#ffd_mismatch"));
			ADD_FR_WARNING(FFDFR,                      QCoreApplication::translate("FRStatuses", "#ffd_fr"));
			ADD_FR_WARNING(FFDFS,                      QCoreApplication::translate("FRStatuses", "#ffd_fs"));
			ADD_FR_WARNING(FirmwareUpdating,           QCoreApplication::translate("FRStatuses", "#cannot_switch_firmware_autoupdating_on"));
			ADD_FR_WARNING(WrongDealerTaxSystem,       QCoreApplication::translate("FRStatuses", "#wrong_dealer_tax_system"));
			ADD_FR_WARNING(WrongDealerAgentFlag,       QCoreApplication::translate("FRStatuses", "#wrong_dealer_agent_flag"));
			ADD_FR_WARNING(WrongFiscalizationSettings, QCoreApplication::translate("FRStatuses", "#wrong_fiscalization_settings"));

			/// Ошибки.
			ADD_FR_ERROR(FR,                   QCoreApplication::translate("FRStatuses", "#fiscal_add_on_error"));
			ADD_FR_ERROR(EKLZ,                 QCoreApplication::translate("FRStatuses", "#EKLZ_error"));
			ADD_FR_ERROR(FiscalCollapse,       QCoreApplication::translate("FRStatuses", "#fiscal_collapse"));
			ADD_FR_ERROR(FM,                   QCoreApplication::translate("FRStatuses", "#FM_error"));
			ADD_FR_ERROR(ZBuffer,              QCoreApplication::translate("FRStatuses", "#z_buffer_error"));
			ADD_FR_ERROR(ZBufferOverflow,      QCoreApplication::translate("FRStatuses", "#z_buffer_overflow"));
			ADD_FR_ERROR(NeedCloseSession,     QCoreApplication::translate("FRStatuses", "#need_close_session"));
			ADD_FR_ERROR(FSEnd,                QCoreApplication::translate("FRStatuses", "#fs_end"));
			ADD_FR_ERROR(NeedOFDConnection,    QCoreApplication::translate("FRStatuses", "#need_ofd_connection"));
			ADD_FR_ERROR(FS,                   QCoreApplication::translate("FRStatuses", "#fs"));
			ADD_FR_ERROR(FSClosed,             QCoreApplication::translate("FRStatuses", "#fs_closed"));
			ADD_FR_ERROR(NoMoney,              QCoreApplication::translate("FRStatuses", "#no_money"));
			ADD_FR_ERROR(WrongDealerTaxSystem, QCoreApplication::translate("FRStatuses", "#wrong_dealer_tax_system"));
			ADD_FR_ERROR(WrongDealerAgentFlag, QCoreApplication::translate("FRStatuses", "#wrong_dealer_agent_flag"));
			ADD_FR_ERROR(CashierINN,           QCoreApplication::translate("FRStatuses", "#cashier_inn"));
			ADD_FR_ERROR(Taxes,                QCoreApplication::translate("FRStatuses", "#taxes"));
		}

		TStatusCodes getFiscalStatusCodes()
		{
			return mFiscal;
		}

		void setFiscal(bool aIsFiscal)
		{
			mIsFiscal = aIsFiscal;
		}

		virtual SStatusCodeSpecification value(const int & aKey) const
		{
			SStatusCodeSpecification result = mBuffer.value(aKey, mDefaultValue);

			if (!mIsFiscal && mFiscal.contains(aKey))
			{
				result.warningLevel = SDK::Driver::EWarningLevel::OK;
			}

			return result;
		}

	private:
		TStatusCodes mFiscal;

		bool mIsFiscal;
	};
}

//--------------------------------------------------------------------------------
inline TStatusCodes getFiscalStatusCodes(SDK::Driver::EWarningLevel::Enum warningLevel)
{
	static FRStatusCode::CSpecifications specification;

	TStatusCodes fiscalStatusCodes = specification.getFiscalStatusCodes();
	TStatusCodes result;

	foreach (int statusCode, fiscalStatusCodes)
	{
		if (specification[statusCode].warningLevel == warningLevel)
		{
			result.insert(statusCode);
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
inline TStatusCodes getErrorFRStatusCodes()
{
	static FRStatusCode::CSpecifications specification;

	QMap<int, SStatusCodeSpecification>::Iterator it = specification.data().begin();
	TStatusCodes result;

	for (; it != specification.data().end(); ++it)
	{
		if (it->warningLevel == SDK::Driver::EWarningLevel::Error)
		{
			result << it.key();
		}
	}

	return result;
}

//--------------------------------------------------------------------------------
