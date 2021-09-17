/* @file Плагин драйвера виртуальных устройств. */

// Modules
#include "Hardware/Plugins/CommonParameters.h"

// Project
#include "CashAcceptor/VirtualCashAcceptor.h"
#include "CashDispenser/VirtualCashDispenser.h"
#include "Printer/VirtualPrinter.h"

using namespace SDK::Plugin;
using namespace SDK::Driver;

//------------------------------------------------------------------------
namespace VirtualDevicesPluginParameterTranslations
{
	static const char * NotesPerEscrow = QT_TRANSLATE_NOOP("VirtualCashAcceptor", "VirtualCashAcceptor#notes_per_escrow");
	static const char * Units          = QT_TRANSLATE_NOOP("VirtualDispenser", "VirtualDispenser#units");
	static const char * NearEndCount   = QT_TRANSLATE_NOOP("VirtualDispenser", "VirtualDispenser#near_end_count");
	static const char * JammedItem     = QT_TRANSLATE_NOOP("VirtualDispenser", "VirtualDispenser#jammed_item");
};

namespace VDPPT = VirtualDevicesPluginParameterTranslations;

//--------------------------------------------------------------------------------------
template <class T>
IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstanceName)
{
	return new DevicePluginBase<T>("Virtual device", aEnvironment, aInstanceName);
}

//--------------------------------------------------------------------------------------
TParameterList cashAcceptorParameters()
{
	return modifyPriority(createNamedList<VirtualCashAcceptor>("Virtual cash acceptor"), EDetectingPriority::Fallback)
		// Количество зачисляемых одновременно купюр.
		<< SPluginParameter(CHardware::VirtualCashAcceptor::NotesPerEscrow, true, VDPPT::NotesPerEscrow, QString(), "1",
			QStringList() << "1" << "2" << "10" << "100" << "250" << "500" << "1000" << "4000", false);
}

//--------------------------------------------------------------------------------------
TParameterList dispenserParameters()
{
	auto getList = [] (int aStart, int aEnd) -> QStringList
	{
		QStringList result;

		for (int i = aStart; i <= aEnd; i++)
		{
			result << QString::number(i);
		}

		return result;
	};

	return modifyPriority(createNamedList<VirtualDispenser>("Virtual dispenser"), EDetectingPriority::Fallback)
		// Количество кассет.
		<< SPluginParameter(CHardware::Dispenser::Units, true, VDPPT::Units, QString(), 2, getList(1, 4))

		// Номер предмета, когда кассета почти пуста.
		<< SPluginParameter(CHardware::Dispenser::NearEndCount, true, VDPPT::NearEndCount, QString(), 10, getList(0, 15))

		// Номер замятого предмета.
		<< SPluginParameter(CHardware::Dispenser::JammedItem, true, VDPPT::JammedItem, QString(), 5, getList(0, 15));
}

//--------------------------------------------------------------------------------------
TParameterList printerParameters()
{
	return modifyPriority(createNamedList<VirtualPrinter>("Virtual printer"), EDetectingPriority::Fallback);
}

//------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
	COMMON_DRIVER(VirtualCashAcceptor, &cashAcceptorParameters)
	COMMON_DRIVER(VirtualDispenser, &dispenserParameters)
	COMMON_DRIVER(VirtualPrinter, &printerParameters)
END_REGISTER_PLUGIN

//--------------------------------------------------------------------------------------
