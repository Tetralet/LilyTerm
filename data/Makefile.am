man_MANS=$(PACKAGE).1

etcdir = $(sysconfdir)/xdg
etc_DATA = $(PACKAGE).conf

desktopdir = $(datadir)/applications
desktop_DATA = $(PACKAGE).desktop

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = $(PACKAGE).png $(PACKAGE).xpm

examplesdir = $(datadir)/doc/$(PACKAGE)/examples
examples_DATA = $(PACKAGE).conf

EXTRA_DIST = $(etc_DATA) $(desktop_DATA) $(pixmaps_DATA) $(examples_DATA)


xml_in_files = $(PACKAGE).xml.in

if DEFAULT_APPS_DEFINITION
xmldir       = $(DEFAULTAPPS_DIR)
xml_DATA     = $(xml_in_files:.xml.in=.xml)
endif

@INTLTOOL_XML_RULE@
