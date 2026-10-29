# pgModeler Copilot Instructions

This file contains guidelines for generating code consistent with pgModeler's codebase style and patterns.

## Architecture and Structure

### Directory Organization
- **libs/**: Contains main libraries organized by functionality
  - `libcore/`: Business logic and data model
  - `libgui/`: Graphical interface and widgets
  - `libutils/`: Shared utilities
  - `libcanvas/`: Graphical canvas and rendering
  - `libconnector/`: Database connectivity
  - `libparsers/`: SQL/XML parsers
  - `libcli/`: Command line interface
- **apps/**: Executable applications
- **plugins/**: Plugins and extensions
- **assets/**: Resources (configurations, schemas, samples)

### File Patterns
- Headers always with guards: `#ifndef HEADER_NAME_H` / `#define HEADER_NAME_H` / `#endif`
- Each class in its own file pair (.h/.cpp)
- Organized includes: system headers first, then Qt, then local
- Standard copyright header in all files

## Formatting Style

### Indentation and Spacing
- **USE TABS**, not spaces for indentation
- No spaces in templates: `std::vector<BaseObject *>` (not `std::vector<BaseObject *>`)
- Spaces around operators: `a = b + c;`
- No space between function name and parentheses: `functionName(args)`
- No space between control/loop instructions and parentheses: `if(condition)` `while(condition)` `for(...)`
- No space between cast and parentheses: `static_cast<Type>(value)`
- No space between unary operators and variable: `!flag`, `++index`
- No space between variable name and asterisk when dereferencing pointers: `*ptr`
- No space between variable name and ampersand when referencing: `&ref`
- Space after function return type or function arguments and asterisk/ampersand, asterisk joined at variable/func name: `Type *func`, `Type &func`, `func(Type *arg, Type &arg2)`
- No space between function name and angle brackets for templates: `func<Type>(arg)`
- No space between class name and angle brackets for templates: `Class<Type> obj;`
- No space between angle brackets in nested templates: `std::map<Key, std::vector<Value>>`
- No space between namespace and scope resolution operator: `Namespace::Class`
- No space between class name and scope resolution operator: `Class::method()`
- No space between function name and scope resolution operator: `func::method()`
- Space after commas: `func(arg1, arg2, arg3)`

### Braces and Blocks
```cpp
// Classes and functions - opening on same line
class MyClass {
private:
    int member;

public:
    void method() {
        // body
    }
};

// Control structures - opening on new line
if(condition)
{
    // code
}
else
{
    // code
}

for(auto &item : container)
{
    // code
}
```

### Naming
- **Variables and methods**: `snake_case`
  ```cpp
  bool is_protected;
  void setCodeInvalidated(bool value);
  std::vector<BaseObject *> object_refs;
  ```

- **Classes**: `PascalCase`
  ```cpp
  class BaseObject;
  class DatabaseModel;
  ```

- **Enums**: `PascalCase` with `enum class`
  ```cpp
  enum class ObjectType: unsigned {
      Column,
      Constraint,
      Function
  };
  ```

- **Constants**: `CamelCase` for `constexpr` and `UPPER_CASE` for constants defined with `#define`
  ```cpp
  static const QString EntityAmp("&amp;");
  static constexpr double DefaultRadius = 9.0;
  ```

- **Namespaces**: `PascalCase`
  ```cpp
  namespace UtilsNs {
      // content
  }
  ```

## C++17 Patterns

### Strongly Typed Enums
```cpp
// ALWAYS use enum class
enum class ConstraintType: unsigned {
    PrimaryKey,
    ForeignKey,
    Check,
    Unique,
    Exclude
};

// Use enum_t() for conversion when needed
unsigned type_id = enum_t(ObjectType::Table);
```

### Templates and Type Traits
```cpp
// Use std::enable_if_t and std::is_base_of_v for constraints
template<class Widget, std::enable_if_t<std::is_base_of_v<BaseConfigWidget, Widget>, bool> = true>
void configureWidget(Widget *widget);

// Use if constexpr for conditional branching
template<class Class>
void processWidget(Class *widget) {
    if constexpr (std::is_base_of_v<QPlainTextEdit, Class>) {
        // specific logic
    }
}
```

### Auto and Type Deduction
```cpp
// Use auto for iterators and obvious types
auto itr = container.begin();
auto end = container.end();

// Range-based for loops with auto&
for(auto &item : container) {
    // process item
}
```

### Using Aliases
```cpp
// Prefer using over typedef
using attribs_map = std::map<QString, QString>;
```

### Constexpr
```cpp
// Use constexpr for compile-time constants
static constexpr unsigned ObjectTypeCount = enum_t(ObjectType::BaseTable) + 1;
static constexpr double MinScaleFactor = 0.100000;
```

## Qt-Specific Patterns

### Signals and Slots
```cpp
// Use new connect syntax
connect(sender, &SenderClass::s_signal, receiver, &ReceiverClass::slot);

// For slots that can throw exceptions, use special macros
connect(sender, &SenderClass::s_signal, context, __slot(receiver, ReceiverClass::method));
```

### Meta Types
```cpp
// Register enums as Qt MetaType when needed
Q_DECLARE_METATYPE(ObjectType)
```

### Qt Inheritance
```cpp
class MyWidget: public QDialog, public Ui::MyWidget {
    Q_OBJECT
    // implementation
};
```

## Error Management and Debug

### Exception Handling
```cpp
// Use standard macros for debug info
throw Exception(ErrorCode::SomeError, PGM_FUNC, PGM_FILE, PGM_LINE);

// Re-throw preserving stack trace
catch(Exception &e) {
    throw Exception(e.getErrorMessage(), e.getErrorCode(), PGM_FUNC, PGM_FILE, PGM_LINE, &e);
}
```

### Parameter Validation
```cpp
void MyClass::setObject(BaseObject *obj) {
    if(!obj)
        throw Exception(ErrorCode::OprNotAllocatedObject, PGM_FUNC, PGM_FILE, PGM_LINE);
    
    // method logic
}
```

## Cache and Invalidation Patterns

### Code Invalidation Pattern
```cpp
void MyClass::setSomeProperty(bool value) {
    setCodeInvalidated(this->property != value);  // Invalidate cache if value changed
    this->property = value;
}
```

### Cached Attributes
```cpp
// Use arrays for multiple cache versions
QString cached_code[2];  // SQL and XML
QString cached_names[3]; // RawName, FmtName, Signature

enum CachedNameId: unsigned {
    RawName,
    FmtName,
    Signature
};
```

## Data Structures

### STL vs Qt Containers
```cpp
// Prefer STL containers for internal data
std::vector<BaseObject *> objects;
std::map<QString, QString> attributes;

// Qt containers when Qt integration is needed
QStringList items;
```

### Initialization
```cpp
// Use uniform member initialization
MyClass::MyClass() : 
    is_protected(false),
    system_obj(false),
    code_invalidated(true) {
    // constructor code
}
```

## Documentation and Comments

### Doxygen Headers
```cpp
/**
\ingroup libcore
\brief Implements operations to manipulate database tables.
\note <strong>Creation date:</strong> 17/09/2006
*/
```

### Inline Comments
```cpp
//! \brief Indicates if the object is protected
bool is_protected;

