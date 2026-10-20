# XML definition for relationships configuration file
# CAUTION: Do not modify this file unless you know what you are doing.
[<?xml version="1.0" encoding="UTF-8" ?>] \n
[<!--] \n
[ CAUTION: Do not modify this file directly on it's code unless you know what you are doing.] \n
[ Unexpected results may occur if the code is changed deliberately.] \n
[-->] \n

<relationships \s link-mode="{link-mode}" \s line-style="{line-style}" > \n
    
\t [<foreign-keys deferrable=] "{deferrable}" 

[ defer-type=] "{defer-type}" 
[ upd-action=] "{upd-action}" 
[ del-action=] "{del-action}" 
[ fk-idx-type=] "{fk-idx-type}"

[/>] \n

\t <name-patterns> \n
{patterns}
\t </name-patterns> \n

</relationships> \n

