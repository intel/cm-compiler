#ifndef GENERAL_H
#define GENERAL_H

#include <iterator>
#include <vector>
#include <stdint.h>

typedef struct
{
	int row;
	int col;
	int nextActiveBlock;
}RelabelBlock;

struct active_block_t
{
	active_block_t(uint16_t row, uint16_t col): row(row), col(col) {}

	uint16_t row;
	uint16_t col;

};


typedef std::vector<active_block_t> active_list;

class ActiveBlock_Iterator: public std::iterator<std::input_iterator_tag, RelabelBlock>
{
private:
	RelabelBlock *p;
	int idx;

public:
	ActiveBlock_Iterator(RelabelBlock *data, int start_idx): p(data), idx(start_idx) {}
	ActiveBlock_Iterator& operator++() {
		if(p[idx].nextActiveBlock != -1)
			idx +=  p[idx].nextActiveBlock;
		else
			idx = -1;
		return *this; 
	}
	bool operator==(const ActiveBlock_Iterator& rhs) {return (p == rhs.p) && (idx == rhs.idx);}
	bool operator!=(const ActiveBlock_Iterator& rhs) {return (p != rhs.p) || (idx != rhs.idx);}
	RelabelBlock& operator*() { return p[idx]; }
};

class ActiveBlocks
{
private:
	active_list &m_local;
public:
	ActiveBlocks(active_list &data): m_local(data) {}

	active_list::iterator begin() { return m_local.begin(); /*ActiveBlock_Iterator(m_local, m_local[0].nextActiveBlock);*/ }
	active_list::iterator end()   { return m_local.end(); } // ActiveBlock_Iterator(m_local, -1); }
};


#endif