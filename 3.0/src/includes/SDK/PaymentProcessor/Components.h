/* @file Список компонентов для разработки плагинов. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
/// Все расширения для ПП должны иметь соответствующий идентификатор приложения.
const char Application[] = "PaymentProcessor";

//---------------------------------------------------------------------------
/// Список возможных компонентов, расширяющих функциональность приложения.
namespace CComponents
{
	const char GraphicsBackend[] = "GraphicsBackend";
	const char PaymentFactory[] = "PaymentFactory";
	const char RemoteClient[] = "RemoteClient";
	const char GraphicsItem[] = "GraphicsItem";
	const char ScenarioFactory[] = "ScenarioFactory";
	const char ChargeProvider[] = "ChargeProvider";
	const char Hook[] = "Hook";
	const char AdSource[] = "AdSource";
	const char FiscalRegister[] = "FiscalRegister";
	const char ScriptFactory[] = "ScriptFactory";
	const char CoreItem[] = "CoreItem";
}

//---------------------------------------------------------------------------
}} // SDK::PaymentProcessor

//---------------------------------------------------------------------------

