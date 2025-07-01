/*!
 * Gtest test cases for VectorTree interface.
 *
 * @author    <baris.albayrak.ieee@gmail.com>
 * @version   0.0.1
 * @see       the README file of the github repository below for an overview of this project.
 * 
 * github:    <https://github.com/BarisAlbayrakIEEE/VectorTree.git>
 */

#ifndef _test_HeaderFile
#define _test_HeaderFile

#include <gtest/gtest.h>
#include <numeric>
#include <random>
#include "../inc/VectorTree.h"

using namespace VectorTreeNamespace;

struct Foo{
    public:
    int _i{};
    Foo() = default;
    Foo(int i) : _i(i) {};
};

using _vt = VectorTree<Foo>;

/*!
 * helper function to create a VectorTree with the input size - use emplace_back
 */
_vt get_vt_1(std::size_t N) {
    _vt vt{};
    for (auto i = 0; i < N; ++i) {
        vt = vt.emplace_back(i);
    }
    return vt;
};

/*!
 * helper function to create a VectorTree with the input size - use VectorTree constructor with std::vector
 */
_vt get_vt_2(std::size_t N) {
    std::vector<Foo> v{};
    for (std::size_t i = 0; i < N; ++i) v.emplace_back(i);
    return _vt(v);
};

static constexpr std::size_t LARGE_SIZE{ 1025 };
static constexpr std::size_t EDGE_SIZE{ 1024 };
_vt VT_0{};
_vt VT_LARGE = get_vt_2(LARGE_SIZE);
_vt VT_EDGE = get_vt_2(EDGE_SIZE);
std::random_device RD; // obtain a random number from hardware
std::mt19937 GEN(RD()); // seed the generator

/*!
 * test VectorTree for the ctor-default
 */
TEST(test_VectorTree, ctor_default) {
    ASSERT_EQ(VT_0.size(), 0);
    ASSERT_TRUE(VT_0.empty());
    for (auto it = VT_0.cbegin(); it != VT_0.cend(); ++it) {
        ASSERT_EQ(0, 1);
    }
}

/*!
 * test VectorTree for the ctor-vector - large size
 */
TEST(test_VectorTree, ctor_vector__large) {
    auto vt{ get_vt_2(LARGE_SIZE) };

    ASSERT_EQ(vt.size(), LARGE_SIZE);
    ASSERT_FALSE(vt.empty());
    std::size_t count{};
    for (auto it = vt.cbegin(); it != vt.cend(); ++it) {
        ASSERT_EQ(it->_i, count);
        ++count;
    }
}

/*!
 * test VectorTree for the ctor-vector - edge size
 */
TEST(test_VectorTree, ctor_vector__edge) {
    auto vt{ get_vt_2(EDGE_SIZE) };

    ASSERT_EQ(vt.size(), EDGE_SIZE);
    ASSERT_FALSE(vt.empty());
    std::size_t count{};
    for (auto it = vt.cbegin(); it != vt.cend(); ++it) {
        ASSERT_EQ(it->_i, count);
        ++count;
    }
}

/*!
 * test VectorTree for emplace_back - large size
 */
TEST(test_VectorTree, emplace_back__large) {
    auto vt_1{ VT_0 };
    for (auto i = 0; i < LARGE_SIZE; ++i) {
        auto vt_2 = vt_1.emplace_back(i);

        ASSERT_EQ(vt_1.size(), i);
        if (!vt_1.empty()) {
            ASSERT_EQ(vt_1.back()._i, i - 1);
        }

        ASSERT_EQ(vt_2.size(), i + 1);
        ASSERT_EQ(vt_2.back()._i, i);

        vt_1 = vt_2;
    }
}

/*!
 * test VectorTree for emplace_back - edge size
 */
