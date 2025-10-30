# XML definition for columns
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.
$tb [<column name=] "&{name}"

%if {alias} %then
	$sp alias="&{alias}"
%end

%if {not-null} %then
	[ not-null=] "true"
%end

%if {default-value} %and %not {sequence} %then
	[ default-value=] "&{default-value}"

	%if {generated} %then
		[ generated=] "true"
	%end
%end

%if {sequence} %then
	[ sequence=] "&{sequence}"
%end

%if {identity-type} %then
	$br $tb
	[ identity-type=] "{identity-type}"

	%if {cycle} %then [ cycle="true"] %end
	%if {start} %then [ start=] "{start}" %end
	%if {increment} %then [ increment=] "{increment}" %end
	%if {min-value} %then [ min-value=] "{min-value}" %end
	%if {max-value} %then [ max-value=] "{max-value}" %end
	%if {cache} %then [ cache=] "{cache}" %end
%end

%if {protected} %then
	[ protected=] "true"
%end

%if {sql-disabled} %then
	[ sql-disabled=] "true"
%end

> $br

$tb {type}

%if {collation} %then $tb {collation} %end

%if {comment} %then $tb {comment} %end

$tb </column> $br
