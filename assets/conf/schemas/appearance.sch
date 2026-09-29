# XML definition for appearance settings.
# CAUTION: Do not modify this file unless you know what you are doing.
[<?xml version="1.0" encoding="UTF-8" ?>] \n
[<!--] \n
[ CAUTION: Do not modify this file directly on it's code unless you know what you are doing.] \n
[ Unexpected results may occur if the code is changed deliberately.] \n
[-->] \n

%set {spc} \t \t

\n <appearance 
\s ui-theme="{ui-theme}" 
\s icons-size="{icons-size}"

%if {custom-scale} %then
	\s custom-scale="{custom-scale}"
%end

%if {ui-font} %then
	\s ui-font="{ui-font}" \s ui-font-size="{ui-font-size}"
%end

> \n

\n \t <design
\s grid-size="{grid-size}"
\s grid-pattern="{grid-pattern}"
\s grid-color="{grid-color}"
\s canvas-color="{canvas-color}"
\s delimiters-color="{delimiters-color}" \n
{spc} 
\s min-object-opacity="{min-object-opacity}"
\s attribs-per-page="{attribs-per-page}"
\s ext-attribs-per-page="{ext-attribs-per-page}"
\s expansion-factor="{expansion-factor}"
[/>] \n

\n \t <code
\s font="&{font}" 
\s font-size="{font-size}"
\s tab-width="{tab-width}" \n
{spc} display-line-numbers=" %if {display-line-numbers} %then true %else false %end "
\s highlight-lines=" %if {highlight-lines} %then true %else false %end "
\s line-numbers-color="{line-numbers-color}" \n
{spc} line-numbers-bg-color="{line-numbers-bg-color}"
\s line-numbers-hl-color="{line-numbers-hl-color}"
\s line-highlight-color="{line-highlight-color}"
[/>] \n

\n \t <objects> \n
{spc} [<global font="] {global-font} [" size="] {global-font-size} ["/>] \n
{spc} [<font id="table-name" color="] {table-name-fcolor} [" italic="] {table-name-italic} [" bold="] {table-name-bold} [" underline="] {table-name-underline} ["/>] \n
{spc} [<font id="table-schema-name" color="] {table-schema-name-fcolor} [" italic="] {table-schema-name-italic} [" bold="] {table-schema-name-bold} [" underline="] {table-schema-name-underline} ["/>] \n
{spc} [<font id="column" color="] {column-fcolor} [" italic="] {column-italic} [" bold="] {column-bold} [" underline="] {column-underline} ["/>] \n
{spc} [<font id="object-type" color="] {object-type-fcolor} [" italic="] {object-type-italic} [" bold="] {object-type-bold} [" underline="] {object-type-underline} ["/>] \n
{spc} [<font id="fk-column" color="] {fk-column-fcolor} [" italic="] {fk-column-italic} [" bold="] {fk-column-bold} [" underline="] {fk-column-underline} ["/>] \n
{spc} [<font id="pk-column" color="] {pk-column-fcolor} [" italic="] {pk-column-italic} [" bold="] {pk-column-bold} [" underline="] {pk-column-underline} ["/>] \n
{spc} [<font id="uq-column" color="] {uq-column-fcolor} [" italic="] {uq-column-italic} [" bold="] {uq-column-bold} [" underline="] {uq-column-underline} ["/>] \n
{spc} [<font id="nn-column" color="] {nn-column-fcolor} [" italic="] {nn-column-italic} [" bold="] {nn-column-bold} [" underline="] {nn-column-underline} ["/>] \n
{spc} [<font id="prot-column" color="] {prot-column-fcolor} [" italic="] {prot-column-italic} [" bold="] {prot-column-bold} [" underline="] {prot-column-underline} ["/>] \n
{spc} [<font id="inh-column" color="] {inh-column-fcolor} [" italic="] {inh-column-italic} [" bold="] {inh-column-bold} [" underline="] {inh-column-underline} ["/>] \n
{spc} [<font id="index" color="] {index-fcolor} [" italic="] {index-italic} [" bold="] {index-bold} [" underline="] {index-underline} ["/>] \n
{spc} [<font id="rule" color="] {rule-fcolor} [" italic="] {rule-italic} [" bold="] {rule-bold} [" underline="] {rule-underline} ["/>] \n
{spc} [<font id="trigger" color="] {trigger-fcolor} [" italic="] {trigger-italic} [" bold="] {trigger-bold} [" underline="] {trigger-underline} ["/>] \n
{spc} [<font id="constraint" color="] {constraint-fcolor} [" italic="] {constraint-italic} [" bold="] {constraint-bold} [" underline="] {constraint-underline} ["/>] \n
{spc} [<font id="policy" color="] {policy-fcolor} [" italic="] {policy-italic} [" bold="] {policy-bold} [" underline="] {policy-underline} ["/>] \n
{spc} [<font id="view-name" color="] {view-name-fcolor} [" italic="] {view-name-italic} [" bold="] {view-name-bold} [" underline="] {view-name-underline} ["/>] \n
{spc} [<font id="view-schema-name" color="] {view-schema-name-fcolor} [" italic="] {view-schema-name-italic} [" bold="] {view-schema-name-bold} [" underline="] {view-schema-name-underline} ["/>] \n
{spc} [<font id="ref-table" color="] {ref-table-fcolor} [" italic="] {ref-table-italic} [" bold="] {ref-table-bold} [" underline="] {ref-table-underline} ["/>] \n
{spc} [<font id="ref-column" color="] {ref-column-fcolor} [" italic="] {ref-column-italic} [" bold="] {ref-column-bold} [" underline="] {ref-column-underline} ["/>] \n
{spc} [<font id="alias" color="] {alias-fcolor} [" italic="] {alias-italic} [" bold="] {alias-bold} [" underline="] {alias-underline} ["/>] \n
{spc} [<font id="attribute" color="] {attribute-fcolor} [" italic="] {attribute-italic} [" bold="] {attribute-bold} [" underline="] {attribute-underline} ["/>] \n
{spc} [<font id="label" color="] {label-fcolor} [" italic="] {label-italic} [" bold="] {label-bold} [" underline="] {label-underline} ["/>] \n
{spc} [<font id="constraints" color="] {constraints-fcolor} [" italic="] {constraints-italic} [" bold="] {constraints-bold} [" underline="] {constraints-underline} ["/>] \n
{spc} [<font id="pos-info" color="] {pos-info-fcolor} [" italic="] {pos-info-italic} [" bold="] {pos-info-bold} [" underline="] {pos-info-underline} ["/>] \n
{spc} [<font id="cutoff-marker" color="] {cutoff-marker-fcolor} [" italic="] {cutoff-marker-italic} [" bold="] {cutoff-marker-bold} [" underline="] {cutoff-marker-underline} ["/>] \n
{spc} [<font id="tag" color="] {tag-fcolor} [" italic="] {tag-italic} [" bold="] {tag-bold} [" underline="] {tag-underline} ["/>] \n
{spc} [<font id="foreigntable-name" color="] {foreigntable-name-fcolor} [" italic="] {foreigntable-name-italic} [" bold="] {foreigntable-name-bold} [" underline="] {foreigntable-name-underline} ["/>] \n
{spc} [<font id="foreigntable-schema-name" color="] {foreigntable-schema-name-fcolor} [" italic="] {foreigntable-schema-name-italic} [" bold="] {foreigntable-schema-name-bold} [" underline="] {foreigntable-schema-name-underline} ["/>] \n

{spc} [<object id="table-title" fill-color="] {table-title-color} [" border-color="] {table-title-bcolor} ["/>] \n
{spc} [<object id="table-body" fill-color="] {table-body-color} [" border-color="] {table-body-bcolor} ["/>] \n
{spc} [<object id="table-ext-body" fill-color="] {table-ext-body-color} [" border-color="] {table-ext-body-bcolor} ["/>] \n
{spc} [<object id="table-toggler-btns" fill-color="] {table-toggler-btns-color} [" border-color="] {table-toggler-btns-bcolor} ["/>] \n
{spc} [<object id="table-toggler-body" fill-color="] {table-toggler-body-color} [" border-color="] {table-toggler-body-bcolor} ["/>] \n
{spc} [<object id="label" fill-color="] {label-color} [" border-color="] {label-bcolor} ["/>] \n
{spc} [<object id="view-title" fill-color="] {view-title-color} [" border-color="] {view-title-bcolor} ["/>] \n
{spc} [<object id="view-body" fill-color="] {view-body-color} [" border-color="] {view-body-bcolor} ["/>] \n
{spc} [<object id="view-ext-body" fill-color="] {view-ext-body-color} [" border-color="] {view-ext-body-bcolor} ["/>] \n
{spc} [<object id="view-toggler-btns" fill-color="] {view-toggler-btns-color} [" border-color="] {view-toggler-btns-bcolor} ["/>] \n
{spc} [<object id="view-toggler-body" fill-color="] {view-toggler-body-color} [" border-color="] {view-toggler-body-bcolor} ["/>] \n
{spc} [<object id="textbox" fill-color="] {textbox-color} [" border-color="] {textbox-bcolor} ["/>] \n
{spc} [<object id="relationship" fill-color="] {relationship-color} [" border-color="] {relationship-bcolor} ["/>] \n
{spc} [<object id="attribute" fill-color="] {attribute-color} [" border-color="] {attribute-bcolor} ["/>] \n
{spc} [<object id="column" fill-color="] {column-color} [" border-color="] {column-bcolor} ["/>] \n
{spc} [<object id="pk-column" fill-color="] {pk-column-color} [" border-color="] {pk-column-bcolor} ["/>] \n
{spc} [<object id="uq-column" fill-color="] {uq-column-color} [" border-color="] {uq-column-bcolor} ["/>] \n
{spc} [<object id="fk-column" fill-color="] {fk-column-color} [" border-color="] {fk-column-bcolor} ["/>] \n
{spc} [<object id="nn-column" fill-color="] {nn-column-color} [" border-color="] {nn-column-bcolor} ["/>] \n
{spc} [<object id="reference" fill-color="] {reference-color} [" border-color="] {reference-bcolor} ["/>] \n
{spc} [<object id="rule" fill-color="] {rule-color} [" border-color="] {rule-bcolor} ["/>] \n
{spc} [<object id="trigger" fill-color="] {trigger-color} [" border-color="] {trigger-bcolor} ["/>] \n
{spc} [<object id="index" fill-color="] {index-color} [" border-color="] {index-bcolor} ["/>] \n
{spc} [<object id="constraint" fill-color="] {constraint-color} [" border-color="] {constraint-bcolor} ["/>] \n
{spc} [<object id="policy" fill-color="] {policy-color} [" border-color="] {policy-bcolor} ["/>] \n
{spc} [<object id="locker-body" fill-color="] {locker-body-color} [" border-color="] {locker-body-bcolor} ["/>] \n
{spc} [<object id="locker-arc" fill-color="] {locker-arc-color} [" border-color="] {locker-arc-bcolor} ["/>] \n
{spc} [<object id="obj-selection" fill-color="] {obj-selection-color} [" border-color="] {obj-selection-bcolor} ["/>] \n
{spc} [<object id="obj-shadow" fill-color="] {obj-shadow-color} ["/>] \n
{spc} [<object id="pos-info" fill-color="] {pos-info-color} [" border-color="] {pos-info-bcolor} ["/>] \n
{spc} [<object id="cutoff-marker" fill-color="] {cutoff-marker-color} [" border-color="] {cutoff-marker-bcolor} ["/>] \n
{spc} [<object id="tag" fill-color="] {tag-color} [" border-color="] {tag-bcolor} ["/>] \n
{spc} [<object id="placeholder" fill-color="] {placeholder-color} [" border-color="] {placeholder-bcolor} ["/>] \n
{spc} [<object id="foreigntable-title" fill-color="] {foreigntable-title-color} [" border-color="] {foreigntable-title-bcolor} ["/>] \n
{spc} [<object id="foreigntable-body" fill-color="] {foreigntable-body-color} [" border-color="] {foreigntable-body-bcolor} ["/>] \n
{spc} [<object id="foreigntable-ext-body" fill-color="] {foreigntable-ext-body-color} [" border-color="] {foreigntable-ext-body-bcolor} ["/>] \n
{spc} [<object id="foreigntable-toggler-btns" fill-color="] {foreigntable-toggler-btns-color} [" border-color="] {foreigntable-toggler-btns-bcolor} ["/>] \n
{spc} [<object id="foreigntable-toggler-body" fill-color="] {foreigntable-toggler-body-color} [" border-color="] {foreigntable-toggler-body-bcolor} ["/>] \n

\t </objects> \n
</appearance>