/*! \brief This method performs specific operation
 * Detailed description of what the method does */
void specificOperation();
```

### Multiline Comments
```cpp
/* This is a multiline comment
 * explaining complex logic or decisions
 * in the code. */ 
```

## Build System (CMake)

### Library Configuration
```cmake
pgm_add_library(mylib
    src/file1.cpp src/file1.h
    src/file2.cpp src/file2.h
)

set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTOMOC ON)
```

### C++17 Standard
```cmake
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## Anti-Patterns to Avoid

### ❌ DON'T Do
```cpp
// Don't use assignement in conditionals
if((value = getValue()) > 0) // BAD
{

}

value = getValue();
if(value > 0) // GOOD
{
    // use value
}

// Don't use single line ifs 
if(condition) statement; // BAD

if(condition)
    statement; // GOOD

// Don't use a single line between brackets in ifs and other structures
if(condition)
{
  statement; // BAD
}

if(condition)
  statement; // GOOD

// Don't use #include <header.h> for Qt classes, use <QtClass> instead
#include <qmap.h>  // BAD
#include <QMap> // GOOD

// Don't use typedef
typedef std::map<QString, QString> AttribsMap;

// Don't repeat unnecessary declarations
void func();
void func(); // DUPLICATE

// Don't use spaces for indentation
class MyClass {
    int member; // Use TABS here
}

// Don't use duplicated data type declarations
QColor bg_color; // BAD
QColor border_color; // BAD

QColor bg_color, border_color; // GOOD

// Avoid declarations of scope-only variables. Check if variable can be declared outside the scope or if it is already declared, before creating.
if(condition)
{
    int temp_var; // BAD
    // use temp_var
}

int temp_var; // GOOD

if(condition)
{
 // use temp_var
}

// Don't use C-Style casts use static_cast/dynamic_cast/reinterpret_cast instead
int value = (int)some_double; // BAD
int value = static_cast<int>(some_double); // GOOD
```

### ✅ DO This
```cpp
// Use using
using AttribsMap = std::map<QString, QString>;

// Use enum class
enum class ObjectType: unsigned { Table, Column };

// Templates without extra spaces
std::vector<BaseObject *> objects;

// One declaration per method
void func();

// Use tabs for indentation
class MyClass {
	int member; // TAB here
}
```

## Specific Guidelines for Generation

0. **When possible use modern C++17 code**
1. **Always check if a method/attribute already exists before creating**
2. **Maintain consistency with existing patterns in the class**
3. **Use the same prefixes/suffixes from related classes**
4. **Implement cache invalidation methods when appropriate**
5. **Follow logical order: private -> protected -> public**
6. **Include appropriate error handling with PGM_* macros**
7. **Use const-correctness where appropriate**
8. **Implement comparison operators when needed**

## Complete Class Example

```cpp
#ifndef MY_CLASS_H
#define MY_CLASS_H

#include "baseobject.h"
#include <vector>

class __libcore MyClass: public BaseObject {
	private:
		bool some_flag;
		std::vector<BaseObject *> objects;
		
	protected:
		void validateObject(BaseObject *obj);
		
	public:
		MyClass();
		virtual ~MyClass();
		
		void setSomeFlag(bool flag);
		bool isSomeFlagSet();
		
		void addObject(BaseObject *obj);
		void removeObject(unsigned idx);
		BaseObject *getObject(unsigned idx);
		unsigned getObjectCount();
		
		virtual QString getSourceCode(SchemaParser::CodeType def_type) final;
};

#endif
```

Following these guidelines, the generated code will be consistent with the established style and patterns in pgModeler's codebase.