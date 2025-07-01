# Persistent Directed Acyclic Graph (DAG)

## 1. Caution
The following two header files are kind of the two versions of the persistent DAG interface:
1. [1st version of the Persistent DAG, PersistentDAG_1.h](PersistentDAG_1.h)
2. [2nd version of the Persistent DAG, PersistentDAG_2.h](PersistentDAG_2.h)

See the documentation of [PersistentDAG_2.h](PersistentDAG_2.h) for the differences between the two.

## 2. Introduction
**Definition**\
Graph is a kind of dynamic linked data structure.
It allows both **many-to-one** and **one-to-many** relationships unlike a tree which only allows one-to-many.
As a result of the many-to-one relation, a graph may have more than one paths between two nodes.
A graph data structure may be directed or not.
The DAG here implements the ancestor and descendant directions **topologically** which yields a **directed graph** data structure.
In this application, the topological difference with the ancestor and descendant relations is not realized by definition (both use `std::vector`)
but **hidden in the algorithms conceptually**.
This is a result of the Data Oriented Design (DOD) approach followed in the definition of the DAG.
I will explain the issue in detail in the following paragraphs.

**The invariant of the DAG requires the absence of any directed cycles.**
A cycle is a path with **more than one node** for which **the beginning and the end nodes are the same**.
This DAG interface allows directed cycles in an *invalid state*.
In other words, when a node is involved in a directed cycle, the **state of the DAG becomes invalid** until the cycle is defeated.
**The algorithms are immune to the cycles and the iterations skip the cycled nodes.**
In summary, as stated above, the DAG allows directed cycles in an invalid state.
So, why not calling Directed Graph (DG) if cycles are allowed.
The reason is to emphasize that the cycles are not desired.
The algorithms are designed to keep the state of the DAG against the directed cycles.
When a user action yields a directed cycle, the DAG terminates the action and inform the user about the cycle formation.
The user intentionally can insist on the action causing the cycle.
Then, the action is executed and the DAG will take care of the cycle waiting for the user to terminate the cycle by modifying the node relations.

**Desing Methodology**\
This DAG interface follows up the concepts coming from:
- **Object-Oriented Design (OOD):** Especially ([PersistentDAG_2.h](PersistentDAG_2.h)) relies on the generically defined types and visitor pattern
- **Function-Oriented Design (FOD):** Use of some types (std::variant), concurency based on the persistency
- **Data-Oriented Design (DOD):** Use of the indices instead of the pointers, memory management (alignment and padding), struct of arrays (SoA)

The above list summarizes the design methodology. The details will be described in the following sections.

**State Management**\
As stated for the directed cycles, this interface defines a state for each node.
The power of this interface is that it covers all the possible states, couples the state data with the node relations and adjusts the algorithms according to the state data.
Followings are the possible states for a node:
1. *uptodate:* the only valid state for a DAG node. a DAG is in a valid state if all the nodes are *uptodate*.
2. *invalid:* **The invalid state is a result of an external definition which is one of the two interfaces of the DAG with the contained type T.**
The conditions causing the invalid state must be defined by T.
In other words, invalid state simulates the invariant of T.
For example, consider the DAG is used by a geometry application which defines Vector type.
The Vector object is defined by 3 components in the space and at least one of them must be non-zero by the Vector invariant.
A user action which makes all the 3 components zero causes the node of the Vector to have invalid state.
3. *ancestor_fail:* Means that one or more of the ancestor nodes of a node is *not uptodate*.
The node causing the ancestor_fail state may not be a direct ancestor of the node as this state is propagated through the whole DAG along the descendant paths.
**This is one of the hidden topological differences between the ancestor and descendant definitions mentioned in the 2nd paragraph.**
4. *cycled:* This is the state of a node involved in a directed cycle and happens if the user insists on the action.
The algorithms are immune to the cycles and the iterations skip the cycled nodes.
The user can terminate a directed cycle by modifying the relations of the nodes.
5. *deleted:* This is another exceptional case like the *cycled* state.
The descendant relations is the source of the memory management where the usual implementation of a pointer-based-DAG defines the ownership.
However, this interface is based on the indices instead of pointers which prevents defining a node with the ownership of the descendant nodes.
Hence, the ownership relation in the descendant direction is not by definition but injected into the algorithms conceptually.
**This is the 2nd hidden topological difference between the ancestor and descendant definitions mentioned in the 2nd paragraph.**
*By definition, the nodes without descendant nodes can be deleted.*
*By definition, the nodes with descendant nodes can be deleted only together with the descendant nodes.*
**However, deleting a node without deleting the descendant nodes is an invalid operation.**
But, the DAG allows this invalid operation by setting the state of the node as *deleted*.
**The algorithms are immune to the deleted nodes and the iterations skip them.**
After the user clears the descendant nodes of the deleted node (by deleting the descendant nodes or by replacing their ancestor nodes) the DAG removes the node safely.

