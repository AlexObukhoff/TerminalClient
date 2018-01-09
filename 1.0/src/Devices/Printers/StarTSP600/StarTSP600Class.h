//---------------------------------------------------------------------------

#ifndef StarTSP600ClassH
#define StarTSP600ClassH
//---------------------------------------------------------------------------
#endif

#include <string>
#include "CPrinter.h"

class CStarTSP600 : public CPrinter
{
 private:
  std::string printerPassCode;
  std::string cmdResult[0xFF];

  WORD translateToNumeric(BYTE* buffer, unsigned int size);
  void logStatus(WORD kkmStatus, BYTE status1, BYTE status2, BYTE status3, BYTE status4); // логирование статуса ККМ
  void prepareCmdResults();

  // методы подготовки команд принтера
  void prologueCmd(BYTE cmdNum); // записать пролог команды
  void epilogueCmd(); // записать эпилог команды
  void addStringToCommand(std::string s); // добавить строку к команде + разделитель полей
  void addByteToCommandAsString(BYTE b); // добавить байт к команде + разделитель полей
  void addByteToCommand(BYTE b); // добавить байт к команде + разделитель полей

  // команды ККМ
  WORD cmd_31(std::string cashierName); // регистрация кассира
  WORD cmd_36(std::string text); // печать нефискального документа
  WORD cmd_45(); // запрос инф. о версии ККМ
  WORD cmd_52(BYTE linesFeed, bool cut); // управление прогоном и отрезом ленты
  //WORD cmd_53(double cash, std::string text); // фискальный документ
  WORD cmd_5C(std::string param); // получить параметры ККМ
  WORD cmd_4F(); // повторить ответ
  WORD cmd_5F(BYTE type, BYTE flags); // печать отчета
  WORD cmd_53_openDocument(double price); // начать оформление документа
  WORD cmd_53_cancelDocument(); // аннулировать документ
  WORD cmd_53_closeDocument();

  void cmd_53_fiscalField(unsigned int type, unsigned int flags, const std::string& data, unsigned int row, unsigned int col = 0);
  void cmd_53_fiscalSale(const std::string& price, unsigned int row, unsigned int col = 0);

  WORD execCmd();
 public:
  CStarTSP600(const std::string& printerName, int comPort, int baudRate = 0, TLogClass* logClass = NULL);
  virtual ~CStarTSP600();

  int Initialize();
  bool IsPrinterEnable();
  bool IsItYou();
  void GetState();

  void PrintCheck(AnsiString text, std::string barcode = ""); // Обычный чек
  void PrintCheck(TStringList* text) {};
  void PrintCheck(double Money, AnsiString Text); // фискальный чек
  void PrintXReport(AnsiString Text = "");
  void PrintZReport(AnsiString Text = "");
};
