//
// All external include files for Terminal Client 
//

#if defined __cplusplus
/* Add C++ includes here */

#include <QFile>
#include <QMap>
#include <QReadWriteLock>
#include <QTimer>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QAbstractListModel>
#include <QtCore/QAbstractTransition>
#include <QtCore/QAtomicInt>
#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QCache>
#include <QtCore/QCoreApplication>
#include <QtCore/QCoreApplication.h>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDate>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QDir.h>
#include <QtCore/QDirIterator>
#include <QtCore/QElapsedTimer>
#include <QtCore/QEvent>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QFinalState>
#include <QtCore/QFlags>
#include <QtCore/QFuture>
#include <QtCore/QFutureSynchronizer>
#include <QtCore/QFutureWatcher>
#include <QtCore/QGenericArgument>
#include <QtCore/QHash>
#include <QtCore/QIODevice>
#include <QtCore/QLibrary>
#include <QtCore/QList>
#include <QtCore/QLocale>
#include <QtCore/QMap>
#include <QtCore/QMetaEnum>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaType>
#include <QtCore/QModelIndex>
#include <QtCore/QMultiMap>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QObject>
#include <QtCore/QObjectList>
#include <QtCore/QPair>
#include <QtCore/QPluginLoader>
#include <QtCore/QPointer>
#include <QtCore/QProcess>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QQueue>
#include <QtCore/QReadLocker>
#include <QtCore/QReadWriteLock>
#include <QtCore/QRect>
#include <QtCore/QRegExp>
#include <QtCore/QResource>
#include <QtCore/QScopedPointer>
#include <QtCore/QSet>
#include <QtCore/QSettings>
#include <QtCore/QSettings.h>
#include <QtCore/QSharedMemory>
#include <QtCore/QSharedPointer>
#include <QtCore/QSharedPointer.h>
#include <QtCore/QSignalMapper>
#include <QtCore/QSize>
#include <QtCore/QStack>
#include <QtCore/QStateMachine>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtCore/QThreadPool>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtCore/QTimerEvent>
#include <QtCore/QTranslator>
#include <QtCore/QUrl>
#include <QtCore/QUuid>
#include <QtCore/QVariant>
#include <QtCore/QVariantList>
#include <QtCore/QVariantMap>
#include <QtCore/QVector>
#include <QtCore/QWaitCondition>
#include <QtCore/QWeakPointer>
#include <QtCore/QWriteLocker>
#include <QtCore/QtAlgorithms>
#include <QFuture>
#include <QtCore/QtEndian>
#include <QtCore/QtGlobal>
#include <QtCore/QtPlugin>
#include <QtCore/qglobal.h>
#include <QtCore/qmath.h>
#include <QtCore/qstring.h>
/*
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeExtensionPlugin>
#include <QtDeclarative/QDeclarativeImageProvider>
#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QtDeclarative>
*/
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtGui/QBitmap>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtGui/QClipboard>
#include <QtGui/QCloseEvent>
#include <QtGui/QColor>
#include <QtWidgets/QDesktopWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFileDialog>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtGui/QIcon>
#include <QtGui/QImage>
#include <QtWidgets/QInputDialog>
#include <QtGui/QInputEvent>
#include <QtWidgets/QItemDelegate>
/*
#include <QtGui/QKeyEvent>
#include <QtGui/QKeySequence>
#include <QtGui/QLayout>
#include <QtGui/QListWidgetItem>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QMovie>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPrintDialog>
#include <QtGui/QPrinter>
#include <QtGui/QProxyStyle>
#include <QtGui/QPushButton>
#include <QtGui/QSessionManager>
#include <QtGui/QShowEvent>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QSpacerItem>
#include <QtGui/QStackedLayout>
#include <QtGui/QStatusBar>
#include <QtGui/QStringListModel>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QSystemTrayIcon>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
*/
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslError>
#include <QtNetwork/QSslKey>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlQueryModel>
#include <QtSql/QtSql>
#include <QtTest/QtTest>
// #include <QtWebKit/QGraphicsWebView>
// #include <QtWebkit/QGraphicsWebView>
// #include <QtWebkit/QWebElement>
// #include <QtWebkit/QWebElementCollection>
// #include <QtWebkit/QWebFrame>
// #include <QtWebkit/QWebPage>
#include <QtXML/QDomDocument>
// #include <QtXML/QXmlStreamWriter>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
// #include <QtXml/QXMLStreamWriter>
// #include <QtXml/QXmlStreamReader>
// #include <QtXml/QXmlStreamWriter>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <functional>
#if _MSC_VER <= 1600
	#include <hash_map>
#else
	#include <filesystem>
	#include <unordered_map>
#endif
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#endif
