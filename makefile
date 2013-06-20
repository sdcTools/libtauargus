GNUDIR  = /usr/bin
JAVADIR = /home/argus/jdk1.7.0_25
SWIGDIR =

SRCDIR  = .
INCDIRS = $(JAVADIR)/include $(JAVADIR)/include/linux
OBJDIR  = Linux
LIBDIR  = Linux

LIBNAME = TauArgusJava

CXX     = $(GNUDIR)/g++

CFLAGS  = $(INCLUDES)

# Use all .cpp files except 4 specific files used under Windows
SOURCES := $(wildcard $(SRCDIR)/*.cpp)
SOURCES := $(patsubst $(SRCDIR)/GhmiterANCO.cpp,,$(SOURCES))
SOURCES := $(patsubst $(SRCDIR)/TauArgCtrl.cpp,,$(SOURCES))
SOURCES := $(patsubst $(SRCDIR)/StdAfx.cpp,,$(SOURCES))
SOURCES := $(patsubst $(SRCDIR)/NewTauArgus.cpp,,$(SOURCES))

OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
INCLUDES = $(INCDIRS:/%=-I/%)

TARGET = $(LIBDIR)/lib$(LIBNAME).so

.PHONY: all clean clean_all

all : $(OBJDIR) $(LIBDIR) $(TARGET)

$(TARGET) : $(OBJECTS)

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
