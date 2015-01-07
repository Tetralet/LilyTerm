#
# Copyright (c) 2008-2014 Lu, Chao-Ming (Tetralet).  All rights reserved.
#
# This file is part of LilyTerm.
#
# LilyTerm is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# LilyTerm is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with LilyTerm.  If not, see <http://www.gnu.org/licenses/>.
#

#!/bin/sh

INCLUDES="$1"

RUN_GDB=0
RUN_VALGRIND=0
TEST_SCRIPT_ONLY=0
BUILD_ONLY=0
LIB_LISTS="lilyterm.h"
FUNCTION_FOUND=0

GDB_ERROR=0

ECHO=`whereis "echo" | tr -s ' ' '\n' | grep "bin/""echo""$" | head -n 1`
PRINTF=`whereis "printf" | tr -s ' ' '\n' | grep "bin/""printf""$" | head -n 1`
CAT=`whereis "cat" | tr -s ' ' '\n' | grep "bin/""cat""$" | head -n 1`
GDB=`whereis "gdb" | tr -s ' ' '\n' | grep "bin/""gdb""$" | head -n 1`
VALGRIND=`whereis "valgrind" | tr -s ' ' '\n' | grep "bin/""valgrind""$" | head -n 1`

MAKE=`whereis "gmake" | tr -s ' ' '\n' | grep "bin/""gmake""$" | head -n 1`
if [ -z "$MAKE" ]; then
	MAKE=`whereis "make" | tr -s ' ' '\n' | grep "bin/""make""$" | head -n 1`
fi

for opt do
	case "$opt" in
		-h|--help)
			$ECHO "Usage: sh $0 INCLUDES --test_all --enable-glib-debugger --enable-gtk-debugger --enable-gdb --enable-valgrind --specific_function=FUNCTION_NAME --skip_function=FUNCTION_NAME --create_program_only --build_program_only"
			exit 0
			;;
		--test_all)
			LIB_LISTS="*.h"
			export G_DEBUG=fatal_warnings
			;;
		--enable-glib-debugger)
			export G_DEBUG=fatal_warnings
			;;
		--enable-gtk-debugger)
			GTK_DEBUG="--g-fatal-warnings"
			;;
		--enable-gdb)
			RUN_GDB=1
			;;
		--enable-valgrind)
			RUN_VALGRIND=1
			;;
		--specific_function=*)
			SPECIFIC_FUNCTION=`$ECHO $opt | cut -d '=' -f 2`
			LIB_LISTS="*.h"
			;;
		--skip_function=*)
			SKIP_FUNCTION=`$ECHO $opt | cut -d '=' -f 2`
			;;
		--create_program_only)
			TEST_SCRIPT_ONLY=1
			;;
		--build_program_only)
			BUILD_ONLY=1
			;;
	esac
done

CHECK_INCLUDES=`$ECHO "$INCLUDES" | grep -- '-DUNIT_TEST'`
if [ -z "$CHECK_INCLUDES" ]; then
	if [ -f lilyterm -o -f lilyterm-dbg -o -f lilyterm_dev ]; then
		$MAKE clean
	fi
	$MAKE uto || exit 1
	INCLUDES="-DSAFEMODE -DDEBUG -DFATAL -DDEVELOP -DUNIT_TEST"
fi

PKGCONFIG=`whereis "pkg-config" | tr -s ' ' '\n' | grep "bin/""pkg-config""$" | head -n 1`
if [ -z "$PKGCONFIG" ]; then
	$PRINTF "\033[1;31m** ERROR: Command pkg-config is not found!\033[0m\n"
	exit 1
fi

VTE=`$PKGCONFIG --exists 'vte' && $ECHO 'vte'`
if [ "$VTE" = "vte" ]; then
  GTK=`$PKGCONFIG --exists 'gtk+-2.0' && $ECHO 'gtk+-2.0'`
  if [ "$GTK" != "gtk+-2.0" ]; then
    $PRINTF "\033[1;31m** ERROR: You need to install GTK+2 develop package first to run this unit test program!\033[0m\n"
    exit 1
  fi
