# Persistent Vector Tree

## 1. Definition
The basic dynamic array data structure (std::vector) is in the most cases the most efficient data structure as stated by Stroustrup.

Accept for the fundamental issues, the most important feature of the standard vector is the contiguous memory allocation (i.e. the cache efficiency).
On the other hand, the standard vector has two disadvantages:
1. Copy operation is linear (O(N)): although the contiguous allocation provides some efficiency (e.g. bitwise copy with memcopy for *trivially copyables*)
2. Iterators may become invalid due to the reallocation/shrinking (e.g. push_back)

The basic data structure in functional programming (FP) is the linked list as it addresses solutions to the above two issues.
However, the linked list is cumbersome and and it has a very weak performance as the iteration is based on the pointer indirection.
Hence, in FP, we need a persistent data structure like a linked list which, as well, provides a contiguous memory allocation like a vector.

Ivan Cukic, in his famous book about FP, Functional Programming in C++, describes a data structure 
invented by Rick Hickey for the Clojure language (Bitmapped Vector Trie, BVT).
Rick Hickey has also been inspired by Phil Bagwell's famous paper, Ideal Hash Trees.
Phill Bagwell's trie structure is Hash Array Mapped Trie (HAMT).
Thus, in summary, the data structure in this header, VectorTree.h, is a persistent replacement for the standard vector and 
HAMT is a persistent replacement for the associative data structures (std::unordered_set).

**Note**\
- The VectorTree is not a trie but a tree data structure.
- The VectorTree aggrees with the definitions, invariants, algorithms and runtime/space complexities defined by Ivan Cukic in Functional Programming in C++.

Obviously, all operations listed in the VectorTree interface are const-qualified as its a functionally persistent data structure.
Each operation returns a new VectorTree keeping the persistent history of the data.

## 2. Structure
VectorTree is actually a **tree data structure formed by the composite and the leaf nodes**.
The leaf nodes allocate a small buffer (usually 32 elements) using `std::vector<T>`.
On the other hand, the composite nodes are the source of the shared ownership on the data
simulated by `std::shared_ptr` applied on the children of the composite nodes.
This is how the shared ownership of the data is simulated.

## 3. Performance
A primitive solution to define a persistent vector is a wrapper class for the standard vector which:
1. copies the original std::vector into a new one
2. applies the request on the given element
3. return the modified copy as the new std::vector
The primitive persistent vector would have a crucial performance problem when:
- the size of the vector is large or
- the contained type has a large size.

VectorTree is the solution to the above performance problem.
It distributes the data with the help of a tree structure and perform the requested operations locally.
The unrelated data is shared with the original VectorTree (i.e. the shared ownership defined in [Structure](#2-Structure) section).

For the operations applied at the end (e.g. push_back, emplace_back or pop_back), the efficiency of the VectorTree data structure:
- is superior to the efficiency of the primitive persistent vector and
- converges to the efficiency of the standard vector.\
However, unlike a linked list, the actions applied not at the end are expensive even more expensive than a standard vector.
We have basically two such operations: insert and erase.
For the erase function, VectorTree applies a work-around based on the **swap-and-pop idiom**.
However, this solution **invalidates the pointer to the last element, which is an obvious confliction for a persistent data structure**.
Nevertheles, **the order is not preserved as the last item is swapped with the item to be deleted**.
However, this is neither a standard library (STL) nor a public open source library (e.g. boost).
**Hence, as an in-house data structure, this confliction can be acceptable.**
The insert operation is excluded as VectorTree does not preserve the order of the contained elements.

The member functions and the iterators of VectorTree are tested hardly using gtest library.
See ../test directory for the test results.

VectorTree is tested against std::vector and a primitive persistent vector using google benchmark library.
See ([benchmark.pdf](benchmark.pdf)) in ../build/bin directory for the discussions on the results of the benchmark.

## 4. Approach
The below flowchart is followed for the operations at the end:
1. Find the path from the root node to the active leaf node (i.e. the active path)
2. Copy (not clone!) all the nodes in the active path.
3. Modify the active leaf node for the requested operation together with the copied composite nodes
4. Create a new VectorTree which increments the shared references of the non-modified nodes and replaces the nodes in the active path with the modified nodes.
The above algorithm provides logN time complexity which yields to a constant time as the base of the log is large (32 usually).

The algorithm may require a few modifications if the active leaf node cannot handle the requested operation.
For example, the next leaf node must be determined if the active leaf node is full and the request is a push.
Considering these exceptional cases, the worst case time complexity becomes logkN which again yields to O(1).

VectorTree has its own STL style random access iterators (const and non-const).
The non-const iterator is private and used internally.
An STL style for_each algorithm is implemented as a member function which:
1. clones the whole data structure
2. applies the input function on each element using the internal non-const iterator
3. returns the modified clone as the new VectorTree.

## 5. Concurrency
VectorTree provides an exception and thread safe solution by default as a result of the persistency.

## 6. Requirements
The contained type T must satisfy the following interface:\
```
std::copy_constructible<T>
std::is_move_assignable<T>
```
