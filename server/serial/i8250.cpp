/*
 * Copyright (C) 2009 Niek Linnenbank
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

#include <FreeNOS/User.h>
#include <FreeNOS/Config.h>
#include <Macros.h>
#include <Types.h>
#include "i8250.h"

i8250::i8250(const u16 base, const u16 irq)
    : Device(FileSystem::CharacterDeviceFile)
    , m_irq(irq)
{
    m_io.setPortBase(base);
    m_identifier << "serial0";
}

FileSystem::Error i8250::initialize()
{
    // 8bit Words, no parity
    m_io.outb(LINECONTROL, 3);

    // Enable interrupts
    m_io.outb(IRQCONTROL, 1);

    // No FIFO
    m_io.outb(FIFOCONTROL, 0);

    // Data Ready, Request to Send
    m_io.outb(MODEMCONTROL, 3);

    // Set baudrate
    m_io.outb(LINECONTROL, m_io.inb(LINECONTROL) | DLAB);
    m_io.outb(DIVISORLOW,  (11500 / BAUDRATE) & 0xff);
    m_io.outb(DIVISORHIGH, (11500 / BAUDRATE) >> 8);
    m_io.outb(LINECONTROL, m_io.inb(LINECONTROL) & ~(DLAB));

    // Done
    return FileSystem::Success;
}

FileSystem::Error i8250::interrupt(Size vector)
{
    ProcessCtl(SELF, EnableIRQ, m_irq);
    return FileSystem::Success;
}

FileSystem::Error i8250::read(IOBuffer & buffer, Size size, Size offset)
{
    Size bytes = 0;
    u8 byte;

    // Read as much bytes as possible
    while (m_io.inb(LINESTATUS) & RXREADY && bytes < size)
    {
        byte = m_io.inb(RECEIVE);
        buffer.bufferedWrite(&byte, 1);
        bytes++;
    }

    if (bytes)
        return (FileSystem::Error) bytes;
    else
        return FileSystem::RetryAgain;
}

FileSystem::Error i8250::write(IOBuffer & buffer, Size size, Size offset)
{
    Size bytes = 0;

    // Write as much bytes as possible
    while (m_io.inb(LINESTATUS) & TXREADY && bytes < size)
    {
        m_io.outb(TRANSMIT, buffer[bytes++]);
    }

    if (bytes)
        return (FileSystem::Error) bytes;
    else
        return FileSystem::RetryAgain;
}
