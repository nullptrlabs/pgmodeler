# SQL definition for columns
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.

@include "ddlend"

%if {decl-in-table} %then
	$tb
%else
	[-- object: ] {name} [ | type: ] {sql-object} [ --] $br

	%if {table} %then
		[ALTER TABLE ] {table} [ ADD COLUMN ]
	%end
%end

{name} $sp {type}

%if {collation} %then
	[ COLLATE ]  {collation}
%end

%if {not-null} %then
	[ NOT NULL]
%end

%if {identity-type} %then
	[ GENERATED ] {identity-type} [ AS IDENTITY ]

	%if {increment} %or {min-value} %or {max-value} %or {start} %or {cache} %or {cycle} %then
		[(]
	%end

	%if {increment} %then
		[ INCREMENT BY ] {increment}
	%end

	%if {min-value} %then
		[ MINVALUE ] {min-value}
	%end

	%if {max-value} %then
		[ MAXVALUE ] {max-value}
	%end

	%if {start} %then
		[ START WITH ] {start}
	%end

	%if {cache} %then
		[ CACHE ] {cache}
	%end

	%if {cycle} %then
		[ CYCLE]
	%end

	%if {increment} %or {min-value} %or {max-value} %or {start} %or {cache} %or {cycle} %then
		[ )]
	%end
%else
	%if ({pgsql-ver} >=f "12.0") %and {generated} %then
		[ GENERATED ALWAYS AS (] {default-value} [) STORED]
	%else
		%if {default-value} %then
			[ DEFAULT ] {default-value}
		%end
	%end
%end

%if {decl-in-table} %then
	[,]
%else
	[;]

	{ddl-end} $br
%end

%if %not {decl-in-table} %and {comment} %then
	{comment} $br
%end

$br
