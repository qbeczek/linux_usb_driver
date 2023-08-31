#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/usb.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("KUBA");
MODULE_DESCRIPTION("Driver for stm32 usb device");

#define VENDOR_ID 0x2312
#define PRODUCT_ID 0xec40
#undef CTRL_MSG_TEST

static struct proc_dir_entry *proc_file;
static struct usb_device *usb_dev;

/**
 * @brief Read data out of the buffer
 */
static ssize_t my_read(struct file *File, char *user_buffer, size_t count,
                       loff_t *offs) {
    char text[32];
    int to_copy, not_copied, delta, status;
    int transferred;
    // uint8_t val;
    uint8_t *bulk_buffer;
    /* Get amount of data copy */
    to_copy = min(count, sizeof(text));

    /* Read from USB Device */
    // status =
    //     usb_control_msg_recv(usb_dev, usb_rcvctrlpipe(usb_dev, 0), 0x5c,
    //     0xc0,
    //                          0, 0, (unsigned char *)&val, 1, 100,
    //                          GFP_KERNEL);
    usb_clear_halt(usb_dev, usb_rcvbulkpipe(usb_dev, 1));

    bulk_buffer = kzalloc(64, GFP_KERNEL);
    if (!bulk_buffer) return -ENOMEM;

    status = usb_bulk_msg(usb_dev, usb_rcvbulkpipe(usb_dev, 1), bulk_buffer, 64,
                          &transferred, 100);
    if (status < 0) {
        printk("usb_driver(%d) - error during bulk message\n", status);
        return -1;
    }

    printk("usb_driver: receive transferred bits: %d\n", transferred);
    printk("%d", bulk_buffer[0]);

    sprintf(text, "0x%x\n", bulk_buffer[0]);

    printk("usb_driver: reading value: %s - %d\n", (text), bulk_buffer[0]);

    /* Copy bulk_buffer to user */
    not_copied = copy_to_user(user_buffer, text, to_copy);

    /* Calculate data */
    delta = to_copy - not_copied;

    kfree(bulk_buffer);

    return delta;
}

/**
 * @brief Write data to buffer
 */
static ssize_t my_write(struct file *File, const char *user_buffer,
                        size_t count, loff_t *offs) {
    char text[255];
    int to_copy, not_copied, delta, status;
    int transferred;
    long val;
    uint16_t seg_val;
    /* Clear text */
    memset(text, 0, sizeof(text));

    /* Get amount of data to copy */
    to_copy = min(count, sizeof(text));

    /* Copy data to user */
    not_copied = copy_from_user(text, user_buffer, to_copy);
    if (0 != kstrtol(text, 0, &val)) {
        printk("usb_driver: Error converting input\n");
        return -1;
    }

    seg_val = (uint16_t)val;
    printk("usb_driver: send value: %d\n", seg_val);
    // status = usb_control_msg(usb_dev, usb_sndctrlpipe(usb_dev, 0), 0x5b,
    // 0x40,
    //                          seg_val, 0, NULL, 0, 100);
    status = usb_bulk_msg(usb_dev, usb_sndbulkpipe(usb_dev, 1), &seg_val, 8,
                          &transferred, 100);
    if (status < 0) {
        printk("usb_driver (%d) - error during bulk message\n", status);
        return -1;
    }

    /* Calculate data */
    delta = to_copy - not_copied;

    return delta;
}

static struct proc_ops fops = {
    .proc_read = my_read,
    .proc_write = my_write,
};

static struct usb_device_id my_usb_array[] = {
    {USB_DEVICE(VENDOR_ID, PRODUCT_ID)},
    {},
};

MODULE_DEVICE_TABLE(usb, my_usb_array);

static int my_usb_probe(struct usb_interface *intf,
                        const struct usb_device_id *id) {
    struct usb_endpoint_descriptor *ep_desc_in;
    struct usb_endpoint_descriptor *ep_desc_out;

    uint8_t endpoint_in_number;
    uint8_t endpoint_out_number;
    uint8_t endpoint_in_direction;
    uint8_t endpoint_out_direction;

    usb_dev = interface_to_usbdev(intf);

    if (usb_dev == NULL) {
        printk("usb_driver: Error when usb_dev");
        return -1;
    }

    printk("usb_driver: Probe function\n");
    printk("usb_driver: Device VID: %x, PID: %x", id->idVendor, id->idProduct);
    printk("usb_driver: Devnum: %d, Product: %s", usb_dev->devnum,
           usb_dev->product);

    ep_desc_in = &usb_dev->ep_in[1]->desc;
    ep_desc_out = &usb_dev->ep_out[1]->desc;

    endpoint_in_number = ep_desc_in->bEndpointAddress & 0x0F;
    endpoint_out_number = ep_desc_out->bEndpointAddress & 0x0F;
    endpoint_in_direction = (ep_desc_in->bEndpointAddress >> 7) & 0x01;
    endpoint_out_direction = (ep_desc_out->bEndpointAddress >> 7) & 0x01;

    printk("usb_driver: Endpoint out addres: %02x\n",
           ep_desc_out->bEndpointAddress);
    printk("usb_driver: Endpoint Number: %u\n", endpoint_out_number);
    printk("usb_driver: Endpoint Direction (IN=1, OUT=0): %u\n",
           endpoint_out_direction);
    printk("usb_driver: Endpoint in addres: %02x\n",
           ep_desc_in->bEndpointAddress);
    printk("usb_driver: Endpoint Number: %u\n", endpoint_in_number);
    printk("usb_driver: Endpoint Direction (IN=1, OUT=0): %u\n",
           endpoint_in_direction);
    usb_clear_halt(usb_dev, usb_rcvbulkpipe(usb_dev, 1));

#ifdef CTRL_MSG_TEST
    int status = usb_control_msg(usb_dev, 0, 0x5b, 0x40, 0, 0, "Siema", 4, 0);
    if (status < 0) {
        printk("usb_driver: error %d", status);
    }

    status =
        usb_control_msg_recv(usb_dev, 0, 0x5c, USB_DIR_IN | USB_TYPE_VENDOR,
                             0xc0, 0, data, sizeof(data), 1000, GFP_KERNEL);
    if (status < 0) {
        printk("usb_driver: error %d", status);
    }
    printk("usb_driver: data received: %s", data);
#endif

    proc_file = proc_create("usb_driver", 0666, NULL, &fops);
    if (proc_file == NULL) {
        printk("usb_driver: Error creating /proc/usb_driver\n");
        return -ENOMEM;
    }

    return 0;
}

static void my_usb_disconnect(struct usb_interface *intf) {
    proc_remove(proc_file);
    printk("usb_driver: remove proc/usb_driver file\n");

    printk("usb_driver: disconnect function\n");
}

static struct usb_driver my_usb_driver = {
    .name = "usb_driver",
    .id_table = my_usb_array,
    .probe = my_usb_probe,
    .disconnect = my_usb_disconnect,
};

static int __init ModuleInit(void) {
    int result;
    printk("usb_driver: ModuleInit execute\n");
    result = usb_register(&my_usb_driver);
    if (result) {
        printk("usb_driver: init error\n");
        return -result;
    }
    printk("usb_driver: end init function\n");
    return 0;
}

static void __exit ModuleExit(void) {
    proc_remove(proc_file);
    printk("usb_driver: remove proc/usb_driver file\n");
    printk("usb_driver: ModuleExit execute\n");
    usb_deregister(&my_usb_driver);
}

module_init(ModuleInit);
module_exit(ModuleExit);
