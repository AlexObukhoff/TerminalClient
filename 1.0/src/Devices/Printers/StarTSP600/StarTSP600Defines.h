#ifndef STARTSP600DEFINES
#define STARTSP600DEFINES

const BYTE STX = 0x02;         // ��������� ����
const BYTE ETX = 0x03;         // ������������ ����
const BYTE ACK = 0x06;         // ������������� ������
const BYTE DIV = 0x00;         // ����������� �����
const BYTE ESC = 0x1B;         // ��� Escape
const BYTE ERR = 0x30;         // ������� ������, ��� ���������� �������

const BYTE PrintContinue = 0x04;  // ����������� ������
const BYTE PrintCancel = 0x01;    // ������ ������
const BYTE PrintExtInfo = 0x07;   // ������� ��������� ������������ �������

// Execution Codes
const BYTE EC_SUCCESS = 0x00;   // ��� ������
const BYTE EC_BAD_ANSWER = 0x01; // �������� ��������� � �������� �����
const BYTE EC_NO_ANSWER = 0x04; // ��� ������

// ��������� ���������� �������
const WORD CMDRES_SUCCESS = 0x0000;
const WORD CMDRES_FAILED = 0xFFFF;

// ��������� ���
const BYTE ST_SESSION_OPENED = 0x01;
const BYTE ST_SESSION_CLOSED = 0x02;
#endif
