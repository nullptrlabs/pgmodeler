# XML definition for files opened on a session
# CAUTION: Do not modify this file unless you know what you are doing.
\t\t [<widget]

%if {validator} %then
	[ id="validator"]
	[ sql-validation="] %if {sql-validation} %then true %else false %end ["]
	[ use-unique-names="] %if {use-unique-names} %then true %else false %end ["]
	[ version="] {version} ["]
%else
	%if {objectfinder} %then
		[ id="objectfinder"]
		[ regexp="] %if {regexp} %then true %else false %end ["]
		[ case-sensitive="] %if {case-sensitive} %then true %else false %end ["]
		[ exact-match="] %if {exact-match} %then true %else false %end ["]
		[ select-objects="] %if {select-objects} %then true %else false %end ["]
		[ fadein-objects="] %if {fadein-objects} %then true %else false %end ["]
	%else
		%if {sqltool} %then
			[ id="sqltool"]
			[ show-attributes-grid="] %if {show-attributes-grid} %then true %else false %end ["]
			[ show-source-pane="] %if {show-source-pane} %then true %else false %end ["]
		%else
			%if {layersconfig} %then
				[ id="layersconfig"]
				[ rels-follow-tabs-visibility="] %if {rels-follow-tabs-visibility} %then true %else false %end ["]
				[ sel-objs-in-layers="] %if {sel-objs-in-layers} %then true %else false %end ["]
			%else
				%if {id} %then
					[ id="] {id} ["]
					[ x="] {x} ["]
					[ y="] {y} ["]

					%if {maximized} %then
						[ maximized="true"]
					%else
						[ width="] {width} ["]
						[ height="] {height} ["]
					%end
				%end
			%end
		%end
	%end
%end

/> \n