The manipulation of the state of a DAG node requires another property that a node needs to have: *updatability*.
Followings are the possible types for the updatability of a node:
1. *non_updatable:* The node has no updatability issue and always has *uptodate* state.
These nodes, by definition, cannot have ancestor nodes.
2. *self_updatable:* Similar to non_updatable type, the node has no updatability issue and always has *uptodate* state.
These nodes, by definition, does not have ancestor nodes.
For example, a Point in a geometry application is self-updatable as it does not have ancestor nodes and an invariant.
3. *ancestor_updatable:* The state of the node depends on the state of the ancestor nodes.
The state is *uptodate* if all the ancestor nodes are *uptodate*, otherwise, *ancestor_fail*.
The node itself does not have any invariant
4. *invariant_updatable:* The state of the node depends on the state of the ancestor nodes (if exists) and the invariant of the contained type T.

[PersistentDAG_1.h](PersistentDAG_1.h) defines the updatability as an enumeration which is assigned to each node individually.
This is a result of DOD approach.
**However, this definition makes the updatability a runtime issue although in reality its usually a part of the definition.**
Hence, OOD approach looks more reasonable in case of the updatability.
The 2nd interface ([PersistentDAG_2.h](PersistentDAG_2.h)) solves this problem by defining an inner node class templated by the updatability type.
See the documentation of [PersistentDAG_2.h](PersistentDAG_2.h) for the details.

As stated above, the most important feature of this DAG interface is the state data managed for all nodes together with the node relations.
Lets inspect two of the basic algorithms:
- node creation
- node manipulation (i.e. updating the ancestots)

When a new entity is added, the DAG creates a node for the entity.
The current paths are just extended by this action as a new node cannot have descendant nodes.
The DAG inspects the ancestors of the new node if the node is required to have ancestor nodes.
If the ancestors are all *uptodate* the state of the new node is also *uptodate*.
Otherwise, *ancestor_fail*.
If the states of the ancestors are all *uptodate*, the invariant of the contained type T is inspected if T is *invariant_updatable*.
This is the whole operation for the node creation.

