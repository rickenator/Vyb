# Vyb Runtime: Mutability, Ownership, and References

## 1. Overview

Vyb's memory model is designed for safety and explicitness, drawing inspiration from modern systems languages. It distinguishes between:

1.  **Binding Mutability**: Whether a variable can be reassigned (default mutable vs `const`).
2.  **Ownership**: Who is responsible for managing the memory of data (`my<T>`, `our<T>`, `their<T>`).
3.  **Data Mutability**: Whether the data itself can be changed, often indicated by `const` on the type (e.g., `my<T const>`).
4.  **References (Borrows)**: How non-owning pointers (`their<T>`) are created and used via `borrow` and `view` keywords.

This document details these aspects of the Vyb runtime.

**Runtime Note**: As of v0.4.0, Vyb uses LLVM's modern ORC JIT infrastructure, providing robust memory management and excellent compatibility with the ownership and borrowing system described here.

## 2. Variable Declarations and Binding Mutability

Vyb uses two keywords for variable bindings:

*   **`var`**: Declares a mutable binding. The variable can be reassigned to a new value or a different instance of its type.

    ```vyb
    x<Int> = 10
    x = 20          // allowed: a later bare assignment updates the binding

    item<my<String>> = my("hello")
    item = my("world")   // allowed; the old "hello" is dropped
    ```

*   **`const`**: Declares an immutable binding. The variable cannot be reassigned after initialization.

    ```vyb
    PI<Float const> = 3.14159
    // PI = 3.0; // Error: cannot reassign a const binding

    GREETING<my<String const>> = my("Hello")
    // GREETING = my("Hi"); // Error
    ```
    Note: `const` on a binding only prevents reassignment. If the bound value holds a mutable type (e.g., `my<Data>`), the data *within* that value might still be modifiable through methods on `Data`, unless the type itself is immutable (e.g., `my<Data const>`).

    ```vyb
    struct Counter { value<Int> }

    bind Countable -> Counter {
        increment(self<their<Counter>>) -> { self.value = self.value + 1 }
    }

    c<my<Counter>> = my(Counter { value: 0 })
    c.increment()   // allowed: the handle is shared, so the object it points to is
                    // mutated through a by-reference bind method
                    // To prevent internal mutation, hold a read-only handle:
                    // `my<Counter const>`.
    ```

## 3. Ownership Qualifiers and Data Mutability

Vyb employs ownership types to manage memory and control data access:

