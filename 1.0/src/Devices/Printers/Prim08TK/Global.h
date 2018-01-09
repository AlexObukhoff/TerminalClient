#ifndef GlobalH
#define GlobalH

#define ACK								0x06
#define CAN								0x18
#define NAK								0x15
#define STX								0x02
#define ETX								0x03
#define SPR								0x1C

#define FirstDataByteAfterFrames                      21
#define FirstDataByteAfterFramesWithOutDateTime       9
#define CountByteOfFramesWithOutDateTime              14
#define CountByteOfFramesWithDateTime                 27
#define FirstDataByteAfterFramesInAnswer              29

// реальное расположение байтов в структуре будет c b0(hi) - b7(low)
struct  
	bytebits {
	unsigned b7	: 1;
	unsigned b6	: 1;
	unsigned b5 : 1;
	unsigned b4 : 1;
	unsigned b3 : 1;
	unsigned b2 : 1;
	unsigned b1 : 1;
	unsigned b0 : 1;
				};

struct  
	wordbits {
	unsigned b15 : 1;
	unsigned b14 : 1;
	unsigned b13 : 1;
	unsigned b12 : 1;
	unsigned b11 : 1;
	unsigned b10 : 1;
	unsigned b9 : 1;
	unsigned b8 : 1;
	unsigned b7 : 1;
	unsigned b6	: 1;
	unsigned b5 : 1;
	unsigned b4 : 1;
	unsigned b3 : 1;
	unsigned b2 : 1;
	unsigned b1 : 1;
	unsigned b0 : 1;
				};

union codes 
{
	struct bytebits	ByteBitCode;
	BYTE				ByteCode;
	struct wordbits	WordBitCode;
	WORD				WordCode;
};

enum tStringType {stDifferentLengthOfString, stEqualLengthOfString};

enum DOCTypes { dtZReport, dtXReport, dtSale, dtStornoSale, dtReturn, dtStornoReturn, dtBuy, dtStornoBuy};

enum TotalTypes { ttOnlyTotal, ttTotatWithReport };

enum TutelageMode { tmRequest, tmSet, tmReset };

//--- Виды отчетов ---

#define Z_REPORT 'Z'
#define X_REPORT 'X'

//---- Виды оплаты ---

#define MONEY     1
#define _CREDIT   2
#define DEBET     3
#define SB        4

#define CASH				"00"
#define CREDIT			"01"
#define CARD				"02"
#define PAYMENT4    "03"
#define PAYMENT5    "04"
#define PAYMENT6    "05"

//--- Типы чеков
#define SALE						"00"    //--- Продажа
#define STORNO_SALE     "01"
#define RETURN					"02"    //--- Возврат
#define STORNO_RETURN   "03"
#define BUY							"04"    //--- Покупка
#define STORNO_BUY      "05"

#define COMPLETED     0x54    //--- завершённый чек
#define UNCOMPLETED   0x43    //--- незавершённый чек

#define GDOC							0x47	// продажа
#define gDOC							0x67	// сторн продажи
#define SDOC							0x53	// наценка
#define sDOC							0x73	// сторн наценки
#define DDOC							0x44	// скидка
#define dDOC							0x64	// сторн скидки
#define RDOC							0x52	// тара
#define rDOC							0x72	// сторн тары
#define BDOC							0x42	// выплаты
#define CDOC							0x43	// возврат выплат
#define PDOC							0x50	// внесение
#define pDOC							0x70	// сторн внесения	
#define TDOC							0x54	// изъятие
#define tDOC							0x74	// сторн изъятия

struct CCD // ComplexChequeData
{
  char cDesc[22];
  float fPrice;
  float fQuantity;
};

struct sDateTime
{
	int Year;
	int Month;
	int Day;
	int Hour;
	int Min;
	int Sec;
};

//состояние фискального документа

#define FISCAL_DOCUMENT_CLOSED							0x00	// закрыт
#define FISCAL_DOCUMENT_ZAGOLOVOK						0x01	//заголовок
#define FISCAL_DOCUMENT_TOVAR								0x02	//товар
#define FISCAL_DOCUMENT_ITOG								0x03	//итог
#define FISCAL_DOCUMENT_RASCHET							0x04	//расчёт
#define FISCAL_DOCUMENT_ZAVERSHENIE					0x05	//завершение
#define FISCAL_DOCUMENT_SKIDKA							0x06	//скидка/наценка на итог
#define FISCAL_DOCUMENT_UNDEFINED						0x07	//неопределено

