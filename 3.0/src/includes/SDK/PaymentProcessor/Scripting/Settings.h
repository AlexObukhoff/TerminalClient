/* @file Прокси класс для получения информации из конфигов в скриптах */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Settings/Directory.h>

namespace SDK {
namespace PaymentProcessor {

class ICore;
class IGUIService;

namespace Scripting {

template<typename T> QString seq2Str2(T aSequence)
{
	QStringList result;

	foreach(auto i, aSequence)
	{
		result << i.toString();
	}

	qSort(result);

	return result.join(";");
}

//------------------------------------------------------------------------------
class TerminalSettings : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString AP READ getAp CONSTANT)
	Q_PROPERTY(QString version READ getVersion CONSTANT)
	Q_PROPERTY(QString dataPath READ getDataPath CONSTANT)
	Q_PROPERTY(QString contentPath READ getContentPath CONSTANT)
	Q_PROPERTY(QString interfacePath READ getInterfacePath CONSTANT)
	Q_PROPERTY(QString skinPath READ getCurrentSkinPath CONSTANT)
	Q_PROPERTY(QString adProfile READ getAdProfile CONSTANT)
	Q_PROPERTY(QString minNote READ getMinNote CONSTANT)
	Q_PROPERTY(QString currencyId READ getCurrencyId CONSTANT)
	Q_PROPERTY(QString currencyCode READ getCurrencyCode CONSTANT)
	Q_PROPERTY(QString currencyName READ getCurrencyName CONSTANT)
	Q_PROPERTY(QString currencyAllNotes READ getCurrencyAllNotes CONSTANT)
	Q_PROPERTY(QString currencyAllCoins READ getCurrencyAllCoins CONSTANT)
	Q_PROPERTY(QString enabledNotes READ geEnabledNotes CONSTANT)
	Q_PROPERTY(QString enabledCoins READ geEnabledCoins CONSTANT)
	Q_PROPERTY(QString disabledNotes READ getDisabledNotes CONSTANT)
	Q_PROPERTY(QString disabledCoins READ getDisabledCoins CONSTANT)

public:
	TerminalSettings(ICore * aCore);

public slots:
	/// Если комбинация провайдера и заполненных полей подходит для входа в сервисное меню.
	bool isItServiceProvider(qint64 aProvider, const QVariantMap & aParameters);

private:
	QString getAp() const { return mTerminalSettings->getKeys()[0].ap; }
	QString getDataPath() const { return mTerminalSettings->getAppEnvironment().userDataPath; }
	QString getContentPath() const { return mTerminalSettings->getAppEnvironment().contentPath; }
	QString getVersion() const { return mTerminalSettings->getAppEnvironment().version; }
	QString getAdProfile() const { return mTerminalSettings->getAdProfile(); }
	QString getMinNote() const { return mTerminalSettings->getCommonSettings().minPar.toString(); }
	QString getCurrencyId() const { return QString::number(mTerminalSettings->getCurrencySettings().id); }
	QString getCurrencyCode() const { return mTerminalSettings->getCurrencySettings().code; }
	QString getCurrencyName() const { return mTerminalSettings->getCurrencySettings().name; }
	QString getCurrencyAllNotes() const { return seq2Str2(mTerminalSettings->getCurrencySettings().notes); }
	QString getCurrencyAllCoins() const { return seq2Str2(mTerminalSettings->getCurrencySettings().coins); }
	QString geEnabledNotes() const { return seq2Str2(mTerminalSettings->getCommonSettings().enabledParNotesList); }
	QString geEnabledCoins() const { return seq2Str2(mTerminalSettings->getCommonSettings().enabledParCoinsList); }

	QString getDisabledNotes() const
	{
		return seq2Str2(mTerminalSettings->getCurrencySettings().notes.toSet()
			.subtract(mTerminalSettings->getCommonSettings().enabledParNotesList));
	}

	QString getDisabledCoins() const
	{
		return seq2Str2(mTerminalSettings->getCurrencySettings().coins.toSet()
			.subtract(mTerminalSettings->getCommonSettings().enabledParCoinsList));
	}

	QString getInterfacePath() const { return mTerminalSettings->getAppEnvironment().interfacePath; }
	QString getCurrentSkinPath() const;

private:
	SDK::PaymentProcessor::TerminalSettings * mTerminalSettings;
	SDK::PaymentProcessor::IGUIService * mGuiService;
};

//------------------------------------------------------------------------------
class DealerSettings : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString pointName READ getPointName)
	Q_PROPERTY(QString pointAddress READ getPointAddress CONSTANT)
	Q_PROPERTY(QString name READ getName CONSTANT)
	Q_PROPERTY(QString address READ getAddress CONSTANT)
	Q_PROPERTY(QString inn READ getInn CONSTANT)
	Q_PROPERTY(QString kbk READ getKbk CONSTANT)
	Q_PROPERTY(QString phone READ getPhone CONSTANT)
	Q_PROPERTY(QString isBank READ getIsBank CONSTANT)
	Q_PROPERTY(QString operatorName READ getOperatorName CONSTANT)
	Q_PROPERTY(QString operatorAddress READ getOperatorAddress CONSTANT)
	Q_PROPERTY(QString operatorInn READ getOperatorInn CONSTANT)
	Q_PROPERTY(QString operatorContractNumber READ getOperatorContractNumber CONSTANT)
	Q_PROPERTY(QString bankName READ getBankName CONSTANT)
	Q_PROPERTY(QString bankAddress READ getBankAddress CONSTANT)
	Q_PROPERTY(QString bankBik READ getBankBik CONSTANT)
	Q_PROPERTY(QString bankInn READ getBankInn CONSTANT)
	Q_PROPERTY(QString bankPhone READ getBankPhone CONSTANT)
	Q_PROPERTY(QString bankContractNumber READ getBankContractNumber CONSTANT)

