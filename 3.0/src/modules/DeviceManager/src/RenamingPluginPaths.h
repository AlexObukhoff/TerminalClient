/* @file Переимнование путей плагинов для учета типа взаимодействия с устройством. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
class RenamePluginPath : public QMap<QString, QString>
{
public:
	RenamePluginPath()
	{
		insert("Common.Driver.BillAcceptor",                                            "Common.Driver.BillAcceptor.COM");
		insert("Common.Driver.BillAcceptor.Virtual",                                    "Common.Driver.BillAcceptor.System.Virtual");
		insert("Common.Driver.Camera.DirectX",                                          "Common.Driver.Camera.External");
		insert("Common.Driver.Camera.WebCamera",                                        "Common.Driver.Camera.External");
		insert("Common.Driver.CardReader.Creator reader",                               "Common.Driver.CardReader.USB.Creator");
		insert("Common.Driver.CardReader.PCSC reader",                                  "Common.Driver.CardReader.System.PCSC");
		insert("Common.Driver.CardReader.SamaraTicket",                                 "Common.Driver.CardReader.External.SamaraTicket");
		insert("Common.Driver.CoinAcceptor.ccTalk coin acceptor - base functional",     "Common.Driver.CoinAcceptor.COM.ccTalk");
		insert("Common.Driver.CoinAcceptor.ccTalk coin acceptor with 2-stage enabling", "Common.Driver.CoinAcceptor.COM.ccTalk.ComplexEnabling");
		insert("Common.Driver.Dispenser.PuloonLCDM",                                    "Common.Driver.Dispenser.COM.Puloon");
		insert("Common.Driver.Dispenser.Virtual",                                       "Common.Driver.Dispenser.System.Virtual");
		insert("Common.Driver.DocumentPrinter.ATOL DP",                                 "Common.Driver.DocumentPrinter.COM.ATOL");
		insert("Common.Driver.FiscalRegistrator.ATOL FR",                               "Common.Driver.FiscalRegistrator.COM.ATOL");
		insert("Common.Driver.FiscalRegistrator.PayPPU-700K",                           "Common.Driver.FiscalRegistrator.COM.ATOL.PayPPU700K");
		insert("Common.Driver.FiscalRegistrator.PayVKP-80K",                            "Common.Driver.FiscalRegistrator.COM.ATOL.PayVKP80K");
		insert("Common.Driver.FiscalRegistrator.CommonIncotex",                         "Common.Driver.FiscalRegistrator.COM.Incotex");
		insert("Common.Driver.FiscalRegistrator.CommonPRIM",                            "Common.Driver.FiscalRegistrator.COM.PRIM");
		insert("Common.Driver.FiscalRegistrator.PresenterPRIM",                         "Common.Driver.FiscalRegistrator.COM.PRIM.Presenter");
		insert("Common.Driver.FiscalRegistrator.EjectorPRIM",                           "Common.Driver.FiscalRegistrator.COM.PRIM.Ejector");
		insert("Common.Driver.FiscalRegistrator.CommonShtrih",                          "Common.Driver.FiscalRegistrator.COM.Shtrih");
		insert("Common.Driver.FiscalRegistrator.RetractorShtrih",                       "Common.Driver.FiscalRegistrator.COM.Shtrih.Ejector");
		insert("Common.Driver.FiscalRegistrator.Shtrih-M Shtrih Kiosk-FR-K",            "Common.Driver.FiscalRegistrator.COM.Shtrih.Kiosk");
		insert("Common.Driver.FiscalRegistrator.Shtrih-M Yarus-01K",                    "Common.Driver.FiscalRegistrator.COM.Shtrih.Yarus01K");
		insert("Common.Driver.FiscalRegistrator.CommonSpark",                           "Common.Driver.FiscalRegistrator.COM.SPARK");
		insert("Common.Driver.FiscalRegistrator.Multisoft MStar-TUP-K based on OPOS",   "Common.Driver.FiscalRegistrator.OPOS.Multisoft");
		insert("Common.Driver.IOPort.COM",                                              "Common.Driver.IOPort.System.COM");
		insert("Common.Driver.IOPort.USB",                                              "Common.Driver.IOPort.System.USB");
		insert("Common.Driver.Health.Win32",                                            "Common.Driver.Health.System");
		insert("Common.Driver.Modem",                                                   "Common.Driver.Modem.COM");
		insert("Common.Driver.Printer.Default POS Printer",                             "Common.Driver.Printer.COM.POS");
		insert("Common.Driver.Printer.Citizen CBM-1000II",                              "Common.Driver.Printer.COM.POS.CitizenCBM1000II");
		insert("Common.Driver.Printer.Citizen CPP-8001",                                "Common.Driver.Printer.COM.POS.CitizenCPP8001");
		insert("Common.Driver.Printer.Citizen CT-S310II",                               "Common.Driver.Printer.COM.POS.CitizenCTS310II");
		insert("Common.Driver.Printer.Citizen CTS-2000",                                "Common.Driver.Printer.COM.POS.CitizenCTS2000");
		insert("Common.Driver.Printer.Citizen PPU-231",                                 "Common.Driver.Printer.COM.POS.CitizenPPU231");
		insert("Common.Driver.Printer.Citizen PPU-700",                                 "Common.Driver.Printer.COM.POS.CitizenPPU700");
		insert("Common.Driver.Printer.Custom Printer",                                  "Common.Driver.Printer.COM.POS.Custom");
		insert("Common.Driver.Printer.Custom VKP-80",                                   "Common.Driver.Printer.COM.POS.CustomVKP80");
		insert("Common.Driver.Printer.Custom VKP-80 III",                               "Common.Driver.Printer.COM.POS.CustomVKP80III");
		insert("Common.Driver.Printer.Star Printer",                                    "Common.Driver.Printer.COM.STAR");
		insert("Common.Driver.Printer.Ejector Star Printer",                            "Common.Driver.Printer.COM.STAR.Ejector");
		insert("Common.Driver.Printer.System",                                          "Common.Driver.Printer.System");
		insert("Common.Driver.Printer.Sunphor POS58IV",                                 "Common.Driver.Printer.System.Sunphor.POS58IV");
		insert("Common.Driver.Printer.ICT GP83",                                        "Common.Driver.Printer.System.ICT.GP83");
		insert("Common.Driver.Printer.SysFuture AV-268",                                "Common.Driver.Printer.COM.SysFuture");
		insert("Common.Driver.Scanner.Generic serial HID",                              "Common.Driver.Scanner.COM");
		insert("Common.Driver.Scanner.Generic Serial HID",                              "Common.Driver.Scanner.USB");
		insert("Common.Driver.Scanner.USB Scanner",                                     "Common.Driver.Scanner.USB");
		insert("Common.Driver.Scanner.OPOS Metrologic Scanner",                         "Common.Driver.Scanner.OPOS.Honeywell");
		insert("Common.Driver.Token.Token",                                             "Common.Driver.Token.System");
		insert("Common.Driver.Watchdog.Alarm",                                          "Common.Driver.Watchdog.COM.Alarm");
		insert("Common.Driver.Watchdog.LDog",                                           "Common.Driver.Watchdog.COM.LDog");
		insert("Common.Driver.Watchdog.OSMP",                                           "Common.Driver.Watchdog.COM.OSMP");
		insert("Common.Driver.Watchdog.OSMP2",                                          "Common.Driver.Watchdog.COM.OSMP2");
		insert("Common.Driver.Watchdog.OSMP2.5",                                        "Common.Driver.Watchdog.COM.OSMP2,5");
		insert("Common.Driver.Watchdog.Platix",                                         "Common.Driver.Watchdog.COM.Platix");
		insert("Common.Driver.Watchdog.STOD",                                           "Common.Driver.Watchdog.COM.STOD");
	}
};

//---------------------------------------------------------------------------
