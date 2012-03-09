#
# Copyright (c) 2008-2010 Lu, Chao-Ming (Tetralet).  All rights reserved.
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

#!/bin/bash

INCLUDES="$1"

RUN_GDB=0
RUN_VALGRIND=0
TEST_SCRIPT_ONLY=0
BUILD_ONLY=0
LIB_LISTS="*.h"
FUNCTION_FOUND=0

for opt do
	case "$opt" in
		--help)
			echo "Usage: sh $0 INCLUDES --enable-glib-debugger --enable-gtk-debugger --enable-gdb --enable-valgrind --specific_function=FUNCTION_NAME --create_program_only --build_program_only"
			exit 0
			;;
		--enable-glib-debugger)
			export G_DEBUG=fatal_warnings
			LIB_LISTS=lilyterm.h
			;;
		--enable-gtk-debugger)
			GTK_DEBUG="--g-fatal-warnings"
			LIB_LISTS=lilyterm.h
			;;
		--enable-gdb)
			RUN_GDB=1
			;;
		--enable-valgrind)
			RUN_VALGRIND=1
			LIB_LISTS=lilyterm.h
			;;
		--specific_function=*)
			SPECIFIC_FUNCTION=`echo $opt | cut -d '=' -f 2`
			;;
		--create_program_only)
			TEST_SCRIPT_ONLY=1
			;;
		--build_program_only)
			BUILD_ONLY=1
			;;
	esac
done

ECHO=`whereis -b echo | awk '{print $2}'`

CHECK_INCLUDES=`$ECHO "$INCLUDES" | grep -- '-DUNIT_TEST'`
if [ -z "$CHECK_INCLUDES" ]; then
	make clean
	make uto
	INCLUDES="-DDEFENSIVE -DDEBUG -DFATAL -DDEVELOP -DUNIT_TEST"
	RUN_GDB=1
fi


PKGCONFIG=`whereis -b pkg-config | awk '{print $2}'`
if [ -z "$PKGCONFIG" ]; then
	$ECHO -e "\x1b[1;31m** ERROR: Command pkg-config is not found!\x1b[0m"
	exit 1
fi

VTE=`$PKGCONFIG --exists 'vte' && $ECHO 'vte'`
if [ $VTE = "vte" ]; then
  GTK=`$PKGCONFIG --exists 'gtk+-2.0' && $ECHO 'gtk+-2.0'`
  if [ "$GTK" != "gtk+-2.0" ]; then
    $ECHO -e "\x1b[1;31m** ERROR: You need GTK+2 to run this unit test program!\x1b[0m"
    exit 1
  fi
else
  VTE=`$PKGCONFIG --exists 'vte-2.90' && $ECHO 'vte-2.90'`
  if [ $VTE = "vte-2.90" ]; then
    GTK=`$PKGCONFIG --exists 'gtk+-3.0' && $ECHO 'gtk+-3.0'`
    if [ "$GTK" != "gtk+-3.0" ]; then
      $ECHO -e "\x1b[1;31m** ERROR: You need GTK+3 to run this unit test program!\x1b[0m"
      exit 1
    fi
  else
    $ECHO -e "\x1b[1;31m** ERROR: You need VTE to run this unit test program!\x1b[0m"
    exit 1
  fi
fi

if [ -z "$CC" ]; then
  CC=gcc
fi

if [ -z "$CFLAGS" ]; then
  CFLAGS="-Wall -Werror -Wformat -Wformat-security -Werror=format-security -O2 -g"
fi

OBJ="menu.o profile.o dialog.o pagename.o notebook.o font.o property.o window.o misc.o console.o main.o"

cat > lilyterm.gdb << EOF
run $GTK_DEBUG
bt full
EOF

# sed '/^\/\*/,/ \*\/$/d': Delete [ /* blah ... blah */ ] (multi lines)
# sed -e 's/[ \t]*\/\*[ \t]*.*[ \t]*\*\///g': Delete [ /* blah ... blah */ ] (single line)
# sed -e 's/[ \t]*\/\/.*//g': Delete [ // blah ... blah ]
# sed -e '/[ \t]*#[ \t]*ifdef[ \t]*USE_NEW_GEOMETRY_METHOD/,/#[ \t]*endif[ \t]*/d': clear #ifdef USE_NEW_GEOMETRY_METHOD ... #endif
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

