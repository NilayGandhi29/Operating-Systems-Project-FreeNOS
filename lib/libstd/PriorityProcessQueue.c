/*
 * Copyright (C) 2019 Niek Linnenbank
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libstd
 * @{
 */

/**
 * Array of items as a First-In-First-Out (FIFO) datastructure.
 */


    
    /**
     * Default constructor
     */
    PriorityProcessQueue::PriorityProcessQueue()
    {
        clear();
    }

    /**
     * Add item to the head of the Queue.
     *
     * @param item The item to add
     *
     * @return True if successful, false otherwise
     */
    


    bool PriorityProcessQueue::push(const T & item){
        if (m_tail >= N - 1)
        {
            NOTICE("Queue overflow no more elements can be inserted");
            return;
        }
        if ((m_head == -1) && (m_tail == -1))
        {
            m_head++;
            m_tail++;
            m_array[m_tail] = item;
            return;
        }    
        else{
            check(item);
        }
        m_tail++;
    }

    void PriorityProcessQueue::check(const T & data)
    {
        uint i,j;
     
        for (i = 0; i <= rear; i++)
        {
            if (data >= m_array[i])
            {
                for (j = rear + 1; j > i; j--)
                {
                    m_array[j] = m_array[j - 1];
                }
                m_array[i] = data;
                return;
            }
        }
        m_array[i] = data;
    }

    /**
     * Remove item from the tail of the Queue.
     *
     * @return Item T
     *
     * @note Do not call this function if the Queue is empty
     */
    


    T & PriorityProcessQueue::pop(){
        uint idx = m_tail;
        m_tail = (m_tail + 1) % N;
        m_count--;

        return m_array[idx];
    }

    T & PriorityProcessQueue::remove(const T & item){
        uint i;
 
        if ((m_head==-1) && (m_tail==-1))
        {
            NOTICE("Queue is empty no elements to delete");
            return;
        }
     
        for (i = 0; i <= m_tail; i++)
        {
            if (item == m_array[i])
            {
                for (; i < m_tail; i++)
                {
                    m_array[i] = m_array[i + 1];
                }
     
            m_array[i] = -99;
            m_tail--;
     
            if (m_tail == -1) 
                m_head = -1;
            return;
            }
        }
        NOTICE("Not found in queue to delete: " << item);
    }

    



    /**
     * Look if an item exists on the Queue
     *
     * @param item Item reference
     *
     * @return True if the item exists, false otherwise
     */
    bool PriorityProcessQueue::contains(const T & item) const
    {
        for (Size i = 0; i < m_count; i++)
        {
            if (m_array[(m_tail + i) % N] == item)
                return true;
        }

        return false;
    }

    /**
     * Remove all items with the given value.
     *
     * @param value Value to remove.
     *
     * @return Number of items removed.
     */


    /**
     * Removes all items from the Queue.
     */
    virtual void PriorityProcessQueue::clear()
    {
        m_head = 0;
        m_tail = 0;
        m_count = 0;
    }

    /**
     * Returns the maximum size of this Queue.
     *
     * @return size The maximum size of the Queue.
     */
    virtual Size PriorityProcessQueue::size() const
    {
        return N;
    }

    /**
     * Returns the number of items in the Queue.
     *
     * @return Number of items in the Queue.
     */
    virtual Size PriorityProcessQueue::count() const
    {
        return m_count;
    }

  


/**
 * @}
 * @}
 */

#endif /* __LIBSTD_QUEUE_H */
