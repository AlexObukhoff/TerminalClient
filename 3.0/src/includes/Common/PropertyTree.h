/* @file Специализация boost::property_tree. */

#pragma once

// boost
#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/property_tree/id_translator.hpp>

// std
#include <string>
#include <iterator>
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QTime>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
typedef boost::property_tree::basic_ptree<std::string, std::wstring> TPtree;

//---------------------------------------------------------------------------
// Custom translator that works with std::wstring
template <class Ext, class Int = std::wstring>
struct WStringTranslator
{
	typedef Ext external_type;
	typedef Int internal_type;

	external_type get_value(const internal_type &value) const;
	internal_type put_value(const external_type &value) const;
};

//---------------------------------------------------------------------------
namespace boost { namespace property_tree {

template <typename T>
struct translator_between<std::wstring, T>
{
	typedef WStringTranslator<T> type;
};

template<>
struct translator_between<std::wstring, std::wstring>
{
	typedef id_translator<std::wstring> type;
};

}}

//---------------------------------------------------------------------------
template<> 
inline qint64 WStringTranslator<qint64>::get_value(const std::wstring & aValue) const
{ 
	try
	{
		return std::stoll(aValue);
	}
	catch (...)
	{
		return 0;
	}
};

//---------------------------------------------------------------------------
template<> 
inline std::wstring WStringTranslator<qint64>::put_value(const qint64 & aValue) const
{ 
	return QString::number(aValue).toStdWString();
}

//---------------------------------------------------------------------------
template<> 
inline QString WStringTranslator<QString>::get_value(const std::wstring & aValue) const
{ 
	return QString::fromStdWString(aValue); 
};

//---------------------------------------------------------------------------
template<> 
inline std::wstring WStringTranslator<QString>::put_value(const QString & aValue) const
{ 
	return aValue.toStdWString(); 
}

//---------------------------------------------------------------------------
template<> 
inline int WStringTranslator<int>::get_value(const std::wstring & aValue) const
{
	return QString::fromStdWString(aValue).toInt();
};

//---------------------------------------------------------------------------
template<> 
inline std::wstring WStringTranslator<int>::put_value(const int & aValue) const
{
	return QString::number(aValue).toStdWString();
}

//---------------------------------------------------------------------------
template<> 
inline ulong WStringTranslator<ulong>::get_value(const std::wstring & aValue) const
{
	try
	{
		return std::stoul(aValue);
	}
	catch (...)
	{
		return 0;
	}
};

//---------------------------------------------------------------------------
template<> 
inline std::wstring WStringTranslator<ulong>::put_value(const ulong & aValue) const
{
	return QString::number(aValue).toStdWString();
}

//---------------------------------------------------------------------------
template<> 
inline double WStringTranslator<double>::get_value(const std::wstring & aValue) const
{
	try
	{
		return std::stod(aValue);
	}
	catch (...)
	{
		return 0;
	}
};

//---------------------------------------------------------------------------
template<> 
inline std::wstring WStringTranslator<double>::put_value(const double & aValue) const
{
	return QString::number(aValue, 'f').toStdWString();
}

//---------------------------------------------------------------------------
template<>
inline std::wstring WStringTranslator<const char *>::put_value(const char * const & aValue) const
{
	return QString::fromLatin1(aValue).toStdWString();
}

//---------------------------------------------------------------------------
template<> 
inline bool WStringTranslator<bool>::get_value(const std::wstring & aValue) const
{
	QString v = QString::fromStdWString(aValue);

	return v.compare("true", Qt::CaseInsensitive) == 0 ||
		v.compare("yes", Qt::CaseInsensitive) == 0 ||
		v.toInt() != 0;
};

//---------------------------------------------------------------------------
template<> 
inline std::wstring WStringTranslator<bool>::put_value(const bool & aValue) const
{
	return aValue ? L"true" : L"false"; 
}

//---------------------------------------------------------------------------
template<>
inline QTime WStringTranslator<QTime>::get_value(const std::wstring & aValue) const
{
	QString v = QString::fromStdWString(aValue);

	return QTime::fromString(v);
};

