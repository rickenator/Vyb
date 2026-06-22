# Vyb Aspect System Tests

This directory contains test files for the Vyb aspect system implementation.

## Working Features (Phase 1 & 2)

### ✅ Basic Aspect Declarations
```vyb
aspect Printable {
    print(self)<Void> -> { }
    describe(self)<String> -> { }
}
```

### ✅ Aspect Implementations for Structs
```vyb
struct Point {
    x<Int>
    y<Int>
}

bind Printable -> Point {
    print(self<Point>)<Void> -> {
        // implementation
    }
}
```

### ✅ Calling Aspect Methods
```vyb
p<Point> = Point { x = 10, y = 20 }
p.print()           // Calls aspect method
result<String> = p.describe()
```

**Test Files:**
- `test_aspect_basic.vyb` - Basic aspect declaration and validation
- `test_aspect_simple.vyb` - ✅ **PASSING** - Aspect method calls with return values

## Working Features (Phase 3)

### ✅ Generic Aspect Implementations
```vyb
// Phase 3 complete
bind<T> Container -> Vec<T> {
    size(self)<Int> -> {
        return self.len()
    }

    is_empty(self)<Bool> -> {
        return self.len() == 0
    }
}
```

**Features:**
- Type parameter registration and validation
- Generic types accepted in bind arrow clause
- Scope isolation for type parameters
- Foundation for monomorphization

**Test Files:**
- `test_aspect_generic.vyb` - ✅ **WORKING** - Generic bind with type parameters

## Working Features (Phase 4)

### ✅ Type Parameter Substitution in Method Bodies
```vyb
// Phase 4 complete - Type parameters work everywhere!
struct Box<T> {
    value<T>  // Type parameter in struct field
}

bind<T> Display -> Box<T> {
    show(self)<Void> -> {
        // T is recognized and available in method body
        temp<T> = self.value  // Can use T for local variables
        return
    }
}
```

**Features:**
- ✅ Generic struct declarations with type parameters
- ✅ Type parameters in struct fields
- ✅ Type parameters in bind method signatures
- ✅ Type parameters in method bodies
- ✅ Proper scope management for type parameters
- ✅ Complete semantic analysis support

**Test Files:**
- `test_type_param_simple.vyb` - ✅ **PASSING** - Generic struct and aspect bind validate correctly

## Remaining Advanced Features

The current suite covers concrete binds, generic binds, receiver shorthand, aspect bounds,
and the first associated-type slice. Remaining advanced work is tracked in `TODO.md`:

- **Bind selection precedence**: choosing a bounded bind over an unbounded bind when both match
- **Aspect objects**: dynamic dispatch with `dyn Aspect`
- **Aspect inheritance**: super-aspect requirements
- **Full associated-type integration**: defaults, constraints, and dynamic-dispatch interactions

## Test Results

| Test File | Status | Returns | Notes |
|-----------|--------|---------|-------|
| test_aspect_basic.vyb | ✅ Parses | N/A | Declaration validation |
| test_aspect_simple.vyb | ✅ **PASSES** | 25 | Method calls work! |
| test_aspect_generic.vyb | ❌ Fails | - | Needs Phase 3 |
| test_aspect_vec.vyb | ❌ Fails | - | Needs Phase 3 |

## Implementation Notes

### Phase 1: Declarations (Complete)
- Parser supports `aspect` and `bind` keywords
- AST nodes: `AspectDeclaration`, `BindDeclaration`
- Semantic validation of method signatures
- Aspect registry and implementation registry

### Phase 2: Method Dispatch (Complete)
- **Semantic Analysis**: Resolves aspect methods in CallExpression
  - Checks if `obj.method()` is an aspect method
  - Looks up implementation in `aspectImpls` registry
  - Sets return type from aspect method signature

- **Code Generation**: LLVM codegen for aspect method calls
  - Detects MemberExpression in CallExpression
  - Extracts object type and method name
  - Looks up bind function in LLVM module
  - Passes object as first argument (self parameter)
  - Generates function call with correct types

### Phase 3: Generics (Planned)
Would require significant compiler infrastructure:
1. **Type Parameter Scope**: Track `<T>` in bind blocks
2. **Type Unification**: Match `Vec<T>` with `Vec<Int>`
3. **Monomorphization**: Generate code for each concrete type
4. **Substitution**: Replace T with actual type throughout bind

This is a major undertaking similar to Rust's trait monomorphization.

## Running Tests

```bash
# Working test (should return 25)
build/vyb test/aspect/test_aspect_simple.vyb

# Future tests (will fail with semantic errors)
build/vyb test/aspect/test_aspect_generic.vyb
build/vyb test/aspect/test_aspect_vec.vyb
```

## Architecture

```
Aspect System Flow:
├── Parser → AspectDeclaration, BindDeclaration
├── Semantic Analyzer
│   ├── Register aspects in aspectRegistry
│   ├── Validate bind blocks match aspect signatures
│   ├── Store binds in aspectImpls map
│   └── Resolve aspect methods in CallExpression
└── LLVM Codegen
    ├── Generate functions for bind methods
    ├── Detect aspect method calls in CallExpression
    └── Generate dispatch to bind functions
```

## Future Work

1. **Generic Aspect Binds** (Phase 3)
   - Type parameter infrastructure
   - Monomorphization engine
   - Generic type matching

2. **Aspect Bounds** (Phase 4)
   - Constraint checking on generics
   - `where` clause support

3. **Advanced Features** (Phase 5+)
   - Default implementations
   - Aspect objects
   - Associated types
   - Super-aspects

---

**Status**: Phase 2 Complete ✅ - Basic aspect method calls working!
**Next**: Phase 3 - Generic aspect implementations (complex, future work)
