# Custom Linux USB Driver for Zephyr OS Firmware

This repository contains a Linux USB driver that works with custom firmware written on Zephyr OS. The driver communicates with a USB device running the Zephyr OS firmware, allowing for message exchange using various methods such as bulk transfer and control messages. The driver module can be compiled using the provided Makefile.

## Requirements

- Linux operating system
- Custom firmware running on a USB device with the following parameters:
  - Vendor ID: 0x2312
  - Product ID: 0xec40 (modifiable)
  
## Compilation

To compile the driver module, follow these steps:

1. Clean the project: `make clean`
2. Build the project: `make`

## Usage

1. Check if the module is currently loaded: `lsmod | grep usb_driver`
2. Insert the kernel module: `sudo insmod usb_driver.ko`
3. Alternatively, you can use the provided script to install the driver: `sudo ./install_module`

### Removing the Module

To remove the module, use the following command:

```bash
sudo rmmod usb_driver.ko
```
### Communication with Zephyr OS Firmware

The driver facilitates communication with the Zephyr OS firmware USB device through various methods:

* Proc File: Messages can be sent to the device and read from the device using the /proc/usb_driver file. This is achieved through the write and read functions.

* Bulk Transfer: Bulk transfer is used for efficient data exchange between the driver and the device.

* Control Messages: The driver supports communication with the device via control messages.

### Installation Verification

After installation, run the following command to check the messages in the kernel log:

```bash
sudo dmesg
```
You should see messages similar to the following:
```bash
[63191.624336] usb_driver: ModuleInit execute
[63191.624388] usbcore: registered new interface driver usb_driver
[63191.624390] usb_driver: end init function
````

And then plug the usb deivce and you can see information about device in the kernel log:
```bash
[63091.480652] usb_driver: Probe function
[63091.480658] usb_driver: Device VID: 2312, PID: ec40
[63091.480663] usb_driver: Devnum: 86, Product: USB Non Standard Class
[63091.480667] usb_driver: Endpoint out addres: 01
[63091.480671] usb_driver: Endpoint Number: 1
[63091.480674] usb_driver: Endpoint Direction (IN=1, OUT=0): 0
[63091.480678] usb_driver: Endpoint in addres: 81
[63091.480681] usb_driver: Endpoint Number: 1
[63091.480683] usb_driver: Endpoint Direction (IN=1, OUT=0): 1
```