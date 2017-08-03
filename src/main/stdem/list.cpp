#include <stdem/list.h>


namespace StdEm {

ListNode::ListNode(void* data)
{
  m_data = data;
  m_next = m_prev = 0;
}

  
List::List()
{
  m_head  = 0;
  m_tail  = 0;
//  m_size  = 0;
}

void List::insertBefore(ListNode* node, ListNode* at)
{
  STDEM_ASSERT(node->m_prev == 0);
  STDEM_ASSERT(node->m_next == 0);
  
  if(isEmpty())
    m_head = m_tail = node;
  else
  {
    STDEM_ASSERT(at != 0);
    
    node->m_next = at;
    
    if(at->m_prev != 0)
    {
      at->m_prev -> m_next = node;
      node->m_prev = at->m_prev;
    }
    else
      m_head = node;
    
    at->m_prev = node;
  }
  
//  ++m_size;
}

void List::insertAfter(ListNode* node, ListNode* at)
{
  STDEM_ASSERT(node->m_prev == 0);
  STDEM_ASSERT(node->m_next == 0);
  
  if(isEmpty())
    m_head = m_tail = node;
  else
  {
    STDEM_ASSERT(at != 0);
    
    node->m_prev = at;
    
    if(at->m_next != 0)
    {
      at->m_next -> m_prev = node;
      node->m_next = at->m_next;
    }
    else
      m_tail = node;
    
    at->m_next = node;
  }

//  ++m_size;
}

void List::remove(ListNode* node)
{
  STDEM_ASSERT(!isEmpty());

  if(node->m_prev != 0)
    node->m_prev->m_next = node->m_next;
  else
  {
    STDEM_ASSERT(m_head == node);
    m_head = node->m_next;
  }
  
  if(node->m_next != 0)
    node->m_next->m_prev = node->m_prev;
  else
  {
    STDEM_ASSERT(m_tail == node);
    m_tail = node->m_prev;
  }
  
  node->m_prev = 0;
  node->m_next = 0;
  
//  --m_size;
}

} // namespace StdEm