public:
	DealerSettings(ICore * mCore);

public slots:
	/// Получение разрешения на платёж.
	bool isPaymentAllowed(const QVariantMap & aParameters) const;

	/// Получение списка комисиий для отображения.
	QObject * getCommissions(qint64 aProvider, const QVariantMap & aParameters, double aAmount);

private:
	QString getPointName() const { return mPersonalSettings.pointName; }
	QString getPointAddress() const { return mPersonalSettings.pointAddress; }
	QString getName() const { return mPersonalSettings.name; }
	QString getAddress() const { return mPersonalSettings.businessAddress.isEmpty() ? mPersonalSettings.address : mPersonalSettings.businessAddress; }
	QString getInn() const { return mPersonalSettings.inn; }
	QString getKbk() const { return mPersonalSettings.kbk; }
	QString getPhone() const { return mPersonalSettings.phone; }
	QString getIsBank() const { return mPersonalSettings.isBank; }
	QString getOperatorName() const { return mPersonalSettings.operatorName; }
	QString getOperatorAddress() const { return mPersonalSettings.operatorAddress; }
	QString getOperatorInn() const { return mPersonalSettings.operatorInn; }
	QString getOperatorContractNumber() const { return mPersonalSettings.operatorContractNumber; }
	QString getBankName() const { return mPersonalSettings.bankName; }
	QString getBankAddress() const { return mPersonalSettings.bankAddress; }
	QString getBankBik() const { return mPersonalSettings.bankBik; }
	QString getBankInn() const { return mPersonalSettings.bankInn; }
	QString getBankPhone() const { return mPersonalSettings.bankPhone; }
	QString getBankContractNumber() const { return mPersonalSettings.bankContractNumber; }

private:
	SDK::PaymentProcessor::DealerSettings * mSettings;
	SDK::PaymentProcessor::SPersonalSettings mPersonalSettings;
};

//------------------------------------------------------------------------------
class SCommission : public QObject
{
	Q_OBJECT

	Q_PROPERTY(double minLimit READ getMinLimit CONSTANT)
	Q_PROPERTY(double maxLimit READ getMaxLimit CONSTANT)
	Q_PROPERTY(double minCharge READ getMinCharge CONSTANT)
	Q_PROPERTY(double maxCharge READ getMaxCharge CONSTANT)
	Q_PROPERTY(double value READ getValue CONSTANT)
	Q_PROPERTY(bool isPercent READ getIsPercent CONSTANT)
	Q_PROPERTY(bool hasLimits READ getHasLimits CONSTANT)
	Q_PROPERTY(bool hasMinLimit READ getHasMinLimit CONSTANT)
	Q_PROPERTY(bool hasMaxLimit READ getHasMaxLimit CONSTANT)

public:
	SCommission(const Commission & aCommission, QObject * aParent)
		: QObject(aParent), mCommission(aCommission)
	{
	}

private:
	double getMinLimit() const { return mCommission.getMinLimit(); }
	double getMaxLimit() const { return mCommission.getMaxLimit(); }
	double getMinCharge() const { return mCommission.getMinCharge(); }
	double getMaxCharge() const { return mCommission.getMaxCharge(); }
	double getValue() const { return mCommission.getValue(); }
	bool getIsPercent() const { return mCommission.getType() == Commission::Percent; }
	bool getHasLimits() const { return mCommission.hasLimits(); }
	bool getHasMinLimit() const { return mCommission.hasMinLimit(); }
	bool getHasMaxLimit() const { return mCommission.hasMaxLimit(); }

private:
	Commission mCommission;
};

//------------------------------------------------------------------------------
class Settings : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QObject * dealer READ getDealerSettings CONSTANT)
	Q_PROPERTY(QObject * terminal READ getTerminalSettings CONSTANT)

public:
	Settings(ICore * aCore)
		: mCore(aCore),
		  mTerminalSettingsProxy(aCore),
		  mDealerSettingsProxy(aCore)
	{
	}

private:
	QObject * getDealerSettings() { return &mDealerSettingsProxy; }
	QObject * getTerminalSettings() { return &mTerminalSettingsProxy; }

private:
	ICore * mCore;
	TerminalSettings mTerminalSettingsProxy;
	DealerSettings mDealerSettingsProxy;
};

//------------------------------------------------------------------------------
}}} // Scripting::PaymentProcessor::SDK
