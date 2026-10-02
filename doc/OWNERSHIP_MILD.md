# Vyb Ownership Types: `mild<T>`

> **Status: shipped (v0.7.6).** `my<T>` / `our<T>` / `their<T>` / `mild<T>` are
> implemented and runtime-enforced: compile-time `my<T>` move tracking, `our<T>`
> copy/assignment/parameter refcounting over an atomic control block, a lexical
> `their<T>` borrow checker, and `mild<T>` weak handles with `soft()` / `grab()` /
> `released()`. The tracked record is `TODO.md` § 3 "Ownership Types — Runtime
> Enforcement (DONE)" and the `doc/FEATURE_STATUS.md` ownership rows, which cite the
> executable tests under `test/ownership/`. Design-note caveat: this page is
> **historical** (`doc/DOCS_POLICY.md`) — for current behaviour trust
> `docs/refman/PROGRAMMERS_GUIDE.md` cross-checked against `build/vyb`. The gaps that
> remain (drop semantics along `fail`/`trap` propagation, thread-safety encoding,
> lifetime inference beyond the lexical model) are tracked rows plus roadmap items in
> `TODO.md` § 3, not claims on this page.

## Overview

`mild<T>` is Vyb's fourth ownership type, providing **mild references** to `our<T>` (shared ownership) objects. It solves the circular reference problem and enables patterns like observers, caches, and back-pointers in tree structures.

## The Four Ownership Types

| Type | Ownership | Copying | Nullability | Use Case |
|------|-----------|---------|-------------|----------|
| `my<T>` | Unique | Move only | Never null | Exclusive ownership |
| `our<T>` | Shared (ref-counted) | Clone via `our(x)` | Never null | Multiple owners |
| `their<T>` | Borrowed | Temporary | Never null | Function parameters |
| `mild<T>` | Mild (non-owning) | Clone via `soft(x)` | Can become null | Break cycles, observers |

## Why `mild<T>`?

### Problem: Circular References

```vyb
# Without mild: Memory leak!
struct Node {
    next<our<Node>>,  # Strong reference
    prev<our<Node>>   # Strong reference - CIRCULAR!
}
# Both nodes hold strong references to each other
# Reference counts never reach zero → memory leak
```

### Solution: Mild References

```vyb
# With mild: No leak!
struct Node {
    next<our<Node>>,   # Strong reference (owns)
    prev<mild<Node>>   # Mild reference (doesn't own, breaks cycle)
}
# When next is dropped, prev doesn't prevent cleanup
```

## Key Methods

### `grab() -> our<T>?`

Attempts to upgrade the mild reference to a strong reference. Returns
the native optional `our<T>?` — present (a retained `our<T>`, `strong_count`
incremented) while the target is live, and absent (`?`) once the object has been
released. The failed-upgrade path is therefore type-safe and detectable with the
optional `?` arm (or `else`) without dereferencing a dangling handle.

```vyb
node<our<Node>> = get_node()
shadow<mild<Node>> = soft(node)

# Match on the upgrade result
result<our<Node>?> = shadow.grab()
match (result) {
    strong -> println(strong.value)
    ? -> println("Node no longer exists")
}
```

### `released() -> Bool`

Checks if the referenced object has been destroyed. Returns `true` if the object is dead, `false` if still alive.

```vyb
if (shadow.released()) {
    println("Object was released")
} else {
    println("Object is still alive")
}

# More idiomatically
if (!shadow.released()) {
    # Safe to try grab()
}
```

## Common Patterns

### 1. Tree with Parent Pointers

```vyb
struct TreeNode {
    value<Int>,
    children<Vec<our<TreeNode>>>,   # Own children
    parent<mild<TreeNode>?>         # Weak parent: ABSENT for the root (`mild<TreeNode>?()`)
}

get_parent_value(node<our<TreeNode>>)<Int> -> {
    weak<mild<TreeNode>?> = node.parent
    match (weak) {
        w -> {
            match (w.grab()) {
                parent -> { return parent.value }
                ? -> { }                # the weak reference was already released
            }
        }
        ? -> { }                        # root: no parent at all
    }
    return -1  # Root node or parent destroyed
}
```

### 2. Observer Pattern

