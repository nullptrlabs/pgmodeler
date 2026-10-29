# XML definition for constraints
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.
%if {decl-in-table} %then $tb %end

[<constraint name=] "&{name}" [ type=] "{type}"

%if {alias} %then
	$sp alias="&{alias}"
%end

%if {fk-constr} %then
	[ comparison-type=] "{comparison-type}"
	$br

	$tb
	[ upd-action=] "{upd-action}"
	[ del-action=] "{del-action}"
	[ ref-table=] "&{ref-table}"
%end


%if {deferrable} %then
	$br $tb
	[deferrable=] "true"
	[ defer-type=] "{defer-type}" $br
%end

%if {uq-constr} %and {nulls-not-distinct} %then
	[ nulls-not-distinct="true"]
%end

%if {pk-constr} %or {uq-constr} %or {fk-constr} %and {temporal-key} %then
	[ temporal-key="true"]
%end

%if {ck-constr} %and {no-inherit} %then
	[ no-inherit="true"]
%end

%if {ex-constr} %and {index-type} %then
	[ index-type=]"{index-type}"
%end

%if {factor} %then
	[ factor=] "{factor}"
%end

%if {protected} %then
	[ protected="true"]
%end

%if {sql-disabled} %then
	[ sql-disabled=] "true"
%end

%if {table} %then
	[ table=] "&{table}"
%end

> $br

%if {tablespace} %then $tb {tablespace} %end

%if {decl-in-table} %then $tb %end

%if {src-columns} %then
	$tb <columns $sp names="&{src-columns}" $sp ref-type="src-columns"/> $br
%end

%if {dst-columns} %then
	%if {decl-in-table} %then $tb %end
	$tb <columns $sp names="&{dst-columns}" $sp ref-type="dst-columns"/> $br
%end

%if {ex-constr} %then
	{elements}
%end

%if {ck-constr} %or {ex-constr} %then
	%if {expression} %then
		%if {decl-in-table} %then $tb %end
		$tb <expression> $sp <! $ob CDATA $ob {expression} $cb $cb > $sp </expression> $br
	%end
%end

%if {comment} %then $tb {comment} %end

%if {decl-in-table} %then $tb %end
</constraint> $br
%if %not {decl-in-table} %then $br %end
