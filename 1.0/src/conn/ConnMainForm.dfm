object Form1: TForm1
  Left = 958
  Top = 325
  Width = 258
  Height = 145
  Caption = 'ConnMainForm'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  PixelsPerInch = 96
  TextHeight = 13
  object CheckTimeTimer: TTimer
    Enabled = False
    Interval = 5000
    OnTimer = CheckTimeTimerTimer
    Left = 8
    Top = 8
  end
  object CheckThreadsTimer: TTimer
    Interval = 10000
    OnTimer = CheckThreadsTimerTimer
    Left = 40
    Top = 8
  end
end
