#include <chrono>
#include <cstdint>
#include <iostream>

#include "complex.hpp"
#include "edge.hpp"
#include "sliceable_set.hpp"
#include "vertex.hpp"

using namespace ncube;

bool slice_5_cube() {
  constexpr int32_t N = 5;
  const auto edges = compute_edges<N>();
  const auto complexes = compute_complexes<N>(is_complex_degree_one<N>);
  const auto usr_1 = complexes_to_usr<N>(complexes, edges);
  const auto mss_1 = expand_usr<N>(usr_1, edges);
  const auto usr_2 = pairwise_unions<N>(usr_1, mss_1, edges);
  const auto mss_2 = expand_usr<N>(usr_2, edges);
  const auto start = std::chrono::high_resolution_clock::now();
  const auto slices_cube = pairwise_unions_slice_cube<N>(usr_2, mss_2);
  const auto stop = std::chrono::high_resolution_clock::now();
  const auto duration =
      std::chrono::duration_cast<std::chrono::seconds>(stop - start);
  std::cout << "Execution time of pairwise_unions_slice_cube: "
            << duration.count() << std::endl;
  return slices_cube;
}

int main() {
  const auto slices_all = slice_5_cube();
  std::cout << "Can four hyperplanes slice the 5-cube: " << slices_all
            << std::endl;
}