*   **`my<T>`**: Unique-owning pointer (similar to Rust's `Box<T>`). Only one `my<T>` can own the data. When a `my<T>` goes out of scope, the data is deallocated.
*   **`our<T>`**: Shared-owning pointer (reference-counted, like `Rc<T>`/`Arc<T>`). Multiple `our<T>` pointers can co-own the data. The data is deallocated when the last `our<T>` is dropped.
*   **`their<T>`**: Borrowed pointer (non-owning reference, like `&T`/`&mut T`). It provides temporary access to data owned by `my<T>` or `our<T>`, or other `their<T>`.
*   **`ptr<T>`**: Raw pointer (like `T*`). Operations involving `ptr<T>` are typically restricted to `freedom` blocks.

**Data Mutability** is controlled by applying `const` to the type `T` *within* the ownership wrapper:

*   `my<T>`: Unique ownership of mutable data `T`.
*   `my<T const>`: Unique ownership of immutable data `T`.
*   `our<T>`: Shared ownership of mutable data `T` (requires synchronization for thread-safety).
*   `our<T const>`: Shared ownership of immutable data `T` (inherently thread-safe for reading).
*   `their<T>`: A mutable borrow of data `T`.
*   `their<T const>`: An immutable borrow view of data `T`.

## 4. Borrowing: Creating `their<T>` References

Borrowed references (`their<T>`) are created using `borrow(expr)` and `view(expr)`:

*   **`view(expr)`**: Creates an immutable borrow `their<T const>`. This provides a read-only view of the data.

    ```vyb
    owner<my<Foo>> = my(Foo{ value: 10 });
    immutable_ref<their<Foo const>> = view(owner);
    // immutable_ref.value = 20; // Error: cannot modify through their<T const>
    print(immutable_ref.value); // OK
    ```

*   **`borrow(expr)`**: Creates a mutable borrow `their<T>`. This allows modification of the data, subject to borrowing rules (e.g., no other active borrows to the same data).

    ```vyb
    owner<my<Foo>> = my(Foo{ value: 10 });
    mutable_ref<their<Foo>> = borrow(owner);
    mutable_ref.value = 20; // OK, owner.value is now 20
    print(mutable_ref.value); // OK
    ```

The compiler enforces borrow-checking rules to ensure memory safety (e.g., preventing simultaneous mutable and immutable borrows to the same data, or ensuring borrows do not outlive the owner). These safe checked borrows do not require `freedom`.

## 5. Function Parameters

Function parameters use ownership types to define how arguments are passed:

*   **`param: T`** (where `T` is a value type like `Int`, `Bool`, or a struct passed by value): The argument is passed by value (copied).

    ```vyb
    process_value(data<Int>) -> { /* ... */ }
    process_value(10)
    ```

*   **`param: my<T>`** (or `our<T>`): The argument is moved into the function. The caller loses ownership.

    ```vyb
    consume_data(data<my<Foo>>) -> { /* data is now owned by this function */ }
    my_foo<my<Foo>> = my(Foo { value: 0 })
    consume_data(my_foo)
    // my_foo is no longer valid here
    ```

*   **`param: their<T>`**: The function receives a mutable borrow. The original data must be accessible via a mutable path.

    ```vyb
    modify_data(data<their<Foo>>) -> {
        data.value = data.value + 1
    }
    owner<my<Foo>> = my(Foo { value: 5 })
    modify_data(borrow(owner))   // owner.value becomes 6
    ```

*   **`param: their<T const>`**: The function receives an immutable borrow. The original data can be mutable or immutable.

    ```vyb
    read_data(data<their<Foo const>>) -> {
        print(data.value)
        // data.value = 10; // Error
    }
    owner_mut<my<Foo>> = my(Foo { value: 7 })
    owner_const<my<Foo const>> = my(Foo { value: 8 })

    read_data(view(owner_mut))
    read_data(view(owner_const))
    ```

## 6. Struct and Class Fields

Fields within structs and classes are declared with a name and a type. Their mutability characteristics are primarily determined by the type system and the mutability of the struct/class instance.

```vyb
struct Point {
    x<Int>,            // a value-type field
    y<Int>,
    label<String>      // a String field
}

// Instance mutability:
p1<my<Point>> = my(Point { x: 10, y: 20, label: "info" })
p1.x = 15                   // allowed: `p1` is a mutable handle and `x` is a value field
p1.label = "new_info"       // allowed: the old value is dropped

// A read-only handle: writes through it are rejected.
p2<my<Point const>> = my(Point { x: 0, y: 0, label: "const_info" })
// p2.x = 5; // Error: fields of a `Point const` cannot be written through this handle

// Fields are declared `name<Type>`; a field-level const is `name<Type const>`.
struct Config {
    refresh_rate<Int>,
    api_key<String const>
}
my_config<Config> = Config { refresh_rate: 60, api_key: "xyz" }
my_config.refresh_rate = 30   // OK
// my_config.api_key = "abc"; // Error: api_key is declared const
```
The interaction between instance binding mutability (`var`/`const`), ownership types (`my<T>`, `my<T const>`), and potential field-level `var`/`const` specifiers defines the overall mutability. The primary mechanism should be instance binding and type-level constness.

## 7. Rationale

This memory model, centered around `var`/`const` bindings, `my`/`our`/`their` ownership, and `view`/`borrow` for references, aims to:

-   **Ensure Memory Safety**: Through compile-time checks like borrow checking and ownership tracking.
-   **Provide Clarity**: Explicit ownership and borrowing make data flow and lifetime predictable.
-   **Offer Control**: Developers have fine-grained control over mutability at both the binding and type levels.
-   **Enable Concurrency**: `our<T const>` is inherently safe for concurrent reads, and `our<Mutex<T>>` (or similar) can be used for shared mutable state.

This model aligns Vyb with modern practices for safe and efficient systems programming.
