/* @file Базовый ФР ПРИМ c эжектором. */

#pragma once

#include "../Presenter/PrimPresenterFR.h"

//--------------------------------------------------------------------------------
template <class T>
class PrimEjectorFR : public PrimPresenterFR<T>
{
	SET_SUBSERIES("Ejector")

public:
	PrimEjectorFR();

protected:
	/// Инициализация устройства.
	virtual bool updateParameters();

	/// Анализирует коды статусов устройства и фильтрует лишние.
	virtual void cleanStatusCodes(TStatusCodes & aStatusCodes);

	/// Проверить длину презентации.
	bool checkPresentationLength();

	/// Выкинуть чек.
	virtual bool push();

	/// Забрать чек в ретрактор.
	virtual bool retract();

	/// Презентовать чек.
	virtual bool present();

	/// Установить режим работы презентера.
	virtual bool setPresentationMode();

	/// Обработка ответа предыдущей команды. Автоисправление некоторых ошибок.
	virtual bool processAnswer(char aError);

	/// Выполнить/установить действие эжектора.
	bool processEjectorAction(const QString & aAction);

	/// Старый номер билда?
	bool mOldBuildNumber;

	/// Количество неисправимых статусов в LPC22 когда надо послать специальную команду.
	int mLPC22RetractorErrorCount;
};

//--------------------------------------------------------------------------------