//---------------------------------------------------------------------------
template<>
inline std::wstring WStringTranslator<QTime>::put_value(const QTime & aValue) const
{
	return aValue.toString("hh:mm:ss").toStdWString();
}

//---------------------------------------------------------------------------
// For std::string with utf-8 encoding
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Custom translator that works with std::string (utf-8)
template <class Ext, class Int = std::string>
struct StringTranslator
{
	typedef Ext external_type;
	typedef Int internal_type;

	external_type get_value(const internal_type &value) const;
	internal_type put_value(const external_type &value) const;
};

//---------------------------------------------------------------------------
namespace boost {
	namespace property_tree {

		template <typename T>
		struct translator_between<std::string, T>
		{
			typedef StringTranslator<T> type;
		};

		template<>
		struct translator_between<std::string, std::string>
		{
			typedef id_translator<std::string> type;
		};

	}
}

//---------------------------------------------------------------------------
template<>
inline qint64 StringTranslator<qint64>::get_value(const std::string & aValue) const
{
	try
	{
		return std::stoll(aValue);
	}
	catch (...)
	{
		return 0;
	}
};

//---------------------------------------------------------------------------
template<>
inline std::string StringTranslator<qint64>::put_value(const qint64 & aValue) const
{
	return QString::number(aValue).toStdString();
}

//---------------------------------------------------------------------------
template<>
inline QString StringTranslator<QString>::get_value(const std::string & aValue) const
{
	return QString::fromUtf8(aValue.data());
};

//---------------------------------------------------------------------------
template<>
inline std::string StringTranslator<QString>::put_value(const QString & aValue) const
{
	return std::string(aValue.toUtf8());
}

//---------------------------------------------------------------------------
template<>
inline int StringTranslator<int>::get_value(const std::string & aValue) const
{
	return QString::fromUtf8(aValue.c_str()).toInt();
};

//---------------------------------------------------------------------------
template<>
inline std::string StringTranslator<int>::put_value(const int & aValue) const
{
	return QString::number(aValue).toStdString();
}

//---------------------------------------------------------------------------
template<>
inline ulong StringTranslator<ulong>::get_value(const std::string & aValue) const
{
	try
	{
		return std::stoul(aValue);
	}
	catch (...)
	{
		return 0;
	}
};

//---------------------------------------------------------------------------
template<>
inline std::string StringTranslator<ulong>::put_value(const ulong & aValue) const
{
	return QString::number(aValue).toStdString();
}

//---------------------------------------------------------------------------
template<>
inline double StringTranslator<double>::get_value(const std::string & aValue) const
{
	try
	{
		return std::stod(aValue);
	}
	catch (...)
	{
		return 0;
	}
};

//---------------------------------------------------------------------------
template<>
inline std::string StringTranslator<double>::put_value(const double & aValue) const
{
	return QString::number(aValue, 'f').toStdString();
}

//---------------------------------------------------------------------------
template<>
inline std::string StringTranslator<const char *>::put_value(const char * const & aValue) const
{
	return std::string(aValue);
}

//---------------------------------------------------------------------------
template<>
inline bool StringTranslator<bool>::get_value(const std::string & aValue) const
{
	QString v = QString::fromUtf8(aValue.c_str());

	return v.compare("true", Qt::CaseInsensitive) == 0 ||
		v.compare("yes", Qt::CaseInsensitive) == 0 ||
		v.toInt() != 0;
};

//---------------------------------------------------------------------------
template<>
inline std::string StringTranslator<bool>::put_value(const bool & aValue) const
{
	return aValue ? "true" : "false";
}

//---------------------------------------------------------------------------
template<>
inline QTime StringTranslator<QTime>::get_value(const std::string & aValue) const
{
	QString v = QString::fromUtf8(aValue.c_str());

	return QTime::fromString(v);
};

//---------------------------------------------------------------------------
template<>
inline std::string StringTranslator<QTime>::put_value(const QTime & aValue) const
{
	return aValue.toString("hh:mm:ss").toStdString();
}

//---------------------------------------------------------------------------
