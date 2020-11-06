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

#ifndef __LIBARCH_ARM_PAGING_H
#define __LIBARCH_ARM_PAGING_H

#include <Types.h>
#include "MemoryContext.h"
#include "MemoryMap.h"

/** Forward declaration */
class SplitAllocator;
class ARMFirstTable;

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libarch
 * @{
 *
 * @addtogroup libarch_arm
 * @{
 */

/**
 * ARM virtual memory implementation.
 */
class ARMPaging : public MemoryContext
{
  public:

    /**
     * Constructor.
     *
     * @param map Virtual memory map.
     * @param alloc Allocator pointer of the physical memory page allocations.
     */
    ARMPaging(MemoryMap *map, SplitAllocator *alloc);

    /**
     * Secondary constructor with pre-allocated 1st page table.
     *
     * @param map Virtual memory map
     * @param firstTableAddr Physical address of 1st page table
     */
    ARMPaging(MemoryMap *map, Address firstTableAddress, Address kernelBaseAddress);

    /**
     * Destructor.
     */
    virtual ~ARMPaging();

    /**
     * Activate the MemoryContext.
     *
     * This function applies this MemoryContext on the hardware MMU.
     *
     * @param initializeMMU If true perform (re)initialization of the MMU
     *
     * @return Result code.
     */
    virtual Result activate(bool initializeMMU = false);

    /**
     * Map a physical page to a virtual address.
     *
     * @param virt Virtual address.
     * @param phys Physical address.
     * @param access Memory access flags.
     *
     * @return Result code
     */
    virtual Result map(Address virt, Address phys, Memory::Access access);

    /**
     * Unmap a virtual address.
     *
     * This function removes a virtual to physical memory
     * mapping without deallocating any physical memory.
     *
     * @param virt Virtual address to unmap.
     *
     * @return Result code
     */
    virtual Result unmap(Address virt);

    /**
     * Translate virtual address to physical address.
     *
     * @param virt Virtual address to lookup on input, physical address on output.
     *
     * @return Result code
     */
    virtual Result lookup(Address virt, Address *phys) const;

    /**
     * Get Access flags for a virtual address.
     *
     * @param virt Virtual address to get Access flags for.
     * @param access MemoryAccess object pointer.
     *
     * @return Result code.
     */
    virtual Result access(Address addr, Memory::Access *access) const;

    /**
     * Release region of memory.
     *
     * @param region Memory region input
     * @param tablesOnly Set to true to only release page tables and not mapped pages.
     *
     * @return Result code.
     */
    virtual Result releaseRegion(MemoryMap::Region region, bool tablesOnly);

    /**
     * Release range of memory.
     *
     * @param range Memory range input
     * @param tablesOnly Set to true to only release page tables and not mapped pages.
     *
     * @return Result code.
     */
    virtual Result releaseRange(Memory::Range *range, bool tablesOnly);

  private:

    /**
     * Installs default mappings on 1st level page table
     *
     * @param map Virtual memory map
     * @param firstTableAddress Physical address of 1st level page table
     */
    void setupFirstTable(MemoryMap *map, Address firstTableAddress, Address kernelBaseAddress);

    /**
     * Enable the MMU
     *
     * @return Result code
     */
    Result enableMMU();

  private:

    /** Pointer to the first level page table. */
    ARMFirstTable *m_firstTable;

    /** Physical address of the first level page table. */
    Address m_firstTableAddr;

    /** Caching implementation */
    Arch::Cache m_cache;
};

namespace Arch
{
    typedef ARMPaging Memory;
};

/**
 * @}
 * @}
 * @}
 */

#endif /* __LIBARCH_ARM_PAGING_H */
