/* @file Прокси-класс для использования константы контейнера в виде класса для статитических вызовов. */

#pragma once

//---------------------------------------------------------------------------
template<class T>
class ContainerProxy: public T
{
public:
	ContainerProxy() {}
	ContainerProxy(const T & aOther)
	{
		operator = (aOther);
	}

	const T & operator= (const T & aOther)
	{
		this->append(aOther);

		return *this;
	}

	T & data()
	{
		return * dynamic_cast<T *>(this);
	}
};

//---------------------------------------------------------------------------
