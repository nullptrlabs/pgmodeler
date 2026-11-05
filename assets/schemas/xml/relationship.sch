# XML definition for relationships
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.
[<relationship name=] "&{name}" [ type=] "{type}"

%if {layers} %then [ layers=] "{layers}" %end

%if {alias} %then
	$sp alias="&{alias}"
%end

%if {src-col-pattern} %or {dst-col-pattern} %then
	$br $tb
	%if {src-col-pattern} %then [ src-col-pattern=] "&{src-col-pattern}" %end
	%if {dst-col-pattern} %then [ dst-col-pattern=] "&{dst-col-pattern}" %end
%end

%if {pk-pattern} %or {uq-pattern} %then
	$br $tb
	%if {pk-pattern} %then [ pk-pattern=] "&{pk-pattern}" %end
	%if {uq-pattern} %then [ uq-pattern=] "&{uq-pattern}" %end
%end

%if {src-fk-pattern} %or {dst-fk-pattern} %then
	$br $tb
	%if {src-fk-pattern} %then [ src-fk-pattern=] "&{src-fk-pattern}" %end
	%if {dst-fk-pattern} %then [ dst-fk-pattern=] "&{dst-fk-pattern}" %end
%end

%if {pk-col-pattern} %then
	$br $tb [ pk-col-pattern=] "&{pk-col-pattern}"
%end

%if {fk-idx-type} %or {fk-idx-pattern} %then
	$br $tb
	%if {fk-idx-type} %then [ fk-idx-type=] "{fk-idx-type}" %end
	%if {fk-idx-pattern} %then [ fk-idx-pattern=] "&{fk-idx-pattern}" %end
%end

%if {custom-color} %then
	$br $tb [ custom-color=] "{custom-color}"
%end

%if {sql-disabled} %then
	$br $tb [ sql-disabled=] "true"
%end

%if {protected} %then
	$br [ protected=] "true"
%end

%if {faded-out} %then
	[ faded-out=] "true"
%end

$br $tb [ src-table=] "&{src-table}"
$br $tb [ dst-table=] "&{dst-table}"

%if {reference-fk} %then
	[ reference-fk=] "&{reference-fk}"
%end


%if %not {relgen} %and %not {reldep} %and %not {relpart} %then
	$br $tb
	[ src-required=] %if {src-required} %then "true" %else "false" %end
	[ dst-required=] %if {dst-required} %then "true" %else "false" %end

	%if {identifier} %then
		$br $tb $sp identifier="true" $br
	%end

	%if {single-pk-col} %then
		$br $tb [ single-pk-col=] "true"
	%end

	%if {table-name} %then
		$br $tb $sp table-name="&{table-name}"
	%end

	%if {deferrable} %then
		$br
		$tb $sp deferrable="true"
		$sp defer-type="{defer-type}"
	%end

	%if {upd-action} %then
		$br $tb upd-action= "{upd-action}"
	%end

	%if {del-action} %then
		$br $tb del-action= "{del-action}"
	%end
%else
	%if {copy-mode} %then
		$tb copy-mode="{copy-mode}" $sp copy-options="{copy-options}"
	%end
%end

%if {reduced-form} %then
	/> $br $br
%else
	> $br

	%if {points} %then $tb <line> $br $tb {points} $tb </line> $br %end
	%if {labels-pos} %then {labels-pos} %end
	%if {segments} %then {segments} %end

	%if %not {relgen} %and %not {reldep} %and %not {relpart} %then
		%if {columns} %then {columns} %end
		%if {constraints} %then {constraints} %end
	%end

	%if {original-pk} %then
		{original-pk}
	%end

	%if {special-pk-cols} %then
		$tb [<special-pk-cols indexes=]"{special-pk-cols}"/> $br
	%end

	%if {partition-bound-expr} %then
		$tb <expression> $sp <! $ob CDATA $ob {partition-bound-expr} $cb $cb > $sp </expression> $br
	%end

	</relationship> $br $br
%end
