/*!
 * Google benchmark main routine for VectorTree interface.
 *
 * @author    <baris.albayrak.ieee@gmail.com>
 * @version   0.0.1
 * @see       the README file of the github repository below for an overview of this project.
 * 
 * github:    <https://github.com/BarisAlbayrakIEEE/VectorTree.git>
 */

#include "benchmark.h"

// concatenation helper macros
#define CAT8(a, b, c, d, e, f, g, h) a##b##c##d##e##f##g##h

// benchmark shortcut macro
#define DEFINE_BENCHMARK(type_operation, type_object, type_size, iteration_mul, type_target, separator)                            \
static void CAT8(BM__, type_operation, __, type_object, __, type_size, __, type_target)(benchmark::State& state) {                 \
    auto v0 = get_v<type_target, type_object>(type_size);                                                                          \
    for (auto _ : state) {                                                                                                         \
        state.PauseTiming();                                                                                                       \
        auto v1{ v0 };                                                                                                             \
        state.ResumeTiming();                                                                                                      \
        for (int i = 0; i < state.range(0); ++i) {                                                                                 \
            v1.type_operation();                                                                                                   \
            benchmark::DoNotOptimize(v1);                                                                                          \
        }                                                                                                                          \
    }                                                                                                                              \
}                                                                                                                                  \
BENCHMARK(BM__##type_operation##separator##type_object##separator##type_size##separator##type_target)                              \
    ->RangeMultiplier(iteration_mul)->Range(type_size / iteration_mul / iteration_mul / iteration_mul, type_size);

DEFINE_BENCHMARK(emplace_back, type_small, DEFAULT_BUFFER_1, 2, type_std, __)
DEFINE_BENCHMARK(emplace_back, type_small, DEFAULT_BUFFER_1, 2, type_persistent, __)
DEFINE_BENCHMARK(emplace_back, type_small, DEFAULT_BUFFER_1, 2, type_VT, __)
DEFINE_BENCHMARK(emplace_back, type_small, DEFAULT_BUFFER_2, 4, type_std, __)
DEFINE_BENCHMARK(emplace_back, type_small, DEFAULT_BUFFER_2, 4, type_persistent, __)
DEFINE_BENCHMARK(emplace_back, type_small, DEFAULT_BUFFER_2, 4, type_VT, __)
DEFINE_BENCHMARK(emplace_back, type_small, DEFAULT_BUFFER_3, 8, type_std, __)
DEFINE_BENCHMARK(emplace_back, type_small, DEFAULT_BUFFER_3, 8, type_persistent, __)
DEFINE_BENCHMARK(emplace_back, type_small, DEFAULT_BUFFER_3, 8, type_VT, __)
DEFINE_BENCHMARK(emplace_back, type_large, DEFAULT_BUFFER_1, 2, type_std, __)
DEFINE_BENCHMARK(emplace_back, type_large, DEFAULT_BUFFER_1, 2, type_persistent, __)
DEFINE_BENCHMARK(emplace_back, type_large, DEFAULT_BUFFER_1, 2, type_VT, __)
DEFINE_BENCHMARK(emplace_back, type_large, DEFAULT_BUFFER_2, 4, type_std, __)
DEFINE_BENCHMARK(emplace_back, type_large, DEFAULT_BUFFER_2, 4, type_persistent, __)
DEFINE_BENCHMARK(emplace_back, type_large, DEFAULT_BUFFER_2, 4, type_VT, __)
DEFINE_BENCHMARK(emplace_back, type_large, DEFAULT_BUFFER_3, 8, type_std, __)
DEFINE_BENCHMARK(emplace_back, type_large, DEFAULT_BUFFER_3, 8, type_persistent, __)
DEFINE_BENCHMARK(emplace_back, type_large, DEFAULT_BUFFER_3, 8, type_VT, __)

DEFINE_BENCHMARK(pop_back, type_small, DEFAULT_BUFFER_1, 2, type_std, __)
DEFINE_BENCHMARK(pop_back, type_small, DEFAULT_BUFFER_1, 2, type_persistent, __)
DEFINE_BENCHMARK(pop_back, type_small, DEFAULT_BUFFER_1, 2, type_VT, __)
DEFINE_BENCHMARK(pop_back, type_small, DEFAULT_BUFFER_2, 4, type_std, __)
DEFINE_BENCHMARK(pop_back, type_small, DEFAULT_BUFFER_2, 4, type_persistent, __)
DEFINE_BENCHMARK(pop_back, type_small, DEFAULT_BUFFER_2, 4, type_VT, __)
DEFINE_BENCHMARK(pop_back, type_small, DEFAULT_BUFFER_3, 8, type_std, __)
DEFINE_BENCHMARK(pop_back, type_small, DEFAULT_BUFFER_3, 8, type_persistent, __)
DEFINE_BENCHMARK(pop_back, type_small, DEFAULT_BUFFER_3, 8, type_VT, __)
DEFINE_BENCHMARK(pop_back, type_large, DEFAULT_BUFFER_1, 2, type_std, __)
DEFINE_BENCHMARK(pop_back, type_large, DEFAULT_BUFFER_1, 2, type_persistent, __)
DEFINE_BENCHMARK(pop_back, type_large, DEFAULT_BUFFER_1, 2, type_VT, __)
DEFINE_BENCHMARK(pop_back, type_large, DEFAULT_BUFFER_2, 4, type_std, __)
DEFINE_BENCHMARK(pop_back, type_large, DEFAULT_BUFFER_2, 4, type_persistent, __)
DEFINE_BENCHMARK(pop_back, type_large, DEFAULT_BUFFER_2, 4, type_VT, __)
DEFINE_BENCHMARK(pop_back, type_large, DEFAULT_BUFFER_3, 8, type_std, __)
DEFINE_BENCHMARK(pop_back, type_large, DEFAULT_BUFFER_3, 8, type_persistent, __)
DEFINE_BENCHMARK(pop_back, type_large, DEFAULT_BUFFER_3, 8, type_VT, __)

DEFINE_BENCHMARK(pop_front, type_small, DEFAULT_BUFFER_1, 2, type_std, __)
DEFINE_BENCHMARK(pop_front, type_small, DEFAULT_BUFFER_1, 2, type_persistent, __)
DEFINE_BENCHMARK(pop_front, type_small, DEFAULT_BUFFER_1, 2, type_VT, __)
DEFINE_BENCHMARK(pop_front, type_small, DEFAULT_BUFFER_2, 4, type_std, __)
DEFINE_BENCHMARK(pop_front, type_small, DEFAULT_BUFFER_2, 4, type_persistent, __)
DEFINE_BENCHMARK(pop_front, type_small, DEFAULT_BUFFER_2, 4, type_VT, __)
DEFINE_BENCHMARK(pop_front, type_small, DEFAULT_BUFFER_3, 8, type_std, __)
DEFINE_BENCHMARK(pop_front, type_small, DEFAULT_BUFFER_3, 8, type_persistent, __)
DEFINE_BENCHMARK(pop_front, type_small, DEFAULT_BUFFER_3, 8, type_VT, __)
DEFINE_BENCHMARK(pop_front, type_large, DEFAULT_BUFFER_1, 2, type_std, __)
DEFINE_BENCHMARK(pop_front, type_large, DEFAULT_BUFFER_1, 2, type_persistent, __)
DEFINE_BENCHMARK(pop_front, type_large, DEFAULT_BUFFER_1, 2, type_VT, __)
DEFINE_BENCHMARK(pop_front, type_large, DEFAULT_BUFFER_2, 4, type_std, __)
DEFINE_BENCHMARK(pop_front, type_large, DEFAULT_BUFFER_2, 4, type_persistent, __)
DEFINE_BENCHMARK(pop_front, type_large, DEFAULT_BUFFER_2, 4, type_VT, __)
DEFINE_BENCHMARK(pop_front, type_large, DEFAULT_BUFFER_3, 8, type_std, __)
DEFINE_BENCHMARK(pop_front, type_large, DEFAULT_BUFFER_3, 8, type_persistent, __)
DEFINE_BENCHMARK(pop_front, type_large, DEFAULT_BUFFER_3, 8, type_VT, __)

DEFINE_BENCHMARK(traversal, type_small, DEFAULT_BUFFER_1, 2, type_std, __)
DEFINE_BENCHMARK(traversal, type_small, DEFAULT_BUFFER_1, 2, type_persistent, __)
DEFINE_BENCHMARK(traversal, type_small, DEFAULT_BUFFER_1, 2, type_VT, __)
DEFINE_BENCHMARK(traversal, type_small, DEFAULT_BUFFER_2, 4, type_std, __)
DEFINE_BENCHMARK(traversal, type_small, DEFAULT_BUFFER_2, 4, type_persistent, __)
DEFINE_BENCHMARK(traversal, type_small, DEFAULT_BUFFER_2, 4, type_VT, __)
DEFINE_BENCHMARK(traversal, type_small, DEFAULT_BUFFER_3, 8, type_std, __)
DEFINE_BENCHMARK(traversal, type_small, DEFAULT_BUFFER_3, 8, type_persistent, __)
DEFINE_BENCHMARK(traversal, type_small, DEFAULT_BUFFER_3, 8, type_VT, __)
DEFINE_BENCHMARK(traversal, type_large, DEFAULT_BUFFER_1, 2, type_std, __)
DEFINE_BENCHMARK(traversal, type_large, DEFAULT_BUFFER_1, 2, type_persistent, __)
DEFINE_BENCHMARK(traversal, type_large, DEFAULT_BUFFER_1, 2, type_VT, __)
DEFINE_BENCHMARK(traversal, type_large, DEFAULT_BUFFER_2, 4, type_std, __)
DEFINE_BENCHMARK(traversal, type_large, DEFAULT_BUFFER_2, 4, type_persistent, __)
DEFINE_BENCHMARK(traversal, type_large, DEFAULT_BUFFER_2, 4, type_VT, __)
DEFINE_BENCHMARK(traversal, type_large, DEFAULT_BUFFER_3, 8, type_std, __)
DEFINE_BENCHMARK(traversal, type_large, DEFAULT_BUFFER_3, 8, type_persistent, __)
DEFINE_BENCHMARK(traversal, type_large, DEFAULT_BUFFER_3, 8, type_VT, __)
