/*
MIT License

Copyright (c) 2025 forgotthepen (https://github.com/forgotthepen/gastar)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <set>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <cstdio> // std::size_t


namespace gastar {

class gastar_exception : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class gastar {
private:
  using t_fn_distance = unsigned (
    const void *from, const void *to,
    const void *start, const void *end
  );
  using t_fn_heuristic = unsigned (
    const void *node, const void *parent,
    const void *start, const void *end
  );
  using t_fn_neighbor = const void* (const void *node);
  using t_fn_usable = bool (
    const void *node, const void *parent,
    const void *start, const void *end
  );
  using t_fn_reconstruct = void (const void *node);

  struct t_node {
    struct {
      bool assigned;
      unsigned value;
    } g_score;
    unsigned f_score;
    t_node *parent;
    // used for path construction at the end
    const void *obj;
  };

  static bool node_sort(const t_node* const lhs, const t_node* const rhs);

  std::unordered_map<const void* /*obj*/, t_node> all_nodes_;
  std::set<t_node*, decltype(&node_sort)> open_set_;

  std::function<t_fn_distance> fn_distance_;
  std::function<t_fn_heuristic> fn_heuristic_;
  std::function<t_fn_neighbor> fn_neighbor_;
  std::function<t_fn_usable> fn_usable_;

  t_node *start_node_;
  t_node *end_node_;
  t_node *current_node_;

  t_node* get_or_cache_node(const void* const obj);

public:
  gastar();

  void setup(
    const void *start, const void *end,
    std::function<t_fn_distance> fn_distance,
    std::function<t_fn_heuristic> fn_heuristic,
    std::function<t_fn_neighbor> fn_neighbor,
    std::function<t_fn_usable> fn_usable
  );
  bool solve_next(bool continuous = false);
  const void* get_current_node() const;
  bool is_done() const;
  bool is_solved() const;
  void reconstruct_end(const std::function<t_fn_reconstruct> &fn_reconstruct) const;
  void reconstruct_current(const std::function<t_fn_reconstruct> &fn_reconstruct) const;

};

}