// режим регистратора
#define TRAIN_MODE													0x00	//технологический режим
#define FISCAL_MODE													0x01	//фискальный режим
#define UNKNOWN_MODE												0x03  // режим неопределён

// состояние смены
#define SHIFT_CLOSED												0x00
#define SHIFT_OPENED												0x01	


#define PE_OK                                 0x00
#define PE_ERROR_OF_STATUS_RECEIVE            0x90
//Постоянный статус регистратора
#define PE_HARDWARE_ERROR                     0x41
#define PE_WORKING_MEMORY_ERROR               0x42
#define PE_FISCAL_MEMORY_ERROR                0x43
#define PE_FISCAL_MODE_NOT_SET                0x44
#define PE_FISCAL_MEMORY_ABOUT_OVERFLOW       0x45
#define PE_REGISTRATION_COUNT_OVERFLOW        0x46
#define PE_HARDWARE_SERIAL_NUMBER_NOT_SET     0x47

//Текущий статус регистратора
#define PE_NEED_TO_CLOSE_SHIFT                 0x48
#define PE_PREVIOUS_COMMAND_UNRECOGNIZED       0x49
#define PE_PREVIOUS_COMMAND_NOT_COMPLETE       0x50
#define PE_SESSION_NOT_CLOSED                  0x51
#define PE_FISCAL_MODE                         0x52
#define PE_TRAIN_MODE                          0x53
#define PE_DOC_BUFFER_ABOUT_OVERFLOW           0x54
#define PE_SHIFT_CLOSED                        0x55
#define PE_BUFFER_OVERFLOW                     0x56
#define PE_PAPER_OVER                          0x57

// Коды ошибок выполнения команды
#define PE_INCORRECT_COMMAND_FORMAT            0x01
#define PE_INCORRECT_COMMAND_FIELD             0x02
#define PE_DATE_TIME_ERROR                     0x03
#define PE_BCC_ERROR                           0x04
#define PE_PASSWORD_ERROR                      0x05
#define PE_INCORRECT_COMMAND_NUMBER            0x06
#define PE_NEED_COMMAND_BEGIN_SESSION          0x07
#define PE_TIME_OVERFLOW                       0x08
#define PE_STRING_FIELD_OVERFLOW               0x09
#define PE_COMMAND_LENGTH_OVERFLOW             0x0A
#define PE_INCORRECT_OPERATION                 0x0B
#define PE_FIELD_VALUE_ERROR                   0x0C
#define PE_INCORRECT_COMMAND_WITH_CURRENT_STATE 0x0D
#define PE_NEEDED_STRING_FIELD_ERROR           0x0E
#define PE_RESULT_OVERFLOW                     0x0F
#define PE_MONEY_COUNT_ERROR                   0x10
#define PE_BACK_OPERATION_ERROR_WITHOUT_DIRECT_OPERATION 0x11
#define PE_NO_CASH_ERROR                       0x12
#define PE_BACK_OPERATION_RESULT_OVERFLOW      0x13
#define PE_PRINTER_NOT_SERTIFICATED            0x14
#define PE_Z_REPORT_NEEDED                     0x15
#define PE_FISCAL_PRINTER_FAILURE              0x17
#define PE_PRINTER_NOT_READY                   0x18
#define PE_PAPER_ABOUT_OVER                    0x19
#define PE_PRINTER_NOT_FISCAL                  0x1A
#define PE_INSPECTOR_PASSWORD_INCORRECT        0x1B
#define PE_PRINTER_HAS_SERTIFICATED            0x1C
#define PE_FISCAL_COUNT_OVERFLOW               0x1D

#define PE_NONCORRECT_PRINTING_BUFFER          0x1E
#define PE_NONCORRECT_G_FIELD                  0x1F