else
  VTE=`$PKGCONFIG --exists 'vte-2.91' && $ECHO 'vte-2.91'`
  if [ "$VTE" = "vte-2.91" ]; then
    LDFLAGS='-lX11'
    GTK=`$PKGCONFIG --exists 'gtk+-3.0' && $ECHO 'gtk+-3.0'`
    if [ "$GTK" != "gtk+-3.0" ]; then
      $PRINTF "\033[1;31m** ERROR: You need to install GTK+3 develop package first to run this unit test program!\033[0m\n"
      exit 1
    fi
  else
    VTE=`$PKGCONFIG --exists 'vte-2.90' && $ECHO 'vte-2.90'`
    if [ "$VTE" = "vte-2.90" ]; then
      GTK=`$PKGCONFIG --exists 'gtk+-3.0' && $ECHO 'gtk+-3.0'`
      if [ "$GTK" != "gtk+-3.0" ]; then
        $PRINTF "\033[1;31m** ERROR: You need to install GTK+3 develop package first to run this unit test program!\033[0m\n"
        exit 1
      fi
    else
      exit 1
    fi
  fi
fi

if [ -z "$CC" ]; then
  CC=gcc
fi

if [ -z "$CFLAGS" ]; then
  CFLAGS="-Wall -Werror -O -g"
fi

OBJ="menu.o profile.o dialog.o pagename.o notebook.o font.o property.o window.o misc.o console.o main.o unit_test.o"

$CAT > gdb_batch << EOF
run
backtrace full
info registers
thread apply all backtrace
quit
EOF

if [ -f gdb.log ]; then
	$PRINTF "\n\033[1;36mDeleting gdb.log...\033[0m\n\n"
	rm -f gdb.log
fi
if [ -f valgrind.log ]; then
	$PRINTF "\n\033[1;36mDeleting valgrind.log...\033[0m\n\n"
	rm -f valgrind.log
fi

# sed '/^\/\*/,/ \*\/$/d': Delete [ /* blah ... blah */ ] (multi lines)
# sed -e 's/[ \t]*\/\*[ \t]*.*[ \t]*\*\///g': Delete [ /* blah ... blah */ ] (single line)
# sed -e 's/[ \t]*\/\/.*//g': Delete [ // blah ... blah ]
# sed -e '/[ \t]*#[ \t]*ifdef[ \t]*USE_GTK3_GEOMETRY_METHOD/,/#[ \t]*endif[ \t]*/d': clear #ifdef USE_GTK3_GEOMETRY_METHOD ... #endif
# sed -e '/^[ \t]*#.*/d' | tr -d '\n': Delete [ #include ]
# sed '/^[ \t]*typedef enum/,/}.*;[ \t]*$/d': Delete typedef enum
# tr -d '\n': Delete [ <NewLine> ]
# sed -e 's/[\t ][\t ]*/_SPACE_/g': convert <Tab> and <Space> to "_SAPCE_"
# sed -e 's/;/;\n/g': Add [ <NewLine> ] back
# sed -e 's/_SPACE_/ /g': convert "_SAPCE_" to <Space>
# sed -e '/[ \t]*struct.*{/,/.*}[ \t]*;/d': clear struct
# sed -e 's/ *\([)(,]\) */ \1 /g': Add <Space> to the start and end of )(,
# sed -e 's/[\t ][\t ]*/_SPACE_/g': convert <Tab> and <Space> to "_SAPCE_" again
# sed -e '/_SPACE_(_SPACE_)_SPACE_/d': clear something like [ blah ( )  ]

