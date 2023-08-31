#!/bin/bash

# Change these variables to match your project setup
module_name="usb_driver.ko"

# Step 1: Clean the project
make clean
if [ $? -ne 0 ]; then
    echo "Error in 'make clean'"
    exit 1
fi

# Step 2: Build the project
make all
if [ $? -ne 0 ]; then
    echo "Error in 'make all'"
    exit 1
fi

# Step 3: Check if the module is currently loaded
if lsmod | grep "usb_driver"; then
    # Step 4: Remove the kernel module
    sudo rmmod "$module_name"
    if [ $? -ne 0 ]; then
        echo "Error in 'sudo rmmod $module_name'"
        exit 1
    fi
fi

# Step 4: Insert the kernel module
sudo insmod "$module_name"
if [ $? -ne 0 ]; then
    echo "Error in 'sudo insmod $module_name'"
    exit 1
fi

echo "Script completed successfully."
