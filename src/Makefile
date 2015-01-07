# This is a sample Makefile for debuging LilyTerm.
#
# Usage:
#	make clean: delete lilyterm binary files.
#	make debug: compile lilyeterm_dbg with CLAGS -g.
#	make dev: compile lilyeterm_dev with CLAGS -g.
#	make vdev: compile lilyeterm_dev with full compiling command message.
#	make ddev: compile lilyeterm_dev with -DSAFEMODE compile option.
#	make ut: run a tiny unit testing script with gdb debug information.
#	make uto: build *.o binary files for using in the unit testing programs.
#	make utb: build the the unit testing programs only.
#	make utd: run a tiny unit testing script with full runtime debug information.
#	make utf: run a tiny unit testing script with glib, gtk, gdb and valgrind debug information.
#	make geometry: compile lilyeterm_dev with geometry debug information.
#	make detail: compile lilyeterm_dev with detail runtime debug information.
#	make full: compile lilyeterm_dev with full runtime debug information.
#	make memory: compile lilyeterm_dev for memory lack testing.
#	make data: compile po and $(BINARY).conf files.
#
# grep 'g_debug *( *" *[^\!@+\?-]' * | grep -v '\/\/'
# (\(console\|dialog\|font\|main\|menu\|misc\|notebook\|pagename\|profile\|property\|window\).c:[0-9]*)
# MANWIDTH=80 man data/lilyterm.1 | col -b > README

include ../.default
-include ../.config

PKGCONFIG := $(shell whereis "pkg-config" | tr -s ' ' '\n' | grep "bin/""pkg-config""$$" | head -n 1)
STRIP := $(shell whereis "strip" | tr -s ' ' '\n' | grep "bin/""strip""$$" | head -n 1)

VTE ?=

ifneq ($(PKGCONFIG), )

ifeq ($(VTE), vte)
GTK := $(shell $(PKGCONFIG) --exists 'gtk+-2.0' && $(ECHO) 'gtk+-2.0')
endif
ifeq ($(VTE), vte-2.91)
GTK := $(shell $(PKGCONFIG) --exists 'gtk+-3.0' && $(ECHO) 'gtk+-3.0')
endif
ifeq ($(VTE), vte-2.90)
GTK := $(shell $(PKGCONFIG) --exists 'gtk+-3.0' && $(ECHO) 'gtk+-3.0')
endif
ifeq ($(VTE), )
VTE := $(shell $(PKGCONFIG) --exists 'vte' && $(ECHO) 'vte')
GTK := $(shell $(PKGCONFIG) --exists 'gtk+-2.0' && $(ECHO) 'gtk+-2.0')
endif
ifeq ($(VTE), )
VTE := $(shell $(PKGCONFIG) --exists 'vte-2.91' && $(ECHO) 'vte-2.91')
GTK := $(shell $(PKGCONFIG) --exists 'gtk+-3.0' && $(ECHO) 'gtk+-3.0')
endif
ifeq ($(VTE), )
VTE := $(shell $(PKGCONFIG) --exists 'vte-2.90' && $(ECHO) 'vte-2.90')
GTK := $(shell $(PKGCONFIG) --exists 'gtk+-3.0' && $(ECHO) 'gtk+-3.0')
endif

endif

BINDIR = $(PREFIX)/bin
date = `date +%Y-%m-%d`
CC ?= gcc
CFLAGS ?= -Wall -Werror -Os
#CFLAGS = $$CFLAGS -Wall -Wextra -Os -std=c99 \
#	 -DG_DISABLE_DEPRECATED \
#	 -DG_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
#	 -DGDK_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED \
#	 -DGTK_DISABLE_SINGLE_INCLUDES -DGSEAL_ENABLE
LDFLAGS += -L$(prefix)/local/lib
ifeq ($(VTE), vte-2.91)
LDFLAGS += -lX11
endif
OBJ_PKG_CMD = $(PKGCONFIG) --cflags $(GTK) $(VTE)
BINARY_PKG_CMD = $(PKGCONFIG) --cflags --libs $(GTK) $(VTE)
SOURCE_FILES = menu.c profile.c dialog.c pagename.c notebook.c font.c \
	       property.c window.c misc.c console.c main.c
OBJ = $(SOURCE_FILES:.c=.o)

AUTHOR_CONVERT = $(shell echo "$(AUTHOR)" | sed -e 's/\([ ()]\)/\\\1/g')

INCLUDES = -DBINARY=\"$(BINARY)\" \
	   -DPACKAGE=\"$(PACKAGE)\" \
	   -DBUGREPORT=\"$(BUGREPORT)\" \
	   -DMAINSITE=\"$(MAINSITE)\" \
	   -DGITHUBURL=\"$(GITHUBURL)\" \
	   -DBLOG=\"$(BLOG)\" \
	   -DWIKI=\"$(WIKI)\" \
	   -DISSUES=\"$(ISSUES)\" \
	   -DIRC=\"$(IRC)\" \
	   -DYEAR=\"$(YEAR)\" \
	   -DETCDIR=\"$(ETCDIR)\" \
	   -DLOCALEDIR=\"$(LOCALEDIR)\" \
	   -DICONDIR=\"$(ICONDIR)\"

