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
VERSION := 0.4-1004
BUILDDATE := $(shell date +%b_%d_%Y)

# system libs
REG_LIBS := -lbe -ltranslation -ltextencoding

# net_server libs
NET_LIBS := -lnet -lnetapi

# bone libs
BONE_LIBS := -lroot -lsocket -lbnetapi -lbind

# lua
LUA_LIBS := -L./src/lua/lib -llua -llualib

################################################

# output directories

OBJDIR := obj
DEPDIR := dep
SRCDIR := src
DATADIR := data

LIBLUA_PATH := ./src/lua/lib/liblua.a

# compiler, linker, ...

CC := g++ -c -pipe
CFLAGS := -DVERSION_STRING=\"$(VERSION)\"
ifeq ($(wildcard /system/lib/libbind.so),)
CFLAGS += -DNETSERVER_BUILD
else
CFLAGS += -DBONE_BUILD
endif

ifeq ($(VIS_BUILD),alan)
CFLAGS += -DALAN_BUILD=1
endif

ifeq ($(VIS_BUILD),be)
CFLAGS += -DBE_INC_BUILD=1
endif

ifndef VIS_BUILD
CFLAGS += -Wcast-qual -Wshadow
endif

ifeq ($(BUILD_DEBUG),true)
	CFLAGS += -g
else
	CFLAGS += -O1
endif

CFLAGS += -DBEOS_BUILD
CFLAGS += -DBUILD_DATE=\"$(BUILDDATE)\"
CFLAGS += -fno-pic
CFLAGS += -ffast-math
CFLAGS += -march=pentium -mcpu=pentium
CFLAGS += -Wall -W -Wno-multichar -Wpointer-arith
CFLAGS += -Wwrite-strings -Woverloaded-virtual
CFLAGS += -Wconversion -Wpointer-arith
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations

LD := g++

ifeq ($(wildcard /system/lib/libbind.so),)
LDFLAGS := $(REG_LIBS) $(LUA_LIBS) $(NET_LIBS)
else
LDFLAGS := $(REG_LIBS) $(LUA_LIBS) $(BONE_LIBS) -nodefaultlibs
endif

DEP := g++ -MM -DBEOS_BUILD

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

clean :
	@echo cleaning
	@rm -rf $(BINARY) $(OBJDIR) $(DEPDIR) $(filter-out data.zip,$(wildcard *.zip)) *.zip~

all : sourcearchive binarchive

sourcearchive : $(FULLNAME)-src.zip

binarchive : $(FULLNAME).zip

cvsup:
	@echo CVS up
	@cvs up

$(BINARY) : $(OBJECTS)
ifeq ($(wildcard ./src/lua/lib/liblua.a),)
	@echo Lua lib not found
	@echo Building...
	@cd $(SRCDIR)/lua && make >/dev/null && cd $(SRCDIR)/..
endif
	@echo linking $@
	@$(LD) -o $@ $(OBJECTS) $(LDFLAGS)
	@echo merging resources
	@xres -o $@ $(SRCDIR)/Vision.rsrc
	@mimeset -f $@
	
$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@echo compiling : $@
	@mkdir -p $(OBJDIR)
	@$(CC) $< $(CFLAGS) -o $@

$(DEPDIR)/%.d : $(SRCDIR)/%.cpp
	@echo generating dependencies for $<
	@mkdir -p $(DEPDIR)
	@$(DEP) $< > $@
	@echo -e yf.i$(OBJDIR)/\\033t:a$(DEPDIR)/\\033pad \\033f:a $(MAKEFILE)\\033ZZ | vi $@ >/dev/null 2>/dev/null

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
include $(DEPENDS)
endif
endif
endif
