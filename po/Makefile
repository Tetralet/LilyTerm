include ../.default
-include ../.config

ifeq ($(VERBOSITY), Y)
	VERBOSITY_OUTPUT =
else
	VERBOSITY_OUTPUT = @
endif


PO_LIST ?= $(LANG_LIST:=.po)
MO_LIST ?= $(LANG_LIST:=.mo)

XGETTEXT := $(shell whereis "xgettext" | tr -s ' ' '\n' | grep "bin/""xgettext""$$" | head -n 1)
ifneq ($(XGETTEXT), )

XGETTEXT_PARAMETER = -k_

SUPPORT_FROM_CODE = $(shell $(XGETTEXT) --help | grep -q -- '--from-code' && echo Y)
ifeq ($(SUPPORT_FROM_CODE), Y)
	XGETTEXT_PARAMETER += --from-code=UTF-8
endif

SUPPORT_COPYRIGHT_HOLDER = $(shell $(XGETTEXT) --help | grep -q -- '--copyright-holder' && echo Y)
ifeq ($(SUPPORT_COPYRIGHT_HOLDER), Y)
	XGETTEXT_PARAMETER += --copyright-holder="$(YEAR) FULL NAME <EMAIL@ADDRESS>"
endif

SUPPORT_PACKAGE_NAME = $(shell $(XGETTEXT) --help | grep -q -- '--package-name' && echo Y)
ifeq ($(SUPPORT_PACKAGE_NAME), Y)
	XGETTEXT_PARAMETER += --package-name=$(PACKAGE)
endif

SUPPORT_PACKAGE_VERSION = $(shell $(XGETTEXT) --help | grep -q -- '--package-version' && echo Y)
ifeq ($(SUPPORT_PACKAGE_VERSION), Y)
	XGETTEXT_PARAMETER += --package-version="$(VER)"
endif

endif

MSGCMP := $(shell whereis "msgcmp" | tr -s ' ' '\n' | grep "bin/""msgcmp""$$" | head -n 1)
ifneq ($(MSGCMP), )

SUPPORT_USE_UNTRANSLATED = $(shell $(MSGCMP) --help | grep -q -- '--use-untranslated' && echo Y)
ifeq ($(SUPPORT_USE_UNTRANSLATED), Y)
	MSGCMP_PARAMETER += --use-untranslated
endif

endif

MSGMERGE := $(shell whereis "msgmerge" | tr -s ' ' '\n' | grep "bin/""msgmerge""$$" | head -n 1)
ifneq ($(MSGMERGE), )

SUPPORT_PREVIOUS = $(shell $(MSGMERGE) --help | grep -q -- '--previous' && echo Y)
ifeq ($(SUPPORT_PREVIOUS), Y)
	MSGMERGE_PARAMETER += --previous
endif

endif

MSGFMT := $(shell whereis "msgfmt" | tr -s ' ' '\n' | grep "bin/""msgfmt""$$" | head -n 1)
TOUCH := $(shell whereis "touch" | tr -s ' ' '\n' | grep "bin/""touch""$$" | head -n 1)

.PHONY: all
all: $(MO_LIST)

.PHONY: update-po
update-po: pot $(PO_LIST)

.PHONY: pot
pot:
ifeq ($(XGETTEXT), )
	@ $(PRINTF) "\033[1;31mERROR: '\033[1;32mxgettext\033[1;31m' command can NOT found!\033[0m\n"
	@ $(PRINTF) "\033[1;31mERROR: \033[1;32mgettext\033[1;31m package is needed to make pot. ABORT!\033[0m\n"
	@ exit 1
endif
ifeq ($(MSGCMP), )
	@ $(PRINTF) "\033[1;31mERROR: '\033[1;32mmsgcmp\033[1;31m' command can NOT found!\033[0m\n"
	@ $(PRINTF) "\033[1;31mERROR: \033[1;32mgettext\033[1;31m package is needed to make pot. ABORT!\033[0m\n"
	@ exit 1
