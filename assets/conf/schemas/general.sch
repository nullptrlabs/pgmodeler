# XML definition for basic configurations file
# CAUTION: Do not modify this file unless you know what you are doing.
[<?xml version="1.0" encoding="UTF-8" ?>] \n
[<!--] \n
[ CAUTION: Do not modify this file directly on it's code unless you know what you are doing.] \n
[ Unexpected results may occur if the code is changed deliberately.] \n
[-->] \n

%set {spc}  \t \t

<general> \n
\t [<configuration pgmodeler-ver="] {pgmodeler-ver} ["] \n 
{spc} [first-run="] {first-run} ["] \n
{spc} [op-list-size="] {op-list-size} ["] \n
{spc} [autosave-interval="] {autosave-interval} ["] \n
{spc} [paper-type="] {paper-type} ["] \n
{spc} [paper-orientation="] {paper-orientation} ["] \n
{spc} [paper-margin="] {paper-margin} ["] \n

%if {paper-custom-size} %then
	{spc} [paper-custom-size="] {paper-custom-size} ["] \n
%end

{spc} [print-grid="] %if {print-grid} %then true %else false %end ["] \n
{spc} [print-pg-num="] %if {print-pg-num} %then true %else false %end ["] \n
{spc} [hide-rel-name="] %if {hide-rel-name} %then true %else false %end ["] \n
{spc} [hide-ext-attribs="] %if {hide-ext-attribs} %then true %else false %end ["] \n
{spc} [hide-table-tags="] %if {hide-table-tags} %then true %else false %end ["] \n
{spc} [canvas-corner-move="] %if {canvas-corner-move} %then true %else false %end ["] \n
{spc} [invert-rangesel-trigger="] %if {invert-rangesel-trigger} %then true %else false %end ["] \n
{spc} [check-update="] %if {check-update} %then true %else false %end ["] \n
{spc} [check-versions="] {check-versions} ["] \n
{spc} [save-last-position="] %if {save-last-position} %then true %else false %end ["] \n
{spc} [show-main-menu="] %if {show-main-menu} %then true %else false %end ["] \n
{spc} [disable-smoothness="] %if {disable-smoothness} %then true %else false %end ["] \n
{spc} [simplified-obj-creation="] %if {simplified-obj-creation} %then true %else false %end ["] \n
{spc} [confirm-validation="] %if {confirm-validation} %then true %else false %end ["] \n
{spc} [code-completion="] %if {code-completion} %then true %else false %end ["] \n
{spc} [use-placeholders="] %if {use-placeholders} %then true %else false %end ["] \n
{spc} [show-canvas-grid="] %if {show-canvas-grid} %then true %else false %end ["] \n
{spc} [show-page-delimiters="] %if {show-page-delimiters} %then true %else false %end ["] \n
{spc} [lock-page-delim-resize="] %if {lock-page-delim-resize} %then true %else false %end ["] \n
{spc} [align-objs-to-grid="] %if {align-objs-to-grid} %then true %else false %end ["] \n
{spc} [history-max-length="] {history-max-length} ["] \n
{spc} [use-curved-lines="] %if {use-curved-lines} %then true %else false %end ["] \n
{spc} [compact-view="] %if {compact-view} %then true %else false %end ["] \n
{spc} [save-restore-geometry="] %if {save-restore-geometry} %then true %else false %end ["] \n
{spc} [low-verbosity="] %if {low-verbosity} %then true %else false %end ["] \n
{spc} [escape-comment="] %if {escape-comment} %then true %else false %end ["] \n
{spc} [hide-schema-names-of-types="] %if {hide-schema-names-of-types} %then true %else false %end ["] \n
{spc} [old-pgsql-versions="] %if {old-pgsql-versions} %then true %else false %end ["] \n
{spc} [hide-obj-shadows="] %if {hide-obj-shadows} %then true %else false %end ["] \n
{spc} [truncate-column-data="] %if {truncate-column-data} %then true %else false %end ["] \n
{spc} [column-trunc-threshold="] {column-trunc-threshold} ["] \n
{spc} [disable-inline-editor="] %if {disable-inline-editor} %then true %else false %end ["] \n
{spc} [alert-unsaved-models="] {alert-unsaved-models} ["] \n
{spc} [alert-open-sqltabs="] {alert-open-sqltabs} ["] \n
{spc} [alert-apply-metadata="] {alert-apply-metadata} ["] \n
{spc} [use-def-disambiguation="] {use-def-disambiguation} ["] \n
{spc} [hide-objects-sel-info="] %if {hide-objects-sel-info} %then true %else false %end ["] \n
{spc} [hide-cur-pos-zoom-info="] %if {hide-cur-pos-zoom-info} %then true %else false %end ["] \n
{spc} [hide-empty-obj-groups="] %if {hide-empty-obj-groups} %then true %else false %end ["] \n

%if {source-editor-app} %then
	{spc} [source-editor-app="] &{source-editor-app} ["] \n
%end

%if {source-editor-app} %and {source-editor-args} %then
	{spc} [source-editor-args="] &{source-editor-args} ["] \n
%end

%if {ui-language} %then
	{spc} [ui-language="] &{ui-language} ["] \n
%end

\t /> \n

%if {file} %then
	\n \t <session> \n
	\t {file}
	\t </session> \n
%end

%if {recent-models} %then
	\n \t <recent-models> \n
	{recent-models}
	\t </recent-models> \n
%end

%if {dock-widgets} %then
	\n \t <dock-widgets> \n
	{dock-widgets}
	\t </dock-widgets> \n
%end

%if {widgets-geometry} %then
	\n \t <widgets-geometry> \n
	{widgets-geometry}
	\t </widgets-geometry> \n
%end

</general> \n
