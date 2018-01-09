/* @file Константы логики адаптивной печати фискального чека. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
namespace CSparkFR
{
	/// Текстовые реквизиты для ПФД.
	namespace TextProperties
	{
		/// Номера реквизитов с максимальной длиной поля
		typedef QList<int> TNumbers;
		const TNumbers Numbers = TNumbers() << 5 << 6 << 9 << 10 << 11 << 12 << 15;

		/// Количество текстовых реквизитов для нефискальных данных
		const int NumberSize = Numbers.size();

		/// Префикс
		const char Prefix = '\xA6';

		/// Максимальный размер
		const int MaxSize = 188;

		/// Максимальный размер списка строк
		const int MaxListSize = 4;

		/// Разделитель строк в пределах 1 реквизита
		const QString Separator = "\n" + QString(3, Prefix) + " ";

		/// Размер строки с учетом разделителя
		const int LineSize = 44;

		/// Откуда удалять разделители
		namespace EPosition
		{
			enum Enum
			{
				No = 0,    /// Не удалять
				Up,        /// Сверху
				Down,      /// Снизу
				Bound,     /// Сверху и снизу
				AllOver    /// Везде
			};
		}
	}
}

//--------------------------------------------------------------------------------
