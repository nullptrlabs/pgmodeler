/*
# PostgreSQL Database Modeler (pgModeler)
#
# Copyright 2006-2025 - Raphael Araújo e Silva <raphael@pgmodeler.io>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# The complete text of GPLv3 is at LICENSE file on source code root directory.
# Also, you can get the complete GNU General Public License at <http://www.gnu.org/licenses/>
*/
#include "exception.h"
#include "enumtype.h"
#include <QApplication>

QString Exception::messages[Exception::ErrorCount][2]={
	{"Custom", " "},
	{"AsgPseudoTypeColumn", QT_TR_NOOP("Assignment of a pseudo-type to the column type!")},
	{"AsgInvalidPrecision", QT_TR_NOOP("Assignment of a precision greater than the type length!")},
	{"AsgInvalidPrecisionTimestamp", QT_TR_NOOP("Assignment of an invalid precision to type time, timestamp or interval. The precision in this case must be equal to or less than 6!")},
	{"AsgNotAllocatedColumn", QT_TR_NOOP("Assignment of a non-allocated column to object `%1' (%2)!")},
	{"RefColumnInvalidIndex", QT_TR_NOOP("Reference to a column with an index that is out of the column list capacity!")},
	{"AsgNotAllocattedObject", QT_TR_NOOP("Assignment of non-allocated object!")},
	{"AsgNotAllocatedSchema", QT_TR_NOOP("Assignment of a non-allocated schema to object `%1' (%2)!")},
	{"AsgObjectInvalidDefinition", QT_TR_NOOP("The object `%1' (%2) has an inconsistent SQL or XML definition!")},
	{"AsgDuplicatedObject", QT_TR_NOOP("The object `%1' (%2) already exists in `%3' (%4)!")},
	{"AsgDuplicatedObjectContainer", QT_TR_NOOP("The object `%1' (%2) cannot be assigned because it already exists in the container object `%3'!")},
	{"AsgObjectInvalidType", QT_TR_NOOP("Assignment of an object with invalid type!")},
	{"RemObjectInvalidType", QT_TR_NOOP("Removal of an object with invalid type!")},
	{"ObtObjectInvalidType", QT_TR_NOOP("Retrieval of an object with invalid type!")},
	{"AsgEmptyNameTableReturnType", QT_TR_NOOP("Assignment of empty name to table return type!")},
	{"AsgDuplicatedParameterFunction", QT_TR_NOOP("The parameter `%1' cannot be inserted because another parameter with the same name already exists in function `%2'!")},
	{"InsDuplicatedTableReturnType", QT_TR_NOOP("The table return type `%1' cannot be inserted because another return type with the same name already exists in `%2'!")},
	{"RefParameterInvalidIndex", QT_TR_NOOP("Reference to a parameter with an index that is out of the parameter list bounds!")},
	{"RefInvalidTriggerEvent", QT_TR_NOOP("Reference to an event that does not belong to the trigger!")},
	{"AsgInvalidColumnTrigger", QT_TR_NOOP("The column `%1' cannot be assigned to trigger `%2' because they belong to different parent tables!")},
	{"AsgNotAllocatedFunction", QT_TR_NOOP("Assignment of a non-allocated function to object `%1' (%2)!")},
	{"AsgInvalidTriggerFunction", QT_TR_NOOP("Assignment of a function with a return type different from `%1'!")},
	{"AsgFunctionInvalidParamCount", QT_TR_NOOP("Assignment of a function with invalid parameter count to object `%1' (%2)!")},
	{"AsgFunctionInvalidLanguage", QT_TR_NOOP("Assignment of a function with invalid language!")},
	{"AsgEventTriggerFuncInvalidLang", QT_TR_NOOP("Event trigger function must be coded in any language other than SQL!")},
	{"AsgNotAllocatedTable", QT_TR_NOOP("Assignment of non-allocated table to object `%1' (%2)!")},
	{"RefArgumentInvalidIndex", QT_TR_NOOP("Reference to an argument with an index that is out of the argument list bounds!")},
	{"AsgEmptyNameObject", QT_TR_NOOP("Assignment of empty name to an object!")},
	{"AsgInvalidNameObject", QT_TR_NOOP("Assignment of a name that contains invalid characters!")},
	{"AsgLongNameObject", QT_TR_NOOP("Assignment of a name with length exceeding the maximum of 63 characters!")},
	{"AsgInvalidSchemaObject", QT_TR_NOOP("Assignment of schema object with invalid type!")},
	{"AsgInvalidTablespaceObject", QT_TR_NOOP("Assignment of tablespace object with invalid type!")},
	{"AsgTablespaceInvalidObject", QT_TR_NOOP("Assignment of tablespace to an invalid object!")},
	{"AsgTablespaceInvalidConstraintType", QT_TR_NOOP("Assignment of tablespace to a constraint with invalid type! To belong to a tablespace, the constraint must be a primary key or unique!")},
	{"AsgInvalidRoleObject", QT_TR_NOOP("Assignment of owner object with invalid type!")},
	{"AsgRoleObjectInvalidType", QT_TR_NOOP("Assignment of owner to an invalid object!")},
	{"AsgCustomSQLObjectInvalidType", QT_TR_NOOP("Assignment of appended or prepended SQL to an invalid object!")},
	{"RefFunctionInvalidType", QT_TR_NOOP("Reference to a function with invalid type!")},
	{"RefOperatorArgumentInvalidType", QT_TR_NOOP("Reference to an operator argument with invalid type!")},
	{"RefOperatorInvalidType", QT_TR_NOOP("Reference to an operator with invalid type!")},
	{"AsgValueInvalidRoleOptionType", QT_TR_NOOP("Assignment of value to an invalid role option type!")},
	{"RefInvalidRoleType", QT_TR_NOOP("Reference to an invalid role type!")},
	{"InsDuplicatedRole", QT_TR_NOOP("The role `%1' cannot be inserted because it is already being referenced by role `%2'!")},
	{"AsgRoleReferenceRedundancy", QT_TR_NOOP("The role `%1' cannot be assigned as a member of `%2' because it will generate membership redundancy since `%3' is already in `%4'!")},
	{"AsgRoleMemberItself", QT_TR_NOOP("The role `%1' cannot be listed as a member of itself!")},
	{"RefRoleInvalidIndex", QT_TR_NOOP("Reference to a role with an index that is out of the role list bounds!")},
	{"InsEmptyRuleCommand", QT_TR_NOOP("Insertion of empty command to the rule!")},
	{"RefRuleCommandInvalidIndex", QT_TR_NOOP("Reference to a command with an index that is out of the command list bounds!")},
	{"InvInheritCopyPartRelationship", QT_TR_NOOP("Cannot create a self generalization/copy/partition relationship! The table cannot inherit or copy its own attributes or be a partition of itself!")},
	{"AsgObjectBelongsAnotherTable", QT_TR_NOOP("Assignment of an object that already belongs to another table!")},
	{"AsgSchemaSequenceDiffersTableSchema", QT_TR_NOOP("Assignment of a schema to the sequence that differs from the owner table's schema!")},
	{"AsgInvalidValueSequenceAttributes", QT_TR_NOOP("Assignment of an invalid value to one of the sequence attributes!")},
	{"AsgInvalidSequenceMinValue", QT_TR_NOOP("Assignment of a minimum value to the sequence that is greater than the maximum value!")},
	{"AsgInvalidSequenceStartValue", QT_TR_NOOP("Assignment of a start value to the sequence that extrapolates the range defined by minimum and maximum values!")},
	{"AsgInvalidSequenceIncrementValue", QT_TR_NOOP("Assignment of a null increment value to the sequence!")},
	{"AsgInvalidSequenceCacheValue", QT_TR_NOOP("Assignment of null cache value to the sequence!")},
	{"AsgSeqOwnerTableDifferentSchema", QT_TR_NOOP("Assignment of owner table that is not in the same schema as sequence `%1'!")},
	{"AsgSeqOwnerTableDifferentRole", QT_TR_NOOP("Assignment of owner table that does not belong to the same owner as sequence `%1'!")},
	{"AsgInexistentSeqOwnerColumn", QT_TR_NOOP("Assignment of a nonexistent owner column to sequence `%1'!")},
	{"AsgInvalidSeqOwnerColumn", QT_TR_NOOP("Assignment of an owner column to sequence `%1' that is not related to any table!")},
	{"RefLabelInvalidIndex", QT_TR_NOOP("Reference to a label with an index that is out of the label list bounds!")},
	{"AllocationObjectInvalidType", QT_TR_NOOP("Allocation of an object with invalid type!")},
	{"AsgFunctionInvalidReturnType", QT_TR_NOOP("Assignment of a function with invalid return type to object `%1' (%2)!")},
	{"AsgFunctionInvalidParameters", QT_TR_NOOP("Assignment of a function with invalid parameter type(s) to object `%1' (%2)!")},
	{"AsgNotAllocatedLanguage", QT_TR_NOOP("Assignment of non-allocated language!")},
	{"AsgInvalidLanguageObject", QT_TR_NOOP("Assignment of language object with invalid type!")},
	{"RefTypeInvalidIndex", QT_TR_NOOP("Reference to data type with an index outside the data type list capacity!")},
	{"AsgNullTypeObject", QT_TR_NOOP("Assignment of a null type to object `%1' (%2)!")},
	{"AsgInvalidTypeObject", QT_TR_NOOP("Assignment of invalid type to the object!")},
	{"AsgEmptyDirectoryName", QT_TR_NOOP("Assignment of an empty directory to tablespace `%1'!")},
	{"ObtTypesInvalidQuantity", QT_TR_NOOP("Retrieval of types with invalid quantity!")},
	{"InsDuplicatedItems", QT_TR_NOOP("Insertion of item that already exists in the type's attribute list!")},
	{"InsInvalidTypeAttribute", QT_TR_NOOP("Insertion of invalid item in the type's attribute list!")},
	{"InsDuplicatedEnumerationItem", QT_TR_NOOP("Insertion of item that already exists in the type's enumeration list!")},
	{"InsInvalidEnumerationItem", QT_TR_NOOP("Insertion of invalid item in the type's enumeration list!")},
	{"RefAttributeInvalidIndex", QT_TR_NOOP("Reference to an attribute with an index that is out of the attribute list bounds!")},
	{"RefEnumerationInvalidIndex", QT_TR_NOOP("Reference to an enumeration with an index that is out of the enumeration list bounds!")},
	{"AsgInvalidTypeConfiguration", QT_TR_NOOP("Assignment of invalid configuration to the type!")},
	{"AsgInvalidOperatorArguments", QT_TR_NOOP("Assignment of an operator with invalid input type count to aggregate function!")},
	{"AsgInvalidOperatorTypes", QT_TR_NOOP("Assignment of an operator with invalid argument types!")},
	{"AsgReservedName", QT_TR_NOOP("Assignment of system reserved name to object `%1' (%2)!")},
	{"AsgFunctionInvalidConfiguration", QT_TR_NOOP("A function with invalid configuration is being used by object `%1' (%2)!")},
	{"AsgInvalidSupportStrategyNumber", QT_TR_NOOP("Assignment of an invalid strategy/support number to an operator class element!")},
	{"InsDuplicatedElement", QT_TR_NOOP("Insertion of element that already exists in the element list!")},
	{"RefElementInvalidIndex", QT_TR_NOOP("Reference to an element with an index that is out of the element list bounds!")},
	{"RefObjectInvalidIndex", QT_TR_NOOP("Reference to an object with an index that is out of the object list bounds!")},
	{"RemNotAllocatedObject", QT_TR_NOOP("Removal of a non-allocated object!")},
	{"RemDirectReference", QT_TR_NOOP("The object `%1' (%2) cannot be removed because it is being referenced by object `%3' (%4)!")},
	{"RemInderectReference", QT_TR_NOOP("The object `%1' (%2) cannot be removed because it is being referenced by object `%3' (%4) that belongs to `%5' (%6)!")},
	{"OprObjectInvalidType", QT_TR_NOOP("Operation with object(s) of invalid type(s)!")},
	{"RefObjectInvalidType", QT_TR_NOOP("Reference to object with invalid type!")},
	{"OprNotAllocatedObject", QT_TR_NOOP("Operation with non-allocated object!")},
	{"InvLinkTablesNoPrimaryKey", QT_TR_NOOP("The relationship `%1' between tables `%2' and `%3' cannot be created because one does not have a primary key. If the relationship is of type n-n, both tables must have primary keys!")},
	{"NotImplementedRelationshipType", QT_TR_NOOP("The 1-1 relationship type where both tables have mandatory participation is not implemented because it requires table fusion that breaks the user's modeling!")},
	{"AsgInvalidExpressionObject", QT_TR_NOOP("Assignment of an invalid expression to the object!")},
	{"AsgExistingPrimaryKeyTable", QT_TR_NOOP("Assignment of a primary key to a table that already has one!")},
	{"InvIdentifierRelationship", QT_TR_NOOP("Identifier relationship cannot be created for a self relationship, n-n relationships, copy or generalization!")},
	{"InvCopyRelationshipDuplicCols", QT_TR_NOOP("Unable to create a copy relationship because column `%1' in table `%2' already exists in table `%3'!")},
	{"InvInheritRelationshipIncompCols", QT_TR_NOOP("Unable to create the generalization relationship because column `%1' in table `%2' cannot be merged with column `%3' of table `%4' because they have incompatible types!")},
	{"InvInheritRelationshipIncompConstrs", QT_TR_NOOP("Unable to create the generalization relationship because constraint `%1' in table `%2' cannot be merged with constraint `%3' of table `%4' due to their incompatible composition!")},
	{"AsgObjectInvalidRelationshipType", QT_TR_NOOP("An attribute cannot be added to a copy, generalization or partitioning relationship!")},
	{"AsgForeignKeyRelationship", QT_TR_NOOP("A foreign key cannot be added to a relationship because it is created automatically when the relationship is connected!")},
	{"RefObjectInexistsModel", QT_TR_NOOP("The object `%1' (%2) is referencing object `%3' (%4) which was not found in the model!")},
	{"RefUserTypeInexistsModel", QT_TR_NOOP("Reference to a user-defined data type that does not exist in the model!")},
	{"AsgInvalidMaxSizeOpList", QT_TR_NOOP("Assignment of invalid maximum size to operation list!")},
	{"FileDirectoryNotWritten", QT_TR_NOOP("Unable to write file or directory `%1'! Make sure that the path exists and the user has write permissions over it!")},
	{"FileNotWrittenInvalidDefinition", QT_TR_NOOP("Unable to write file `%1' due to one or more errors in the definition generation process!")},
	{"InsDuplicatedRelationship", QT_TR_NOOP("There is already a relationship between `%1' (%2) and `%3' (%4) in the model! When using generalization, copy and one-to-one relationships, there cannot be other relationships linked to the pair of tables.")},
	{"InsRelationshipRedundancy", QT_TR_NOOP("The configuration of relationship `%1' generates redundancy between relationships `%2'. Redundancy on identifier or generalization/copy relationships is not accepted since they result in incorrect column spreading making the model inconsistent!")},
	{"RemInvalidatedObjects", QT_TR_NOOP("One or more objects were invalidated and automatically removed because they were referencing table columns that were included through relationships and no longer exist due to relationship disconnection or exclusion of such generated columns!")},
	{"InvPrimaryKeyAllocation", QT_TR_NOOP("The primary key `%1' can only be allocated if declared within a code block that defines a table or relationship!")},
	{"RefInvalidPrivilegeType", QT_TR_NOOP("Reference to an invalid privilege type!")},
	{"InsDuplicatedRolePermission", QT_TR_NOOP("Insertion of a role that already exists in the permission's role list!")},
	{"AsgInvalidPrivilegeObject", QT_TR_NOOP("Assignment of privilege incompatible with the object type referenced by permission!")},
	{"AsgDuplicatedPermission", QT_TR_NOOP("There is already a permission on object `%1' (%2) that has one or more equal roles from those present on the permission to be assigned to the object!")},
	{"PermissionRefInexistObject", QT_TR_NOOP("A permission is referencing object `%1' (%2) which was not found in the model!")},
	{"InvObjectAllocationNoSchema", QT_TR_NOOP("The object `%1' (%2) cannot be created because it is not being assigned to any schema!")},
	{"AsgTablespaceDuplicatedDirectory", QT_TR_NOOP("The tablespace `%1' cannot be inserted into the model because it points to the same directory as tablespace `%2'!")},
	{"AsgInvalidSequenceTypeArray", QT_TR_NOOP("Cannot create array sequences (dimension >= 1)! PostgreSQL does not yet implement this feature!")},
	{"AsgSourceCodeFuncCLanguage", QT_TR_NOOP("The function `%1' cannot get a source code as definition because its language is set to C. Use the symbol and dynamic library attributes instead!")},
	{"AsgRefLibraryFuncLanguageNotC", QT_TR_NOOP("The function `%1' can have the symbol and dynamic library attributes configured only if the language is set to C. For all other cases you must specify a source code that defines it in the DBMS!")},
	{"AsgInvalidCommutatorOperator", QT_TR_NOOP("The operator `%1' cannot be assigned as a commutator of operator `%2' because it has incompatible settings!")},
	{"AsgInvalidNegatorOperator", QT_TR_NOOP("The operator `%1' cannot be assigned as negator of operator `%2' because it has incompatible settings!")},
	{"InvUserTypeSelfReference", QT_TR_NOOP("The type `%1' cannot self-reference in the `element' or `copy type' attributes or be used as a data type of an attribute in the configuration of a composite type!")},
	{"AsgInvalidElementType", QT_TR_NOOP("Assignment of invalid element to type `%1'!")},
	{"AsgInvalidAlignmentType", QT_TR_NOOP("Assignment of invalid alignment to type `%1'!")},
	{"AsgInvalidNameTableRelNN",  QT_TR_NOOP("Assignment of invalid name to the table generated from N-N relationship!")},
	{"InvUseSpecialPrimaryKey", QT_TR_NOOP("The relationship `%1' cannot make use of the special primary key because it is marked as identifier or it is a self relationship!")},
	{"OprRelationshipAddedObject", QT_TR_NOOP("The object `%1' (%2) cannot be edited or deleted because it was automatically included through a relationship! If the object is an attribute or constraint, the modifications must be done on the relationship editing form.")},
	{"RemProtectedObject", QT_TR_NOOP("The object `%1' (%2) cannot be deleted because it is protected!")},
	{"InvGroupDeclaration", QT_TR_NOOP("The group `%1' cannot be built in the groups declaration block (%2)!")},
	{"DefEmptyGroup", QT_TR_NOOP("The group `%1' cannot be built without possessing child elements!")},
	{"DefDuplicatedGroup", QT_TR_NOOP("The group `%1' cannot be built once more because this was done in previous blocks!")},
	{"RefColObjectTabInvalidIndex", QT_TR_NOOP("Reference to a column of the objects table with invalid index!")},
	{"RefRowObjectTabInvalidIndex", QT_TR_NOOP("Reference to a row of the objects table with invalid index!")},
	{"OprReservedObject", QT_TR_NOOP("The object `%1' (%2) cannot be manipulated because it is a protected object that is present in the database model only to be used as dependency of other objects!")},
	{"InvFuncConfigInvalidatesObject", QT_TR_NOOP("The new function configuration invalidates object `%1' (%2)! In this case it is necessary to undo the relationship between the affected object and function in order for the new configuration to take effect!")},
	{"InvSQLScopeViewReference", QT_TR_NOOP("A view reference should be used in at least one of these SQL scopes: View Definition, SELECT, FROM, WHERE or GROUP/HAVING!")},
	{"InvConstratintNoColumns", QT_TR_NOOP("Constraints like primary key, foreign key or unique must have at least one column related to them! For foreign keys, the referenced columns must also be selected!")},
	{"ConfigurationNotLoaded", QT_TR_NOOP("Unable to load the configuration file `%1'! Please check if the file exists in its folder and/or if it is not corrupted!")},
	{"DefaultConfigNotRestored", QT_TR_NOOP("Could not find the default settings file `%1'! To restore default settings, check the file existence and try again!")},
	{"PluginNotLoaded", QT_TR_NOOP("Could not load the plugin `%1' from library `%2'! Message returned by plugin manager: `%3'")},
	{"PluginsNotLoaded", QT_TR_NOOP("One or more plugins were not activated due to errors during the loading process! Check the exception stack for more details.")},
	{"InvalidSyntax", QT_TR_NOOP("Invalid syntax in file `%1', line %2, column %3!")},
	{"InvalidInclude", QT_TR_NOOP("Invalid include statement in file `%1', line %2, column %3!")},
	{"InvalidInstruction", QT_TR_NOOP("Invalid instruction `%1' in file `%2', line %3, column %4!")},
	{"UnkownAttribute", QT_TR_NOOP("Unknown attribute `%1' in file `%2', line %3, column %4!")},
	{"InvalidMetacharacter", QT_TR_NOOP("Invalid metacharacter `%1' in file `%2', line %3, column %4!")},
	{"InvalidOperatorInExpression", QT_TR_NOOP("Invalid operator `%1' in comparison expression, file `%2', line %3, column %4!")},
	{"UndefinedAttributeValue", QT_TR_NOOP("Attribute `%1' with an undefined value in file `%2', line %3, column %4!")},
	{"InvalidAttribute", QT_TR_NOOP("Attribute `%1' with an invalid name in file `%2', line %3, column %4!")},
	{"AsgEmptyXMLBuffer", QT_TR_NOOP("Assignment of empty XML buffer to parser!")},
	{"FileDirectoryNotAccessed", QT_TR_NOOP("Could not access file or directory `%1'! Make sure that it exists or if the user has access permissions on it!")},
	{"AsgEmptyDTDFile", QT_TR_NOOP("Assignment of empty DTD file name!")},
	{"AsgEmptyDTDName", QT_TR_NOOP("Assignment of empty name to the DTD declaration!")},
	{"LibXMLError", QT_TR_NOOP("Error while interpreting XML buffer at line %1 column %2.\nMessage generated by the parser: %3. %4")},
	{"OprNotAllocatedElementTree", QT_TR_NOOP("Operation on unallocated element tree! It is necessary to load the XML parser buffer and interpret it so that the tree is generated!")},
	{"InvModelFileNotLoaded", QT_TR_NOOP("Could not load file `%1'. The file appears to be inconsistent or one of its dependencies (DTD files) has errors or is missing!")},
	{"OprNotAllocatedElement", QT_TR_NOOP("Operation with unallocated tree element!")},
	{"OprInexistentElement", QT_TR_NOOP("Operation with element that does not exist in the currently loaded element tree!")},
	{"AsgInvalidConnParameter", QT_TR_NOOP("Assignment of a value to an invalid connection parameter!")},
	{"OprNotAllocatedConnection", QT_TR_NOOP("Operation on connection not established!")},
	{"ConnectionNotConfigured", QT_TR_NOOP("Attempt to connect without defining configuration parameters!")},
	{"ConnectionAlreadyStablished", QT_TR_NOOP("Attempt to start a connection already established!")},
	{"ConnectionNotStablished", QT_TR_NOOP("Could not connect to the database.\nMessage returned: `%1'")},
	{"AsgNotAllocatedSQLResult", QT_TR_NOOP("Assignment of non-allocated SQL command result!")},
	{"IncomprehensibleDBMSResponse", QT_TR_NOOP("Unable to allocate the SQL command result because the DBMS response was not understood by the client!")},
	{"DBMSFatalError", QT_TR_NOOP("Unable to allocate command result for the SQL because the server has generated a fatal error!\nMessage returned by the DBMS: `%1'")},
	{"RefTupleColumnInvalidIndex", QT_TR_NOOP("Reference to a tuple column with invalid index!")},
	{"RefTupleColumnInvalidName", QT_TR_NOOP("Reference to a tuple column with invalid name!")},
	{"RefInvalidTuple", QT_TR_NOOP("Reference to a tuple with an invalid index or the result is empty (no tuples)!")},
	{"RefInvalidTupleColumn", QT_TR_NOOP("Reference to a tuple column that was not yet initialized (tuple navigation not started)!")},
	{"SQLCommandNotExecuted", QT_TR_NOOP("Could not execute the SQL command.\n Message returned: `%1'")},
	{"AsgInvalidViewDefExpression", QT_TR_NOOP("Invalid use of a view reference as whole SQL definition! The assigned reference must be an expression!")},
	{"AsgSecondViewDefExpression", QT_TR_NOOP("Assignment of a second definition expression to the view!")},
	{"MixingViewDefExprsReferences", QT_TR_NOOP("Cannot mix ordinary references (SELECT-FROM, FROM-WHERE, After WHERE) with references used as view SQL definition!")},
	{"AsgInvalidCollationObject", QT_TR_NOOP("Assignment of collation object with invalid type!")},
	{"UnsupportedPKColsAddedByRel", QT_TR_NOOP("At the moment pgModeler does not support the creation of primary keys with some columns generated by relationship connection. To create primary keys with this feature you can use the `Identifier' field or the `Primary key' tab on the relationship editing form!")},
	{"EmptyLCCollationAttributes", QT_TR_NOOP("Collations must be created with at least the LC_COLLATE and LC_CTYPE attributes defined!")},
	{"ObjectReferencingItself", QT_TR_NOOP("The object `%1' (%2) cannot reference itself! This operation is not permitted for this kind of object!")},
	{"AsgInvalidOpFamilyOpClassElem", QT_TR_NOOP("Only operator families that use `btree' as indexing method are accepted by operator class elements!")},
	{"RefInvalidLikeOptionType", QT_TR_NOOP("Reference to an invalid copy table option!")},
	{"InvCopyRelTableDefined", QT_TR_NOOP("The copy relationship between tables `%1' and `%2' cannot be done because the first one already copies attributes from `%3'! Tables can have only one copy table!")},
	{"InvPartRelPartitionedDefined", QT_TR_NOOP("The partitioning relationship between tables `%1' and `%2' cannot be done because the first one is already a partition of table `%3'! Partition tables can participate in only one partition hierarchy at a time!")},
	{"InvRelTypeForPatitionTables", QT_TR_NOOP("The relationship between tables `%1' and `%2' cannot be created because one of the entities is part of a partitioning hierarchy! Table `%3' cannot be used in `generalization', `copy' and `one-to-one' relationships. In `one-to-many' and `many-to-many' relationships the mentioned table cannot be referenced by the generated foreign key(s).")},
	{"InvTableTriggerInsteadOfFiring", QT_TR_NOOP("The INSTEAD OF mode cannot be used on triggers that belong to tables! This is available only for view triggers!")},
	{"InvUsageTruncateOnTrigger", QT_TR_NOOP("The TRUNCATE event can only be used when the trigger executes for each statement and belongs to a table!")},
	{"InvUsageInsteadOfOnTrigger", QT_TR_NOOP("The INSTEAD OF mode cannot be used on view triggers that execute for each statement!")},
	{"InvConstrTriggerNotAfterRow", QT_TR_NOOP("Constraint triggers can only be executed on AFTER events and for each row!")},
	{"InvUsageAfterBeforeViewTrigger", QT_TR_NOOP("A view trigger cannot be AFTER/BEFORE when it executes for each row!")},
	{"InvUsageInsteadOfUpdateTrigger", QT_TR_NOOP("A trigger cannot make reference to columns when using INSTEAD OF mode and UPDATE event!")},
	{"AsgColumnNoParent", QT_TR_NOOP("Assignment of a column that has no parent table to object `%1' (%2)!")},
	{"InvUseConstraintTriggerAttribs", QT_TR_NOOP("Only constraint triggers can be deferrable or reference another table!")},
	{"RefInvalidFunctionIdTypeConfig", QT_TR_NOOP("Reference to a function id that is incompatible with the user-defined type configuration!")},
	{"AsgInvalidOpClassObject", QT_TR_NOOP("The operator class assigned to object `%1' (%2) must use `btree' as indexing method!")},
	{"InvPostgreSQLVersion", QT_TR_NOOP("Unsupported PostgreSQL version (%1) detected! Valid versions must be between %2 and %3.")},
	{"ValidationFailure", QT_TR_NOOP("The validation process failed due to an error triggered by the validation helper. For more details about the error check the exception stack!")},
	{"InvAllocationFKRelationship", QT_TR_NOOP("The fk relationship `%1' cannot be created because the foreign-key that represents it was not created on table `%2'!")},
	{"AsgInvalidNamePattern", QT_TR_NOOP("Assignment of an invalid object name pattern to relationship `%1'!")},
	{"RefInvalidNamePatternId", QT_TR_NOOP("Reference to an invalid object name pattern id on relationship `%1'!")},
	{"InvUsageVariadicParamMode", QT_TR_NOOP("Invalid use of `variadic' parameter mode! This mode can be used only with an array or \"any\" data type!")},
	{"MixingIncompExportOptions", QT_TR_NOOP("Mixing incompatible DBMS export options: `ignore object duplications', `drop database', `drop objects' or `transactional' cannot be used with `simulate export'!")},
	{"MixingIncompDropOptions", QT_TR_NOOP("Mixing incompatible DROP options: `drop database' and `drop objects' cannot be used at the same time!")},
	{"InvIdSwapSameObject", QT_TR_NOOP("Invalid object id swapping operation! The objects involved are the same!")},
	{"InvIdSwapInvalidObjectType", QT_TR_NOOP("Invalid object swapping operation! Databases, tablespaces and roles can have their ids swapped only with other objects of the same kind!")},
	{"AsgWidgetAlreadyHasParent", QT_TR_NOOP("The widget already has a parent and cannot be assigned to a different object!")},
	{"ObjectNotImported",  QT_TR_NOOP("The object `%1' (%2), oid `%3', could not be imported due to one or more errors! Check the exception stack for more details. `HINT:' if the object somehow references objects in `pg_catalog' or `information_schema' consider enabling the import of system/extension objects.")},
	{"ModelFileNotLoaded", QT_TR_NOOP("Could not load the database model file `%1'. Check the error stack to see details. Try to run `pgmodeler-cli --fix-model' in order to correct the file structure if that is the case.")},
	{"InvColumnTableType", QT_TR_NOOP("The column `%1' cannot reference its parent table `%2' as data type!")},
	{"OprInvalidElementId", QT_TR_NOOP("Operation with an invalid element id `%1'!")},
	{"RefInvalidElementColorId", QT_TR_NOOP("Reference to an invalid color id `%1' for element `%2'!")},
	{"AsgInvalidObjectType", QT_TR_NOOP("Assignment of an invalid object to `%1' (%2)! The assigned object must be of the type(s): `%3'.")},
	{"IncompColumnTypeForSequence", QT_TR_NOOP("The sequence `%1' cannot be assigned to column `%2' because the data type of the latter is incompatible. The type used must be an integer one!")},
	{"InvUsageTempNamesExportOption", QT_TR_NOOP("The option to generate temporary object names can only be used in simulation mode!")},
	{"InvConversionIntegerToSerial", QT_TR_NOOP("Cannot convert the type of column `%1' to serial! It must have an `integer' based type and its default value must be a call to `nextval(seq_name::regclass)' function or a sequence object must be directly assigned to the column!")},
	{"AsgInvalidEventTriggerVariable", QT_TR_NOOP("Could not assign the variable `%1' to event trigger's filter. Currently, PostgreSQL supports only the `TAG' variable!")},
	{"RowDataNotManipulated", QT_TR_NOOP("Could not perform the `%1' operation on `%2' using the data on row `%3'! All changes were rolled back. \n\n ** Returned error ** \n\n%4")},
	{"MalformedUnescapedValue", QT_TR_NOOP("Malformed unescaped value on row `%1' column `%2'!")},
	{"UndoRedoOperationInvalidObject", QT_TR_NOOP("Trying to undo/redo an invalid operation over an object that does not exist anymore or cannot be handled! The operation history will be cleaned up.")},
	{"RequiredFieldsNotFilled", QT_TR_NOOP("The object `%1' (%2) cannot be handled because some needed fields are not set! Please, make sure to fill at least the required fields in order to properly create or update the object.")},
	{"InvRelationshipIdSwap", QT_TR_NOOP("A relationship can only be swapped by another object of the same kind!")},
	{"InvInheritParentTableNotFound", QT_TR_NOOP("A parent table of `%1' with OID `%2' was not found in the set of imported objects!")},
	{"AsgEnumInvalidChars", QT_TR_NOOP("The enumeration `%1' cannot be assigned to type `%2' because it contains invalid characters!")},
	{"AsgEnumLongName", QT_TR_NOOP("The enumeration `%1' cannot be assigned to type `%2' because it is too long!")},
	{"ConnectionTimeout", QT_TR_NOOP("The connection was idle for too long and was automatically closed!")},
	{"ConnectionBroken", QT_TR_NOOP("The connection was unexpectedly closed by the database server `%1' at port `%2'!")},
	{"DropCurrentDBDefault", QT_TR_NOOP("Failed to drop the database `%1' because it is defined as the default database for connection `%2'!")},
	{"NullPrimaryKeyColumn", QT_TR_NOOP("The column `%1' must be `NOT NULL' because it composes the primary key of table `%2'. You need to remove the column from the mentioned constraint in order to disable the `NOT NULL' on it!")},
	{"InvalidIdentityColumn", QT_TR_NOOP("The identity column `%1' has an invalid data type! The data type must be `smallint', `integer' or `bigint'.")},
	{"RefInvalidAffectedCommand", QT_TR_NOOP("Reference to an invalid affected command in policy `%1'!")},
	{"RefInvalidSpecialRole", QT_TR_NOOP("Reference to an invalid special role in policy `%1'!")},
	{"InvColumnCountPartRel", QT_TR_NOOP("Unable to create a partition relationship because the partition table `%1' is not empty or has columns that are not present on the partitioned table `%2'!")},
	{"InvPartitioningTypePartRel", QT_TR_NOOP("Unable to create a partition relationship between tables `%1' (partition) and `%2' (partitioned) because no partitioning type is defined on the latter!")},
	{"InvPartitionKeyCount", QT_TR_NOOP("Invalid amount of partition keys being assigned to table `%1'! Multiple partition keys are allowed only on `HASH' and `RANGE' partitioning strategies.")},
	{"PartKeyObjectInexistsModel", QT_TR_NOOP("A partition key of table `%1' is referencing object `%3' (%4) which was not found in the model!")},
	{"AsgInvalidColumnPartitionKey", QT_TR_NOOP("The column `%1' cannot be assigned to a partition key because it was created by a relationship and this kind of operation is not yet supported! HINT: create the column manually on the table and then create the partition key using it.")},
	{"RemColumnRefByPartitionKey", QT_TR_NOOP("The column `%1' on table `%2' cannot be removed because it is being referenced by one or more partition keys!")},
	{"AsgOptionInvalidName", QT_TR_NOOP("Assignment of an option to the object with an invalid name!")},
	{"AsgInvalidNameObjReference", QT_TR_NOOP("Assignment of an invalid name or alias to the object reference!")},
	{"AsgNotAllocatedObjectReference", QT_TR_NOOP("Assignment of a non-allocated object to the object reference!")},
	{"InsDuplicatedObjectReference", QT_TR_NOOP("The object reference name `%1' is already defined!")},
	{"ModelFileInvalidSize", QT_TR_NOOP("An empty file was detected after saving the database model to `%1'. In order to avoid data loss, the original contents of the file prior to the last saving was saved to `%2'!")},
	{"AsgInvalidObjectForeignTable", QT_TR_NOOP("The object `%1' (%2) cannot be assigned to foreign table `%3' because it is unsupported! Foreign tables only accept columns, check constraints and triggers.")},
	{"InvRelTypeForeignTable", QT_TR_NOOP("The relationship `%1' between tables `%2' and `%3' cannot be created because one of the entities is a foreign table. Foreign tables can only be part of an inheritance, copy or partitioning relationship!")},
	{"InvCopyRelForeignTable", QT_TR_NOOP("The copy relationship `%1' between tables `%2' and `%3' cannot be created because a foreign table is not allowed to copy table columns!")},
	{"InvOutputDirectory", QT_TR_NOOP("Failed to save the data dictionary into `%1'! Make sure that the provided path points to a directory or if the user has write permissions over it!")},
	{"InitialUserConfigNotCreated", QT_TR_NOOP("Failed to create initial configuration in `%1'! Check if the current user has write permission over that path and at least read permission over `%2'.")},
	{"InvalidObjectFilter", QT_TR_NOOP("An invalid object filter was detected: `%1'! Filters should have the following format: `[object type]:[pattern]:[%2]'.")},
	{"InvChildObjectTypeFilter", QT_TR_NOOP("The object type `%1' is not a valid table child object type!")},
	{"InvChangelogEntryValues", QT_TR_NOOP("Trying to register a changelog entry with invalid values: `%1', `%2', `%3', `%4'!")},
	{"ExportFailureDbSQLDisabled", QT_TR_NOOP("The SQL code of database `%1' is disabled! The export process cannot proceed. Please, enable the SQL code of the mentioned object and try again.")},
	{"InvConfigParameterName", QT_TR_NOOP("Invalid configuration parameter `%1' assigned to function `%2'!")},
	{"EmptyConfigParameterValue", QT_TR_NOOP("Empty value assigned to the configuration parameter `%1' in function `%2'!")},
	{"InvGroupRegExpPattern", QT_TR_NOOP("Invalid regexp pattern detected in syntax highlighting group `%1' at file `%2'! Error detected: `%3'")},
	{"UnsupportedPGVersion", QT_TR_NOOP("Unsupported PostgreSQL version `%1' detected! pgModeler supports only PostgreSQL 10 and above.")},
	{"InvCodeGenerationMode", QT_TR_NOOP("Invalid code generation mode `%1' specified!")},
	{"InvCsvParserOptions", QT_TR_NOOP("The characters for text delimiter, value separator, and line break in CSV document must be different from each other!")},
	{"MalformedCsvInvalidCols", QT_TR_NOOP("Malformed CSV document detected! The number of columns is `%1' but row `%2' has `%3' columns!")},
	{"MalformedCsvMissingDelim", QT_TR_NOOP("Malformed CSV document detected! Missing close text delimiter `%1' row `%2'!")},
	{"RefInvCsvDocumentValue", QT_TR_NOOP("Trying to get a value from the CSV document in an invalid position: row `%1', column `%2'!")},
	{"ModelFileSaveFailure", QT_TR_NOOP("Failed to save the database model to file `%1'! In order to avoid data loss, the backup file `%2' was restored. Note that the backup file will not be erased automatically, the user must delete it manually or, if preferred, copy it to a safe place to have an extra security copy!")},
	{"RemExtRefChildObject", QT_TR_NOOP("The extension `%1' cannot be removed because its child object `%2' (%3) is being referenced by `%4' (%5)!")},
	{"AddExtDupChildObject", QT_TR_NOOP("The extension `%1' cannot be added to the database model because its child object `%2' (%3) has conflicting name and type with another object in the model!")},
	{"AsgSchExtTypeConflict", QT_TR_NOOP("The schema `%1' cannot be assigned to extension `%2' because the child object `%3' (%4) will have a conflicting name with another object in the model!")},
	{"MalformedViewDefObject", QT_TR_NOOP("Malformed definition object assigned to view `%1'!")},
	{"InvalidEscapedCharacter", QT_TR_NOOP("Invalid escaped character `%1' in file `%2', line %3, column %4!")},
	{"InvExprMultilineGroup", QT_TR_NOOP("The group `%1' contains an expression declared as both initial and final! An expression must be either initial or final, not both.")},
	{"InvExprPersistentGroup", QT_TR_NOOP("The group `%1' has been declared as persistent but contains initial and/or final expression(s)! Persistent groups must not declare initial or final expressions.")},
	{"InvExtensionObject", QT_TR_NOOP("Invalid child object being assigned to extension `%1'!")},
	{"AsgInvSchemaExtension", QT_TR_NOOP("Assigning the schema `%1' to extension `%2' is not allowed because the schema is a child of the extension!")},
};

