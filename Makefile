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

distclean: clean
	@ rm .config

.config: configure
	@ ./configure > /dev/null
