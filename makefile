# ====================================================
# Configurable items
# ====================================================
GNUDIR  = /usr/bin
SWIGDIR = /usr/local/bin
JAVADIR = /home/argus/jdk1.7.0_25

LIBNAME       = TauArgusJava
MAJOR_VERSION = 1
MINOR_VERSION = 0
JAVAPACKAGE   = tauargus.extern

SRCDIR = .
OBJDIR = Linux
LIBDIR = Linux
OUTDIR = Linux

# ====================================================
# Non configurable items
# ====================================================

INCDIRS = $(JAVADIR)/include $(JAVADIR)/include/linux

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
INCLUDES = $(INCDIRS:/%=-I/%)

LIBBASEFILENAME = lib$(LIBNAME).so
SONAME = lib$(LIBNAME).so
#.$(MAJOR_VERSION)

TARGET = $(LIBDIR)/$(LIBBASEFILENAME)
#.$(MAJOR_VERSION).$(MINOR_VERSION)

.PHONY: all clean clean_all

all : $(OBJDIR) $(LIBDIR) $(TARGET)

$(TARGET) : $(OBJECTS)
	$(LINK) -shared -Wl,-soname,$(SONAME) -o $@ $^
#	cp -p $(TARGET) /opt/lib
#	ln -sf /opt/lib/$(LIBBASEFILENAME).$(MAJOR_VERSION).$(MINOR_VERSION) /opt/lib/$(LIBBASEFILENAME).$(MAJOR_VERSION)
#	ln -sf /opt/lib/$(LIBBASEFILENAME).$(MAJOR_VERSION) /opt/lib/$(LIBBASEFILENAME)

$(OBJDIR) :
	mkdir -p $@

$(LIBDIR) :
	mkdir -p $@

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

#############################################################################
# Implicit rules
#############################################################################
# compile and generate dependency info
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	rm -f $@
	$(CXX) $(CFLAGS) -c $< -o $@ 
	$(CXX) $(CFLAGS) -MM $< > $(@:.o=.d)

$(OBJDIR)/%.o: Makefile

$(SRCDIR)/%_wrap.cpp : $(SRCDIR)/%.swg
	$(SWIG) $(SFLAGS) -o $@ $<
