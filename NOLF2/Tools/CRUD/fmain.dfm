object Form_CRUD: TForm_CRUD
  Left = 194
  Top = 102
  Width = 407
  Height = 388
  Caption = 'CRUD'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object c_Splitter_Main: TSplitter
    Left = 0
    Top = 235
    Width = 399
    Height = 3
    Cursor = crVSplit
    Align = alTop
    Beveled = True
  end
  object c_Panel_Main: TPanel
    Left = 0
    Top = 0
    Width = 399
    Height = 235
    Align = alTop
    BevelOuter = bvNone
    Constraints.MinHeight = 160
    Constraints.MinWidth = 200
    TabOrder = 0
    object c_Panel_Disabler: TPanel
      Left = 0
      Top = 0
      Width = 399
      Height = 235
      Align = alClient
      BevelOuter = bvNone
      TabOrder = 1
      object c_Label_Source: TLabel
        Left = 0
        Top = 8
        Width = 105
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '&Source Directory'
        FocusControl = c_Edit_SourceDir
      end
      object c_Label_Dest: TLabel
        Left = 0
        Top = 32
        Width = 105
        Height = 13
        Alignment = taRightJustify
        AutoSize = False
        Caption = '&Destination Directory'
        FocusControl = c_Edit_DestDir
      end
      object c_Label_SFiles: TLabel
        Left = 47
        Top = 56
        Width = 58
        Height = 13
        Alignment = taRightJustify
        Caption = 'Source &Files'
        FocusControl = c_LB_SourceFiles
      end
      object c_Edit_SourceDir: TEdit
        Left = 112
        Top = 4
        Width = 258
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 0
        OnExit = Edit_DirExit
      end
      object c_Edit_DestDir: TEdit
        Left = 112
        Top = 28
        Width = 258
        Height = 21
        Anchors = [akLeft, akTop, akRight]
        TabOrder = 2
        OnExit = Edit_DirExit
      end
      object c_Button_SourceBrowse: TButton
        Left = 374
        Top = 4
        Width = 23
        Height = 22
        Hint = 'Browse'
        Anchors = [akTop, akRight]
        Caption = '...'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 1
        OnClick = c_Button_SourceBrowseClick
      end
      object c_Button_DestBrowse: TButton
        Left = 374
        Top = 28
        Width = 23
        Height = 22
        Hint = 'Browse'
        Anchors = [akTop, akRight]
        Caption = '...'
        ParentShowHint = False
        ShowHint = True
        TabOrder = 3
        OnClick = c_Button_DestBrowseClick
      end
      object c_Button_SourceAdd: TButton
        Left = 374
        Top = 185
        Width = 23
        Height = 22
        Hint = 'Add File To Source List'
        Anchors = [akRight, akBottom]
        Caption = '+'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -16
        Font.Name = 'Arial'
        Font.Style = [fsBold]
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 7
        OnClick = c_Button_SourceAddClick
      end
      object c_Button_SourceRemove: TButton
        Left = 374
        Top = 209
        Width = 23
        Height = 22
        Hint = 'Remove File From Source List'
        Anchors = [akRight, akBottom]
        Caption = '-'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -16
        Font.Name = 'Arial'
        Font.Style = [fsBold]
        ParentFont = False
        ParentShowHint = False
        ShowHint = True
        TabOrder = 8
        OnClick = c_Button_SourceRemoveClick
      end
      object c_Button_LoadSource: TBitBtn
        Left = 374
        Top = 53
        Width = 23
        Height = 22
        Hint = 'Load Source List'
        Anchors = [akTop, akRight]
        ParentShowHint = False
        ShowHint = True
        TabOrder = 5
        OnClick = c_Button_LoadSourceClick
        Glyph.Data = {
          7E030000424D7E030000000000003600000028000000120000000F0000000100
          18000000000048030000C40E0000C40E00000000000000000000FF00FFFF00FF
          FF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00
          FFFF00FFFF00FFFF00FFFF00FFFF00FF0000FF00FF0000000000000000000000
          00000000000000000000000000000000000000000000FF00FFFF00FFFF00FFFF
          00FFFF00FFFF00FF0000FF00FF00000000000000848400848400848400848400
          8484008484008484008484008484000000FF00FFFF00FFFF00FFFF00FFFF00FF
          0000FF00FF00000000FFFF000000008484008484008484008484008484008484
          008484008484008484000000FF00FFFF00FFFF00FFFF00FF0000FF00FF000000
          FFFFFF00FFFF0000000084840084840084840084840084840084840084840084
          84008484000000FF00FFFF00FFFF00FF0000FF00FF00000000FFFFFFFFFF00FF
          FF00000000848400848400848400848400848400848400848400848400848400
          0000FF00FFFF00FF0000FF00FF000000FFFFFF00FFFFFFFFFF00FFFF00000000
          0000000000000000000000000000000000000000000000000000000000FF00FF
          0000FF00FF00000000FFFFFFFFFF00FFFFFFFFFF00FFFFFFFFFF00FFFFFFFFFF
          00FFFF000000FF00FFFF00FFFF00FFFF00FFFF00FFFF00FF0000FF00FF000000
          FFFFFF00FFFFFFFFFF00FFFFFFFFFF00FFFFFFFFFF00FFFFFFFFFF000000FF00
          FFFF00FFFF00FFFF00FFFF00FFFF00FF0000FF00FF00000000FFFFFFFFFF00FF
          FF000000000000000000000000000000000000000000FF00FFFF00FFFF00FFFF
          00FFFF00FFFF00FF0000FF00FFFF00FF000000000000000000FF00FFFF00FFFF
          00FFFF00FFFF00FFFF00FFFF00FFFF00FF000000000000000000FF00FFFF00FF
          0000FF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FF
          FF00FFFF00FFFF00FFFF00FF000000000000FF00FFFF00FF0000FF00FFFF00FF
          FF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FF000000FF00FFFF00FFFF00
          FF000000FF00FF000000FF00FFFF00FF0000FF00FFFF00FFFF00FFFF00FFFF00
          FFFF00FFFF00FFFF00FFFF00FFFF00FF000000000000000000FF00FFFF00FFFF
          00FFFF00FFFF00FF0000FF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF
          00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FF
          0000}
      end
      object c_Button_SaveSource: TBitBtn
        Left = 374
        Top = 78
        Width = 23
        Height = 22
        Hint = 'Save Source List'
        Anchors = [akTop, akRight]
        ParentShowHint = False
        ShowHint = True
        TabOrder = 6
        OnClick = c_Button_SaveSourceClick
        Glyph.Data = {
          36030000424D3603000000000000360000002800000010000000100000000100
          18000000000000030000C40E0000C40E00000000000000000000FF00FFFF00FF
          FF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00
          FFFF00FFFF00FFFF00FFFF00FFFF00FF00000000000000000000000000000000
          0000000000000000000000000000000000000000000000FF00FFFF00FF000000
          008484008484000000000000000000000000000000000000C6C6C6C6C6C60000
          00008484000000FF00FFFF00FF00000000848400848400000000000000000000
          0000000000000000C6C6C6C6C6C6000000008484000000FF00FFFF00FF000000
          008484008484000000000000000000000000000000000000C6C6C6C6C6C60000
          00008484000000FF00FFFF00FF00000000848400848400000000000000000000
          0000000000000000000000000000000000008484000000FF00FFFF00FF000000
          0084840084840084840084840084840084840084840084840084840084840084
          84008484000000FF00FFFF00FF00000000848400848400000000000000000000
          0000000000000000000000000000008484008484000000FF00FFFF00FF000000
          008484000000C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C60000
          00008484000000FF00FFFF00FF000000008484000000C6C6C6C6C6C6C6C6C6C6
          C6C6C6C6C6C6C6C6C6C6C6C6C6C6000000008484000000FF00FFFF00FF000000
          008484000000C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C60000
          00008484000000FF00FFFF00FF000000008484000000C6C6C6C6C6C6C6C6C6C6
          C6C6C6C6C6C6C6C6C6C6C6C6C6C6000000008484000000FF00FFFF00FF000000
          008484000000C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C6C60000
          00000000000000FF00FFFF00FF000000008484000000C6C6C6C6C6C6C6C6C6C6
          C6C6C6C6C6C6C6C6C6C6C6C6C6C6000000C6C6C6000000FF00FFFF00FF000000
          0000000000000000000000000000000000000000000000000000000000000000
          00000000000000FF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF
          00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FFFF00FF}
      end
      object c_LB_SourceFiles: TListView
        Left = 112
        Top = 52
        Width = 258
        Height = 179
        Anchors = [akLeft, akTop, akRight, akBottom]
        Checkboxes = True
        Columns = <
          item
            Caption = 'File'
            Width = -1
            WidthType = (
              -1)
          end>
        HideSelection = False
        MultiSelect = True
        RowSelect = True
        ShowColumnHeaders = False
        SortType = stText
        TabOrder = 4
        ViewStyle = vsReport
        OnChange = c_LB_SourceFilesChange
        OnChanging = c_LB_SourceFilesChanging
        OnKeyPress = c_LB_SourceFilesKeyPress
      end
      object c_CheckBox_Recursive: TCheckBox
        Left = 24
        Top = 88
        Width = 81
        Height = 17
        Alignment = taLeftJustify
        Caption = '&Recursive'
        TabOrder = 9
      end
    end
    object c_Button_Go: TButton
      Left = 19
      Top = 206
      Width = 75
      Height = 25
      Anchors = [akLeft, akBottom]
      Caption = '&Go'
      Default = True
      TabOrder = 0
      OnClick = c_Button_GoClick
    end
  end
  object c_Panel_Results: TPanel
    Left = 0
    Top = 238
    Width = 399
    Height = 123
    Align = alClient
    BevelOuter = bvNone
    BorderWidth = 1
    Constraints.MinHeight = 50
    TabOrder = 1
    object c_Memo_Results: TRichEdit
      Left = 1
      Top = 1
      Width = 397
      Height = 121
      Align = alClient
      PlainText = True
      ReadOnly = True
      ScrollBars = ssBoth
      TabOrder = 0
      WordWrap = False
    end
  end
  object c_OpenDialog_Dir: TOpenDialog
    Options = [ofNoChangeDir, ofPathMustExist, ofEnableSizing]
    Left = 8
    Top = 88
  end
  object c_OpenDialog_File: TOpenDialog
    Filter = 'All Files (*.*)|*.*|File Listings (*.lst)|*.lst'
    Options = [ofNoChangeDir, ofAllowMultiSelect, ofFileMustExist, ofEnableSizing]
    Left = 8
    Top = 120
  end
  object c_SaveDialog: TSaveDialog
    Filter = 'File Listings (*.lst)|*.lst|All Files (*.*)|*.*'
    Options = [ofNoChangeDir, ofPathMustExist, ofShareAware, ofEnableSizing]
    Left = 8
    Top = 152
  end
end
