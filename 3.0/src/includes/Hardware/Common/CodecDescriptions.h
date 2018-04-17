/* @file Описатель кодеков. */

#pragma once

#include "Hardware/FR/AtolCodec.h"
#include "Hardware/FR/SparkCodec.h"
#include "Hardware/Printers/CustomKZTCodec.h"

//---------------------------------------------------------------------------
#define APPEND_CODEC(aName, aCodec) append(CHardware::Codepages::aName, QTextCodec::codecForName(#aCodec))

class CodecDescriptions : public CSpecification<QString, QTextCodec *>
{
public:
	CodecDescriptions()
	{
		static AtolTextCodec atolCodec;
		static SparkTextCodec sparkCodec;
		static CustomKZTCodec customKZTCodec;
		static CodecBase baseCodec;

		APPEND_CODEC(CP850, IBM 850);
		APPEND_CODEC(CP866, IBM 866);
		APPEND_CODEC(Win1250, Windows-1250);
		APPEND_CODEC(Win1251, Windows-1251);
		APPEND_CODEC(Win1252, Windows-1252);
		APPEND_CODEC(ATOL, ATOL);
		APPEND_CODEC(SPARK, SPARK);
		APPEND_CODEC(CustomKZT, CP866 (Kazakhstan));
		APPEND_CODEC(Base, Base);
	}
};

static CodecDescriptions CodecByName;

//---------------------------------------------------------------------------
