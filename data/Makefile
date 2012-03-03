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
	@ $(ECHO) -e "\x1b[1;33m** creating $(BINARY).conf...\x1b[0m"
	@ ../src/$(MAIN_BINARY) -p > $(BINARY).conf

$(BINARY).1.gz:
	@ if [ -f $(BINARY).1 ]; then \
		$(ECHO) -e "\x1b[1;33m** adding $(BINARY).1...\x1b[0m" ; \
		gzip $(BINARY).1 ; \
	fi

.PHONY: clean
clean:
	@ if [ -f $(BINARY).conf ]; then \
		$(ECHO) -e "\x1b[1;35m** deleteing $(BINARY).conf...\x1b[0m" ; \
		rm $(BINARY).conf ; \
	fi
	@ if [ -f $(BINARY).1.gz ]; then \
		$(ECHO) -e "\x1b[1;35m** unziping $(BINARY).1...\x1b[0m" ; \
		gzip -d  $(BINARY).1.gz ; \
	fi

.PHONY: install
install: all
	@ $(ECHO) -e "\x1b[1;33m** installing $(BINARY).conf to $(DESTDIR)/$(ETCDIR)...\x1b[0m"
	@ install -d $(DESTDIR)/$(ETCDIR)
	@ install -m 644 $(BINARY).conf $(DESTDIR)/$(ETCDIR)

	@ $(ECHO) -e "\x1b[1;33m** installing AUTHORS COPYING ChangeLog to $(DESTDIR)/$(DOCDIR)...\x1b[0m"
	@ install -d $(DESTDIR)/$(DOCDIR)
	@ install -m 644 ../AUTHORS ../COPYING ../ChangeLog $(DESTDIR)/$(DOCDIR)

	@ $(ECHO) -e "\x1b[1;33m** installing $(BINARY).1.gz to $(DESTDIR)/$(MANDIR)...\x1b[0m"
	@ install -d $(DESTDIR)/$(MANDIR)
	@ install -m 644 $(BINARY).1.gz $(DESTDIR)/$(MANDIR)

	@ $(ECHO) -e "\x1b[1;33m** installing $(BINARY).desktop to $(DESTDIR)/$(DESKTOP_DIR)...\x1b[0m"
	@ install -d $(DESTDIR)/$(DESKTOP_DIR)
	@ install -m 644  $(BINARY).desktop $(DESTDIR)/$(DESKTOP_DIR)

	@ $(ECHO) -e "\x1b[1;33m** installing $(BINARY).png $(BINARY).xpm to $(DESTDIR)/$(ICONDIR)...\x1b[0m"
	@ install -d $(DESTDIR)/$(ICONDIR)
	@ install -m 644 $(BINARY).png $(BINARY).xpm $(DESTDIR)/$(ICONDIR)

	@ $(ECHO) -e "\x1b[1;33m** installing $(BINARY).conf to $(DESTDIR)/$(EXAMPLES_DIR)...\x1b[0m"
	@ install -d $(DESTDIR)/$(EXAMPLES_DIR)
	@ install -m 644 $(BINARY).conf $(DESTDIR)/$(EXAMPLES_DIR)

ifeq ($(GNOME_CONTROL_CENTER), Y)
    ifneq ($(GNOME_CONTROL_CENTER_DIR), )
	@ $(ECHO) -e "\x1b[1;33m** installing $(BINARY).xml to $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)...\x1b[0m"
	@ install -d $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)
	@ install -m 644 $(BINARY).xml $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)
    endif
endif

.PHONY: uninstall
uninstall:
	@ - for FileName in $(FileList) ; do \
		if [ -f "$$FileName" ]; then \
			$(ECHO) -e "\x1b[1;35m** "deleting $$FileName..."\x1b[0m" ; \
			rm -f "$$FileName" ; \
			DirName=`dirname $$FileName` ; \
			until [ -n "`ls -A $$DirName`" -o "$$DirName" = "$(DESTDIR)/$(PREFIX)" ]; do \
				$(ECHO) -e "\x1b[1;35m** deleting $$DirName...\x1b[0m" ; \
				rmdir "$$DirName" ; \
				DirName=`dirname $$DirName` ; \
			done \
		else \
			$(ECHO) -e "\x1b[1;31m** "$$FileName is NOT exist!"\x1b[0m"; \
		fi ; \
	done