ifeq ($(BSD), 1)
	INCLUDES += -DBSD
endif
ifeq ($(VERBOSITY), Y)
	VERBOSITY_OUTPUT =
else
	VERBOSITY_OUTPUT = @
endif

.PHONY: all
all: default
ifeq ($(DEBUG), N)
	@ $(PRINTF) "\033[1;36m** striping \033[1;32m$(BINARY)\033[1;36m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) $(STRIP) --remove-section=.comment --remove-section=.note $(BINARY)
endif

.PHONY: default
ifneq ($(SAFEMODE), N)
default: CFLAGS += -DSAFEMODE
else
default: CFLAGS += -DREALMODE
endif
default: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(AUTHOR_CONVERT)\" -DVERSION=\"$(VERSION)\ \($(date)\)\"
ifeq ($(DEBUG), Y)
default: INCLUDES += -DDEBUG -DFATAL
endif
default: $(BINARY)

.PHONY: debug
debug: CFLAGS := $(CFLAGS) -g
debug: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(AUTHOR_CONVERT)\" -DDEBUG -DFATAL -DFORCE_ENABLE_VTE_BACKGROUND -DVERSION=\"$(VERSION)\ Debug\ Version\ \($(date)\)\"
debug: BINARY := $(BINARY)_dbg
debug: $(BINARY)

.PHONY: dev
dev: develop
.PHONY: develop
# develop: CFLAGS := -pedantic -pipe -fno-common -W -Wall -Wcast-align -Wformat=2 -Wpointer-arith -Wundef -Waggregate-return -Wcast-qual -Wmissing-declarations -Wnested-externs -Wstrict-prototypes -O3 -g
develop: CFLAGS := $(CFLAGS) -g
develop: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(AUTHOR_CONVERT)\" -DREALMODE -DDEBUG -DFATAL -DFORCE_ENABLE_VTE_BACKGROUND -DDEVELOP -DVERSION=\"$(VERSION)\ Develop\ Version\ \($(date)\)\"
develop: BINARY := $(BINARY)_dev
develop: $(BINARY)

.PHONY: vdev
vdev: verbosity-output
.PHONY: verbosity-output
verbosity-output: VERBOSITY_OUTPUT :=
verbosity-output: develop

.PHONY: ddev
ddev: defensive-develop
.PHONY: defensive-develop
defensive-develop: CFLAGS := $(CFLAGS) -g
defensive-develop: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(AUTHOR_CONVERT)\" -DSAFEMODE -DDEBUG -DFATAL -DDEVELOP -DVERSION=\"$(VERSION)\ Develop\ Version\ \($(date)\)\"
defensive-develop: BINARY := $(BINARY)_dev
defensive-develop: $(BINARY)

.PHONY: detail
detail: CFLAGS := $(CFLAGS) -g
detail: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(AUTHOR_CONVERT)\" -DGEOMETRY -DDEBUG -DFATAL -DDEVELOP -DDETAIL -DVERSION=\"$(VERSION)\ Develop\ Version\ \($(date)\)\"
detail: BINARY := $(BINARY)_dev
detail: $(BINARY)

.PHONY: full
full: CFLAGS = -Wall -Werror -Wformat -Wformat-security -Werror=format-security -Os -g
full: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(AUTHOR_CONVERT)\" -DGEOMETRY -DDEBUG -DFATAL -DDEVELOP -DDETAIL -DFULL -DVERSION=\"$(VERSION)\ Develop\ Version\ \($(date)\)\"
full: BINARY := $(BINARY)_dev
full: $(BINARY)

.PHONY: geometry
geometry: CFLAGS := $(CFLAGS) -g
geometry: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(AUTHOR_CONVERT)\" -DGEOMETRY -DDEBUG -DFATAL -DDEVELOP -DVERSION=\"$(VERSION)\ Develop\ Version\ \($(date)\)\"
geometry: BINARY := $(BINARY)_dev
geometry: $(BINARY)

.PHONY: ut
ut: unit_test
.PHONY: unit_test
unit_test: CFLAGS := $(CFLAGS) -g
unit_test: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(PACKAGE)\" -DSAFEMODE -DDEBUG -DFATAL -DDEVELOP -DUNIT_TEST -DVERSION=\"$(VERSION)_UNIT_TEST_$(date)\"
unit_test: $(OBJ)
	@ sh unit_test.sh "$(INCLUDES)" --enable-gdb --test_all

.PHONY: uto
uto: unit_test_object
.PHONY: unit_test_object
unit_test_object: CFLAGS := $(CFLAGS) -g
unit_test_object: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(PACKAGE)\" -DSAFEMODE -DGEOMETRY -DDEBUG -DFATAL -DDEVELOP -DDETAIL -DFULL -DUNIT_TEST -DVERSION=\"$(VERSION)_UNIT_TEST_$(date)\"
unit_test_object: $(OBJ)