Exception::Exception()
{
	configureException("",ErrorCode::Custom,"","",-1,"");
}

Exception::Exception(Exception &e, const QString &method, const QString &file, int line)
{
	configureException(e.getErrorMessage(), e.getErrorCode(), method, file, line, e.getExtraInfo());
	addException(&e);
}

Exception::Exception(const QString &msg, const QString &method, const QString &file, int line, Exception *exception, const QString &extra_info)
{
	configureException(msg,ErrorCode::Custom, method, file, line, extra_info);
	addException(exception);
}

Exception::Exception(ErrorCode error_code, const QString &method, const QString &file, int line, Exception *exception, const QString &extra_info)
{
	/* Because the Exception class is not derived from QObject the function tr() is inefficient to translate messages
		so the translation method is called  directly from the application specifying the
		context (Exception) in the ts file and the text to be translated */
	configureException(qApp->translate("Exception",messages[enum_t(error_code)][ErrorMessage].toStdString().c_str(),"", -1),
										 error_code, method, file, line, extra_info);

	addException(exception);
}

Exception::Exception(const QString &msg, ErrorCode error_code, const QString &method, const QString &file, int line, Exception *exception, const QString &extra_info)
{
	configureException(msg,error_code, method, file, line, extra_info);
	addException(exception);
}

