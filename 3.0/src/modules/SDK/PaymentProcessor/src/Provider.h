/* @file Описание платёжного оператора. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QVariantMap>
#include <QtCore/QSet>
#include <QtCore/QUrl>
#include <Common/QtHeadersEnd.h>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
/// Структура описывает одно поле с данными, необходимое для формирования платежа.
struct SProviderField
{
	/// Структура описывает элемент списка возможных значения для поля типа "список".
	struct SEnumItem
	{
		QString title;
		QString value;
		QString id;
		int     sort;

		QList<SEnumItem> subItems;

		SEnumItem() : sort(0) {}
	};

	typedef QList<SEnumItem> TEnumItems;

	/// Подсистемы для маскирования поля
	enum SecuritySubsystem
	{
		Default,
		Display,
		Log,
		Printer
	};

	SProviderField()
		: sort(0),
		  minSize(-1),
		  maxSize(-1),
		  isRequired(true),
		  isPassword(false)
	{
	}

	QString type;            /// Тип поля.
	QString id;              /// Идентификатор поля.

	QString keyboardType;    /// Название шаблона клавиатуры для данного типа поля.
	QString letterCase;      /// Регистр символов.
	QString language;        /// Язык.

	int sort;                /// Порядок следования поля в интерфейсе
	int minSize;             /// Минимальный размер поля.
	int maxSize;             /// Максимальный размер поля.

	bool isRequired;         /// Обязательность к заполнению.

	QString title;           /// Заголовок поля.
	QString comment;         /// Комментарий.
	QString extendedComment; /// Расширенный комментарий.
	QString mask;            /// Маска.
	bool isPassword;         /// Отображать ли поле как пароль.
	QString behavior;        /// Управление отображением. Возможные варианты readonly:hidden
	QString format;          /// Формат поля.

	QString url;             /// Путь к html-файлу.
	QString html;            /// Внедренный html.
	QString backButton;
	QString forwardButton;

	TEnumItems enumItems;    /// Возможные значения для спискового поля.

	QString defaultValue;    /// Значение по умолчанию.

	QString dependency;      /// Условие показа.

	QMap<SecuritySubsystem, QString> security; /// Маски безопастности для разных подсистем

	/// Возвращает признак, шифровать ли поля для хранения в БД
	bool keepEncrypted() const { return isPassword || !security.isEmpty(); }
};

typedef QList<SProviderField> TProviderFields;

//------------------------------------------------------------------------------
/// Структура описывает стандартного провайдера для оплаты.
struct SProvider
{
	enum FeeType
	{
		FeeByAmount = 0,
		FeeByAmountAll
	};

	struct SLimits
	{
		QString min;
		QString max;
		QString system;
		QString check;

		// Лимиты, выставляемые снаружи
		QString externalMin;
		QString externalMax;
	};

	struct SProcessingTraits
	{
		struct SRequest
		{
			struct SField
			{
				SField()
				{
					crypted = false;
					algorithm = Ipriv;
				}

				QString name;   /// имя поля
				QString value;  /// значение поля

				bool crypted;   /// флаг, что поле зашифровано
				enum {
					Ipriv,
					Md5,
					Sha1,
					Sha256
				} algorithm; /// алгоритм шифрования поля
			};

			struct SResponseField : SField
			{
				typedef enum 
				{
					Windows1251,
					Utf8
				} Codepage;

				typedef enum
				{
					Text,
					Url,
					Base64
				} Encoding;

				SResponseField();
				
				void setCodepage(const QString & aCodepage);
				void setEncoding(const QString & aEncoding);
				
				bool required;     /// Флаг, что параметр обязательный
				Encoding encoding; /// Кодировка параметра
				Codepage codepage; /// Кодовая страница
			};

			typedef QList<SField> TFields;
			typedef QList<SResponseField> TResponseField;

			QString url;
			TFields requestFields;
			TResponseField responseFields;
		};

		typedef QMap<QString, SRequest> TRequests;

		SProcessingTraits()
		{
			keyPair = 0;
			clientCard = 0;
			feeType = FeeByAmount;
			skipCheck = payOnline = askForRetry = requirePrinter = rounding = showAddInfo = false;
		}

		int keyPair;
		int clientCard;

		QString type;

		FeeType feeType;

		bool skipCheck;
		bool payOnline;
		bool askForRetry;
		bool requirePrinter;
		bool rounding;
		bool showAddInfo;

		TRequests requests;
	};

	SProvider()
		: id(-1),
		  cid(-1)
	{
	}

	bool isNull() const
	{
		return (id == -1);
	}

	qint64 id;
	qint64 cid;
	QSet<qint64> ttList;

	/// Ограничения по суммам.
	SLimits limits;

	/// Настройки процессинга.
	SProcessingTraits processor;

	/// Тип оператора по приёму/переводу денежных средств ("bank", "cyberplat").
	QString type;

	QString name;
	QString comment;

	/// Поля данных.
	TProviderFields fields;

	/// Реакция на внешние данные (сканер штрих-кода, кард ридер и т.п.).
	QString externalDataHandler;

	/// Типы чеков и чековые параметры.
	QVariantMap receipts;
	QVariantMap receiptParameters;

	/// конвертация списка полей из/в json
	static QString fields2Json(const TProviderFields & aFields);
	static TProviderFields json2Fields(const QString & aJson);
};

//------------------------------------------------------------------------------
}} // PaymentProcessor::SDK
