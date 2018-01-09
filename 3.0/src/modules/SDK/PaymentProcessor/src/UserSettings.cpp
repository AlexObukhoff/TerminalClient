/* @file Настройки пользователя. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// Boost
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>



// Проект
#include "UserSettings.h"

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
UserSettings::UserSettings(TPtree & aProperties)
	: mProperties(aProperties.get_child(CAdapterNames::UserAdapter, aProperties))
{
}

//---------------------------------------------------------------------------
UserSettings::~UserSettings()
{
}

//---------------------------------------------------------------------------
bool UserSettings::isValid() const
{
	return true;
}

//---------------------------------------------------------------------------
QString UserSettings::getAdapterName()
{
	return CAdapterNames::UserAdapter;
}

//---------------------------------------------------------------------------
bool UserSettings::reportAllPayments() const
{
	return mProperties.get<bool>("user.monitoring.report_all_payments", false);
}

//---------------------------------------------------------------------------
bool UserSettings::useStackerID() const
{
	return mProperties.get<bool>("user.encashment.use_stacker_id", false);
}

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor
