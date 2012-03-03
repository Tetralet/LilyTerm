include .default
-include .config

DIR = src data

ECHO = $(shell whereis -b echo | awk '{print $$2}')

ifeq ($(NLS), Y)
DIR += po
endif

.PHONY: all
all: $(DIR)

.PHONY: src
src:
	@ $(MAKE) -C src

.PHONY: data
data:
	@ $(MAKE) -C data

.PHONY: po
po:
	@ $(MAKE) -C po

clean:
	@ $(MAKE) -C src clean
	@ $(MAKE) -C data clean
ifeq ($(NLS), Y)
	@ $(MAKE) -C po clean
endif

install: all
	@ $(MAKE) -C src install
	@ $(MAKE) -C data install
ifeq ($(NLS), Y)
	@ $(MAKE) -C po install
endif

uninstall:
	@ $(MAKE) -C src uninstall
	@ $(MAKE) -C data uninstall
ifeq ($(NLS), Y)
	@ $(MAKE) -C po uninstall
endif
	@ if [ -z "`ls -A "$(DESTDIR)/$(PREFIX)"`" ]; then \
		$(ECHO) "===========================================" ; \
		$(ECHO) -e "\x1b[1;31m** WARNING: \"$(DESTDIR)/$(PREFIX)\" is empty. Please remove it manually if necessary.\x1b[0m" ; \
		$(ECHO) "===========================================" ; \
	fi
distclean: clean
	@ if [ -f .config ]; then \
		$(ECHO) -e "\x1b[1;35m** deleting .config...\x1b[0m" ; \
		rm .config ; \
	fi

.config: configure
	@ ./configure > /dev/null
