# Template code for data dictionary generation
# CAUTION: Do not modify this file unless you know what you are doing.
# Code generation can be broken if incorrect changes are made.

%set {spc} \n \s

{spc} <tr>
{spc} <td> {name} </td>
{spc} <td \s class="data-type"> {type} </td>

%if ({parent} != "view") %then
	%if ({parent} == "table") %then
		{spc} <td \s class="bool-field"> {pk-constr} </td>
		{spc} <td \s class="bool-field"> {fk-constr} </td>
		{spc} <td \s class="bool-field"> {uq-constr} </td>
	%end

	{spc} <td \s class="bool-field"> {not-null} </td>
	{spc} <td \s class="value"> &{default-value} </td>
	{spc} <td><p \s class="max-td-wth"><em> {comment} </em></p></td>
%end

{spc} </tr>
