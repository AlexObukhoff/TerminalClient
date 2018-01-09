#ifndef STARTSP600DEFINES
#define STARTSP600DEFINES

const BYTE STX = 0x02;         // стартовый байт
const BYTE ETX = 0x03;         // терминальный байт
const BYTE ACK = 0x06;         // подтверждение работы
const BYTE DIV = 0x00;         // разделитель полей
const BYTE ESC = 0x1B;         // код Escape
const BYTE ERR = 0x30;         // признак ошибки, при выполнении команды

const BYTE PrintContinue = 0x04;  // продолжение печати
const BYTE PrintCancel = 0x01;    // отмена печати
const BYTE PrintExtInfo = 0x07;   // команда получения расширенного статуса

// Execution Codes
const BYTE EC_SUCCESS = 0x00;   // нет ошибок
const BYTE EC_BAD_ANSWER = 0x01; // неверный стартовый и стоповый байты
const BYTE EC_NO_ANSWER = 0x04; // нет ответа

// результат выполнения команды
const WORD CMDRES_SUCCESS = 0x0000;
const WORD CMDRES_FAILED = 0xFFFF;

// состояние ккм
const BYTE ST_SESSION_OPENED = 0x01;
const BYTE ST_SESSION_CLOSED = 0x02;
#endif
