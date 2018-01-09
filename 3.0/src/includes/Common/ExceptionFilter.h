#pragma once

#include <iostream>

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

#include "ILog.h"

//---------------------------------------------------------------------------
namespace
{
	void ExceptionFilter(const QString& file, const QString& function, int line, ILog * log, bool aThrow)
	{
		try
		{
			throw;
		}
		catch(std::exception& ex)
		{
			LOG(log, LogLevel::Error, QString("std::exception in file: %1 ,from function: %2, catch line: %3, message: %4.").arg(file).arg(function).arg(line).arg(ex.what()));
		}
		catch(...)
		{
			LOG(log, LogLevel::Error, QString("Unknown Exception in file: %1 ,from function: %2, catch line: %3.").arg(file).arg(function).arg(line));
			
			if (aThrow)
			{
				throw;
			}
		}
	}

	void ExceptionFilterCout(const QString& file, const QString& function, int line, const std::string & message)
	{
		try
		{
			std::cout << "message: " << message << std::endl;
			throw;
		}
		catch(std::exception& ex)
		{
			std::cout << "std::exception in file: " << file.toStdString() << ", from function: " << function.toStdString() 
				<< ", catch line: " << line << ", message: " << ex.what() << std::endl;
		}
		catch(...)
		{
			std::cout << "Unknown Exception in file: " << file.toStdString() << ", from function: " << function.toStdString() 
				<< ", catch line: " << line << std::endl;
		}
	}
}

//---------------------------------------------------------------------------
#define EXCEPTION_FILTER(log) ExceptionFilter(__FILE__, __FUNCTION__, __LINE__, log, true)

#define EXCEPTION_FILTER_NO_THROW(log) ExceptionFilter(__FILE__, __FUNCTION__, __LINE__, log, false)

#define EXCEPTION_FILTER_COUT(string) ExceptionFilterCout(__FILE__, __FUNCTION__, __LINE__, string)

//---------------------------------------------------------------------------
