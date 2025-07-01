/*!
 * VectorTree interface
 * 
 * the basic dynamic array data structure (std::vector)
 * is in most cases the most efficient data structure
 * as stated by Stroustrup.
 * 
 * additional to the fundamental issues,
 * the most important feature of the standard vector is
 * the contiguous memory allocation
 * which enhances the cache efficiency of algorithms.
 * the power of the standard vector by the contiguous memory allocation
 * is a very important requirement
 * especially in performance critical applications.
 * 
 * on the other hand, the standard vector has two disadvantages:
 *     1. Copy operation is linear (O(N))
 *        although the contiguous allocation provides some efficiency
 *        (e.g. bitwise copy with memcopy if the contained element is trivially copyable)
 *     2. Iterators may become invalid due to the reallocation/shrinking (e.g. push_back)
 * 
 * the basic data structure in functional programming (FP)
 * is the linked list as it addresses solutions to the above two issues.
 * however, the linked list is cumbersome and
 * and it has a very weak performance
 * as the iteration is based on the pointer indirection.
 * 
 * hence, in FP, we need a persistent data structure like a linked list
 * which, as well, provides a contiguous memory allocation like a vector.
 * 
 * Ivan Cukic, in his famous book about FP (Functional Programming in C++)
 * describes Bitmapped Vector Trie (BVT) like data structure
 * invented by Rick Hickey for the Clojure language.
 * Rick Hickey has also been inspired by Phil Bagwell's famous paper, Ideal Hash Trees.
 * Phill Bagwell's trie structure is Hash Array Mapped Trie (HAMT).
 * thus, in summary, the data structure in this header, VectorTree.h,
 * is a persistent replacement for the standard vector and
 * HAMT is a persistent replacement for
 * the associative data structures (std::unordered_set).
 * 
 * note that a VectorTree is not a trie but a tree.
 * 
 * the VectorTree implemented in this header aggrees with
 * the definitions, invariants, algorithms and runtime/space complexities
 * defined by Ivan Cukic in Functional Programming in C++.
 * 
 * the implementation splits the vector
 * into small buffers (usually 32 elements)
 * which are allocated in a tree structure.
 * the buffers are allocated in the leaf nodes.
 * the composite nodes shares the ownership of the leafs.
 * 
 * the efficiency of the VectorTree data structure
 * converges to the efficiency of the standard vector
 * for the operations applied at the end
 * (e.g. push_back, emplace_back or pop_back).
 * however, unlike a linked list,
 * the actions applied not at the end are expensive
 * even more expensive than a standard vector.
 * 
 * in this interface,
 * a work-around is applied for the erase function
 * based on the swap and pop idiom.
 * however, this solution invalidates
 * the pointer to the last element,
 * which is an obvious confliction for a persistent data structure.
 * additionally, the order is not preserved as the last item is
 * swapped with the item to be deleted.
 * however, this is neither a standard library (STL) nor
 * a public open source library (e.g. boost).
 * hence, as an in-house data structure,
 * this confliction can be acceptable.
 * 
 * the following approach is followed for the operations at the end:
 *     1. find the path from the root node to
 *        the active leaf node (i.e. the active path)
 *     2. copy (not clone!) all the nodes in the active path.
 *        copying the nodes means
 *        incrementing the shared reference counts on the nodes.
 *        cloning the nodes means duplicating the nodes.
 *        only VectorTree copy constructor performs clone on the nodes.
 *     3. modify the active leaf node for the requested operation
 *        together with the copied composite nodes
 *     4. create a new VectorTree
 *        which increments the shared references for the non-modified nodes
 *        and replaces the nodes in the active path with the modified nodes.
 * 
 * the above algorithm provides logN time complexity
 * which yields to a constant time as the base of the log is large (32 usually).
 * 
 * the algorithm may require a few modifications
 * if the active leaf node cannot handle the requested operation.
 * for example, the next leaf node must be determined
 * if the active leaf node is full and the request is a push.
 * considering these exceptional cases,
 * the worst case time complexity becomes logkN
 * which again yields to O(1).
 * 
 * obviously, all operations listed in the VectorTree interface are const-qualified
 * as its a functionally persistent data structure.
 * each operation returns a new VectorTree
 * keeping the persistent history of the data.
 * 
 * VectorTree has its own STL style random access iterators (const and non-const).
 * the non-const iterator is private and used internally.
 * STL style for_each algorithm is implemented as a member function
 * which clones (not copy!) the whole data structure and
 * applies the input function on each element
 * using the internal non-const iterator.
 * 
 * 
 * 
 * The member functions and the iterators of VectorTree are tested hardly
 * using gtest library.
 * See ../test directory for the test results.
 * 
 * VectorTree is tested against std::vector and a primitive persistent vector
 * using google benchmark library.
 * see ../benchmark directory for the discussions on the results of the benchmark.
 * 
 * 
 * 
 * @author    <baris.albayrak.ieee@gmail.com>
 * @version   0.0.1
 * @see       the README file of the github repository below for an overview of this project.
 * 
 * github:    <https://github.com/BarisAlbayrakIEEE/VectorTree.git>
 */

#ifndef _VectorTree_HeaderFile
#define _VectorTree_HeaderFile

#include <string>
#include <vector>
#include <memory>
#include <variant>
#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <iterator>
#include <cmath>
#include <algorithm>
#include <assert.h>

namespace VectorTreeNamespace {
	using _a_path = std::vector<unsigned char>;

	static const unsigned char DEFAULT_BUFFER = 32;
	static const unsigned char MAX_VectorTree_HEIGHT = 8;

	/*!
	 * @brief VectorTree class
	 * 
	 * the composite and leaf nodes and the iterators are implemented as inner classes.
	 * A static typing (templated class hyerarchy) is utilized
	 * together with std::variant
	 * instead of the dynamic polymorphism.
	 * 
	 * the iterators are almost the same as std::vector::iterator
	 * with a few modifications to keep track of the leaf buffers.
	 * 
	 * @see the main documentation of this header file for the details
	 */
	template <typename T, unsigned char BufferSize = DEFAULT_BUFFER, class Allocator = std::allocator<T>>
	class VectorTree {
		using _a_VT = VectorTree<T, BufferSize, Allocator>;

