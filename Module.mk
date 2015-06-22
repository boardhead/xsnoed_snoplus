# Module.mk for qsnoed module
#
# Author:  P. Harvey 01/04/02
#
# Contact: P. Harvey (phil@nsun.phy.queensu.ca)

MODNAME      := QSnoed
MODDIR       := qsnoed

QSNOED_FLAGS := -DROOT_FILE -DROOT_SYSTEM -DNO_MAIN -DFITTR

QSNOED_DIR   := $(MODDIR)
QSNOED_DIRS  := $(MODDIR)
QSNOED_DIRI  := $(MODDIR)

QSNOED_LH    := $(QSNOED_DIRI)/$(MODNAME)_LinkDef.h
QSNOED_DC    := $(QSNOED_DIRS)/$(MODNAME)_Dict.C
QSNOED_DO    := $(QSNOED_DC:.C=.o)
QSNOED_DH    := $(QSNOED_DC:.C=.h)

QSNOED_H     := $(filter-out $(QSNOED_LH) $(QSNOED_DH),$(wildcard $(QSNOED_DIRI)/*.h))
QSNOED_H     += $(wildcard $(QSNOED_DIRI)/include/*.h)
QSNOED_QH    := $(filter $(QSNOED_DIRI)/Q%,$(QSNOED_H))
QSNOED_QH    := $(filter-out $(QSNOED_DIRI)/QRchHist.h,$(QSNOED_QH))
QSNOED_CXX   := $(wildcard $(QSNOED_DIRS)/*.cxx)
QSNOED_O     := $(QSNOED_CXX:.cxx=.o)

# remove necessary object files from the qsnoed library
IGNORE       := root redispatch

# remove POpticalWindow from link unless -DOPTICAL_CAL specified
ifeq ($(findstring $(QSNOED_FLAGS),-DOPTICAL_CAL),)
IGNORE       += POpticalWindow
endif

# disable dispatcher if no dispatcher libraries available
ifeq ($(QDISP_LIB),)
QSNOED_FLAGS += -DNO_DISPATCH
endif

# don't add dispatcher library if NO_DISPATCH option enabled
ifeq ($(findstring $(QSNOED_FLAGS),-DNO_DISPATCH),)
QSNOED_DISP  := -lqdisp
else
QSNOED_DISP  :=
IGNORE       += QDispatch
endif

# now remove the ignored objects
IGNORE_O     := $(patsubst %,$(QSNOED_DIRS)/%.o,$(IGNORE))
QSNOED_O     := $(filter-out $(IGNORE_O),$(QSNOED_O))

QSNOED_DEP   := $(QSNOED_O:.o=.d) $(QSNOED_DO:.o=.d)

# used in the main Makefile
ALLHDRS      += $(patsubst $(QSNOED_DIRI)/%.h,include/%.h,$(QSNOED_H))
ALLLIBS      += $(QSNOED_LIB)
ALLEXECS     += bin/root

# include all dependency files
INCLUDEFILES += $(QSNOED_DEP)

# include local MyConfig.mk file if required
-include $(QSNOED_DIR)/MyConfig.mk

##### local rules #####

# we depend on all of our header files being up to date in the include directory
include/%.h:    $(QSNOED_DIRI)/%.h
		$(COPY_HEADER) $< $@
		
include/include/%.h:    $(QSNOED_DIRI)/include/%.h
		$(COPY_HEADER) $< $@

# rule for compiling ROOT objects
$(QSNOED_DIRS)/root.o:  $(QSNOED_DIRS)/root.cxx
	$(CXX) $(OPT) $(CXXFLAGS) $(QSNOED_FLAGS) $(ROOTINCS) -o $@ -c $<

# rule for compiling standard QSNO/X objects
$(QSNOED_DIRS)/Q%.o:    $(QSNOED_DIRS)/Q%.cxx
	$(CXX) $(OPT) $(CXXFLAGS) $(QSNOED_FLAGS) $(ROOTINCS) $(XMINCS) -o $@ -c $<
		
# rules for extra QSNO/X objects
$(QSNOED_DIRS)/x%.o:    $(QSNOED_DIRS)/x%.cxx
	$(CXX) $(OPT) $(CXXFLAGS) $(QSNOED_FLAGS) $(ROOTINCS) $(XMINCS) -o $@ -c $<

$(QSNOED_DIRS)/XSnoedWindow.o:  $(QSNOED_DIRS)/XSnoedWindow.cxx
	$(CXX) $(OPT) $(CXXFLAGS) $(QSNOED_FLAGS) $(ROOTINCS) $(XMINCS) -o $@ -c $<

# rule for compiling all other cxx files
$(QSNOED_DIRS)/%.o:   $(QSNOED_DIRS)/%.cxx
	$(CXX) $(OPT) $(CXXFLAGS) $(QSNOED_FLAGS) $(XMINCS) -o $@ -c $<

$(QSNOED_DO):         $(QSNOED_DC)
	$(CXX) $(NOOPT) $(CXXFLAGS) $(QSNOED_FLAGS) $(ROOTINCS) -I. -o $@ -c $<

$(QSNOED_DC):         $(QSNOED_QH) $(QSNOED_LH)
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -f $@ $(ROOTCINTFLAGS) $(QSNOED_FLAGS) $(QSNOED_QH) $(QSNOED_LH)

# rule for building library
$(QSNOED_LIB):        $(QSNOED_O) $(QSNOED_DO) $(QSNOED_LIBDEP)
	@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
	   "$(SOFLAGS)" libqsnoed.$(SOEXT) $@ "$(QSNOED_O) $(QSNOED_DO)" \
	   "$(QSNOED_LIBEXTRA) $(ROOTLIBS) $(XMLIBS) -L$(QSNO_ROOT)/lib -lqtree -lqdisp"
bin/root: $(QSNOED_DIRS)/root.o $(QSNOED_LIB) $(QTREE_LIB)
	@echo "=== Linking $@ ==="
	$(LD) $(LDFLAGS) -o $@ $< $(QSNOLIBDIRS) -lqsnoed -lqtree $(QSNOED_DISP) $(ROOTLIBS) $(XMLIBS) $(F77LIBS)

all-qsnoed:       $(QSNOED_LIB)

clean-qsnoed:
		@rm -f $(QSNOED_DIRS)/*~ $(QSNOED_DIRS)/*.o
		@rm -f $(QSNOED_DC) $(QSNOED_DH) $(QSNOED_DEP)

clean::         clean-qsnoed

prepare-qsnoed:
	@$(INSTALLDIR) include/include

prepare::       prepare-qsnoed

#end

