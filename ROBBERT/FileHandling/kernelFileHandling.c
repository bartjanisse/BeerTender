#include <linux/kernel.h> 
#include <linux/module.h> 
#include <linux/fs.h> 
#include <asm/uaccess.h> 
#include <linux/mm.h>


void fileread(const char * filename)
{
  struct file *filp;
  struct inode *inode;
  mm_segment_t fs;
  off_t fsize;
  char *buf;
  unsigned long magic;
  printk("<1>start....\n");
  filp=filp_open(filename,o_rdonly,0);
  inode=filp->f_dentry->d_inode;

  magic=inode->i_sb->s_magic;
  printk("<1>file system magic:%li \n",magic);
  printk("<1>super blocksize:%li \n",inode->i_sb->s_blocksize);
  printk("<1>inode %li \n",inode->i_ino);
  fsize=inode->i_size;
  printk("<1>file size:%i \n",(int)fsize);
  buf=(char *) kmalloc(fsize+1,gfp_atomic);

  fs=get_fs();
  set_fs(kernel_ds);
  filp->f_op->read(filp,buf,fsize,&(filp->f_pos));
  set_fs(fs);

  buf[fsize]='\0';
  printk("<1>the file content is:\n");
  printk("<1>%s",buf);


  filp_close(filp,null);
}

void filewrite(char* filename, char* data)
{
  struct file *filp;
  mm_segment_t fs;
  filp = filp_open(filename, o_rdwr|o_append, 0644);
  if(is_err(filp))
  {
    printk("open error...\n");
    return;
  }

  fs=get_fs();
  set_fs(kernel_ds);
  filp->f_op->write(filp, data, strlen(data),&filp->f_pos);
  set_fs(fs);
  filp_close(filp,null);
}

int init_module()
{
  char *filename="/root/test1.c";

  printk("<1>read file from kernel.\n");
  fileread(filename);
  filewrite(filename, "kernel write test\n");
  return 0;
}

void cleanup_module()
{
  printk("<1>good,bye!\n");
}
