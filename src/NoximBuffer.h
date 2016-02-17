/*****************************************************************************

  NoximBuffer.h -- Buffer definition

 *****************************************************************************/

#ifndef __NoximBuffer_H__
#define __NoximBuffer_H__

//---------------------------------------------------------------------------

#include <cassert>
#include <queue>
#include "NoximMain.h"

using namespace std;

//---------------------------------------------------------------------------

class NoximBuffer
{
 public:

  NoximBuffer();

  virtual ~NoximBuffer() {}
  
  void SetMaxBufferSize(const unsigned int bms); // Set buffer max size (in flits)

  unsigned int GetMaxBufferSize() const; // Get max buffer size
  unsigned int getCurrentFreeSlots() const; // free buffer slots

  bool IsFull() const; // Returns true if buffer is full
  bool IsEmpty() const; // Returns true if buffer is empty

  virtual void Drop(const NoximFlit &flit) const; // Called by Push() when buffer is full

  virtual void Empty() const; // Called by Pop() when buffer is empty

  void Push(const NoximFlit &flit); // Push a flit. Calls Drop method if buffer is full.

  NoximFlit Pop(); // Pop a flit.

  NoximFlit Front() const; // Return a copy of the first flit in the buffer.

  unsigned int Size() const;

private:
  
  unsigned int max_buffer_size;
  queue<NoximFlit> buffer;
};

//---------------------------------------------------------------------------

#endif