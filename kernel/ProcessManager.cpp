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

#include <FreeNOS/System.h>
#include <Log.h>
#include <ListIterator.h>
#include "Scheduler.h"
#include "ProcessEvent.h"
#include "ProcessManager.h"

ProcessManager::ProcessManager()
    : m_procs(MAX_PROCS)
    , m_interruptNotifyList(256)
{
    //WARNING("##############inside ProcessManager::ProcessManager");
    DEBUG("m_procs = " << MAX_PROCS);

    m_scheduler = new Scheduler();
    m_current   = ZERO;
    m_idle      = ZERO;
    m_interruptNotifyList.fill(ZERO);
}

ProcessManager::~ProcessManager()
{
    //WARNING("##############inside ProcessManager::~ProcessManager");
    if (m_scheduler != NULL)
    {
        //WARNING("##############inside if (m_scheduler != NULL)");
        delete m_scheduler;
    }
}
//WARNING("##############creating ProcessManager::create");
Process * ProcessManager::create(const Address entry,
                                 const MemoryMap &map,
                                 const bool readyToRun,
                                 const bool privileged)
{
    Process *proc = new Arch::Process(m_procs.count(), entry, privileged, map);

    // Insert to the process table
    if (proc && proc->initialize() == Process::Success)
    {
        //WARNING("##############creating if (proc && proc->initialize() == Process::Success)");
        m_procs.insert(proc);

        if (readyToRun)
        {
            wakeup(proc);
        }

        if (m_current != 0)
            proc->setParent(m_current->getID());

        return proc;
    }
    return ZERO;
}

Process * ProcessManager::get(const ProcessID id)
{
    //WARNING("##############inside ProcessManager::get()");
    Process **p = (Process **) m_procs.get(id);
    return p ? *p : ZERO;
}

void ProcessManager::remove(Process *proc, const uint exitStatus)
{
    //WARNING("##############inside ProcessManager::remove()");
    if (proc == m_idle)
        m_idle = ZERO;

    if (proc == m_current)
        m_current = ZERO;

    // Wakeup any Processes which are waiting for this Process
    const Size size = m_procs.size();

    for (Size i = 0; i < size; i++)
    {
        if (m_procs[i] != ZERO &&
            m_procs[i]->getState() == Process::Waiting &&
            m_procs[i]->getWait() == proc->getID())
        {
            m_procs[i]->setWaitResult(exitStatus);

            const Result result = wakeup(m_procs[i]);
            if (result != Success)
            {
                FATAL("failed to wakeup PID " << m_procs[i]->getID());
            }
        }
    }

    // Unregister any interrupt events for this process
    unregisterInterruptNotify(proc);

    // Remove process from administration and schedule
    m_procs[proc->getID()] = ZERO;

    if (proc->getState() == Process::Ready)
    {
        const Result result = dequeueProcess(proc, true);
        if (result != Success)
        {
            FATAL("failed to dequeue PID " << proc->getID());
        }
    }

    const Size countRemoved = m_sleepTimerQueue.remove(proc);
    assert(countRemoved <= 1U);
    (void) countRemoved;

    // Free the process memory
    delete proc;
}

ProcessManager::Result ProcessManager::schedule()
{
    //WARNING("##############inside ProcessManager::Result()");
    const Timer *timer = Kernel::instance()->getTimer();
    const Size sleepTimerCount = m_sleepTimerQueue.count();

    // Let the scheduler select a new process
    // NOTICE("########################Selected");
    Process *proc = m_scheduler->select();

    // If no process ready, let us idle
    if (!proc)
        proc = m_idle;

    if (!proc)
    {
        FATAL("no process found to run!");
    }

    // Try to wakeup processes that are waiting for a timer to expire
    for (Size i = 0; i < sleepTimerCount; i++)
    {
        Process *p = m_sleepTimerQueue.pop();
        const Timer::Info & procTimer = p->getSleepTimer();

        if (timer->isExpired(procTimer))
        {
            const Result result = wakeup(p);
            if (result != Success)
            {
                FATAL("failed to wakeup PID " << p->getID());
            }
        }
        else
        {
            m_sleepTimerQueue.push(p);
        }
    }

    // Only execute if its a different process
    if (proc != m_current)
    {
        Process *previous = m_current;
        m_current = proc;
        proc->execute(previous);
    }

    return Success;
}

Process * ProcessManager::current()
{
    //WARNING("##############inside ProcessManager::current()");
    return m_current;
}

void ProcessManager::setIdle(Process *proc)
{
    //WARNING("##############inside ProcessManager::setIdle()");
    const Result result = dequeueProcess(proc, true);
    if (result != Success)
    {
        FATAL("failed to dequeue PID " << proc->getID());
    }

    m_idle = proc;
}

Vector<Process *> * ProcessManager::getProcessTable()
{
    //WARNING("##############inside ProcessManager::getProcessTable()");
    return &m_procs;
}

