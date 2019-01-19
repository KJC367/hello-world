object Form1: TForm1
  Left = 213
  Top = 128
  Width = 383
  Height = 226
  Caption = 'Movie Database '
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnClose = FormClose
  DesignSize = (
    367
    167)
  PixelsPerInch = 96
  TextHeight = 13
  object lblStats: TLabel
    Left = 88
    Top = 120
    Width = 34
    Height = 13
    Caption = 'lblStats'
  end
  object btnMovie: TSpeedButton
    Tag = 1
    Left = 88
    Top = 40
    Width = 75
    Height = 32
    Caption = 'Movies'
    OnClick = btnFormClick
  end
  object btnPeople: TSpeedButton
    Tag = 2
    Left = 88
    Top = 80
    Width = 75
    Height = 32
    Caption = 'People'
    OnClick = btnFormClick
  end
  object Memo1: TMemo
    Left = 184
    Top = 8
    Width = 177
    Height = 153
    Anchors = [akLeft, akTop, akRight, akBottom]
    Lines.Strings = (
      'Status')
    ScrollBars = ssVertical
    TabOrder = 0
  end
  object btnSave: TButton
    Left = 8
    Top = 40
    Width = 75
    Height = 25
    Caption = 'Save DB'
    TabOrder = 1
    OnClick = btnSaveClick
  end
  object btnRead: TButton
    Left = 8
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Read DB'
    TabOrder = 2
    OnClick = btnReadClick
  end
  object btnOutStat: TButton
    Left = 8
    Top = 72
    Width = 75
    Height = 25
    Caption = 'Output Stats'
    TabOrder = 3
    OnClick = btnOutStatClick
  end
  object btnClear: TButton
    Left = 120
    Top = 136
    Width = 50
    Height = 21
    Caption = 'Clear>>>'
    TabOrder = 4
    OnClick = btnClearClick
  end
  object btnReport: TButton
    Tag = 4
    Left = 8
    Top = 104
    Width = 75
    Height = 25
    Caption = 'Report'
    TabOrder = 5
    OnClick = btnFormClick
  end
  object btnGraph: TButton
    Tag = 3
    Left = 8
    Top = 136
    Width = 75
    Height = 25
    Caption = 'Graphs'
    TabOrder = 6
    OnClick = btnFormClick
  end
  object btnQuery: TButton
    Left = 88
    Top = 8
    Width = 75
    Height = 25
    Caption = 'Query'
    TabOrder = 7
    OnClick = btnQueryClick
  end
  object SaveDialog1: TSaveDialog
    Left = 256
    Top = 56
  end
  object OpenDialog1: TOpenDialog
    Left = 256
    Top = 128
  end
  object MainMenu1: TMainMenu
    Left = 256
    Top = 24
    object File1: TMenuItem
      Caption = 'File'
      object mnuFileRead: TMenuItem
        Caption = 'Read DB'
        OnClick = btnReadClick
      end
      object mnuFileSave: TMenuItem
        Caption = 'Save DB'
        OnClick = btnSaveClick
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object mnuExit: TMenuItem
        Caption = 'Exit'
        OnClick = mnuExitClick
      end
    end
    object Actions1: TMenuItem
      Caption = 'Actions'
      object mnuActBuild: TMenuItem
        Tag = 10
        Caption = 'Build Old'
        OnClick = btnFormClick
      end
      object mnuActDump: TMenuItem
        Caption = 'Dump'
        OnClick = btnDumpClick
      end
      object mnuActSave: TMenuItem
        Caption = 'Save'
        OnClick = mnuActSaveClick
      end
    end
  end
  object PrintDialog1: TPrintDialog
    Left = 256
    Top = 88
  end
end
