#include <cstdint>
#include <iostream>

#include "edge.hpp"
#include "low_weight.hpp"
#include "vertex.hpp"

using namespace ncube;

template <int32_t N>
int32_t slice_cube_one_weight(const std::vector<int32_t>& distances) {
  const auto edges = compute_edges(N);
  const auto low_weight_sets =
      compute_one_weight_sliceable_sets<N>(distances, edges);
  const auto k = combine_low_weight_sliceable_sets<N>(low_weight_sets, edges);
  return k;
}

int main() {
  constexpr int32_t N = 5;
  std::vector<int32_t> distances = {0, 1};
  std::cout << slice_cube_one_weight<N>(distances) << std::endl;
}
