INCLUDES = -DICONDIR=\"$(datadir)/pixmaps\"

bin_PROGRAMS = lilyterm
lilyterm_CFLAGS  = $(GTK_CFLAGS) $(vte_CFLAGS)
lilyterm_LDADD   = $(GTK_LIBS) $(vte_LIBS) $(INTLLIBS)
lilyterm_SOURCES = data.h lilyterm.h \
		   misc.h misc.c \
		   console.h console.c \
		   menu.h menu.c \
		   profile.h profile.c \
		   property.h property.c \
		   dialog.h dialog.c \
		   pagename.h pagename.c \
		   notebook.h notebook.c \
		   font.h font.c \
		   window.h window.c \
		   main.h main.c

