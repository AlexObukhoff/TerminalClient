/* @file Плагин c драйверами валидаторов. */

#include "Hardware/Plugins/DevicePluginBase.h"
#include "Hardware/Plugins/CommonParameters.h"
#include "Hardware/CashAcceptors/CashAcceptorDevices.h"

using namespace SDK::Driver;
using namespace SDK::Plugin;

//------------------------------------------------------------------------------
template <class T>
IPlugin * CreatePlugin(SDK::Plugin::IEnvironment * aEnvironment, const QString & aInstancePath)
{
	return new DevicePluginBase<T>(SDK::Driver::CComponents::BillAcceptor, aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <class T1, class T2>
TParameterList defaultParameters(const QString & aProtocol, const T2 & aModelList)
{
	return createNamedList<T1>(aModelList, CComponents::BillAcceptor)
		<< setProtocol(aProtocol)

		// ID валюты. TODO: необходимо получать поддержку валют статически от каждого протокола.
		<< SPluginParameter(
			CHardwareSDK::CashAcceptor::SystemCurrencyId,
			false, QT_TRANSLATE_NOOP("BillAcceptorParameters", "BillAcceptorParameters#system_currency_id"), QString(), QVariant(),
			QStringList() << "643" << "810" << "978" << "840" << "980" << "398" << "756" << "356" << "364");
}

//------------------------------------------------------------------------------
template <class T1, class T2>
TParameterList ID003Parameters(const QString & aProtocol, const T2 & aModelList)
{
	QVariantMap modelNames;
	modelNames.insert("JCM iVISION", "JCM IVISION");

	return defaultParameters<T1, T2>(aProtocol, aModelList)
		<< setModifiedValues(CHardwareSDK::ModelName, modelNames);
}

// Регистрация плагина.
#define COMMON_CASH_ACCEPTOR_PLUGIN(aClassName, aProtocol, aParameters) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName, QStringList>, ProtocolNames::CashAcceptor::aProtocol, aClassName::getModelList()))
#define SINGLE_CASH_ACCEPTOR_PLUGIN(aClassName, aProtocol, aModel) COMMON_DRIVER(aClassName, std::bind(&defaultParameters<aClassName, QString>, ProtocolNames::CashAcceptor::aProtocol, aModel))
#define CCNET_CASH_ACCEPTOR_PLUGIN(aClassName, aModel) SINGLE_CASH_ACCEPTOR_PLUGIN(aClassName, CCNet, CCCNet::Models::aModel)

BEGIN_REGISTER_PLUGIN
	COMMON_CASH_ACCEPTOR_PLUGIN(CCNetCashAcceptorBase, CCNet, defaultParameters)
	CCNET_CASH_ACCEPTOR_PLUGIN(CCNetCashcodeGX, CashcodeGX)
	CCNET_CASH_ACCEPTOR_PLUGIN(CCNetCreator, CreatorC100)
	CCNET_CASH_ACCEPTOR_PLUGIN(CCNetRecycler, CashcodeG200)

	COMMON_CASH_ACCEPTOR_PLUGIN(EBDSCashAcceptor,  EBDS,  defaultParameters)
	COMMON_CASH_ACCEPTOR_PLUGIN(ICTCashAcceptor,   ICT,   defaultParameters)
	COMMON_CASH_ACCEPTOR_PLUGIN(ID003CashAcceptor, ID003, ID003Parameters)
	COMMON_CASH_ACCEPTOR_PLUGIN(V2eCashAcceptor,   V2e,   defaultParameters)
	//COMMON_CASH_ACCEPTOR_PLUGIN(SSPCashAcceptor,   SSP,   defaultParameters)
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
