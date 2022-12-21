/***************************************************************************//**
*  \file       driver.c
*
*  \details    OS Lab2 (procfs: task_cputime, inode)
*
*  \author     ErekhinskyAndrey
*
* *******************************************************************************/

#include <linux/kernel.h> /* Для работы с ядром. */
#include <linux/init.h> 
#include <linux/module.h> /* Для модулей. */
#include <linux/slab.h> /* Для kmalloc */
#include <linux/uaccess.h> /* Для copy_from_user. */
#include <linux/proc_fs.h> /* Для использования procfs.*/
#include <linux/string.h>
#include <linux/version.h>

#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/mount.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/fs.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_MAX_SIZE 1024

#define PROCFS_NAME "mykernelmodule"
#define PROCFS_NAME_TASK "cputime"
#define PROCFS_NAME_INODE "inode"
#define USER_FILE_NAME "Desktop/LR2/myusermodule/myusermodule"

static struct proc_dir_entry *parent;
int len;

static char procfs_buffer[PROCFS_MAX_SIZE];
static unsigned long procfs_buffer_size = 0;

pid_t p_id = 1;
struct pid *pid_struct;
struct task_struct *cputime;

struct inode *inode;
struct path path;
char *path_name = "/proc/meminfo";

char *str;

/***************** Procfs Functions *******************/

static int      __init tct_ind_init(void);
static void     __exit tct_ind_exit(void);

static ssize_t  read_task_cputime(struct file *filp, char __user *buffer, size_t buffer_length, loff_t * offset);
static ssize_t write_task_cputime(struct file *filp, const char *buffer, size_t buffer_length, loff_t *off);

static ssize_t  read_inode(struct file *filp, char __user *buffer, size_t buffer_length,loff_t * offset);
static ssize_t write_inode(struct file *filp, const char *buffer, size_t buffer_length, loff_t *off);

/***************** fops Functions *******************/

#ifdef HAVE_PROC_OPS
static const struct proc_ops tct_fops = {
	.proc_read = read_task_cputime,
	.proc_write = write_task_cputime,
};
static const struct proc_ops ind_fops = {
    .proc_read = read_inode,
    .proc_write = write_inode,
};
#else
static const struct file_operations tct_fops = {
	.read = read_task_cputime,
	.write = write_task_cputime,
};
static const struct file_operations ind_fops = {
    .read = read_inode,
    .write = write_inode,
};
#endif

/***************** Task_cputime *******************/

static ssize_t  read_task_cputime(struct file *filp, char __user *buffer, size_t buffer_length, loff_t *offset) {

	// cat /proc/mymodule/cputime

	pid_struct = find_get_pid(p_id);
	
	cputime = pid_task(pid_struct, PIDTYPE_PID);

	str = (char *)kmalloc(300*sizeof(char),GFP_KERNEL);

	sprintf(str, "^PID:%d^time spent in user mode:%lld^time spent in kernel mode:%lld^total time spent on the CPU:%lld", p_id, cputime->utime / 1000000, cputime->stime / 1000000, cputime->last_sum_exec_runtime / 1000000);
	
	len = strlen(str);
	
	if(copy_to_user(buffer, str, strlen(str)) || *offset >= len){
		len = 0;
   	} else {
		pr_info("read_task_cputime called.\n");
		*offset += len;
	}

	kfree(str);
	
	return len;
}

static ssize_t write_task_cputime(struct file *filp, const char *buffer, size_t buffer_length, loff_t *off) {

	//printf "PID" | sudo tee /proc/mymodule/cputime
	
	procfs_buffer_size = buffer_length;
	
	if (procfs_buffer_size > PROCFS_MAX_SIZE){
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}
	
	if(copy_from_user(procfs_buffer,buffer,procfs_buffer_size))
		return -EFAULT;

	procfs_buffer[buffer_length] = (char) 0;

	p_id = simple_strtoul(procfs_buffer,NULL,0);

	printk(KERN_INFO "write_task_cputime called.\nNew PID: %d\n", p_id);
	
	return procfs_buffer_size;
}

/***************** Inode *******************/


static ssize_t  read_inode(struct file *filp, char __user *buffer, size_t buffer_length,loff_t * offset) {

	// cat /proc/mymodule/inode

	kern_path(path_name, LOOKUP_FOLLOW, &path);

    	inode = path.dentry->d_inode;

	str = (char *)kmalloc(1000*sizeof(char),GFP_KERNEL);
	
	sprintf(str, "^Path name: %s^Номер индекса: %ld^Права доступа: %d^Размер: %lld^Количество жестких ссылок: %d", path_name, inode->i_ino, inode->i_mode, inode->i_size, inode->i_nlink);
	
	len = strlen(str);

	if(copy_to_user(buffer, str, strlen(str)) || *offset >= len){
		len = 0;
   	} else {
		pr_info("read_inode called.\n");
		*offset += len;
	}

	kfree(str);

	return len;
}


static ssize_t write_inode(struct file *filp, const char *buffer, size_t buffer_length, loff_t *off) {

	//printf "PATH" | sudo tee /proc/mymodule/inode
	
	procfs_buffer_size = buffer_length;
	
	if (procfs_buffer_size > PROCFS_MAX_SIZE){
		procfs_buffer_size = PROCFS_MAX_SIZE;
	}
	
	if(copy_from_user(procfs_buffer,buffer,procfs_buffer_size))
		return -EFAULT;

	procfs_buffer[buffer_length] = (char) 0;
	
	path_name = procfs_buffer;

	printk(KERN_INFO "write_inode called.\nNew PATH: %s\n", path_name);
	
	return procfs_buffer_size;
}


/***************** Driver init *******************/

static int __init tct_ind_init(void)
{

	parent = proc_mkdir(PROCFS_NAME, NULL);

	proc_create(PROCFS_NAME_TASK, 0666, parent, &tct_fops);
	proc_create(PROCFS_NAME_INODE, 0666, parent, &ind_fops);
		
	pr_info("/proc/%s created\n/proc/%s created\n", PROCFS_NAME_TASK, PROCFS_NAME_INODE);

	return 0;
	
}

/***************** Driver exit *******************/

static void __exit tct_ind_exit(void)
{
    proc_remove(parent);
    pr_info("/proc/%s removed\n", PROCFS_NAME);
}

/*************************************************/

module_init(tct_ind_init);
module_exit(tct_ind_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ErekhinskyAndrey");
MODULE_DESCRIPTION("OS Lab2");
MODULE_VERSION("1.8");
