# SQL definition for comments
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.

%if ({obj-type} != "genericsql") %and {comment} %or ({comment}=="unset") %then
	@include "ddlend"

	[COMMENT ON ] {sql-object} $sp 

	%if ({sql-object}=="CONSTRAINT") %or ({sql-object}=="TRIGGER") %or 
		({sql-object}=="RULE") %or ({sql-object}=="POLICY")  %then
		{name} [ ON ] {table}
	%else
		{signature} 
	%end

	[ IS ]

	%if ({comment}=="unset") %then
		''
	%else
		%if {escape-comment} %then E %end
		'{comment}'
	%end

	; 

	{ddl-end}
%end