ProcessManager::Result ProcessManager::wait(Process *proc)
{
    //WARNING("##############inside ProcessManager::wait()");
    if (m_current->wait(proc->getID()) != Process::Success)
    {
        //WARNING("##############inside if (m_current->wait(proc->getID()) != Process::Success)");
        ERROR("process ID " << m_current->getID() << " failed to wait");
        return IOError;
    }

    return dequeueProcess(m_current);
}

ProcessManager::Result ProcessManager::sleep(const Timer::Info *timer, const bool ignoreWakeups)
{
    //WARNING("##############inside ProcessManager::sleep()");
    const Process::Result result = m_current->sleep(timer, ignoreWakeups);
    switch (result)
    {
        case Process::WakeupPending:
            return WakeupPending;

        case Process::Success: {
            const Result res = dequeueProcess(m_current);
            if (res != Success)
            {
                FATAL("failed to dequeue PID " << m_current->getID());
            }

            if (timer)
            {
                assert(!m_sleepTimerQueue.contains(m_current));
                m_sleepTimerQueue.push(m_current);
            }
            break;
        }

        default:
            ERROR("failed to sleep process ID " << m_current->getID() <<
                  ": result: " << (uint) result);
            return IOError;
    }

    return Success;
}

ProcessManager::Result ProcessManager::wakeup(Process *proc)
{
    //WARNING("##############inside ProcessManager::result()");
    const Process::State state = proc->getState();
    const Process::Result result = proc->wakeup();

    if (result != Process::Success)
    {
        ERROR("failed to wakeup process ID " << proc->getID() <<
              ": result: " << (uint) result);
        return IOError;
    }

    if (state != Process::Ready)
    {
        return enqueueProcess(proc);
    }
    else
    {
        return Success;
    }
}

ProcessManager::Result ProcessManager::raiseEvent(Process *proc, const struct ProcessEvent *event)
{
    //WARNING("##############inside ProcessManager::raiseEvent()");
    const Process::State state = proc->getState();
    const Process::Result result = proc->raiseEvent(event);

    if (result != Process::Success)
    {
        ERROR("failed to raise event in process ID " << proc->getID() <<
              ": result: " << (uint) result);
        return IOError;
    }

    if (state != Process::Ready)
    {
        return enqueueProcess(proc);
    }
    else
    {
        return Success;
    }
}

ProcessManager::Result ProcessManager::registerInterruptNotify(Process *proc, const u32 vec)
{
    //WARNING("##############inside ProcessManager::registerInterruptNotify()");
    // Create List if necessary
    if (!m_interruptNotifyList[vec])
    {
        m_interruptNotifyList.insert(vec, new List<Process *>());
    }

    // Check for duplicates
    if (m_interruptNotifyList[vec]->contains(proc))
        return AlreadyExists;

    // Append the Process
    m_interruptNotifyList[vec]->append(proc);
    return Success;
}

ProcessManager::Result ProcessManager::unregisterInterruptNotify(Process *proc)
{
    //WARNING("##############inside ProcessManager::unregisterInterruptNotify()");
    // Remove the Process from all notify lists
    for (Size i = 0; i < m_interruptNotifyList.size(); i++)
    {
        List<Process *> *lst = m_interruptNotifyList[i];
        if (lst)
        {
            lst->remove(proc);
        }
    }

    return Success;
}

ProcessManager::Result ProcessManager::interruptNotify(const u32 vector)
{
    //WARNING("##############inside ProcessManager::interruptNotify()");
    List<Process *> *lst = m_interruptNotifyList[vector];
    if (lst)
    {
        ProcessEvent event;
        event.type   = InterruptEvent;
        event.number = vector;

        for (ListIterator<Process *> i(lst); i.hasCurrent(); i++)
        {
            if (raiseEvent(i.current(), &event) != Success)
            {
                ERROR("failed to raise InterruptEvent for IRQ #" << vector <<
                      " on Process ID " << i.current()->getID());
                return IOError;
            }
        }
    }

    return Success;
}

ProcessManager::Result ProcessManager::enqueueProcess(Process *proc, const bool ignoreState)
{
    //WARNING("##############inside ProcessManager::enqueueProcess()");
    if (m_scheduler->enqueue(proc, ignoreState) != Scheduler::Success)
    {
        ERROR("process ID " << proc->getID() << " not added to Scheduler");
        return IOError;
    }

    const Size countRemoved = m_sleepTimerQueue.remove(proc);
    assert(countRemoved <= 1U);
    (void) countRemoved;

    return Success;
}

ProcessManager::Result ProcessManager::dequeueProcess(Process *proc, const bool ignoreState) const
{
    //WARNING("##############inside ProcessManager::dequeueProcess()");
    if (m_scheduler->dequeue(proc, ignoreState) != Scheduler::Success)
    {
        ERROR("process ID " << proc->getID() << " not removed from Scheduler");
        return IOError;
    }

    return Success;
}
