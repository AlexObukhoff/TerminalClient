/* @file Реализация компоненты для редактирования профилей устройств. */

// Project
#include "DeviceSlot.h"
#include "EditorPaneListItem.h"
#include "IDeviceBackend.h"
#include "EditorPane.h"
#include "SIPStyle.h"

namespace CEditorPane
{
	const QString SystemPrinterName = "Default Default";
}

//------------------------------------------------------------------------
EditorPane::EditorPane()
	: mSlot(0),
	  mFirstShow(true)
{
}

//------------------------------------------------------------------------
EditorPane::~EditorPane()
{
}

//------------------------------------------------------------------------
void EditorPane::setSlot(IDeviceBackend * aBackend, DeviceSlot * aSlot)
{
	mSlot = aSlot;
	mValues = aSlot->getParameterValues();
	mFirstShow = true;

	// Обновление списка поддерживаемых моделей.
	mModels.clear();

	QStringList models = aBackend->getModels(aSlot->getType());

	foreach (const QString & model, models)
	{
		mModels[model] = aBackend->getModelParameters(aSlot->getType(), model);
	}

	if (!mWidget.data())
	{
		mWidget = createWidget();
	}

	// Первоначально доступна только модель устройства.
	SDK::Plugin::SPluginParameter parameter;

	parameter.title = QCoreApplication::translate("Hardware::CommonParameters", QT_TRANSLATE_NOOP("Hardware::CommonParameters", "#model"));
	parameter.description = QCoreApplication::translate("Hardware::CommonParameters", QT_TRANSLATE_NOOP("Hardware::CommonParameters", "#model_howto"));
	parameter.name = "model_name";
	parameter.type = SDK::Plugin::SPluginParameter::Set;
	parameter.required = true;
	parameter.readOnly = false;

	foreach (const QString & model, mModels.keys())
	{
		if (model == CEditorPane::SystemPrinterName)
		{
			parameter.possibleValues.insert(QCoreApplication::translate("Hardware::PrinterParameters", QT_TRANSLATE_NOOP("Hardware::PrinterParameters", "#sytem_printer_name")), model);
		}
		else
		{
			parameter.possibleValues.insert(model, QVariant());
		}
	}

	mParameters.clear();
	mParameters.push_back(parameter);

	updateView();
	selectEmptyParameter();
}

//------------------------------------------------------------------------
void EditorPane::updateView()
{
	// Первоначальная инициализация, выбор модели.
	QVariantMap::iterator model = mValues.find("model_name");
	if (model != mValues.end())
	{
		QString modelValue(model.value().toString());
		foreach (QString modelsValue, mModels.keys())
		{
			if (modelsValue.toLower() == modelValue.toLower())
			{
				// Модель задана, можно заполнять параметры.
				SDK::Plugin::TParameterList driverParameters = mModels[modelsValue];

				mParameters.resize(1);
				mParameters << driverParameters;
				break;
			}
		}
	}

	mUi.lwParameters->blockSignals(true);
	mUi.lwParameters->clear();
	mUi.lwParameters->blockSignals(false);

	foreach (const SDK::Plugin::SPluginParameter & parameter, mParameters)
	{
		// Добавляем только редактируемые параметры
		if (parameter.readOnly)
		{
			continue;
		}

		EditorPaneListItem * item = new EditorPaneListItem();
		item->setData(EditorPaneListItem::ParameterName, parameter.title);

		// Заполняем параметрами по умолчанию (если есть)
		if (!parameter.defaultValue.isNull() && mValues[parameter.name].isNull())
		{
			item->setData(EditorPaneListItem::ParameterValue, parameter.defaultValue.toString());
			mValues[parameter.name] = parameter.defaultValue;
		}
		else
		{
			item->setData(EditorPaneListItem::ParameterValue, mValues[parameter.name]);
		}

		mUi.lwParameters->addItem(item);
	}
}

//------------------------------------------------------------------------
void EditorPane::selectEmptyParameter()
{
	mUi.btnOk->setEnabled(false);

	foreach (const SDK::Plugin::SPluginParameter & parameter, mParameters)
	{
		// TODO parameter.required завязать логику на требуемый ресурс
		if (mValues.value(parameter.name).isNull() && !parameter.readOnly && parameter.defaultValue.isNull())
		{
			QList<QListWidgetItem *> items = mUi.lwParameters->findItems(parameter.title, Qt::MatchCaseSensitive | Qt::MatchStartsWith);

			if (!items.isEmpty())
			{
				mUi.lwParameters->setCurrentItem(items.first());
				return;
			}
		}
	}

	if (mFirstShow)
	{
		// Всё заполнено, выделяем последний параметр.
		if (mUi.lwParameters->count())
		{
			mUi.lwParameters->setCurrentItem(mUi.lwParameters->item(mUi.lwParameters->count() - 1));

			mFirstShow = false;
		}
	}

	mUi.btnOk->setEnabled(true);
}

