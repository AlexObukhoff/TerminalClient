/* @file Графический движок. */

#pragma once

#include <memory>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QStack>
#include <QtCore/QSharedPointer>
#include <QtGui/QInputEvent>
#include <QtGui/QColor>
#include <QtQuick/QQuickView>
#include <QtQuickWidgets/QQuickWidget>
#include <QtQuick/QQuickItem>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickPaintedItem>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/GUI/IGraphicsEngine.h>
#include <SDK/GUI/IGraphicsHost.h>
#include <SDK/GUI/IGraphicsBackend.h>

// Модули
#include <Common/ILogable.h>
#include <SDK/GUI/MessageBoxParams.h>

namespace GUI {

//---------------------------------------------------------------------------
namespace
{
	const QString DefaultBackgroundColor = "#003e75";
	const int DefaultAlpha = 0xd8;

	class ModalBackgroundWidget : public QQuickPaintedItem
	{
	public:
		ModalBackgroundWidget()
		{
			setFlag(QQuickItem::ItemHasContents);			
			setFillColor(ModalBackgroundWidget::getColor(DefaultBackgroundColor));

			// Необходимо для обработки мыши
			setAcceptedMouseButtons(Qt::AllButtons);
		}

	public:
		void setColor(const QString & aColor, int aAlpha = DefaultAlpha)
		{
			setFillColor(ModalBackgroundWidget::getColor(aColor.isEmpty() ? DefaultBackgroundColor : aColor, aAlpha));
		}

	protected:
		void paint(QPainter * aPainter) override { Q_UNUSED(aPainter) }
		void mousePressEvent(QMouseEvent * aEvent) override { Q_UNUSED(aEvent) }

	private:
		QColor getColor(const QString & aColor, int aAlpha = DefaultAlpha)
		{
			QColor color(aColor);
			color.setAlpha(aAlpha);
			return color;
		}
	};

	class DebugWidget : public QQuickItem
	{
	public:
		DebugWidget()
		{
			/*setBrush(QBrush(QColor(255, 0, 255)));
			setPen(QPen(QColor(255, 255, 0)));
			setZValue(10);*/
		}

	protected:
		/*virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
		{
			QQuickItem::paint(painter, option, widget);

			painter->setFont(QFont("Arial", 12));
			painter->drawText(rect(), Qt::AlignCenter, getDebugInfo());
		}*/

	public:
		void setPosition(const QPoint & aLeftBottomPoint)
		{
			//setRect(QRect(aLeftBottomPoint.x(), aLeftBottomPoint.y() - 20, 100, 20));
		}

		void updateMousePos(const QPoint & aPos) { mMousePos = aPos; update(); }

	private:
		QString getDebugInfo() const
		{
			return QString("(%1; %2)").arg(mMousePos.x()).arg(mMousePos.y());
		}

	private:
		QPoint mMousePos;
	};

}

//---------------------------------------------------------------------------
/*class KeyboardContext : public QInputContext
{
	Q_OBJECT

public:
	virtual QString identifierName();
	virtual QString language();
	virtual bool isComposing() const;
	virtual void reset();
	virtual bool filterEvent(const QEvent* aEvent);

signals:
	void requestSoftwareInputPanel();
};*/

//---------------------------------------------------------------------------
/// Графический интерфейс (канва + контейнер графических движков).
class GraphicsEngine : public QObject, public SDK::GUI::IGraphicsEngine, private ILogable
{
	Q_OBJECT

public:
	GraphicsEngine();
	~GraphicsEngine();

public:
	/// Инициализация. Вызывается после addContentDirectory() и addEngine(), 
	/// инициализирует экран.
	bool initialize(int aDisplay, int aWidth, int aHeight, bool aShowCursor, bool aShowDebugInfo = false);

	/// Освобождение ресурсов
	bool finalize();

	/// Показывает пустой экран.
	void start();

	/// Прячет экран
	void pause();

	/// Закрывает все графические элементы, прячет экран.
	void stop();

	enum EWidgetType
	{
		All,
		Qml,
		Native,
		Web
	};

	Q_ENUMS(EWidgetType)

	/// Очистить сцену от виджетов.
	void reset(EWidgetType aType = GraphicsEngine::Qml);

