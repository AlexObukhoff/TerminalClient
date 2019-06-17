/* @file Интерфейсы сервисного меню */

#pragma once

class ServiceMenuBackend;

//------------------------------------------------------------------------
class IServiceWindow
{
public:
	virtual bool initialize() = 0;
	virtual bool shutdown() = 0;

	virtual bool activate() = 0;
	virtual bool deactivate() = 0;

	virtual ~IServiceWindow() {}
};

//------------------------------------------------------------------------
class ServiceWindowBase : public IServiceWindow
{
public:
	ServiceWindowBase(ServiceMenuBackend * aBackend)
		: mBackend(aBackend)
	{
	}

protected:
	ServiceMenuBackend * mBackend;
};

//------------------------------------------------------------------------
