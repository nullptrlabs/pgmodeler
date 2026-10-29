# XML definition for views
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.
[<view name=] "&{name}"

%if {layers} %then [ layers=] "{layers}" %end
%if {scene} %then [ scene=] "{scene}" %end
%if {alias} %then [ alias=] "&{alias}" %end
%if {protected} %then [ protected=] "true" %end
%if {materialized} %then [ materialized=] "true" %end
%if {recursive} %then [ recursive=] "true" %end
%if {with-no-data} %then [ with-no-data=] "true" %end
%if {check-option} %then [ check-option=] "{check-option}" %end
%if {security-invoker} %then [ security-invoker=] "{security-invoker}" %end
%if {security-barrier} %then [ security-barrier=] "{security-barrier}" %end
%if {sql-disabled} %then [ sql-disabled=] "true" %end
%if {collapse-mode} %then [ collapse-mode=] "{collapse-mode}" %end
%if {pagination} %then [ pagination=] "true" %end
%if {attribs-page} %then [ attribs-page=] "{attribs-page}" %end
%if {ext-attribs-page} %then [ ext-attribs-page=] "{ext-attribs-page}" %end
%if {faded-out} %then [ faded-out=] "true" %end
%if {max-obj-count} %then [ max-obj-count=] "{max-obj-count}" %end
%if {z-value} %then [ z-value=] "{z-value}" %end
> $br

{schema}
%if {owner} %then {owner} %end
%if {tablespace} %and {materialized} %then {tablespace} %end
%if {comment} %then {comment} %end
%if {tag} %then {tag} %end
%if {appended-sql} %then {appended-sql} %end
%if {prepended-sql} %then {prepended-sql} %end

{position}
%if {rel-marker-pos} %then {rel-marker-pos} %end

%if {references} %then {references} %end

%if {columns} %then {columns} %end

%if {definition} %then 
	$tb <definition> $sp <! $ob CDATA $ob {definition} $cb $cb > $sp </definition> $br
%end

</view> $br $br
