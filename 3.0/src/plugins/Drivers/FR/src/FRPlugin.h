/* @file Плагин c драйверами фискальных регистраторов. */

#pragma once

#include "Hardware/Plugins/DevicePluginBase.h"

// ПРИМы
#include "../../../../modules/Hardware/FR/src/Prim/PrimFRBase.h"
#include "../../../../modules/Hardware/FR/src/Prim/Presenter/PrimPresenterFR.h"
#include "../../../../modules/Hardware/FR/src/Prim/Ejector/PrimEjectorFRBase.h"
#include "../../../../modules/Hardware/FR/src/Prim/Online/PrimOnlineFRBase.h"
#include "../../../../modules/Hardware/FR/src/Prim/Online/PrimOnlineFR68.h"
#include "../../../../modules/Hardware/FR/src/Prim/Online/PrimOnlineFRSpecial.h"

// Штрихи
#include "../../../../modules/Hardware/FR/src/Shtrih/Base/ShtrihSerialFR.h"
#include "../../../../modules/Hardware/FR/src/Shtrih/Base/ShtrihRetractorFR.h"
#include "../../../../modules/Hardware/FR/src/Shtrih/Base/Yarus01K.h"
#include "../../../../modules/Hardware/FR/src/Shtrih/Base/ShtrihKiosk.h"
#include "../../../../modules/Hardware/FR/src/Shtrih/Online/Pay/PayOnline.h"
#include "../../../../modules/Hardware/FR/src/Shtrih/Online/Pay/PayVKP80FA.h"
#include "../../../../modules/Hardware/FR/src/Shtrih/Online/ShtrihOnlineFRBase.h"
#include "../../../../modules/Hardware/FR/src/Shtrih/Online/MStarTK2.h"
#include "../../../../modules/Hardware/FR/src/Shtrih/Base/VirtualShtrihFR.h"

// АТОЛы
#include "../../../../modules/Hardware/FR/src/Atol/Base/AtolFR.h"
#include "../../../../modules/Hardware/FR/src/Atol/Base/Ejector/PayVKP80.h"
#include "../../../../modules/Hardware/FR/src/Atol/Base/Ejector/PayPPU700.h"
#include "../../../../modules/Hardware/FR/src/Atol/Online/AtolOnlineFRBase.h"
#include "../../../../modules/Hardware/FR/src/Atol/Online/Paymaster.h"

// остальные
//#include "../../../../modules/Hardware/FR/src/MStar/MStarPrinters.h"
#include "../../../../modules/Hardware/FR/src/OPOSMStarTUPK/OPOSMStarTUPK.h"
#include "../../../../modules/Hardware/FR/src/Spark/SparkFR.h"
#include "../../../../modules/Hardware/FR/src/Kasbi/KasbiFRBase.h"

//------------------------------------------------------------------------------
template <class T>
class FRPluginBase : public DevicePluginBase<T>
{
public:
	FRPluginBase(SDK::Plugin::IEnvironment * aEnvironment, const QString & aInstancePath);

	/// Настраивает плагин.
	virtual void setConfiguration(const QVariantMap & aParameters);

protected:
	/// Добавить кастомные настройки в конфигурацию.
	template<class T2>
	void addConfiguration(QVariantMap & /*aParameters*/) {}

	template <>
	void addConfiguration<AtolSeriesType>(QVariantMap & aParameters)
	{
		using namespace CHardware::FR::Strings;

		aParameters.insert(WithoutTaxes, QCoreApplication::translate("ChequeParameters", "#without_taxes"));              // Без налогов.
	}

	template <>
	void addConfiguration<PrimSeriesType>(QVariantMap & aParameters)
	{
		using namespace CHardware::FR::Strings;

		aParameters.insert(Payment,        QCoreApplication::translate("ChequeParameters", "#cheque_payment"));             // Платеж.
		aParameters.insert(Depositing,     QCoreApplication::translate("ChequeParameters", "#cheque_depositing"));          // Внесено.
		aParameters.insert(SerialNumber,   QCoreApplication::translate("ChequeParameters", "#cheque_fr_serial_number"));    // Серийный номер.
		aParameters.insert(DocumentNumber, QCoreApplication::translate("ChequeParameters", "#cheque_number"));              // Номер документа.
		aParameters.insert(INN,            QCoreApplication::translate("ChequeParameters", "#cheque_inn"));                 // ИНН.
		aParameters.insert(Date,           QCoreApplication::translate("ChequeParameters", "#cheque_date"));                // Дата.
		aParameters.insert(Time,           QCoreApplication::translate("ChequeParameters", "#cheque_time"));                // Время.
		aParameters.insert(Amount,         QCoreApplication::translate("ChequeParameters", "#cheque_sum"));                 // Сумма.
		aParameters.insert(Session,        QCoreApplication::translate("ChequeParameters", "#cheque_session"));             // Смена.
		aParameters.insert(Cashier,        QCoreApplication::translate("ChequeParameters", "#cheque_cashier"));             // Кассир.
		aParameters.insert(ReceiptNumber,  QCoreApplication::translate("ChequeParameters", "#cheque_receipt_number"));      // Чек.
		aParameters.insert(Total,          QCoreApplication::translate("ChequeParameters", "#cheque_total"));               // Итоговая сумма.
	}
};

//--------------------------------------------------------------------------------
