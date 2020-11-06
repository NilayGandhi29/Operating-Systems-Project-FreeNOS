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

#include <KernelLog.h>
#include <DeviceServer.h>
#include "ATAController.h"
#include <Types.h>

int main(int argc, char **argv)
{
    KernelLog log;
    DeviceServer server("/dev/ata");
    server.registerDevice(new ATAController, "ata0");

    // Initialize
    const FileSystem::Result result = server.initialize();
    if (result != FileSystem::Success)
    {
        ERROR("failed to initialize: result = " << (int) result);
        return 1;
    }

    // Start serving requests
    return server.run();
}

ATAController::ATAController()
    : Device(FileSystem::BlockDeviceFile)
{
    m_identifier << "ata0";
}

FileSystem::Error ATAController::initialize()
{
    ATADrive *drive;

    // Detect ATA Controller
    if (m_io.inb(ATA_BASE_CMD0 + ATA_REG_STATUS) == 0xff)
    {
        return FileSystem::NotSupported;
    }
    pollReady(true);

    // Attempt to detect first drive
    m_io.outb(ATA_BASE_CMD0 + ATA_REG_SELECT, ATA_SEL_MASTER);
    pollReady(true);
    m_io.outb(ATA_BASE_CMD0 + ATA_REG_CMD,    ATA_CMD_IDENTIFY);

    switch (m_io.inb(ATA_BASE_CMD0 + ATA_REG_STATUS))
    {
    case 0:
        NOTICE("No ATA drive(s) detected");
        break;

    default:
        // Wait until the command completed
        pollReady();

        // Allocate a new drive
        drive = new ATADrive;
        drives.append(drive);

        // Read IDENTIFY data
        for (int i = 0; i < 256; i++)
        {
            ((u16 *) &drive->identity)[i] = m_io.inw(ATA_BASE_CMD0 + ATA_REG_DATA);
        }

        // Fixup ASCII bytes
        IDENTIFY_TEXT_SWAP(drive->identity.firmware, 8);
        IDENTIFY_TEXT_SWAP(drive->identity.serial, 20);
        IDENTIFY_TEXT_SWAP(drive->identity.model, 40);

        // Print out information
        NOTICE("ATA drive detected: SERIAL=" << drive->identity.serial <<
               " FIRMWARE=" << drive->identity.firmware <<
               " MODEL=" << drive->identity.model <<
               " MAJOR=" << drive->identity.majorRevision <<
               " MINOR=" << drive->identity.minorRevision <<
               " SECTORS=" << drive->identity.sectors28);
        break;
    }

    return FileSystem::Success;
}

FileSystem::Error ATAController::read(IOBuffer & buffer, Size size, Size offset)
{
    u8 sectors = CEIL(size, 512);
    u16 block[256];
    u32 lba     = offset / 512;
    Size result = 0;

    // Verify LBA
    if (drives.isEmpty() || drives.first()->identity.sectors28 < lba)
    {
        return FileSystem::IOError;
    }

    // Perform ATA Read Command
    m_io.outb(ATA_BASE_CMD0 + ATA_REG_SELECT, ATA_SEL_MASTER_28);
    m_io.outb(ATA_BASE_CMD0 + ATA_REG_COUNT,  sectors);
    m_io.outb(ATA_BASE_CMD0 + ATA_REG_ADDR0,  (lba) & 0xff);
    m_io.outb(ATA_BASE_CMD0 + ATA_REG_ADDR1,  (lba >> 8) & 0xff);
    m_io.outb(ATA_BASE_CMD0 + ATA_REG_ADDR2,  (lba >> 16) & 0xff);
    m_io.outb(ATA_BASE_CMD0 + ATA_REG_CMD,    ATA_CMD_READ);

    // Read out all requested sectors
    while(result < size)
    {
        // Poll the status register
        pollReady(true);

        // Read out bytes
        for (int i = 0; i < 256; i++)
        {
            block[i] = m_io.inw(ATA_BASE_CMD0 + ATA_REG_DATA);
        }

        // Calculate maximum bytes
        Size bytes = (size - result) < 512 - (offset % 512) ?
                     (size - result) : 512 - (offset % 512);

        // Copy to buffer
        buffer.bufferedWrite(((u8 *)block) + (offset % 512), bytes);

        // Update state
        result += bytes;
        offset += bytes;
    }

    return result;
}

FileSystem::Error ATAController::interrupt(Size vector)
{
    INFO("ATA interrupted on IRQ " << vector);
    return FileSystem::Success;
}

void ATAController::pollReady(bool noData)
{
    while (true)
    {
        u8 status = m_io.inb(ATA_BASE_CMD0 + ATA_REG_STATUS);

        if (!(status & ATA_STATUS_BUSY) &&
             (status & ATA_STATUS_DATA || noData))
        {
            break;
        }
    }
}
