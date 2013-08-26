# ====================================================
# Detect architecture on which we're running (limited)
# ====================================================

ifeq ($(OS),Windows_NT)
	ARC=Win32
else
	ARC=Linux
endif

# ====================================================
# Configurable items
# ====================================================

ifeq ($(ARC),Linux)
	GNUDIR  = /usr/bin
	SWIGDIR = /usr/local/bin
	JAVADIR = /home/argus/jdk1.7.0_25
else
	GNUDIR  = c:/MinGW/bin
	SWIGDIR = c:/swigwin-2.0.10
	JAVADIR = c:/Progra~1/Java/jdk1.7.0_17
endif

LIBNAME       = TauArgusJava
#MAJOR_VERSION = 1
#MINOR_VERSION = 0
JAVAPACKAGE   = tauargus.extern

# ======================================================================
# Set directories for input and output files
# ======================================================================

ARCDIR = $(ARC)
ifdef debug
	VERDIR = $(ARCDIR)/debug
else
	VERDIR = $(ARCDIR)/release
endif

SRCDIR = .
OBJDIR = $(VERDIR)
LIBDIR = $(VERDIR)
OUTDIR = $(VERDIR)

# ====================================================
# Non configurable items
# ====================================================

INCDIRS = $(JAVADIR)/include 
ifeq ($(ARC),Linux)
	INCDIRS += $(JAVADIR)/include/linux
else
	INCDIRS += $(JAVADIR)/include/Win32
endif

CXX     = $(GNUDIR)/g++
SWIG    = $(SWIGDIR)/swig
LINK    = $(GNUDIR)/g++

SFLAGS  = -c++ -java -package $(JAVAPACKAGE) -outdir $(OUTDIR)
CFLAGS  = $(INCLUDES) -Wall
ifeq ($(ARC),Linux)
	CFLAGS += -fPIC
else
	CFLAGS += -D_WIN32
endif
ifdef debug
	CFLAGS += -D_DEBUG -g3 -ggdb
else
	CFLAGS += -DNDEBUG -O2
endif
LDFLAGS = -shared 
ifeq ($(ARC),Linux)
	LDFLAGS += $(CFLAGS) -Wl,-soname,$(SONAME)
else
	LDFLAGS += -Wl,--subsystem,windows -Wl,--kill-at
endif

# Use all .cpp files except 4 specific files used under Windows and the generate source files
# Use := for static evaluation!
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
SOURCES := $(patsubst $(SRCDIR)/GhmiterANCO.cpp,,$(SOURCES)) 
SOURCES := $(patsubst $(SRCDIR)/TauArgCtrl.cpp,,$(SOURCES))
SOURCES := $(patsubst $(SRCDIR)/StdAfx.cpp,,$(SOURCES))
SOURCES := $(patsubst $(SRCDIR)/NewTauArgus.cpp,,$(SOURCES))
SOURCES := $(patsubst $(SRCDIR)/%_wrap.cpp,,$(SOURCES))

SWIGSOURCES       = $(wildcard $(SRCDIR)/*.swg)
GENERATED_SOURCES = $(patsubst $(SRCDIR)/%.swg,$(SRCDIR)/%_wrap.cpp,$(SWIGSOURCES))

OBJECTS  = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES) $(GENERATED_SOURCES))
INCLUDES = $(INCDIRS:%=-I%)

ifeq ($(ARC),Linux)
	LIBBASENAME = lib$(LIBNAME).so
	SONAME = $(LIBBASENAME) #.$(MAJOR_VERSION)
	LIBFILENAME = $(SONAME) #.$(MINOR_VERSION)
else
	LIBFILENAME = $(LIBNAME).dll
endif

TARGET = $(LIBDIR)/$(LIBFILENAME)

.PHONY: all clean

#.SECONDARY: $(SRCDIR)/TauArgusJava_wrap.cpp

all : $(OBJDIR) $(LIBDIR) $(TARGET)
	
clean :
	rm -rf $(OBJDIR) $(LIBDIR)
	rm -f $(SRCDIR)/*_wrap.*

$(TARGET) : $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $@ $^
#	cp -p $(TARGET) /opt/lib
#	ln -sf /opt/lib/$(LIBFILENAME) /opt/lib/$(SONAME)
#	ln -sf /opt/lib/$(SONAME) /opt/lib/$(LIBBASENAME)

$(ARCDIR) $(OBJDIR) $(LIBDIR) :
	mkdir -p $@

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

#############################################################################
# Implicit rules
#############################################################################
# compile and generate dependency info
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c -MMD $< -o $@ 

$(OBJDIR)/%.o: makefile

$(SRCDIR)/%_wrap.cpp : $(SRCDIR)/%.swg
	$(SWIG) $(SFLAGS) -o $@ $<
