#include "../base/buf.h"


namespace mclib {

void Buf::alloc(int newSize)
{
  if(size() >= newSize)
    return;

  free();

  m_buf = (newSize != 0) ? new Byte[newSize] : 0;
  m_size = newSize;
}

void Buf::free()
{
  if(m_buf != 0)
  {
    delete [] m_buf;

    m_buf  = 0;
    m_size = 0;
  }
}

void Buf::copy(const Buf& b2)
{
  alloc(b2.m_size);
  m_size = b2.m_size;
  memcpy(m_buf, b2.m_buf, size());
}

void Buf::takeTo(Buf& b2)
{
  b2.m_buf = m_buf;
  b2.m_size = m_size;

  m_buf = 0;
  m_size = 0;
}

Byte* Buf::take()
{
  Byte* b = m_buf;

  m_buf  = 0;
  m_size = 0;

  return b;
}

void Buf::realloc(int newSize)
{
  Buf savedData;
  takeTo(savedData);
  
  alloc(newSize);
  memcpy(m_buf, savedData.buf(), min(size(), savedData.size()));
}

} // namespace mclib
