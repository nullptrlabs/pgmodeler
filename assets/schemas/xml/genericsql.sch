# XML definition for generic sql objects
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.
[<genericsql name=] "&{name}"

%if {protected} %then
	[ protected=] "true"
%end

%if {sql-disabled} %then
	[ sql-disabled=] "true"
%end

> $br

%if {references} %then 
	{references} 
%end

$tb <definition> $sp <! $ob CDATA $ob {definition} $cb $cb > $sp </definition> $br

%if {comment} %then
	{comment}
%end

</genericsql> $br $br
