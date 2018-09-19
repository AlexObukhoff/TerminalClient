/* @file Данные валюты устройств приема денег на протоколе ccTalk. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/CashAcceptor/Par.h>

// Project
#include "Hardware/Common/Specifications.h"
#include "Hardware/CashAcceptors/CashAcceptorStatusCodes.h"

/// Константы, команды и коды состояний устройств на протоколе ccTalk.
namespace CCCTalk
{
	struct SCurrencyData
	{
		QString country;
		int code;

		SCurrencyData() : country(""), code(Currency::NoCurrency) {}
		SCurrencyData(const QString & aCountry, int aCode) : country(aCountry), code(aCode) {}
	};

	const char TeachMode[] = "TM";

	class CCurrencyData : public CSpecification<QByteArray, SCurrencyData>
	{
	public:
		CCurrencyData()
		{
			append("AL" ,"Albania");
			append("AD" ,"Andorra", Currency::EUR);
			append("AT" ,"Austria", Currency::EUR);
			append("BE" ,"Belgium", Currency::EUR);
			append("BA" ,"Bosnia Herzegovina");
			append("BG" ,"Bulgaria");
			append("HR" ,"Croatia");
			append("CZ" ,"Czech Republic");
			append("DK" ,"Denmark");
			append("EE" ,"Estonia");
			append("EU" ,"Europe", Currency::EUR);
			append("FI" ,"Finland", Currency::EUR);
			append("FR" ,"France", Currency::EUR);
			append("GI" ,"Gibraltar");
			append("DE" ,"Germany", Currency::EUR);
			append("GR" ,"Greece", Currency::EUR);
			append("HU" ,"Hungary");
			append("IS" ,"Iceland");
			append("IE" ,"Irish Republic", Currency::EUR);
			append("IL" ,"Israel");
			append("IT" ,"Italy", Currency::EUR);
			append("LV" ,"Latvia");
			append("LI" ,"Liechtenstein");
			append("LT" ,"Lithuania");
			append("LU" ,"Luxembourg", Currency::EUR);
			append("MK" ,"Macedonia");
			append("MD" ,"Moldova", Currency::MDL);
			append("MC" ,"Monaco", Currency::EUR);
			append("NL" ,"Netherlands", Currency::EUR);
			append("NO" ,"Norway");
			append("PL" ,"Poland");
			append("PT" ,"Portugal", Currency::EUR);
			append("RO" ,"Romania");
			append("SM" ,"San Marino", Currency::EUR);
			append("CS" ,"Serbia & Montenegro");
			append("SK" ,"Slovakia");
			append("SI" ,"Slovenia");
			append("ES" ,"Spain", Currency::EUR);
			append("SE" ,"Sweden");
			append("CH" ,"Switzerland", Currency::CHF);
			append("TR" ,"Turkey");
			append("GB" ,"United Kingdom");
			append("VA" ,"Vatican City", Currency::EUR);
			append("AF" ,"Afghanistan");
			append("DZ" ,"Algeria");
			append("AO" ,"Angola");
			append("AQ" ,"Antarctica");
			append("AR" ,"Argentina");
			append("AM" ,"Armenia");
			append("AU" ,"Australia");
			append("AZ" ,"Azerbaijan");
			append("BH" ,"Bahrain");
			append("BD" ,"Bangladesh");
			append("BT" ,"Bhutan");
			append("BY" ,"Belarus");
			append("BZ" ,"Belize");
			append("BJ" ,"Benin");
			append("BO" ,"Bolivia");
			append("BW" ,"Botswana");
			append("BR" ,"Brazil");
			append("BN" ,"Brunei");
			append("BF" ,"Burkina Faso");
			append("BI" ,"Burundi");
			append("KH" ,"Cambodia");
			append("CM" ,"Cameroon");
			append("CA" ,"Canada", Currency::CAD);
			append("CF" ,"Central African Republic");
			append("TD" ,"Chad");
			append("CL" ,"Chile");
			append("CN" ,"China", Currency::CNY);
			append("CO" ,"Columbia");
			append("CG" ,"Congo");
			append("CR" ,"Costa Rica");
			append("CI" ,"Cote D’Ivoire");
			append("DJ" ,"Djibouti");
			append("TP" ,"East Timor");
			append("EC" ,"Ecuador");
			append("EG" ,"Egypt");
			append("SV" ,"El Salvador");
			append("ER" ,"Eritrea");
			append("ET" ,"Ethiopia");
			append("GQ" ,"Equatorial Guinea");
			append("GF" ,"French Guiana");
			append("PF" ,"French Polynesia");
			append("GA" ,"Gabon");
			append("GM" ,"Gambia");
			append("GE" ,"Georgia");
			append("GH" ,"Ghana");
			append("GL" ,"Greenland");
			append("GT" ,"Guatemala");
			append("GN" ,"Guinea");
			append("GW" ,"Guinea-Bissau");
			append("GY" ,"Guyana");
			append("HN" ,"Hondura");
			append("HK" ,"Hong Kong");
			append("IN" ,"India");
			append("ID" ,"Indonesia");
			append("IR" ,"Iran", Currency::IRR);
			append("IQ" ,"Iraq");
			append("JP" ,"Japan");
			append("JO" ,"Jordan");
			append("KZ" ,"Kazakhstan", Currency::KZT);
			append("KE" ,"Kenya");
			append("KP" ,"Korea North");
			append("KR" ,"Korea South");
			append("KW" ,"Kuwait");
			append("KG" ,"Kyrgyzstan");
			append("LA" ,"Laos");
			append("LB" ,"Lebanon");
			append("LS" ,"Lesotho");
			append("LR" ,"Liberia");
			append("LY" ,"Libya");
			append("MO" ,"Macau");
			append("MY" ,"Malaysia");
			append("MW" ,"Malawi");
			append("ML" ,"Mali");
			append("MR" ,"Mauritania");
			append("MX" ,"Mexico");
			append("MN" ,"Mongolia");
			append("MA" ,"Morocco");
			append("MZ" ,"Mozambique");
			append("MM" ,"Myanmar");
			append("NA" ,"Namibia");
			append("NP" ,"Nepal");
			append("NZ" ,"New Zealand");
			append("NI" ,"Nicaragua");
			append("NE" ,"Niger");
			append("NG" ,"Nigeria");
			append("OM" ,"Oman");
			append("PK" ,"Pakistan");
			append("PA" ,"Panama");
			append("PG" ,"Papua New Guinea");
			append("PY" ,"Paraguay");
			append("PE" ,"Peru");
			append("PH" ,"Philippines");
			append("PR" ,"Puerto Rico");
			append("QA" ,"Qatar");
			append("RU" ,"Russia", Currency::RUB);
			append("RW" ,"Rwanda");
			append("WS" ,"Samoa");
			append("SA" ,"Saudi Arabia");
			append("SN" ,"Senegal");
			append("SL" ,"Sierra Leone");
			append("SG" ,"Singapore");
			append("SO" ,"Somalia");
			append("ZA" ,"South Africa");
			append("SD" ,"Sudan");
			append("SR" ,"Suriname");
			append("SZ" ,"Swaziland");
			append("SY" ,"Syria");
			append("TJ" ,"Tajikistan");
			append("TZ" ,"Tanzania");
			append("TH" ,"Thailand");
			append("TW" ,"Taiwan");
			append("TG" ,"Togo");
			append("TN" ,"Tunisia");
			append("TM" ,"Turkmenistan");
			append("UG" ,"Uganda");
			append("UA" ,"Ukraine", Currency::UAH);
			append("AE" ,"United Arab Emirates");
			append("US" ,"United States", Currency::USD);
			append("UY" ,"Uruguay");
			append("UZ" ,"Uzbekistan", Currency::UZS);
			append("VE" ,"Venezuela");
			append("EH" ,"Western Sahara");
			append("VN" ,"Vietnam");
			append("YE" ,"Yemen");
			append("ZR" ,"Zaire");
			append("ZM" ,"Zambia");
			append("ZW" ,"Zimbabwe");
			append("AS" ,"American Samoa");
			append("AI" ,"Anguilla");
			append("AG" ,"Antigua & Barbuda");
			append("AW" ,"Aruba");
			append("BS" ,"Bahamas");
			append("BB" ,"Barbados");
			append("BM" ,"Bermuda");
			append("QQ" ,"Bonaire");
			append("BV" ,"Bouvet Island");
			append("CV" ,"Cape Verde");
			append("KY" ,"Cayman Islands");
			append("CX" ,"Christmas Island");
			append("CC" ,"Cocos (Keeling) Islands");
			append("KM" ,"Comoros");
			append("CK" ,"Cook Islands");
			append("CU" ,"Cuba");
			append("CY" ,"Cyprus");
			append("DM" ,"Dominica");
			append("DO" ,"Dominican Republic");
			append("EA" ,"East Caribbean");
			append("FK" ,"Falkland Islands/Malvinas");
			append("FO" ,"Faroe Islands");
			append("FJ" ,"Fiji");
			append("JM" ,"Jamaica");
			append("JS" ,"Jersey");
			append("GB" ,"Jersey or Guernse");
			append("GD" ,"Grenada");
			append("GP" ,"Guadeloupe");
			append("GU" ,"Guam");
			append("GS" ,"Guernse");
			append("HT" ,"Haiti");
			append("HM" ,"Heard & McDonald Islands");
			append("IM" ,"Isle of Man");
			append("KI" ,"Kiribati");
			append("MG" ,"Madagascar");
			append("MV" ,"Maldives");
			append("MT" ,"Malta");
			append("MH" ,"Marshall Islands");
			append("MQ" ,"Martinique");
			append("MU" ,"Mauritius");
			append("YT" ,"Mayotte");
			append("FM" ,"Federative States of Micronesia");
			append("MS" ,"Montserrat");
			append("NR" ,"Nauru");
			append("AN" ,"Netherlands Antilles");
			append("NC" ,"New Caledonia");
			append("NU" ,"Niue");
			append("NF" ,"Norfolk Island");
			append("MP" ,"Northern Mariana Islands");
			append("PW" ,"Palau");
			append("PN" ,"Pitcairn");
			append("RE" ,"Reunion");
			append("ST" ,"Sao Tome and Principe");
			append("SC" ,"Seychelles");
			append("SB" ,"Solomon Islands");
			append("LK" ,"Sri Lanka");
			append("KN" ,"Saint Kitts & Nevis");
			append("SH" ,"Saint Helena");
			append("LC" ,"Saint Lucia");
			append("PM" ,"Saint Pierre & Miquelon");
			append("VC" ,"Saint Vincent & Grenadines");
			append("SJ" ,"Svalbard & Jan Mayen Islands");
			append("TK" ,"Tokelau");
			append("TO" ,"Tonga");
			append("TT" ,"Trinidad & Tobago");
			append("TC" ,"Turks & Caicos");
			append("TV" ,"Tuvalu");
			append("VU" ,"Vanuatu");
			append("VG" ,"Virgin Islands (GB)");
			append("VI" ,"Virgin Islands (US)");
			append("WF" ,"Wallis & Futuna");
		}

	private:
		void append(const QByteArray & aCountryCode, const QString & aCountry, int aCode = Currency::NoCurrency) { mBuffer.insert(aCountryCode, SCurrencyData(aCountry, aCode)); }
	};

	static CCurrencyData CurrencyData;
}

//--------------------------------------------------------------------------------
