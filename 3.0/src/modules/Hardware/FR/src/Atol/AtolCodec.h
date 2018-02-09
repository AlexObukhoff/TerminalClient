/* @file Кодек АТОЛ-а. */

#pragma once

#include "Hardware/Common/CodecBase.h"

//---------------------------------------------------------------------------
class AtolTextCodec : public CodecBase
{
public:
	AtolTextCodec()
	{
		mName = CHardware::Codepages::ATOL;
		mMIB = 3002;
		mMinValueActive = 0;

		mData.add('\x00', "А");               mData.add('\x10', "Р");               mData.add('\x20', " ");               mData.add('\x30', "0");
		mData.add('\x01', "Б");               mData.add('\x11', "С");               mData.add('\x21', "!");               mData.add('\x31', "1");
		mData.add('\x02', "В");               mData.add('\x12', "Т");               mData.add('\x22', "\"");              mData.add('\x32', "2");
		mData.add('\x03', "Г");               mData.add('\x13', "У");               mData.add('\x23', "#");               mData.add('\x33', "3");
		mData.add('\x04', "Д");               mData.add('\x14', "Ф");               mData.add('\x24', "№");               mData.add('\x34', "4");
		mData.add('\x05', "Е");               mData.add('\x15', "Х");               mData.add('\x25', "%");               mData.add('\x35', "5");
		mData.add('\x06', "Ж");               mData.add('\x16', "Ц");               mData.add('\x26', "&");               mData.add('\x36', "6");
		mData.add('\x07', "З");               mData.add('\x17', "Ч");               mData.add('\x27', "’");               mData.add('\x37', "7");
		mData.add('\x08', "И");               mData.add('\x18', "Ш");               mData.add('\x28', "(");               mData.add('\x38', "8");
		mData.add('\x09', "Й");               mData.add('\x19', "Щ");               mData.add('\x29', ")");               mData.add('\x39', "9");
		mData.add('\x0A', "К");               mData.add('\x1A', "Ъ");               mData.add('\x2A', "*");               mData.add('\x3A', ":");
		mData.add('\x0B', "Л");               mData.add('\x1B', "Ы");               mData.add('\x2B', "+");               mData.add('\x3B', ";");
		mData.add('\x0C', "М");               mData.add('\x1C', "Ь");               mData.add('\x2C', ",");               mData.add('\x3C', "<");
		mData.add('\x0D', "Н");               mData.add('\x1D', "Э");               mData.add('\x2D', "-");               mData.add('\x3D', "=");
		mData.add('\x0E', "О");               mData.add('\x1E', "Ю");               mData.add('\x2E', ".");               mData.add('\x3E', ">");
		mData.add('\x0F', "П");               mData.add('\x1F', "Я");               mData.add('\x2F', "/");               mData.add('\x3F', "?");

		mData.add('\x40', "@");               mData.add('\x50', "P");               mData.add('\x60', "`");               mData.add('\x70', "p");
		mData.add('\x41', "A");               mData.add('\x51', "Q");               mData.add('\x61', "a");               mData.add('\x71', "q");
		mData.add('\x42', "B");               mData.add('\x52', "R");               mData.add('\x62', "b");               mData.add('\x72', "r");
		mData.add('\x43', "C");               mData.add('\x53', "S");               mData.add('\x63', "c");               mData.add('\x73', "s");
		mData.add('\x44', "D");               mData.add('\x54', "T");               mData.add('\x64', "d");               mData.add('\x74', "t");
		mData.add('\x45', "E");               mData.add('\x55', "U");               mData.add('\x65', "e");               mData.add('\x75', "u");
		mData.add('\x46', "F");               mData.add('\x56', "V");               mData.add('\x66', "f");               mData.add('\x76', "v");
		mData.add('\x47', "G");               mData.add('\x57', "W");               mData.add('\x67', "g");               mData.add('\x77', "w");
		mData.add('\x48', "H");               mData.add('\x58', "X");               mData.add('\x68', "h");               mData.add('\x78', "x");
		mData.add('\x49', "I");               mData.add('\x59', "Y");               mData.add('\x69', "i");               mData.add('\x79', "y");
		mData.add('\x4A', "J");               mData.add('\x5A', "Z");               mData.add('\x6A', "j");               mData.add('\x7A', "z");
		mData.add('\x4B', "K");               mData.add('\x5B', "[");               mData.add('\x6B', "k");               mData.add('\x7B', "{");
		mData.add('\x4C', "L");               mData.add('\x5C', "\\");              mData.add('\x6C', "l");               mData.add('\x7C', "|");
		mData.add('\x4D', "M");               mData.add('\x5D', "]");               mData.add('\x6D', "m");               mData.add('\x7D', "}");
		mData.add('\x4E', "N");               mData.add('\x5E', "^");               mData.add('\x6E', "n");               mData.add('\x7E', "~");
		mData.add('\x4F', "O");               mData.add('\x5F', "_");               mData.add('\x6F', "o");               mData.add('\x7F');

		mData.add('\x80', "а");               mData.add('\x90', "р");               mData.add('\xA0', "$");               mData.add('\xF0', "Ё");
		mData.add('\x81', "б");               mData.add('\x91', "с");               mData.add('\xA1', "€");               mData.add('\xF1', "ё");
		mData.add('\x82', "в");               mData.add('\x92', "т");               mData.add('\xA2', "—");
		mData.add('\x83', "г");               mData.add('\x93', "у");
		mData.add('\x84', "д");               mData.add('\x94', "ф");
		mData.add('\x85', "е");               mData.add('\x95', "х");
		mData.add('\x86', "ж");               mData.add('\x96', "ц");
		mData.add('\x87', "з");               mData.add('\x97', "ч");
		mData.add('\x88', "и");               mData.add('\x98', "ш");
		mData.add('\x89', "й");               mData.add('\x99', "щ");
		mData.add('\x8A', "к");               mData.add('\x9A', "ъ");
		mData.add('\x8B', "л");               mData.add('\x9B', "ы");
		mData.add('\x8C', "м");               mData.add('\x9C', "ь");
		mData.add('\x8D', "н");               mData.add('\x9D', "э");
		mData.add('\x8E', "о");               mData.add('\x9E', "ю");
		mData.add('\x8F', "п");               mData.add('\x9F', "я");
	}
};

//---------------------------------------------------------------------------
