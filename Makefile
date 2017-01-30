# ====================================================
# Detect platform on which we're running (limited)
# ====================================================

ifeq ($(OS),Windows_NT)
	PLATFORM=MinGW-Windows
else
	PLATFORM=Linux
endif

# ====================================================
# Configurable items
# ====================================================

ifeq ($(PLATFORM),Linux)
	GNUDIR  = /usr/bin
	SWIGDIR = /usr/local/bin
	JAVADIR = /home/argus/jdk1.7.0_25
else
	GNUDIR  = C:/MinGW/bin
	SWIGDIR = C:/swigwin-3.0.10
	JAVADIR = C:/Progra~2/Java/jdk1.7.0_80
endif

LIBNAME       = TauArgusJava
#MAJOR_VERSION = 1
#MINOR_VERSION = 0
JAVAPACKAGE   = tauargus.extern.dataengine

# ======================================================================
# Set directories for input and output files
# ======================================================================

BUILDDIR = build
DISTDIR  = dist

ifdef debug # NB: does not set CONF correctly from within Netbeans IDE
	CONF = Debug
else
	CONF = Release
endif
BUILDFILEDIR = $(BUILDDIR)/$(CONF)/$(PLATFORM)
DISTFILEDIR  = $(DISTDIR)/$(CONF)/$(PLATFORM)

SRCDIR = .
OBJDIR = $(BUILDFILEDIR)
LIBDIR = $(DISTFILEDIR)
OUTDIR = $(DISTFILEDIR)

# ====================================================
# Non configurable items
# ====================================================

INCDIRS = $(JAVADIR)/include 
ifeq ($(PLATFORM),Linux)
	INCDIRS += $(JAVADIR)/include/linux
else
	INCDIRS += $(JAVADIR)/include/Win32
endif

CXX     = $(GNUDIR)/g++
SWIG    = $(SWIGDIR)/swig
LINK    = $(GNUDIR)/g++

SFLAGS  = -c++ -java -package $(JAVAPACKAGE) -outdir $(OUTDIR)
CFLAGS  = $(INCLUDES) -Wall
ifeq ($(PLATFORM),Linux)
	CFLAGS += -fPIC
endif
ifdef debug
	CFLAGS += -D_DEBUG -g
else
	CFLAGS += -DNDEBUG -O2 -fno-strict-aliasing
endif
LDFLAGS = -shared 
ifeq ($(PLATFORM),Linux)
	LDFLAGS += $(CFLAGS) -Wl,-soname,$(SONAME)
else
	LDFLAGS += $(CFLAGS) -Wl,--subsystem,windows -Wl,--kill-at -static-libgcc -static-libstdc++
endif

# Exclude source files needed for a COM dll for Visual Basic 6.0
NOSOURCES         = $(SRCDIR)/GhmiterANCO.cpp $(SRCDIR)/TauArgCtrl.cpp $(SRCDIR)/StdAfx.cpp $(SRCDIR)/NewTauArgus.cpp
SWIGSOURCES       = $(wildcard $(SRCDIR)/*.swg)
GENERATED_SOURCES = $(patsubst $(SRCDIR)/%.swg,$(SRCDIR)/%_wrap.cpp,$(SWIGSOURCES))
SOURCES           = $(filter-out $(NOSOURCES),$(wildcard $(SRCDIR)/*.cpp))

OBJECTS  = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES) $(GENERATED_SOURCES))
INCLUDES = $(INCDIRS:%=-I%)

ifeq ($(PLATFORM),Linux)
	LIBBASENAME = lib$(LIBNAME).so
	SONAME = $(LIBBASENAME) #.$(MAJOR_VERSION)
	LIBFILENAME = $(SONAME) #.$(MINOR_VERSION)
else
	LIBFILENAME = $(LIBNAME).dll
endif

TARGET = $(LIBDIR)/$(LIBFILENAME)

.PHONY: all clean

.SECONDARY: $(SRCDIR)/TauArgusJava_wrap.cpp

all : $(OBJDIR) $(LIBDIR) $(TARGET)
	
clean :
	rm -rf $(OBJDIR) $(LIBDIR)
	rm -f $(SRCDIR)/*_wrap.*

$(BUILDDIR) $(BUILDDIR)/$(CONF) $(OBJDIR) $(DISTDIR) $(DISTDIR)/$(CONF) $(LIBDIR) :
	mkdir -p $@

$(TARGET) : $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $@ $^
	cp -p $(TARGET) $(SRCDIR)/../tauargus/TauArgusJava.dll
#	cp -p $(TARGET) /opt/lib
#	ln -sf /opt/lib/$(LIBFILENAME) /opt/lib/$(SONAME)
#	ln -sf /opt/lib/$(SONAME) /opt/lib/$(LIBBASENAME)

# pull in dependency info for *existing* .o files
-include $(wildcard $(addsuffix .d, ${OBJECTS})) 

#############################################################################
# Implicit rules
#############################################################################
# compile and generate dependency info
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c -MMD -MP -MF $@.d $< -o $@ 

$(OBJDIR)/%.o: makefile

$(SRCDIR)/TauArgusJava_wrap.cpp : $(SRCDIR)/TauArgusJava.swg $(SRCDIR)/TauArgus.h
	$(SWIG) $(SFLAGS) -o $@ $<
