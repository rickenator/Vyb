# Vyn Trait System Design

**Version:** 0.4.2 (Planned)  
**Status:** Design Phase  
**Priority:** HIGH  

## Overview

Vyn's trait system provides interface-based polymorphism without mandatory classes. Traits define shared behavior that types can implement, enabling generic programming with compile-time guarantees.

## Core Principles

1. **Opt-in Polymorphism** - Traits are optional; simple structs work without them
2. **No Mandatory Classes** - Structs can implement traits directly
3. **Compile-time Dispatch** - Static dispatch via monomorphization (no vtables by default)
4. **Explicit Implementation** - Types explicitly `impl` traits, no duck typing
5. **Trait Bounds** - Generic type parameters can require trait implementation

## Design Philosophy

### Why Not Mandatory Classes?

```vyn
// Simple data without behavior - no class needed
struct Point {
    x<Int>,
    y<Int>
}

// Behavior through standalone functions
distance(p1<Point>, p2<Point>)<Float> -> {
    dx<Int> = p1.x - p2.x
    dy<Int> = p1.y - p2.y
    return sqrt((dx * dx + dy * dy).to_float())
}

// This is valid and preferred for simple data types
```

**vs. mandatory classes:**
```vyn
// Unnecessary ceremony for simple data
class Point {
    x<Int>
    y<Int>
    
    new(x<Int>, y<Int>)<Point> -> { ... }  // Boilerplate
    distance(other<Point>)<Float> -> { ... } // Forces OOP mindset
}
```

**Vyn's Approach:**
- Use **structs** for data
- Use **functions** for behavior
- Use **traits** when you need polymorphism
- Use **classes** when you want encapsulation and inheritance (future feature)

## Trait Declaration Syntax

### Basic Trait

```vyn
trait Comparable {
    // Method signatures only - no implementations
    lt(self<their<Self>>, other<their<Self>>)<Bool> ->
    gt(self<their<Self>>, other<their<Self>>)<Bool> ->
    eq(self<their<Self>>, other<their<Self>>)<Bool> ->
}
```

**Key Features:**
- `Self` represents the implementing type
- Methods take `their<Self>` (borrowed self) by convention
- No method bodies - pure interface

### Trait with Default Methods

```vyn
trait Comparable {
    // Required methods
    lt(self<their<Self>>, other<their<Self>>)<Bool> ->
    eq(self<their<Self>>, other<their<Self>>)<Bool> ->
    
    // Default implementations (can be overridden)
    gt(self<their<Self>>, other<their<Self>>)<Bool> -> {
        return !self.lt(other) && !self.eq(other)
    }
    
    lte(self<their<Self>>, other<their<Self>>)<Bool> -> {
        return self.lt(other) || self.eq(other)
    }
}
```

### Trait Inheritance

```vyn
trait Equatable {
    eq(self<their<Self>>, other<their<Self>>)<Bool> ->
}

trait Comparable : Equatable {
    // Requires Equatable::eq to be implemented
    lt(self<their<Self>>, other<their<Self>>)<Bool> ->
    gt(self<their<Self>>, other<their<Self>>)<Bool> ->
}
```

## Trait Implementation Syntax

### Implementing for Structs

```vyn
struct Point {
    x<Int>,
    y<Int>
}

impl Comparable for Point {
    lt(self<their<Point>>, other<their<Point>>)<Bool> -> {
        return self.x < other.x || (self.x == other.x && self.y < other.y)
    }
    
    eq(self<their<Point>>, other<their<Point>>)<Bool> -> {
        return self.x == other.x && self.y == other.y
    }
    
    // gt() uses default implementation from trait
}
```

### Implementing for Primitive Types

```vyn
// Extend built-in types with new traits
impl Numeric for Int {
    add(self<Int>, other<Int>)<Int> -> { return self + other }
    sub(self<Int>, other<Int>)<Int> -> { return self - other }
    mul(self<Int>, other<Int>)<Int> -> { return self * other }
    div(self<Int>, other<Int>)<Int> -> { return self / other }
}
```

### Generic Implementations

```vyn
// Implement trait for all types that meet constraints
impl<T: Comparable> Equatable for Vec<T> {
    eq(self<their<Vec<T>>>, other<their<Vec<T>>>)<Bool> -> {
        if (self.len() != other.len()) {
            return false
        }
        i<Int> = 0
        while (i < self.len()) {
            if (!self.get(i).eq(other.get(i))) {
                return false
            }
            i = i + 1
        }
        return true
    }
}
```

## Using Traits

### Trait Bounds on Templates

```vyn
// Simple bound
template Container<T: Comparable> {
    struct Data {
        value<T>
    }
}

// Multiple bounds
template SortedList<T: Comparable + Equatable> {
    items<Vec<T>>
}

// Multiple parameters with different bounds
template HashMap<K: Hashable + Equatable, V> {
    buckets<Vec<Pair<K, V>>>
}
```

### Trait Objects (Future - Dynamic Dispatch)

