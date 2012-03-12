include ../.default
-include ../.config

LANG_LIST ?= de fr es it nl pl ru sk tr uk zh_CN zh_TW
PO_LIST ?= $(LANG_LIST:=.po)
MO_LIST ?= $(LANG_LIST:=.mo)

.PHONY: all
all: pot $(PO_LIST) $(MO_LIST)

.PHONY: pot
pot:
	@ cd ../src ; \
	xgettext --from-code=UTF-8 -k_ \
		 --copyright-holder="$(YEAR) FULL NAME <EMAIL@ADDRESS>" \
		 --package-name=$(PACKAGE) \
		 --package-version="$(VER)" \
		 --add-comments='TRANSLATE NOTE:' -o ../po/$(BINARY).po *.c
	@#  echo '' >> $(BINARY).po
	@#  echo 'msgid "A light and easy to use libvte based X Terminal Emulator"' >> $(BINARY).po
	@#  echo 'msgstr ""' >> $(BINARY).po
	@ if (msgcmp --use-untranslated $(BINARY).pot $(BINARY).po) then \
		rm $(BINARY).po ; \
	else \
		$(PRINTF) "\x1b\x5b1;36m** creating $(BINARY).pot...\x1b\x5b0m\n" ; \
		mv $(BINARY).po $(BINARY).pot ; \
	fi

$(BINARY).pot: pot

.PHONY: po
po: $(PO_LIST)

%.po: $(BINARY).pot
	@ $(PRINTF) "\x1b\x5b1;33m** msgmerge $@...\x1b\x5b0m\n"
	-msgmerge --previous $@ $< -o $@

%.mo: %.po $(BINARY).pot
	@ $(PRINTF) "\x1b\x5b1;33m** creating $@...\x1b\x5b0m\n"
	-msgfmt --check --statistics $< -o $@

.PHONY: clean
clean:
	@ for FILE in $(MO_LIST); do \
		if [ -f $$FILE ]; then \
			$(PRINTF) "\x1b\x5b1;35m** deleting $$FILE...\x1b\x5b0m\n" ; \
			rm $$FILE ; \
		fi ; \
	done

.PHONY: install
install: all
	@ install -d $(DESTDIR)/$(LOCALEDIR)
	@ for FILE in $(MO_LIST); do \
		if [ -f $$FILE ]; then \
			$(PRINTF) "\x1b\x5b1;33m** installing $$FILE to $(DESTDIR)/$(LOCALEDIR)...\x1b\x5b0m\n" ; \
			install -m 644 $$FILE $(DESTDIR)/$(LOCALEDIR) ; \
		fi ; \
	done

.PHONY: uninstall
uninstall:
	@ - for FILE in $(MO_LIST); do \
		if [ -f $(DESTDIR)/$(LOCALEDIR)/$$FILE ]; then \
			$(PRINTF) "\x1b\x5b1;35m** deleting $(DESTDIR)/$(LOCALEDIR)/$$FILE...\x1b\x5b0m\n" ; \
			rm "$(DESTDIR)/$(LOCALEDIR)/$$FILE" ; \
			DirName="$(DESTDIR)/$(LOCALEDIR)" ; \
			until [ -n "`ls -A $$DirName`" -o "$$DirName" = "$(DESTDIR)/$(PREFIX)" ]; do \
				$(PRINTF) "\x1b\x5b1;35m** deleting $$DirName...\x1b\x5b0m\n" ; \
				rmdir "$$DirName" ; \
				DirName=`dirname $$DirName` ; \
			done \
		else \
			$(PRINTF) "\x1b\x5b1;31m** $(DESTDIR)/$(LOCALEDIR)/$$FILE is NOT exist!\x1b\x5b0m\n" ; \
		fi ; \
	done
