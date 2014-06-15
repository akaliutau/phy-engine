

#include "CSList.H"

CISLink* CSList_Base::Remove()
{
  // Remove first element and return it 

  if(m_Last == NULL)
  {
    // Not really an error.
    // Error("CList_Base::Remove", "Remove from empty list");
    return NULL;
  }

  CISLink* first = m_Last->m_Next;

  if(first == m_Last)
  {
    m_Last = NULL;
  }
  else
  {
    m_Last->m_Next = first->m_Next;
  }

  return first;
}