#define PE_PAYMENT_TYPE_NUMBER_INCORRECT       0x20
#define PE_TIMEOUT_OF_RECIEVE_ERROR            0x21
#define PE_RECIEVE_ERROR                       0x22
#define PE_PRINTER_STATE_ERROR                 0x23
#define PE_OPERATION_COUNT_ERROR               0x24
#define PE_BEGIN_SHIFT_COMMAND_NEED            0x25
#define PE_PAYMENT_NUMBER_INCORRECT            0x27
#define PE_NONCORRECT_PRINTER_STATE            0x28
#define PE_SHIFT_ALREADY_OPENED                0x29
#define PE_DATE_ERROR                          0x2B

#define PE_NOT_SPACE_FOR_ADDING_DEPART         0x2C
#define PE_DEPARTS_INDEX_ALREADY_EXIST         0x2D
#define PE_DEPART_CANT_DELETED                 0x2E
#define PE_DEPARTS_INDEX_NOT_FOUND             0x2F

#define PE_FISCAL_MEMORY_FAILURE               0x30
#define PE_OPERATION_TIME_ERROR                0x31
#define PE_FISCAL_MEMORY_NOT_INITIATED         0x32
#define PE_FISCAL_MEMORY_OVERFLOW              0x33

#define PE_NONCORRECT_START_SYMBOL                0x34
#define PE_UNKNOWN_ANSWER_FROM_EKLZ               0x35
#define PE_UNKNOWN_COMMAND_EKLZ                   0x36
#define PE_UNKNOWN_STATE_EKLZ                     0x37
#define PE_TIMEOUT_RECIEVING_EKLZ                 0x38
#define PE_TIMEOUT_TRANSMITING_EKLZ               0x39
#define PE_NONCORRECT_CRC_ANSWER_FROM_EKLZ        0x3A
#define PE_NONCORRECT_STATE_EKLZ                  0x3B
#define PE_NO_FREE_SPACE_EKLZ                     0x3C
#define PE_NONCORRECT_CRC_COMMAND_EKLZ            0x3D
#define PE_EKLZ_NOT_FOUND                         0x3E
#define PE_DATA_EKLZ_NOT_EXIST                    0x3F
#define PE_DATA_EKLZ_UNSYNCHRONIZED               0x40
//#define PE_ERROR_RIK_STATE                        0x41
//#define PE_NONCORRECT_DATETIME_IN_EKLZ_COMMAND    0x42
//#define PE_TIME_OF_EKLZ_EXPIRED                   0x43
//#define PE_EKLZ_OVERFLOW                          0x44
//#define PE_ACTIVATION_TIMES_EXPIRED               0x45
//#define PE_NEED_PRINTING_SKL                      0x51
//#define PE_ERROR_SKL_STATE                        0x52
#define PE_ERROR_PRINT_STRING_CREATING            0x95

// Коды ошибок состояния фискального блока
#define PE_BATTERY                             0x58
#define PE_DATA_INCORRECT                      0x59
#define PE_INAPPROPRIATE_COMMAND               0x60
#define PE_RECEIPT_OPENED                      0x61
#define PE_DOCUMENT_OPENED                     0x62
#define PE_RECEIPT_NOT_OPENED                  0x63
#define PE_FISCAL_MEMORY_CHECKSUM_ERROR        0x64
#define PE_WORKING_MEMORY_CHECKSUM_ERROR       0x65

// Коды ошибок состояния принтера
#define PE_LINK_ERROR                          0x66
#define PE_DOCUMENT_PRESENT                    0x67
#define PE_BUFFER_CLEAR                        0x68

// Коды ошибок при работе с СОМ портом
#define PE_DIFFERBYTE_ERROR                    0x69
#define PE_ERRORRESET_ERROR                    0x70
#define PE_WRITEFILE_ERROR                     0x71
#define PE_READFILE_ERROR                      0x72

//Состояние фискального документа
#define DOC_CLOSED                             0x73
#define DOC_ZAGOLOVOK                          0x74
#define DOC_TOVAR                              0x75
#define DOC_ITOG                               0x76
#define DOC_RASCHET                            0x77
#define DOC_ZAVERSHENIE                        0x78
#define DOC_SKIDKA_NA_ITOG                     0x79

#define END_OF_SEQUENCE                        0xFF

#endif