```vyb
struct Subject {
    observers<Vec<mild<Observer>>>
}

# `Observer.update()` in the old example was a bind method on the unwrapped
# handle; a plain call keeps the example runnable, and the handle is bound to a
# local before it is unwrapped (also required by `grab()` on a `mild` element).
notify(subject<our<Subject>>) -> {
    shadows<Vec<mild<Observer>>> = subject.observers
    kept<Vec<mild<Observer>>> = Vec()
    i<Int> = 0
    while (i < shadows.len()) {
        shadow<mild<Observer>> = shadows.get(i)
        match (shadow.grab()) {
            observer -> { observer_notify(observer) }   # still alive
            ? -> { }                                    # skip: was destroyed
        }
        if (!shadow.released()) { kept.push(shadow) }   # drop released observers
        i = i + 1
    }
    subject.observers = kept
}

# `for (shadow in subject.observers)` would be the idiomatic walk, but a
# `Vec<mild<T>>` loop variable is not bound today, so the walk indexes instead.
```

### 3. Cache with Expiring Entries

```vyb
struct Cache {
    entries<Vec<mild<Entry>>>
}

# Cache doesn't prevent entries from being freed
# When user drops all strong references to an entry,
# the cache's mild reference becomes invalid

get_valid_entries(cache<our<Cache>>)<Vec<our<Entry>>> -> {
    result<Vec<our<Entry>>> = Vec()
    shadows<Vec<mild<Entry>>> = cache.entries
    i<Int> = 0
    while (i < shadows.len()) {
        shadow<mild<Entry>> = shadows.get(i)
        match (shadow.grab()) {
            entry -> { result.push(entry) }
            ? -> { }
        }
        i = i + 1
    }
    return result
}
```

### 4. Back-References in Doubly-Linked List

```vyb
struct ListNode {
    value<Int>,
    next<our<ListNode>>,       # Owns the next node
    prev<mild<ListNode>?>      # Weak back-reference; ABSENT at the head
}

# Splice `new_node` in after `node`: `next` is an owned field, so a plain
# assignment moves the owned handle.
insert_after(node<our<ListNode>>, new_node<our<ListNode>>) -> {
    new_node.next = node.next
    new_node.prev = soft(node)
    node.next = our(new_node)
}

# NOTE (declaration-level example): a *recursive* `our<T>` field is not
# codegen-clean on this build, so this snippet is not compiled by the doc pass.
# The old spelling of the same model (`next: our<ListNode>?` for the tail) also
# needs an absent `our<T>?`, which cannot be built as a literal; a list is
# expressed with a sentinel node or as a `Vec` of nodes instead.
```

## Implementation Details

### Current Implementation Status

Vyb now has a minimal real runtime model for `our<T>` / `mild<T>`:

- `our(expr)` allocates the payload and a control block.
- `soft(ourValue)` increments `weak_count` and returns a `mild<T>` handle tied
  to the same control block.
- `mild<T>.released()` reads the control block release flag.
- `mild<T>.grab()` increments `strong_count` and returns an `our<T>` handle when
  the payload is live.
- When the last local strong owner in a scope is cleaned up, the payload is
  freed and the control block is marked released while weak handles remain.
- Returning a local `our<T>` or `mild<T>` transfers that local handle to the
  caller instead of cleaning it up before return.
- `our<T>` copy/assignment/parameter semantics are complete: every storage
  location holds its own strong ref, retained on shared copy and released on
  scope exit / overwrite. `my<T>` move semantics also ship: use-after-move is
  rejected at compile time, ownership transfers on assignment / init / `my`
  parameter / move-capture, and a moved binding can be revived by reassignment
  (`test/ownership/`). What remains is not the move checker but the **drop path**:
  destructor behaviour along `fail`/`trap` propagation is unverified (it crosses the
  dual-return `{T, i8*}` ABI) — tracked by the `doc/FEATURE_STATUS.md` row "Drop
  semantics on propagation paths (`fail`/`trap`)" and the matching `TODO.md` item.
- Control-block cleanup covers the current scope/return paths; the propagation-path
  cleanup above is the open item, and thread-safety is not encoded in the type system
  (the "Thread-safety encoding" row) — atomic refcounts exist, but no `Send`/`Sync`
  analogue marks a type as safe to move across threads.

