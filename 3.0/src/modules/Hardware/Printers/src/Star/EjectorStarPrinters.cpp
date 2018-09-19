/* @file Принтеры семейства Star c эжектором. */

#include "EjectorStarPrinters.h"
#include "StarPrinterData.h"
#include "ModelData.h"

using namespace SDK::Driver::IOPort::COM;
using namespace PrinterStatusCode;

//--------------------------------------------------------------------------------
// Модели данной реализации.
namespace CSTAR { inline QStringList getEjectorModels()
{
	return QStringList()
		<< Models::TUP592
		<< Models::TUP992
		<< Models::UnknownEjector;
}}

//--------------------------------------------------------------------------------
EjectorStarPrinter::EjectorStarPrinter()
{
	using namespace CHardware::Printer;

	setConfigParameter(RetractorEnable, true);
	setConfigParameter(Settings::PreviousReceipt, Values::Retract);
	setConfigParameter(Settings::NotTakenReceipt, Values::Retract);

	mModels = CSTAR::getEjectorModels();
}

//--------------------------------------------------------------------------------
QStringList EjectorStarPrinter::getModelList()
{
	return CSTAR::getEjectorModels();
}

//--------------------------------------------------------------------------------
bool EjectorStarPrinter::retract()
{
	return StarPrinter::retract() && waitEjectorState(false);
}

//--------------------------------------------------------------------------------
