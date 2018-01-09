/* @file Плагин c драйверами монетоприемников. */

#include "Hardware/Plugins/CommonParameters.h"
#include "Hardware/CoinAcceptors/CoinAcceptorDevices.h"

using namespace SDK::Driver;
using namespace SDK::Plugin;

//------------------------------------------------------------------------------
template <class T>
IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstancePath)
{
	return new DevicePluginBase<T>(CComponents::CoinAcceptor, aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList EnumParameters()
{
	return createNamedList<T>(T::getModelList(), CComponents::CoinAcceptor)
		<< setProtocol(ProtocolNames::CashAcceptor::CCTalk)

		// ID валюты. TODO: необходимо получать поддержку валют статически от каждого протокола.
		<< SPluginParameter(
			CHardwareSDK::CashAcceptor::SystemCurrencyId,
			false, QT_TRANSLATE_NOOP("BillAcceptorParameters", "BillAcceptorParameters#system_currency_id"), QString(), QVariant(),
			QStringList() << "643" << "810" << "978" << "840" << "980" << "398" << "756" << "356" << "364");
}

//------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
	SIMPLE_COMMON_DRIVER(CCTalkCoinAcceptorBase, EnumParameters)
	SIMPLE_COMMON_DRIVER(ComplexEnableCCTalkCoinAcceptor, EnumParameters)
	//COMMON_COIN_ACCEPTOR_PLUGIN(NPSTalkCoinAcceptor)   // отключено из-за отсутствия возможности отключения отдельных номиналов и других критичных проблем
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