for DATA in `cat $LIB_LISTS | sed '/^\/\*/,/ \*\/$/d' | sed -e 's/[ \t]*\/\*[ \t]*.*[ \t]*\*\///g' | sed -e 's/[ \t]*\/\/.*//g' | sed -e '/[ \t]*#[ \t]*ifdef[ \t]*USE_NEW_GEOMETRY_METHOD/,/#[ \t]*endif[ \t]*/d' | sed -e '/^[ \t]*#.*/d' | sed '/^[ \t]*typedef.*;[ \t]*$/d' | sed '/^[ \t]*typedef enum/,/}.*;[ \t]*$/d' | tr -d '\n' | sed -e 's/[\t ][\t ]*/_SPACE_/g' | sed -e 's/;/;\n/g' | sed -e 's/_SPACE_/ /g' | sed -e '/[ \t]*struct.*{/,/.*}[ \t]*;/d' | sed -e 's/ *\([)(,]\) */ \1 /g' | sed -e 's/[\t ][\t ]*/_SPACE_/g' | sed -e '/_SPACE_(_SPACE_)_SPACE_/d'`; do

	if [ $FUNCTION_FOUND -eq 1 ]; then
		break
	fi

	if [ -n "$SPECIFIC_FUNCTION" ]; then
		CHECK_STR="_SPACE_\**"$SPECIFIC_FUNCTION"_SPACE_"
		CHECK_PROGRAM=`echo "$DATA" | grep "$CHECK_STR"`
		if [ -z "$CHECK_PROGRAM" ]; then
			continue
		else
			FUNCTION_FOUND=1
		fi
	fi
	MAX_STR=0
	MAX_VAR=-1
	unset FULL_FUNCTION

	# echo "Got original data = $DATA"
	# sed -e 's/_SPACE_\([)(,]\)_SPACE_/ \1 /g': convert _SPACE_)(,_SPACE_ to " ) " " ( " or " , "
	# sed -e 's/ , / /g': convert " , " to " "
	# sed -e 's/const_SPACE_//g': clear const_SPACE_
	DATA_STR=`echo $DATA | sed -e 's/_SPACE_\([)(,]\)/ \1/g' | sed -e 's/\([)(,]\)_SPACE_/\1 /g' | sed -e 's/ , / /g' | sed -e 's/const_SPACE_//g'`
	# echo "GOT DATA_STR = $DATA_STR" 1>&2
	START=0
	END=0
	VAR=-1

	for STR in $DATA_STR; do
		# echo "GOT and Checking STR='$STR'"
		case $START in
			0)
				if [ "$STR" = "(" ]; then
					START=1
					FUNCTION="("
					# echo "GOT FUNCTION = $FUNCTION"
				else
					# Got the function name.
					unset FUNC_STAR
					unset FUNC_END
					unset SPACE
					# sed -e 's/_SPACE_/ /g' : convert "_SAPCE_" to <Space>
					# sed -e 's/^ *//g': clear the leading <Space>
					# sed -e 's/ *$//g': clear the end <Space>
					# sed -e 's/^.* \**\([^ ]*\)/\1/g': clear the declare, like "gchar **"
					FUNC_NAME=`echo $STR | sed -e 's/_SPACE_/ /g' | sed -e 's/^ *//g' | sed -e 's/ *$//g' | sed -e 's/^.* \**\([^ ]*\)/\1/g'`
					# echo "GOT FUNC_NAME = $FUNC_NAME"
					echo -e "\x1b[1;36m$FUNC_NAME(): \x1b[1;33m** Createing unit_test.c...\x1b[0m"
				fi
				;;
			1)
				if [ "$STR" = ")" ]; then
					FUNCTION=`echo $FUNCTION | sed -e 's/,*$/)/g'`
					START=2
					continue
				fi
				# echo "CHECKING: GOT FUNCTION = $FUNCTION"
				# echo "Testing $STR..." 1>&2

				# sed -e 's/_SPACE_/ /g': convert "_SAPCE_" to <Space>
				# sed -e 's/^\(.* \**\) *\([^][ ]*\)\([[]*\)[^][ ]*\([]]*\)/\1\3\4/g': convert "gchar *profile[A]" to gchar *[]
				# sed -e 's/^ *//g': clear the leading <Space>
				# sed -e 's/ *$//g': clear the end <Space>
				# sed -e 's/^.* \**\([^ ]*\)/\1/g': clear the variable
				STR=`echo $STR | sed -e 's/_SPACE_/ /g' | sed -e 's/^\(.* \**\) *\([^][ ]*\)\([[]*\)[^][ ]*\([]]*\)/\1\3\4/g' | sed -e 's/^ *//g' | sed -e 's/ *$//g' | sed -e 's/ *\*/*/g'`
				# echo "GOT and Testing \"$STR\"..." 1>&2
				case $STR in
					'...')
						;;
					'GtkWidget*' | 'GSourceFunc' | 'gpointer' | 'GtkColorSelection*' | 'GtkTreePath*' | 'GtkTreeModel*' | 'GtkTreeIter*' | 'GdkEvent*' | 'GdkEventKey*' | 'GtkCellLayout*' | 'GtkTreeSelection*' | 'GtkClipboard*' | 'GError*' | 'GSList*' | 'GIOChannel*' | 'GtkFileChooser*' | 'GtkRequisition*' | 'GdkEventButton*' | 'GtkStyle*' | 'GtkAllocation*' | 'GdkEventFocus*' | 'GdkEventWindowState*')
						FUNCTION="$FUNCTION NULL,"
						;;
					'GKeyFile*'|'GdkColor')
						OLD_SPACE="$SPACE"
						SPACE="$SPACE""_SPACE_"
						VAR=`expr $VAR + 1`
						if [ $MAX_VAR -le $VAR ]; then
							MAX_VAR=$VAR
						fi
						case $STR in
							'GKeyFile*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""GKeyFile *V$VAR = g_key_file_new();"
								FUNCTION="$FUNCTION V$VAR,"
								FUN_DATA="$SPACE""g_key_file_free(V$VAR);\n"
								;;
							'GdkColor')
								FUNC_STAR="$FUNC_STAR\n$SPACE""GdkColor V$VAR;\n$SPACE""gdk_color_parse (\"dark\", &V$VAR);"
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
					'gboolean' | 'gchar' | 'guint' | 'GtkScrollType' | 'gint' | 'pid_t' | 'int' | 'gsize' | 'glong' | 'GdkColor' | 'Dialog_Button_Type' | 'Dialog_Find_Type' | 'Dialog_Type_Flags' | 'Font_Name_Type' | 'Font_Reset_Type' | 'Switch_Type' | 'Font_Set_Type' | 'GtkFileChooserAction' | 'GIOCondition' | 'Check_Zero' | 'Check_Max' | 'Check_Min' | 'Check_Empty' | 'Menu_Itemn_Type' | 'Apply_Profile_Type' | 'Clipboard_Type' | 'gchar*' | 'char*' | 'StrLists*' | 'StrAddr**' | 'gdouble' | 'struct Dialog*' | 'struct Window*' | 'struct Page*' | 'struct Color_Data*' | 'struct Preview*' | 'GtkButton*' | 'GtkCellRenderer*' | 'GtkRange*' | 'gchar**' | 'char*[]' | 'char**' | 'gsize*' | 'GString*' | 'GtkNotebook*' | 'GdkColor*' | 'GdkColor []' | 'VteTerminal*'  | 'gboolean*' | 'gint*' | 'guint*')
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
							'gboolean*' | 'gint*' | 'guint*' | 'gsize*')
								NO_STAR_STR=`echo $STR | sed -e 's/\*$//g'`
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$NO_STAR_STR V$VAR = V[$VAR];"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""$STR G$VAR = NULL;"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""if (V[$VAR]) G$VAR = ($STR)&(V$VAR);"
								FUNCTION="$FUNCTION G$VAR,"
								;;
							'Dialog_Button_Type' | 'Dialog_Find_Type' | 'Dialog_Type_Flags' | 'Font_Name_Type' | 'Font_Reset_Type' | 'Switch_Type' | 'Font_Set_Type' | 'Check_Zero' | 'Check_Max' | 'Check_Min' | 'Check_Empty' | 'Menu_Itemn_Type' | 'Apply_Profile_Type' | 'Clipboard_Type')
								LAST=`grep -B 1 "$STR;" *.h | head -n 1 | sed -e 's/^.*[ \t][ \t]*\([^ \t]*\),.*/\1/g'`
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
								STRUCT=`echo $STR | sed -e 's/struct \(.*\)\*/\1/g'`
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
								FUN_DATA="$SPACE""_SPACE_""if (V$VAR) gtk_object_destroy(GTK_OBJECT(V$VAR));\n"
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
							'GdkColor*' | 'GdkColor []')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GdkColor *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) {\n$SPACE""_SPACE__SPACE_""GdkColor dark;\n$SPACE""_SPACE__SPACE_""gdk_color_parse (\"dark\", &dark);\n$SPACE""_SPACE__SPACE_""V$VAR = &dark;\n""$SPACE""_SPACE_""}"
								FUNCTION="$FUNCTION V$VAR,"
								;;
							'VteTerminal*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GtkWidget *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = vte_terminal_new();"
								FUNCTION="$FUNCTION VTE_TERMINAL(V$VAR),"
								FUN_DATA="$SPACE""_SPACE_""if (V$VAR) gtk_widget_destroy(V$VAR);\n"
								;;
							*)
								echo "ERROR: $STR NOT Found!" 1>&2
								exit
								;;
						esac
						FUNC_END="$FUN_DATA$SPACE}\n$FUNC_END"
						unset FUN_DATA
						# echo "Got FUNC_STAR = $FUNC_STAR"
						# echo "Got FUNC_END = $FUNC_END" 1>&2
						;;
					')')
						FUNCTION=`echo $FUNCTION | sed -e 's/,$/)/g'`
						;;
					*)
						echo "ERROR: \"$STR\" NOT Found!" 1>&2
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
						echo "ERROR: \"$STR\" NOT Found!" 1>&2
						exit
						;;
				esac
				;;
			*)
				echo "ERROR: \"$START\" is out of range!" 1>&2
		esac
	done

	# echo "GOT FULL_FUNCTION = $FULL_FUNCTION" 1>&2
	# echo "GOT FUNC_STAR = $FUNC_STAR" 1>&2
	# echo "GOT FUNCTION = $FUNCTION" 1>&2
	# echo "GOT FUNC_END = $FUNC_END" 1>&2
	FUNCTION=`echo $FUNCTION | sed -e 's/^( /(/g'`
	FUNCTION="$SPACE""_SPACE_""$FUNC_NAME$FUNCTION"
	FULL_FUNCTION="$FULL_FUNCTION\n  g_debug(\"UNIT TEST: testing $FUNC_NAME()...\");"
	FULL_FUNCTION="$FULL_FUNCTION$FUNC_STAR\n$FUNCTION\n$FUNC_END"

	MAX_VAR=`expr $MAX_VAR + 1`

	grep "include <.*>" *.h | cut -f 2 -d : | sed -e 's/^[ \t]//g' | sort | uniq > unit_test.c
	for FILE in *.h; do
		echo "#include \"$FILE\"" >> unit_test.c
	done

	cat >> unit_test.c << EOF

