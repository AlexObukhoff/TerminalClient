/* @file Получение пароля для секретных ключей терминала (rootp, roots). */

// Project
#include "CryptEngine.h"

//---------------------------------------------------------------------------
QList<QByteArray> CryptEngine::getRootPassword() const
{
	QList<QByteArray> result;

	result << "This is my secret password";

	return result;
}

//---------------------------------------------------------------------------