.PHONY: utb
utb: unit_test_build
.PHONY: unit_test_build
unit_test_build: CFLAGS := $(CFLAGS) -g
unit_test_build: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(PACKAGE)\" -DSAFEMODE -DGEOMETRY -DDEBUG -DFATAL -DDEVELOP -DDETAIL -DFULL -DUNIT_TEST -DVERSION=\"$(VERSION)_UNIT_TEST_$(date)\"
unit_test_build: $(OBJ)
	@ sh unit_test.sh "$(INCLUDES)" --build_program_only

.PHONY: utd
utd: unit_test_debug
.PHONY: unit_test_debug
unit_test_debug: CFLAGS := $(CFLAGS) -g
unit_test_debug: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(PACKAGE)\" -DSAFEMODE -DGEOMETRY -DDEBUG -DFATAL -DDEVELOP -DDETAIL -DFULL -DUNIT_TEST -DVERSION=\"$(VERSION)_UNIT_TEST_$(date)\"
unit_test_debug: $(OBJ)
	@ sh unit_test.sh "$(INCLUDES)" --enable-glib-debugger --enable-gtk-debugger --enable-gdb

.PHONY: utf
utf: unit_test_full
.PHONY: unit_test_full
unit_test_full: CFLAGS := $(CFLAGS) -g
unit_test_full: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(PACKAGE)\" -DSAFEMODE -DDEBUG -DFATAL -DDEVELOP -DUNIT_TEST -DVERSION=\"$(VERSION)_UNIT_TEST_$(date)\"
unit_test_full: $(OBJ)
	@ sh unit_test.sh "$(INCLUDES)" --enable-glib-debugger --enable-gtk-debugger --enable-gdb --enable-valgrind

.PHONY: memory
memory: CFLAGS = -Wall -Wformat -Wformat-security -Werror=format-security -O2
memory: INCLUDES := $(INCLUDES) -DAUTHOR=\"$(AUTHOR_CONVERT)\" -DDEBUG -DFATAL -DDEVELOP -DSAFEMODE -DOUT_OF_MEMORY -DVERSION=\"$(VERSION)\ Develop\ Version\ \($(date)\)\"
memory: BINARY := $(BINARY)_dev
memory: $(BINARY)

.PHONY: clean
clean:
	-@ for FileName in $(BINARY) $(BINARY)_dev $(BINARY)_dbg $(OBJ) unit_test unit_test.[co] gdb_batch gdb.log valgrind.log vgcore.*; \
	do \
		if [ -f "$$FileName" ]; then \
			$(PRINTF) "\033[1;35m** deleting \033[1;32m$$FileName\033[1;35m ...\033[0m\n"; \
			rm "$$FileName"; \
		fi; \
	done

%.o: %.c %.h data.h $(BINARY).h
	@ $(PRINTF) "\033[1;33m** compiling \033[1;32m$@\033[1;33m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) $(CC) $(CPPFLAGS) $(CFLAGS) $(INCLUDES) -c $< `$(OBJ_PKG_CMD)`

$(BINARY): $(OBJ)
	@ $(PRINTF) "\033[1;36m** compiling \033[1;32m$(BINARY)\033[1;36m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) $(CC) $(LDFLAGS) $(CPPFLAGS) $(CFLAGS) -o $(BINARY) $(OBJ) `$(BINARY_PKG_CMD)`

.PHONY: data
data: debug
	@ $(MAKE) -C ../po po

.PHONY: install
install: $(BINARY)
	@ install -d $(DESTDIR)/$(BINDIR)
	@ if [ -f $(BINARY) ]; then \
		$(PRINTF) "\033[1;33m** installing \033[1;32m$(BINARY)\033[1;33m to \033[1;34m$(DESTDIR)/$(BINDIR)\033[1;33m ...\033[0m\n" ; \
		install -m 755 $(BINARY) $(DESTDIR)/$(BINDIR) ; \
	fi ; \

.PHONY: uninstall
uninstall:
	@ - if [ -f $(DESTDIR)/$(BINDIR)/$(BINARY) ]; then \
		$(PRINTF) "\033[1;35m** deleting \033[1;32m$(DESTDIR)/$(BINDIR)/$(BINARY)\033[1;35m ...\033[0m\n" ; \
		rm "$(DESTDIR)/$(BINDIR)/$(BINARY)" ; \
		DirName="$(DESTDIR)/$(BINDIR)" ; \
		until [ -n "`ls -A $$DirName`" -o "$$DirName" = "$(DESTDIR)/$(PREFIX)" ]; do \
			$(PRINTF) "\033[1;35m** deleting directory \033[1;34m$$DirName\033[1;35m ...\033[0m\n" ; \
			rmdir "$$DirName" ; \
			DirName=`dirname $$DirName` ; \
		done \
	else \
		$(PRINTF) "\033[1;31m** \033[1;32m$(DESTDIR)/$(BINDIR)/$(BINARY)\033[1;31m is NOT exist!\033[0m\n" ; \
	fi
