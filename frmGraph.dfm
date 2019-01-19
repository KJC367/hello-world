object GraphForm: TGraphForm
  Left = 207
  Top = 131
  Width = 882
  Height = 410
  Caption = 'Graphes'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnResize = FormResize
  DesignSize = (
    866
    371)
  PixelsPerInch = 96
  TextHeight = 13
  object graph: TImage
    Left = 312
    Top = 136
    Width = 3217
    Height = 1200
    Anchors = [akLeft, akTop, akRight, akBottom]
    OnMouseDown = graphMouseDown
  end
  object chkStats: TCheckBox
    Left = 328
    Top = 24
    Width = 97
    Height = 17
    Caption = 'Min/Max'
    TabOrder = 0
    OnClick = chkStatsClick
  end
  object cbGraph: TComboBox
    Left = 424
    Top = 40
    Width = 145
    Height = 21
    ItemHeight = 13
    TabOrder = 1
    Text = 'cbGraph'
    OnSelect = cbGraphSelect
  end
  object chkLog: TCheckBox
    Left = 328
    Top = 8
    Width = 97
    Height = 17
    Caption = 'Logrithmic'
    TabOrder = 2
    OnClick = chkStatsClick
  end
  object ColorDialog1: TColorDialog
    Ctl3D = True
    Options = [cdFullOpen, cdAnyColor]
    Left = 248
    Top = 16
  end
end
