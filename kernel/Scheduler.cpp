/*
 * Copyright (C) 2015 Niek Linnenbank
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

#include <Log.h>
#include "Kernel.h"
#include "Scheduler.h"
#include <PriorityQueue.h>

Scheduler::Scheduler()
{
    m_pqueue.clear();
    DEBUG("");
}

Size Scheduler::count() const
{
    return m_pqueue.count();
}

Scheduler::Result Scheduler::enqueue(Process *proc, bool ignoreState)
{ 
    if (proc->getState() != Process::Ready && !ignoreState)
    {
        ERROR("process ID " << proc->getID() << " not in Ready state");
        return InvalidArgument;
    }

    m_pqueue.push(proc);
    return Success;
}

Scheduler::Result Scheduler::dequeue(Process *proc, bool ignoreState)
{
    if (proc->getState() == Process::Ready && !ignoreState)
    {
        ERROR("process ID " << proc->getID() << " is in Ready state");
        return InvalidArgument;
    }

    Size count = m_pqueue.count();

    // Traverse the Queue to remove the Process
    for (Size i = 0; i < count; i++)
    {
        Process *p = m_pqueue.pop();

        if (p == proc)
            return Success;
        else
            m_pqueue.push(p);
    }

    FATAL("process ID " << proc->getID() << " is not in the schedule");
    return InvalidArgument;
}

Process * Scheduler::select()
{
    if (m_pqueue.count() > 0)
    {
        Process *p = m_pqueue.getMin();
        
        p->up_cnt();
        m_pqueue.push(p);

        return p;
    }
    
    return (Process *) NULL;
}
