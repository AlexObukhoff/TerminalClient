/* @file Файл, предназначенный для подавления warning'ов от библиотеки Qt. Этот
        файл должен включаться после QtHeadersBegin.h и требующихся файлов Qt. 
 
         Пример:
           #include <Common/QtHeadersBegin.h>
           #include <QtCore/QList>
           #include <Common/QtHeadersEnd.h>
*/

#ifdef _MSC_VER

#pragma comment(user, "WTF VS_2013 !!!")

#ifndef CYBER_SUPPRESS_QT_WARNINGS
#error QtHeadersEnd.h included without QtHeadersBegin.h!!!
#endif

#undef CYBER_SUPPRESS_QT_WARNINGS

#pragma warning(pop)

#endif