int main(int argc, char *argv[])
{
EOF
	if [ $MAX_VAR -gt 0 ]; then
		cat >> unit_test.c << EOF
  gint V[$MAX_VAR];
EOF
fi
	cat >> unit_test.c << EOF
  gtk_init(&argc, &argv);
EOF

	# echo "GOT FULL_FUNCTION = $FULL_FUNCTION"
	echo "$FULL_FUNCTION" | sed -e 's/\\n/\n/g' | sed -e 's/_SPACE_/  /g' >> unit_test.c

	cat >> unit_test.c << EOF
  return 0;
}
EOF
	if [ $TEST_SCRIPT_ONLY -eq 0 ]; then
		echo -e "\x1b[1;36m$FUNC_NAME(): \x1b[1;33m** Compiling unit_test...\x1b[0m"
		$CC $CFLAGS $INCLUDES -o unit_test unit_test.c $OBJ `$PKGCONFIG --cflags --libs $GTK $VTE` || exit 1

		if [ $BUILD_ONLY -eq 0 ]; then
			if [ $RUN_GDB -eq 1 ]; then
				echo -e "\x1b[1;36m$FUNC_NAME(): \x1b[1;33m** Testing with gdb...\x1b[0m"
				if [ -n "$FUNCTION_NAME"]; then
					gdb -batch -x ./lilyterm.gdb ./unit_test
				else
					echo "Testing $FUNC_NAME() with gdb..." > /tmp/lilyterm_$FUNC_NAME.log
					gdb -batch -x ./lilyterm.gdb ./unit_test >> /tmp/lilyterm_$FUNC_NAME.log 2>&1
					CHECK_STR=`tail -n 3 /tmp/lilyterm_$FUNC_NAME.log | tr -d '\n'`
					if [ "$CHECK_STR" != 'Program exited normally.No stack.' ]; then
						cat /tmp/lilyterm_$FUNC_NAME.log >> lilyterm_gdb.log
						echo "" >> lilyterm_gdb.log
					else
						echo -e "\x1b[1;36m$FUNC_NAME(): \x1b[1;33m** Program exited normally. Clear log...\x1b[0m"
					fi
					rm /tmp/lilyterm_$FUNC_NAME.log
				fi
			fi
	
			if [ $RUN_VALGRIND -eq 1 ]; then
				echo -e "\x1b[1;36m$FUNC_NAME(): \x1b[1;33m** Testing with valgrind...\x1b[0m"
				echo "Testing $FUNC_NAME() with valgrind..." >> lilyterm_valgrind.log
				valgrind --leak-check=full ./unit_test >> lilyterm_valgrind.log 2>&1
				echo "" >> lilyterm_valgrind.log
			fi
		fi
	fi
done

if [ -f ./lilyterm.gdb ]; then
	rm ./lilyterm.gdb
fi
