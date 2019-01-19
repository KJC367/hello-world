object MoviesForm: TMoviesForm
  Left = 207
  Top = 130
  Width = 882
  Height = 411
  Caption = 'Movie'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object lblYear: TLabel
    Left = 152
    Top = 88
    Width = 32
    Height = 13
    Caption = 'lblYear'
  end
  object lblRating: TLabel
    Left = 240
    Top = 80
    Width = 41
    Height = 13
    Caption = 'lblRating'
  end
  object edYear: TEdit
    Left = 160
    Top = 120
    Width = 121
    Height = 21
    TabOrder = 0
    Text = 'edYear'
  end
  object cbRating: TComboBox
    Left = 352
    Top = 128
    Width = 75
    Height = 21
    ItemHeight = 13
    TabOrder = 1
    Text = 'cbRating'
  end
  object grpGenre: TGroupBox
    Left = 112
    Top = 176
    Width = 329
    Height = 33
    Caption = 'Genre'
    TabOrder = 2
    OnClick = lblGenreClick
    object lblGenre: TLabel
      Left = 6
      Top = 12
      Width = 35
      Height = 26
      AutoSize = False
      Caption = 'Genre: '
      WordWrap = True
      OnClick = lblGenreClick
    end
  end
end
