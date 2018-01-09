object Form1: TForm1
  Left = 506
  Top = 158
  HorzScrollBar.Visible = False
  VertScrollBar.Visible = False
  ActiveControl = CppWebBrowser1
  BorderIcons = []
  BorderStyle = bsNone
  Caption = 'WebClientForm'
  ClientHeight = 380
  ClientWidth = 630
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poMainFormCenter
  WindowState = wsMaximized
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object CppWebBrowser1: TCppWebBrowser
    Left = 0
    Top = 0
    Width = 630
    Height = 380
    TabStop = False
    Align = alClient
    TabOrder = 0
    OnBeforeNavigate2 = CppWebBrowser1BeforeNavigate2
    OnDocumentComplete = CppWebBrowser1DocumentComplete
    ControlData = {
      4C0000001D410000462700000000000000000000000000000000000000000000
      000000004C000000000000000000000001000000E0D057007335CF11AE690800
      2B2E126201000000000000004C0000000114020000000000C000000000000046
      8000000000000000000000000000000000000000000000000000000000000000
      00000000000000000100000000000000000000000000000000000000}
  end
  object CCNETStateChange: TTimer
    Enabled = False
    Interval = 300
    OnTimer = CCNETStateChangeTimer
    Left = 12
    Top = 40
  end
  object CheckTimeTimer: TTimer
    Enabled = False
    Interval = 5000
    OnTimer = CheckTimeTimerTimer
    Left = 12
    Top = 80
  end
  object ApplicationEvents1: TApplicationEvents
    OnMessage = ApplicationEvents1Message
    OnRestore = ApplicationEvents1Restore
    Left = 12
    Top = 8
  end
  object CheckThreadsTimer: TTimer
    Enabled = False
    Interval = 10000
    OnTimer = CheckThreadsTimerTimer
    Left = 12
    Top = 116
  end
  object PaymentTimeOutTimer: TTimer
    Enabled = False
    Interval = 60000
    OnTimer = PaymentTimeOutTimerTimer
    Left = 56
    Top = 8
  end
  object StartAppTimer: TTimer
    Interval = 2000
    OnTimer = StartAppTimerTimer
    Left = 56
    Top = 40
  end
  object CheckPrinterStateTimer: TTimer
    Enabled = False
    Interval = 10000
    OnTimer = CheckPrinterStateTimerTimer
    Left = 88
    Top = 8
  end
  object Timer1: TTimer
    Enabled = False
    OnTimer = Timer1Timer
    Left = 56
    Top = 80
  end
  object process: TTimer
    Interval = 200
    OnTimer = processTimer
    Left = 16
    Top = 152
  end
end
