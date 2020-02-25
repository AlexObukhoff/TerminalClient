/* @file Класс для выполнения функционала при выходе из области видимости. */
/* @brief Если отпадает необходимость в его вызове - вызвать reset(). */

#pragma once

// STL
#include <functional>

typedef std::function<void()> TVoidMethod;

//---------------------------------------------------------------------------
class ExitAction
{
public:
	ExitAction(const TVoidMethod & aAction) : mAction(aAction) {}
	~ExitAction() { if (mAction) mAction(); }

	bool reset(const TVoidMethod & aAction = TVoidMethod()) { mAction = aAction; return true; }

private:
	TVoidMethod mAction;
};

//---------------------------------------------------------------------------