		/*!
		 * @brief Implementation for the leaf nodes
		 * 
		 * VectorTree is a tree structure formed by composite and leaf nodes.
		 * a leaf node stores the data in a fixed size buffer (BufferSize).
		 */
		class impl_VectorTree_node_leaf {
			friend class VectorTree<T, BufferSize, Allocator>;
			friend class impl_VectorTree_node_composite;

			// local aliases
			using _a_leaf = impl_VectorTree_node_leaf;
			using _a_childs = std::vector<T, Allocator>;

			// members
			_a_childs _childs{};

		public:

			/*!
			 * @brief default constructor
			 * 
			 * reserves the size of the children vector
			 * in order to prevent std::vector reallocation
			 * for the safety of the pointers to the elements contained
			 */
			impl_VectorTree_node_leaf() {
				_childs.reserve(BufferSize);
			};

			/*!
			 * @brief the big 5
			 * apply copy-and-swap idiom for the assignment operator
			 */
			impl_VectorTree_node_leaf(const _a_leaf& rhs) = default;
			_a_leaf& operator=(_a_leaf rhs) {
				swap(*this, rhs);
				return *this;
			}
			impl_VectorTree_node_leaf(_a_leaf&& rhs) noexcept = default;
			_a_leaf& operator=(_a_leaf&& rhs) noexcept = default;
			~impl_VectorTree_node_leaf() = default;

			/*!
			 * @brief friend swap function
			 */
			friend inline void swap(_a_leaf& lhs, _a_leaf& rhs) noexcept {
				using std::swap;
				swap(lhs._childs, rhs._childs);
			};
		};

		/*!
		 * @brief Implementation for the composite nodes
		 * 
		 * VectorTree is a tree structure formed by composite and leaf nodes
		 * composite nodes may store other composites or leafs.
		 * 
		 * persistent VectorTree is based on the shared data
		 * in order to minimize the amount of the data to be copied.
		 * hence, the composite nodes have shared ownership on the children nodes.
		 */
		class impl_VectorTree_node_composite {
			friend class VectorTree<T, BufferSize, Allocator>;

			// local aliases
			using _a_composite = impl_VectorTree_node_composite;
			using _a_leaf = impl_VectorTree_node_leaf;
			using _a_node_sh_var = std::variant<std::shared_ptr<_a_composite>, std::shared_ptr<_a_leaf>>;
			using _a_childs = std::vector<_a_node_sh_var>;

			// members
			_a_childs _childs{};

			/*!
			 * @brief clone the node performing a deep copy
			 * instead of incrementing the reference counts
			 * 
			 * persistent VectorTree is based on the shared data
			 * in order to minimize the amount of the data to be copied.
			 * hence, the copy constructor does not perform a deep copy
			 * but increments the shared reference count.
			 * however, VectorTree copy constructor requires a deep copy
			 * 
			 * @throws clones composites recursively
			 * which may result with stack overflow or bad_alloc exceptions
			 * 
			 * @exceptsafe strong exception safety as non-mutating
			 */
			[[nodiscard]] auto clone() const -> std::shared_ptr<_a_composite>
			{
				_a_childs childs;
				if (!_childs.empty()) {
					if (std::holds_alternative<std::shared_ptr<_a_composite>>(_childs[0])) {
						for (auto& child : _childs) {
							auto composite_sh{ std::get<std::shared_ptr<_a_composite>>(child) };
							childs.emplace_back(composite_sh->clone());
						}
					}
					else {
						for (auto& child : _childs) {
							auto leaf_sh{ std::get<std::shared_ptr<_a_leaf>>(child) };
							childs.emplace_back(std::make_shared<_a_leaf>(*leaf_sh));
						}
					}
				}
				return std::make_shared<_a_composite>(childs);
			};

		public:

			impl_VectorTree_node_composite() { _childs.reserve(BufferSize); };
			explicit impl_VectorTree_node_composite(const _a_childs& childs) : _childs{childs} {};
			explicit impl_VectorTree_node_composite(unsigned char height) {
				assert(height > 0);

				_childs.reserve(BufferSize);
				if (height == 1) {
					for (auto i = 0; i < BufferSize; ++i) {
						_childs.emplace_back(std::make_shared<_a_leaf>());
					}
				}
				else {
					for (auto i = 0; i < BufferSize; ++i) {
						_childs.emplace_back(std::make_shared<_a_composite>(height - 1));
					}
				}
			};
			explicit impl_VectorTree_node_composite(
				unsigned char height,
				const std::shared_ptr<_a_composite>& current_root_node)
			{
				assert(height > 1);

				_childs.emplace_back(current_root_node);
				for (auto i = 1; i < BufferSize; ++i) {
					_childs.emplace_back(std::make_shared<_a_composite>(height - 1));
				}
			};

			/*!
			 * @brief the big 5
			 * apply copy-and-swap idiom for the assignment operator
			 */
			impl_VectorTree_node_composite(const _a_composite& rhs) = default;
			_a_composite& operator=(_a_composite rhs) {
				swap(*this, rhs);
				return *this;
			}
			impl_VectorTree_node_composite(_a_composite&& rhs) noexcept = default;
			_a_composite& operator=(_a_composite&& rhs) noexcept = default;
			~impl_VectorTree_node_composite() = default;

			/*!
			 * @brief friend swap function
			 */
			friend inline void swap(_a_composite& lhs, _a_composite& rhs) noexcept {
				using std::swap;
				swap(lhs._childs, rhs._childs);
			};
		};

	public:

		/*!
		 * @brief the STL style const iterator class
		 * stores two additional data
		 * comparing with std::vector::const_iterator:
		 *     _leaf_node: current active leaf node:
		 *                 in order to increase iteration performance
		 *     _index: the index of the current element of the container:
		 *                 in order to realize the end iterator
		 */
		template <typename VectorTreeType>
		class VectorTree_const_iterator {
			friend class VectorTree<T, BufferSize, Allocator>;
			
		public:

			// STL aliases
			using iterator_category = std::random_access_iterator_tag;
			using value_type = typename VectorTreeType::value_type;
			using difference_type = typename VectorTreeType::difference_type;
			using pointer = typename VectorTreeType::const_pointer;
			using reference = const value_type&;

			// local aliases
			using _a_container = VectorTreeType const*;
			using _a_leaf = impl_VectorTree_node_leaf const*;
			using _a_childs_it = std::vector<T, Allocator>::const_iterator;

