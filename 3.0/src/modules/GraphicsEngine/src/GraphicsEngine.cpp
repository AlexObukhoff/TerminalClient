/* @file Графический интерфейс. */


// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDirIterator>
#include <QtCore/QSettings>
#include <QtGui/QKeyEvent>
#include <QtGui/QKeySequence>
#include <QtGui/QApplication>
#include <QtGui/QWidget>
#include <QtGui/QDesktopWidget>
#include <QtGui/QGraphicsProxyWidget>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtCore/QMetaEnum>
#include <qjson.h>
#include <Common/QtHeadersEnd.h>

#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif

// Проект
#include "GraphicsEngine.h"

namespace GUI {

//---------------------------------------------------------------------------
namespace CGraphicsEngine
{
	const char DescriptorFileSuffix[] = "ini";

	const char PopupClosedReason[] = "popup_closed";
	const char SectionItemName[] = "graphics_item";
	const char NameKey[] = "graphics_item/name";
	const char TypeKey[] = "graphics_item/type";
	const char SectionContextName[] = "context";
	const char VirtualKeyboardKey[] = "useVirtualKeyboard";

	const QString InputContextName = "VirtualKeyboard";
	const QString DefaultLanguage = "en_US";

	const int IntruderTreshold = 3;
}

//--------------------------------------------------------------------------
QString KeyboardContext::identifierName()
{
	return CGraphicsEngine::InputContextName;
}

//--------------------------------------------------------------------------
QString KeyboardContext::language()
{
	return CGraphicsEngine::DefaultLanguage;
}

//--------------------------------------------------------------------------
bool KeyboardContext::isComposing() const
{
	return false;
}

//--------------------------------------------------------------------------
void KeyboardContext::reset()
{
}

//--------------------------------------------------------------------------
bool KeyboardContext::filterEvent(const QEvent* aEvent)
{
	if (aEvent->type() == QEvent::RequestSoftwareInputPanel)
	{
		emit requestSoftwareInputPanel();
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------
GraphicsEngine::GraphicsEngine() : 
	ILogable("Interface"),
	mShowingModal(false),
	mHost(nullptr),
	mIsVirtualKeyboardVisible(false)
{
	QDesktopWidget * desktop = QApplication::desktop();
	
	for (int i = 0, index = 1; i < desktop->numScreens(); ++i)
	{
		SScreen screen;
		screen.number = i;
		screen.isPrimary = i == desktop->primaryScreen();
		screen.geometry = desktop->screenGeometry(i);
		screen.widget = desktop->screen(i);

		mScreens[screen.isPrimary ? 0 : index++] = screen;
	}

	// Наилучшие для QML настройки графической оптимизации (описаны в документации для QDeclarativeView)
	mView.setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::DontSavePainterState);
	mView.setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
	mView.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
	
	mView.viewport()->setFocusPolicy(Qt::NoFocus);
	mView.setFocusPolicy(Qt::StrongFocus);
	mView.setMouseTracking(true);

	mView.setFrameShape(QFrame::NoFrame);
	mView.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	mView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
	// для перехвата Alt+F4
	mView.installEventFilter(this);

	// Настраиваем сцену
	mScene.setItemIndexMethod(QGraphicsScene::NoIndex);
	mScene.setStickyFocus(true);
	mScene.installEventFilter(this);

	mView.setScene(&mScene);

	// Настраиваем маску
	mModalBackgroundWidget.setZValue(2);
	mModalBackgroundWidget.setVisible(false);

	// Настраиваем виртуальную клавиатуру
	KeyboardContext * keyboardContext = new KeyboardContext;
	qApp->setInputContext(keyboardContext);
	connect(keyboardContext, SIGNAL(requestSoftwareInputPanel()), this, SLOT(onRequestSoftwareInputPanel()));
}

//---------------------------------------------------------------------------
GraphicsEngine::~GraphicsEngine()
{
}

//---------------------------------------------------------------------------
bool GraphicsEngine::initialize(int aDisplay, int aWidth, int aHeight, bool aShowCursor, bool aShowDebugInfo)
{
	// Если задан отсутствующий в системе дисплей, откатываемся на нулевой
	if (aDisplay < 0 || aDisplay > mScreens.size() - 1)
	{
		aDisplay = 0;
	}

#ifdef _DEBUG
	mView.setGeometry(mScreens[aDisplay].geometry.left(), mScreens[aDisplay].geometry.top(), 1280, 1024);
#else
	mView.setGeometry(mScreens[aDisplay].geometry);

	// Настраиваем масштабирование
	mView.scale(mScreens[aDisplay].geometry.width() / qreal(aWidth), mScreens[aDisplay].geometry.height() / qreal(aHeight));
#endif

	mScene.setSceneRect(0, 0, aWidth, aHeight);

	mModalBackgroundWidget.setRect(0, 0, aWidth, aHeight);
	mScene.addItem(&mModalBackgroundWidget);

	mDebugWidget.setPosition(QPoint(0, aHeight));
	mScene.addItem(&mDebugWidget);
	mDebugWidget.setVisible(aShowDebugInfo);

	// Настраиваем курсор
	if (!aShowCursor)
	{
		qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
	}

	// Хост должен быть задан до вызова initialize.
	//Q_ASSERT(mHost != 0);

	mTopWidget = mWidgets.end();
	mPopupWidget = mWidgets.end();

	foreach (const SWidget & widget, mWidgets)
	{
		if (!mBackends.contains(widget.info.type))
		{
			toLog(LogLevel::Error, QString("Cannot find '%1' backend for '%2' widget.").arg(widget.info.type).arg(widget.info.name));
			return false;
		}
	}

	return !mWidgets.isEmpty();
}

//---------------------------------------------------------------------------
bool GraphicsEngine::finalize()
{
	foreach (const SWidget & widget, mWidgets)
	{
		if (!widget.graphics.expired() && widget.graphics.lock()->isValid() && widget.graphics.lock()->getWidget()->scene() == &mScene)
		{
			mScene.removeItem(widget.graphics.lock()->getWidget());
		}
	}

	mWidgets.clear();
	mContentDirectories.clear();
	mIsVirtualKeyboardVisible = false;

	return true;
}

//---------------------------------------------------------------------------
void GraphicsEngine::start()
{
#ifndef _DEBUG
	mView.setWindowFlags(mView.windowFlags() | Qt::WindowStaysOnTopHint);
#endif

	mView.showFullScreen();
}

//---------------------------------------------------------------------------
void GraphicsEngine::pause()
{
	mView.hide();
}

//---------------------------------------------------------------------------
void GraphicsEngine::stop()
{
	mView.hide();

	foreach (SWidget widget, mWidgets.values())
	{
		if (!widget.graphics.expired() && widget.graphics.lock()->isValid())
		{
			widget.graphics.lock()->getWidget()->setVisible(false);
			widget.graphics.lock()->getWidget()->setZValue(0);
		}
	}

	mTopWidget = mWidgets.end();
	mPopupWidget = mWidgets.end();
	mModalBackgroundWidget.setVisible(false);
}

//---------------------------------------------------------------------------
void GraphicsEngine::reset(EWidgetType aType)
{
	auto getWidgetType = [](GraphicsEngine::EWidgetType aType) -> QString
	{
		const QMetaObject metaObject = GraphicsEngine::staticMetaObject;
		int enumIndex = metaObject.indexOfEnumerator("EWidgetType");
		if (enumIndex == -1) {
			return "";
		}

		QMetaEnum en = metaObject.enumerator(enumIndex);
		return QString(en.valueToKey(aType));
	};

	QString widgetType = getWidgetType(aType).toLower();

	foreach(const SWidget & widget, mWidgets)
	{
		if (widget.info.type != widgetType && aType != GraphicsEngine::All)
		{
			continue;
		}
		
		if (!widget.graphics.expired() && 
			widget.graphics.lock()->isValid() && 
			widget.graphics.lock()->getWidget()->scene() == &mScene)
		{
			mScene.removeItem(widget.graphics.lock()->getWidget());
			mBackends[widget.info.type]->removeItem(widget.info);
			toLog(LogLevel::Debug, QString("REMOVE widget '%1'").arg(widget.info.name));
		}
	}
	
	mTopWidget = mWidgets.end();
	mPopupWidget = mWidgets.end();
	mModalBackgroundWidget.setVisible(false);
}

//---------------------------------------------------------------------------
void GraphicsEngine::addContentDirectory(const QString & aDirectory)
{
	mContentDirectories << aDirectory;
	
	bool fromResources = false;
	if (aDirectory.left(2) == ":/")
	{
		fromResources = true;
	}

	// Ищем в данном каталоге рекурсивно описатели графических элементов и правила сценариев
	QDirIterator entry(aDirectory,
		QStringList() << QString("*.%1").arg(CGraphicsEngine::DescriptorFileSuffix),
		QDir::Files, QDirIterator::Subdirectories);

	while (entry.hasNext())
	{
		entry.next();

		QSettings file(entry.fileInfo().absoluteFilePath(), QSettings::IniFormat);
		file.setIniCodec("UTF-8");

		if (file.childGroups().contains(CGraphicsEngine::SectionItemName))
		{
			SWidget widget;

			widget.info.name = file.value(CGraphicsEngine::NameKey).toString();
			widget.info.type = file.value(CGraphicsEngine::TypeKey).toString();

			// для контента из ресурсов добавляем специальный префикс
			widget.info.directory = (fromResources ? "qrc" : "") + entry.fileInfo().absolutePath();

			// Описатель должен содержать как минимум имя и тип
			if (widget.info.name.isEmpty() || widget.info.type.isEmpty())
			{
				toLog(LogLevel::Warning, QString("Skipping %1: item name or type not specified.").arg(entry.filePath()));
			}
			else
			{
				// Читаем специфичные для движка параметры
				file.beginGroup(widget.info.type);
				foreach (QString key, file.childKeys())
				{
					widget.info.parameters[key] = file.value(key).toString();
				}
				file.endGroup();

				// Читаем специфичные для виджета параметры
				file.beginGroup(CGraphicsEngine::SectionContextName);
				foreach (QString key, file.childKeys())
				{
					widget.info.context[key] = file.value(key).toString();
				}
				file.endGroup();

				// Добавляем элемент в список доступных
				mWidgets.insertMulti(widget.info.name, widget);

				toLog(LogLevel::Normal, QString("Added graphics item '%1'.").arg(entry.filePath()));
			}
		}
	}
}

//---------------------------------------------------------------------------
void GraphicsEngine::addHandledKeys(const QStringList & aHandledKeyList)
{
	mHandledKeyList = aHandledKeyList;
}

//---------------------------------------------------------------------------
void GraphicsEngine::addBackend(SDK::GUI::IGraphicsBackend * aBackend)
{
	mBackends[aBackend->getType()] = aBackend;

	if (aBackend->getType() == "native")
	{
		foreach(SDK::GUI::GraphicsItemInfo info, aBackend->getItemList())
		{
			// Для "нативных" виджетов необходимо обновить параметр directory
			TWidgetList::Iterator widget = mWidgets.find(info.name);
			if (widget != mWidgets.end())
			{
				widget->info.directory = info.directory;
			}
			else
			{
				SWidget widget;
				widget.info = info;
				mWidgets.insertMulti(widget.info.name, widget);
			}
		}
	}
}

//------------------------------------------------------------------------------
ILog * GraphicsEngine::getLog() const
{
	return ILogable::getLog();
}

//---------------------------------------------------------------------------
QRect GraphicsEngine::getDisplayRectangle(int aIndex) const
{
	return mScreens.value(aIndex).geometry;
}

//---------------------------------------------------------------------------
QPixmap GraphicsEngine::getScreenshot()
{
	return QPixmap::grabWidget(&mView);
}

//---------------------------------------------------------------------------
void GraphicsEngine::onRequestSoftwareInputPanel()
{
	if (mTopWidget->info.parameters[CGraphicsEngine::VirtualKeyboardKey] == "true")
	{
		showVirtualKeyboard();
	}
}

//---------------------------------------------------------------------------
bool GraphicsEngine::show(const QString & aWidget, const QVariantMap & aParameters)
{
	return showWidget(aWidget, false, aParameters);
}

//---------------------------------------------------------------------------
bool GraphicsEngine::showPopup(const QString & aWidget, const QVariantMap & aParameters)
{
	return showWidget(aWidget, true, aParameters);
}

//---------------------------------------------------------------------------
QVariantMap GraphicsEngine::showModal(const QString & aWidget, const QVariantMap & aParameters)
{	
	if (showPopup(aWidget, aParameters))
	{
		mShowingModal = true;

		while (mShowingModal)
		{
			QApplication::processEvents(QEventLoop::WaitForMoreEvents);
		}
	}
	
	return mModalResult;
}

//---------------------------------------------------------------------------
bool GraphicsEngine::hidePopup(const QVariantMap & aParameters)
{
	if (mPopupWidget == mWidgets.end())
	{
		return false;
	}

	mModalResult = aParameters;
	mModalBackgroundWidget.setVisible(false);

	mPopupWidget->graphics.lock()->getWidget()->setVisible(false);
	mPopupWidget->graphics.lock()->getWidget()->setScale(1.0);
	mPopupWidget->graphics.lock()->hide();
	mPopupWidget = mWidgets.end();

	if (mTopWidget != mWidgets.end())
	{
		mTopWidget->graphics.lock()->getWidget()->setFocus();
		mTopWidget->graphics.lock()->notify(CGraphicsEngine::PopupClosedReason, aParameters);
	}

	mShowingModal = false;

	return true;
}

//---------------------------------------------------------------------------
void GraphicsEngine::notify(const QString & aEvent, const QVariantMap & aParameters)
{
	auto formatParams = [](const QVariantMap & aParameters) -> QString
	{
		return QString::fromUtf8(QJsonDocument::fromVariant(aParameters).toJson(QJsonDocument::Compact));
	};

	TWidgetList::Iterator w = mWidgets.end();
	
	if (mPopupWidget != mWidgets.end())
	{
		w = mPopupWidget;
		mPopupWidget->graphics.lock()->notify(aEvent, aParameters);		
	}
	else if (mTopWidget != mWidgets.end())
	{
		w = mTopWidget;
		mTopWidget->graphics.lock()->notify(aEvent, aParameters);
	}
	
	if (w != mWidgets.end())
	{
		toLog(LogLevel::Debug, QString("NOTIFY '%1'. Parameters: %2").arg(w->info.name).arg(formatParams(aParameters)));
	}
	else
	{
		toLog(LogLevel::Debug, QString("NOTIFY WIDGET NOT FOUND. Parameters: %1").arg(formatParams(aParameters)));
	}
}

//---------------------------------------------------------------------------
void GraphicsEngine::popupNotify(const QString & aEvent, const QVariantMap & aParameters)
{
	if (mTopWidget != mWidgets.end())
	{
		// Оповещаем виджет, который находится под всплывающим окном
		mTopWidget->graphics.lock()->notify(aEvent, aParameters);

		toLog(LogLevel::Normal, QString("NOTIFY_BY_POPUP '%1'. Parameters: %2")
			.arg(mTopWidget->info.name)
			.arg(QString::fromUtf8(QJsonDocument::fromVariant(aParameters).toJson(QJsonDocument::Compact))));
	}
}

//---------------------------------------------------------------------------
bool GraphicsEngine::showWidget(const QString & aWidget, bool aPopup, const QVariantMap & aParameters)
{
	// Нативная клавиатура может остаться от предыдущего виджета. Закроем ее
	if (mIsVirtualKeyboardVisible)
	{
		hideVirtualKeyboard();
	}
	
	// Если находимся в модальном режиме, то обычный show не отрабатываем
	if (mShowingModal)
	{
		toLog(LogLevel::Error, QString("Cannot show '%1': already showing a modal widget.").arg(aWidget));
		return false;
	}

	TWidgetList::Iterator newWidget = mWidgets.find(aWidget);
	TWidgetList::Iterator oldWidget = mTopWidget;

	// Проверяем существет ли такой виджет
	if (newWidget == mWidgets.end())
	{
		toLog(LogLevel::Error, QString("Cannot show '%1': not found.").arg(aWidget));
		return false;
	}

	// Если виджетов с одинаковым именем больше одного
	// Ищем виджет с таким же контекстом. Если не нашли, то оставляем первый c пустым контекстом
	if (mWidgets.values(aWidget).count() > 1)
	{
		TWidgetList::Iterator widgetWithContext;
		TWidgetList::Iterator firstWidget = newWidget;
		
		for (widgetWithContext = firstWidget; widgetWithContext != mWidgets.end(); ++widgetWithContext)
		{
			if (widgetWithContext->info.name == aWidget && widgetWithContext->info.context.isEmpty())
			{
				newWidget = widgetWithContext;
				break;
			}
		}

		foreach (QString paramKey, aParameters.keys())
		{
			for (widgetWithContext = firstWidget; widgetWithContext != mWidgets.end(); ++widgetWithContext)
			{
				if (widgetWithContext->info.name != aWidget)
				{
					continue;
				}

				foreach (QString contextKey, widgetWithContext->info.context.keys())
				{
					if (contextKey == paramKey)
					{
						if (widgetWithContext->info.context[paramKey].toString().split("|").contains(aParameters[paramKey].toString()))
						{
							newWidget = widgetWithContext;
						}
					}
				}
			}
		}
	}

	// Если ещё не создан - создаём
	if (newWidget->graphics.expired())
	{
		newWidget->graphics = mBackends[newWidget->info.type]->getItem(newWidget->info);
	}

	if (!newWidget->graphics.expired() && newWidget->graphics.lock()->isValid())
	{
		if (!newWidget->graphics.lock()->getWidget()->scene())
		{
			mScene.addItem(newWidget->graphics.lock()->getWidget());
		}
	}
	else 
	{
		toLog(LogLevel::Error, QString("Failed to instanciate widget '%1'.").arg(aWidget));
		return false;
	}

	// По умолчанию показываем на слое для обычных виджетов
	int level = 1;

	// Если отображается всплывающее окно, то закрываем его
	if (mPopupWidget != mWidgets.end() && !aPopup)
	{
		hidePopup(QVariantMap());
	}

	// Если нужно, инициализируем новый виджет
	if (aParameters.contains("reset") ? aParameters["reset"].toBool() : true)
	{
		newWidget->graphics.lock()->reset(aParameters);
	}

	// Если виджет уже отображается, то только вызываем обработчк showHandler
	if (newWidget == mTopWidget)
	{
		mTopWidget->graphics.lock()->show();
		return true;
	}

	// Если всплывающий или модальный виджет, то маска и всплывающее окно выводятся уровнем выше текущего виджета
	if (aPopup)
	{
		// Масштабируем, если надо, всплывающее окно
		if (aParameters["scaled"].toBool())
		{
			QRectF rect = newWidget->graphics.lock()->getWidget()->sceneBoundingRect();
			qreal scale = qMin(mScene.width() / rect.width(), mScene.height() / rect.height());
			newWidget->graphics.lock()->getWidget()->setScale(scale);
		}
		
		//Установим цвет фона модального окна
		QString popupOverlayColor = aParameters["popup_overlay_color"].toString();
		if (!popupOverlayColor.isEmpty())
		{
			mModalBackgroundWidget.setColor(popupOverlayColor);
		}
		
		// Показываем маску
		mModalBackgroundWidget.setVisible(true);

		// Показываем всплывающее окно
		level = 3;
		mPopupWidget = newWidget;
	}
	else
	{
		if (mTopWidget != mWidgets.end())
		{
			oldWidget = mTopWidget;
			oldWidget->graphics.lock()->hide();
		}

		mTopWidget = newWidget;
	}

	if (!aPopup && newWidget->graphics.expired())
	{
		return false;
	}

	newWidget->graphics.lock()->show();

	// Отрисовка через очередь, чтобы showHandler/resetHandler успели отработать
	QMetaObject::invokeMethod(this, "setFrontWidget", Qt::QueuedConnection, 
		Q_ARG(QGraphicsObject *, dynamic_cast<QGraphicsObject *>(newWidget->graphics.lock()->getWidget())),
		Q_ARG(QGraphicsObject *, (aPopup || oldWidget == mWidgets.end()) ? 0 : dynamic_cast<QGraphicsObject *>(oldWidget->graphics.lock()->getWidget())),
		Q_ARG(int, level), Q_ARG(bool, aPopup));

	QString popupMessage = 
		aPopup ? QString::fromUtf8(QJsonDocument::fromVariant(aParameters).toJson(QJsonDocument::Compact)) : QString();
	
	aPopup ?
		toLog(LogLevel::Normal, QString("SHOW POPUP '%1'. Parameters: %2").arg(aWidget).arg(popupMessage)) :
		toLog(LogLevel::Normal, QString("SHOW '%1' scene. %2")
		.arg(aWidget).arg(newWidget->info.context.isEmpty() ? "" : QString("CONTEXT: %1").arg(QStringList(newWidget->info.context.keys()).join(";"))));

	return true;
}

//---------------------------------------------------------------------------
void GraphicsEngine::setFrontWidget(QGraphicsObject * aNewWidget, QGraphicsObject * aOldWidget, int aLevel, bool aPopup)
{
	// Всплывающее окно уже могли спрятать
	if (aPopup && mPopupWidget == mWidgets.end())
	{
		return;
	}
	
	aNewWidget->setVisible(true);
	aNewWidget->setFocus();
	aNewWidget->setZValue(aLevel);

	if (aOldWidget)
	{
		aOldWidget->setVisible(false);
		aOldWidget->setZValue(0);
	}

	if (mPopupWidget != mWidgets.end())
	{
		QRectF rect(aNewWidget->sceneBoundingRect());
		qreal scale = qobject_cast<QGraphicsItem *>(aNewWidget)->scale();
		aNewWidget->setPos((mScene.sceneRect().width() - rect.width() / scale) / 2, (mScene.sceneRect().height() - rect.height() / scale) / 2);
	}
}

//---------------------------------------------------------------------------
void GraphicsEngine::showVirtualKeyboard()
{
	QGraphicsItem * focusItem = mTopWidget->graphics.lock()->getWidget();
	QWidget * focusWidget = static_cast<QGraphicsProxyWidget *>(focusItem)->widget()->focusWidget();

	if (focusWidget == nullptr)
	{
		return;
	}
	
	// Проверяем существет ли такой виджет
	TWidgetList::Iterator widget = mWidgets.find(CGraphicsEngine::InputContextName);

	if (widget == mWidgets.end())
	{
		toLog(LogLevel::Error, QString("Cannot show '%1': not found.").arg(CGraphicsEngine::InputContextName));
		return;
	}

	// Если ещё не создан - создаём
	if (widget->graphics.expired())
	{
		widget->graphics = mBackends[widget->info.type]->getItem(widget->info);
	}

	if (!widget->graphics.expired() && widget->graphics.lock()->isValid())
	{
		if (!widget->graphics.lock()->getWidget()->scene())
		{
			mScene.addItem(widget->graphics.lock()->getWidget());
		}
	}

	// Формируем список всех возможных положений виртуальной клавиатуры
	QGraphicsItem * keyboardItem = widget->graphics.lock()->getWidget();
	qreal focusItemScale = focusItem->scale();

	QRectF sceneRect(mScene.sceneRect());
	QRectF keyboardRect(keyboardItem->sceneBoundingRect());
	QPointF newPos((sceneRect.width() - keyboardRect.width()) / 2, 0.0f);

	QList<QRectF> keyboardPositions;

	keyboardRect.moveBottomLeft(focusWidget->mapToGlobal(focusWidget->rect().topLeft()) * focusItemScale);
	keyboardPositions << keyboardRect;

	keyboardRect.moveBottomRight(focusWidget->mapToGlobal(focusWidget->rect().topRight()) * focusItemScale);
	keyboardPositions << keyboardRect;

	keyboardRect.moveTopLeft(focusWidget->mapToGlobal(focusWidget->rect().bottomLeft()) * focusItemScale);
	keyboardPositions << keyboardRect;

	keyboardRect.moveTopRight(focusWidget->mapToGlobal(focusWidget->rect().bottomRight()) * focusItemScale);
	keyboardPositions << keyboardRect;

	keyboardRect.moveTopLeft(QPointF(newPos.x(), focusWidget->mapToGlobal(focusWidget->rect().bottomLeft()).y() * focusItemScale));
	keyboardPositions << keyboardRect;
	
	keyboardRect.moveBottomLeft(QPointF(newPos.x(), focusWidget->mapToGlobal(focusWidget->rect().topLeft()).y() * focusItemScale));
	keyboardPositions << keyboardRect;

	foreach (QRectF r, keyboardPositions)
	{
		if (sceneRect.contains(r))
		{
			newPos.setX(r.x());
			newPos.setY(r.y());
			break;
		}
	}

	keyboardItem->setPos(newPos.x(), newPos.y());

	widget->graphics.lock()->getWidget()->setZValue(3);
	widget->graphics.lock()->getWidget()->setVisible(true);

	mIsVirtualKeyboardVisible = true;
}

//---------------------------------------------------------------------------
void GraphicsEngine::hideVirtualKeyboard()
{
	TWidgetList::Iterator widget = mWidgets.find(CGraphicsEngine::InputContextName);
	QGraphicsItem * item = widget->graphics.lock()->getWidget();
	item->setVisible(false);

	mIsVirtualKeyboardVisible = false;
}

//---------------------------------------------------------------------------
bool GraphicsEngine::eventFilter(QObject * aObject, QEvent * aEvent)
{
	QEvent::Type type = aEvent->type();
	static int intruder;

	switch (type)
	{
	case QEvent::GraphicsSceneMouseMove:
	{
		QGraphicsSceneMouseEvent * mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(aEvent);
		mDebugWidget.updateMousePos(mouseEvent->scenePos().toPoint());

			if (++intruder >= CGraphicsEngine::IntruderTreshold)
			{
				emit intruderActivity();
			}
	}
		break;
	
	case QEvent::GraphicsSceneWheel:
		emit intruderActivity();
		break;

	case QEvent::GraphicsSceneMousePress:
	case QEvent::GraphicsSceneMouseDoubleClick:
		if (mIsVirtualKeyboardVisible)
		{
			QGraphicsSceneMouseEvent * mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(aEvent);
			TWidgetList::Iterator widget = mWidgets.find(CGraphicsEngine::InputContextName);
			QGraphicsItem * keyboardItem = widget->graphics.lock()->getWidget();
			QRectF keyboardRect(keyboardItem->sceneBoundingRect());

			if (!keyboardRect.contains(mouseEvent->scenePos()))
			{
				hideVirtualKeyboard();
			}
		}
	case QEvent::GraphicsSceneMouseRelease:
		emit userActivity();
		intruder = 0;
		break;

	case QEvent::KeyPress:
		{
			if (!mHandledKeyList.isEmpty())
			{
				QKeyEvent * keyEvent = static_cast<QKeyEvent *>(aEvent);

				QString key = QKeySequence(keyEvent->key()).toString();
				if (mHandledKeyList.indexOf(key) != -1)
				{
					emit keyPressed(key);
					return true;
				}
			}
		}
		break;

	case QEvent::Close:
		emit closed();
		break;
	}

	return QObject::eventFilter(aObject, aEvent);
}

//---------------------------------------------------------------------------
SDK::GUI::IGraphicsHost * GraphicsEngine::getGraphicsHost()
{
	return mHost;
}

//---------------------------------------------------------------------------
void GraphicsEngine::setGraphicsHost(SDK::GUI::IGraphicsHost * aHost)
{
	mHost = aHost;
}

} // namespace GUI
