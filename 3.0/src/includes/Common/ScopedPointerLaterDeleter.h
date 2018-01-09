#pragma once

//---------------------------------------------------------------------------
template <class T>
class ScopedPointerLaterDeleter
{
public:
	static inline void cleanup(T * aPointer)
	{
		if (aPointer)
		{
			aPointer->deleteLater();
		}
	}
};

//---------------------------------------------------------------------------