```vyn
// Dynamic dispatch using trait objects (planned)
draw_shape(shape<dyn Drawable>)<Void> -> {
    shape.draw()  // Runtime dispatch
}

// vs. static dispatch (current)
draw_shape<T: Drawable>(shape<T>)<Void> -> {
    shape.draw()  // Compile-time dispatch
}
```

## Standard Library Traits

### Core Traits (Priority)

```vyn
trait Equatable {
    eq(self<their<Self>>, other<their<Self>>)<Bool> ->
    ne(self<their<Self>>, other<their<Self>>)<Bool> -> {
        return !self.eq(other)  // Default
    }
}

trait Comparable : Equatable {
    lt(self<their<Self>>, other<their<Self>>)<Bool> ->
    lte(self<their<Self>>, other<their<Self>>)<Bool> -> {
        return self.lt(other) || self.eq(other)
    }
    gt(self<their<Self>>, other<their<Self>>)<Bool> -> {
        return !self.lte(other)
    }
    gte(self<their<Self>>, other<their<Self>>)<Bool> -> {
        return !self.lt(other)
    }
}

trait Numeric {
    add(self<Self>, other<Self>)<Self> ->
    sub(self<Self>, other<Self>)<Self> ->
    mul(self<Self>, other<Self>)<Self> ->
    div(self<Self>, other<Self>)<Self> ->
    zero()<Self> ->  // Static method
}

trait Hashable : Equatable {
    hash(self<their<Self>>)<UInt64> ->
}

trait Cloneable {
    clone(self<their<Self>>)<Self> ->
}

trait Display {
    to_string(self<their<Self>>)<String> ->
}
```

### Advanced Traits (Future)

```vyn
trait Iterator<T> {
    next(self<their<Self>>)<Option<T>> ->
    has_next(self<their<Self>>)<Bool> ->
}

trait Serializable {
    serialize(self<their<Self>>)<Bytes> ->
    deserialize(data<their<Bytes>>)<Result<Self, Error>> ->
}

trait Async<T> {
    await(self<Self>)<T> ->
}
```

## Class Types (Future Feature)

### When to Use Classes vs Structs+Traits

**Use Structs + Traits when:**
- Simple data types (Point, Color, Date)
- Immutable data structures
- Functional programming style
- No need for inheritance
- Explicit behavior through traits

**Use Classes when:**
- Need inheritance hierarchy
- Want encapsulation with private fields
- Object-oriented design patterns
- Mutable state management
- Need constructors and destructors

### Class Syntax (Proposed)

```vyn
class Animal {
    // Private by default
    name<String>
    age<Int>
    
    // Constructor
    pub new(name<String>, age<Int>)<Animal> -> {
        self.name = name
        self.age = age
    }
    
    // Public method
    pub speak(self<their<Animal>>)<Void> -> {
        println("Animal speaks")
    }
    
    // Protected method (for subclasses)
    protected get_info(self<their<Animal>>)<String> -> {
        return self.name + " is " + self.age.to_string()
    }
}

// Inheritance
class Dog : Animal {
    breed<String>
    
    pub new(name<String>, age<Int>, breed<String>)<Dog> -> {
        super.new(name, age)  // Call parent constructor
        self.breed = breed
    }
    
    // Override method
    pub speak(self<their<Dog>>)<Void> -> {
        println("Woof!")
    }
}

// Implement traits for classes
impl Comparable for Dog {
    lt(self<their<Dog>>, other<their<Dog>>)<Bool> -> {
        return self.age < other.age
    }
}
```

**Key Class Features:**
- Inheritance with `:`
- Method overriding
- Access modifiers (pub, protected, private)
- Constructors with `new`
- `super` for parent access
- Still implement traits like structs

## Implementation Phases

### Phase 1: Trait Declarations (v0.4.2) - HIGH PRIORITY

**Goal:** Make trait definitions real and usable

**Tasks:**
1. ✅ Parse trait declarations (already working)
2. ⬜ Store trait definitions in semantic analyzer
3. ⬜ Validate trait method signatures
4. ⬜ Register trait in type system
5. ⬜ Check trait inheritance (trait A : B)

**Test:**
```vyn
trait Comparable {
    lt(self<their<Self>>, other<their<Self>>)<Bool> ->
}

// Should register and validate successfully
```

### Phase 2: Trait Implementation (v0.4.2) - HIGH PRIORITY

**Goal:** Allow types to implement traits

**Tasks:**
1. ⬜ Parse impl blocks (already working)
2. ⬜ Validate impl matches trait signature
3. ⬜ Check all required methods implemented
4. ⬜ Register impl in type system
5. ⬜ Enable trait method calls (value.method())

**Test:**
```vyn
struct Point { x<Int>, y<Int> }

impl Comparable for Point {
    lt(self<their<Point>>, other<their<Point>>)<Bool> -> {
        return self.x < other.x
    }
}

main()<Int> -> {
    p1<Point> = Point { x = 1, y = 2 }
    p2<Point> = Point { x = 3, y = 4 }
    result<Bool> = p1.lt(p2)  // Should work
    return 0
}
```

