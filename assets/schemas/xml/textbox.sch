# XML definition for textboxes
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.
[<textbox name=] "&{name}" [ layers=] "{layers}"

%if {scene} %then [ scene=] "{scene}" %end
%if {protected} %then [ protected=] "true" %end
%if {italic} %then [ italic=] "true" %end
%if {bold} %then [ bold=] "true" %end
%if {underline} %then [ underline=] "true" %end
%if {color} %then [ color=] "{color}" %end
%if {faded-out} %then [ faded-out=] "true" %end
%if {z-value} %then [ z-value=] "{z-value}" %end
%if {width} %then [ width=] "{width}" %end

[ font-size=] "{font-size}"

> $br

{position}
{comment}
</textbox> $br $br
