include ../.default
-include ../.config

ifeq ($(VERBOSITY), Y)
	VERBOSITY_OUTPUT =
else
	VERBOSITY_OUTPUT = @
endif

DESKTOP_DIR ?= $(DATADIR)/applications
GNOME_CONTROL_CENTER ?= Y
GNOME_CONTROL_CENTER_DIR ?= $(DATADIR)/gnome-control-center/default-apps

MAIN_BINARY := $(shell if [ -f ../src/$(BINARY)_dev ]; then echo $(BINARY)_dev; fi)
MAIN_BINARY := $(shell if [ -f ../src/$(BINARY)_dbg ]; then echo $(BINARY)_dbg; fi)
ifeq ($(MAIN_BINARY), )
MAIN_BINARY = $(BINARY)
endif

RomveFileList = $(DESTDIR)/$(ETCDIR)/$(BINARY).conf \
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
RomveFileList += $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)/$(BINARY).xml
endif
endif

.PHONY: all
all: $(BINARY).conf $(BINARY).1.gz

$(BINARY).conf:
	# @ if [ ! -f ../src/$(BINARY) -a \
	#     ! -f ../src/$(BINARY)_dbg -a \
	#     ! -f ../src/$(BINARY)_dev ]; then \
	#	cd ../src ; \
	#	$(MAKE) ; \
	# fi
	# @ $(PRINTF) "\033[1;33m** creating $(BINARY).conf...\033[0m\n"
	# @ ../src/$(MAIN_BINARY) -p > $(BINARY).conf

$(BINARY).1.gz:
	@ if [ -f $(BINARY).1 ]; then \
		$(PRINTF) "\033[1;33m** adding \033[1;32m$(BINARY).1\033[1;33m ...\033[0m\n" ; \
		gzip $(BINARY).1 ; \
	fi

.PHONY: clean
clean:
	@ # @ if [ -f $(BINARY).conf ]; then \
	#	$(PRINTF) "\033[1;35m** deleteing $(BINARY).conf...\033[0m\n" ; \
	#	rm $(BINARY).conf ; \
	# fi
	@ if [ -f $(BINARY).1.gz ]; then \
		$(PRINTF) "\033[1;35m** unziping \033[1;32m$(BINARY).1\033[1;35m ...\033[0m\n" ; \
		gzip -d  $(BINARY).1.gz ; \
	fi

.PHONY: install
install: all
	@ $(PRINTF) "\033[1;33m** installing \033[1;32m$(BINARY).conf\033[1;33m to \033[1;34m$(DESTDIR)/$(ETCDIR)\033[1;33m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) install -d $(DESTDIR)/$(ETCDIR)
	$(VERBOSITY_OUTPUT) install -m 644 $(BINARY).conf $(DESTDIR)/$(ETCDIR)

	@ $(PRINTF) "\033[1;33m** installing \033[1;32mAUTHORS COPYING ChangeLog\033[1;33m to \033[1;34m$(DESTDIR)/$(DOCDIR)\033[1;33m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) install -d $(DESTDIR)/$(DOCDIR)
	$(VERBOSITY_OUTPUT) install -m 644 ../AUTHORS ../COPYING ../ChangeLog $(DESTDIR)/$(DOCDIR)

	@ $(PRINTF) "\033[1;33m** installing \033[1;32m$(BINARY).1.gz\033[1;33m to \033[1;34m$(DESTDIR)/$(MANDIR)\033[1;33m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) install -d $(DESTDIR)/$(MANDIR)
	$(VERBOSITY_OUTPUT) install -m 644 $(BINARY).1.gz $(DESTDIR)/$(MANDIR)

	@ $(PRINTF) "\033[1;33m** installing \033[1;32m$(BINARY).desktop\033[1;33m to \033[1;34m$(DESTDIR)/$(DESKTOP_DIR)\033[1;33m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) install -d $(DESTDIR)/$(DESKTOP_DIR)
	$(VERBOSITY_OUTPUT) install -m 644  $(BINARY).desktop $(DESTDIR)/$(DESKTOP_DIR)

	@ $(PRINTF) "\033[1;33m** installing \033[1;32m$(BINARY).png $(BINARY).xpm\033[1;33m to \033[1;34m$(DESTDIR)/$(ICONDIR)\033[1;33m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) install -d $(DESTDIR)/$(ICONDIR)
	$(VERBOSITY_OUTPUT) install -m 644 $(BINARY).png $(BINARY).xpm $(DESTDIR)/$(ICONDIR)

	@ $(PRINTF) "\033[1;33m** installing \033[1;32m$(BINARY).conf\033[1;33m to \033[1;34m$(DESTDIR)/$(EXAMPLES_DIR)\033[1;33m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) install -d $(DESTDIR)/$(EXAMPLES_DIR)
	$(VERBOSITY_OUTPUT) install -m 644 $(BINARY).conf $(DESTDIR)/$(EXAMPLES_DIR)

ifeq ($(GNOME_CONTROL_CENTER), Y)
    ifneq ($(GNOME_CONTROL_CENTER_DIR), )
	@ $(PRINTF) "\033[1;33m** installing \033[1;32m$(BINARY).xml\033[1;33m to \033[1;34m$(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)\033[1;33m ...\033[0m\n"
	$(VERBOSITY_OUTPUT) install -d $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)
	$(VERBOSITY_OUTPUT) install -m 644 $(BINARY).xml $(DESTDIR)/$(GNOME_CONTROL_CENTER_DIR)
    endif
endif

.PHONY: uninstall
uninstall:
	@ - for FileName in $(RomveFileList) ; do \
		if [ -f "$$FileName" ]; then \
			$(PRINTF) "\033[1;35m** deleting \033[1;32m$$FileName\033[1;35m ...\033[0m\n" ; \
			rm -f "$$FileName" ; \
			DirName=`dirname $$FileName` ; \
			until [ -n "`ls -A $$DirName`" -o "$$DirName" = "$(DESTDIR)/$(PREFIX)" ]; do \
				$(PRINTF) "\033[1;35m** deleting directory \033[1;34m$$DirName\033[1;35m ...\033[0m\n" ; \
				rmdir "$$DirName" ; \
				DirName=`dirname $$DirName` ; \
			done \
		else \
			$(PRINTF) "\033[1;31m** \033[1;32m$$FileName\033[1;31m is NOT exist!\033[0m\n"; \
		fi ; \
	done
