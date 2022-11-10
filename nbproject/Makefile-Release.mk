#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=MinGW-Windows
CND_DLIB_EXT=dll
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/AMPL.o \
	${OBJECTDIR}/src/Code.o \
	${OBJECTDIR}/src/DataCell.o \
	${OBJECTDIR}/src/General.o \
	${OBJECTDIR}/src/Ghmiter.o \
	${OBJECTDIR}/src/Hitas.o \
	${OBJECTDIR}/src/JJFormat.o \
	${OBJECTDIR}/src/PTable.o \
	${OBJECTDIR}/src/PTableCont.o \
	${OBJECTDIR}/src/Properties.o \
	${OBJECTDIR}/src/SubCodeList.o \
	${OBJECTDIR}/src/Table.o \
	${OBJECTDIR}/src/TauArgus.o \
	${OBJECTDIR}/src/Variable.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/TauArgusJava.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/TauArgusJava.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/TauArgusJava.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--kill-at -shared

${OBJECTDIR}/src/AMPL.o: src/AMPL.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/AMPL.o src/AMPL.cpp

${OBJECTDIR}/src/Code.o: src/Code.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Code.o src/Code.cpp

${OBJECTDIR}/src/DataCell.o: src/DataCell.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/DataCell.o src/DataCell.cpp

${OBJECTDIR}/src/General.o: src/General.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/General.o src/General.cpp

${OBJECTDIR}/src/Ghmiter.o: src/Ghmiter.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Ghmiter.o src/Ghmiter.cpp

${OBJECTDIR}/src/Hitas.o: src/Hitas.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Hitas.o src/Hitas.cpp

${OBJECTDIR}/src/JJFormat.o: src/JJFormat.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/JJFormat.o src/JJFormat.cpp

${OBJECTDIR}/src/PTable.o: src/PTable.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/PTable.o src/PTable.cpp

${OBJECTDIR}/src/PTableCont.o: src/PTableCont.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/PTableCont.o src/PTableCont.cpp

${OBJECTDIR}/src/Properties.o: src/Properties.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Properties.o src/Properties.cpp

${OBJECTDIR}/src/SubCodeList.o: src/SubCodeList.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/SubCodeList.o src/SubCodeList.cpp

${OBJECTDIR}/src/Table.o: src/Table.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Table.o src/Table.cpp

${OBJECTDIR}/src/TauArgus.o: src/TauArgus.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/TauArgus.o src/TauArgus.cpp

.NO_PARALLEL:TauArgusJava_wrap.cpp TauArgusJava_wrap.h
TauArgusJava_wrap.cpp TauArgusJava_wrap.h: src/TauArgusJava.swg
	@echo Performing Custom Build Step
	D:/Peter-Paul/Documents/Thuiswerk/Programmatuur/swigwin-4.0.2/swig.exe -c++ -java -package tauargus.extern.dataengine -outdir ${CND_DISTDIR}/${CDN_CONF}/${CND_PLATFORM} -o TauArgusJava_wrap.cpp TauArgusJava.swg

${OBJECTDIR}/src/Variable.o: src/Variable.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -Wall -I../build/include -std=c++11  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Variable.o src/Variable.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} TauArgusJava_wrap.cpp TauArgusJava_wrap.h

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
