# XML definition for database model
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.
<?xml $sp version="1.0" $sp encoding="UTF-8"?> $br
[<!--] $br
[CAUTION: Do not modify this file unless you know what you are doing.] $br
[ Unexpected results may occur if the code is changed deliberately.] $br
[-->] $br
<dbmodel [ pgmodeler-ver=] "{pgmodeler-ver}" [ use-changelog]= "{use-changelog}"

%if {author} %then 
	[ author=] "&{author}" 
%end

%if {protected} %then
	[ protected=] "true"
%end

%if {max-obj-count} %then
	[ max-obj-count=] "{max-obj-count}"
%end

$br $tb 

[ gen-disabled-objs-code=] "{gen-disabled-objs-code}"

[ show-sys-schemas-rects=] "{show-sys-schemas-rects}"

[ seg-mode=] "{seg-mode}"

[ seg-threshold=] "{seg-threshold}"

%if {seg-obj-count} %then
	[ seg-obj-count=] "{seg-obj-count}"
%end

%if {last-position} %or {last-zoom} %or {scene-rect} %or {last-scene} %then
	$br $tb

	%if {last-position} %then
		[ last-position=] "{last-position}"
	%end

	%if {last-zoom} %then
		[ last-zoom=] "{last-zoom}"
	%end

	%if {scene-rect} %then
		[ scene-rect=] "{scene-rect}"
	%end

	%if {last-scene} %then
		[ last-scene=] "{last-scene}"
	%end
%end

%if {default-schema} %or {default-owner} %then
	$br $tb

	%if {default-schema} %then
		[ default-schema=] "&{default-schema}"
	%end

	%if {default-owner} %then
		[ default-owner=] "&{default-owner}"
	%end
%end

%if {default-collation} %or {default-tablespace} %then
	$br $tb

	%if {default-collation} %then
		[ default-collation=] "&{default-collation}"
	%end

	%if {default-tablespace} %then
		[ default-tablespace=] "&{default-tablespace}"
	%end
%end

%if {layers} %then
	$br $tb
	[ layers]="&{layers}"

	$br $tb [ active-layers]=

	%if {active-layers} %then
		"{active-layers}"
	%else
		""
	%end

	$br $tb
	[ layer-name-colors]="{layer-name-colors}"

	$br $tb
	[ layer-rect-colors]="{layer-rect-colors}"

	$br $tb
	[ show-layer-names]="{show-layer-names}"
	[ show-layer-rects]="{show-layer-rects}"
%end

> $br
%if {scenes} %then {scenes} %end
%if {objects} %then {objects} %end
%if {permission} %then {permission} %end
%if {changelog} %then {changelog} %end
</dbmodel> $br