//------------------------------------------------------------------------
void EditorPane::setCurrentParameterValue(const QString & aValue)
{
	QListWidgetItem * item = mUi.lwParameters->currentItem();

	if (item)
	{
		for (SDK::Plugin::TParameterList::iterator it = mParameters.begin(); it != mParameters.end(); ++it)
		{
			if (it->title == item->data(EditorPaneListItem::ParameterName))
			{
				QVariant oldValue = mValues[it->name];

				if (aValue.isNull() && !it->defaultValue.isNull())
				{
					mValues[it->name] = it->defaultValue;
				}
				else
				{
					if (!it->possibleValues[aValue].isNull())
					{
						mValues[it->name] = it->possibleValues[aValue];
					}
					else
					{
						mValues[it->name] = aValue;
					}
				}

				item->setData(EditorPaneListItem::ParameterValue, mValues[it->name]);

				if ((it->name == "model_name") && (oldValue != mValues[it->name]))
				{
					QVariant oldModel = mValues["model_name"];
					
					mValues.clear();
					mValues["model_name"] = oldModel;

					updateView();
				}

				mUi.lwValues->blockSignals(true);
				selectEmptyParameter();
				mUi.lwValues->blockSignals(false);

				mUi.lwParameters->repaint();

				break;
			}
		}
	}
}

//------------------------------------------------------------------------
void EditorPane::showCurrentParameterValues()
{
	QListWidgetItem * item = mUi.lwParameters->currentItem();
	if (item)
	{
		foreach (const SDK::Plugin::SPluginParameter & parameter, mParameters)
		{
			if (parameter.title == item->data(EditorPaneListItem::ParameterName))
			{
				switch (parameter.type)
				{
					case SDK::Plugin::SPluginParameter::Set:
					case SDK::Plugin::SPluginParameter::MultiSet:
					{
						mUi.stackedWidget->setCurrentIndex(Enum);
						mUi.lwValues->clear();
						
						// Отсортируем возможные значения параметра драйвера
						auto intOrderLessThan = [&](const QString &s1, const QString &s2) -> bool
						{
							bool ok;
							s1.toInt(&ok);
							return  ok ? s1.toInt() < s2.toInt() : s1 < s2;
						};

						QStringList possibleValues(parameter.possibleValues.keys());
						qSort(possibleValues.begin(), possibleValues.end(), intOrderLessThan);
						mUi.lwValues->addItems(possibleValues);

						QString title;

						if (!mValues[parameter.name].isNull())
						{
							if (parameter.possibleValues.contains(mValues[parameter.name].toString()))
							{
								title = mValues[parameter.name].toString();
							}
							else
							{
								for (QVariantMap::const_iterator it = parameter.possibleValues.begin(); it != parameter.possibleValues.end(); ++it)
								{
									if (!it.value().isNull() && mValues[parameter.name] == it.value())
									{
										title = it.key();

										break;
									}
								}
							}
						}

						QList<QListWidgetItem *> items = mUi.lwValues->findItems(title, Qt::MatchCaseSensitive);
						if (!items.isEmpty())
						{
							mUi.lwValues->blockSignals(true);
							mUi.lwValues->setCurrentItem(items.first());
							mUi.lwValues->blockSignals(false);
						}

						break;
					}

					case SDK::Plugin::SPluginParameter::Bool:
					{
						mUi.stackedWidget->setCurrentIndex(Bool);

						if (mValues[parameter.name].isNull())
						{
							mUi.rbOn->setAutoExclusive(false);
							mUi.rbOff->setAutoExclusive(false);
							mUi.rbOn->setChecked(false);
							mUi.rbOff->setChecked(false);
						}
						else
						{
							mUi.rbOn->setAutoExclusive(true);
							mUi.rbOff->setAutoExclusive(true);
							mUi.rbOn->setChecked(mValues[parameter.name].toBool());
							mUi.rbOff->setChecked(!mValues[parameter.name].toBool());
						}

						break;
					}

					case SDK::Plugin::SPluginParameter::Number:
					{
						mUi.stackedWidget->setCurrentIndex(Number);

						QString value = QString("%1").arg(mValues[parameter.name].toInt() % 1000, 3, 10, QChar('0'));

						mUi.sbDigit1->setValue(QString(value[0]).toInt());
						mUi.sbDigit2->setValue(QString(value[1]).toInt());
						mUi.sbDigit3->setValue(QString(value[2]).toInt());
					}

					case SDK::Plugin::SPluginParameter::Text:
					{
						mUi.stackedWidget->setCurrentIndex(Text);

						mUi.leValue->setInputMask(parameter.possibleValues.value("mask").toString());
						mUi.leValue->setText(mValues[parameter.name].toString());

						break;
					}

					default: break;
				}

				mUi.lbDescription->setVisible(!parameter.description.isEmpty());
				mUi.lbDescription->setText(parameter.description);
				mUi.lbParameterDescription->setText(parameter.title);
			}
		}
	}
}