	/// Добавляет каталог с графическими элементами (сразу производится поиск всех описателей и их парсинг).
	void addContentDirectory(const QString & aDirectory);

	/// Дабавляет клавиши, которые желаем обрабатывать отдельно
	void addHandledKeys(const QStringList & aHandledKeyList);

	/// Добавляет графический движок.
	void addBackend(SDK::GUI::IGraphicsBackend * aEngine);

	/// Отображает виджет.
	bool show(const QString & aWidget, const QVariantMap & aParameters);

	/// Отображает popup-виджет (на уровнь выше текущего).
	bool showPopup(const QString & aWidget, const QVariantMap & aParameters);

	/// Отображает модальный виджет.
	QVariantMap showModal(const QString & aWidget, const QVariantMap & aParameters);

	/// Скрывает текущий popup или модальный виджет.
	bool hidePopup(const QVariantMap & aParameters = QVariantMap());

	/// Оповещает виджет.
	void notify(const QString & aEvent, const QVariantMap & aParameters);

	/// Оповещение от popup-виджета
	void popupNotify(const QString & aEvent, const QVariantMap & aParameters);

	/// Установить графический контейнер.
	void setGraphicsHost(SDK::GUI::IGraphicsHost * aHost);

	/// IGraphicsEngine: Возвращает указатель на владельца графического контейнера.
	virtual SDK::GUI::IGraphicsHost * getGraphicsHost();

	/// IGraphicsEngine: Возвращает лог.
	virtual ILog * getLog() const;

	/// Возвращает разрешение дисплея
	QRect getDisplayRectangle(int aIndex) const;

	/// Получить снимок view
	QPixmap getScreenshot();

private slots:
	void onRequestSoftwareInputPanel();
	void setFrontWidget(QQuickItem * aNewWidget, QQuickItem * aOldWidget, int aLevel, bool aPopup);

signals:
	/// Сигнал об активности пользователя.
	void userActivity();

	/// Сигнал об нездоровой активности пользователя.
	void intruderActivity();

	/// Срабатывает при закрытии виджета, на котором рисует движок.
	void closed();

	void keyPressed(QString aText);

private: // Методы
			// Перехватываем события активности пользователя и закрытия окна.
	virtual bool eventFilter(QObject * aObject, QEvent * aEvent);
	bool showWidget(const QString & aScene, bool aPopup, const QVariantMap & aParameters);

	// Показать/спрятать виртуальную клавиатуру
	void showVirtualKeyboard();
	void hideVirtualKeyboard();

private: // Типы
			/// Параметры дисплея.
	struct SScreen
	{
		int   number;     /// Номер дисплея.
		bool  isPrimary;  /// Флаг для главного дисплея.
		QRect geometry;   /// Геометрия.
		QWidget * widget; /// Виджет, соответствующий данному дисплею.
	};

	/// Список графических элементов.
	struct SWidget
	{
		SDK::GUI::GraphicsItemInfo info;
		std::weak_ptr<SDK::GUI::IGraphicsItem> graphics;
	};

	typedef QMap<QString, SWidget> TWidgetList;

private: // Данные
			// Интерфейс приложения.
	SDK::GUI::IGraphicsHost * mHost;

	/// Список доступных экранов.
	QMap<int, SScreen> mScreens;

	// Родительское окно приложения
	QWidget * mRootView;

	// Контейнер для отображения QtQuick сцен
	QQuickWindow * mQuickView;
	

	// Список каталогов с контентом.
	QStringList mContentDirectories;

	QStringList mHandledKeyList;

	// Список доступных бэкэндов.
	QMap<QString, SDK::GUI::IGraphicsBackend *> mBackends;

	// Список доступных виджетов.
	TWidgetList mWidgets;

	QQmlEngine mEngine;

	// Текущий виджет.
	TWidgetList::Iterator mTopWidget;
	// Popup виджет
	TWidgetList::Iterator mPopupWidget;

	bool mShowingModal;
	ModalBackgroundWidget mModalBackgroundWidget;
	DebugWidget mDebugWidget;
	QVariantMap mModalResult;
	bool mIsVirtualKeyboardVisible;
};

} // namespace GUI

//---------------------------------------------------------------------------
