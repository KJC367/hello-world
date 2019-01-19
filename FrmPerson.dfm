object PersonForm: TPersonForm
  Left = 207
  Top = 131
  Width = 882
  Height = 410
  Caption = 'Person'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object graph: TImage
    Left = 312
    Top = 136
    Width = 500
    Height = 200
    OnMouseDown = graphMouseDown
  end
  object lblScores: TLabel
    Left = 136
    Top = 128
    Width = 43
    Height = 13
    Caption = 'lblScores'
  end
  object ColorDialog1: TColorDialog
    Ctl3D = True
    Options = [cdFullOpen, cdAnyColor]
    Left = 248
    Top = 16
  end
end
