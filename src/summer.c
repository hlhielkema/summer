/* 
*
*  Author: Hielke Hielkema
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "summer.h"

/* the memory of the device (all static)
*/
static int total;
static int sessions[MAX_SESSIONS];
static int session_count;

/* called after the device is opened
*/
int device_open(struct inode *inode, struct file *file)
{
  printk("\nsummer device is open\n");
  total = 0;
  return 0;
}

/* called after the device is closed
*/
int device_close(struct inode *inode, struct file *file)
{
  if (total > 0 && session_count < MAX_SESSIONS)
  {
    if (session_count == 0)
      sessions[session_count] = total;
    else
      sessions[session_count] = sessions[session_count - 1] + total;
    session_count++;
  }

  printk("\nsummer device is closed\n");
  return 0;
}

/* handling of ioctl events
*/
long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
  
  int number;
  switch(ioctl_num)
  {
    case SUMMER_SET:
      __get_user(number, (int*)ioctl_param);
      total += number;
      break;
    case SUMMER_GET:
      __put_user(total, (int*)ioctl_param);
      break;
  }
  return 0;
}

/* table of event handlers of the device
*/
struct file_operations fops =
{
  open   : device_open,
  release: device_close,
  read : device_read,
  write : device_write,
  unlocked_ioctl : device_ioctl
};

/* called after the kernelmodule is opened
*/
int init_module(void)
{
/* register the device
  ** after registration the device is known to linux by its major number
  ** example: mknod /dev/summer0 c 240 0
  ** this creates the device /dev/summer0
  ** which is a character device (c) with major number 240 and minor number 0
  */
  int retval = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops);
  if(retval < 0)
  {
    printk("character device not registered\n");
    return retval;
  }
  printk("summer kernel module loaded\n");

  // Init the sessions registration
  session_count = 0;
  sessions[0] = 0;

  return 0;
}

/* called after the module is closed
*/
void cleanup_module(void)
{
/* unregister the device
  */
  unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
  printk("summer kernel module unloaded\n");
}

/* Read from the device
*/
ssize_t device_read(struct file * filp, char * buffer, size_t length, loff_t *pos)
{
  int i, j;
  int bytes_read = 0;

  // Loop trough the sessions
  for(i = 0; i < session_count && length; i++)
  {
     // Create a string buffer to store the text version of the number in
     char str[10];

     // Convert the 
     int_to_char_array(sessions[i], (char *)&str);

     // Add ", " for a beter overview
     for(j = 0; str[j] != '\0'; j++);
     str[j++] = ',';
     str[j++] = ' ';
     str[j] = '\0';

     // Add the charachters to the buffer
     for(j = 0; str[j] != '\0' && length;j++)
     {
       put_user(str[j], buffer++);
       length--;
       bytes_read++;
     }
  }

  // Reset the sessions
  session_count = 0;
  sessions[0] = 0;  
  total = 0;
  
  return bytes_read;
}

/* Write to the device
*/ 
ssize_t device_write(struct file *filp, const char * buffer, size_t count, loff_t *pos)
{
   char local_buf[100];
   int i, value;

   // Copy the data from the user memory to the kernel memory
   for(i = 0; i < 99; i++)
   {
      __get_user(local_buf[i], (char *)(&buffer[i]));      
      if (local_buf[i] == '\0')
        break;
   }
   local_buf[99] = '\0';

   // Try to parse the char*
   value = char_array_to_int((char*)&local_buf);

   printk("Parse result:%d", value);

   // Check if the parsing worked and use the variable
   if(value != -1)
     total = value;

   // return 10, this indicates that the device can read more numbers
   return i;
}

/* Convert an integer to a char array
*  
*  For example:
*  452 -> "542"
*/
void int_to_char_array(int value, char * ch)
{
   char dec[10];
   int dec_c = 0;
   if (value == 0)
   {
     ch[0] = '0';
     ch[1] = '\0';
   }  
   else
   {
     while(value > 0)
     {
       dec[dec_c++] = (char)(((int)'0')+(value % 10));
       value /= 10;
     }
     while(dec_c > 0)
       *(ch++) = dec[--dec_c];
     *(ch++) = '\0';
   }
}

/* Parse an integer from a char array
*
*  For example:
*  "419" -> 419
*
*  returns result or -1 in case of a parse error
*/
int char_array_to_int(const char * ch)
{
   int i, value; 
   if (ch[0] >= '0' && ch[0] <= '9')
   {
     value = ch[0];
     for(i = 1; ch[i] != '\0' && ch[i] != '\n'; i++)
     {
        // validate the char
        if (ch[i] >= '0' && ch[i] <= '9')
        {
          value *= 10;
          value += (char)((int)ch[i] - (int)'0');
        }
        else
          return -1; // char out of range
     }
     return value;
   }
   else
     return -1; // char out of range
}


















