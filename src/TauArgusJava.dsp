# Microsoft Developer Studio Project File - Name="TauArgusJava" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=TauArgusJava - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "TauArgusJava.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "TauArgusJava.mak" CFG="TauArgusJava - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TauArgusJava - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "TauArgusJava - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "TauArgusJava - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "dist\Release\MSVC6"
# PROP Intermediate_Dir "build\Release\MSVC6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TAUARGUSJAVA_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "$(JAVA_HOME)\include" /I "$(JAVA_HOME)\include\win32" /D "NDEBUG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "TauArgusJava - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "TauArgusJava___Win32_Debug"
# PROP BASE Intermediate_Dir "TauArgusJava___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "dist\Debug\MSVC6"
# PROP Intermediate_Dir "build\Debug\MSVC6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "TAUARGUSJAVA_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "$(JAVA_HOME)\include" /I "$(JAVA_HOME)\include\win32" /D "_DEBUG" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "TauArgusJava - Win32 Release"
# Name "TauArgusJava - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AMPL.CPP
# End Source File
# Begin Source File

SOURCE=.\Code.cpp
# End Source File
# Begin Source File

SOURCE=.\DataCell.cpp
# End Source File
# Begin Source File

SOURCE=.\General.cpp
# End Source File
# Begin Source File

SOURCE=.\Ghmiter.cpp
# End Source File
# Begin Source File

SOURCE=.\Hitas.cpp
# End Source File
# Begin Source File

SOURCE=.\JavaProgressListener.cpp
# End Source File
# Begin Source File

SOURCE=.\JJFormat.cpp
# End Source File
# Begin Source File

SOURCE=.\Properties.cpp
# End Source File
# Begin Source File

SOURCE=.\SubCodeList.cpp
# End Source File
# Begin Source File

SOURCE=.\Table.cpp
# End Source File
# Begin Source File

SOURCE=.\TauArgus.cpp
# End Source File
# Begin Source File

SOURCE=.\tauargus_extern_TauArgusX.cpp
# End Source File
# Begin Source File

SOURCE=.\TauArgusJava_wrap.cpp
# ADD CPP /GR
# End Source File
# Begin Source File

SOURCE=.\Variable.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AMPL.H
# End Source File
# Begin Source File

SOURCE=.\Code.h
# End Source File
# Begin Source File

SOURCE=.\DataCell.h
# End Source File
# Begin Source File

SOURCE=.\defines.h
# End Source File
# Begin Source File

SOURCE=.\General.h
# End Source File
# Begin Source File

SOURCE=.\Ghmiter.h
# End Source File
# Begin Source File

SOURCE=.\Globals.h
# End Source File
# Begin Source File

SOURCE=.\Hitas.h
# End Source File
# Begin Source File

SOURCE=.\IProgressListener.h
# End Source File
# Begin Source File

SOURCE=.\JavaProgressListener.h
# End Source File
# Begin Source File

SOURCE=.\JJFormat.h
# End Source File
# Begin Source File

SOURCE=.\Properties.h
# End Source File
# Begin Source File

SOURCE=.\SubCodeList.h
# End Source File
# Begin Source File

SOURCE=.\Table.h
# End Source File
# Begin Source File

SOURCE=.\TauArgus.h
# End Source File
# Begin Source File

SOURCE=.\tauargus_extern_TauArgusX.h
# End Source File
# Begin Source File

SOURCE=.\Variable.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\TauArgusJava.swg

!IF  "$(CFG)" == "TauArgusJava - Win32 Release"

# Begin Custom Build
OutDir=.\dist\Release\MSVC6
ProjDir=.
InputPath=.\TauArgusJava.swg
InputName=TauArgusJava

"$(ProjDir)\$(InputName)_wrap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SWIG_HOME)\swig -c++ -java -package tauargus.extern -outdir $(OutDir) -o $(ProjDir)\$(InputName)_wrap.cpp $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "TauArgusJava - Win32 Debug"

# Begin Custom Build
OutDir=.\dist\Debug\MSVC6
ProjDir=.
InputPath=.\TauArgusJava.swg
InputName=TauArgusJava

"$(ProjDir)\$(InputName)_wrap.cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(SWIG_HOME)\swig -c++ -java -package tauargus.extern -outdir $(OutDir) -o $(ProjDir)\$(InputName)_wrap.cpp $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# End Target
# End Project
