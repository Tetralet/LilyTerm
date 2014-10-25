include .default
-include .config

DIR = src data

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
	@ $(MAKE) -C po clean

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
		$(PRINTF) "\033[1;31m** WARNING: \"\033[1;34m$(DESTDIR)/$(PREFIX)\033[1;31m\" is empty. Please remove it manually if necessary.\033[0m\n" ; \
		$(ECHO) "===========================================" ; \
	fi
distclean: clean
	@ if [ -f .config ]; then \
		$(PRINTF) "\033[1;35m** deleting \033[1;32m.config\033[1;35m ...\033[0m\n" ; \
		rm .config ; \
	fi