Exception::Exception(ErrorCode error_code, const QString &method, const QString &file, int line, std::vector<Exception> &exceptions, const QString &extra_info)
{
	std::vector<Exception>::iterator itr, itr_end;

	/* Because the Exception class is not derived from QObject the function tr() is inefficient to translate messages
		so the translation method is called  directly from the application specifying the
		context (Exception) in the ts file and the text to be translated */
	configureException(qApp->translate("Exception",messages[enum_t(error_code)][ErrorMessage].toStdString().c_str(),"",-1),
										 error_code, method, file, line, extra_info);

	itr=exceptions.begin();
	itr_end=exceptions.end();
	while(itr!=itr_end)
	{
		addException(&(*itr));
		itr++;
	}
}

Exception::Exception(const QString &msg, const QString &method, const QString &file, int line, std::vector<Exception> &exceptions, const QString &extra_info)
{
	std::vector<Exception>::iterator itr, itr_end;

	configureException(msg,ErrorCode::Custom, method, file, line, extra_info);

	itr=exceptions.begin();
	itr_end=exceptions.end();
	while(itr!=itr_end)
	{
		addException(&(*itr));
		itr++;
	}
}

Exception::Exception(const QString &msg, ErrorCode error_code, const QString &method, const QString &file, int line, std::vector<Exception> &exceptions, const QString &extra_info)
{
	std::vector<Exception>::iterator itr=exceptions.begin();

	configureException(msg,error_code, method, file, line, extra_info);

	while(itr!=exceptions.end())
	{
		addException(&(*itr));
		itr++;
	}
}