		private:

			// members
			_a_container _container{};
			_a_path _path_to_leaf_node__current{};
			_a_childs_it _leaf_node__current__it{};
			unsigned char _leaf_node__current__counter{};
			std::size_t _index{};

		public:

			/*!
			* @brief ctor with the input VectorTree
			*/
			explicit VectorTree_const_iterator(const VectorTreeType& container) : _container(&container) {
				if (_container->empty()) return;

				_path_to_leaf_node__current = _a_path(container._height, 0);
				auto leaf_node__current{ container.get_leaf_node(_path_to_leaf_node__current) };
				_leaf_node__current__it = leaf_node__current->_childs.cbegin();
			};

			/*!
			* @brief ctor with all inputs
			*/
			VectorTree_const_iterator(
				const VectorTreeType& container,
				_a_path path_to_leaf_node__current,
				_a_childs_it leaf_node__current__it,
				unsigned char leaf_node__current__counter,
				std::size_t index) noexcept
				:
				_container{ &container }
			{
				if (_container->empty()) return;

				_path_to_leaf_node__current = path_to_leaf_node__current;
				_leaf_node__current__it = leaf_node__current__it;
				_leaf_node__current__counter = leaf_node__current__counter;
				_index = index;
			};

			[[nodiscard]] inline reference operator*() const noexcept {
				if (_index == _container->size()) {
					throw std::logic_error("Cannot dereference the end iterator.");
				}
				return *_leaf_node__current__it;
			};

			[[nodiscard]] inline pointer operator->() const noexcept {
				if (_index == _container->size()) {
					throw std::logic_error("Cannot dereference the end iterator.");
				}
				return &(*_leaf_node__current__it);
			};

			inline VectorTree_const_iterator& operator++() noexcept {
				// increment
				++_index;
				++_leaf_node__current__it;
				++_leaf_node__current__counter;

				// inspect if the end of the container is reached
				if (_index == _container->size()) { return *this; }

				// inspect if the end of the current leaf node buffer was already reached
				if (_leaf_node__current__counter == BufferSize) {
					// step to the next leaf node
					_path_to_leaf_node__current = _container->get_path_to_leaf_node__next(_path_to_leaf_node__current);

					// set the leaf node iterator at the beginning of the next leaf node buffer
					auto leaf_node__current{ _container->get_leaf_node(_path_to_leaf_node__current) };
					_leaf_node__current__it = leaf_node__current->_childs.cbegin();

					// reset the leaf node counter
					_leaf_node__current__counter = 0;
				}
				return *this;
			};

			[[nodiscard]] inline VectorTree_const_iterator operator++(int) noexcept {
				VectorTree_const_iterator iterator{ *this };
				++*this;
				return iterator;
			};

			[[nodiscard]] inline VectorTree_const_iterator operator+(const difference_type offset) noexcept {
				VectorTree_const_iterator iterator{ *this };
				iterator += offset;
				return iterator;
			};

			VectorTree_const_iterator& operator+=(const difference_type offset) noexcept {
				// inspect if the end of the container is passed
				auto new_index{ _index + offset };
				if (new_index > _container->size()) {
					throw std::logic_error("The offset passes the end of the container.");
				}

				// offset the index
				_index = new_index;

				// inspect if the end of the container is reached
				if (_index == _container->size()) {
					_path_to_leaf_node__current = _container->_path_to_leaf_node__current;
					auto leaf_node__current{ _container->get_leaf_node(_path_to_leaf_node__current) };
					_leaf_node__current__it = leaf_node__current->_childs.cend();
					return *this;
				}

				// get path to the current leaf node
				auto path_to_element{ _container->get_path_to_element(_index) };
				_path_to_leaf_node__current = _a_path(_container->_height, 0);
				std::copy(
					path_to_element.begin(),
					path_to_element.begin() + path_to_element.size() - 1,
					_path_to_leaf_node__current.begin()
				);

				// set the leaf node iterator
				auto leaf_node__current{ _container->get_leaf_node(_path_to_leaf_node__current) };
				_leaf_node__current__it = leaf_node__current->_childs.cbegin() + path_to_element.back();

				// set the leaf node counter
				_leaf_node__current__counter += offset % BufferSize;
				_leaf_node__current__counter %= BufferSize + 1;
				return *this;
			};

			inline VectorTree_const_iterator& operator--() noexcept {
				// decrement the index
				--_index;
				--_leaf_node__current__it;
				--_leaf_node__current__counter;

				// inspect if the beginning of the container is reached
				if (!_index) { return *this; }

				// inspect if the beginning of the current leaf node buffer was already reached
				if (!_leaf_node__current__counter) {
					// step to the previous leaf node
					_path_to_leaf_node__current = _container->get_path_to_leaf_node__previous(_path_to_leaf_node__current);

					// set the leaf node iterator at the last element of the previous leaf node buffer
					auto leaf_node__current{ _container->get_leaf_node(_path_to_leaf_node__current) };
					_leaf_node__current__it = leaf_node__current->_childs.cbegin() + leaf_node__current->_childs.size() - 1;

					// reset the leaf node counter
					_leaf_node__current__counter = BufferSize;
				}
				return *this;
			};

			[[nodiscard]] inline VectorTree_const_iterator operator--(int) noexcept {
				VectorTree_const_iterator iterator{ *this };
				--*this;
				return iterator;
			};

			[[nodiscard]] inline VectorTree_const_iterator operator-(const difference_type offset) noexcept {
				VectorTree_const_iterator iterator{ *this };
				iterator -= offset;
				return iterator;
			};

			inline VectorTree_const_iterator& operator-=(const difference_type offset) noexcept {
				// inspect if the beginning of the container is passed
				int new_index{ _index - offset };
				if (new_index < 0) {
					throw std::logic_error("The offset passes the beginning of the container.");
				}

				// offset the index
				_index = new_index;

				// get path to the current leaf node
				auto path_to_element{ _container->get_path_to_element(_index) };
				_path_to_leaf_node__current = _a_path(_container->_height, 0);
				std::copy(
					path_to_element.begin(),
					path_to_element.begin() + path_to_element.size() - 1,
					_path_to_leaf_node__current.begin()
				);

				// set the leaf node iterator
				auto leaf_node__current{ _container->get_leaf_node(_path_to_leaf_node__current) };
				_leaf_node__current__it = leaf_node__current->_childs.cbegin() + path_to_element.back();

				// set the leaf node counter
				auto step{ offset % BufferSize };
				if (_leaf_node__current__counter >= step) _leaf_node__current__counter -= step;
				else _leaf_node__current__counter += BufferSize - step + 1;
				return *this;
			};

