/* @file Константы, коды команд и ответов протокола ФР АТОЛ для ФР с эжектором. */

#pragma once

#include "../AtolFRConstants.h"

//------------------------------------------------------------------------------------------------
namespace CAtolEjectorFR
{
	/// Теги.
	class TagEngine : public CAtolFR::TagEngine
	{
	public:
		TagEngine()
		{
			set(Tags::Type::Image);
		}
	};

	/// Печать изображений
	namespace ImageProcessing
	{
		/// Максимальная ширина печатаемой части изображения, [пикс] (1 пикс == 1 бит).
		const int MaxWidth = 608;

		/// Высота линии изображения, [пикс].
		const int LineHeight = 8;

		/// Максимальный размер блока данных, [байт].
		const int MaxBlockSize = 50;

		/// Максимальный размер печатаемой части картинки, [байт].
		const int MaxPartSize = 8192;

		/// Минимальное количество блоков в печатаемой части картинки.
		const int MinBlocksInPart = MaxPartSize / MaxBlockSize;

		/// Данные команды инициализации записи изображения.
		const char InitData[] = "\x8F\x1D\x2A";

		/// Данные команды обнуления счетчика байтов.
		const char ClearCounterData[] = "\x90\x01";

		/// Данные команды обнуления счетчика байтов.
		const QByteArray WriteData = QByteArray::fromRawData("\x8F\x1D\x2F\x00", 4);
	}
}

//--------------------------------------------------------------------------------
