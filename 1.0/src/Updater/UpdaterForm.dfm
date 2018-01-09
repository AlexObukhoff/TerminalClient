object Form1: TForm1
  Left = 255
  Top = 0
  BorderStyle = bsNone
  Caption = 'WebClientUpdaterForm'
  ClientHeight = 926
  ClientWidth = 862
  Color = 16776176
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  WindowState = wsMaximized
  OnClose = FormClose
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 112
    Top = 472
    Width = 1093
    Height = 77
    Caption = #1058#1077#1088#1084#1080#1085#1072#1083' '#1074#1088#1077#1084#1077#1085#1085#1086' '#1085#1077' '#1088#1072#1073#1086#1090#1072#1077#1090
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clRed
    Font.Height = -64
    Font.Name = 'Tahoma'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object StartTimer: TTimer
    Interval = 5000
    OnTimer = StartTimerTimer
    Left = 8
    Top = 8
  end
end