			[[nodiscard]] inline reference operator[](const difference_type offset) const noexcept {
				return *(*this + offset);
			};

			[[nodiscard]] inline bool operator==(const VectorTree_const_iterator& rhs) const noexcept {
				return _container == rhs._container && _index == rhs._index;
			};

			[[nodiscard]] inline bool operator!=(const VectorTree_const_iterator& rhs) const noexcept {
				return !(*this == rhs);
			};

			[[nodiscard]] inline bool operator<(const VectorTree_const_iterator& rhs) const noexcept {
				return _index < rhs._index;
			};

			[[nodiscard]] inline bool operator<=(const VectorTree_const_iterator& rhs) const noexcept {
				return _index <= rhs._index;
			};

			[[nodiscard]] inline bool operator>(const VectorTree_const_iterator& rhs) const noexcept {
				return _index > rhs._index;
			};

			[[nodiscard]] inline bool operator>=(const VectorTree_const_iterator& rhs) const noexcept {
				return _index >= rhs._index;
			};
		};

	private:

		/*!
		 * @brief the STL style iterator class
		 * follows std::vector::iterator approach which bases std::vector::const_iterator
		 */
		template <typename VectorTreeType>
		class VectorTree_iterator : public VectorTree_const_iterator<VectorTreeType> {

		public:

			// STL aliases
			using iterator_category = std::random_access_iterator_tag;
			using value_type = typename VectorTreeType::value_type;
			using difference_type = typename VectorTreeType::difference_type;
			using pointer = typename VectorTreeType::pointer;
			using reference = value_type&;

			// local aliases
			using _a_base = VectorTree_const_iterator<VectorTreeType>;
			using _a_container = VectorTreeType*;
			using _a_base::_a_base;

			[[nodiscard]] inline reference operator*() const noexcept {
				return const_cast<reference>(_a_base::operator*());
			}

			[[nodiscard]] inline pointer operator->() const noexcept {
				return const_cast<pointer>(_a_base::operator->());
			};

			inline VectorTree_iterator& operator++() noexcept {
				_a_base::operator++();
				return *this;
			};

			[[nodiscard]] inline VectorTree_iterator operator++(int) noexcept {
				VectorTree_iterator iterator = *this;
				_a_base::operator++();
				return iterator;
			};

			inline VectorTree_iterator& operator--() noexcept {
				_a_base::operator--();
				return *this;
			};

			[[nodiscard]] inline VectorTree_iterator operator--(int) noexcept {
				VectorTree_iterator iterator = *this;
				_a_base::operator--();
				return iterator;
			};

			inline VectorTree_iterator& operator+=(const difference_type offset) noexcept {
				_a_base::operator+=(offset);
				return *this;
			};

			[[nodiscard]] inline VectorTree_iterator operator+(const difference_type offset) const noexcept {
				VectorTree_iterator iterator = *this;
				iterator += offset;
				return iterator;
			};

			[[nodiscard]] friend inline VectorTree_iterator operator+(const difference_type offset, VectorTree_iterator rhs) noexcept {
				rhs += offset;
				return rhs;
			};

			inline VectorTree_iterator& operator-=(const difference_type offset) noexcept {
				_a_base::operator-=(offset);
				return *this;
			};

			using _a_base::operator-;

			[[nodiscard]] inline VectorTree_iterator operator-(const difference_type offset) const noexcept {
				VectorTree_iterator iterator = *this;
				iterator -= offset;
				return iterator;
			};

			[[nodiscard]] inline reference operator[](const difference_type offset) const noexcept {
				return *(*this + offset);
			};

			[[nodiscard]] inline bool operator==(const VectorTree_iterator& rhs) const noexcept {
				return this->_container == rhs._container && this->_index == rhs._index;
			};

			[[nodiscard]] inline bool operator!=(const VectorTree_iterator& rhs) const noexcept {
				return !(*this == rhs);
			};

			[[nodiscard]] inline bool operator<(const VectorTree_iterator& rhs) const noexcept {
				return this->_index < rhs._index;
			};

			[[nodiscard]] inline bool operator<=(const VectorTree_iterator& rhs) const noexcept {
				return this->_index <= rhs._index;
			};

			[[nodiscard]] inline bool operator>(const VectorTree_iterator& rhs) const noexcept {
				return this->_index > rhs._index;
			};

			[[nodiscard]] inline bool operator>=(const VectorTree_iterator& rhs) const noexcept {
				return this->_index >= rhs._index;
			};
		};

	public:

		// STL aliases
		using value_type = T;
		using allocator_type = Allocator;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using iterator = VectorTree_iterator<_a_VT>;
		using const_iterator = VectorTree_const_iterator<_a_VT>;
		using reference = T&;
		using const_reference = const T&;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;

	private:

		// local aliases
		using _a_composite = impl_VectorTree_node_composite;
		using _a_leaf = impl_VectorTree_node_leaf;
		using _a_node_sh_var = std::variant<std::shared_ptr<_a_composite>, std::shared_ptr<_a_leaf>>;
		using _a_root = std::shared_ptr<_a_composite>;

		// members
		unsigned char _height{};
		_a_root _root{};
		std::size_t _size{};
		_a_path _path_to_leaf_node__current{};

