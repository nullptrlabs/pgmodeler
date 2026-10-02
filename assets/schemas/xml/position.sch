# XML definition for object positioning
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.

$tb <position

%if {id} %then 
	[ id=] "&{id}"
%end

 [ x=] "{x}" [ y=] "{y}" /> $br
