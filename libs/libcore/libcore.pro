include(../../pgmodeler.pri)

TEMPLATE = lib
TARGET = core

# Enables shared library symbols exporting
DEFINES += CORE_SYMBOLS

HEADERS += src/coreglobal.h \
	src/reference.h \
	   src/textbox.h \
	   src/basefunction.h \
	   src/cast.h \
	   src/defaultlanguages.h \
	   src/function.h \
	   src/language.h \
	   src/operator.h \
	   src/baserelationship.h \
	   src/basetable.h \
	   src/column.h \
	   src/domain.h \
	   src/aggregate.h \
	   src/permission.h \
	   src/databasemodel.h \
	   src/pgsqltypes/actiontype.h \
	   src/pgsqltypes/behaviortype.h \
	   src/pgsqltypes/categorytype.h \
	   src/pgsqltypes/constrainttype.h \
	   src/pgsqltypes/deferraltype.h \
	   src/pgsqltypes/encodingtype.h \
	   src/pgsqltypes/eventtriggertype.h \
	   src/pgsqltypes/eventtype.h \
	   src/pgsqltypes/executiontype.h \
	   src/pgsqltypes/firingtype.h \
	   src/pgsqltypes/functiontype.h \
	   src/pgsqltypes/identitytype.h \
	   src/pgsqltypes/indexingtype.h \
	   src/pgsqltypes/intervaltype.h \
	   src/pgsqltypes/matchtype.h \
	   src/pgsqltypes/paralleltype.h \
	   src/pgsqltypes/partitioningtype.h \
	   src/pgsqltypes/pgsqltype.h \
	   src/pgsqltypes/policycmdtype.h \
	   src/pgsqltypes/providertype.h \
	   src/pgsqltypes/securitytype.h \
	   src/pgsqltypes/spatialtype.h \
	   src/pgsqltypes/storagetype.h \
	   src/pgsqltypes/templatetype.h \
	   src/pgsqltypes/usertypeconfig.h \
		 src/pgsqltypes/checkoptiontype.h \
	   src/procedure.h \
	   src/role.h \
	   src/constraint.h \
	src/simplecolumn.h \
	   src/transform.h \
	   src/type.h \
	   src/tablespace.h \
	   src/trigger.h \
	   src/baseobject.h \
	   src/rule.h \
	   src/sequence.h \
	   src/conversion.h \
	   src/schema.h \
	   src/indexelement.h \
	   src/index.h \
	   src/basegraphicobject.h \
	   src/relationship.h \
	   src/table.h \
	   src/usermapping.h \
	   src/view.h \
	   src/operatorfamily.h \
	   src/operatorclasselement.h \
	   src/operatorclass.h \
	   src/operationlist.h \
	   src/tableobject.h \
	   src/collation.h \
	   src/excludeelement.h \
	   src/element.h \
	   src/parameter.h \
	   src/operation.h \
	   src/copyoptions.h \
	   src/typeattribute.h \
	   src/extension.h \
	   src/tag.h \
	   src/eventtrigger.h \
	   src/genericsql.h \
	   src/policy.h \
	   src/partitionkey.h \
	   src/foreigndatawrapper.h \
	   src/foreignobject.h \
	   src/foreignserver.h \
	   src/physicaltable.h \
	   src/foreigntable.h \
    src/coreutilsns.h

SOURCES +=  src/textbox.cpp \
	    src/basefunction.cpp \
	    src/domain.cpp \
	    src/pgsqltypes/actiontype.cpp \
	    src/pgsqltypes/behaviortype.cpp \
	    src/pgsqltypes/categorytype.cpp \
	    src/pgsqltypes/constrainttype.cpp \
	    src/pgsqltypes/deferraltype.cpp \
	    src/pgsqltypes/encodingtype.cpp \
	    src/pgsqltypes/eventtriggertype.cpp \
	    src/pgsqltypes/eventtype.cpp \
	    src/pgsqltypes/executiontype.cpp \
	    src/pgsqltypes/firingtype.cpp \
	    src/pgsqltypes/functiontype.cpp \
	    src/pgsqltypes/identitytype.cpp \
	    src/pgsqltypes/indexingtype.cpp \
	    src/pgsqltypes/intervaltype.cpp \
	    src/pgsqltypes/matchtype.cpp \
	    src/pgsqltypes/paralleltype.cpp \
	    src/pgsqltypes/partitioningtype.cpp \
	    src/pgsqltypes/pgsqltype.cpp \
	    src/pgsqltypes/policycmdtype.cpp \
	    src/pgsqltypes/providertype.cpp \
	    src/pgsqltypes/securitytype.cpp \
	    src/pgsqltypes/spatialtype.cpp \
	    src/pgsqltypes/storagetype.cpp \
			src/pgsqltypes/checkoptiontype.cpp \
	    src/procedure.cpp \
	src/reference.cpp \
	src/simplecolumn.cpp \
	    src/transform.cpp \
	    src/trigger.cpp \
	    src/basegraphicobject.cpp \
	    src/baserelationship.cpp \
	    src/type.cpp \
	    src/column.cpp \
	    src/tablespace.cpp \
	    src/indexelement.cpp \
	    src/index.cpp \
	    src/operator.cpp \
	    src/constraint.cpp \
	    src/schema.cpp \
	    src/language.cpp \
	    src/role.cpp \
	    src/sequence.cpp \
	    src/usermapping.cpp \
	    src/view.cpp \
	    src/conversion.cpp \
	    src/function.cpp \
	    src/permission.cpp \
	    src/databasemodel.cpp \
	    src/rule.cpp \
	    src/table.cpp \
	    src/cast.cpp \
	    src/aggregate.cpp \
	    src/baseobject.cpp \
	    src/relationship.cpp \
	    src/basetable.cpp \
	    src/operatorfamily.cpp \
	    src/operatorclasselement.cpp \
	    src/operatorclass.cpp \
	    src/operationlist.cpp \
	    src/tableobject.cpp \
	    src/collation.cpp \
	    src/excludeelement.cpp \
	    src/element.cpp \
	    src/parameter.cpp \
	    src/copyoptions.cpp \
	    src/typeattribute.cpp \
	    src/extension.cpp \
	    src/tag.cpp \
	    src/eventtrigger.cpp \
	    src/operation.cpp \
	    src/genericsql.cpp \
	    src/policy.cpp \
	    src/partitionkey.cpp \
	    src/foreigndatawrapper.cpp \
	    src/foreignobject.cpp \
	    src/foreignserver.cpp \
	    src/physicaltable.cpp \
	    src/foreigntable.cpp \
    src/coreutilsns.cpp

unix|windows: LIBS += $$LIBPARSERS_LIB \
		      $$LIBUTILS_LIB

INCLUDEPATH += $$LIBPARSERS_INC \
	       $$LIBUTILS_INC

DEPENDPATH += $$LIBPARSERS_ROOT \
	      $$LIBUTILS_ROOT

# Deployment settings
target.path = $$PRIVATELIBDIR
INSTALLS = target