		/*!
		 * @brief helper function for random access
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto get_path_to_leaf_node(std::size_t index) const -> _a_path
		{
			auto index_{ index };
			_a_path path_to_leaf_node{};
			for (auto i = 0; i < _height; ++i) {
				std::size_t level_capacity{ _a_VT::calculate_capacity(_height - i - 1) };
				std::size_t level_index{ index_ / level_capacity };
				path_to_leaf_node.push_back(level_index);
				index_ -= level_index * level_capacity;
			}
			return path_to_leaf_node;
		};

		/*!
		 * @brief helper function for random access
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto get_path_to_element(std::size_t index) const -> _a_path
		{
			auto index_{ index };
			_a_path path_to_element{};
			for (auto i = 0; i < _height; ++i) {
				std::size_t level_capacity{ _a_VT::calculate_capacity(_height - i - 1) };
				std::size_t level_index{ index_ / level_capacity };
				path_to_element.push_back(level_index);
				index_ -= level_index * level_capacity;
			}
			path_to_element.push_back(index_);
			return path_to_element;
		};

		/*!
		 * @brief gets the pointer to the leaf node with the given path to the node
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto get_leaf_node(
			const _a_path& path_to_leaf_node) const
			-> _a_leaf*
		{
			_a_composite* composite_raw = _root.get();
			for (auto i = 0; i < _height - 1; ++i) {
				auto& composite_sh_var{ composite_raw->_childs[path_to_leaf_node[i]] };
				auto& composite_sh{ std::get<std::shared_ptr<_a_composite>>(composite_sh_var) };
				composite_raw = composite_sh.get();
			}
			auto& leaf_sh_var{ composite_raw->_childs[path_to_leaf_node[_height - 1]] };
			auto& leaf_sh{ std::get<std::shared_ptr<_a_leaf>>(leaf_sh_var) };
			return leaf_sh.get();
		};

		/*!
		 * @brief Gets the pointer to the element with the given path to element
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] inline auto get_element(
			const _a_path& path_to_element) const -> reference
		{
			auto leaf_node{ get_leaf_node(path_to_element) };
			return leaf_node->_childs[path_to_element[_height]];
		};

		/*!
		 * @brief calculates the capacity for the given heightt
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] inline static auto calculate_capacity(unsigned char height) -> std::size_t
		{
			return static_cast<std::size_t>(std::pow(BufferSize, height + 1));
		};

		/*!
		 * @brief copies all the nodes involved in the path to the given leaf node
		 * this is the basic approach of a persistent VectorTree.
		 * copy and update only the nodes which affected from the operation (push, emplace or pop)
		 * and increment the shared reference count to the remaining nodes.
		 * this approach reduces the amount of the data copied by logN, N being BufferSize.
		 *
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto copy_nodes_in_the_path_to_leaf_node_1(
			const _a_path& path_to_leaf_node__current) const
			-> _a_VT
		{
			auto new_root_sh{ std::make_shared<_a_composite>(*_root.get()) };
			auto new_composite_raw{ new_root_sh.get() };
			for (auto i = 0; i < _height - 1; ++i) {
				auto child_index{ path_to_leaf_node__current[i] };
				const auto& child_composite_sh_var{ new_composite_raw->_childs[child_index] };
				const auto& child_composite_sh{ std::get<std::shared_ptr<_a_composite>>(child_composite_sh_var) };
				auto new_child_composite_sh = std::make_shared<_a_composite>(*child_composite_sh);
				new_composite_raw->_childs[child_index] = _a_node_sh_var(new_child_composite_sh);
				new_composite_raw = new_child_composite_sh.get();
			}
			const auto& child_leaf_sh_var{ new_composite_raw->_childs[path_to_leaf_node__current[_height - 1]] };
			const auto& child_leaf_sh{ std::get<std::shared_ptr<_a_leaf>>(child_leaf_sh_var) };
			auto new_child_leaf_sh = std::make_shared<_a_leaf>(*child_leaf_sh);
			new_composite_raw->_childs[path_to_leaf_node__current[_height - 1]] = _a_node_sh_var(new_child_leaf_sh);

			return _a_VT(_height, new_root_sh, _size, path_to_leaf_node__current);
		};

		/*!
		 * @brief copy_nodes_in_the_path_to_leaf_node_1 for two nodes
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto copy_nodes_in_the_path_to_leaf_node_2(
			const _a_path& path_to_leaf_node__current,
			const _a_path& path_to_leaf_node__other) const
			-> _a_VT
		{
			auto new_root_sh{ std::make_shared<_a_composite>(*_root.get()) };
			auto new_composite_raw{ new_root_sh.get() };
			for (auto i = 0; i < _height - 1; ++i) {
				auto child_index{ path_to_leaf_node__current[i] };
				const auto& child_composite_sh_var{ new_composite_raw->_childs[child_index] };
				const auto& child_composite_sh{ std::get<std::shared_ptr<_a_composite>>(child_composite_sh_var) };
				auto new_child_composite_sh = std::make_shared<_a_composite>(*child_composite_sh);
				new_composite_raw->_childs[child_index] = _a_node_sh_var(new_child_composite_sh);
				new_composite_raw = new_child_composite_sh.get();
			}
			const auto& child_leaf_sh_var1{ new_composite_raw->_childs[path_to_leaf_node__current[_height - 1]] };
			const auto& child_leaf_sh1{ std::get<std::shared_ptr<_a_leaf>>(child_leaf_sh_var1) };
			auto new_child_leaf_sh1{ std::make_shared<_a_leaf>(*child_leaf_sh1) };
			new_composite_raw->_childs[path_to_leaf_node__current[_height - 1]] = _a_node_sh_var(new_child_leaf_sh1);

			new_composite_raw = new_root_sh.get();
			for (auto i = 0; i < _height - 1; ++i) {
				auto child_index{ path_to_leaf_node__other[i] };
				const auto& child_composite_sh_var{ new_composite_raw->_childs[child_index] };
				const auto& child_composite_sh{ std::get<std::shared_ptr<_a_composite>>(child_composite_sh_var) };
				auto new_child_composite_sh = std::make_shared<_a_composite>(*child_composite_sh);
				new_composite_raw->_childs[child_index] = _a_node_sh_var(new_child_composite_sh);
				new_composite_raw = new_child_composite_sh.get();
			}
			const auto& child_leaf_sh_var2{ new_composite_raw->_childs[path_to_leaf_node__other[_height - 1]] };
			const auto& child_leaf_sh2{ std::get<std::shared_ptr<_a_leaf>>(child_leaf_sh_var2) };
			auto new_child_leaf_sh2{ std::make_shared<_a_leaf>(*child_leaf_sh2) };
			new_composite_raw->_childs[path_to_leaf_node__other[_height - 1]] = _a_node_sh_var(new_child_leaf_sh2);

			return _a_VT(_height, new_root_sh, _size, path_to_leaf_node__current);
		};

		/*!
		 * @brief helper function in order to locate the next leaf node
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto get_path_to_leaf_node__next(
			const _a_path& path_to_leaf_node__current) const -> _a_path
		{
			auto path_to_leaf_node__next = path_to_leaf_node__current;
			bool check{};
			for (int i = _height - 1; i >= 0; --i) {
				if (path_to_leaf_node__next[i] < BufferSize - 1) {
					check = true;
					++path_to_leaf_node__next[i];
					for (auto j = i + 1; j < _height; ++j) {
						path_to_leaf_node__next[j] = 0;
					}
					break;
				}
			}
			if (!check) { // condition is already dealt with the caller function
				throw std::logic_error("Algorithm error. Contact with the developer. 1");
			}
			return path_to_leaf_node__next;
		};

		/*!
		 * @brief helper function in order to locate the previous leaf node
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto get_path_to_leaf_node__previous(
			const _a_path& path_to_leaf_node__current) const -> _a_path
		{
			auto path_to_leaf_node___previous = path_to_leaf_node__current;
			bool check{};
			for (int i = _height - 1; i >= 0; --i) {
				if (path_to_leaf_node___previous[i] > 0) {
					check = true;
					--path_to_leaf_node___previous[i];
					for (auto j = i + 1; j < _height; ++j) {
						path_to_leaf_node___previous[j] = BufferSize - 1;
					}
					break;
				}
			}
			if (!check) { // condition is already dealt with the caller function
				throw std::logic_error("Algorithm error. Contact with the developer. 2");
			}
			return path_to_leaf_node___previous;
		};

		/*!
		 * @brief helper function to append (push or emplace) a new element:
		 *     Case 1. VectorTree is empty: create a new VectorTree for a single element
		 *     Case 2. VectorTree is full (_size == capacity): increase the height of the VectorTree and set the current leaf node
		 *     Case 3. the current leaf node has room for a new entity: append the new item into the current leaf node
		 *     Case 4. find the next leaf node to append the new item
		 *
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto append_helper() const -> _a_VT
		{
			// initialize the output VectorTree
			_a_VT new_VT{};

			// case 1
			if (empty()) {
				// the new VectorTree would have only one element
				new_VT = _a_VT(1);
			}
			// case 2
			else if (_size == capacity()) {
				// create a new root for which the 0th child is the current root
				auto new_root_sh{ std::make_shared<_a_composite>(_height + 1, _root) };

				// current leaf node of the new VectorTree would follow the 0th child of all the new composite nodes
				// accept for the new root for which the path would go to the 1st child as the 0th child is the current root.
				auto path_to_leaf_node__current{ _a_path(_height + 1, 0) };
				path_to_leaf_node__current[0] = 1;

				// the new VectorTree
				new_VT = _a_VT(
					_height + 1,
					new_root_sh,
					_size + 1,
					path_to_leaf_node__current);
			}
			else {
				auto leaf_node__current{ get_leaf_node(_path_to_leaf_node__current) };

				// case 3
				if (leaf_node__current->_childs.size() < BufferSize) {
					new_VT = copy_nodes_in_the_path_to_leaf_node_1(_path_to_leaf_node__current);
					++new_VT._size;
				}
				// case 4
				else {
					auto path_to_leaf_node__next{ get_path_to_leaf_node__next(_path_to_leaf_node__current) };
					new_VT = copy_nodes_in_the_path_to_leaf_node_1(path_to_leaf_node__next);
					++new_VT._size;
				}
			}
			return new_VT;
		};

		[[nodiscard]] inline iterator begin() {
			if (empty()) { return end(); }
			return iterator(*this);
		};
		[[nodiscard]] inline iterator end() {
			if (empty()) { return iterator(*this); }
			auto leaf_node__current{ get_leaf_node(_path_to_leaf_node__current) };
			return iterator(
				*this,
				_path_to_leaf_node__current,
				leaf_node__current->_childs.end(),
				leaf_node__current->_childs.size(),
				_size);
		};

		/*!
		 * @brief a constructor for internal use
		 */
		VectorTree(
			unsigned char height,
			_a_root root,
			std::size_t size_,
			_a_path path_to_leaf_node__current)
			:
			_height{ height },
			_root{ root },
			_size{ size_ },
			_path_to_leaf_node__current{ path_to_leaf_node__current } {};