for DATA in `$CAT $LIB_LISTS | sed '/^\/\*/,/ \*\/$/d' | sed -e 's/[ \t]*\/\*[ \t]*.*[ \t]*\*\///g' | sed -e 's/[ \t]*\/\/.*//g' | sed -e '/^[ \t]*#.*/d' | sed '/^[ \t]*typedef.*;[ \t]*$/d' | sed '/^[ \t]*typedef enum/,/}.*;[ \t]*$/d' | tr -d '\n' | sed -e 's/[\t ][\t ]*/_SPACE_/g' | sed -e 's/;/;\n/g' | sed -e 's/_SPACE_/ /g' | sed -e '/[ \t]*struct.*{/,/.*}[ \t]*;/d' | sed -e 's/ *\([)(,]\) */ \1 /g' | sed -e 's/[\t ][\t ]*/_SPACE_/g' | sed -e '/_SPACE_(_SPACE_)_SPACE_/d'`; do

	if [ $FUNCTION_FOUND -eq 1 ]; then
		break
	fi

	if [ -n "$SKIP_FUNCTION" ]; then
		CHECK_STR="_SPACE_\**"$SKIP_FUNCTION"_SPACE_"
		CHECK_PROGRAM=`$ECHO "$DATA" | grep "$CHECK_STR"`
		if [ -n "$CHECK_PROGRAM" ]; then
			continue
		fi
	fi

	if [ -n "$SPECIFIC_FUNCTION" ]; then
		CHECK_STR="_SPACE_\**"$SPECIFIC_FUNCTION"_SPACE_"
		CHECK_PROGRAM=`$ECHO "$DATA" | grep "$CHECK_STR"`
		if [ -z "$CHECK_PROGRAM" ]; then
			continue
		else
			FUNCTION_FOUND=1
		fi
	fi
	MAX_STR=0
	MAX_VAR=-1
	unset FULL_FUNCTION

	# $ECHO "Got original data = $DATA"
	# sed -e 's/_SPACE_\([)(,]\)_SPACE_/ \1 /g': convert _SPACE_)(,_SPACE_ to " ) " " ( " or " , "
	# sed -e 's/ , / /g': convert " , " to " "
	# sed -e 's/const_SPACE_//g': clear const_SPACE_
	DATA_STR=`$ECHO $DATA | sed -e 's/_SPACE_\([)(,]\)/ \1/g' | sed -e 's/\([)(,]\)_SPACE_/\1 /g' | sed -e 's/ , / /g' | sed -e 's/const_SPACE_//g'`
	# $ECHO "GOT DATA_STR = $DATA_STR" 1>&2
	START=0
	END=0
	VAR=-1

	for STR in $DATA_STR; do
		# $ECHO "GOT and Checking STR='$STR'"
		case $START in
			0)
				if [ "$STR" = "(" ]; then
					START=1
					FUNCTION="("
					# $ECHO "GOT FUNCTION = $FUNCTION"
				else
					# Got the function name.
					unset FUNC_STAR
					unset FUNC_END
					unset SPACE
					# sed -e 's/_SPACE_/ /g' : convert "_SAPCE_" to <Space>
					# sed -e 's/^ *//g': clear the leading <Space>
					# sed -e 's/ *$//g': clear the end <Space>
					# sed -e 's/^.* \**\([^ ]*\)/\1/g': clear the declare, like "gchar **"
					FUNC_NAME=`$ECHO $STR | sed -e 's/_SPACE_/ /g' | sed -e 's/^ *//g' | sed -e 's/ *$//g' | sed -e 's/^.* \**\([^ ]*\)/\1/g'`
					# $ECHO "GOT FUNC_NAME = $FUNC_NAME"
					$PRINTF "\033[1;36m$FUNC_NAME(): \033[1;33m** Createing unit_test.c...\033[0m\n"
				fi
				;;
			1)
				if [ "$STR" = ")" ]; then
					FUNCTION=`$ECHO $FUNCTION | sed -e 's/,*$/)/g'`
					START=2
					continue
				fi
				# $ECHO "CHECKING: GOT FUNCTION = $FUNCTION"
				# $ECHO "Testing $STR..." 1>&2

				# sed -e 's/_SPACE_/ /g': convert "_SAPCE_" to <Space>
				# sed -e 's/^\(.* \**\) *\([^][ ]*\)\([[]*\)[^][ ]*\([]]*\)/\1\3\4/g': convert "gchar *profile[A]" to gchar *[]
				# sed -e 's/^ *//g': clear the leading <Space>
				# sed -e 's/ *$//g': clear the end <Space>
				# sed -e 's/^.* \**\([^ ]*\)/\1/g': clear the variable
				STR=`$ECHO $STR | sed -e 's/_SPACE_/ /g' | sed -e 's/^\(.* \**\) *\([^][ ]*\)\([[]*\)[^][ ]*\([]]*\)/\1\3\4/g' | sed -e 's/^ *//g' | sed -e 's/ *$//g' | sed -e 's/ *\*/*/g'`
				# $ECHO "GOT and Testing \"$STR\"..." 1>&2
				case $STR in
					'...')
						;;
					'GtkWidget*' | 'GSourceFunc' | 'gpointer' | 'GtkColorSelection*' | 'GtkTreePath*' | 'GtkTreeModel*' | 'GtkTreeIter*' | 'GdkEvent*' | 'GdkEventKey*' | 'GtkCellLayout*' | 'GtkTreeSelection*' | 'GtkClipboard*' | 'GError*' | 'GSList*' | 'GIOChannel*' | 'GtkFileChooser*' | 'GtkRequisition*' | 'GdkEventButton*' | 'GtkStyle*' | 'GtkAllocation*' | 'GdkEventFocus*' | 'GdkEventWindowState*' | 'GdkColor []' | 'GdkRGBA []' | 'PangoFontFamily*' | 'PangoFontFace*' | 'GtkColorButton*' | 'GtkColorChooser*')
						FUNCTION="$FUNCTION NULL,"
						;;
					'GKeyFile*'|'GdkColor'|'GdkRGBA')
						OLD_SPACE="$SPACE"
						SPACE="$SPACE""_SPACE_"
						VAR=`expr $VAR + 1`
						case $STR in
							'GKeyFile*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""GKeyFile *V$VAR = g_key_file_new();"
								FUNCTION="$FUNCTION V$VAR,"
								FUN_DATA="$SPACE""g_key_file_free(V$VAR);\n"
								;;
							'GdkColor')
								FUNC_STAR="$FUNC_STAR\n$SPACE""GdkColor V$VAR;\n$SPACE""V$VAR.red=0xFFFF;\n$SPACE""V$VAR.green=0xFFFF;\n$SPACE""V$VAR.blue=0xFFFF;"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'GdkRGBA')
								FUNC_STAR="$FUNC_STAR\n$SPACE""GdkRGBA V$VAR;\n$SPACE""V$VAR.red=1;\n$SPACE""V$VAR.green=1;\n$SPACE""V$VAR.blue=1;"
								FUNCTION="$FUNCTION V$VAR,"
								;;
						esac
						if [ -n "$FUNC_END" ]; then
							FUNC_END="\n$FUNC_END"
						fi
						FUNC_END="$FUN_DATA$FUNC_END"
						unset FUN_DATA
						SPACE=$OLD_SPACE
						;;
					'gboolean' | 'gchar' | 'guint' | 'GtkScrollType' | 'gint' | 'pid_t' | 'int' | 'gsize' | 'glong' | 'GdkColor' | 'GdkRGBA' | 'Dialog_Button_Type' | 'Dialog_Find_Type' | 'Dialog_Type_Flags' | 'Font_Name_Type' | 'Key_Bindings' | 'Hints_Type' | 'Window_Status' | 'Geometry_Resize_Type' | 'Font_Reset_Type' | 'Switch_Type' | 'Font_Set_Type' | 'Set_ANSI_Theme_Type' | 'GtkFileChooserAction' | 'GIOCondition' | 'Check_Zero' | 'Check_Max' | 'Check_Min' | 'Check_Empty' | 'Menu_Itemn_Type' | 'Apply_Profile_Type' | 'Clipboard_Type' | 'gchar*' | 'char*' | 'StrLists*' | 'StrAddr**' | 'gdouble' | 'struct Dialog*' | 'struct Window*' | 'struct Page*' | 'struct Color_Data*' | 'struct Preview*' | 'GtkButton*' | 'GtkCellRenderer*' | 'GtkRange*' | 'gchar**' | 'char*[]' | 'char**' | 'gsize*' | 'GString*' | 'GtkNotebook*' | 'GdkColor*' | 'GdkRGBA*' | 'VteTerminal*'  | 'gboolean*' | 'gint*' | 'guint*' | 'glong*')
						SPACE="$SPACE""_SPACE_"
						VAR=`expr $VAR + 1`
						if [ $MAX_VAR -le $VAR ]; then
							MAX_VAR=$VAR
						fi
						case $STR in
							'gboolean' | 'guint' | 'gint' | 'pid_t' | 'int' | 'gsize' | 'glong' | 'gdouble' | 'gchar')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$STR V$VAR = V[$VAR];"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'gboolean*' | 'gint*' | 'guint*' | 'gsize*' | 'glong*')
								NO_STAR_STR=`$ECHO $STR | sed -e 's/\*$//g'`
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$NO_STAR_STR V$VAR = V[$VAR];"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$STR G$VAR = NULL;"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""if (V[$VAR]) G$VAR = ($STR)&(V$VAR);"
								FUNCTION="$FUNCTION G$VAR,"
								;;
							'Dialog_Button_Type' | 'Dialog_Find_Type' | 'Dialog_Type_Flags' | 'Font_Name_Type' | 'Key_Bindings' | 'Hints_Type' | 'Window_Status' | 'Geometry_Resize_Type' | 'Font_Reset_Type' | 'Font_Reset_Type' | 'Switch_Type' | 'Font_Set_Type' | 'Set_ANSI_Theme_Type' | 'Check_Zero' | 'Check_Max' | 'Check_Min' | 'Check_Empty' | 'Menu_Itemn_Type' | 'Apply_Profile_Type' | 'Clipboard_Type')
								LAST=`grep -B 3 "$STR;" *.h | sed -e '/\.h[-:][ \t]*#/d'| tail -n 2 | head -n 1 | head -n 1| sed -e 's/^.*[ \t][ \t]*\([^ \t]*\),.*/\1/g'`
								# $ECHO Got LAST=$LAST
								# $ECHO Got STR=$STR

								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<=$LAST; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$STR V$VAR = V[$VAR];"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'GtkScrollType')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<=GTK_SCROLL_END; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$STR V$VAR = V[$VAR];"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'GtkFileChooserAction')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<=GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$STR V$VAR = V[$VAR];"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'GIOCondition')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<=G_IO_NVAL; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$STR V$VAR = V[$VAR];"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'gchar')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$STR V$VAR = CHAR[V[$VAR]];"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'gchar*' | 'char*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""gchar *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = g_strdup(\"\");"
								FUNCTION="$FUNCTION V$VAR,"
								FUN_DATA="$FUN_DATA$SPACE""_SPACE_""g_free(V$VAR);\n"
								;;
							'StrLists*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""StrLists *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = g_strdup(\"\");"
								FUNCTION="$FUNCTION V$VAR, NULL"
								FUN_DATA="$FUN_DATA$SPACE""_SPACE_""g_free(V$VAR);\n"
								;;
							'StrAddr**')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""StrAddr *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = g_strdup(\"\");"
								FUNCTION="$FUNCTION &V$VAR,"
								# FUN_DATA="$SPACE""_SPACE_""g_debug(\"$FUNC_NAME(): Trying to free V$VAR \\\"%s\\\" (%p)\", V$VAR, V$VAR);\n"
								FUN_DATA="$FUN_DATA$SPACE""_SPACE_""g_free(V$VAR);\n"
								;;
							'struct Dialog*' | 'struct Window*' | 'struct Page*' | 'struct Color_Data*' | 'struct Preview*')
								STRUCT=`$ECHO $STR | sed -e 's/struct \(.*\)\*/\1/g'`
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""struct $STRUCT *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = g_new0(struct $STRUCT, 1);"
								FUNCTION="$FUNCTION V$VAR,"
								FUN_DATA="$SPACE""_SPACE_""g_free(V$VAR);\n"
								;;
							'GtkButton*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GtkWidget *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = gtk_button_new();"
								FUNCTION="$FUNCTION GTK_BUTTON(V$VAR),"
								FUN_DATA="$SPACE""_SPACE_""if (V$VAR) gtk_widget_destroy(V$VAR);\n"
								;;
							'GtkCellRenderer*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GtkCellRenderer *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = gtk_cell_renderer_text_new();"
								FUNCTION="$FUNCTION V$VAR,"
								# FUN_DATA="$SPACE""_SPACE_""if (V$VAR) gtk_object_destroy(GTK_OBJECT(V$VAR));\n"
								;;
							'GtkRange*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GtkWidget *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = gtk_hscale_new_with_range(-1, 1, 0.1);"
								FUNCTION="$FUNCTION GTK_RANGE(V$VAR),"
								FUN_DATA="$SPACE""_SPACE_""if (V$VAR) gtk_widget_destroy(V$VAR);\n"
								;;
							'gchar**' | 'char*[]' | 'char**')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""gchar **V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = g_strsplit(\"\", \" \", -1);"
								FUNCTION="$FUNCTION V$VAR,"
								FUN_DATA="$SPACE""_SPACE_""g_strfreev(V$VAR);\n"
								;;
							'GString*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GString *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = g_string_new(NULL);"
								FUNCTION="$FUNCTION V$VAR,"
								FUN_DATA="$SPACE""_SPACE_""if (V$VAR) g_string_free(V$VAR, TRUE);\n"
								;;
							'GtkNotebook*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GtkWidget *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = gtk_notebook_new();"
								FUNCTION="$FUNCTION GTK_NOTEBOOK(V$VAR),"
								FUN_DATA="$SPACE""_SPACE_""if (V$VAR) gtk_widget_destroy(V$VAR);\n"
								;;
							'GdkColor*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GdkColor *V$VAR = NULL;\n$SPACE""_SPACE_""GdkColor color;\n""$SPACE""_SPACE_""if (V[$VAR]) {\n$SPACE""_SPACE__SPACE_""color.red=0xFFFF;\n$SPACE""_SPACE__SPACE_""color.green=0xFFFF;\n$SPACE""_SPACE__SPACE_""color.blue=0xFFFF;\n$SPACE""_SPACE__SPACE_""V$VAR = &color;\n""$SPACE""_SPACE_""}"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'GdkRGBA*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GdkRGBA *V$VAR = NULL;\n$SPACE""_SPACE_""GdkRGBA color;\n""$SPACE""_SPACE_""if (V[$VAR]) {\n$SPACE""_SPACE__SPACE_""color.red=0xFFFF;\n$SPACE""_SPACE__SPACE_""color.green=1;\n$SPACE""_SPACE__SPACE_""color.blue=0xFFFF;\n$SPACE""_SPACE__SPACE_""V$VAR = &color;\n""$SPACE""_SPACE_""}"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'VteTerminal*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GtkWidget *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = vte_terminal_new();"
								FUNCTION="$FUNCTION VTE_TERMINAL(V$VAR),"
								FUN_DATA="$SPACE""_SPACE_""if (V$VAR) gtk_widget_destroy(V$VAR);\n"
								;;
							*)
								$ECHO "ERROR: $STR NOT Found!" 1>&2
								exit
								;;
						esac
						FUNC_END="$FUN_DATA$SPACE}\n$FUNC_END"
						unset FUN_DATA
						# $ECHO "Got FUNC_STAR = $FUNC_STAR"
						# $ECHO "Got FUNC_END = $FUNC_END" 1>&2
						;;
					')')
						FUNCTION=`$ECHO $FUNCTION | sed -e 's/,$/)/g'`
						;;
					*)
						$ECHO "ERROR: \"$STR\" NOT Found!" 1>&2
						exit
						;;
				esac
				;;
			2)
				case $STR in
					';')
						FUNCTION="$FUNCTION;"
						;;
					*)
						$ECHO "ERROR: \"$STR\" NOT Found!" 1>&2
						exit
						;;
				esac
				;;
			*)
				$ECHO "ERROR: \"$START\" is out of range!" 1>&2
		esac
	done

	# $ECHO "GOT FULL_FUNCTION = $FULL_FUNCTION" 1>&2
	# $ECHO "GOT FUNC_STAR = $FUNC_STAR" 1>&2
	# $ECHO "GOT FUNCTION = $FUNCTION" 1>&2
	# $ECHO "GOT FUNC_END = $FUNC_END" 1>&2
	FUNCTION=`$ECHO $FUNCTION | sed -e 's/^( /(/g'`
	FUNCTION="$SPACE""_SPACE_""$FUNC_NAME$FUNCTION"
	FULL_FUNCTION="$FULL_FUNCTION\n  g_debug(\"UNIT TEST: testing $FUNC_NAME()...\");"
	FULL_FUNCTION="$FULL_FUNCTION$FUNC_STAR\n$FUNCTION\n$FUNC_END"

	MAX_VAR=`expr $MAX_VAR + 1`

	grep "include <.*>" *.h | cut -f 2 -d : | sed -e 's/^[ \t]//g' | sort | uniq > unit_test.c
	for FILE in *.h; do
		$ECHO "#include \"$FILE\"" >> unit_test.c
	done

	$CAT >> unit_test.c << EOF

