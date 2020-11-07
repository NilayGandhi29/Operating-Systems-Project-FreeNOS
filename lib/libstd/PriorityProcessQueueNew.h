

#define N 1000
#include "Types.h"
#include "Macros.h"
#include "Container.h"
#include "Log.h"
// #include "Process.h"
class PriorityProcessQueueNew
{
  public:

      

	PriorityProcessQueueNew();

	ProcessID pop(); 

	bool push(uint processID, uint m_count_p);

	void remove(uint processID);

	bool contains(uint processID) const; 

    uint get_m_count(ProcessID pid);

    ProcessID peep(); 	

	void check(ProcessID processID, uint m_count_p);

	Size count() const;

	Size size() const;

	void clear();

    void printQueue();

  private:

     /** The actual array where the data is stored. */
    ProcessID m_array[N];

    uint m_count_array[N];

    /** Head of the queue */
    uint m_head;

    /** Tail of the queue */
    uint m_tail;

    /** Number of items in the queue */
    uint m_count;


};

