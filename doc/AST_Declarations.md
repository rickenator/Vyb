# Vyn AST: Declarations

This document details the various declaration AST nodes used in Vyn, as defined in `include/vyn/parser/ast.hpp`. All declaration nodes inherit from `vyn::ast::Declaration` (which itself inherits from `vyn::ast::Statement`).

## Common Pointer Aliases

Throughout the AST definitions, the following smart pointer aliases are used for brevity and clarity, representing `std::unique_ptr<T>`:

- `IdentifierPtr = std::unique_ptr<Identifier>;`
- `TypeNodePtr = std::unique_ptr<TypeNode>;`
- `ExprPtr = std::unique_ptr<Expression>;`
- `StmtPtr = std::unique_ptr<Statement>;`
- `DeclPtr = std::unique_ptr<Declaration>;`
- `BlockStatementPtr = std::unique_ptr<BlockStatement>;`
- `FieldDeclarationPtr = std::unique_ptr<FieldDeclaration>;`
- `GenericParameterPtr = std::unique_ptr<GenericParameter>;`
- `FunctionDeclarationPtr = std::unique_ptr<FunctionDeclaration>;`


## 1. Variable Declaration (`vyn::ast::VariableDeclaration`)

Represents the declaration of a variable.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class VariableDeclaration : public Declaration {
public:
    IdentifierPtr name;
    std::optional<TypeNodePtr> type; // Type annotation (mandatory with generic-angled var<T>/const<T> syntax)
    std::optional<ExprPtr> initializer;
    bool isMutable;
    bool isConst; // For compile-time constants

    VariableDeclaration(SourceLocation loc, IdentifierPtr name,
                        std::optional<TypeNodePtr> type,
                        std::optional<ExprPtr> initializer,
                        bool isMutable, bool isConst);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

**Note:** As of v0.3.7, Vyn uses the `var<T>`/`const<T>` declaration syntax with full support for modern struct syntax (`field<Type>`), match statements, break/continue, and Vec<T> collections, so every `VariableDeclaration` node will have `type` set (no longer optional in practice).

## 2. Function Parameter (`vyn::ast::FunctionParameter`)

Represents a single parameter in a function declaration. This is a helper structure and inherits directly from `Node`.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class FunctionParameter : public Node { // Inherits from Node
public:
    IdentifierPtr name;
    TypeNodePtr type;
    bool isMutable; // Whether the parameter is mutable within the function body

    FunctionParameter(SourceLocation loc, IdentifierPtr name, TypeNodePtr type, bool isMutable);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

### Function Parameter Syntax

Vyn supports two parameter syntax styles that produce identical AST structures:

#### Standard Syntax
```vyn
// Explicit mutability with angle brackets
fn<String> format(var<String> prefix, const<Int> value) -> {
    return prefix + value.to_string()
}
```

#### Shorthand Syntax 
```vyn
// Type-first shorthand (more concise)
fn<String> format(String prefix, const Int value) -> {
    return prefix + value.to_string()
}
```

#### Mixed Syntax
```vyn
// Both forms can be used in the same function
fn<Int> calculate(var<Int> base, Int multiplier, const<Int> offset) -> {
    return base * multiplier + offset
}
```

Both syntaxes produce identical `FunctionParameter` AST nodes and LLVM IR. The shorthand syntax assumes `var` (mutable) unless `const` is explicitly specified.

## 3. Function Declaration (`vyn::ast::FunctionDeclaration`)

Represents the declaration of a function.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class FunctionDeclaration : public Declaration {
public:
    std::unique_ptr<Identifier> id;
    std::vector<FunctionParameter> params;
    std::unique_ptr<BlockStatement> body;
    bool isAsync;
    TypeNodePtr returnTypeNode; // Optional return type annotation

    FunctionDeclaration(SourceLocation loc, std::unique_ptr<Identifier> id,
                        std::vector<FunctionParameter> params,
                        std::unique_ptr<BlockStatement> body,
                        bool isAsync = false,
                        TypeNodePtr returnTypeNode = nullptr);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

### Multi-Value Returns & Auto-Serialization

Function declarations in Vyn support multi-value return types using the generic syntax `fn<T1, T2, ...>`. When the function name is `main`, the Vyn runtime automatically serializes returned values to JSON format:

```vyn
// Single return type with standard parameter syntax
fn<Int> add(var<Int> a, var<Int> b) -> a + b

// Same function with shorthand parameter syntax
fn<Int> add_shorthand(Int a, Int b) -> a + b

// Multi-value return type with mixed parameter syntax
fn<Int, String> get_values(String prefix, const Int count) -> {
    return count, prefix + "Hello, World!"
}

// Auto-serialization in main()
fn<Int, String> main() -> {
    return get_values()  // Output: {"Int":42,"String":"Hello, World!"}
}
```

For comprehensive documentation on multi-value returns and auto-serialization behavior, see [`Auto_Serialization_Main_Returns.md`](./Auto_Serialization_Main_Returns.md).

## 4. Type Alias Declaration (`vyn::ast::TypeAliasDeclaration`)

Represents a type alias (e.g., `type MyInt = i32;`).

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class TypeAliasDeclaration : public Declaration {
public:
    IdentifierPtr name;
    TypeNodePtr aliasedType;
    std::vector<GenericParameterPtr> genericParameters;

    TypeAliasDeclaration(SourceLocation loc, IdentifierPtr name,
                         TypeNodePtr aliasedType,
                         std::vector<GenericParameterPtr> genericParams);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

## 5. Import Specifier (`vyn::ast::ImportSpecifier`)

Represents a single item being imported in an import declaration (e.g., `foo` or `foo as bar` in `import module::{foo as bar};`). This is a helper structure and inherits directly from `Node`.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class ImportSpecifier : public Node { // Inherits from Node
public:
    IdentifierPtr name;
    std::optional<IdentifierPtr> alias; // e.g., `bar` in `foo as bar`

    ImportSpecifier(SourceLocation loc, IdentifierPtr name, std::optional<IdentifierPtr> alias);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

## 6. Import Declaration (`vyn::ast::ImportDeclaration`)

Represents an import statement (e.g., `import module::submodule;` or `import module::{Item1, Item2};`).

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class ImportDeclaration : public Declaration {
public:
    std::vector<IdentifierPtr> path; // e.g., ["module", "submodule"]
    std::vector<ImportSpecifier> specifiers; // Specific items to import, empty if importing module directly or with '*'
    bool importAll; // True if `import module::*;`

    ImportDeclaration(SourceLocation loc, std::vector<IdentifierPtr> path,
                      std::vector<ImportSpecifier> specifiers, bool importAll);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

## 7. Struct Declaration (`vyn::ast::StructDeclaration`)

Represents the declaration of a struct.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class StructDeclaration : public Declaration {
public:
    IdentifierPtr name;
    std::vector<FieldDeclarationPtr> fields;
    std::vector<GenericParameterPtr> genericParameters;

    StructDeclaration(SourceLocation loc, IdentifierPtr name,
                      std::vector<FieldDeclarationPtr> fields,
                      std::vector<GenericParameterPtr> genericParams);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

### Field Declaration Syntax

Vyn supports two syntaxes for struct field declarations that can be used interchangeably or mixed within the same struct:

#### Colon Syntax (Original)
```vyn
struct Person {
    id: Int,
    name: String
}
```

#### Angle Bracket Syntax (New)
```vyn
struct Person {
    id<Int>,
    name<String>
}
```

#### Mixed Syntax
```vyn
struct MixedPoint {
    x<Int>,   // Angle bracket syntax
    y: Int    // Colon syntax
}
```

The angle bracket syntax aligns with Vyn's type-first approach used in function signatures (`fn<ReturnType>`) and variable declarations (`var<Type>`), providing visual consistency across the language.

## 8. Class Declaration (`vyn::ast::ClassDeclaration`)

*Note: Vyn does not have a class system. The `ClassDeclaration` AST node exists as a
parser artifact from early development and is **not** exposed in the language. Use structs
+ aspects instead. This node may be removed in a future cleanup.*

```cpp
// Structure in include/vyn/parser/ast.hpp (legacy — not exposed to Vyn programs)
namespace vyn::ast {

class ClassDeclaration : public Declaration {
public:
    IdentifierPtr name;
    std::vector<FieldDeclarationPtr> fields;
    std::vector<FunctionDeclarationPtr> methods;
    std::vector<GenericParameterPtr> genericParameters;

    ClassDeclaration(SourceLocation loc, IdentifierPtr name,
                     std::vector<FieldDeclarationPtr> fields,
                     std::vector<FunctionDeclarationPtr> methods,
                     std::vector<GenericParameterPtr> genericParams);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

## 9. Field Declaration (`vyn::ast::FieldDeclaration`)

Represents a field within a struct. It's a declaration itself.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class FieldDeclaration : public Declaration {
public:
    IdentifierPtr name;
    TypeNodePtr type;
    std::optional<ExprPtr> defaultValue; // Optional default value for the field
    bool isMutable; // If the field can be mutated after struct initialization

    FieldDeclaration(SourceLocation loc, IdentifierPtr name, TypeNodePtr type,
                     std::optional<ExprPtr> defaultValue, bool isMutable);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

### Field Declaration Syntax

Fields can be declared using either colon syntax or angle bracket syntax:

```vyn
struct Example {
    // Colon syntax (original)
    field1: Int,
    field2: String,
    
    // Angle bracket syntax (new)
    field3<Float>,
    field4<Bool>,
    
    // Optional default values work with both syntaxes
    field5: Int = 42,
    field6<String> = "default"
}
```

Both syntaxes produce identical AST structures and runtime behavior. The choice between them is primarily stylistic, though the angle bracket syntax provides consistency with function return types and variable declarations.

## 10. Bind Declaration (`vyn::ast::ImplDeclaration`)

Represents a `bind` block — binding an aspect to a type. The underlying AST node is
`ImplDeclaration` (legacy name); the Vyn keyword is `bind`. See `doc/TRAIT_SYSTEM_DESIGN.md`.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class ImplDeclaration : public Declaration {
public:
    IdentifierPtr structName;            // The type being bound to
    std::optional<IdentifierPtr> traitName; // The aspect being bound (aspectName)
    std::vector<FunctionDeclarationPtr> methods;
    std::vector<GenericParameterPtr> genericParameters;

    ImplDeclaration(SourceLocation loc, IdentifierPtr structName,
                    std::optional<IdentifierPtr> traitName,
                    std::vector<FunctionDeclarationPtr> methods,
                    std::vector<GenericParameterPtr> genericParams);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

## 11. Enum Variant (`vyn::ast::EnumVariant`)

Represents a variant within an enum declaration. This is a helper structure and inherits directly from `Node`.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class EnumVariant : public Node { // Inherits from Node
public:
    IdentifierPtr name;
    std::optional<TypeNodePtr> associatedType; // e.g., Option::Some(T) -> T is associatedType

    EnumVariant(SourceLocation loc, IdentifierPtr name, std::optional<TypeNodePtr> associatedType);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

## 12. Enum Declaration (`vyn::ast::EnumDeclaration`)

Represents the declaration of an enumeration.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class EnumDeclaration : public Declaration {
public:
    IdentifierPtr name;
    std::vector<EnumVariant> variants; // Note: stores EnumVariant objects directly
    std::vector<GenericParameterPtr> genericParameters;

    EnumDeclaration(SourceLocation loc, IdentifierPtr name,
                    std::vector<EnumVariant> variants,
                    std::vector<GenericParameterPtr> genericParams);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

## 13. Generic Parameter (`vyn::ast::GenericParameter`)

Represents a generic parameter in a declaration (e.g., `T` in `foo<T>(p<T>)` or
`struct Bar<T<SomeAspect>>`). This node was formerly named `GenericParamNode`. It inherits
directly from `Node`. Vyn uses `<T<Aspect>>` bound syntax — not `T: Aspect`.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class GenericParameter : public Node {
public:
    IdentifierPtr name;
    std::optional<TypeNodePtr> constraint; // Optional aspect constraint

    GenericParameter(SourceLocation loc, IdentifierPtr name, std::optional<TypeNodePtr> constraint);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

## 14. Template Declaration (`vyn::ast::TemplateDeclaration`)

Represents a template declaration, which is a parameterized declaration.
*Note: The exact semantics and usage of `TemplateDeclaration` are still being refined. It might be used for concepts like C++ templates or for more advanced metaprogramming features.*

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class TemplateDeclaration : public Declaration {
public:
    IdentifierPtr name; // Name of the template itself
    std::vector<GenericParameterPtr> parameters; // Template parameters
    DeclPtr declaration; // The actual declaration being templated (e.g., a FunctionDeclaration, StructDeclaration)

    TemplateDeclaration(SourceLocation loc, IdentifierPtr name,
                        std::vector<GenericParameterPtr> parameters,
                        DeclPtr declaration);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

## 15. Module (`vyn::ast::Module`)

Represents a module, which is typically a file containing a collection of declarations. This node inherits directly from `Node` as it's a top-level container rather than a language-level declaration statement.

```cpp
// Structure in include/vyn/parser/ast.hpp
namespace vyn::ast {

class Module : public Node { // Inherits from Node
public:
    std::optional<IdentifierPtr> name; // Optional module name (e.g., from 'module my_module;')
    std::vector<DeclPtr> declarations;

    Module(SourceLocation loc, std::optional<IdentifierPtr> name, std::vector<DeclPtr> declarations);
    // ... accept, getType, toString methods ...
};

} // namespace vyn::ast
```

This covers the primary declaration AST nodes. For planned declarations, refer to `AST_Roadmap.md`.
