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
// #include "PriorityProcessQueueNew.h"

Scheduler::Scheduler()
{
    //NOTICE("##############inside Scheduler::Scheduler()");
    DEBUG("");
}

Size Scheduler::count() const
{
    //NOTICE("##############inside Scheduler::count()");
    return m_queue.count();
}

Scheduler::Result Scheduler::enqueue(Process *proc, bool ignoreState)
{
    //NOTICE("##############inside Scheduler::enqueue()");
    if (proc->getState() != Process::Ready && !ignoreState)
    {
        // //NOTICE("##############inside if (proc->getState() != Process::Ready && !ignoreState)");
        ERROR("process ID " << proc->getID() << " not in Ready state");
        return InvalidArgument;
    }
    NOTICE("Scheduler proc->getID: "<<(ProcessID) proc->getID()<<"\n");
    m_queue.push(proc->getID(),proc->m_count);
    process_queue[m_queue.count()-1]=proc;
    return Success;
}

Scheduler::Result Scheduler::dequeue(Process *proc, bool ignoreState)
{
    // //NOTICE("##############inside Scheduler::dequeue()");
    if (proc->getState() == Process::Ready && !ignoreState)
    {
        ERROR("process ID " << proc->getID() << " is in Ready state");
        return InvalidArgument;
    }

    Size count = m_queue.count();

    // Traverse the Queue to remove the Process
    for (Size i = 0; i < count; i++)
    {
        // Process *p = m_queue.pop();
        ProcessID pid_peep=m_queue.peep();
        uint m_count_pid=m_queue.get_m_count(pid_peep);
        ProcessID pid= m_queue.pop();
        Process* p=process_queue.pop();
        
        if (pid == proc->getID())
            return Success;
        else{
            m_queue.push(pid, m_count_pid);
            process_queue[m_queue.count()-1]=p;
        }
            

    }

    FATAL("process ID " << proc->getID() << " is not in the schedule");
    return InvalidArgument;
}

Process * Scheduler::select()
{
    
    if (m_queue.count() > 0)
    {
        // NOTICE("##############inside Scheduler::select()");
        m_queue.printQueue();

        ProcessID pid_peep=m_queue.peep();
        uint m_count_pid=m_queue.get_m_count(pid_peep);
        
        ProcessID pid = m_queue.pop();

        NOTICE("pid: " << pid <<"\n");

        Process* p=process_queue.getValue(pid);

        // Process* p = process_queue.pop();

        NOTICE("Process State: "<< (uint) p->getState()<<"\n");
        NOTICE("p->getID(): " << p->getID()<<"\n");
        // NOTICE("#############P_count: " << p->m_count);
        NOTICE("m_count_pid: "<<m_count_pid<<"\n");
        NOTICE("Before p->m_count: " << p->m_count<<"\n");

        p->m_count++;
        // m_count_pid++;
        m_queue.push(p->getID(), p->m_count);
        process_queue.push(p);

        m_queue.printQueue();
        
        return p;
    }

    return (Process *) NULL;
}

Process* getValue(uint ind){
    uint i;
    for(i=0; i<=m_queue.count(); i++){
        if(process_queue[i].getID()==ind){
            return process_queue[i];
        }
    }
    return NULL;
}