TEST(test_VectorTree, emplace_back__edge) {
    auto vt_1{ VT_0 };
    for (auto i = 0; i < EDGE_SIZE; ++i) {
        auto vt_2 = vt_1.emplace_back(i);

        ASSERT_EQ(vt_1.size(), i);
        if (!vt_1.empty()) {
            ASSERT_EQ(vt_1.back()._i, i - 1);
        }

        ASSERT_EQ(vt_2.size(), i + 1);
        ASSERT_EQ(vt_2.back()._i, i);

        vt_1 = vt_2;
    }
}

/*!
 * test VectorTree for push_back - large size
 */
TEST(test_VectorTree, push_back__large) {
    auto vt_1{ VT_0 };
    for (auto i = 0; i < LARGE_SIZE; ++i) {
        auto vt_2 = vt_1.push_back(Foo(i));

        ASSERT_EQ(vt_1.size(), i);
        if (!vt_1.empty()) {
            ASSERT_EQ(vt_1.back()._i, i - 1);
        }

        ASSERT_EQ(vt_2.size(), i + 1);
        ASSERT_EQ(vt_2.back()._i, i);

        vt_1 = vt_2;
    }
}

/*!
 * test VectorTree for push_back - edge size
 */
TEST(test_VectorTree, push_back__edge) {
    auto vt_1{ VT_0 };
    for (auto i = 0; i < EDGE_SIZE; ++i) {
        auto vt_2 = vt_1.push_back(Foo(i));

        ASSERT_EQ(vt_1.size(), i);
        if (!vt_1.empty()) {
            ASSERT_EQ(vt_1.back()._i, i - 1);
        }

        ASSERT_EQ(vt_2.size(), i + 1);
        ASSERT_EQ(vt_2.back()._i, i);

        vt_1 = vt_2;
    }
}

/*!
 * test VectorTree for pop_back - large size
 */
TEST(test_VectorTree, pop_back__large) {
    auto vt_1{ VT_LARGE };
    for (auto i = LARGE_SIZE; i > 0; --i) {
        auto vt_2 = vt_1.pop_back();
        ASSERT_EQ(vt_1.size(), i);
        ASSERT_EQ(vt_1.back()._i, i - 1);

        ASSERT_EQ(vt_2.size(), i - 1);
        if (vt_2.size()) {
            ASSERT_EQ(vt_2.back()._i, i - 2);
        }
        vt_1 = vt_2;
    }
}

/*!
 * test VectorTree for pop_back - edge size
 */
TEST(test_VectorTree, pop_back__edge) {
    auto vt_1{ VT_EDGE };
    for (auto i = EDGE_SIZE; i > 0; --i) {
        auto vt_2 = vt_1.pop_back();
        ASSERT_EQ(vt_1.size(), i);
        ASSERT_EQ(vt_1.back()._i, i - 1);

        ASSERT_EQ(vt_2.size(), i - 1);
        if (vt_2.size()) {
            ASSERT_EQ(vt_2.back()._i, i - 2);
        }
        vt_1 = vt_2;
    }
}

/*!
 * test VectorTree for erase - large size
 */
TEST(test_VectorTree, erase__large) {
    auto vt_1{ VT_LARGE };
    for (auto i = LARGE_SIZE; i > 0; --i) {
        std::uniform_int_distribution<> distr(0, i - 1); // allow _size - 1 -> pop_back
        auto index{ distr(GEN) };
        auto vt_2 = vt_1.erase(index);

        ASSERT_EQ(vt_1.size(), i);

        ASSERT_EQ(vt_2.size(), i - 1);
        if (vt_2.size()) {
            if (index == i - 1) { // inspect pop_back
                ASSERT_EQ(vt_2.back()._i, vt_1[vt_1.size() - 2]._i);
            }
            else { // inspect swap-and-pop
                ASSERT_EQ(vt_2[index]._i, vt_1.back()._i);
            }
        }
        vt_1 = vt_2;
    }
}

/*!
 * test VectorTree for erase - edge size
 */