endif
	@ cd ../src ; \
	$(XGETTEXT) $(XGETTEXT_PARAMETER) --add-comments='TRANSLATE NOTE:' -o ../po/$(BINARY).po *.c
	@#  echo '' >> $(BINARY).po
	@#  echo 'msgid "A light and easy to use libvte based X Terminal Emulator"' >> $(BINARY).po
	@#  echo 'msgstr ""' >> $(BINARY).po
	@ if ($(MSGCMP) $(MSGCMP_PARAMETER) $(BINARY).pot $(BINARY).po > /dev/null 2>&1) then \
		rm $(BINARY).po ; \
	else \
		$(PRINTF) "\033[1;36m** creating \033[1;32m$(BINARY).pot\033[1;36m ...\033[0m\n" ; \
		mv $(BINARY).po $(BINARY).pot ; \
		echo "" ; \
	fi

$(BINARY).pot: pot

.PHONY: reset
reset:
	@ $(TOUCH) $(BINARY).pot

.PHONY: po
po: $(PO_LIST)

%.po: $(BINARY).pot
ifeq ($(MSGMERGE), )
	@ $(PRINTF) "\033[1;31mERROR: '\033[1;32mmsgmerge\033[1;31m' command can NOT found!\033[0m\n"
	@ $(PRINTF) "\033[1;31mERROR: \033[1;32mgettext\033[1;31m package is needed to make po files. ABORT!\033[0m\n"
	@ exit 1
endif
	@ $(PRINTF) "\033[1;33m** msgmerge \033[1;32m$@\033[1;33m ...\033[0m\n"
	-$(VERBOSITY_OUTPUT) $(MSGMERGE) $(MSGMERGE_PARAMETER) $@ $< -o $@

%.mo: %.po $(BINARY).pot
ifeq ($(MSGFMT), )
	@ $(PRINTF) "\033[1;31mERROR: '\033[1;32mmsgfmt\033[1;31m' command can NOT found!\033[0m\n"
	@ $(PRINTF) "\033[1;31mERROR: \033[1;32mgettext\033[1;31m package is needed to make mo files. ABORT!\033[0m\n"
	@ exit 1
endif
	@ $(PRINTF) "\033[1;33m** creating \033[1;32m$@\033[1;33m ...\033[0m\n"
	-$(VERBOSITY_OUTPUT) $(MSGFMT) --check --statistics $< -o $@

.PHONY: clean
clean:
	@ for FILE in $(MO_LIST); do \
		if [ -f $$FILE ]; then \
			$(PRINTF) "\033[1;35m** deleting \033[1;32m$$FILE\033[1;35m...\033[0m\n" ; \
			rm $$FILE ; \
		fi ; \
	done

.PHONY: install
install: all
	@ for FILE in $(LANG_LIST); do \
		if [ -f $$FILE.mo ]; then \
			install -d $(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/ ; \
			$(PRINTF) "\033[1;33m** installing \033[1;32m$$FILE.mo\033[1;33m to \033[1;34m$(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/\033[1;32m ...\033[0m\n" ; \
			install -m 644 $$FILE.mo $(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/$(BINARY).mo ; \
		fi ; \
	done

.PHONY: uninstall
uninstall:
	@ - for FILE in $(LANG_LIST); do \
		if [ -f $(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/$(BINARY).mo ]; then \
			$(PRINTF) "\033[1;35m** \033[1;32mdeleting $(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/$(BINARY).mo\033[1;35m ...\033[0m\n" ; \
			rm "$(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/$(BINARY).mo" ; \
			DirName="$(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/" ; \
			until [ -n "`ls -A $$DirName`" -o "$$DirName" = "$(DESTDIR)/$(PREFIX)" ]; do \
				$(PRINTF) "\033[1;35m** deleting directory \033[1;34m$$DirName\033[1;35m ...\033[0m\n" ; \
				rmdir "$$DirName" ; \
				DirName=`dirname $$DirName` ; \
			done \
		else \
			$(PRINTF) "\033[1;31m** \033[1;32m$(DESTDIR)/$(LOCALEDIR)/$$FILE\033[1;31m is NOT exist!\033[0m\n" ; \
		fi ; \
	done
