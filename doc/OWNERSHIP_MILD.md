# Vyn Ownership Types: `mild<T>`

## Overview

`mild<T>` is Vyn's fourth ownership type, providing **weak references** to `our<T>` (shared ownership) objects. It solves the circular reference problem and enables patterns like observers, caches, and back-pointers in tree structures.

## The Four Ownership Types

| Type | Ownership | Copying | Nullability | Use Case |
|------|-----------|---------|-------------|----------|
| `my<T>` | Unique | Move only | Never null | Exclusive ownership |
| `our<T>` | Shared (ref-counted) | Clone via `our(x)` | Never null | Multiple owners |
| `their<T>` | Borrowed | Temporary | Never null | Function parameters |
| `mild<T>` | Weak (non-owning) | Clone via `mild(x)` | Can become null | Break cycles, observers |

## Why `mild<T>`?

### Problem: Circular References

```vyn
# Without mild: Memory leak!
struct Node {
    next: our<Node>,  # Strong reference
    prev: our<Node>   # Strong reference - CIRCULAR!
}
# Both nodes hold strong references to each other
# Reference counts never reach zero → memory leak
```

### Solution: Weak References

```vyn
# With mild: No leak!
struct Node {
    next: our<Node>,   # Strong reference (owns)
    prev: mild<Node>   # Weak reference (doesn't own, breaks cycle)
}
# When next is dropped, prev doesn't prevent cleanup
```

## Key Methods

### `grab() -> our<T>?`

Attempts to upgrade the weak reference to a strong reference. Returns `our<T>` if the target is still alive, or `nil` if it has been released.

```vyn
node<our<Node>> = get_node()
weak_ref<mild<Node>> = mild(node)

# Try to access the object
if (strong<our<Node>> = weak_ref.grab()) {
    # Success! Object is still alive
    println(strong.value)
} else {
    # Object was destroyed
    println("Node no longer exists")
}
```

### `released() -> Bool`

Checks if the referenced object has been destroyed. Returns `true` if the object is dead, `false` if still alive.

```vyn
if (weak_ref.released()) {
    println("Object was released")
} else {
    println("Object is still alive")
}

# More idiomatically
if (!weak_ref.released()) {
    # Safe to try grab()
}
```

## Common Patterns

### 1. Tree with Parent Pointers

```vyn
struct TreeNode {
    value: Int,
    children: Vec<our<TreeNode>>,  # Own children
    parent: mild<TreeNode>         # Don't own parent
}

fn get_parent_value(node: our<TreeNode>) -> Int {
    if let parent = node.parent.grab() {
        return parent.value
    }
    return -1  # Root node or parent destroyed
}
```

### 2. Observer Pattern

```vyn
struct Subject {
    observers: Vec<mild<Observer>>
}

fn notify(subject: our<Subject>) -> Void {
    for (weak_observer in subject.observers) {
        if let observer = weak_observer.grab() {
            observer.update()  # Still alive
        }
        # Otherwise skip - observer was destroyed
    }
    
    # Optional: Clean up released observers
    subject.observers = subject.observers.filter(|o| !o.released())
}
```

### 3. Cache with Expiring Entries

```vyn
struct Cache {
    entries: Vec<mild<Entry>>
}

# Cache doesn't prevent entries from being freed
# When user drops all strong references to an entry,
# the cache's weak reference becomes invalid

fn get_valid_entries(cache: our<Cache>) -> Vec<our<Entry>> {
    result: Vec<our<Entry>> = Vec()
    for (weak_entry in cache.entries) {
        if let entry = weak_entry.grab() {
            result.push(entry)
        }
    }
    return result
}
```

### 4. Back-References in Doubly-Linked List

```vyn
struct ListNode {
    value: Int,
    next: our<ListNode>?,    # Strong reference (owns next)
    prev: mild<ListNode>     # Weak reference (doesn't own prev)
}

fn insert_after(node: our<ListNode>, new_node: our<ListNode>) -> Void {
    new_node.next = node.next
    new_node.prev = mild(node)
    node.next = our(new_node)
}
```

## Implementation Details

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

| Language | Weak Reference Type | Upgrade Method | Check Method |
|----------|---------------------|----------------|--------------|
| **Vyn** | `mild<T>` | `grab() -> our<T>?` | `released() -> Bool` |
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

```vyn
struct TreeNode {
    value: Int,
    children: Vec<our<TreeNode>>,
    parent: mild<TreeNode>
}

fn create_node(value: Int, parent: mild<TreeNode>) -> our<TreeNode> {
    return our(TreeNode {
        value: value,
        children: Vec(),
        parent: parent
    })
}

fn add_child(parent: our<TreeNode>, value: Int) -> our<TreeNode> {
    child: our<TreeNode> = create_node(value, mild(parent))
    parent.children.push(child)
    return child
}

fn get_ancestors(node: our<TreeNode>) -> Vec<Int> {
    ancestors: Vec<Int> = Vec()
    current: mild<TreeNode> = node.parent
    
    while (!current.released()) {
        if let parent = current.grab() {
            ancestors.push(parent.value)
            current = parent.parent
        } else {
            break
        }
    }
    
    return ancestors
}

fn main() -> Int {
    root: our<TreeNode> = create_node(1, mild<TreeNode>())  # Root has no parent
    child1: our<TreeNode> = add_child(root, 2)
    child2: our<TreeNode> = add_child(child1, 3)
    
    ancestors: Vec<Int> = get_ancestors(child2)
    # Should return [2, 1]
    
    return 0
}
```

## Summary

`mild<T>` is Vyn's solution to circular references and the observer pattern. It provides:
- **Non-owning references** to `our<T>` objects
- **Safe access** via `grab()` that returns `our<T>?`
- **Lifecycle detection** via `released()`
- **Zero cost** when not used (no impact on `my<T>`, `our<T>`, or `their<T>`)

Use `mild<T>` when you need long-lived references that don't prevent cleanup, and use `their<T>` for temporary borrows with guaranteed validity.
