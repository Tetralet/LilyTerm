# This is a sample Makefile for debuging LilyTerm.
# Usage:
#	make clean: delete lilyterm binary files.
#	make debug: compile lilyeterm_dbg with CLAGS -g.
#	make dev: compile lilyeterm_dev with CLAGS -g.
#	make ddev: compile lilyeterm_dev with -DDEFENSIVE compile option.
#	make ut: compile lilyeterm_dev for unit testing.
#	make utd: compile lilyeterm_dev for unit testing with detail runtime debug information.
#	make geometry: compile lilyeterm_dev with geometry debug information.
#	make detail: compile lilyeterm_dev with detail runtime debug information.
#	make full: compile lilyeterm_dev with full runtime debug information.
#	make memory: compile lilyeterm_dev for memory lack testing.
#	make data: compile po and lilyterm.conf files.

BINARY = lilyterm
VER = 0.9.9

PKGCONFIG := $(shell whereis pkg-config | tr -d '\n' | sed 's/^pkg-config://g')
VTE ?= ""

ifneq ($(PKGCONFIG), )

ifeq ($(VTE), "vte")
GTK := $(shell pkg-config --exists 'gtk+-2.0' && echo 'gtk+-2.0')
endif
ifeq ($(VTE), "vte-2.90")
GTK := $(shell pkg-config --exists 'gtk+-3.0' && echo 'gtk+-3.0')
endif
ifeq ($(VTE), "")
VTE := $(shell pkg-config --exists 'vte' && echo 'vte')
GTK := $(shell pkg-config --exists 'gtk+-2.0' && echo 'gtk+-2.0')
endif
ifeq ($(VTE), )
VTE := $(shell pkg-config --exists 'vte-2.90' && echo 'vte-2.90')
GTK := $(shell pkg-config --exists 'gtk+-3.0' && echo 'gtk+-3.0')
endif

endif

prefix ?= /usr
bindir = $(prefix)/bin
etcdir = /etc/xdg
datadir = $(prefix)/share
date = `date +%Y-%m-%d`
CC ?= gcc
CFLAGS = $$CFLAGS -Wall -Werror -Wformat -Wformat-security -Werror=format-security -O2
#CFLAGS = $$CFLAGS -Wall -Wextra -Os -std=c99 \
#	 -DG_DISABLE_DEPRECATED \
#	 -DG_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
#	 -DGDK_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED \
#	 -DGTK_DISABLE_SINGLE_INCLUDES -DGSEAL_ENABLE
LDFLAGS = -L$(prefix)/local/lib
OBJ_PKG_CMD = pkg-config --cflags $(GTK) $(VTE)
BINARY_PKG_CMD = pkg-config --cflags --libs $(GTK) $(VTE)
PO_LIST = zh_TW de fr es it nl pl ru sk tr uk zh_CN
OBJ = menu.o profile.o dialog.o pagename.o notebook.o font.o \
      property.o window.o misc.o console.o main.o

INCLUDES = -DPACKAGE=\"$(BINARY)\" \
	   -DPACKAGE_NAME=\"LilyTerm\" \
	   -DPACKAGE_BUGREPORT=\"tetrlet@gmail.com\" \
	   -DLOCALEDIR=\"$(datadir)/locale\" \
	   -DICONDIR=\"$(datadir)/pixmaps\"

.PHONY: all
all: default
	strip --remove-section=.comment --remove-section=.note $(BINARY)

.PHONY: default
default: CFLAGS := $(CFLAGS) -DDEFENSIVE
default: INCLUDES := $(INCLUDES) -DVERSION=\"$(VER)\ \($(date)\)\"
default: compile

.PHONY: debug
debug: CFLAGS := $(CFLAGS) -g
debug: INCLUDES := $(INCLUDES) -DDEFENSIVE -DDEBUG -DFATAL -DVERSION=\"$(VER)\ Debug\ Version\ \($(date)\)\"
debug: BINARY := $(BINARY)_dbg
debug: compile

.PHONY: dev
dev: develop
.PHONY: develop
develop: CFLAGS := $(CFLAGS) -g
develop: INCLUDES := $(INCLUDES) -DDEBUG -DFATAL -DDEVELOP -DVERSION=\"$(VER)\ Develop\ Version\ \($(date)\)\"
develop: BINARY := $(BINARY)_dev
develop: compile

.PHONY: ddev
ddev: defensive-develop
.PHONY: defensive-develop
defensive-develop: CFLAGS := $(CFLAGS) -g
defensive-develop: INCLUDES := $(INCLUDES) -DDEFENSIVE -DDEBUG -DFATAL -DDEVELOP -DVERSION=\"$(VER)\ Develop\ Version\ \($(date)\)\"
defensive-develop: BINARY := $(BINARY)_dev
defensive-develop: compile

.PHONY: ut
ut: unit_test
.PHONY: unit_test
unit_test: CFLAGS := $(CFLAGS) -g
unit_test: INCLUDES := $(INCLUDES)        -DDEFENSIVE -DDEBUG -DFATAL -DDEVELOP -DUNIT_TEST -DVERSION=\"$(VER)\ Develop\ Version\ \($(date)\)\"
unit_test: BINARY := $(BINARY)_dev
unit_test: OBJ := $(OBJ) unit_test.o
unit_test: unit_test.o compile

