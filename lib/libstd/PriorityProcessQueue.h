
#ifndef __LIBSTD_QUEUE_H
#define __LIBSTD_QUEUE_H

#include "Types.h"
#include "Macros.h"
#include "Container.h"
#include "Process.h"

class PriorityProcessQueue
{
  public:

	PriorityProcessQueue();

	Process * pop(); 

	bool push(Process *p);

	void remove(Process *p);

	bool contains(Process *p);  	

	void check();

	Size count();

	Size size();

	void clear();

  private:

    /** The actual array where the data is stored. */
    Process * m_array[N];

    /** Head of the queue */
    uint m_head;

    /** Tail of the queue */
    uint m_tail;

    /** Number of items in the queue */
    uint m_count;
}

#endif /* __LIBSTD_QUEUE_H */