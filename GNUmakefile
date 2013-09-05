# ====================================================
# Detect platform on which we're running (limited)
# ====================================================

BUILDDIR = build
DISTDIR  = dist
ifdef debug
	CONF = Debug
else
	CONF = Release
endif
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
	CFLAGS += -DNDEBUG -O2
endif
LDFLAGS = -shared 
ifeq ($(PLATFORM),Linux)
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

$(TARGET) : $(OBJECTS)
	$(LINK) $(LDFLAGS) -o $@ $^
#	cp -p $(TARGET) /opt/lib
#	ln -sf /opt/lib/$(LIBFILENAME) /opt/lib/$(SONAME)
#	ln -sf /opt/lib/$(SONAME) /opt/lib/$(LIBBASENAME)

$(BUILDDIR) $(BUILDDIR)/$(CONF) $(OBJDIR) $(DISTDIR) $(DISTDIR)/$(CONF) $(LIBDIR) :
	mkdir -p $@

# pull in dependency info for *existing* .o files
#-include $(OBJECTS:.o=.d)
-include $(wildcard $(addsuffix .d, ${OBJECTS})) 

#############################################################################
# Implicit rules
#############################################################################
# compile and generate dependency info
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CXX) $(CFLAGS) -c -MMD -MP -MF $@.d $< -o $@ 

$(OBJDIR)/%.o: makefile

$(SRCDIR)/%_wrap.cpp : $(SRCDIR)/%.swg
	$(SWIG) $(SFLAGS) -o $@ $<
