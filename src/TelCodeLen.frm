VERSION 5.00
Begin VB.Form Form1 
   Caption         =   "Form1"
   ClientHeight    =   3090
   ClientLeft      =   60
   ClientTop       =   450
   ClientWidth     =   4680
   LinkTopic       =   "Form1"
   ScaleHeight     =   3090
   ScaleWidth      =   4680
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton Command1 
      Caption         =   "Command1"
      Height          =   375
      Left            =   600
      TabIndex        =   0
      Top             =   600
      Width           =   1095
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Private Sub Command1_Click()
Dim Fn As String, CodeLen As Long, TotLen As Long
Dim hs As String, Pad As String
Open App.Path + "\lijst.txt" For Append As #1
hs = "T:\PP\JJsource\*.cpp"
'Pad = Left(hs, Len(hs) - 5)
Pad = hs
While Right(Pad, 1) <> "\"
 Pad = Left(Pad, Len(Pad) - 1)
Wend
Fn = Dir(hs)
TotLen = 0
Print #1,
Print #1, "-----------------------------------"
Print #1, "Lijst van bestanden in " + hs
Print #1, "-----------------------------------"
While Fn <> ""
 Open Pad + Fn For Input As #2
  CodeLen = 0
  While Not EOF(2)
   Line Input #2, hs
   CodeLen = CodeLen + 1
  Wend
 Close #2
 hs = Str(CodeLen)
 hs = Space(7 - Len(hs)) + hs
 Print #1, hs + " : " + Fn
 TotLen = TotLen + CodeLen
 Fn = Dir
Wend
Print #1, "-----------------------------------"
hs = Str(TotLen)
hs = Space(7 - Len(hs)) + hs
Print #1, hs + " : Totaal"
Print #1, "-----------------------------------"
Close #1
End Sub
