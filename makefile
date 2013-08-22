# ====================================================
# Detect architecture on which we're running (limited)
# ====================================================

ifndef OS
	OS := $(shell uname)
endif

ifeq ($(OS),Linux)
	ARC=Linux
else
	ARC=Win32
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
MAJOR_VERSION = 1
MINOR_VERSION = 0
JAVAPACKAGE   = tauargus.extern

# ======================================================================
# Set directories for input and output files
# ======================================================================

ARCDIR = $(ARC)
ifdef debug
	VERDIR = $(ARCDIR)/debug
#CFLAGS += $(CFLAGS_D)
else
	VERDIR = $(ARCDIR)/release
#CFLAGS += $(CFLAGS_R)
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

CFLAGS  = $(INCLUDES) -Wall -fPIC
SFLAGS  = -c++ -java -package $(JAVAPACKAGE) -outdir $(OUTDIR)
LFLAGS  = $(LINK) -shared -Wl,-soname,$(TARGET).1 -o $(TARGET).1.0 $(OBJECTS)

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

LIBBASEFILENAME = lib$(LIBNAME).so
SONAME = lib$(LIBNAME).so
#.$(MAJOR_VERSION)

TARGET = $(LIBDIR)/$(LIBBASEFILENAME)
#.$(MAJOR_VERSION).$(MINOR_VERSION)

.PHONY: all clean clean_all

all : $(OBJDIR) $(LIBDIR) $(TARGET)
	echo $(INCDIRS)
	
clean :
	rm -rf $(OBJDIR) $(LIBDIR)

$(TARGET) : $(OBJECTS)
	$(LINK) -shared -Wl,-soname,$(SONAME) -o $@ $^
#	cp -p $(TARGET) /opt/lib
#	ln -sf /opt/lib/$(LIBBASEFILENAME).$(MAJOR_VERSION).$(MINOR_VERSION) /opt/lib/$(LIBBASEFILENAME).$(MAJOR_VERSION)
#	ln -sf /opt/lib/$(LIBBASEFILENAME).$(MAJOR_VERSION) /opt/lib/$(LIBBASEFILENAME)

$(ARCDIR) $(OBJDIR) $(LIBDIR) :
	mkdir -p $@

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

#############################################################################
# Implicit rules
#############################################################################
# compile and generate dependency info
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
#	rm -f $@
	$(CXX) $(CFLAGS) -c -MMD $< -o $@ 

$(OBJDIR)/%.o: Makefile

$(SRCDIR)/%_wrap.cpp : $(SRCDIR)/%.swg
	$(SWIG) $(SFLAGS) -o $@ $<