void Exception::configureException(const QString &msg, ErrorCode error_code, const QString &method, const QString &file, int line, const QString &extra_info)
{
	this->error_code = error_code;
	this->error_msg = msg;
	this->method = method;
	this->file = file;
	this->line = line;
	this->extra_info = extra_info;
}

QString Exception::getErrorMessage()
{
	return error_msg;
}

QString Exception::getErrorMessage(ErrorCode error_code)
{
	/* Because the Exception class is not derived from QObject the function tr() is inefficient to translate messages
	 so the translation method is called  directly from the application specifying the
	 context (Exception) in the ts file and the text to be translated */
	return qApp->translate("Exception", messages[enum_t(error_code)][ErrorMessage].toStdString().c_str(), "", -1);
}

QString Exception::getErrorCode(ErrorCode error_code)
{
	return messages[enum_t(error_code)][ErrorCodeId];
}

QString Exception::getMethod()
{
	return method;
}

QString Exception::getFile()
{
	return file;
}

QString Exception::getLine()
{
	return QString("%1").arg(line);
}

ErrorCode Exception::getErrorCode()
{
	return error_code;
}

QString Exception::getExtraInfo()
{
	return extra_info;
}

void Exception::addException(Exception *exception)
{
	if(!exception)
		return;

	for(auto &ex : exception->exceptions)
	{
		this->exceptions.push_back(Exception(ex.error_msg,ex.error_code,
																				 ex.method, ex.file, ex.line, nullptr,
																				 ex.extra_info));
	}

	this->exceptions.push_back(*exception);
	//exception->exceptions.clear();
}

