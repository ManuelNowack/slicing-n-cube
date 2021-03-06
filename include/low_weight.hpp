#ifndef N_CUBE_LOW_WEIGHT_H_
#define N_CUBE_LOW_WEIGHT_H_

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

#include "bitset_comparator.hpp"
#include "edge.hpp"
#include "prettyprint.hpp"
#include "sliceable_set.hpp"
#include "vertex.hpp"

namespace ncube {

/**
 *  Returns the sliceable set induced by a low weight halfspace.
 *
 *  The low weight halfspace is given by its normal vector and threshold
 *  (distance to the origin).
 **/
template <int32_t N>
sliceable_set_t<N> low_weight_halfspace_to_sliceable_set(
    const std::array<int32_t, N>& normal, int32_t threshold,
    const edge_lexicon_t<N>& edges) {
  sliceable_set_t<N> ss;
  for (const auto& e : edges) {
    int32_t u_scalar = 0, v_scalar = 0;
    for (int32_t i = 0; i < N; ++i) {
      const auto u_i = get_coordinate(e.first, i);
      const auto v_i = get_coordinate(e.second, i);
      u_scalar += u_i * normal[i];
      v_scalar += v_i * normal[i];
    }
    if ((u_scalar < threshold && v_scalar > threshold) ||
        (u_scalar > threshold && v_scalar < threshold)) {
      ss[edge_to_int<N>(e, edges)] = true;
    }
  }
  return ss;
}

/**
 *  Changes a normal vector containing only values in {-1, 1} into the next
 *  normal vector containing only values in {-1, 1}.
 *
 *  Returns true if the resulting normal vector is all -1 and false otherwise.
 *
 *  Naturally, the first call should be on a normal vector that is all -1.
 **/
template <int32_t N>
bool next_one_weight_vector(std::array<int32_t, N>& normal) {
  for (auto it = normal.rbegin(); it != normal.rend(); ++it) {
    if (*it == -1) {
      *it = 1;
      return true;
    } else {
      *it = -1;
    }
  }
  return false;
}

/**
 *  Changes a normal vector containing only values in {-max, ..., max} into the
 *  next normal vector containing only values in {-max, ..., max}.
 *
 *  Returns true if the resulting normal vector is all -max and false otherwise.
 *
 *  Naturally, the first call should be on a normal vector that is all -max.
 **/
template <int32_t N>
bool next_low_weight_vector(std::array<int32_t, N>& normal, int32_t max) {
  for (auto it = normal.rbegin(); it != normal.rend(); ++it) {
    if (*it == max) {
      *it = -max;
    } else {
      *it += 1;
      return true;
    }
  }
  return false;
}

/**
 *  Returns all maximal sliceable sets induced by low weight halfspaces
 *  satisfying the following:
 *    - The normal vector contains only values in {-1, 1}.
 *    - The threshold (distance to the origin) is one of the given thresholds.
 *
 *  The returned sliceable sets are sorted in lexicographic order.
 **/
template <int32_t N>
std::vector<sliceable_set_t<N>> compute_one_weight_mss(
    const std::vector<int32_t>& thresholds, const edge_lexicon_t<N>& edges) {
  std::vector<sliceable_set_t<N>> sets;
  std::array<int32_t, N> normal;
  normal.fill(-1);
  do {
    for (const auto& threshold : thresholds) {
      const auto mss =
          low_weight_halfspace_to_sliceable_set<N>(normal, threshold, edges);
      if (mss.any()) {
        const auto is_superset_of_mss = [mss](const sliceable_set_t<N>& ss) {
          return (ss | mss) == ss;
        };
        if (std::none_of(sets.begin(), sets.end(), is_superset_of_mss)) {
          const auto is_subset_of_mss = [mss](const sliceable_set_t<N>& ss) {
            return (ss | mss) == mss;
          };
          const auto it = remove_if(sets.begin(), sets.end(), is_subset_of_mss);
          sets.erase(it, sets.end());
          sets.push_back(mss);
        }
      }
    }
  } while (next_one_weight_vector<N>(normal));
  std::sort(sets.begin(), sets.end());
  return sets;
}

/**
 *  Returns all maximal sliceable sets induced by low weight halfspaces
 *  satisfying the following:
 *    - The normal vector contains only values in {-max, ..., max}.
 *    - The threshold (distance to the origin) is any integer value.
 *
 *  The returned sliceable sets are sorted in lexicographic order.
 **/
template <int32_t N>
std::vector<sliceable_set_t<N>> compute_low_weight_mss(
    int32_t max, const edge_lexicon_t<N>& edges) {
  std::vector<sliceable_set_t<N>> sets;
  std::array<int32_t, N> normal;
  normal.fill(-max);
  do {
    for (int32_t threshold = 0; threshold < max * N; ++threshold) {
      const auto mss =
          low_weight_halfspace_to_sliceable_set<N>(normal, threshold, edges);
      if (mss.any()) {
        const auto is_superset_of_mss = [mss](const sliceable_set_t<N>& ss) {
          return (ss | mss) == ss;
        };
        if (std::none_of(sets.begin(), sets.end(), is_superset_of_mss)) {
          const auto is_subset_of_mss = [mss](const sliceable_set_t<N>& ss) {
            return (ss | mss) == mss;
          };
          const auto it = remove_if(sets.begin(), sets.end(), is_subset_of_mss);
          sets.erase(it, sets.end());
          sets.push_back(mss);
        }
      }
    }
  } while (next_low_weight_vector<N>(normal, max));
  std::sort(sets.begin(), sets.end());
  return sets;
}

/**
 *  Writes in lexicographic order all sliceable sets induced by low weight
 *  halfspaces satisfying the following to a file at the given path:
 *    - The normal vector contains only values in {-1, 1}.
 *    - The threshold (distance to the origin) is one of the given thresholds.
 *
 *  For each sliceable set the following is written in text:
 *    - The bitstring encoding of the sliceable set.
 *    - The normal vector.
 *    - The threshold.
 **/
template <int32_t N>
void write_one_weight_halfspaces_to_file(const std::vector<int32_t>& thresholds,
                                         const edge_lexicon_t<N>& edges,
                                         const std::filesystem::path& path) {
  std::vector<std::string> output;
  std::array<int32_t, N> normal;
  normal.fill(-1);
  do {
    for (const auto& threshold : thresholds) {
      const auto ss =
          low_weight_halfspace_to_sliceable_set<N>(normal, threshold, edges);
      if (ss.any()) {
        std::stringstream str_stream;
        str_stream << ss << " " << normal << " " << threshold;
        output.push_back(str_stream.str());
      }
    }
  } while (next_one_weight_vector<N>(normal));
  std::sort(output.begin(), output.end());
  std::ofstream file(path);
  for (const auto& str : output) {
    file << str << std::endl;
  }
}

/**
 *  Writes in lexicographic order all sliceable sets induced by low weight
 *  halfspaces satisfying the following to a file at the given path:
 *    - The normal vector contains only values in {-max, ..., max}.
 *    - The threshold (distance to the origin) is any integer value.
 *
 *  For each sliceable set the following is written in text:
 *    - The bitstring encoding of the sliceable set.
 *    - The normal vector.
 *    - The threshold.
 **/
template <int32_t N>
void write_low_weight_halfspaces_to_file(int32_t max,
                                         const edge_lexicon_t<N>& edges,
                                         const std::filesystem::path& path) {
  std::vector<std::string> output;
  std::array<int32_t, N> normal;
  normal.fill(-max);
  do {
    for (int32_t threshold = 0; threshold < max * N; ++threshold) {
      const sliceable_set_t<N> ss =
          low_weight_halfspace_to_sliceable_set<N>(normal, threshold, edges);
      if (ss.any()) {
        std::stringstream str_stream;
        str_stream << ss << " " << normal << " " << threshold;
        output.push_back(str_stream.str());
      }
    }
  } while (next_low_weight_vector<N>(normal, max));
  std::sort(output.begin(), output.end());
  std::ofstream file(path);
  for (const auto& str : output) {
    file << str << std::endl;
  }
}

}  // namespace ncube

#endif  // N_CUBE_LOW_WEIGHT_H_
