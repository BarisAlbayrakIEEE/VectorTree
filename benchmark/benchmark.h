/*!
 * Google benchmarks for VectorTree interface.
 *
 * @author    <baris.albayrak.ieee@gmail.com>
 * @version   0.0.1
 * @see       the README file of the github repository below for an overview of this project.
 * 
 * github:    <https://github.com/BarisAlbayrakIEEE/VectorTree.git>
 */

#ifndef _benchmark_HeaderFile
#define _benchmark_HeaderFile

#include <benchmark/benchmark.h>
#include <type_traits>
#include "../inc/VectorTree.h"

using namespace VectorTreeNamespace;

struct type_std;
struct type_persistent;
struct type_VT;

/*!
 * small object
 */
struct type_small{
    int _i{};

    public:
    type_small() = default;
    type_small(int i) : _i(i) {};
};

/*!
 * large object
 */
using _u_large = std::array<int, 256>;
struct type_large{
    _u_large _arr{{}};
    int _i{};

    public:
    type_large() = default;
    type_large(int i) : _i(i) { for (auto& val : _arr) val = i; };
};

/*!
 * type traits for small and large types
 */
template <typename T>
using _vs = std::vector<T>;
template <typename T>
using _vp = std::vector<T>;
template <typename T>
using _vt = VectorTree<T>;

using _vs_s = _vs<type_small>;
using _vs_l = _vs<type_large>;
using _vp_s = _vp<type_small>;
using _vp_l = _vp<type_large>;
using _vt_s = _vt<type_small>;
using _vt_l = _vt<type_large>;

template <typename VectorType, typename T>
using _u_v_t = typename std::conditional_t<
    std::is_same<VectorType, type_std>::value,
    std::conditional_t<
        std::is_same<T, type_small>::value,
        _vs_s,
        _vs_l>,
    std::conditional_t<
        std::is_same<VectorType, type_persistent>::value,
        std::conditional_t<
            std::is_same<T, type_small>::value,
            _vp_s,
            _vp_l>,
            std::conditional_t<
                std::is_same<T, type_small>::value,
                _vt_s,
                _vt_l>>>;

/*!
 * vector_wrapper: a wrapper class to provide a uniform interface for the benchmark targets:
 *   std::vector
 *   simply persistent std::vector
 *   VectorTree
 * base template for:
 *   VectorTree
 */
template <typename VectorType, typename T>
class vector_wrapper{
    using _u_v = _u_v_t<VectorType, T>;
    _u_v _v{};
    std::size_t _c{};

    public:
    vector_wrapper() = default;
    vector_wrapper(std::size_t N) : _v(_u_v(N)) {};
    template <typename... Ts>
    vector_wrapper(std::size_t N, Ts&&... args) {
        auto v(std::vector<T>(N, T(std::forward<Ts...>(args...))));
        _v = _u_v(v);
    };

    void emplace_back() {
        _v = _v.emplace_back(1);
    };
    void pop_back() {
        _v = _v.pop_back();
    };
    void pop_front() {
        _v = _v.erase(0);
    };
    auto traversal() {
        _c = 0;
        for (auto it = _v.cbegin(); it != _v.cend(); ++it) _c += it->_i % 2;
    };
};

/*!
 * vector_wrapper: a wrapper class to represent the benchmark operations
 * specialization for:
 *   std::vector
 */
template <typename T>
class vector_wrapper<type_std, T>{
    using _u_v = _u_v_t<type_std, T>;
    _u_v _v{};
    std::size_t _c{};

    public:
    vector_wrapper() = default;
    vector_wrapper(std::size_t N) : _v(_u_v(N)) {};
    template <typename... Ts>
    vector_wrapper(std::size_t N, Ts&&... args) : _v(_u_v(N, T(std::forward<Ts...>(args...)))) {};

    void emplace_back() {
        _v.emplace_back(1);
    };
    void pop_back() {
        _v.pop_back();
    };
    void pop_front() {
        _v.erase(_v.begin());
    };
    auto traversal() {
        _c = 0;
        for (const auto& t : _v) _c += t._i % 2;
    };
};

/*!
 * vector_wrapper: a wrapper class to represent the benchmark operations
 * specialization for:
 *   a simple persistent vector
 */
template <typename T>
class vector_wrapper<type_persistent, T>{
    using _u_v = _u_v_t<type_persistent, T>;
    _u_v _v{};
    std::size_t _c{};

    public:
    vector_wrapper() = default;
    vector_wrapper(std::size_t N) : _v(_u_v(N)) {};
    template <typename... Ts>
    vector_wrapper(std::size_t N, Ts&&... args) : _v(_u_v(N, T(std::forward<Ts...>(args...)))) {};

    void emplace_back() {
        auto temp{ _v };
        temp.emplace_back(1);
        _c = temp.size();
    };
    void pop_back() {
        auto temp{ _v };
        temp.pop_back();
        _c = temp.size();
    };
    void pop_front() {
        auto temp{ _v };
        temp.erase(temp.begin());
        _c = temp.size();
    };
    auto traversal() {
        _c = 0;
        for (const auto& t : _v) _c += t._i % 2;
    };
};

/*!
 * helper function to create a filled vector wrapper of the input size
 */
template <typename VectorType, typename T>
auto get_v(std::size_t N) -> vector_wrapper<VectorType, T> {
    return vector_wrapper<VectorType, T>(N, 42);
};

static constexpr std::size_t DEFAULT_BUFFER_1{ VectorTreeNamespace::DEFAULT_BUFFER };
static constexpr std::size_t DEFAULT_BUFFER_2{
    VectorTreeNamespace::DEFAULT_BUFFER *
    VectorTreeNamespace::DEFAULT_BUFFER };
static constexpr std::size_t DEFAULT_BUFFER_3{
    VectorTreeNamespace::DEFAULT_BUFFER *
    VectorTreeNamespace::DEFAULT_BUFFER *
    VectorTreeNamespace::DEFAULT_BUFFER };

#endif