		/*!
		 * @brief determines the required height for the given size
		 */
		auto get_height_for_size(std::size_t s) const -> unsigned char {
			if (!s) return 1;

			unsigned char height{ 1 };
			for (unsigned char i = MAX_VectorTree_HEIGHT; i > 0; i--) {
				std::size_t level_capacity{ _a_VT::calculate_capacity(i) };
				if (level_capacity < s) {
					height = i + 1;
					break;
				}
			}
			return height;
		};

	public:

		/*!
		 * @brief the default cctor
		 */
		VectorTree() = default;

		/*!
		 * @brief ctor with the input size
		 */
		explicit VectorTree(std::size_t s) {
			_size = s;
			_height = get_height_for_size(_size);
			_root = std::make_shared<_a_composite>(_height);
			_path_to_leaf_node__current = _a_path(_height, 0);
		};

		/*!
		 * @brief ctor with the input std::vector
		 */
		explicit VectorTree(const std::vector<T>& v) : VectorTree(v.size())
		{
			std::size_t size_current{};
			auto it{ v.cbegin() };
			while (size_current < _size) {
				auto leaf_node__current{ get_leaf_node(_path_to_leaf_node__current) };
				std::size_t size_remaining{ std::min(static_cast<std::size_t>(BufferSize), _size - size_current) };
				leaf_node__current->_childs.resize(size_remaining);
				std::copy(
					it,
					it + size_remaining,
					leaf_node__current->_childs.begin());

				size_current += size_remaining;
				it += size_remaining;
				if (size_current < _size) {
					_path_to_leaf_node__current = get_path_to_leaf_node__next(_path_to_leaf_node__current);
				}
			}
			_path_to_leaf_node__current = get_path_to_leaf_node(_size - 1);
		};