void Exception::getExceptionsList(std::vector<Exception> &list)
{
	list.assign(this->exceptions.begin(), this->exceptions.end());
	list.push_back(Exception(this->error_msg,this->error_code,
													 this->method,this->file,this->line,nullptr,this->extra_info));
}

QString Exception::getExceptionsText()
{
	std::vector<Exception> exceptions;
	std::vector<Exception>::reverse_iterator itr, itr_end;
	unsigned idx = 0, hidden_errors_cnt = 0;
	QString exceptions_txt;
	bool stack_truncated = false;

	//Get the generated exceptions list
	this->getExceptionsList(exceptions);
	itr = exceptions.rbegin();
	itr_end = exceptions.rend();
	idx = 0;

	if(exceptions.size() > MaximumStackSize)
	{
		hidden_errors_cnt = exceptions.size() - Exception::MaximumStackSize;
		stack_truncated = true;
	}

	//Append all usefull information about the exceptions on the string
	while(itr != itr_end)
	{
		exceptions_txt+=QString("[%1] %2 (%3)\n").arg(idx).arg(itr->getFile(), itr->getLine());
		exceptions_txt+=QString("  %1\n").arg(itr->getMethod());
		exceptions_txt+=QString("    [%1] %2\n").arg(Exception::getErrorCode(itr->getErrorCode()), itr->getErrorMessage().replace('`', '\''));

		if(!itr->getExtraInfo().isEmpty())
			exceptions_txt+=QString("       ** %1\n\n").arg(itr->getExtraInfo());
		else
			exceptions_txt+="\n";

		itr++; idx++;

		if(stack_truncated && idx >= Exception::MaximumStackSize)
		{
			exceptions_txt += QString(QT_TR_NOOP("** Another %1 error(s) were suppressed due to stack trace size limits.\n\n")).arg(hidden_errors_cnt);
			break;
		}
	}

	return exceptions_txt;
}

QString Exception::getExceptiosExtraInfo()
{
	QStringList list;

	for(auto &ex : exceptions)
		list.prepend(ex.extra_info);

	list.prepend(extra_info);
	list.removeAll("");
	list.removeDuplicates();

	return list.join('\n');
}
