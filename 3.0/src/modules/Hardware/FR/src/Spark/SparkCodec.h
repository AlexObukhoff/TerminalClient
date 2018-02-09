/* @file Кодек СПАРК-а. */

#pragma once

#include "Hardware/Common/CodecBase.h"

//---------------------------------------------------------------------------
class SparkTextCodec : public CodecBase
{
public:
	SparkTextCodec()
	{
		mName = CHardware::Codepages::SPARK;
		mMIB = 3001;

		mData.add('\x80', "А");               mData.add('\x90', "П");               mData.add('\xA0', "Я");               mData.add('\xB0', "а");
		mData.add('\x81', "Б");               mData.add('\x91', "Р");               mData.add('\xA1', "«");               mData.add('\xB1', "б");
		mData.add('\x82', "В");               mData.add('\x92', "С");               mData.add('\xA2', "»");               mData.add('\xB2', "в");
		mData.add('\x83', "Г");               mData.add('\x93', "Т");               mData.add('\xA3', "€");               mData.add('\xB3', "г");
		mData.add('\x84', "Д");               mData.add('\x94', "У");               mData.add('\xA4', "€", false);        mData.add('\xB4', "д");
		mData.add('\x85', "Е");               mData.add('\x95', "Ф");               mData.add('\xA5', "Σ");               mData.add('\xB5', "е");
		mData.add('\x86', "Ё");               mData.add('\x96', "Х");               mData.add('\xA6', ".", false);        mData.add('\xB6', "Ё");
		mData.add('\x87', "Ж");               mData.add('\x97', "Ц");               mData.add('\xA7', "º");               mData.add('\xB7', "ж");
		mData.add('\x88', "З");               mData.add('\x98', "Ч");               mData.add('\xA8', "°");               mData.add('\xB8', "з");
		mData.add('\x89', "И");               mData.add('\x99', "Ш");               mData.add('\xA9', "И", false);        mData.add('\xB9', "и");
		mData.add('\x8A', "Й");               mData.add('\x9A', "Щ");               mData.add('\xAA', "Т", false);        mData.add('\xBA', "й");
		mData.add('\x8B', "К");               mData.add('\x9B', "Ъ");               mData.add('\xAB', "О", false);        mData.add('\xBB', "к");
		mData.add('\x8C', "Л");               mData.add('\x9C', "Ы");               mData.add('\xAC', "Г", false);        mData.add('\xBC', "л");
		mData.add('\x8D', "М");               mData.add('\x9D', "Ь");               mData.add('\xAD', "▬");               mData.add('\xBD', "м");
		mData.add('\x8E', "Н");               mData.add('\x9E', "Э");               mData.add('\xAE', "▬", false);        mData.add('\xBE', "н");
		mData.add('\x8F', "О");               mData.add('\x9F', "Ю");               mData.add('\xAF');                    mData.add('\xBF', "о");

		mData.add('\xC0', "п");               mData.add('\xD0', "я");               mData.add('\xE0', "⁴");               mData.add('\xF0', "=", false);
		mData.add('\xC1', "р");               mData.add('\xD1');                    mData.add('\xE1', "⁵");               mData.add('\xF1');
		mData.add('\xC2', "с");               mData.add('\xD2', "0", false);        mData.add('\xE2', "⁶");               mData.add('\xF2');
		mData.add('\xC3', "т");               mData.add('\xD3', "1", false);        mData.add('\xE3', "⁷");               mData.add('\xF3');
		mData.add('\xC4', "у");               mData.add('\xD4', "2", false);        mData.add('\xE4', "⁸");               mData.add('\xF4');
		mData.add('\xC5', "ф");               mData.add('\xD5', "3", false);        mData.add('\xE5', "⁹");               mData.add('\xF5');
		mData.add('\xC6', "х");               mData.add('\xD6', "4", false);        mData.add('\xE6', ":", false);        mData.add('\xF6', "|");
		mData.add('\xC7', "ц");               mData.add('\xD7', "5", false);        mData.add('\xE7', "█");               mData.add('\xF7');
		mData.add('\xC8', "ч");               mData.add('\xD8', "6", false);        mData.add('\xE8', "√");               mData.add('\xF8');
		mData.add('\xC9', "ш");               mData.add('\xD9', "7", false);        mData.add('\xE9');                    mData.add('\xF9', "½");
		mData.add('\xCA', "щ");               mData.add('\xDA', "8", false);        mData.add('\xEA');                    mData.add('\xFA', "≡");
		mData.add('\xCB', "ъ");               mData.add('\xDB', "9", false);        mData.add('\xEB', "◙");               mData.add('\xFB');
		mData.add('\xCC', "ы");               mData.add('\xDC', "⁰");               mData.add('\xEC');                    mData.add('\xFC');
		mData.add('\xCD', "ь");               mData.add('\xDD', "¹");               mData.add('\xED', "▬", false);        mData.add('\xFD');
		mData.add('\xCE', "э");               mData.add('\xDE', "²");               mData.add('\xEE');                    mData.add('\xFE');
		mData.add('\xCF', "ю");               mData.add('\xDF', "³");               mData.add('\xEF', "×");               mData.add('\xFF');
	}
};

//---------------------------------------------------------------------------
