/* @file Цифровые коды валют. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
/// Список валют согласно российскому классификатору. Значения соответствуют кодам ISO-4217.
namespace Currency
{
	/// No currency.
	const int NoCurrency = 999;

	/// Россия.
	const int RUB = 643;

	/// Россия.
	const int RUR = 810;

	/// Евро.
	const int EUR = 978;

	/// Доллар США.
	const int USD = 840;

	/// Канадский доллар.
	const int CAD = 124;

	/// Украина.
	const int UAH = 980;

	/// Казахстан.
	const int KZT = 398;

	/// Узбекистан.
	const int UZS = 860;

	/// Молдова.
	const int MDL = 498;

	/// Венгрия.
	const int HUF = 348;

	/// Швейцария.
	const int CHF = 756;

	/// Китай.
	const int CNY = 156;

	/// Индия.
	const int INR = 356;

	/// Иранский риал.
	const int IRR = 364;
}

//--------------------------------------------------------------------------------
