/* @file Кэш состояний устройства. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
typedef QSet<int> TStatusCodes;

template <class T>
class StatusCache: public QMap<T, TStatusCodes>
{
public:
	using QMap::contains;
	using QMap::isEmpty;
	using QMap::size;

	const StatusCache<T> operator-(const StatusCache<T> & aStatusCache)
	{
		StatusCache<T> result(*this);

		foreach(T key, aStatusCache.keys())
		{
			if (result.contains(key))
			{
				result[key] -= aStatusCache[key];
			}
		}

		return result;
	}

	int size(T key) const
	{
		return contains(key) ? value(key).size() : 0;
	}

	int isEmpty(T key) const
	{
		return !contains(key) || value(key).isEmpty();
	}

	bool contains(int aStatusCode) const
	{
		foreach(const TStatusCodes & statusCodes, values())
		{
			if (statusCodes.contains(aStatusCode))
			{
				return true;
			}
		}

		return false;
	}
};

//--------------------------------------------------------------------------------