int main(int argc, char *argv[])
{
EOF
	if [ $MAX_VAR -gt 0 ]; then
		$CAT >> unit_test.c << EOF
  gint V[$MAX_VAR];
EOF
fi
	$CAT >> unit_test.c << EOF
  gtk_init(&argc, &argv);
EOF

	# $ECHO "GOT FULL_FUNCTION = $FULL_FUNCTION"
	$ECHO "$FULL_FUNCTION" | sed -e 's/\\n/\n/g' | sed -e 's/_SPACE_/  /g' >> unit_test.c

	$CAT >> unit_test.c << EOF
  return 0;
}
EOF
	if [ $TEST_SCRIPT_ONLY -eq 0 ]; then
		$PRINTF "\033[1;36m$FUNC_NAME(): \033[1;33m** Compiling unit_test.o...\033[0m\n"
		$CC $LDFLAGS $CFLAGS $INCLUDES -c unit_test.c `$PKGCONFIG --cflags $GTK $VTE` || exit 1
		$PRINTF "\033[1;36m$FUNC_NAME(): \033[1;33m** Compiling unit_test...\033[0m\n"
		$CC $LDFLAGS $CFLAGS $INCLUDES -o unit_test $OBJ `$PKGCONFIG --cflags --libs $GTK $VTE` || exit 1
		# if [ $? != 0 ]; then exit 1; fi

		if [ $BUILD_ONLY -eq 0 ]; then
			if [ $RUN_GDB -eq 1 ]; then
				$PRINTF "\033[1;36m$FUNC_NAME(): \033[1;33m** Testing with $GDB...\033[0m\n"
				if [ -n "$SPECIFIC_FUNCTION" ]; then
					$GDB -batch -x ./gdb_batch --args ./unit_test $GTK_DEBUG
				else
					$ECHO "Testing $FUNC_NAME() with $GDB..." > /tmp/lilyterm_$FUNC_NAME.log
					$GDB -batch -x ./gdb_batch --args ./unit_test $GTK_DEBUG  >> /tmp/lilyterm_$FUNC_NAME.log 2>&1
					CHECK_STR=`tail -n 4 /tmp/lilyterm_$FUNC_NAME.log | grep 'exited normally'`
					if [ -z "$CHECK_STR" ]; then
						$CAT /tmp/lilyterm_$FUNC_NAME.log >> gdb.log
						$ECHO "" >> gdb.log
						$PRINTF "\033[1;36m$FUNC_NAME(): \033[1;31m** Exit with GDB error!!\033[0m\n"
						GDB_ERROR=1
						GDB_ERROR_PROGRAM="$GDB_ERROR_PROGRAM$FUNC_NAME()\n             "
					else
						$PRINTF "\033[1;36m$FUNC_NAME(): \033[1;33m** Program exited normally. Clear log...\033[0m\n"
					fi
					rm /tmp/lilyterm_$FUNC_NAME.log
				fi
			fi

			if [ $RUN_VALGRIND -eq 1 ]; then
				$PRINTF "\033[1;36m$FUNC_NAME(): \033[1;33m** Testing with valgrind...\033[0m\n"
				$ECHO "Testing $FUNC_NAME() with $VALGRIND..." >> valgrind.log
				$VALGRIND --leak-check=full ./unit_test >> valgrind.log 2>&1
				$ECHO "" >> valgrind.log
			fi
		fi
	fi
	$ECHO ""
done

if [ -f ./gdb_batch ]; then
	rm ./gdb_batch
fi

if [ $GDB_ERROR -eq 1 ]; then
	$PRINTF "\n\033[1;31mERROR: \033[1;36mThis unit test program had exited with GDB error.\n       LIST: \033[1;33m`printf "$GDB_ERROR_PROGRAM" | sed -e '$d'`\033[1;36m\n       Please check \033[1;32mgdb.log\033[1;36m for feature information!!\033[0m\n\n"
fi
