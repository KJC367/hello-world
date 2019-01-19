object BaseForm: TBaseForm
  Left = 430
  Top = 202
  Width = 1087
  Height = 750
  Caption = 'Query'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnDestroy = FormDestroy
  OnMouseWheel = FormMouseWheel
  OnResize = FormResize
  OnShow = FormShow
  DesignSize = (
    1071
    711)
  PixelsPerInch = 96
  TextHeight = 13
  object lblMain: TLabel
    Left = 40
    Top = 16
    Width = 28
    Height = 13
    Caption = 'Name'
    OnClick = SortLabel
  end
  object lblAlias: TLabel
    Left = 0
    Top = 66
    Width = 22
    Height = 13
    Caption = 'Alias'
    Layout = tlBottom
  end
  object lblAction: TLabel
    Left = 88
    Top = 456
    Width = 3
    Height = 13
  end
  object grid: TStringGrid
    Left = 200
    Top = 104
    Width = 457
    Height = 257
    Anchors = [akLeft, akTop, akBottom]
    Options = [goFixedVertLine, goFixedHorzLine, goVertLine, goHorzLine, goRangeSelect, goColSizing]
    TabOrder = 0
    OnDrawCell = GridDrawCell
    OnMouseDown = GridMouseDown
    OnSelectCell = GridSelectCell
    ColWidths = (
      64
      84
      64
      97
      64)
  end
  object btnOK: TButton
    Left = 8
    Top = 200
    Width = 75
    Height = 25
    Caption = 'OK'
    ModalResult = 1
    TabOrder = 1
  end
  object btnCancel: TButton
    Left = 88
    Top = 200
    Width = 75
    Height = 25
    Caption = 'Cancel'
    ModalResult = 2
    TabOrder = 2
  end
  object btnNext: TButton
    Left = 88
    Top = 232
    Width = 75
    Height = 25
    Caption = 'Next >>>'
    TabOrder = 3
    OnClick = btnNextClick
  end
  object btnPrev: TButton
    Left = 8
    Top = 232
    Width = 75
    Height = 25
    Caption = '<<< Previous'
    TabOrder = 4
    OnClick = btnPrevClick
  end
  object cbMain: TComboBox
    Left = 40
    Top = 32
    Width = 745
    Height = 21
    AutoComplete = False
    DropDownCount = 20
    ItemHeight = 13
    TabOrder = 5
    Text = '<enter name>'
    OnChange = cbMainChange
    OnEnter = cbMainChange
    OnExit = cbMainExit
    OnKeyPress = cbMainKeyPress
    OnSelect = cbMainExit
  end
  object btnDelete: TButton
    Left = 8
    Top = 264
    Width = 75
    Height = 25
    Caption = 'Delete'
    Enabled = False
    TabOrder = 6
  end
  object memAlias: TMemo
    Left = 0
    Top = 84
    Width = 185
    Height = 93
    Lines.Strings = (
      'Memo1')
    TabOrder = 7
    OnExit = memAliasExit
  end
  object btnRename: TButton
    Left = 88
    Top = 264
    Width = 75
    Height = 25
    Caption = 'Rename'
    TabOrder = 8
  end
  object cbEdit: TComboBox
    Left = 704
    Top = 112
    Width = 145
    Height = 21
    ItemHeight = 13
    TabOrder = 9
    Text = 'cbEdit'
    Visible = False
    OnExit = cbAttribExit
    OnSelect = cbEditSelect
  end
  object cbGridFmt: TComboBox
    Left = 336
    Top = 72
    Width = 145
    Height = 21
    ItemHeight = 13
    TabOrder = 10
    Text = 'cbGridFmt'
    OnSelect = cbGridFmtSelect
  end
  object FontDialogQF: TFontDialog
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    MinFontSize = 0
    MaxFontSize = 0
    Left = 664
    Top = 208
  end
  object ColorDialogQF: TColorDialog
    Ctl3D = True
    Options = [cdFullOpen]
    Left = 672
    Top = 256
  end
end