### Control Block Structure

`our<T>` objects maintain a control block with:

- **strong_count**: Number of `our<T>` references
- **weak_count**: Number of `mild<T>` references
- **object**: Pointer to the actual data

### Lifecycle Rules

1. **When `our<T>` strong_count reaches 0:**
   - Object is destroyed
   - Memory is freed
   - Control block is kept if `weak_count > 0`
   - Control block marks object as "released"

2. **When `mild<T>` is destroyed:**
   - `weak_count` is decremented
   - If `weak_count == 0` and object was already freed, control block is freed

3. **When `mild<T>.grab()` is called:**
   - If object is still alive, `strong_count++` and return `our<T>`
   - If object was released, return `nil`

4. **When `mild<T>.released()` is called:**
   - Check control block's "released" flag
   - Return `true` if object destroyed, `false` otherwise

## Comparison with Other Languages

| Language | Mild/Weak Reference Type | Upgrade Method | Check Method |
|----------|---------------------|----------------|--------------|
| **Vyb** | `mild<T>` | `grab() -> our<T>?` | `released() -> Bool` |
| C++ | `std::weak_ptr<T>` | `lock() -> shared_ptr<T>` | `expired() -> bool` |
| Rust | `Weak<T>` | `upgrade() -> Option<Rc<T>>` | `strong_count() == 0` |
| Swift | `weak var` | Automatic upgrade | Check `!= nil` |

## Best Practices

### ✅ Do Use `mild<T>` For:
- Parent pointers in tree structures
- Back-references in linked structures
- Observer/listener patterns
- Caches that don't own entries
- Breaking reference cycles

### ❌ Don't Use `mild<T>` For:
- Function parameters (use `their<T>` instead - faster, simpler)
- Short-lived references (use `their<T>`)
- When you need guaranteed validity (use `our<T>`)

### Performance Considerations
- `mild<T>` has overhead: control block must survive object destruction
- `grab()` requires atomic increment of strong count
- Use `their<T>` for temporary borrows (zero overhead)
- Use `mild<T>` only when you need to detect object destruction

## Example: Complete Tree Implementation

```vyb
struct TreeNode {
    value<Int>,
    children<Vec<our<TreeNode>>>,
    parent<mild<TreeNode>?>        # ABSENT for the root
}

create_root(value<Int>)<our<TreeNode>> -> {
    return our(TreeNode {
        value: value,
        children: Vec(),
        parent: mild<TreeNode>?()  # Root has no parent
    })
}

create_node(value<Int>, parent<our<TreeNode>>)<our<TreeNode>> -> {
    return our(TreeNode {
        value: value,
        children: Vec(),
        parent: soft(parent)       # Weak back-reference to the parent
    })
}

add_child(parent<our<TreeNode>>, value<Int>)<our<TreeNode>> -> {
    child<our<TreeNode>> = create_node(value, parent)
    parent.children.push(child)
    return child
}

get_ancestors(node<our<TreeNode>>)<Vec<Int>> -> {
    ancestors<Vec<Int>> = Vec()
    weak<mild<TreeNode>?> = node.parent
    running<Bool> = true
    while (running) {
        match (weak) {
            w -> {
                match (w.grab()) {
                    parent -> {
                        ancestors.push(parent.value)
                        weak = parent.parent
                    }
                    ? -> { running = false }
                }
            }
            ? -> { running = false }
        }
    }
    return ancestors
}

main()<Int> -> {
    root<our<TreeNode>> = create_root(1)
    child1<our<TreeNode>> = add_child(root, 2)
    child2<our<TreeNode>> = add_child(child1, 3)

    ancestors<Vec<Int>> = get_ancestors(child2)
    # [2, 1]: the parent chain above child2
    return 0
}
```

## Summary

`mild<T>` is Vyb's solution to circular references and the observer pattern. It provides:

- **Mild (non-owning) references** to `our<T>` objects
- **Safe access** via `grab()` that returns `our<T>?`
- **Lifecycle detection** via `released()`
- **Zero cost** when not used (no impact on `my<T>`, `our<T>`, or `their<T>`)

Use `mild<T>` when you need long-lived references that don't prevent cleanup, and use `their<T>` for temporary borrows with guaranteed validity.
