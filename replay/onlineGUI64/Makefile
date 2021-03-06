# Makefile for standalone version of panguin/onlineGUI
#  - B. Moffit 
# 

# Choose the compiler.
GCC=g++
GLD=g++
ifdef INSURE
  GCC=insure
  GLD=insure
endif

ROOTLIBS      = $(shell root-config --libs)
ROOTGLIBS     = $(shell root-config --glibs)
INCLUDES      = -I$(ROOTSYS)/include
SOFLAGS       = -shared 
GLIBS         = $(ROOTGLIBS) -L/usr/lib64/libXpm.so.4 -L/usr/lib64 -lX11
LIBS          = $(GLIBS) $(ROOTLIBS) $(ROOTGLIBS)

CXX           = $(GCC)
CXXFLAGS      = -Wall -fno-exceptions -fPIC $(INCLUDES) -fpermissive
LD            = $(GLD)
LDFLAGS       = 
MAKEDEPEND    = $(GCC)

ifdef ANALYZER
  INCDIRS  = $(wildcard $(addprefix $(ANALYZER)/, include src hana_decode hana_scaler))
  LIBDIRS  = $(wildcard $(addprefix $(ANALYZER)/, . lib))
  ifneq ($(strip $(INCDIRS)),)
    INCLUDES     += $(addprefix -I, $(INCDIRS) )
  endif
  ifneq ($(strip $(LIBS)),)
    LIBS         += $(addprefix -L, $(LIBDIRS) ) -lHallA -ldc -lscaler
  endif
endif

ALL_LIBS = $(LIBS)


CXXFLAGS += -DSTANDALONE
CXXFLAGS += -g -O0

# Insert extra headers needed by GetRootFileName.C or GetRunNumber.C
ifdef PAN_SRC_PATH
	PANSRCPATH = $(PAN_SRC_PATH)
else
	PANSRCPATH    = ../../pan/src
endif
# XTRAHEADERS = $(PANSRCPATH)/TaFileName.hh $(PANSRCPATH)/TaString.hh\
# 	$(PANSRCPATH)/TaDataBase.hh
# INCLUDES     += -I$(PANSRCPATH)
# LIBPANCLASS   = $(PANSRCPATH)/libpan.a
# LIBS +=  $(LIBPANCLASS)

SRC = online.C
DEPS = $(SRC:.C=.d)
OBJS = $(SRC:.C=.o) 
HEAD = $(SRC:.C=.h)

# Make the dictionary
CXXFLAGS += -DDICT
OBJS += onlineDict.o

PROGS = online

install: all
#	 mv $(PROGS) ../

all: $(PROGS) 

$(PROGS): $(OBJS) $(SRC) $(HEAD) 
	rm -f $@
	@echo "Generating" $@
	$(LD) $(CXXFLAGS) -o $@ $(OBJS) $(ALL_LIBS) 

# Dictionary
onlineDict.C: $(DOBJS) onlineLinkDef.h
	@echo "Generating Dictionary..."
	$(ROOTSYS)/bin/rootcint -f $@ -c -p -DDICT $(HEAD) \
	$(XTRAHEADERS) onlineLinkDef.h

clean:
	rm -f *.o core *~ *.d *.tar *.so  *Dict* $(PROGS)

realclean:  clean
	rm -f *.d ../$(PROGS) 

.SUFFIXES:
.SUFFIXES: .c .cc .cpp .C .o .d

%.o:	%.C
	$(CXX) $(CXXFLAGS) -c $<

%.d:	%.C
	@echo Creating dependencies for $<
	@$(SHELL) -ec '$(MAKEDEPEND) -MM $(INCLUDES) -c $< \
	      | sed '\''s%^.*\.o%$*\.o%g'\'' \
	      | sed '\''s%\($*\)\.o[ :]*%\1.o $@ : %g'\'' > $@; \
	      [ -s $@ ] || rm -f $@'

-include $(DEPS)