		/*!
		 * @brief the big 5
		 * apply copy-and-swap idiom for the assignment operator
		 * notice that the root node is cloned (i.e. deep copy)
		 * instead of incrementing the shared reference counts
		 */
		VectorTree(const VectorTree& rhs)
			:
			_height{ rhs._height },
			_size{ rhs._size },
			_path_to_leaf_node__current{ rhs._path_to_leaf_node__current }
		{
			if (rhs._root) _root = rhs._root->clone();
		};
		VectorTree& operator=(VectorTree rhs) {
			swap(*this, rhs);
			return *this;
		};
		VectorTree(VectorTree&& rhs) noexcept = default;
		~VectorTree() = default;

		/*!
		 * @brief friend swap function
		 */
		friend inline void swap(VectorTree& lhs, VectorTree& rhs) noexcept {
			using std::swap;
			swap(lhs._height, rhs._height);
			swap(lhs._root, rhs._root);
			swap(lhs._size, rhs._size);
			swap(lhs._path_to_leaf_node__current, rhs._path_to_leaf_node__current);
		};

		/*!
		 * @brief random access operator
		 * creates a vector to locate the path to the input index.
		 * hence, theoritically is not noexcept.
		 * but the size of the vector is very small
		 * (less than MAX_VectorTree_HEIGHT)
		 */
		[[nodiscard]] inline const_reference operator[](std::size_t index) const {
			return get_element(get_path_to_element(index));
		};

		[[nodiscard]] inline auto operator<=>(const VectorTree& rhs) const noexcept {
			return size() <=> rhs.size();
		};
		[[nodiscard]] inline bool operator==(const VectorTree& rhs) const noexcept = default;

		/*!
		 * @brief get VectorTree_const_iterator - at the beginning
		 */
		[[nodiscard]] inline const_iterator cbegin() const {
			if (empty()) { return cend(); }
			return const_iterator(*this);
		};
		/*!
		 * @brief get VectorTree_const_iterator - at the end
		 */
		[[nodiscard]] inline const_iterator cend() const {
			if (empty()) { return const_iterator(*this); }
			auto leaf_node__current{ get_leaf_node(_path_to_leaf_node__current) };
			return const_iterator(
				*this,
				_path_to_leaf_node__current,
				leaf_node__current->_childs.cend(),
				leaf_node__current->_childs.size(),
				_size);
		};

		/*!
		 * @brief get height
		 */
		[[nodiscard]] inline unsigned char height() const noexcept { return _height; };
		/*!
		 * @brief get size
		 */
		[[nodiscard]] inline std::size_t size() const noexcept { return _size; };
		/*!
		 * @brief get capacity
		 */
		[[nodiscard]] inline std::size_t capacity() const noexcept { return _a_VT::calculate_capacity(_height); };
		/*!
		 * @brief get if empty
		 */
		[[nodiscard]] inline bool empty() const noexcept { return _size == 0; };
		/*!
		 * @brief get the last element 
		 */
		[[nodiscard]] inline reference back() const {
			if (empty()) {
				throw std::logic_error("Cannot get the last element from an empty container.");
			}
			return get_element(get_path_to_element(_size - 1));
		};

		/*!
		 * @see the documentation of append_helper for the details
		 * @exceptsafe strong exception safety as non-mutating
		 */
		template <typename... Ts>
		[[nodiscard]] inline auto emplace_back(Ts&& ... ts) const -> _a_VT
		{
			auto new_VT{ append_helper() };
			auto leaf_node__current{ new_VT.get_leaf_node(new_VT._path_to_leaf_node__current) };
			leaf_node__current->_childs.emplace_back(std::forward<Ts>(ts)...);
			return new_VT;
		};

		/*!
		 * @see the documentation of append_helper for the details
		 * @exceptsafe strong exception safety as non-mutating
		 */
		template <typename U = T>
		[[nodiscard]] inline auto push_back(U&& t) const -> _a_VT
		{
			auto new_VT{ append_helper() };
			auto leaf_node__current{ new_VT.get_leaf_node(new_VT._path_to_leaf_node__current) };
			leaf_node__current->_childs.push_back(std::forward<U>(t));
			return new_VT;
		};

		/*!
		 * @brief pop the last element:
		 *     Case 1: VectorTree is empty: throw exception
		 *     Case 2: _size == 1: return an empty VectorTree
		 *     Case 3: _size - 1 == capacity(_height - 1): decrease the capacity by setting the root as the 1st child of the current root
		 *     Case 4: otherwise: pop the current leaf node and if required set _path_to_leaf_node__current as the previous node
		 * 
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto pop_back() const -> _a_VT {
			// initialize the output VectorTree
			_a_VT new_VT{};

			// case 1
			if (empty()) {
				throw std::logic_error("Cannot pop an empty container.");
			}
			if (_size > 1) { // hides case 2: the empty initialized VectorTree
				// case 3
				auto new_size{ _size - 1 };
				if (_height > 1 && new_size == _a_VT::calculate_capacity(_height - 1)) {
					auto new_root_sh{ std::get<std::shared_ptr<_a_composite>>(_root->_childs[0]) };
					auto path_to_leaf_node__current{ _a_path(_height - 1, BufferSize - 1) };
					new_VT = _a_VT(
						_height - 1,
						new_root_sh,
						new_size,
						path_to_leaf_node__current);
				}
				// case 4
				else {
					new_VT = copy_nodes_in_the_path_to_leaf_node_1(_path_to_leaf_node__current);
					auto leaf_node__current{ new_VT.get_leaf_node(new_VT._path_to_leaf_node__current) };
					leaf_node__current->_childs.pop_back();
					new_VT._size = new_size;

					// inspect if the current leaf node has children
					if (leaf_node__current->_childs.empty()) {
						new_VT._path_to_leaf_node__current = new_VT.get_path_to_leaf_node__previous(
							new_VT._path_to_leaf_node__current
						);
					}
				}
			}
			return new_VT;
		};

		/*!
		 * @brief swap and push can be applied BUT NOT REASONABLE for this VectorTree as the ordering is not reserved
		 */
		[[deprecated("inserting is not reasonable for this VectorTree as it does not reserve ordering.")]]
		auto insert(std::size_t, T) const -> _a_VT {
			throw std::logic_error("inserting is not reasonable for this VectorTree as it does not reserve ordering.");
		};

