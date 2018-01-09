/* @file Класс - описание рекламной кампании. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
namespace Ad
{
	const char DefaultChannelPostfix[] = "_default";

	struct Campaign
	{
		qint64 id;            /// ID кампании
		QString type;         /// Тип рекламы
		QUrl url;             /// Url для скачивания контента кампании
		QString md5;          /// Контрольная сумма файла контента
		QDateTime expired;    /// Дата окончания кампании
		QString text;         /// Текст рекламной кампании

		Campaign() : id(-1) {}

		/// Проверка, валидна ли кампания
		bool isValid() const
		{
			return !type.isEmpty() && id > 0;
		}

		/// Возвращает true, если кампания баннерная
		bool isBanner() const
		{
			return (type.contains("banner", Qt::CaseInsensitive) == true);
		}

		/// Проверка, это кампания по умолчанию?
		bool isDefault() const
		{
			return type.endsWith(DefaultChannelPostfix, Qt::CaseInsensitive);
		}

		/// Сравнение двух кампаний
		bool isEqual(const Campaign & aCompaign) const
		{
			return
				id == aCompaign.id &&
				type == aCompaign.type &&
				url.toString() == aCompaign.url.toString() &&
				md5 == aCompaign.md5 &&
				text == aCompaign.text &&
				expired == aCompaign.expired;
		}

		/// Принятие решения о необходимости скачать
		bool isDownloaded() const
		{
			return !url.isEmpty() && !md5.isEmpty();
		}

		bool isExpired() const
		{
			return QDateTime::currentDateTime() > expired || expired.isNull();
		}

		QString toString()
		{
			QStringList result;
			result 
				<< "id:" << QString::number(id) << " "
				<< "CH:" << type << " "
				<< "URL:" << url.toString() << " "
				<< "md5:" << md5 << " "
				<< "exp:" << expired.toString() << " "
				<< "text:'" << text << "' ";

			return result.join("");
		}
	};
}
