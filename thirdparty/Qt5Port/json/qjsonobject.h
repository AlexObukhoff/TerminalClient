/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QJSONOBJECT_H
#define QJSONOBJECT_H

#include "qjsonvalue.h"

#include <QtCore/qmetatype.h>
#include <QtCore/qiterator.h>
#ifdef Q_COMPILER_INITIALIZER_LISTS
#include <QtCore/qpair.h>
#include <initializer_list>
#endif

QT_BEGIN_NAMESPACE

class QDebug;
template <class Key, class T> class QMap;
typedef QMap<QString, QVariant> QVariantMap;

class QJSON_EXPORT QJsonObject
{
public:
    QJsonObject();

#if defined(Q_COMPILER_INITIALIZER_LISTS) || defined(Q_QDOC)
    QJsonObject(std::initializer_list<QPair<QString, QJsonValue> > args)
    {
        initialize();
        for (std::initializer_list<QPair<QString, QJsonValue> >::const_iterator i = args.begin(); i != args.end(); ++i)
            insert(i->first, i->second);
    }
#endif

    ~QJsonObject();

    QJsonObject(const QJsonObject &other);
    QJsonObject &operator =(const QJsonObject &other);

    static QJsonObject fromVariantMap(const QVariantMap &map);
    QVariantMap toVariantMap() const;

    QStringList keys() const;
    int size() const;
    inline int count() const { return size(); }
    inline int length() const { return size(); }
    bool isEmpty() const;

    QJsonValue value(const QString &key) const;
    QJsonValue operator[] (const QString &key) const;
    QJsonValueRef operator[] (const QString &key);

    void remove(const QString &key);
    QJsonValue take(const QString &key);
    bool contains(const QString &key) const;

    bool operator==(const QJsonObject &other) const;
    bool operator!=(const QJsonObject &other) const;

    class const_iterator;

    class iterator
    {
        friend class const_iterator;
        friend class QJsonObject;
        QJsonObject *o;
        int i;

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef int difference_type;
        typedef QJsonValue value_type;
        typedef QJsonValueRef reference;

        inline iterator() : o(0), i(0) {}
        inline iterator(QJsonObject *obj, int index) : o(obj), i(index) {}

        inline QString key() const { return o->keyAt(i); }
        inline QJsonValueRef value() const { return QJsonValueRef(o, i); }
        inline QJsonValueRef operator*() const { return QJsonValueRef(o, i); }
#ifdef Q_QDOC
        inline QJsonValueRef* operator->() const;
#else
        inline QJsonValueRefPtr operator->() const { return QJsonValueRefPtr(o, i); }
#endif
        inline bool operator==(const iterator &other) const { return i == other.i; }
        inline bool operator!=(const iterator &other) const { return i != other.i; }

        inline iterator &operator++() { ++i; return *this; }
        inline iterator operator++(int) { iterator r = *this; ++i; return r; }
        inline iterator &operator--() { --i; return *this; }
        inline iterator operator--(int) { iterator r = *this; --i; return r; }
        inline iterator operator+(int j) const
        { iterator r = *this; r.i += j; return r; }
        inline iterator operator-(int j) const { return operator+(-j); }
        inline iterator &operator+=(int j) { i += j; return *this; }
        inline iterator &operator-=(int j) { i -= j; return *this; }

    public:
        inline bool operator==(const const_iterator &other) const { return i == other.i; }
        inline bool operator!=(const const_iterator &other) const { return i != other.i; }
    };
    friend class iterator;

    class const_iterator
    {
        friend class iterator;
        const QJsonObject *o;
        int i;

    public:
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef int difference_type;
        typedef QJsonValue value_type;
        typedef QJsonValue reference;

        inline const_iterator() : o(0), i(0) {}
        inline const_iterator(const QJsonObject *obj, int index)
            : o(obj), i(index) {}
        inline const_iterator(const iterator &other)
            : o(other.o), i(other.i) {}

        inline QString key() const { return o->keyAt(i); }
        inline QJsonValue value() const { return o->valueAt(i); }
        inline QJsonValue operator*() const { return o->valueAt(i); }
#ifdef Q_QDOC
        inline QJsonValue* operator->() const;
#else
        inline QJsonValuePtr operator->() const { return QJsonValuePtr(o->valueAt(i)); }
#endif
        inline bool operator==(const const_iterator &other) const { return i == other.i; }
        inline bool operator!=(const const_iterator &other) const { return i != other.i; }

        inline const_iterator &operator++() { ++i; return *this; }
        inline const_iterator operator++(int) { const_iterator r = *this; ++i; return r; }
        inline const_iterator &operator--() { --i; return *this; }
        inline const_iterator operator--(int) { const_iterator r = *this; --i; return r; }
        inline const_iterator operator+(int j) const
        { const_iterator r = *this; r.i += j; return r; }
        inline const_iterator operator-(int j) const { return operator+(-j); }
        inline const_iterator &operator+=(int j) { i += j; return *this; }
        inline const_iterator &operator-=(int j) { i -= j; return *this; }

        inline bool operator==(const iterator &other) const { return i == other.i; }
        inline bool operator!=(const iterator &other) const { return i != other.i; }
    };
    friend class const_iterator;

    // STL style
    inline iterator begin() { detach(); return iterator(this, 0); }
    inline const_iterator begin() const { return const_iterator(this, 0); }
    inline const_iterator constBegin() const { return const_iterator(this, 0); }
    inline iterator end() { detach(); return iterator(this, size()); }
    inline const_iterator end() const { return const_iterator(this, size()); }
    inline const_iterator constEnd() const { return const_iterator(this, size()); }
    iterator erase(iterator it);

    // more Qt
    typedef iterator Iterator;
    typedef const_iterator ConstIterator;
    iterator find(const QString &key);
    const_iterator find(const QString &key) const { return constFind(key); }
    const_iterator constFind(const QString &key) const;
    iterator insert(const QString &key, const QJsonValue &value);

    // STL compatibility
    typedef QJsonValue mapped_type;
    typedef QString key_type;
    typedef int size_type;

    inline bool empty() const { return isEmpty(); }

private:
    friend class QJsonPrivate::Data;
    friend class QJsonValue;
    friend class QJsonDocument;
    friend class QJsonValueRef;

    friend QJSON_EXPORT QDebug operator<<(QDebug, const QJsonObject &);

    QJsonObject(QJsonPrivate::Data *data, QJsonPrivate::Object *object);
    void initialize();
    void detach(uint reserve = 0);
    void compact();

    QString keyAt(int i) const;
    QJsonValue valueAt(int i) const;
    void setValueAt(int i, const QJsonValue &val);

    QJsonPrivate::Data *d;
    QJsonPrivate::Object *o;
};

Q_DECLARE_METATYPE(QJsonObject)

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_JSON_READONLY)
QJSON_EXPORT QDebug operator<<(QDebug, const QJsonObject &);
#endif

QT_END_NAMESPACE

#endif // QJSONOBJECT_H
