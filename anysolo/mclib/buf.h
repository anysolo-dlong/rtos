#ifndef _anysolo_mclib_buf_h_
#define _anysolo_mclib_buf_h_

#include <anysolo/mclib/assert.h>
#include <string.h>


namespace mclib {

class Buf
{
  Byte* m_buf;
  int   m_size;

  void reset()                          { m_buf = 0; m_size = 0; }

public:
  Buf()                                 { reset(); }
  explicit Buf(int size)                { reset(); alloc(size); }
  Buf(const Buf& b2)                    { reset(); copy(b2); }
  ~Buf()                                { free(); }

  void alloc(int newSize);
  void realloc(int newSize);
  void free();

  void copy(const Buf& b2);

  void takeTo(Buf& b2);
  Byte* take();

  bool empty() const                    {return m_size == 0;}

  void clear()                          {free();}

  int size() const                      {return m_size;}

  Byte* buf()                           {return m_buf;}

  const Byte* buf() const               {return m_buf;}

  const Byte* ptr(int n) const          {O_ASSERT(n >=0 && n < size()); return m_buf + n;}
  Byte* ptr(int n)                      {O_ASSERT(n >=0 && n < size()); return m_buf + n;}

  const Byte& operator[] (int n) const  {return *ptr(n);}
  Byte& operator[] (int n)              {return *ptr(n);}
  Buf& operator= (const Buf& b2)        {copy(b2); return *this;}

  void fill(Byte b = 0)                 {memset(buf(), size(), b);}
};

} // namespace mclib


#endif // _anysolo_mclib_buf_h_
