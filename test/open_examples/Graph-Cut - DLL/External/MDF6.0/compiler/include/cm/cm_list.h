/**             
*** -----------------------------------------------------------------------------------------------
*** cvs_id[] = "$Id: cm_list.h 20177 2009-08-08 00:28:00Z ayermolo $"
*** -----------------------------------------------------------------------------------------------
***
*** Copyright  (C) 2008-2016 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation. and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
***
***
*** Authors:             
***                      
***                      
***                      
***
*** Description: Cm list template class. Added to avoid MSVC STL dependencies.
***
*** -----------------------------------------------------------------------------------------------
**/

#ifndef CM_LIST_H
#define CM_LIST_H

template <class T>
class cm_list {

    template <class Tl>
    struct _CM_List_Node {

        void *_Next;
        void *_Prev;
        Tl _Data;

        _CM_List_Node() : _Next(NULL), _Prev(NULL) {}
        _CM_List_Node(Tl d) :  _Next(NULL), _Prev(NULL), _Data(d) {}
    };
    
    template <class Ti>
    struct _CM_List_Iterator {
        typedef _CM_List_Node<Ti> *cm_node_ptr;
        typedef _CM_List_Iterator<Ti> iterator;

        cm_node_ptr _Ptr;

        _CM_List_Iterator() : _Ptr(NULL) {}
        _CM_List_Iterator(cm_node_ptr p) : _Ptr(p) {}
        _CM_List_Iterator(const iterator& i) : _Ptr(i._Ptr) {}

        iterator& operator++() { 
            _Ptr = (cm_node_ptr)(_Ptr->_Next);
            return *this;
        }
        bool operator==(const iterator& i) const { return _Ptr == i._Ptr; }
        bool operator!=(const iterator& i) const { return _Ptr != i._Ptr; }

        Ti& operator*() const { return (*_Ptr)._Data; }
        Ti* operator->() const { return &(operator*()); }

    };

    typedef _CM_List_Node<T> *cm_node_ptr;

    _CM_List_Node<T> _Base;

public:
    typedef _CM_List_Iterator<T> iterator;

    cm_list() {_Base._Next = &_Base;_Base._Prev = &_Base;}
    
    ~cm_list() {
        for (iterator next, i = begin(); i != end(); i = next) {
            next = i;
            ++next;
            cm_node_ptr n = i._Ptr;
            delete n;
        }
    }
    
    iterator begin() {return (cm_node_ptr)(_Base._Next);}
    
    iterator end() {return (cm_node_ptr)(&_Base);}

    void add(T data) {
        cm_node_ptr n = new _CM_List_Node<T>(data);
        cm_node_ptr top = (cm_node_ptr)_Base._Next;
        n->_Next = top;
        n->_Prev = top->_Prev;
        top->_Prev = n;
        _Base._Next = n;
    }
    //added for dyn generation stuff so that parameters are in the correct order for iteration
    void push_back(T data)
    {
        cm_node_ptr n = new _CM_List_Node<T>(data);
        cm_node_ptr bot = (cm_node_ptr)_Base._Prev;
        n->_Prev=bot;
        n->_Next = bot->_Next;
        bot->_Next = n;
        _Base._Prev=n;
    }
    void push_front(T data) { add(data); }
    
    void remove(iterator i) {
        cm_node_ptr n = i._Ptr;

        ((cm_node_ptr)n->_Next)->_Prev =
            n->_Prev;
        ((cm_node_ptr)n->_Prev)->_Next =
            n->_Next;
        delete n;
    }
    
    void remove(const T& d) {
        for (iterator next, i = begin(); i != end(); i = next) {
            next = i;
            ++next;
            if (*i == d) {
                remove(i);
            }
        }
    }

    size_t size() {
        size_t res = 0;
        for (iterator i = begin(); i != end(); ++i, ++res);
        return res;
    }

    T& front() { return *begin(); }

    bool empty() {              
        return size() == 0; 
    }
    void clear()
    {
        for (iterator next, i = begin(); i != end(); i = next) {
            next = i;
            ++next;
            cm_node_ptr n = i._Ptr;
            delete n;
        }
        _Base._Next = &_Base;_Base._Prev = &_Base;
    }
};

#endif /* CM_LIST_H */