On the other hand, in case of an update action, the paths in the current DAG are being modified which requires an inspection following the modified descendant paths.
In the worst case, the length of the inspection approaches to the whole DAG when the modified node approaches to the head node.
A BFS traversal starting from the ancestors of the modified node is the best choice
as the BFS is a level based algorithm which ensures a healthy ancestor state inspection for each node.
**BFS solves the problem in O(N) while for DFS its quadratic**.
DFS requires an additional traversal in the ancestor direction for each node traversed during the iteration in the descendant direction.
Hence, a modification in the paths of the DAG is the most complicated algorithm, O(N).
In most of the DAG implementations, this algorithm is replaced by
a generalized function (*update_DAG_state*) which is executed by a background thread.
This interface relies on this approach where a main thread performs the operations on the DAG
while a background thread keeps inspecting and updating the states of the nodes.
See the [Concurrency](#5-Concurrency) section for the details.

## 3. Data Structures and Memory Management
**All the discussions in this section corresponds to some aspects of DOD approach (e.g. structs of arrays).**

The DAG stores the contained data in a VectorTree which is a fully persistent data structure and relies on the data sharing.
See [persistent vector tree](VectorTree.h) for the details.

The DAG requires internal data structures in order to define the node relations and the states.
Additionally, some auxilary data is needed such as the cycled nodes.
All the internal data is stored in `std::vector` accept for the cycled nodes which is stored in `std::unordered_map`.

`std::vector` is the main container in STL and used widely.
Hence, the properties of `std::vector` is well known.
However, a discussion about the contiguous containers in STL would provide a better understanding of the DAG.
The DAG in the 1st interface ([PersistentDAG_1.h](PersistentDAG_1.h)) stores the data in `std::vector` and does not use `std::array`.
Especially the members defined by `std::vector<std::vector<std::size_t>>` would use the cache more efficiently
if the inner vector could be replaced by a `std::array` (e.g. `std::vector<std::array<std::size_t, 2>>`) as a vector of arrays would allocate a single contiguous memory.
The 2nd interface ([PersistentDAG_2.h](PersistentDAG_2.h)) replaces the use of a single vector:

`std::vector<std::vector<std::size_t>>`

with a number of vectors:

`std::vector<std::array<std::size_t, 1>>`\
`std::vector<std::array<std::size_t, 2>>`\
...\
`std::vector<std::array<std::size_t, k>>`

The value of k is defined statically.

The indexing of the VectorTree and the vectors are all dependent/parallel.
The index dependency is secured by the persistency itself (i.e. *public const* functions).
However, the DAG contains some private mutating functions (e.g. *update_DAG_state*).
All private mutating functions satisfy the strong exception safety.

`std::vector` allocates a contiguous memory which is highly efficient for the two most critical procedures of a persistent DAG structure:
1. copy construction
2. traversal

In order to define the node relations, this DAG interface uses indices instead of the pointers
so that the efficiency of the copy constructor of the DAG is improved.
Because, copying a DAG with pointers would require an algorithm which resets the pointers to point to the new nodes.
Such an algorithm would have a time complexity of O(kN) additional to the copy operation where N is the number of nodes.
Hence, the choice of using indices instead of the pointers provides an enhancement for the copy construction.
The performance of the copy constructor increases more with the trivially copyable objects as `std::vector` guarantees the bitwise copy in such a case.
Hence, `std::vector<std::size_t>` where `std::size_t` is for the node index is the optimum data structure to define the node relations in a DAG.
On the other hand, index based approach creates an additional load on the iteration because the random access comes with an additional pointer indirection.
See [Iterators](#4-Iterators) and [Concurrency](#5-Concurrency) sections for the details.

The relations within the DAG structure does not imply that the objects of the contained type T are also related to each other (e.g. via pointers).
Actually, it implies that T should not define such relations as the DAG already does.
As a result, T is thought to be independent of the relations defined in the DAG.
Hence, the structural sharing can be utilized for the contained type T which minimizes the amount of copied data.
The structural sharing is crucial when T stores large data.

## 4. Iterators
- STL style bidirectional iterator is defined as an inner class.
- const iterator is public and non-const iterator is private for internal usage.
- Bidirectionality is provided by the direction type template parameter (ancestor or descendant).
- Performs looping instead of recursion as the DAG is a large structure which may result with stack overflow.
- Defines all STL aliases (e.g. *iterator_category*, *value_type*, etc.) and the required interface (e.g. increment operator, etc.).
**Thus, the iterator class satisfies the STL rules and can be used safely with the STL algorithms.**
- The iterator stores the visited nodes (i.e. indices) due to the *many-to-one* relations.
The use of indices instead of the pointers creates an additional indirection during the iteration
which increases the runtime of the iteration: locate the beginning of the node vector's data and locate the relative position of the node using random access operator.
Both constant time but now we have two pointer indirections.
However, the index usage allows storing the visited nodes into a `std::vector` instead of a `std::unordered_map`.
`std::vector` guarantees the constant time access while `std::unordered_map` may suffer from the hash collisions.
Hence, the index usage provides highly efficient iteration for the large DAGs.
- The iterators implements both **BFS** and **DFS**.
I will not go into details with the details of BFS and DFS and the use cases for each.
However, the internal functions (e.g. state propogation, state update) mainly uses BFS as it provides a level traversal.
- As specified above, the DAG iterator must store auxilary data (i.e. the visited nodes) due to the many-to-one relationship.
This additional memory allocation may cause the iterator throw `std::bad_alloc` exception.
**Hence, the functions using the DAG iterator shall consider the exception safety.**
The persistency provides strong exception safety for the public functions.
However, the DAG has two private mutating functions to be executed by the background thread: `update_DAG_state()` and `erase_deleted_nodes()`.
In these two functions, the strong exception safety is achieved by storing a backup data that would be modified during the function execution.
The backup data refers to the states of the nodes (copy is cheap) for `update_DAG_state()` and the node relation data (copy is not cheap) for `erase_deleted_nodes()`.
Hence, backup solution is not reasonable for `erase_deleted_nodes()`.
The 2nd interface ([PersistentDAG_2.h](PersistentDAG_2.h)) embeds `erase_deleted_nodes()` into erase function
without increasing the runtime complexity of the erase function significantly.

## 5. Concurrency
As stated above in the [Introduction](#2-Introduction) section, multithreading is required to maintain the state of the nodes by a background thread.
As specified earlier, if the relations of a node is modified by a user action,
the node state data must be inspected and modified (if needed)
by a BFS traversal in the descendant direction starting from the updated node.
The traversal starts from the head node if the DAG contains a directed cycle.
This process is executed by the background thread.

For the concurrent implementation of a data structure we have three approaches:
1. lock-based mutable
2. lock-free mutable
3. persistent immutable

### 1st (lock-based) approach
This approach requires a fine-grained locking scheme based on the nodes.
However, maintaining the fine granularity for a DAG structure is not possible.
This is quite well-known but let me describe the details as this application is a part of my resume.

Lets inspect lock-based doubly linked list data structure before discussing the DAG.
When a node in the list is to be modified, lock-based approach would lock the node together with the nodes on either sides
as all the three nodes are related to each other via pointers and the thread safety requires the safety of the pointers during the mutation.
However, the forward and backward traversals through the doubly linked list would end up with a dead lock.
Consider a linked list with nodes A, B, C and D.
Consider the forward traversal inspects node B and the backward inspects node C.
A dead lock arises if the forward iteration acquires the lock on node B first and the backward acquires the lock on node C first.
This can be solved by assigning a lock order in each direction.

The DAG is also a linked data structure.
Thus, the same discussion applies to the DAG also.
However, we have three problems for the above locking scheme:
1. The ancestor/descendant node definitions are dynamic unlike a linked list:
STL does not have a tool to lock mutexes in a dynamic container.
On the other hand, boost::lock has an overload which supports `std::vector<std::mutex>`.
However, STL has a reason while excluding the dynamic containers:
- compile time locking ensures the RAII, exception safety and the intent of the lock while runtime does not
- the locking process may include too many mutexes which is basically not reasonable and open to practical deadlocks.
2. Dead locks would arise even in a traversal in the same direction:\
Consider we have node N1 with descendant nodes N11, N12, N13, N100 and N200.\
Consider we have node N2 with descendant nodes N21, N22, N23, N100 and N200.\
Lets ignore the ancestor nodes for simplicity.
Consider two threads work on N1 and N2 respectively.
Both threads need to acquire the locks as specified in the doubly linked list example.\
Assume thread1 acquired the locks on N1, N11, N12, N13 and N100.\
Assume thread2 acquired the locks on N2, N21, N22, N23 and N200.\
So thread1 waits for N200 holding N100 while thread2 waits for N100 holding N200.
**This is a deadlock arised in a traversal in the same direction** even we did not consider the ancestor nodes.
**Even worst, its not possible to define a locking order.**
The problem can be solved by switching to coarser locking (lock the whole DAG) or by algorithmically creating a pool of nodes from which the threads request the common node.
The 1st one is not a choice and the 2nd one adds too much overhead especially when the number of nodes gets larger.
3. The number of the nodes to be locked gets very large in a few cycle.
Locking too many mutexes (e.g. tens or hundreds of them) is not reasonable due to many reasons.
In case of a DAG the number of descendant nodes can be any number even thousands.
However, even the number of descendant nodes is not large for individual nodes, the locking scheme ends up locking too many nodes.
Consider again the case in the 2nd item.
When a thread is working on N11, it needs to hold the locks on N1, N11 and the descendant nodes of N11.
When a thread is working on N200, it needs to hold the locks on N1, N11, N12, N13, N100, N200 and the descendant nodes of all.
The lock on N1 can be released only when all of its descendant nodes are examined.
Hence, as the iteration goes on, the number of new locks acquired is far more larger than the number of the released locks.

Another weakness of the lock-based approach is that the data structure must be constrained to have a traversal only in one direction as explained for the doubly linked list.

### 2nd (lock-free) approach
Remember that the 1st and this 2nd approaches do not refer to a persistent/immutable data structure.

As described for the 1st option, there are mainly two data shared by the main and the background threads in case of a mutating DAG:
- node state data
- node relation data

Node state data could be stored in a node class as an atomic type for the thread safety.
However, the same does not apply to the ancestor and descendant nodes as the ancestor and descendant relations are coupled.
Hence, like the lock-based approach, the ancestor and descendant relations must be loaded and stored together by atomic operations.
This is not possible due to the reasons explained in the 1st approach.

On the other hand, the lock-free approach could be mixed with the 3rd option below.
This would be a persistent DAG with an inner node class (e.g. [PersistentDAG_2.h](PersistentDAG_2.h)).
The nodes store the state data via an atomic member.
**Problem with this mixed approach is that the atomic types are neither copyable nor movable.**
The state of each node must be loaded and stored by atomic operations.
This reality cancels out all the efficiency of the copy constructors (expecially the bitwise copy) of the contiguous containers (`std::vector` or `std::array`)
storing the node state data.
Hence, the most important requirement of a persistent data structure (i.e. the efficient copy constructor) is not satisfied.

### 3rd (persistent DAG) approach (FP)
The 3rd approach for the concurrency comes from the Functional Programming (FP).
I will not go through the details about the persistent data structures.
In summary, this option provides a clean solution to the problem if the copy constructor of the DAG can be implemented efficiently.
The 1st interface ([PersistentDAG_1.h](PersistentDAG_1.h)) provides an effective copy constructor following the rules dictated by data-oriented design (DOD) methodology.
The 2nd interface ([PersistentDAG_2.h](PersistentDAG_2.h)) improves the approach further.

Any operation executed on the persistent DAG will follow the following steps:
1. copy the DAG
2. apply the operation on the new DAG

These two steps are necessary and sufficient
- if the DAG is in a valid state and
- if the operation is not a modification

The relations in a DAG can be so complex that a change in the state of a node can affect a distant node.
Each modification on a DAG requires some portion of DAG to be inspected when the state of any node has changed.
Hence, the required steps become:
1. copy the DAG
2. apply the operation on the new DAG
3. (if...) `update_DAG_state()`

The 2nd process can be assumed constant time.
The time complexities for the other two (copy and update) are the same which is O(kN).
The 1st one can be improved by improving the cache usage and the bitwise copy.
However, the 3rd one cannot use the cache effectively as the DAG is not a linear data structure.
The ancestors of a node can be very far from the node which would not be fetched to the cache together with the node.
Hence, the 1st process is more and more faster than the 3rd one which ensures that the multithreaded solution with persistent data structure
is more and more efficient than the single threaded solution as it isolates the long-running 3rd process to a background thread.

Lets inspect the copy constructor in detail, although, these are the basic issues in DOD (as this project is an extension to my resume).
Firstly, a DAG is a link based data structure like a tree.
In traditional OOP, link based data structures are usualy implemented using a node class and defining the node relations via pointers.
Hence, a traditional (single threaded) DAG would have:
1. inner node class with the ancestor nodes defined by a vector of raw pointers and the descendant nodes defined by a vector of shared pointers
2. tail node
3. head node

The copy constructor of the DAG above would copy the head node which will triger the copy constructors of all nodes recursively.
We have the following problems with this copy constructor:
1. too many pointer indirection
2. pointer indirection is a sign of bad cache usage
3. the pointers must be updated after copying the nodes
4. many-to-one relations force us to keep track of the nodes copied (`std::unordered_set`)
5. recursive process would probably cause stack overflow for a large DAG.

A solution to the 1st issue is storing the nodes in the DAG in a contiguous container (`std::vector<node>`)
and replacing the shared pointers of descendant container with raw pointers or weak pointers.
Now all issues accept for the 3rd one is solved as the ancestor and descendant nodes still point to the nodes of the old DAG.
The solution is again provided by the contiguous memory allocation of `std::vector`.
We can determine the location of each node relative to the beginning of the original vector's data
and relocate the new pointer using the location relative to the copied vector's data.
Consider we store the nodes in the DAG like: `std::vector<node> _nodes`;
1. copy _nodes vector\
`_nodes = rhs._nodes`
2. loop through _nodes\
`for (auto& new_node : _nodes)`
3. loop through the ancestor nodes of the node\
`for (node* ancestor_node : new_node._ancestor_nodes);`
4. relocate the ancestor pointer\
`auto new_ancestor_node = _nodes.data() + (ancestor_node - rhs._nodes.data());`

Steps 3 and 4 must be repeated for the descendant nodes.
The runtime complexity for this copy constructor is *2O(k1k2N)* where k1 and k2 are the average number of ancestor and descendant nodes for a node respectively.
Constant 2 comes from the fact that both the vector copy constructor and the pointer correction has the same complexity.
However, the time complexity is misleading as the vector's copy constructor would benefit the cache optimization.

The key point in the above solution is using the relative adresses pointed by the pointers which is provided by `std::vector` contiguous allocation.
Hence, a further optimization would be achieved by using the relative addresses (i.e. indices) directly in the definition of the node relations instead of the pointers.
**We achieved one of the fundamental rules of DOD that suggests replacing the pointers with indices.**
This will cancel out the need for the 3rd process and the copy constructor of the DAG would process only the vector's copy constructor which is very cheap.
Additionally, now, the vector's copy constructor performs bitwise copy with `std::memcpy` as `std::size_t` is trivially copyable which increases the performance further.
My tests show that the vector's copy constructor is around 5 times faster with `std::size_t` comparing to `std::vector<node>` with a simple non-trivially copyable node definition.

Obviously, the problems of working with indices are the *erase at the middle* and *insert at the middle* functions.
The insert function is not needed as the DAG is a link-based data structure and so it does not need to preserve the order of the nodes while storing them in `std::vector`.
Hence, insert member function can be treated as a push-back.
With the index-based approach, the time complexity of the erase member function is O(k1k2N) as all the indices after the erased index must be decremented.
The solution is replacing the erase member function by **swap-and-pop idiom** which swaps the last element with the element to be erased and pop the container.
Now, the element is erased by changing the index for only the last element which is O(1).
The node relations including the last index can easily be updated.
However, there is one more work to do that the contained type T belonging to the last element must be informed.
This is the **2nd interface of the DAG with the contained type T:**\
`void set_DAG_node(std::size_t index)`

Defining the node relations via indices has an enhancement on the iteration for large DAGs.
See [Iterators](#4-Iterators) section for the details.

On the other hand, the persistent DAG approach comes with a problem: **shared data**.
The node state data is shared between the main and the background threads
because the main thread copies the node state data together with the other members of the DAG
while the background thread modifies it.
This problem is solved by securing the node state data by a mutex.
The pseudocode for the main thread is as follows:
1. copy all members of the current DAG accept for the node state data
2. acquire the lock on the mutex of the node state data
3. copy the node state data (`std::vector<unsigned char>`)
4. release the lock
5. perform the requested operation on the copy of the DAG
6. return the modified copy of the DAG

The pseudocode for the background thread is as follows:
1. acquire the lock on the mutex of the node state data
2. create a copy of the node state data
3. release the lock
4. execute `update_DAG_state()` which updates the copy of the node state data
5. acquire the lock on the mutex again
6. copy the modified copy of the state data onto the original state data
7. release the lock
8. update the state of the DAG according to the final node state data.

Hence, the **critical section** for the mutex is the node state data copy process which is guaranteed to be very fast as its a bitwise copy process.
Thus, in this approach, the runtime of the main thread is very short which results with a very fast application from the user's point of view.

Besides, one of the advantages of the persistency is that it stores the history of the data structure so that there is no need to implement the command design pattern.

In summary, the DAG is implemented using the 3rd approach by injecting the lock-based approach partially into a persistent definition.

*Note: The state of the DAG is an `std::atomic<bool>` which secures the DAG state without a higher level locking mechanism.*

## 6. Requirements
1. The contained type T must satisfy the following interface:
```
std::copy_constructible<T>
std::is_move_assignable<T>
void set_DAG_node(std::size_t) // PersistentDAG_1.h
void set_DAG_node(_a_node_var_raw) // PersistentDAG_2.h
bool inspect_invariant() // case dependent: invariant_updatable
```
2. T objects should not have pointers to each other.
As a persistent data structure, the DAG relies on the structural sharing for the contained type T which performs copy construction on the objects of T.
Hence, if T objects have pointers to other T objects, the structural sharing will just copy the pointers.
Therefore, reconsider using this interface in such a case.
The issue is explained in [Data Structures and Memory Management](#3-Data-Structures-and-Memory-Management) section.

## 7. Assumptions and Limitations
The only aim of the this DAG interface is to store the data (T) in a directed graph data structure while keeping the state of the data uptodate.
Hence, many fundamental operations of a DAG are excluded.
Two important parameters are missing in this DAG interface:
1. edges
2. node weight factors

Edges are simulated in node relation data (i.e. ancestor nodes and descendant nodes).
Node weight factors are not defined which means that all nodes are treated equally.
Therefore, this is not a generalized DAG data structure and many graph algorithms (e.g. shortest path) are missing.
