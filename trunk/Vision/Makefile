################################################
#
# Vision
# 
# This makefile is free software; you can redistribute it and/or
# modify it under the terms of the enclosed JBQ code license.
# See data/LICENSE.Makefile
# 
# This makefile is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# License for more details.
# 
#
################################################

# the file name, ...
BINARY := Vision
VERSION := 0.9.7-H-090319
BUILDDATE := $(shell date +%b_%d_%Y)
BUILD_TYPE := Release

# system libs
REG_LIBS := -lbe -ltranslation -ltextencoding -ltracker -lnetwork -lroot

# lua
LUA_LIBS := -L./src/lua/lib -llua -llualib

################################################

# output directories

OBJDIR := obj
DEPDIR := dep
SRCDIR := src
DATADIR := data

TEMPDEPFILE := /tmp/vision_temp_deps

LIBLUA_PATH := ./src/lua/lib/liblua.a

# compiler, linker, ...

CC := gcc -c -pipe

CFLAGS := -D_ZETA_TS_FIND_DIR_ -DVERSION_STRING=\"$(VERSION)\" -D_ZETA_USING_DEPRECATED_API_

# These can be set from the shell to change the build type
ifeq ($(CHECK_MEM),true)
	CFLAGS += -fcheck-memory-usage -D_NO_INLINE_ASM=1
	BUILD_DEBUG=true
endif

ifeq ($(BUILD_DEBUG),true)
	CFLAGS += -g -O0
	
	ifeq ($(shell uname -r), 5.1)
		CFLAGS += -fno-debug-opt
	endif

	BUILD_TYPE = Debug
else
	CFLAGS += -O1
endif

ifeq ($(CHECK_MEM),true)
	BUILD_TYPE = MemCheck
endif

CFLAGS += -DBEOS_BUILD
CFLAGS += -DBUILD_DATE=\"$(BUILDDATE)\"
CFLAGS += -fno-pic
CFLAGS += -ffast-math
CFLAGS += -march=pentium -mcpu=pentium
CFLAGS += -Wall -W -Wno-multichar -Wpointer-arith
CFLAGS += -Wwrite-strings -Woverloaded-virtual
CFLAGS += -Wconversion -Wpointer-arith
CFLAGS += -Wmissing-declarations
CFLAGS += -Wcast-qual -Wshadow

# be able to add flags from the env at will without chaging the makefile
CFLAGS += $(VISION_ADDL_BUILD_FLAGS)

LD := gcc

LDFLAGS := $(REG_LIBS) -nodefaultlibs

ifeq ($(wildcard /boot/home/config/include/libim/InfoPopper.h), )
else
  CFLAGS += -DUSE_INFOPOPPER=1
endif

DEP := gcc -MM -DBEOS_BUILD

# create the dep dir.
MAKEDEP := $(shell mkdir -p $(DEPDIR))

ZIP := zip -r -9 -y

################################################

# the engine

MAKEFILE := Makefile

FULLNAME := $(subst \ ,_,$(BINARY))-$(VERSION)

DATA := $(shell find $(DATADIR) -type f)

EXTRADATA:= $(wildcard data.zip)

BASESOURCES := $(shell cd $(SRCDIR) && ls -1 *.cpp)
SOURCES := $(addprefix $(SRCDIR)/,$(BASESOURCES))
OBJECTS := $(addprefix $(OBJDIR)/,$(addsuffix .o,$(basename $(BASESOURCES))))
DEPENDS := $(addprefix $(DEPDIR)/,$(addsuffix .d,$(basename $(BASESOURCES))))

BASEHEADERS := $(shell cd $(SRCDIR) && ls -1 *.h)
HEADERS := $(addprefix $(SRCDIR)/,$(BASEHEADERS))

.PHONY : default release clean binarchive sourcearchive all

.DELETE_ON_ERROR : $(BINARY)

default : build

build : $(BINARY)

