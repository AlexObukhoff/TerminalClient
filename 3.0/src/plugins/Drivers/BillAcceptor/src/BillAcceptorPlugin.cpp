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
	return defaultParameters<T1, T2>(aProtocol, aModelList)
		<< setModifiedValues(CHardwareSDK::ModelName, "JCM iVISION", "JCM IVISION");
}

//------------------------------------------------------------------------------
template <class T1, class T2>
TParameterList CCTalkParameters(const QString & aProtocol, const T2 & aModelList)
{
	return defaultParameters<T1, T2>(aProtocol, aModelList)
		<< setProtocolType(CHardware::CashDevice::CCTalkTypes::CRC8, CCCTalk::ProtocolTypes);
}

// Регистрация плагина.
#define COMMON_CASH_ACCEPTOR_PLUGIN(aClassName, aProtocol, aParameters) COMMON_DRIVER(aClassName, std::bind(&aParameters<aClassName, QStringList>, ProtocolNames::aProtocol, aClassName::getModelList()))
#define SINGLE_CASH_ACCEPTOR_PLUGIN(aClassName, aProtocol, aModel) COMMON_DRIVER(aClassName, std::bind(&defaultParameters<aClassName, QString>, ProtocolNames::aProtocol, aModel))
#define CCNET_CASH_ACCEPTOR_PLUGIN(aClassName, aModel) SINGLE_CASH_ACCEPTOR_PLUGIN(aClassName, CashDevice::CCNet, CCCNet::Models::aModel)

BEGIN_REGISTER_PLUGIN
	COMMON_CASH_ACCEPTOR_PLUGIN(CCNetCashAcceptorBase, CashDevice::CCNet, defaultParameters)
	CCNET_CASH_ACCEPTOR_PLUGIN(CCNetCashcodeGX, CashcodeGX)
	CCNET_CASH_ACCEPTOR_PLUGIN(CCNetCreator, CreatorC100)
	CCNET_CASH_ACCEPTOR_PLUGIN(CCNetRecycler, CashcodeG200)

	COMMON_CASH_ACCEPTOR_PLUGIN(EBDSCashAcceptor,   CashAcceptor::EBDS,  defaultParameters)
	COMMON_CASH_ACCEPTOR_PLUGIN(ICTCashAcceptor,    CashAcceptor::ICT,   defaultParameters)
	COMMON_CASH_ACCEPTOR_PLUGIN(ID003CashAcceptor,  CashAcceptor::ID003, ID003Parameters)
	COMMON_CASH_ACCEPTOR_PLUGIN(V2eCashAcceptor,    CashAcceptor::V2e,   defaultParameters)
	COMMON_CASH_ACCEPTOR_PLUGIN(CCTalkCashAcceptor, CashDevice::CCTalk,  CCTalkParameters)
	//COMMON_CASH_ACCEPTOR_PLUGIN(SSPCashAcceptor,  CashDevice::SSP,     defaultParameters)
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