//------------------------------------------------------------------------
void EditorPane::onParameterRowChanged(QListWidgetItem * /*aCurrent*/, QListWidgetItem * /*aPrevious*/)
{
	showCurrentParameterValues();
}

//------------------------------------------------------------------------
void EditorPane::onEnumValueChanged(QListWidgetItem * aItem)
{
	setCurrentParameterValue(aItem->text());
}

//------------------------------------------------------------------------
void EditorPane::onBoolValueChanged()
{
	setCurrentParameterValue(sender() == mUi.rbOn ? "true" : "false");
}

//------------------------------------------------------------------------
void EditorPane::onNumberValueChanged()
{
	QString value = QString::number(mUi.sbDigit1->value()) + QString::number(mUi.sbDigit2->value()) + QString::number(mUi.sbDigit3->value());
	
	setCurrentParameterValue(QString::number(value.toInt()));
}

//------------------------------------------------------------------------
void EditorPane::onTextValueChanged()
{
	setCurrentParameterValue(mUi.leValue->text());
}

//------------------------------------------------------------------------
DeviceSlot * EditorPane::getSlot() const
{
	return mSlot;
}

//------------------------------------------------------------------------
QWidget * EditorPane::getWidget()
{
	if (!mWidget.data())
	{
		mWidget = createWidget();
	}

	return mWidget.data();
}

//------------------------------------------------------------------------
QWidget * EditorPane::createWidget()
{
	QScopedPointer<QWidget> widget(new QWidget());

	mUi.setupUi(widget.data());

	connect(mUi.lwParameters, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
		SLOT(onParameterRowChanged(QListWidgetItem *, QListWidgetItem *)));

	connect(mUi.lwValues, SIGNAL(itemClicked(QListWidgetItem *)), SLOT(onEnumValueChanged(QListWidgetItem *)));

	connect(mUi.btnOk, SIGNAL(clicked()), SLOT(onOk()));
	connect(mUi.btnCancel, SIGNAL(clicked()), SLOT(onCancel()));

	connect(mUi.rbOn, SIGNAL(clicked()), SLOT(onBoolValueChanged()));
	connect(mUi.rbOff, SIGNAL(clicked()), SLOT(onBoolValueChanged()));

	connect(mUi.btnPlus1, SIGNAL(clicked()), SLOT(onNumberValueChanged()));
	connect(mUi.btnPlus2, SIGNAL(clicked()), SLOT(onNumberValueChanged()));
	connect(mUi.btnPlus3, SIGNAL(clicked()), SLOT(onNumberValueChanged()));

	connect(mUi.btnMinus1, SIGNAL(clicked()), SLOT(onNumberValueChanged()));
	connect(mUi.btnMinus2, SIGNAL(clicked()), SLOT(onNumberValueChanged()));
	connect(mUi.btnMinus3, SIGNAL(clicked()), SLOT(onNumberValueChanged()));

	connect(mUi.leValue, SIGNAL(returnPressed()), this, SLOT(onTextValueChanged()));
	mUi.leValue->setStyle(new SIPStyle);

	mUi.lwParameters->setItemDelegate(new EditorPaneListItemDelegate());

	return widget.take();
}

//------------------------------------------------------------------------
bool EditorPane::isChanged() const
{
	return (mValues != mSlot->getParameterValues());
}

//------------------------------------------------------------------------
const QVariantMap & EditorPane::getParameterValues() const
{
	return mValues;
}

//------------------------------------------------------------------------
void EditorPane::onDefault()
{
	setCurrentParameterValue(QString());
}

//------------------------------------------------------------------------
void EditorPane::onOk()
{
	emit finished();
}

//------------------------------------------------------------------------
void EditorPane::onCancel()
{
	mValues = mSlot->getParameterValues();

	emit finished();
}

//------------------------------------------------------------------------