### Phase 3: Template Instantiation (v0.4.3)

**Goal:** Make templates actually usable

**Tasks:**
1. ⬜ Implement monomorphization (template expansion)
2. ⬜ Validate trait bounds during instantiation
3. ⬜ Generate specialized code for each concrete type
4. ⬜ Cache instantiations to avoid duplication
5. ⬜ Support template functions and structs

**Test:**
```vyn
template Container<T: Comparable> {
    struct Data { value<T> }
}

main()<Int> -> {
    int_data<Container<Int>::Data> = Container<Int>::Data { value = 42 }
    return int_data.value
}
```

### Phase 4: Advanced Trait Features (v0.5.0)

**Goal:** Complete trait system

**Tasks:**
1. ⬜ Default method implementations
2. ⬜ Associated types
3. ⬜ Trait objects (dynamic dispatch)
4. ⬜ Generic impl blocks
5. ⬜ Trait aliases

### Phase 5: Class System (v0.6.0)

**Goal:** Optional OOP features

**Tasks:**
1. ⬜ Class declarations with inheritance
2. ⬜ Access modifiers (pub, protected, private)
3. ⬜ Constructors and destructors
4. ⬜ Method overriding
5. ⬜ Abstract classes

## Roadmap Priority

### Immediate (v0.4.2)
1. **Trait declarations** - Make traits real types
2. **Trait implementations** - Connect types to traits
3. **Trait method calls** - value.method() syntax
4. **Trait bounds validation** - Check T: Comparable works

### Short-term (v0.4.3)
1. **Template instantiation** - Monomorphization
2. **Generic functions** - max<T: Comparable>(a, b)
3. **Trait inheritance** - trait A : B

### Medium-term (v0.5.0)
1. **Default methods** - Trait with implementations
2. **Associated types** - type Item in Iterator
3. **Generic impl** - impl<T> Trait for Vec<T>

### Long-term (v0.6.0)
1. **Class system** - Inheritance and OOP
2. **Trait objects** - Dynamic dispatch
3. **Advanced generics** - Higher-kinded types

## Design Questions

### Q: Why allow both structs and classes?

**A:** Flexibility. Simple data types shouldn't require class ceremony. Classes are for when you want OOP patterns.

```vyn
// Simple: Just data + functions
struct Point { x<Int>, y<Int> }
distance(p1<Point>, p2<Point>)<Float> -> { ... }

// Complex: Encapsulation + inheritance
class Shape {
    protected center<Point>
    pub area(self<their<Shape>>)<Float> ->  // Abstract
}
```

### Q: How to avoid trait/class confusion?

**A:** Clear guidelines:
- **Struct** = Data structure (no behavior)
- **Trait** = Interface/capability (behavior contract)
- **Class** = Object with state + behavior + inheritance

### Q: Performance implications?

**A:**
- **Traits with templates** → Zero-cost (static dispatch)
- **Trait objects** → Dynamic dispatch (small vtable cost)
- **Classes** → Same as C++ virtual calls when needed

### Q: How to handle trait conflicts?

```vyn
trait A { fn method(self<their<Self>>)<Int> -> }
trait B { fn method(self<their<Self>>)<String> -> }

struct MyType { }
impl A for MyType { ... }
impl B for MyType { ... }

// Explicit disambiguation
x<Int> = value.A::method()
y<String> = value.B::method()
```

## Examples

### Full Working Example (Target v0.4.2)

```vyn
// Define trait
trait Comparable {
    lt(self<their<Self>>, other<their<Self>>)<Bool> ->
    eq(self<their<Self>>, other<their<Self>>)<Bool> ->
}

// Define struct
struct Point {
    x<Int>,
    y<Int>
}

// Implement trait for struct
impl Comparable for Point {
    lt(self<their<Point>>, other<their<Point>>)<Bool> -> {
        return self.x < other.x || (self.x == other.x && self.y < other.y)
    }
    
    eq(self<their<Point>>, other<their<Point>>)<Bool> -> {
        return self.x == other.x && self.y == other.y
    }
}

// Use in generic function
min<T: Comparable>(a<T>, b<T>)<T> -> {
    if (a.lt(b)) {
        return a
    } else {
        return b
    }
}

main()<Int> -> {
    p1<Point> = Point { x = 1, y = 2 }
    p2<Point> = Point { x = 3, y = 4 }
    
    smaller<Point> = min(p1, p2)  // Works! Monomorphized to min<Point>
    
    return smaller.x
}
```

## Conclusion

Vyn's trait system balances:
- **Simplicity** - Structs for data, traits for interfaces
- **Power** - Generic programming with compile-time safety
- **Flexibility** - Optional classes for OOP when needed
- **Performance** - Zero-cost abstractions via monomorphization

This design avoids forcing users into OOP while still providing powerful abstraction mechanisms when needed.
