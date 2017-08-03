#ifndef __STDEM_LIST_H__
#define __STDEM_LIST_H__

#include <stdem/assert.h>

namespace StdEm {


class List;

class ListNode
{
  friend class List;

  ListNode* m_prev;
  ListNode* m_next;
  void*     m_data;

public:
  ListNode(void* data = 0);

  void setData(void* data)  {m_data = data;}

  ListNode* prev() const    {return m_prev;}
  ListNode* next() const    {return m_next;}
};


class List
{
  ListNode* m_head;
  ListNode* m_tail;
//  uint16_t  m_size;

public:
  List();

  void insertBefore(ListNode* node, ListNode* at);
  void insertAfter(ListNode* node, ListNode* at);
  void remove(ListNode* node);

  void insertHead(ListNode* node) {insertBefore(node, head());}
  void insertTail(ListNode* node) {insertAfter(node, tail());}

//  Int2u size() const      {return m_size;}
  bool isEmpty() const    {return m_head == 0;}

  ListNode* head() const  {return m_head;}
  ListNode* tail() const  {return m_tail;}

  // TODO: get rid of this function?
  static void* item(const ListNode* nd) {STDEM_ASSERT(nd != 0); return nd->m_data;}
};


template <class T>
class ListT: public List
{
public:
  T* headItem() const  {return item(head());}
  T* tailItem() const  {return item(tail());}

  static T* item(const ListNode* nd)        {return (T*)List::item(nd);}
  T* operator() (const ListNode* nd) const  {return item(nd);}
  T& operator[] (const ListNode* nd) const  {return * item(nd);}
};

} // namespace StdEm


#endif // __STDEM_LIST_H__
