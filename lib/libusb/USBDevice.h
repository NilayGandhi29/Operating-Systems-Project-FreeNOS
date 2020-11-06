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

#ifndef __LIBUSB_USBDEVICE_H
#define __LIBUSB_USBDEVICE_H

#include <Types.h>
#include <Device.h>
#include <Index.h>
#include <Array.h>
#include <Callback.h>
#include "USBTransfer.h"
#include "USBDescriptor.h"
#include "USBMessage.h"

/**
 * @addtogroup lib
 * @{
 *
 * @addtogroup libusb
 * @{
 */

/**
 * USB device class.
 */
class USBDevice : public Device
{
  private:

    static const Size MaxInterfaces = 64u;
    static const Size MaxEndpoints = 64u;
    static const Size MaxStrings = 64u;

  public:

    /**
     * Constructor
     */
    USBDevice(u8 deviceId, const char *usbPath = "/usb");

    /**
     * Destructor
     */
    virtual ~USBDevice();

    /**
     * Initialize the USBDevice.
     *
     * @return Result code
     */
    virtual FileSystem::Error initialize();

  protected:

    /**
     * Get device descriptor.
     *
     * @return Result code
     */
    FileSystem::Error getDeviceDescriptor(USBDescriptor::Device *desc,
                              Size size = sizeof(USBDescriptor::Device));

    /**
     * Get configuration descriptor.
     *
     * @return Result code
     */
    FileSystem::Error getConfigDescriptor(USBDescriptor::Configuration *desc,
                              Size size = sizeof(USBDescriptor::Configuration));

    /**
     * Get interface descriptor.
     *
     * @return Result code
     */
    FileSystem::Error getInterfaceDescriptor(USBDescriptor::Interface *desc);

    /**
     * Get endpoint descriptor.
     *
     * @return Result code
     */
    FileSystem::Error getEndpointDescriptor(u8 endpointId, USBDescriptor::Endpoint *desc);

    /**
     * Set device address.
     *
     * @return Result code
     */
    FileSystem::Error setAddress(u8 address);

    /**
     * Activate a configuration.
     *
     * @return Result code
     */
    FileSystem::Error setConfiguration(u8 configId);

    /**
     * Send a control message.
     *
     * @return Result code
     */
    FileSystem::Error controlMessage(u8 request,
                         const USBTransfer::Direction direction,
                         const USBTransfer::RequestType type,
                         const USBTransfer::Recipient recipient,
                         u16 value,
                         u16 index,
                         void *buffer,
                         Size size);

    /**
     * Start a synchronous USB transfer.
     *
     * @return Result code
     */
    FileSystem::Error transfer(const USBTransfer::Type type,
                   const USBTransfer::Direction direction,
                   Address endpointId,
                   void *buffer,
                   Size size,
                   Size maxPacketSize = 0);

    /**
     * Start an asynchronous USB transfer
     *
     * @return Result code
     */
    FileSystem::Error beginTransfer(const USBTransfer::Type type,
                        const USBTransfer::Direction direction,
                        Address endpointId,
                        void *buffer,
                        Size size,
                        Size maxPacketSize = 0,
                        CallbackFunction *callback = 0);

    /**
     * Complete asynchronous USB transfer
     *
     * @return Result code
     */
    FileSystem::Error finishTransfer(FileSystemMessage *msg);

    /**
     * Submit a USB transfer to the Host controller.
     *
     * @return Result code
     */
    FileSystem::Error submit(USBMessage & msg);

  protected:

    /** USB device identifier. */
    Address m_id;

    /** USB speed. */
    USBTransfer::Speed m_speed;

    /** USB transfer I/O file. */
    int m_transferFile;

    /** USB transfer I/O file path. */
    String m_transferFilePath;

    /** USB bus path. */
    String m_busPath;

    /** USB device descriptor */
    USBDescriptor::Device *m_device;

    /** USB configuration descriptor. */
    USBDescriptor::Configuration *m_config;

    /** USB interface descriptors. */
    Index<USBDescriptor::Interface, MaxInterfaces> m_interfaces;

    /** USB endpoint descriptors. */
    Index<USBDescriptor::Endpoint, MaxEndpoints> m_endpoints;

    /** USB string descriptor */
    Index<USBDescriptor::String, MaxStrings> m_strings;

    /**
     * Contains endpoint to saved packet id mapping.
     *
     * Some host controllers require the endpoint to save
     * the next packet identifier (Data0, Data1) for non-control transfers.
     */
    Array<u8, 128> m_endpointsPacketId;
};

/**
 * @}
 * @}
 */

#endif /* __LIBUSB_USBDEVICE_H */
