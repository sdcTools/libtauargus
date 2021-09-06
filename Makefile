# Makefile for TauArgusJava
####### Compiler, tools and options
# Environment

MKDIR           = mkdir
RM              = rm -f
CP              = cp -p

32BIT           = true
#32BIT           = false

SWIGDIR         = D:/Peter-Paul/Documents/Thuiswerk/Programmatuur/swigwin-4.0.1

ifeq ($(32BIT), false)  # 64 bit assumed
    BITS        = -m64 -D_LP64
    ARCH        = x86_64
    CND_PLATFORM= MinGW-Windows64
    JAVADIR     = ../../../Java/zulu8.52.0.23-ca-jdk8.0.282-win_x64
    GNUDIR      = C:/Progra~1/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/mingw64/bin
else                    # 32 bit assumed
    BITS        = -m32
    ARCH        = x86
    CND_PLATFORM= MinGW-Windows
    JAVADIR     = ../../../Java/zulu8.52.0.23-ca-jdk8.0.282-win_i686
    GNUDIR      = C:/Progra~2/mingw-w64/i686-8.1.0-win32-sjlj-rt_v6-rev0/mingw32/bin
endif

JAVAINC         = -I$(JAVADIR)/include -I$(JAVADIR)/include/win32
CXX             = $(GNUDIR)/g++
LINK            = $(GNUDIR)/g++
WINDRES         = $(GNUDIR)/windres
SWIG            = $(SWIGDIR)/swig.exe

# Macros
CND_DLIB_EXT    = dll
CND_CONF        = Debug
CND_DISTDIR     = dist
CND_BUILDDIR    = build

LIBNAME         = TauArgusJava
JAVAPACKAGE     = tauargus.extern.dataengine

# ======================================================================
# Set directories for input and output files
# ======================================================================

BUILDFILEDIR    = $(CND_BUILDDIR)/$(CND_CONF)/$(CND_PLATFORM)
DISTFILEDIR     = $(CND_DISTDIR)/$(CND_CONF)/$(CND_PLATFORM)

SRCDIR          = .
OBJDIR          = $(BUILDFILEDIR)
LIBDIR          = $(DISTFILEDIR)
OUTDIR          = $(DISTFILEDIR)

# ====================================================
# Non configurable items
# ====================================================
SFLAGS          = -c++ -I./src -java -package $(JAVAPACKAGE) -outdir $(OUTDIR) 
CFLAGS          = -Wall $(BITS) -std=c++11 -fPIC -Wno-unused-function -fno-strict-aliasing
CFLAGS          += -O2 -g#-DNDEBUG -O2
CFLAGS          += $(JAVAINC)
LDFLAGS         = $(CFLAGS) -Wl,--subsystem,windows -Wl,--kill-at -shared -static-libgcc -static-libstdc++


# Exclude source files needed for a COM dll for Visual Basic 6.0
NOSOURCES        = $(SRCDIR)/GhmiterANCO.cpp $(SRCDIR)/TauArgCtrl.cpp $(SRCDIR)/StdAfx.cpp $(SRCDIR)/NewTauArgus.cpp
SWIGSOURCES      = $(SRCDIR)/TauArgusJava.swg
GENERATED_SOURCES= $(patsubst $(SRCDIR)/%.swg,$(SRCDIR)/%_wrap.cpp,$(SWIGSOURCES))
SOURCES          = $(filter-out $(NOSOURCES),$(wildcard $(SRCDIR)/*.cpp))

OBJECTS          = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES) $(GENERATED_SOURCES)) $(OBJDIR)/Versioninfo.o

LIBFILENAME      = $(LIBNAME).dll

TARGET           = $(LIBDIR)/$(LIBFILENAME)

.PHONY: all clean

.SECONDARY: $(SRCDIR)/TauArgusJava_wrap.cpp

all : $(OBJDIR) $(SPECIALSOURCE) $(LIBDIR) $(TARGET)

clean :
	$(RM) -r $(OBJDIR) $(LIBDIR)
	$(RM) $(SRCDIR)/*_wrap.*

$(CND_BUILDDIR) $(CND_BUILDDIR)/$(CND_CONF) $(OBJDIR) $(CND_DISTDIR) $(CND_DISTDIR)/$(CND_CONF) $(LIBDIR) :
	$(MKDIR) -p $@

$(TARGET) : $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $@ $^
	$(CP) $(TARGET) $(SRCDIR)/../tauargus/$(LIBFILENAME)
	$(CP) $(LIBDIR)/*.java $(SRCDIR)/../tauargus/src/tauargus/extern/dataengine

# pull in dependency info for *existing* .o files
-include $(wildcard $(addsuffix .d, $(OBJECTS))) 

#############################################################################
# Implicit rules
#############################################################################
# compile and generate dependency info
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c -MMD -MP -MF $@.d $< -o $@ 

$(OBJDIR)/%.o: makefile

$(SRCDIR)/TauArgusJava_wrap.cpp : $(SRCDIR)/TauArgusJava.swg $(SRCDIR)/TauArgus.h
	$(SWIG) $(SFLAGS) -o $@ $<

$(OBJDIR)/Versioninfo.o : $(SRCDIR)/Versioninfo.rc
	$(WINDRES) $(SRCDIR)/Versioninfo.rc $(OBJDIR)/Versioninfo.o