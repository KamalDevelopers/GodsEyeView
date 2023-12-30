#include "usb.hpp"
#include <LibC++/bitarray.hpp>

static usb_device usb_devices[MAX_USB_DEVICES];
static BitArray<MAX_USB_DEVICES> usb_device_bitmap;
static uint32_t devices_in_use;

usb_device* usb_allocate_device()
{
    int i = usb_device_bitmap.find_unset();
    usb_device_bitmap.bit_set(i);
    devices_in_use = (i == 0) ? 0 : devices_in_use;
    devices_in_use++;
    return &usb_devices[i];
}

void usb_free_device(usb_device* device)
{
    for (uint32_t i = 0; i < MAX_USB_DEVICES; i++) {
        if (&(usb_devices[i]) == device) {
            devices_in_use--;
            usb_device_bitmap.bit_clear(i);
            return;
        }
    }
}

uint32_t usb_devices_count()
{
    return devices_in_use;
}
