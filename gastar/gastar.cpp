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

#include "gastar.hpp"
#include <utility>

namespace gastar {

  bool gastar::node_sort(const t_node* const lhs, const t_node* const rhs) {
    // avoid duplicates, this fails in both cases: cmp(a, b) and cmp(b, a)
    // which means a & b are the same thing
    if (lhs == rhs || lhs->obj == rhs->obj) {
      return false;
    }

    // prefer paths with shorter distance to end
    if (lhs->f_score < rhs->f_score) {
      return true;
    } else if (lhs->f_score > rhs->f_score) {
      return false;
    }
    
    // otherwise, prefer paths with shorter distance from start
    if (lhs->g_score.value < rhs->g_score.value) {
      return true;
    } else if (lhs->g_score.value > rhs->g_score.value) {
      return false;
    }

    // otherwise, compare addresses
    return reinterpret_cast<const void* const>(lhs) < reinterpret_cast<const void* const>(rhs);
  }

  gastar::t_node* gastar::get_or_cache_node(const void* const obj) {
    auto node_pair = all_nodes_.emplace(obj, t_node{}); // TODO
    const bool is_new = node_pair.second;
    auto node_kv = node_pair.first;
    if (!is_new) {
      return &node_kv->second;
    }

    t_node &node = node_kv->second;
    node.g_score.assigned = false;
    node.g_score.value = 0;
    node.f_score = 0;
    node.parent = nullptr;
    node.obj = obj;
    return &node;
  }

  gastar::gastar():
    open_set_(&node_sort),
    start_node_(nullptr),
    end_node_(nullptr),
    current_node_(nullptr)
  { }

  void gastar::setup(
    const void *start, const void *end,
    std::function<t_fn_distance> fn_distance,
    std::function<t_fn_heuristic> fn_heuristic,
    std::function<t_fn_neighbor> fn_neighbor,
    std::function<t_fn_usable> fn_usable
  ) {
    if (nullptr == start) {
      throw gastar_exception("start node cannot be empty");
    }

    if (nullptr == end) {
      throw gastar_exception("end node cannot be empty");
    }

    if (!fn_distance) {
      throw gastar_exception("distance calculation function cannot be empty");
    }

    if (!fn_heuristic) {
      throw gastar_exception("heuristic calculation function cannot be empty");
    }

    if (!fn_neighbor) {
      throw gastar_exception("neighbor getter function cannot be empty");
    }

    fn_distance_ = std::move(fn_distance);
    fn_heuristic_ = std::move(fn_heuristic);
    fn_neighbor_ = std::move(fn_neighbor);
    fn_usable_ = std::move(fn_usable);

    all_nodes_.clear();
    open_set_.clear();

    current_node_ = nullptr;

    end_node_ = get_or_cache_node(end);

    start_node_ = get_or_cache_node(start);
    start_node_->g_score.assigned = true;
    start_node_->g_score.value = 0; // we just started
    start_node_->f_score = fn_heuristic_(start, nullptr, start, end);

    open_set_.emplace(start_node_);
  }

  bool gastar::solve_next(bool continuous) {
    do {
      if (open_set_.empty()) {
        break;
      }

      current_node_ = *open_set_.begin();
      open_set_.erase(open_set_.begin());

      for (const void *neighbor_obj = fn_neighbor_(current_node_->obj);
        nullptr != neighbor_obj;
        neighbor_obj = fn_neighbor_(current_node_->obj)
      ) {
        if (fn_usable_ && !fn_usable_(
          neighbor_obj, current_node_->obj,
          start_node_->obj, end_node_->obj)
        ) {
          continue;
        }

        t_node *neighbor_node = get_or_cache_node(neighbor_obj);
        unsigned new_g_score = current_node_->g_score.value + fn_distance_(
          current_node_->obj, neighbor_obj,
          start_node_->obj, end_node_->obj
        );

        if (!neighbor_node->g_score.assigned || new_g_score < neighbor_node->g_score.value) {
          neighbor_node->g_score.assigned = true;
          neighbor_node->g_score.value = new_g_score;
          neighbor_node->f_score = new_g_score + fn_heuristic_(
            neighbor_obj, current_node_->obj,
            start_node_->obj, end_node_->obj
          );
          neighbor_node->parent = current_node_;
          open_set_.emplace(neighbor_node);
        }
      }
    } while(continuous);

    return open_set_.empty();
  }

  const void* gastar::get_current_node() const {
    return current_node_;
  }

  bool gastar::is_solved() const {
    return is_done() && nullptr != end_node_->parent;
  }

  bool gastar::is_done() const {
    return nullptr != end_node_ && open_set_.empty();
  }

  void gastar::reconstruct_end(const std::function<t_fn_reconstruct> &fn_reconstruct) const {
    if (!fn_reconstruct) {
      throw gastar_exception("reconstruction function cannot be empty");
    }

    if (!is_solved()) {
      throw gastar_exception("cannot reconstruct from end, path not solved");
    }

    auto node = end_node_;
    while (nullptr != node) {
      fn_reconstruct(node->obj);
      node = node->parent;
    }
  }

  void gastar::reconstruct_current(const std::function<t_fn_reconstruct> &fn_reconstruct) const {
    if (!fn_reconstruct) {
      throw gastar_exception("reconstruction function cannot be empty");
    }

    if (nullptr == current_node_) {
      throw gastar_exception("cannot reconstruct from current node, path not evaluated");
    }

    auto node = current_node_;
    while (nullptr != node) {
      fn_reconstruct(node->obj);
      node = node->parent;
    }
  }

}
