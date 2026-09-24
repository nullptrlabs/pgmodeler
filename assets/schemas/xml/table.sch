# XML definition for tables
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.
[<table name=] "&{name}"
%if {layers} %then [ layers=] "{layers}" %end
%if {scene} %then [ scene=] "{scene}" %end
%if {alias} %then [ alias=] "&{alias}" %end
%if {oids} %then [ oids="true"] %end
%if {gen-alter-cmds} %then [ gen-alter-cmds="true"] %end
%if {unlogged} %then [ unlogged="true"] %end
%if {protected} %then [ protected=] "true" %end
%if {sql-disabled} %then [ sql-disabled=] "true" %end
%if {collapse-mode} %then [ collapse-mode=] "{collapse-mode}" %end
%if {pagination} %then [ pagination=] "true" %end
%if {attribs-page} %then [ attribs-page=] "{attribs-page}" %end
%if {ext-attribs-page} %then [ ext-attribs-page=] "{ext-attribs-page}" %end
%if {faded-out} %then [ faded-out=] "true" %end
%if {rls-enabled} %then [ rls-enabled=] "true" %end
%if {rls-forced} %then [ rls-forced=] "true" %end
%if {max-obj-count} %then [ max-obj-count=] "{max-obj-count}" %end
%if {z-value} %then [ z-value=] "{z-value}" %end
> $br

{schema}
%if {owner} %then {owner} %end
%if {tablespace} %then {tablespace} %end
%if {comment} %then {comment} %end
%if {tag} %then {tag} %end
%if {appended-sql} %then {appended-sql} %end
%if {prepended-sql} %then {prepended-sql} %end
{position}
%if {cutoff-pos} %then {cutoff-pos} %end

%if {columns} %then {columns} %end
%if {constraints} %then {constraints} %end

%if {col-indexes} %then {col-indexes} %end
%if {constr-indexes} %then {constr-indexes} %end

%if {partitioning} %and {partitionkey} %then
	$tb [<partitioning type=] "{partitioning}" [>] $br
	{partitionkey}
	$tb [</partitioning>] $br
%end

%if {initial-data} %then
	$tb <initial-data> $br
	<! $ob CDATA $ob {initial-data} $cb $cb >
	$br $tb </initial-data> $br
%end

</table> $br $br