#	rule to create the object file directory if needed
$(OBJDIR)::
	@[ -d $(OBJDIR) ] || mkdir $(OBJDIR) > /dev/null 2>&1

clean :
	@echo cleaning
	@rm -rf $(BINARY) $(OBJDIR) $(DEPDIR) $(filter-out data.zip,$(wildcard *.zip)) *.zip~

all : sourcearchive binarchive

sourcearchive : $(FULLNAME)-src.zip

binarchive : $(FULLNAME).zip

cvsup:
	@echo CVS up -d
	@cvs up -d

$(BINARY) : $(OBJDIR) $(OBJECTS)
	@rm -f $(TEMPDEPFILE)
	@echo linking $@
	@$(LD) -o $@ $(OBJECTS) $(LDFLAGS)
	@echo merging resources
	@xres -o $@ $(SRCDIR)/Vision.rsrc
	@mimeset -f $@
	
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@echo compiling [$(BUILD_TYPE) Build] : $@
	@$(CC) $< $(CFLAGS) -o $@

$(DEPDIR)/%.d : $(SRCDIR)/%.cpp
	@/bin/echo generating dependencies for $<
	@/bin/echo $@ $(OBJDIR)/$(shell $(DEP) $<) > $(TEMPDEPFILE)
	@/bin/echo $(OBJDIR)/$(basename $(@F))".o : $(MAKEFILE)" | /bin/cat - $(TEMPDEPFILE) > $@

$(FULLNAME).zip : $(BINARY) $(DATA) $(EXTRADATA) $(MAKEFILE)
	@rm -rf $@~
	@mkdir -p $@~/$(FULLNAME)/
	@copyattr -d -r $(BINARY) $(DATADIR)/* $@~/$(FULLNAME)/
ifneq "$(EXTRADATA)" ""
	@unzip $(EXTRADATA) -d $@~/$(FULLNAME)/
endif
	@find $@~ -type f | xargs mimeset
	@rm -rf $@~/$(FULLNAME)/CVS $@~/$(FULLNAME)/protocol/CVS $@~/$(FULLNAME)/help/CVS $@~/$(FULLNAME)/scripts/CVS
	@rm -rf $@~/$(FULLNAME)/CVS $@~/$(FULLNAME)/help/content/CVS $@~/$(FULLNAME)/help/images/CVS
	@cd $@~ && $(ZIP) $@ $(FULLNAME)
	@mv -f $@~/$@ .
	@rm -rf $@~

$(FULLNAME)-src.zip : $(SOURCES) $(HEADERS) $(DATA) $(EXTRADATA) $(MAKEFILE)
	@rm -rf $@~
	@mkdir -p $@~/$(FULLNAME)-src/
	@copyattr -d -r $(SRCDIR) $(DATADIR) $(MAKEFILE) $@~/$(FULLNAME)-src/
ifneq "$(EXTRADATA)" ""
	@copyattr -d -r $(EXTRADATA) $@~/$(FULLNAME)-src/
endif
	@find $@~ -type f | xargs mimeset
	@rm -rf $@~/$(FULLNAME)-src/src/CVS $@~/$(FULLNAME)-src/data/CVS
	@rm -rf $@~/$(FULLNAME)-src/CVS $@~/$(FULLNAME)-src/data/protocol/CVS $@~/$(FULLNAME)-src/data/help/CVS $@~/$(FULLNAME)-src/data/scripts/CVS
	@rm -rf $@~/$(FULLNAME)-src/CVS $@~/$(FULLNAME)-src/data/help/content/CVS $@~/$(FULLNAME)-src/data/help/images/CVS
	@cd $@~ && $(ZIP) $@ $(FULLNAME)-src
	@mv -f $@~/$@ .
	@rm -rf $@~

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),depend)
ifneq ($(MAKECMDGOALS),sourcearchive)
-include $(DEPENDS)
endif
endif
endif

