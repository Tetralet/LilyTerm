include ../.default
-include ../.config

ETCDIR ?= /etc
MANDIR ?= $(DATADIR)/man
DESKTOP_DIR ?= $(DATADIR)/applications
EXAMPLES_DIR ?= $(DATADIR)/doc/$(BINARY)/examples
GNOME_CONTROL_CENTER ?= Y
GNOME_CONTROL_CENTER_DIR ?= $(DATADIR)/gnome-control-center/default-apps

MAIN_BINARY := $(shell if [ -f ../src/$(BINARY)_dev ]; then echo $(BINARY)_dev; fi)
MAIN_BINARY := $(shell if [ -f ../src/$(BINARY)_dbg ]; then echo $(BINARY)_dbg; fi)
ifeq ($(MAIN_BINARY), )
MAIN_BINARY = $(BINARY)
endif

FileList = $(DESTDIR)/$(ETCDIR)/$(BINARY).conf \
	   $(DESTDIR)/$(DOCDIR)/AUTHORS \
	   $(DESTDIR)/$(DOCDIR)/COPYING \
	   $(DESTDIR)/$(DOCDIR)/ChangeLog \
	   $(DESTDIR)/$(MANDIR)/$(BINARY).1.gz \
	   $(DESTDIR)/$(DESKTOP_DIR)/$(BINARY).desktop \
	   $(DESTDIR)/$(ICONDIR)/$(BINARY).png \
	   $(DESTDIR)/$(ICONDIR)/$(BINARY).xpm \
	   $(DESTDIR)/$(EXAMPLES_DIR)/$(BINARY).conf
ifeq ($(GNOME_CONTROL_CENTER), Y)
ifneq ($(GNOME_CONTROL_CENTER_DIR), )
FileList += $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)/$(BINARY).xml
endif
endif

.PHONY: all
all: $(BINARY).conf $(BINARY).1.gz

$(BINARY).conf:
	@ if [ ! -f ../src/$(BINARY) -a \
	     ! -f ../src/$(BINARY)_dbg -a \
	     ! -f ../src/$(BINARY)_dev ]; then \
		cd ../src ; \
		$(MAKE) ; \
	fi
	@ $(PRINTF) "\x1b\x5b1;33m** creating $(BINARY).conf...\x1b\x5b0m\n"
	@ ../src/$(MAIN_BINARY) -p > $(BINARY).conf

$(BINARY).1.gz:
	@ if [ -f $(BINARY).1 ]; then \
		$(PRINTF) "\x1b\x5b1;33m** adding $(BINARY).1...\x1b\x5b0m\n" ; \
		gzip $(BINARY).1 ; \
	fi

.PHONY: clean
clean:
	@ if [ -f $(BINARY).conf ]; then \
		$(PRINTF) "\x1b\x5b1;35m** deleteing $(BINARY).conf...\x1b\x5b0m\n" ; \
		rm $(BINARY).conf ; \
	fi
	@ if [ -f $(BINARY).1.gz ]; then \
		$(PRINTF) "\x1b\x5b1;35m** unziping $(BINARY).1...\x1b\x5b0m\n" ; \
		gzip -d  $(BINARY).1.gz ; \
	fi

.PHONY: install
install: all
	@ $(PRINTF) "\x1b\x5b1;33m** installing $(BINARY).conf to $(DESTDIR)/$(ETCDIR)...\x1b\x5b0m\n"
	@ install -d $(DESTDIR)/$(ETCDIR)
	@ install -m 644 $(BINARY).conf $(DESTDIR)/$(ETCDIR)

	@ $(PRINTF) "\x1b\x5b1;33m** installing AUTHORS COPYING ChangeLog to $(DESTDIR)/$(DOCDIR)...\x1b\x5b0m\n"
	@ install -d $(DESTDIR)/$(DOCDIR)
	@ install -m 644 ../AUTHORS ../COPYING ../ChangeLog $(DESTDIR)/$(DOCDIR)

	@ $(PRINTF) "\x1b\x5b1;33m** installing $(BINARY).1.gz to $(DESTDIR)/$(MANDIR)...\x1b\x5b0m\n"
	@ install -d $(DESTDIR)/$(MANDIR)
	@ install -m 644 $(BINARY).1.gz $(DESTDIR)/$(MANDIR)

	@ $(PRINTF) "\x1b\x5b1;33m** installing $(BINARY).desktop to $(DESTDIR)/$(DESKTOP_DIR)...\x1b\x5b0m\n"
	@ install -d $(DESTDIR)/$(DESKTOP_DIR)
	@ install -m 644  $(BINARY).desktop $(DESTDIR)/$(DESKTOP_DIR)

	@ $(PRINTF) "\x1b\x5b1;33m** installing $(BINARY).png $(BINARY).xpm to $(DESTDIR)/$(ICONDIR)...\x1b\x5b0m\n"
	@ install -d $(DESTDIR)/$(ICONDIR)
	@ install -m 644 $(BINARY).png $(BINARY).xpm $(DESTDIR)/$(ICONDIR)

	@ $(PRINTF) "\x1b\x5b1;33m** installing $(BINARY).conf to $(DESTDIR)/$(EXAMPLES_DIR)...\x1b\x5b0m\n"
	@ install -d $(DESTDIR)/$(EXAMPLES_DIR)
	@ install -m 644 $(BINARY).conf $(DESTDIR)/$(EXAMPLES_DIR)

ifeq ($(GNOME_CONTROL_CENTER), Y)
    ifneq ($(GNOME_CONTROL_CENTER_DIR), )
	@ $(PRINTF) "\x1b\x5b1;33m** installing $(BINARY).xml to $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)...\x1b\x5b0m\n"
	@ install -d $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)
	@ install -m 644 $(BINARY).xml $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)
    endif
endif

.PHONY: uninstall
uninstall:
	@ - for FileName in $(FileList) ; do \
		if [ -f "$$FileName" ]; then \
			$(PRINTF) "\x1b\x5b1;35m** deleting $$FileName...\x1b\x5b0m\n" ; \
			rm -f "$$FileName" ; \
			DirName=`dirname $$FileName` ; \
			until [ -n "`ls -A $$DirName`" -o "$$DirName" = "$(DESTDIR)/$(PREFIX)" ]; do \
				$(PRINTF) "\x1b\x5b1;35m** deleting $$DirName...\x1b\x5b0m\n" ; \
				rmdir "$$DirName" ; \
				DirName=`dirname $$DirName` ; \
			done \
		else \
			$(PRINTF) "\x1b\x5b1;31m** $$FileName is NOT exist!\x1b\x5b0m\n"; \
		fi ; \
	done
