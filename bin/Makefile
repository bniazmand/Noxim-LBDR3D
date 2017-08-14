##### LIBRARIES CONFIGURATION #####

SYSTEMC := libs/systemc-2.3.0


##### DIRECTORIES #####

SYSTEMC_LIBS := $(wildcard $(SYSTEMC)/lib-*)

SRCDIR  := ../src
SUBDIRS := $(filter %/,$(wildcard $(SRCDIR)/*/))
OBJDIR  := ./build

SRCS := $(wildcard $(SRCDIR)/*.cpp) $(wildcard $(addsuffix *.cpp,$(SUBDIRS)))
OBJS := $(subst $(SRCDIR),$(OBJDIR),$(SRCS:.cpp=.o))

MODULE := noxim


##### COMPILER OPTIONS #####

CXX      := g++
OPT      := -O3
DEBUG    := -g
OTHER    := # -Wall -Wno-deprecated
CXXFLAGS := $(OPT) $(OTHER) $(DEBUG)

INCDIR := -I$(SRCDIR) -I$(SYSTEMC)/include 
LIBDIR := -L$(SRCDIR) -L$(SYSTEMC_LIBS)

LIBS := -lsystemc -lm 

SPACE := $(subst ,, )
VPATH := $(SRCDIR):$(subst $(SPACE),:,$(SUBDIRS))

##### RULES #####

$(MODULE): $(OBJS)
	$(CXX) $(LIBDIR) $^ $(LIBS) -o $@ 2>&1 | c++filt

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJS) $(MODULE) *~

depend: 
	makedepend $(SRCS) -Y -f- 2>/dev/null | sed 's_$(SRCDIR)_$(OBJDIR)_' > Makefile.deps

-include Makefile.deps
