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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/AMPL.o \
	${OBJECTDIR}/Code.o \
	${OBJECTDIR}/DataCell.o \
	${OBJECTDIR}/General.o \
	${OBJECTDIR}/Ghmiter.o \
	${OBJECTDIR}/Hitas.o \
	${OBJECTDIR}/JJFormat.o \
	${OBJECTDIR}/JavaProgressListener.o \
	${OBJECTDIR}/Properties.o \
	${OBJECTDIR}/SubCodeList.o \
	${OBJECTDIR}/Table.o \
	${OBJECTDIR}/TauArgus.o \
	${OBJECTDIR}/TauArgusJava_wrap.o \
	${OBJECTDIR}/Variable.o \
	${OBJECTDIR}/tauargus_extern_TauArgusX.o


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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libNewTauComDLL.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libNewTauComDLL.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libNewTauComDLL.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--kill-at -shared

${OBJECTDIR}/AMPL.o: AMPL.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/AMPL.o AMPL.cpp

${OBJECTDIR}/Code.o: Code.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Code.o Code.cpp

${OBJECTDIR}/DataCell.o: DataCell.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/DataCell.o DataCell.cpp

${OBJECTDIR}/General.o: General.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/General.o General.cpp

${OBJECTDIR}/Ghmiter.o: Ghmiter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Ghmiter.o Ghmiter.cpp

${OBJECTDIR}/Hitas.o: Hitas.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Hitas.o Hitas.cpp

${OBJECTDIR}/JJFormat.o: JJFormat.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/JJFormat.o JJFormat.cpp

${OBJECTDIR}/JavaProgressListener.o: JavaProgressListener.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/JavaProgressListener.o JavaProgressListener.cpp

${OBJECTDIR}/Properties.o: Properties.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Properties.o Properties.cpp

${OBJECTDIR}/SubCodeList.o: SubCodeList.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/SubCodeList.o SubCodeList.cpp

${OBJECTDIR}/Table.o: Table.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Table.o Table.cpp

${OBJECTDIR}/TauArgus.o: TauArgus.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TauArgus.o TauArgus.cpp

${OBJECTDIR}/TauArgusJava_wrap.o: TauArgusJava_wrap.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TauArgusJava_wrap.o TauArgusJava_wrap.cpp

${OBJECTDIR}/Variable.o: Variable.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Variable.o Variable.cpp

${OBJECTDIR}/tauargus_extern_TauArgusX.o: tauargus_extern_TauArgusX.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -I../../../../Progra~1/Java/jdk1.7.0_17/include -I../../../../Progra~1/Java/jdk1.7.0_17/include/win32  -MMD -MP -MF $@.d -o ${OBJECTDIR}/tauargus_extern_TauArgusX.o tauargus_extern_TauArgusX.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libNewTauComDLL.${CND_DLIB_EXT}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
