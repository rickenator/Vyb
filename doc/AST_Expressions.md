# AST Nodes: Expressions

This document describes the Abstract Syntax Tree (AST) nodes for various expressions in the Vyb programming language, as defined in `include/vyb/parser/ast.hpp`. Expressions are constructs that evaluate to a value.

All expression nodes inherit from `vyb::ast::Expression`.

## Common Pointer Aliases

Throughout the AST definitions, the following smart pointer aliases are used for brevity and clarity, representing `std::unique_ptr<T>`:

- `ExprPtr = std::unique_ptr<Expression>;`
- `IdentifierPtr = std::unique_ptr<Identifier>;`
- `TypeNodePtr = std::unique_ptr<TypeNode>;`

## 1. `UnaryExpression`

Represents a unary expression (e.g., `-x`, `!flag`).

-   **C++ Class**: `vyb::ast::UnaryExpression`
-   **`NodeType`**: `UNARY_EXPRESSION`
-   **Fields**:
    -   `op` (`std::string`): The unary operator (e.g., "-", "!").
    -   `operand` (`ExprPtr`): The expression the operator applies to.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class UnaryExpression : public Expression {
public:
    std::string op;
    ExprPtr operand;

    UnaryExpression(SourceLocation loc, std::string op, ExprPtr operand);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 2. `BinaryExpression`

Represents a binary expression (e.g., `a + b`, `x == y`).

-   **C++ Class**: `vyb::ast::BinaryExpression`
-   **`NodeType`**: `BINARY_EXPRESSION`
-   **Fields**:
    -   `op` (`std::string`): The binary operator (e.g., "+", "==").
    -   `left` (`ExprPtr`): The left-hand side operand.
    -   `right` (`ExprPtr`): The right-hand side operand.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class BinaryExpression : public Expression {
public:
    std::string op;
    ExprPtr left;
    ExprPtr right;

    BinaryExpression(SourceLocation loc, std::string op, ExprPtr left, ExprPtr right);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 3. `CallExpression`

Represents a function or method call.

-   **C++ Class**: `vyb::ast::CallExpression`
-   **`NodeType`**: `CALL_EXPRESSION`
-   **Fields**:
    -   `callee` (`ExprPtr`): The expression that evaluates to the function to be called (e.g., an identifier or a member expression).
    -   `arguments` (`std::vector<ExprPtr>`): The list of arguments passed to the function.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class CallExpression : public Expression {
public:
    ExprPtr callee;
    std::vector<ExprPtr> arguments;

    CallExpression(SourceLocation loc, ExprPtr callee, std::vector<ExprPtr> arguments);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

Intrinsics Parsed as CallExpression
----------------------------------

Several types of intrinsics are parsed as CallExpression nodes with specific `callee` identifiers:

### Borrowing Intrinsics

The canonical `borrow(expr)` and `view(expr)` forms are parsed as `BorrowExpression` nodes.
They produce `their<T>` or `their<T const>` respectively.

### Serialization Mode Intrinsics

The following serialization intrinsics are parsed as CallExpression nodes:

- **`lit(value)`**: Emits raw JSON literals without type wrapping. Restricted to primitive values (Int, Float, String, Bool).
- **`notype(value)`**: Removes `<Type>` suffixes from field names in struct serialization. Only valid for structs.
- **`bare(value)`**: Emits only raw field values as JSON array, removing all type and field metadata. Only valid for structs.
- **`deserial(json_string)`**: Deserializes JSON string back to typed Vyb values.

These intrinsics are primarily used for customizing JSON serialization behavior of `main()` function returns.
See `doc/Auto_Serialization_Main_Returns.md` for comprehensive documentation.

## 4. `ConstructionExpression`

Represents an expression that constructs an instance of a type using constructor-like syntax (e.g., `Point(10, 20)`).

-   **C++ Class**: `vyb::ast::ConstructionExpression`
-   **`NodeType`**: `CONSTRUCTION_EXPRESSION`
-   **Fields**:
    -   `type` (`TypeNodePtr`): The type being constructed.
    -   `arguments` (`std::vector<ExprPtr>`): The arguments passed to the constructor.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class ConstructionExpression : public Expression {
public:
    TypeNodePtr type;
    std::vector<ExprPtr> arguments;

    ConstructionExpression(SourceLocation loc, TypeNodePtr type, std::vector<ExprPtr> arguments);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 5. `ArrayInitializationExpression`

Represents the initialization of an array with a specific type and size, potentially with an initializer list (e.g., `[int; 3](1, 2, 3)` or `[int; 10]()`).

-   **C++ Class**: `vyb::ast::ArrayInitializationExpression`
-   **`NodeType`**: `ARRAY_INITIALIZATION_EXPRESSION`
-   **Fields**:
    -   `elementType` (`TypeNodePtr`): The type of the array elements.
    -   `size` (`ExprPtr`): The size of the array.
    -   `initializerList` (`std::optional<std::vector<ExprPtr>>`): Optional list of expressions to initialize array elements.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class ArrayInitializationExpression : public Expression {
public:
    TypeNodePtr elementType;
    ExprPtr size;
    std::optional<std::vector<ExprPtr>> initializerList;

    ArrayInitializationExpression(SourceLocation loc, TypeNodePtr elementType, ExprPtr size, std::optional<std::vector<ExprPtr>> initializerList);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 6. `MemberExpression`

Represents accessing a member of a struct, class, or object (e.g., `object.field`, `ptr->field`, `module::item`).

-   **C++ Class**: `vyb::ast::MemberExpression`
-   **`NodeType`**: `MEMBER_EXPRESSION`
-   **Fields**:
    -   `object` (`ExprPtr`): The expression whose member is being accessed.
    -   `member` (`IdentifierPtr`): The identifier of the member.
    -   `isArrow` (`bool`): True if accessed with `->` (pointer dereference), false for `.`.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class MemberExpression : public Expression {
public:
    ExprPtr object;
    IdentifierPtr member;
    bool isArrow; // True for ->, false for .

    MemberExpression(SourceLocation loc, ExprPtr object, IdentifierPtr member, bool isArrow);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 7. `AssignmentExpression`

Represents an assignment operation (e.g., `variable = value`).

-   **C++ Class**: `vyb::ast::AssignmentExpression`
-   **`NodeType`**: `ASSIGNMENT_EXPRESSION`
-   **Fields**:
    -   `left` (`ExprPtr`): The left-hand side of the assignment (l-value).
    -   `right` (`ExprPtr`): The right-hand side of the assignment (r-value).

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class AssignmentExpression : public Expression {
public:
    ExprPtr left;
    ExprPtr right;

    AssignmentExpression(SourceLocation loc, ExprPtr left, ExprPtr right);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 8. `IfExpression`

Represents an if-expression, which evaluates to one of two values based on a condition (e.g., `if (cond) expr1 else expr2`).

-   **C++ Class**: `vyb::ast::IfExpression`
-   **`NodeType`**: `IF_EXPRESSION`
-   **Fields**:
    -   `condition` (`ExprPtr`): The condition to evaluate.
    -   `thenBranch` (`ExprPtr`): The expression to evaluate if the condition is true.
    -   `elseBranch` (`ExprPtr`): The expression to evaluate if the condition is false (must be present for it to be an expression).

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class IfExpression : public Expression {
public:
    ExprPtr condition;
    ExprPtr thenBranch;
    ExprPtr elseBranch; // Mandatory for an if-expression

    IfExpression(SourceLocation loc, ExprPtr condition, ExprPtr thenBranch, ExprPtr elseBranch);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 9. `RangeExpression`

Represents a range expression (e.g., `1..10`, `a..=b`).

-   **C++ Class**: `vyb::ast::RangeExpression`
-   **`NodeType`**: `RANGE_EXPRESSION`
-   **Fields**:
    -   `start` (`ExprPtr`): The starting value of the range.
    -   `end` (`ExprPtr`): The ending value of the range.
    -   `isInclusive` (`bool`): Whether the range includes the end value (true for `..=`, false for `..`).

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class RangeExpression : public Expression {
public:
    ExprPtr start;
    ExprPtr end;
    bool isInclusive;

    RangeExpression(SourceLocation loc, ExprPtr start, ExprPtr end, bool isInclusive);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 10. `BorrowExpression`

Represents a borrow expression (e.g., `&x`, `&mut y`).

-   **C++ Class**: `vyb::ast::BorrowExpression`
-   **`NodeType`**: `BORROW_EXPRESSION`
-   **Fields**:
    -   `operand` (`ExprPtr`): The expression being borrowed.
    -   `kind` (`BorrowKind`): The kind of borrow e.g., `SHARED`, `MUTABLE`, `UNIQUE_MUTABLE`. `BorrowKind` is an enum defined in `ast.hpp`.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
// enum class BorrowKind { SHARED, MUTABLE, UNIQUE_MUTABLE }; // Defined in ast.hpp
class BorrowExpression : public Expression {
public:
    ExprPtr operand;
    BorrowKind kind;

    BorrowExpression(SourceLocation loc, ExprPtr operand, BorrowKind kind);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 11. `PointerDerefExpression`

Represents dereferencing a pointer (e.g., `*ptr`).

-   **C++ Class**: `vyb::ast::PointerDerefExpression`
-   **`NodeType`**: `POINTER_DEREF_EXPRESSION`
-   **Fields**:
    -   `operand` (`ExprPtr`): The pointer expression being dereferenced.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class PointerDerefExpression : public Expression {
public:
    ExprPtr operand;

    PointerDerefExpression(SourceLocation loc, ExprPtr operand);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 12. `AddrOfExpression`

Represents the address-of operation (e.g., `&variable` in contexts where it means address, distinct from borrow for ownership system).

-   **C++ Class**: `vyb::ast::AddrOfExpression`
-   **`NodeType`**: `ADDR_OF_EXPRESSION`
-   **Fields**:
    -   `operand` (`ExprPtr`): The operand whose address is being taken.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class AddrOfExpression : public Expression {
public:
    ExprPtr operand;

    AddrOfExpression(SourceLocation loc, ExprPtr operand);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 13. `FromIntToLocExpression`

Represents a Vyb-specific expression for converting integers to a source location. Likely for internal or metaprogramming use.

-   **C++ Class**: `vyb::ast::FromIntToLocExpression`
-   **`NodeType`**: `FROM_INT_TO_LOC_EXPRESSION`
-   **Fields**:
    -   `line` (`ExprPtr`): Expression evaluating to the line number.
    -   `column` (`ExprPtr`): Expression evaluating to the column number.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class FromIntToLocExpression : public Expression {
public:
    ExprPtr line;
    ExprPtr column;

    FromIntToLocExpression(SourceLocation loc, ExprPtr line, ExprPtr column);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 14. `ArrayElementExpression`

Represents accessing an element of an array or slice (e.g., `array[index]`).

-   **C++ Class**: `vyb::ast::ArrayElementExpression`
-   **`NodeType`**: `ARRAY_ELEMENT_EXPRESSION`
-   **Fields**:
    -   `array` (`ExprPtr`): The array or slice expression.
    -   `index` (`ExprPtr`): The index expression.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class ArrayElementExpression : public Expression {
public:
    ExprPtr array;
    ExprPtr index;

    ArrayElementExpression(SourceLocation loc, ExprPtr array, ExprPtr index);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 15. `LocationExpression`

Represents an expression that evaluates to the current source code location. Likely for metaprogramming or debugging.

-   **C++ Class**: `vyb::ast::LocationExpression`
-   **`NodeType`**: `LOCATION_EXPRESSION`

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class LocationExpression : public Expression {
public:
    LocationExpression(SourceLocation loc);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 16. `ListComprehension`

Represents a list comprehension (e.g., `[x * 2 for x in items if x > 0]`).

-   **C++ Class**: `vyb::ast::ListComprehension`
-   **`NodeType`**: `LIST_COMPREHENSION`
-   **Fields**:
    -   `output` (`ExprPtr`): The expression for each element in the new list (e.g., `x * 2`).
    -   `variable` (`IdentifierPtr`): The variable used in the comprehension (e.g., `x`).
    -   `iterable` (`ExprPtr`): The expression for the collection being iterated over (e.g., `items`).
    -   `condition` (`std::optional<ExprPtr>`): An optional condition to filter elements (e.g., `x > 0`).

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class ListComprehension : public Expression {
public:
    ExprPtr output;
    IdentifierPtr variable;
    ExprPtr iterable;
    std::optional<ExprPtr> condition;

    ListComprehension(SourceLocation loc, ExprPtr output, IdentifierPtr variable, ExprPtr iterable, std::optional<ExprPtr> condition);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 17. `ConstructionExpression`

Represents a constructor call or type instantiation expression (e.g., `MyStruct(1, "hello")`, `Vec<i32>(10)`). This node is also used for various intrinsics and type conversions.

-   **C++ Class**: `vyb::ast::ConstructionExpression`
-   **`NodeType`**: `CONSTRUCTION_EXPRESSION`
-   **Fields**:
    -   `constructedType` (`TypeNodePtr`): The type being constructed or instantiated.
    -   `arguments` (`std::vector<ExprPtr>`): The arguments passed to the constructor.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class ConstructionExpression : public Expression {
public:
    TypeNodePtr constructedType; // The type being constructed (e.g., MyStruct, my_module::MyType<T>)
    std::vector<ExprPtr> arguments;

    ConstructionExpression(SourceLocation loc, TypeNodePtr constructedType, std::vector<ExprPtr> arguments);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

### Memory Operation Expressions

Several memory operations in Vyb are represented using `ConstructionExpression` or `CallExpression` nodes:

#### 1. Creating pointers with `loc<T>(expr)`

Creates a pointer to a variable's memory location.

```
x<Int> = 42
p<loc<Int>> = loc(x) // Creates a pointer to x
```

AST Representation:
- When parsed as a `ConstructionExpression`:
  - `constructedType`: A `TypeName` node for "loc"
  - `arguments`: A vector containing one expression (the variable being pointed to)

#### 2. Converting between pointer types with `from<loc<T>>(expr)`

Converts an integer address or a different pointer type to a specific pointer type.

```
addr<Int> = 0x12345678
p<loc<Int>> = from<loc<Int>>(addr) // Converts integer to pointer
```

AST Representation:
- Typically a `ConstructionExpression` with:
  - `constructedType`: A `GenericInstanceTypeNode` with the target type (e.g., `from<loc<Int>>`)
  - `arguments`: A vector containing one expression (the value to convert)

#### 3. Pointer dereferencing with `at(ptr)`

Accesses the value at a pointer's memory location.

```
p<loc<Int>> = loc(x)
y<Int> = at(p) // Reads from pointer
at(p) = 99;         // Writes to pointer
```

AST Representation:
- When parsed as a `CallExpression`:
  - `callee`: An `Identifier` node for "at"
  - `arguments`: A vector containing one expression (the pointer to dereference)
- When parsed as a `ConstructionExpression`:
  - `constructedType`: A `TypeName` node for "at"
  - `arguments`: A vector containing one expression (the pointer to dereference)

Note that the behavior of `at(ptr)` depends on context:
- On the right-hand side of an assignment: Returns the value at the pointer's address
- On the left-hand side of an assignment: Returns the pointer itself to enable direct assignment

## 18. `ArrayInitializationExpression`

Represents the initialization of an array with a specific type and size, potentially with an initializer list (e.g., `[int; 3](1, 2, 3)` or `[int; 10]()`).

-   **C++ Class**: `vyb::ast::ArrayInitializationExpression`
-   **`NodeType`**: `ARRAY_INITIALIZATION_EXPRESSION`
-   **Fields**:
    -   `elementType` (`TypeNodePtr`): The type of the array elements.
    -   `size` (`ExprPtr`): The size of the array.
    -   `initializerList` (`std::optional<std::vector<ExprPtr>>`): Optional list of expressions to initialize array elements.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class ArrayInitializationExpression : public Expression {
public:
    TypeNodePtr elementType;
    ExprPtr size;
    std::optional<std::vector<ExprPtr>> initializerList;

    ArrayInitializationExpression(SourceLocation loc, TypeNodePtr elementType, ExprPtr size, std::optional<std::vector<ExprPtr>> initializerList);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

## 19. `SelectExpression`

Represents a select expression for pattern matching that returns a value (e.g., `select(x) -> { 1 -> 10, 2 -> 20, ? -> 0 };`).

-   **C++ Class**: `vyb::ast::SelectExpression`
-   **`NodeType`**: `SELECT_EXPRESSION`
-   **Fields**:
    -   `value` (`ExprPtr`): The expression being matched against.
    -   `cases` (`std::vector<std::pair<ExprPtr, ExprPtr>>`): Vector of pattern-result pairs.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class SelectExpression : public Expression {
public:
    ExprPtr value;
    std::vector<std::pair<ExprPtr, ExprPtr>> cases; // pattern -> result

    SelectExpression(SourceLocation loc, ExprPtr value, std::vector<std::pair<ExprPtr, ExprPtr>> cases);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

**Key Characteristics**:
- Expression-based pattern matching (unlike match which is statement-based)
- Returns a value that can be assigned to variables
- Requires semicolon terminator: `select(...) -> { ... };`
- Supports two result forms:
  - **Naked expressions**: `pattern -> value` (auto-returns)
  - **Block expressions**: `pattern -> { statements; pass value }` (explicit return)
- Type inferred from first case
- Uses SelectContext stack for proper code generation

**Example**:
```vyb
result<Int> = select(status) -> {
    1 -> 100,           // Naked expression
    2 -> {
        x<Int> = 200;   // Block with statements
        pass x          // Explicit return
    },
    ? -> 0              // Wildcard default
};
```

## 20. `PassStatement`

Represents a pass statement that returns a value from a select block without returning from the enclosing function.

-   **C++ Class**: `vyb::ast::PassStatement`
-   **`NodeType`**: `PASS_STATEMENT`
-   **Fields**:
    -   `value` (`ExprPtr`): The expression to return from the select block.

```cpp
// From include/vyb/parser/ast.hpp
namespace vyb::ast {
class PassStatement : public Statement {
public:
    ExprPtr value;

    PassStatement(SourceLocation loc, ExprPtr value);
    // ... accept, getType, toString methods ...
};
} // namespace vyb::ast
```

**Key Characteristics**:
- Only valid inside select expression blocks
- Returns value from the **block**, not the enclosing function
- Requires an expression argument (cannot be empty)
- Semantic analysis checks that pass appears within select context
- Generates store to resultAlloca and branch to endBlock in LLVM IR

**Example**:
```vyb
select(code) -> {
    3 -> {
        msg<Int> = 300;
        println(msg);      // Side effect
        pass msg           // Return from this block
    }
};
```

*Note: `TernaryExpression` was previously listed but is not present in `include/vyb/parser/ast.hpp` and has been removed. Other expression types might be detailed in `AST_Literals.md` (like `Identifier`) or `AST_Types.md` (if type constructs can be expressions).* For planned expressions, refer to `AST_Roadmap.md`.
