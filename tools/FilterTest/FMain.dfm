object Form_Main: TForm_Main
  Left = 266
  Top = 109
  Width = 400
  Height = 178
  Caption = 'Filter Experimentation Lounge'
  Color = clBtnFace
  Constraints.MaxHeight = 178
  Constraints.MinHeight = 178
  Constraints.MinWidth = 353
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Arial'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  Scaled = False
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  PixelsPerInch = 96
  TextHeight = 14
  object c_Panel_Main: TPanel
    Left = 0
    Top = 0
    Width = 392
    Height = 151
    Align = alClient
    BevelOuter = bvNone
    TabOrder = 0
    object c_Label_FileName: TLabel
      Left = 0
      Top = 12
      Width = 57
      Height = 13
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'File &Name'
      FocusControl = c_ComboBox_Filter
    end
    object Label1: TLabel
      Left = 0
      Top = 44
      Width = 57
      Height = 13
      Alignment = taRightJustify
      AutoSize = False
      Caption = '&Filter'
      FocusControl = c_ComboBox_Filter
    end
    object Label2: TLabel
      Left = 0
      Top = 76
      Width = 57
      Height = 13
      Alignment = taRightJustify
      AutoSize = False
      Caption = '&Property'
      FocusControl = c_ComboBox_Property
    end
    object Label3: TLabel
      Left = 281
      Top = 76
      Width = 33
      Height = 13
      Alignment = taRightJustify
      Anchors = [akTop, akRight]
      AutoSize = False
      Caption = '&Value'
    end
    object Label7: TLabel
      Left = 0
      Top = 110
      Width = 57
      Height = 13
      Alignment = taRightJustify
      AutoSize = False
      Caption = 'P&an'
      FocusControl = c_ComboBox_Property
    end
    object Label4: TLabel
      Left = 306
      Top = 96
      Width = 10
      Height = 7
      Anchors = [akTop, akRight]
      Caption = '0.1'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -7
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
    end
    object Label5: TLabel
      Left = 330
      Top = 96
      Width = 4
      Height = 7
      Anchors = [akTop, akRight]
      Caption = '1'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -7
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
    end
    object Label6: TLabel
      Left = 347
      Top = 96
      Width = 12
      Height = 7
      Anchors = [akTop, akRight]
      Caption = '100'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -7
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
    end
    object Label8: TLabel
      Left = 366
      Top = 96
      Width = 16
      Height = 7
      Anchors = [akTop, akRight]
      Caption = '1000'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -7
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
    end
    object Label9: TLabel
      Left = 283
      Top = 96
      Width = 14
      Height = 7
      Anchors = [akTop, akRight]
      Caption = '0.01'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -7
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
    end
    object c_Edit_FileName: TEdit
      Left = 64
      Top = 8
      Width = 298
      Height = 22
      Anchors = [akLeft, akTop, akRight]
      TabOrder = 0
      OnChange = c_Edit_FileNameChange
      OnKeyPress = c_Edit_FileNameKeyPress
    end
    object c_Button_OpenFile: TBitBtn
      Left = 369
      Top = 8
      Width = 17
      Height = 21
      Anchors = [akTop, akRight]
      Caption = '&...'
      TabOrder = 1
      OnClick = c_Button_OpenFileClick
    end
    object c_ComboBox_Filter: TComboBox
      Left = 64
      Top = 40
      Width = 322
      Height = 22
      Style = csDropDownList
      Anchors = [akLeft, akTop, akRight]
      ItemHeight = 14
      TabOrder = 2
      OnChange = c_ComboBox_FilterChange
    end
    object c_ComboBox_Property: TComboBox
      Left = 64
      Top = 72
      Width = 214
      Height = 22
      Style = csDropDownList
      Anchors = [akLeft, akTop, akRight]
      Enabled = False
      ItemHeight = 14
      TabOrder = 3
      OnChange = c_ComboBox_PropertyChange
    end
    object c_Edit_Value: TEdit
      Left = 317
      Top = 72
      Width = 69
      Height = 22
      Anchors = [akTop, akRight]
      TabOrder = 4
      OnKeyPress = c_Edit_ValueKeyPress
    end
    object c_Button_Play: TBitBtn
      Left = 209
      Top = 104
      Width = 25
      Height = 25
      Anchors = [akTop, akRight]
      Enabled = False
      TabOrder = 6
      OnClick = c_Button_PlayClick
      Glyph.Data = {
        FE000000424DFE000000000000003600000028000000060000000A0000000100
        180000000000C8000000C40E0000C40E00000000000000000000C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C00000C0C0C0000000C0C0C0C0C0C0C0C0C0C0C0C0
        0000C0C0C0000000000000C0C0C0C0C0C0C0C0C00000C0C0C000000000000000
        0000C0C0C0C0C0C00000C0C0C0000000000000000000000000C0C0C00000C0C0
        C0000000000000000000000000C0C0C00000C0C0C0000000000000000000C0C0
        C0C0C0C00000C0C0C0000000000000C0C0C0C0C0C0C0C0C00000C0C0C0000000
        C0C0C0C0C0C0C0C0C0C0C0C00000C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0
        0000}
    end
    object c_Button_Stop: TBitBtn
      Left = 241
      Top = 104
      Width = 25
      Height = 25
      Anchors = [akTop, akRight]
      Enabled = False
      TabOrder = 7
      OnClick = c_Button_StopClick
      Glyph.Data = {
        0E010000424D0E01000000000000360000002800000008000000090000000100
        180000000000D8000000C40E0000C40E00000000000000000000C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C00000000000000000000000
        00000000000000C0C0C0C0C0C0000000000000000000000000000000000000C0
        C0C0C0C0C0000000000000000000000000000000000000C0C0C0C0C0C0000000
        000000000000000000000000000000C0C0C0C0C0C00000000000000000000000
        00000000000000C0C0C0C0C0C0000000000000000000000000000000000000C0
        C0C0C0C0C0000000000000000000000000000000000000C0C0C0C0C0C0C0C0C0
        C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0C0}
    end
    object c_Button_AdjSmall_Up: TButton
      Left = 301
      Top = 104
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '+'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 10
      TabStop = False
      OnClick = c_Button_AdjSmall_UpClick
    end
    object c_Button_AdjMed_Up: TButton
      Left = 322
      Top = 104
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '+'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 11
      TabStop = False
      OnClick = c_Button_AdjMed_UpClick
    end
    object c_Button_AdjLarge_Up: TButton
      Left = 343
      Top = 104
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '+'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 12
      TabStop = False
      OnClick = c_Button_AdjLarge_UpClick
    end
    object c_Button_AdjSmall_Down: TButton
      Left = 301
      Top = 117
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '-'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 13
      TabStop = False
      OnClick = c_Button_AdjSmall_DownClick
    end
    object c_Button_AdjMed_Down: TButton
      Left = 322
      Top = 117
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '-'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 14
      TabStop = False
      OnClick = c_Button_AdjMed_DownClick
    end
    object c_Button_AdjLarge_Down: TButton
      Left = 343
      Top = 117
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '-'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 15
      TabStop = False
      OnClick = c_Button_AdjLarge_DownClick
    end
    object c_TrackBar_Pan: TTrackBar
      Left = 64
      Top = 108
      Width = 89
      Height = 17
      Max = 127
      Orientation = trHorizontal
      PageSize = 16
      Frequency = 16
      Position = 64
      SelEnd = 0
      SelStart = 0
      TabOrder = 5
      ThumbLength = 7
      TickMarks = tmBottomRight
      TickStyle = tsAuto
      OnChange = c_TrackBar_PanChange
    end
    object c_Button_AdjExLarge_Up: TButton
      Left = 364
      Top = 104
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '+'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 16
      TabStop = False
      OnClick = c_Button_AdjExLarge_UpClick
    end
    object c_Button_AdjExLarge_Down: TButton
      Left = 364
      Top = 117
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '-'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 17
      TabStop = False
      OnClick = c_Button_AdjExLarge_DownClick
    end
    object c_Panel_Status: TPanel
      Left = 0
      Top = 134
      Width = 392
      Height = 17
      Align = alBottom
      BevelOuter = bvLowered
      Caption = 'Waiting...'
      TabOrder = 18
    end
    object c_Button_AdjExSmall_Up: TButton
      Left = 280
      Top = 104
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '+'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 8
      TabStop = False
      OnClick = c_Button_AdjExSmall_UpClick
    end
    object c_Button_AdjExSmall_Down: TButton
      Left = 280
      Top = 117
      Width = 21
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '-'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Arial'
      Font.Style = []
      ParentFont = False
      TabOrder = 9
      TabStop = False
      OnClick = c_Button_AdjExSmall_DownClick
    end
  end
  object c_OpenDialog: TOpenDialog
    DefaultExt = 'wav'
    Filter = 'Wave file (*.wav)|*.wav|All Files (*.*)|*.*'
    Options = [ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Title = 'Please select an audio file'
    Left = 24
  end
end
