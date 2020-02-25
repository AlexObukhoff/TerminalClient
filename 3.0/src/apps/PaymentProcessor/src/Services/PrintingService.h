/* @file Сервис печати и формирования чеков. */

#pragma once

// STL
#include <algorithm>
#include <array>
#include <functional>
#include <random>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFutureWatcher>
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QVariantMap>
#include <QtCore/QSignalMapper>
#include <QtCore/QWaitCondition>
#include <QtCore/QAtomicInt>
#include <QtCore/QFileInfo>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/Drivers/IPrinter.h>
#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/FR/FiscalPrinterCommand.h>
#include <SDK/PaymentProcessor/FiscalRegister/IFiscalRegister.h>

// Модули
#include <Common/ILogable.h>

// PP
#include "DatabaseUtils/IHardwareDatabaseUtils.h"

class IApplication;
class IPayment;

namespace DeviceStatus = SDK::Driver::EWarningLevel;

//---------------------------------------------------------------------------
class PrintingService : public SDK::PaymentProcessor::IPrinterService, public SDK::PaymentProcessor::IService, private ILogable
{
	Q_OBJECT

	/// Комманды печати чеков определенного типа.
	friend class PrintCommand;
	friend class PrintPayment;
	friend class PrintBalance;
	friend class PrintEncashment;
	friend class PrintZReport;
	friend class PrintReceipt;

public:
	PrintingService(IApplication * aApplication);
	virtual ~PrintingService();

	#pragma region SDK::PaymentProcessor::IService interface

	/// Получение имени сервиса.
	virtual QString getName() const;

	/// Инициализация сервиса.
	virtual bool initialize();

	/// IService: Закончена инициализация всех сервисов.
	virtual void finishInitialize();

	/// Возвращает false, если сервис не может быть остановлен в текущий момент.
	virtual bool canShutdown();

	/// Завершение работы, освобождение ресурсов.
	virtual bool shutdown();

	/// Получение списка зависимостей.
	virtual const QSet<QString> & getRequiredServices() const;

	/// Получить параметры сервиса.
	virtual QVariantMap getParameters() const;

	/// Сброс служебной информации.
	virtual void resetParameters(const QSet<QString> & aParameters);

	#pragma endregion

	#pragma region SDK::PaymentProcessor::IPrinterService interface

	/// Проверка принтера на возможность печати чека.
	virtual bool canPrintReceipt(const QString & aReceiptType, bool aRealCheck);

	/// Печать типизированного чека с параметрами aParameters. Возвращает индекс задания, поставленного в очередь.
	/// Результат придёт в сигнале receiptPrinted.
	virtual int printReceipt(const QString & aReceiptType, const QVariantMap & aParameters, const QString & aReceiptTemplate, DSDK::EPrintingModes::Enum aPrintingMode, bool aServiceOperation = false);

	/// Сохранение электронной версии типизированного чека с параметрами aParameters.
	virtual void saveReceipt(const QVariantMap & aParameters, const QString & aReceiptTemplate);

	/// Загрузить и вернуть содержимое чека по номеру платежа
	virtual QString loadReceipt(qint64 aPaymentId);

	/// Печать тестового чека.
	virtual bool printReceiptDirected(SDK::Driver::IPrinter * aPrinter, const QString & aReceiptTemplate, const QVariantMap & aParameters);

	/// Печать отчета.
	virtual int printReport(const QString & aReceiptType, const QVariantMap & aParameters);

	/// Может ли работать с фискальным сервером?
	virtual bool hasFiscalRegister();

	#pragma endregion

	bool enableBlankFiscalData() const { return mEnableBlankFiscalData; }

public:
	/// Получить объект фискального регистратора
	SDK::PaymentProcessor::IFiscalRegister * getFiscalRegister() const;

	/// Записать в параметры платежа фскальный номер 
	void setFiscalNumber(qint64 aPaymentId, const QVariantMap & aParameters);

	/// Получить настройки валюты
	SDK::PaymentProcessor::SCurrencySettings getCurrencySettings() const;

signals:
	/// Сигнал о состоянии принтера
	void printerStatus(bool aReady);

private:
	/// Возвращает чек определенного тип с заполненными полями.
	QStringList getReceipt(const QString & aReceiptTemplate, const QVariantMap & aFields);