TEST(test_VectorTree, erase__edge) {
    auto vt_1{ VT_EDGE };
    for (auto i = EDGE_SIZE; i > 0; --i) {
        std::uniform_int_distribution<> distr(0, i - 1); // allow _size - 1 -> pop_back
        auto index{ distr(GEN) };
        auto vt_2 = vt_1.erase(index);

        ASSERT_EQ(vt_1.size(), i);

        ASSERT_EQ(vt_2.size(), i - 1);
        if (vt_2.size()) {
            if (index == i - 1) { // inspect pop_back
                ASSERT_EQ(vt_2.back()._i, vt_1[vt_1.size() - 2]._i);
            }
            else { // inspect swap-and-pop
                ASSERT_EQ(vt_2[index]._i, vt_1.back()._i);
            }
        }
        vt_1 = vt_2;
    }
}

/*!
 * test VectorTree for set_at
 */
TEST(test_VectorTree, set_at) {
    auto vt_1{ VT_LARGE };
    _vt vt_2;
    for (auto i = 0; i < LARGE_SIZE; ++i) {
        vt_2 = vt_1.set_at(i, Foo(2 * i));
        ASSERT_EQ(vt_1[i]._i, i);
        vt_1 = vt_2;
    }
    ASSERT_EQ(vt_1.size(), LARGE_SIZE);

    ASSERT_EQ(vt_2.size(), LARGE_SIZE);
    for (auto i = 0; i < LARGE_SIZE; ++i) {
        ASSERT_EQ(vt_2[i]._i, 2 * i);
    }
}

/*!
 * test VectorTree for traversal - large size
 */
TEST(test_VectorTree, traversal__large) {
    int i{};
    for (auto it = VT_LARGE.cbegin(); it != VT_LARGE.cend(); ++it) {
        ASSERT_EQ(it->_i, i);
        ++i;
    }
}

/*!
 * test VectorTree for traversal - edge size
 */
TEST(test_VectorTree, traversal__edge) {
    int i{};
    for (auto it = VT_EDGE.cbegin(); it != VT_EDGE.cend(); ++it) {
        ASSERT_EQ(it->_i, i);
        ++i;
    }
}

/*!
 * test VectorTree for for_each - unary function without args - large size
 */
TEST(test_VectorTree, for_each__unary__large) {
    auto vt_1{ VT_LARGE };
    auto vt_2{ vt_1.for_each([](auto& element){ element._i *= 2;}) };

    ASSERT_EQ(vt_1.size(), LARGE_SIZE);

    ASSERT_EQ(vt_2.size(), LARGE_SIZE);
    for (auto i = 0; i < LARGE_SIZE; ++i) {
        ASSERT_EQ(vt_2[i]._i, 2 * i);
    }
}

/*!
 * test VectorTree for for_each - unary function without args - edge size
 */
TEST(test_VectorTree, for_each__unary__edge) {
    auto vt_1{ VT_EDGE };
    auto vt_2{ vt_1.for_each([](auto& element){ element._i *= 2;}) };

    ASSERT_EQ(vt_1.size(), EDGE_SIZE);

    ASSERT_EQ(vt_2.size(), EDGE_SIZE);
    for (auto i = 0; i < EDGE_SIZE; ++i) {
        ASSERT_EQ(vt_2[i]._i, 2 * i);
    }
}

/*!
 * test VectorTree for for_each - non-unary function with args
 */
TEST(test_VectorTree, for_each__nonunary) {
    auto vt_1{ VT_LARGE };
    int coeff1{ 2 };
    int coeff2{ 3 };
    auto vt_2{
        vt_1.for_each(
            [](auto& element, auto c1, auto c2){ element._i *= c1 * c2;},
            coeff1,
            coeff2
        )
    };

    ASSERT_EQ(vt_1.size(), LARGE_SIZE);

    ASSERT_EQ(vt_2.size(), LARGE_SIZE);
    for (auto i = 0; i < LARGE_SIZE; ++i) {
        ASSERT_EQ(vt_2[i]._i, 6 * i);
    }
}

#endif
