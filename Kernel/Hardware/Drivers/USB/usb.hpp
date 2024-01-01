#ifndef USB_HPP
#define USB_HPP

#include <LibC/types.h>

#define MAX_USB_DEVICES 160
#define USB_CONTROLLER_NONE 0
#define USB_CONTROLLER_EHCI 1

typedef struct {
    uint8_t length;
    uint8_t type;

    uint16_t bcd_usb;
    uint8_t device_class;
    uint8_t device_subclass;
    uint8_t protocol;
    uint8_t max_packet_size;
    uint16_t vendor;
    uint16_t product;
    uint16_t bcd_device;

    uint8_t string_index_manufacturer;
    uint8_t string_index_product;
    uint8_t string_index_serial;
    uint8_t configurations;
} __attribute__((packed)) usb_device_descriptor;

typedef struct {
    uint8_t length;
    uint8_t type;

    uint16_t total_length;
    uint8_t interfaces;
    uint8_t configuration_value;
    uint8_t configuration;
    uint8_t attributes;
    uint8_t max_power;
} __attribute__((packed)) usb_conf_descriptor;

typedef struct {
    uint8_t length;
    uint8_t type;

    uint8_t interface_number;
    uint8_t alt_setting;
    uint8_t endpoints;

    uint8_t interface_class;
    uint8_t interface_subclass;
    uint8_t interface_protocol;
    uint8_t interface;
} __attribute__((packed)) usb_interface_descriptor;

typedef struct {
    uint8_t address;
    uint8_t protocol;
    uint8_t port;
    uint8_t ep_in;
    uint8_t ep_out;
    uint8_t controller;

    usb_device_descriptor device_descriptor;
} usb_device;

usb_device* usb_allocate_device();
void usb_free_device(usb_device* device);
uint32_t usb_devices_count();
usb_device* usb_device_at(uint32_t i);

#endif
