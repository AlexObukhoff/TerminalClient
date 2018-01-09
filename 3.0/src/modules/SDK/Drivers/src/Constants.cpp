/* @file Инициализация констант. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QEvent>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/ICashAcceptor.h>
#include <SDK/Drivers/IHID.h>
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/IWatchdog.h>
#include <SDK/Drivers/ICardReader.h>
#include <SDK/Drivers/IDispenser.h>

//--------------------------------------------------------------------------------
namespace SDK {
namespace Driver {

const char * IDevice::StatusSignal = SIGNAL(status(SDK::Driver::EWarningLevel::Enum, const QString &, int));
const char * IDevice::InitializedSignal = SIGNAL(initialized());
const char * IDevice::UpdatedSignal = SIGNAL(updated(bool));
const char * IDevice::ConfigurationChangedSignal = SIGNAL(configurationChanged());

const char * ICashAcceptor::EscrowSignal = SIGNAL(escrow(SDK::Driver::SPar));
const char * ICashAcceptor::StackedSignal = SIGNAL(stacked(SDK::Driver::TParList));

const char * IHID::DataSignal = SIGNAL(data(const QVariantMap &));

const char * IFiscalPrinter::FRSessionClosedSignal = SIGNAL(FRSessionClosed(const QVariantMap &));

const char * ICardReader::InsertedSignal = SIGNAL(inserted(SDK::Driver::ECardType::Enum, const QVariantMap &));
const char * ICardReader::EjectedSignal = SIGNAL(ejected());

const char * IDispenser::DispensedSignal = SIGNAL(dispensed(int, int));
const char * IDispenser::RejectedSignal = SIGNAL(rejected(int, int));
const char * IDispenser::UnitEmptySignal = SIGNAL(unitEmpty(int));
const char * IDispenser::UnitsDefinedSignal = SIGNAL(unitsDefined());

const char * IWatchdog::KeyRegisteredSignal = SIGNAL(keyRegistered(bool));

/// Регистрация своих типов.

int Type1 = qRegisterMetaType<SDK::Driver::EWarningLevel::Enum>();
int Type2 = qRegisterMetaType<SDK::Driver::ECashAcceptorStatus::Enum>();
int Type4 = qRegisterMetaType<SDK::Driver::SPar>();
int Type5 = qRegisterMetaType<SDK::Driver::TParList>();
int Type6 = qRegisterMetaType<SDK::Driver::ECardType::Enum>();

}} // namespace SDK::Driver
