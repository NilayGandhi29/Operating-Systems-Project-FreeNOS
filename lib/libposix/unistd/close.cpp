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

#include <FileDescriptor.h>
#include "errno.h"
#include "unistd.h"

int close(int fildes)
{
    FileDescriptor *files = getFiles();

    if (fildes >= FILE_DESCRIPTOR_MAX || fildes < 0)
    {
        errno = ERANGE;
        return -1;
    }

    if (!files[fildes].open)
    {
        errno = ENOENT;
        return -1;
    }

    files[fildes].open = false;
    return 0;
}