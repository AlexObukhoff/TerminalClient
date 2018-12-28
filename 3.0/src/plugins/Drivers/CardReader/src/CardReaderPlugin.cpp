/* @file Плагин с драйверами кардридеров. */

// Modules
#include "Hardware/Plugins/CommonParameters.h"
#include "../../../../modules/Hardware/Cardreaders/src/Creator/CreatorReader.h"
#include "../../../../modules/Hardware/Cardreaders/src/IDTech/IDTechReader.h"

// Project
#include "MifareReader.h"

using namespace SDK::Plugin;
using namespace SDK::Driver;

//------------------------------------------------------------------------------
template<class T>
IPlugin * CreatePlugin(IEnvironment * aEnvironment, const QString & aInstancePath)
{
	return new DevicePluginBase<T>("Card readers", aEnvironment, aInstancePath);
}

//------------------------------------------------------------------------------
template <class T>
TParameterList EnumParameters()
{
	return createNamedList<T>(T::getModelList(), CComponents::CardReader);
}

//------------------------------------------------------------------------------
BEGIN_REGISTER_PLUGIN
	SIMPLE_COMMON_DRIVER(MifareReader, EnumParameters)
	SIMPLE_COMMON_DRIVER(CreatorReader, EnumParameters)
	//SIMPLE_COMMON_DRIVER(IDTechReader, EnumParameters)
END_REGISTER_PLUGIN

//------------------------------------------------------------------------------
