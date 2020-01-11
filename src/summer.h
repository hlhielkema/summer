#ifndef SUMMER_H
#define SUMMER_H

#include <linux/ioctl.h>

#define MAJOR_NUM   240
#define DEVICE_NAME "summer"
#define MAX_SESSIONS 100

#define SUMMER_SET _IOR(MAJOR_NUM, 0, int *)
#define SUMMER_GET _IOW(MAJOR_NUM, 1, int *)

/* Prototypes
*/
int device_open(struct inode *inode, struct file *file);
int device_close(struct inode *inode, struct file *file);
long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param);
int init_module(void);
void cleanup_module(void);
ssize_t device_read(struct file * filp, char * buffer, size_t length, loff_t *pos);
ssize_t device_write(struct file * filp, const char * buf, size_t count, loff_t *pos);
void int_to_char_array(int value, char * ch);
int char_array_to_int(const char * ch);

#endif

