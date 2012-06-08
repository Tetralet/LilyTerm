include ../.default
-include ../.config

PO_LIST ?= $(LANG_LIST:=.po)
MO_LIST ?= $(LANG_LIST:=.mo)

XGETTEXT := $(shell whereis -b xgettext | awk '{print $$2}')
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

MSGCMP := $(shell whereis -b msgcmp | awk '{print $$2}')
ifneq ($(MSGCMP), )

SUPPORT_USE_UNTRANSLATED = $(shell $(MSGCMP) --help | grep -q -- '--use-untranslated' && echo Y)
ifeq ($(SUPPORT_USE_UNTRANSLATED), Y)
	MSGCMP_PARAMETER += --use-untranslated
endif

endif

MSGMERGE := $(shell whereis -b msgmerge | awk '{print $$2}')
ifneq ($(MSGMERGE), )

SUPPORT_PREVIOUS = $(shell $(MSGMERGE) --help | grep -q -- '--previous' && echo Y)
ifeq ($(SUPPORT_PREVIOUS), Y)
	MSGMERGE_PARAMETER += --previous
endif

endif

MSGFMT := $(shell whereis -b msgfmt | awk '{print $$2}')
TOUCH := $(shell whereis -b touch | awk '{print $$2}')

.PHONY: all
all: $(MO_LIST)

.PHONY: update-po
update-po: pot $(PO_LIST)

.PHONY: pot
pot:
ifeq ($(XGETTEXT), )
	@ $(PRINTF) "\033[1;31mERROR: 'xgettext' command can NOT found!\033[0m\n"
	@ $(PRINTF) "\033[1;31mERROR: gettext package is needed to make pot. ABORT!\033[0m\n"
	@ exit 1
endif
ifeq ($(MSGCMP), )
	@ $(PRINTF) "\033[1;31mERROR: 'msgcmp' command can NOT found!\033[0m\n"
	@ $(PRINTF) "\033[1;31mERROR: gettext package is needed to make pot. ABORT!\033[0m\n"
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
		$(PRINTF) "\033[1;36m** creating $(BINARY).pot...\033[0m\n" ; \
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
	@ $(PRINTF) "\033[1;31mERROR: 'msgmerge' command can NOT found!\033[0m\n"
	@ $(PRINTF) "\033[1;31mERROR: gettext package is needed to make po files. ABORT!\033[0m\n"
	@ exit 1
endif
	@ $(PRINTF) "\033[1;33m** msgmerge $@...\033[0m\n"
	-@ $(MSGMERGE) $(MSGMERGE_PARAMETER) $@ $< -o $@

%.mo: %.po $(BINARY).pot
ifeq ($(MSGFMT), )
	@ $(PRINTF) "\033[1;31mERROR: 'msgfmt' command can NOT found!\033[0m\n"
	@ $(PRINTF) "\033[1;31mERROR: gettext package is needed to make mo files. ABORT!\033[0m\n"
	@ exit 1
endif
	@ $(PRINTF) "\033[1;33m** creating $@...\033[0m\n"
	-@ $(MSGFMT) --check --statistics $< -o $@

.PHONY: clean
clean:
	@ for FILE in $(MO_LIST); do \
		if [ -f $$FILE ]; then \
			$(PRINTF) "\033[1;35m** deleting $$FILE...\033[0m\n" ; \
			rm $$FILE ; \
		fi ; \
	done

.PHONY: install
install: all
	@ for FILE in $(LANG_LIST); do \
		if [ -f $$FILE.mo ]; then \
			install -d $(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/ ; \
			$(PRINTF) "\033[1;33m** installing $$FILE.mo to $(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/...\033[0m\n" ; \
			install -m 644 $$FILE.mo $(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/$(BINARY).mo ; \
		fi ; \
	done

.PHONY: uninstall
uninstall:
	@ - for FILE in $(LANG_LIST); do \
		if [ -f $(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/$(BINARY).mo ]; then \
			$(PRINTF) "\033[1;35m** deleting $(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/$(BINARY).mo...\033[0m\n" ; \
			rm "$(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/$(BINARY).mo" ; \
			DirName="$(DESTDIR)/$(LOCALEDIR)/$$FILE/LC_MESSAGES/" ; \
			until [ -n "`ls -A $$DirName`" -o "$$DirName" = "$(DESTDIR)/$(PREFIX)" ]; do \
				$(PRINTF) "\033[1;35m** deleting $$DirName...\033[0m\n" ; \
				rmdir "$$DirName" ; \
				DirName=`dirname $$DirName` ; \
			done \
		else \
			$(PRINTF) "\033[1;31m** $(DESTDIR)/$(LOCALEDIR)/$$FILE is NOT exist!\033[0m\n" ; \
		fi ; \
	done
