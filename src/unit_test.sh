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

MAX_STR=0
MAX_VAR=0
unset FULL_FUNCTION

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

for DATA in `cat *.h | sed '/^\/\*/,/ \*\/$/d' | sed -e 's/[ \t]*\/\*[ \t]*.*[ \t]*\*\///g' | sed -e 's/[ \t]*\/\/.*//g' | sed -e '/[ \t]*#[ \t]*ifdef[ \t]*USE_NEW_GEOMETRY_METHOD/,/#[ \t]*endif[ \t]*/d' | sed -e '/^[ \t]*#.*/d' | sed '/^[ \t]*typedef.*;[ \t]*$/d' | sed '/^[ \t]*typedef enum/,/}.*;[ \t]*$/d' | tr -d '\n' | sed -e 's/[\t ][\t ]*/_SPACE_/g' | sed -e 's/;/;\n/g' | sed -e 's/_SPACE_/ /g' | sed -e '/[ \t]*struct.*{/,/.*}[ \t]*;/d' | sed -e 's/ *\([)(,]\) */ \1 /g' | sed -e 's/[\t ][\t ]*/_SPACE_/g' | sed -e '/_SPACE_(_SPACE_)_SPACE_/d'`; do
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
					'gboolean' | 'gchar' | 'guint' | 'GtkScrollType' | 'gint' | 'pid_t' | 'int' | 'gsize' | 'glong' | 'GdkColor' | 'Dialog_Button_Type' | 'Dialog_Find_Type' | 'Dialog_Type_Flags' | 'Font_Name_Type' | 'Font_Reset_Type' | 'Switch_Type' | 'Font_Set_Type' | 'GtkFileChooserAction' | 'GIOCondition' | 'Check_Zero' | 'Check_Max' | 'Check_Min' | 'Check_Empty' | 'Menu_Itemn_Type' | 'Apply_Profile_Type' | 'gchar*' | 'char*' | 'StrLists*' | 'StrAddr**' | 'gdouble' | 'struct Dialog*' | 'struct Window*' | 'struct Page*' | 'struct Color_Data*' | 'struct Preview*' | 'GtkButton*' | 'GtkCellRenderer*' | 'GtkRange*' | 'gchar**' | 'char*[]' | 'char**' | 'gsize*' | 'GString*' | 'GtkNotebook*' | 'GKeyFile*' | 'GdkColor*' | 'GdkColor []' | 'VteTerminal*'  | 'gboolean*' | 'gint*' | 'guint*')
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
							'Dialog_Button_Type' | 'Dialog_Find_Type' | 'Dialog_Type_Flags' | 'Font_Name_Type' | 'Font_Reset_Type' | 'Switch_Type' | 'Font_Set_Type' | 'Check_Zero' | 'Check_Max' | 'Check_Min' | 'Check_Empty' | 'Menu_Itemn_Type' | 'Apply_Profile_Type')
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
							'GKeyFile*')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GKeyFile *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) V$VAR = g_key_file_new();"
								FUNCTION="$FUNCTION V$VAR,"
								FUN_DATA="$SPACE""_SPACE_""g_key_file_free(V$VAR);\n"
								;;
							'GdkColor')
								FUNC_STAR="$FUNC_STAR\n$SPACE""for (V[$VAR]=0; V[$VAR]<2; V[$VAR]++) {"
								FUNC_STAR="$FUNC_STAR\n$SPACE""_SPACE_""GdkColor *V$VAR = NULL;\n""$SPACE""_SPACE_""if (V[$VAR]) {\n$SPACE""_SPACE__SPACE_""GdkColor dark;\n$SPACE""_SPACE__SPACE_""gdk_color_parse (\"dark\", &dark);\n$SPACE""_SPACE__SPACE_""V$VAR = &dark;\n""$SPACE""_SPACE_""}"
								FUNCTION="$FUNCTION *V$VAR,"
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
	case $FUNC_NAME in
		'main_quit')
			FINISH="\n  g_debug(\"UNIT TEST: testing $FUNC_NAME()...\");""$FUNC_STAR\n$FUNCTION\n$FUNC_END"
			;;
		*)
			FULL_FUNCTION="$FULL_FUNCTION\n  g_debug(\"UNIT TEST: testing $FUNC_NAME()...\");"
			FULL_FUNCTION="$FULL_FUNCTION$FUNC_STAR\n$FUNCTION\n$FUNC_END"
			;;
	esac
done


MAX_VAR=`expr $MAX_VAR + 1`

cat << EOF
/*															  
 * Copyright (c) 2008-2010 Lu, Chao-Ming (Tetralet).  All rights reserved.
 *
 * This file is part of LilyTerm.
 *
 * LilyTerm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * LilyTerm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LilyTerm.  If not, see <http://www.gnu.org/licenses/>.
 */

EOF

grep "include <.*>" *.h | cut -f 2 -d : | sed -e 's/^[ \t]//g' | sort | uniq

echo ""

for FILE in *.h; do
	echo "#include \"$FILE\""
done

cat << EOF

int main(int argc, char *argv[])
{
  gint V[$MAX_VAR];

  gtk_init(&argc, &argv);                                                                                                   
  fake_main(0, NULL);
EOF

# echo "GOT FULL_FUNCTION = $FULL_FUNCTION"
echo "$FULL_FUNCTION$FINISH" | sed -e 's/\\n/\n/g' | sed -e 's/_SPACE_/  /g'

cat << EOF
  return 0;
}
EOF

