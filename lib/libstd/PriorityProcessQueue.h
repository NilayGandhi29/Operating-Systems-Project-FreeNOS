
#ifndef __LIBSTD_PRIQUEUE_H
#define __LIBSTD_PRIQUEUE_H
#define N 1000
#include "Types.h"
#include "Macros.h"
#include "Container.h"
#include "Log.h"
#include "Process.h"
class PriorityProcessQueue
{
  public:

	PriorityProcessQueue();

	Process* pop(); 

	bool push(Process* p);

	void remove(Process* p);

	bool contains(Process* p);  	

	void check();

	Size count() const;

	Size size() const;

	void clear();

  private:

    /** The actual array where the data is stored. */

    Process* m_array[N];

    /** Head of the queue */
    uint m_head;

    /** Tail of the queue */
    uint m_tail;

    /** Number of items in the queue */
    uint m_count;
}

#endif /* __LIBSTD_PRIQUEUE_H */