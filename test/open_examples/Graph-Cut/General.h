/*
 * Copyright (c) 2018, Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef GENERAL_H
#define GENERAL_H

#include <iterator>
#include <stdint.h>
#include <vector>

typedef struct {
  int row;
  int col;
  int nextActiveBlock;
} RelabelBlock;

struct active_block_t {
  active_block_t(uint16_t row, uint16_t col) : row(row), col(col) {}

  uint16_t row;
  uint16_t col;
};

typedef std::vector<active_block_t> active_list;

class ActiveBlock_Iterator
    : public std::iterator<std::input_iterator_tag, RelabelBlock> {
private:
  RelabelBlock *p;
  int idx;

public:
  ActiveBlock_Iterator(RelabelBlock *data, int start_idx)
      : p(data), idx(start_idx) {}
  ActiveBlock_Iterator &operator++() {
    if (p[idx].nextActiveBlock != -1)
      idx += p[idx].nextActiveBlock;
    else
      idx = -1;
    return *this;
  }
  bool operator==(const ActiveBlock_Iterator &rhs) {
    return (p == rhs.p) && (idx == rhs.idx);
  }
  bool operator!=(const ActiveBlock_Iterator &rhs) {
    return (p != rhs.p) || (idx != rhs.idx);
  }
  RelabelBlock &operator*() { return p[idx]; }
};

class ActiveBlocks {
private:
  active_list &m_local;

public:
  ActiveBlocks(active_list &data) : m_local(data) {}

  active_list::iterator begin() {
    return m_local.begin();
  }
  active_list::iterator end() {
   return m_local.end(); 
  }

};

#endif