		/*!
		 * @brief erase an element at the middle by applying swap-and-pop:
		 *     Case 1: it._index >= _size: throw exception
		 *     Case 2: it._index == _size - 1: return pop_back()
		 *     Case 3:
		 *         _size - 1 == capacity(_height - 1):
		 *             swap the element to be removed with the last element
		 *             decrease the capacity by setting the root as the 1st child of the current root
		 *             reset _path_to_leaf_node__current
		 *     Case 4:
		 *         otherwise:
		 *             swap the element to be removed with the last element
		 *             pop the current leaf node
		 *             reset _path_to_leaf_node__current
		 * 
		 * CAUTION:
		 *     the order of all elements accept for the last element is reserved.
		 *     hence, only the iterator to the last element is invalidated.
		 *
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto erase(const const_iterator& it) const -> _a_VT {
			// initialize the output VectorTree
			_a_VT new_VT{};

			// case 1
			if (it._index >= _size) {
				throw std::logic_error("input iterator exceeds the end iterator.");
			}

			// case 2
			if (it._index == _size - 1) {
				new_VT = pop_back();
			}
			else {
				// get path to the element to be removed
				auto path_to_element__index__ori{ get_path_to_element(it._index) };
				const auto& element__last__ori{ get_element(get_path_to_element(_size - 1)) };
				auto new_size{ _size - 1 };

				// case 3
				if (_height > 1 && new_size == _a_VT::calculate_capacity(_height - 1)) {
					// create a VectorTree with dropped capacity
					auto new_root_sh{ std::get<std::shared_ptr<_a_composite>>(_root->_childs[0]) };
					auto path_to_leaf_node__current{ _a_path(_height - 1, BufferSize - 1) };
					_a_VT new_VT__temp(
						_height - 1,
						new_root_sh,
						new_size,
						path_to_leaf_node__current);

					// create a new VectorTree by copying the nodes of the dropped VectorTree in the path to the element to be removed.
					// keep in mind that this function modifies _path_to_leaf_node__current as the input path
					// reset _path_to_leaf_node__current after calling this function
					auto path_to_leaf_node__index{ _a_path(new_VT__temp._height) };
					std::copy(
						it._path_to_leaf_node__current.cbegin() + 1,
						it._path_to_leaf_node__current.cend(),
						path_to_leaf_node__index.begin()
					);
					new_VT = new_VT__temp.copy_nodes_in_the_path_to_leaf_node_1(path_to_leaf_node__index);
					new_VT._path_to_leaf_node__current = path_to_leaf_node__current;

					// swap the element to be removed (of the new VectorTree) with the last element of the original VectorTree
					auto path_to_element__index{ _a_path(new_VT._height + 1) };
					std::copy(
						path_to_element__index__ori.cbegin() + 1,
						path_to_element__index__ori.cend(),
						path_to_element__index.begin()
					);
					auto& element__index{ new_VT.get_element(path_to_element__index) };
					element__index = element__last__ori;
				}
				// case 4
				else {
					// get path to the leaf node corresponding to the input index
					auto path_to_leaf_node__index{ _a_path(_height) };
					std::copy(
						it._path_to_leaf_node__current.cbegin(),
						it._path_to_leaf_node__current.cend(),
						path_to_leaf_node__index.begin()
					);

					// create a new VectorTree by copying the nodes in the two paths: to the last element and to the element to be removed
					if (path_to_leaf_node__index == _path_to_leaf_node__current) {
						new_VT = copy_nodes_in_the_path_to_leaf_node_1(_path_to_leaf_node__current);
					}
					else {
						new_VT = copy_nodes_in_the_path_to_leaf_node_2(
							_path_to_leaf_node__current,
							path_to_leaf_node__index);
					}
					new_VT._size = new_size;

					// swap the element to be removed (of the new VectorTree) with the last element of the original VectorTree
					auto& element__index{ new_VT.get_element(path_to_element__index__ori) };
					element__index = element__last__ori;

					// pop the last element
					auto leaf_node__current{ new_VT.get_leaf_node(new_VT._path_to_leaf_node__current) };
					leaf_node__current->_childs.pop_back();

					// inspect if the current leaf node has children
					if (leaf_node__current->_childs.empty()) {
						new_VT._path_to_leaf_node__current = new_VT.get_path_to_leaf_node__previous(
							new_VT._path_to_leaf_node__current
						);
					}
				}
			}
			return new_VT;
		};

		/*!
		 * @brief erase an element at the middle by applying swap-and-pop - overload: by index
		 * @see the documentation of erase by iterator for the descriptions
		 *
		 * @exceptsafe strong exception safety as non-mutating
		 */
		[[nodiscard]] auto erase(std::size_t index) const -> _a_VT {
			return erase(cbegin() + index);
		};

		/*!
		 * @brief setter for the contained data
		 * @exceptsafe strong exception safety as non-mutating
		 */
		template <typename U = T>
		[[nodiscard]] auto set_at(std::size_t index, U&& value) const -> _a_VT {
			if (index >= _size) {
				throw std::logic_error("index out of bounds.");
			}

			// copy the nodes from the root to the leaf node containing the element to be set
			auto new_VT{ copy_nodes_in_the_path_to_leaf_node_1(get_path_to_leaf_node(index)) };

			// set the value of the element at the input index
			auto& element{ new_VT.get_element(new_VT.get_path_to_element(index)) };
			element = std::forward<U>(value);

			return new_VT;
		};

		/*!
		 * @brief STL for_each algorithm - unary function without args
		 * clone this VectorTree and apply the input function on the clone using non-const iterator
		 *
		 * @exceptsafe strong exception safety as non-mutating
		 */
		template<class UnaryFunc>
		[[nodiscard]] auto for_each(UnaryFunc f) const -> _a_VT
		{
			_a_VT new_VT{ *this };
			for (auto it = new_VT.begin(); it != new_VT.end(); ++it) {
				f(*it);
			}
			return new_VT;
		};

		/*!
		 * @brief STL for_each algorithm - non-unary function with args to apply on the elements of the VectorTree
		 * clone this VectorTree and apply the input function on the clone using non-const iterator
		 *
		 * @exceptsafe strong exception safety as non-mutating
		 */
		template<class UnaryFunc, typename... Ts>
		[[nodiscard]] auto for_each(UnaryFunc f, Ts... args) const -> _a_VT
		{
			_a_VT new_VT{ *this };
			for (auto it = new_VT.begin(); it != new_VT.end(); ++it) {
				f(*it, args...);
			}
			return new_VT;
		};
	};
}

#endif