	/// Получить принтер из списка доступных.
	SDK::Driver::IPrinter * takePrinter(const QString & aReceiptType, bool aCheckOnline);

	/// Вернуть принтер в список доступных.
	void giveBackPrinter(SDK::Driver::IPrinter * aPrinter);

	/// Получение команды для печати чека определенного типа.
	PrintCommand * getPrintCommand(const QString & aReceiptType);

	/// Заполнение тегов значениями во всём чеке.
	void expandTags(QStringList & aReceipt, const QVariantMap & aParameters);

	/// Сохраняет чек в файл.
	void saveReceiptContent(const QString & aReceiptName, const QStringList & aContents);

	/// Первоначальная загрузка значений тегов.
	bool loadTags();

	/// Загрузка шаблонов чеков.
	void loadReceiptTemplates();

	/// Загрузка шаблона чека из файла.
	bool loadReceiptTemplate(const QFileInfo & aFileInfo);

	/// Печать чека, возвращает индекс задания, поставленного в очередь.
	int performPrint(PrintCommand * aCommand, const QVariantMap & aParameters, QStringList aReceiptTemplate = QStringList());

	/// Увеличение числа чеков в бд.
	void incrementReceiptCount(SDK::Driver::IPrinter * aPrinter) const;

	/// Получить номер чека.
	unsigned getReceiptID() const;

	/// Получить кол-во чеков, напечатанных на терминале
	int getReceiptCount() const;

	/// Загружаем содержимое картинки и вставляем его в тег [img]
	QString convertImage2base64(const QString & aString);

	/// Генерим содержмое на основе тега [qr] и заменяем его на тег [img]
	QString generateQR(const QString & aString);

	/// Генерим содержмое на основе тега [pdf417] и заменяем его на тег [img]
	QString generatePDF417(const QString & aString);

	/// Генерим содержмое на основе тега [1d] и заменяем его на тег [img]
	QString generate1D(const QString & aString);

	QString generateLine(const QString & aString);

private slots:
	/// Обработка статусов принтера.
	void onStatusChanged(SDK::Driver::EWarningLevel::Enum aWarningLevel, const QString & aTranslation, int aStatus);

	/// Обработка сигнала закрытия смены
	void onFRSessionClosed(const QVariantMap & aParameters);

	/// Инициализация устройств.
	void updateHardwareConfiguration();

	/// Создаеём объект фискального регистратора
	void createFiscalRegister();

	/// Обработчик завершении печати задания
	void taskFinished();

	/// Обработчик печати с ошибкой
	void printEmptyReceipt(int aJobIndex, bool aError);

	/// Обработчик сигнала о наличии неотправленных чеков в фисклаьном регистраторе.
	void onOFDNotSent(bool aExist);

private:
	typedef QMap<QString, QString> TStaticParameters;
	typedef QMap<QString, QStringList> TCachedReceipts;

	TStaticParameters mStaticParameters;
	TCachedReceipts mCachedReceipts;

	IApplication * mApplication;
	IHardwareDatabaseUtils * mDatabaseUtils;
	SDK::PaymentProcessor::IDeviceService * mDeviceService;

	/// Список всех доступных принтеров.
	QList<SDK::Driver::IPrinter *> mPrinterDevices;

	/// Список доступных в данный момент принтеров.
	QSet<SDK::Driver::IPrinter *> mAvailablePrinters;
	QMutex mAvailablePrintersMutex;
	QWaitCondition mPrintersAvailable;

	QFutureWatcher<bool> mFutureWatcher;

	struct Task
	{
		int index;
		PrintCommand * command;
		QVariantMap parameters;
	};

	QQueue<Task> mQueue;
	std::function<bool(int, PrintCommand *, QVariantMap)> mPrintingFunction;

	/// Режим непрерывной печати чеков.
	DSDK::EPrintingModes::Enum mPrintingMode;
	bool mServiceOperation;
	bool mRandomReceiptsID;
	mutable std::mt19937 mRandomGenerator;
	bool mEnableBlankFiscalData;

	/// Индекс следующего задания на печать
	QAtomicInt mNextReceiptIndex;

	/// Список доступных фискальных регистраторов.
	SDK::PaymentProcessor::IFiscalRegister * mFiscalRegister;
};

//---------------------------------------------------------------------------