.PHONY: utd
utd: unit_test_detail
.PHONY: unit_test_detail
unit_test_detail: CFLAGS := $(CFLAGS) -g
unit_test_detail: INCLUDES := $(INCLUDES) -DDEFENSIVE -DDEBUG -DFATAL -DDEVELOP -DGEOMETRY -DDETAIL -DUNIT_TEST -DVERSION=\"$(VER)\ Develop\ Version\ \($(date)\)\"
unit_test_detail: BINARY := $(BINARY)_dev
unit_test_detail: OBJ := $(OBJ) unit_test.o
unit_test_detail: unit_test.o compile

.PHONY: geometry
geometry: CFLAGS := $(CFLAGS) -g
geometry: INCLUDES := $(INCLUDES) -DGEOMETRY -DDEBUG -DFATAL -DDEVELOP -DVERSION=\"$(VER)\ Develop\ Version\ \($(date)\)\"
geometry: BINARY := $(BINARY)_dev
geometry: compile

.PHONY: detail
detail: CFLAGS := $(CFLAGS) -g
detail: INCLUDES := $(INCLUDES) -DGEOMETRY -DDEBUG -DFATAL -DDEVELOP -DDETAIL -DVERSION=\"$(VER)\ Develop\ Version\ \($(date)\)\"
detail: BINARY := $(BINARY)_dev
detail: compile

.PHONY: full
full: CFLAGS := $(CFLAGS) -g
full: INCLUDES := $(INCLUDES) -DGEOMETRY -DDEBUG -DFATAL -DDEVELOP -DDETAIL -DFULL -DVERSION=\"$(VER)\ Develop\ Version\ \($(date)\)\"
full: BINARY := $(BINARY)_dev
full: compile

.PHONY: memory
memory: CFLAGS := $(CFLAGS) -g
memory: INCLUDES := $(INCLUDES) -DDEBUG -DFATAL -DDEVELOP -DDEFENSIVE -DOUT_OF_MEMORY -DVERSION=\"$(VER)\ Develop\ Version\ \($(date)\)\"
memory: BINARY := $(BINARY)_dev
memory: compile

.PHONY: clean
clean:
	-@ for FileName in $(BINARY) $(BINARY)_dev $(BINARY)_dbg *.o unit_test.c ../po/*.mo; \
	do \
		if [ -f "$$FileName" ]; then \
			echo "deleting $$FileName..."; \
			rm "$$FileName"; \
		fi; \
	done

.PHONY: data
data: debug
	xgettext --from-code=UTF-8 -k_ --add-comments='TRANSLATE NOTE:' -o ../po/lilyterm.pot *.c
	@ for FileName in $(PO_LIST); \
	do \
		echo "msgmerge $$FileName.po..."; \
		msgmerge --previous ../po/$$FileName.po ../po/lilyterm.pot -o ../po/$$FileName.po; \
		msgfmt --check --statistics ../po/$$FileName.po -o ../po/$$FileName.mo; \
	done
	./$(BINARY)_dbg -p > ../data/lilyterm.conf
	@ cd ../po; /usr/bin/intltool-update -m; cd -

menu.o: data.h lilyterm.h menu.h menu.c
	$(CC) $(CFLAGS) $(INCLUDES) -c menu.c `$(OBJ_PKG_CMD)`

font.o: data.h lilyterm.h font.h font.c
	$(CC) $(CFLAGS) $(INCLUDES) -c font.c `$(OBJ_PKG_CMD)`

dialog.o: data.h lilyterm.h dialog.h dialog.c
	$(CC) $(CFLAGS) $(INCLUDES) -c dialog.c `$(OBJ_PKG_CMD)`

profile.o: data.h lilyterm.h profile.h profile.c
	$(CC) $(CFLAGS) $(INCLUDES) -c profile.c `$(OBJ_PKG_CMD)`

pagename.o: data.h lilyterm.h pagename.h pagename.c
	$(CC) $(CFLAGS) $(INCLUDES) -c pagename.c `$(OBJ_PKG_CMD)`

property.o: data.h lilyterm.h property.h property.c
	$(CC) $(CFLAGS) $(INCLUDES) -c property.c `$(OBJ_PKG_CMD)`

notebook.o: data.h lilyterm.h notebook.h notebook.c
	$(CC) $(CFLAGS) $(INCLUDES) -c notebook.c `$(OBJ_PKG_CMD)`

window.o: data.h lilyterm.h window.h window.c
	$(CC) $(CFLAGS) $(INCLUDES) -c window.c `$(OBJ_PKG_CMD)`

misc.o: data.h lilyterm.h misc.h misc.c
	$(CC) $(CFLAGS) $(INCLUDES) -c misc.c `$(OBJ_PKG_CMD)`

console.o: data.h lilyterm.h console.h console.c
	$(CC) $(CFLAGS) $(INCLUDES) -c console.c `$(OBJ_PKG_CMD)`

main.o: data.h lilyterm.h main.h main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c main.c `$(OBJ_PKG_CMD)`

unit_test.c : unit_test.sh
	sh unit_test.sh > unit_test.c

unit_test.o: console.h data.h dialog.h font.h lilyterm.h main.h menu.h misc.h notebook.h pagename.h profile.h property.h window.h unit_test.c
	$(CC) $(CFLAGS) $(INCLUDES) -c unit_test.c `$(OBJ_PKG_CMD)`

.PHONY: compile
compile: $(OBJ)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $(BINARY) $(OBJ) `$(BINARY_PKG_CMD)`
