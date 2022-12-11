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

#include <stdio.h>
#include <stdlib.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
#define HAVE_PROC_OPS
#endif

#define PROCFS_NAME "mykernelmodule"
#define PROCFS_NAME_TASK "cputime"
#define PROCFS_NAME_INODE "inode"
#define USER_FILE_NAME "Desktop/myusermodule"

static int len = 1;
static struct proc_dir_entry *parent;

pid_t p_id = 1;
struct pid *pid_struct;
struct task_struct *cputime;

struct inode *inode;
struct path path;
char *path_name = "/proc/meminfo";

FILE *USER;

/***************** Procfs Functions *******************/

static int      __init tct_ind_init(void);
static void     __exit tct_ind_exit(void);

static ssize_t  read_task_cputime(struct file *filp, char __user *buffer, size_t length, loff_t * offset);
static ssize_t write_task_cputime(struct file *filp, const char *buff, size_t len, loff_t *off);

static ssize_t  read_inode(struct file *filp, char __user *buffer, size_t length,loff_t * offset);
static ssize_t write_inode(struct file *filp, const char *buff, size_t len, loff_t *off);

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

/***************** Buffer Functions *******************/

static char *arr;
static char *str;
int k = 0;
int i;

static void clean_line(void){
    for (i = k; i < k + 60; i++){
        arr[i] = '\0';
    }
    for (i = 0; i < 60; i++){
        str[i] = '\0';
    }
}

static void clean_buffer(void){
    for (i = 0; i < 100000; i++){
        arr[i] = '\0';
    }
}

static void go_to_new_line(void){
	
    int ll = 0;
    for (i = k; i < k + 60; i++){
        arr[i] = str[i - k];
        ll++;
    }
    k += ll;
}

/***************** Task_cputime *******************/

static ssize_t  read_task_cputime(struct file *filp, char __user *buffer, size_t length, loff_t * offset) {

	// cat /proc/mymodule/cputime

	printk(KERN_INFO "read_task_cputime called.\n");

	if (len) {
        	len=0;
	}
	else {
		len=1;
		return 0;
	}

	pid_struct = find_get_pid(p_id);
	cputime = pid_task(pid_struct, PIDTYPE_PID);
	
	clean_buffer();

	clean_line();
	sprintf(str, "PID: %d\n", p_id);
	go_to_new_line();

	clean_line();
	sprintf(str, "task_cputime time spent in user mode: %lld\n", cputime->utime / 1000000);
	go_to_new_line();
	
	clean_line();
	sprintf(str, "task_cputime time spent in kernel mode: %lld\n", cputime->stime / 1000000);
	go_to_new_line();
	
	clean_line();
	sprintf(str, "task_cputime total time spent on the CPU: %lld\n", cputime->last_sum_exec_runtime / 1000000);
	go_to_new_line();

	/*
	if(copy_to_user(buffer, arr, k)){
        	pr_err("Data Send : Err!\n");
    }
	*/
	
	USER = fopen(USER_FILE_NAME, "r+");
	
	fprintf(USER, "%s", arr);
	
	fclose(USER);
	
	return length;
}

static ssize_t write_task_cputime(struct file *filp, const char *buff, size_t len, loff_t *off) {

	//printf "PID" | sudo tee /proc/mymodule/cputime
	
	USER = fopen(USER_FILE_NAME, "r+");
	
	fscanf(USER, "%d", p_id);
	
	fclose(USER);
	
	/*
	char *id;
	id = (char *)kmalloc(1000*sizeof(char),GFP_KERNEL);

	if(copy_from_user(id,buff,len))
		return -EFAULT;

	p_id = simple_strtoul(id,NULL,0);
	*/

	printk(KERN_INFO "write_task_cputime called.\nNew PID: %d\n", p_id);
	
	return len;
}

/***************** Inode *******************/


static ssize_t  read_inode(struct file *filp, char __user *buffer, size_t length,loff_t * offset) {

	// cat /proc/mymodule/inode
	
	printk(KERN_INFO "read_inode called.\n");

	if (len) {
        	len=0;
	}
	else {
		len=1;
		return 0;
	}

	kern_path(path_name, LOOKUP_FOLLOW, &path);
    	inode = path.dentry->d_inode;

	clean_buffer();

	clean_line();
	sprintf(str, "Path name: %s\n", path_name);
	go_to_new_line();
	
	clean_line();
	sprintf(str, "Номер индекса: %ld\n", inode->i_ino);
	go_to_new_line();
	
	clean_line();
	sprintf(str, "Права доступа: %d\n", inode->i_mode);
	go_to_new_line();
	
	clean_line();
	sprintf(str, "Размер: %lld\n", inode->i_size);
	go_to_new_line();

	clean_line();
	sprintf(str, "Количество жестких ссылок: %d\n", inode->i_nlink);
	go_to_new_line();

	clean_line();
	sprintf(str, "Номер версии индекса: %d\n", inode->i_generation);
	go_to_new_line();

	/*
	if(copy_to_user(buffer, arr, k)){
        	pr_err("Data Send : Err!\n");
    }
	*/
		
	USER = fopen(USER_FILE_NAME, "r+");
	
	fprintf(USER, "%s", arr);
	
	fclose(USER);

	return length;
}



static ssize_t write_inode(struct file *filp, const char *buff, size_t len, loff_t *off) {

	//printf "PATH" | sudo tee /proc/mymodule/inode
	
	USER = fopen(USER_FILE_NAME, "r+");
	
	fscanf(USER, "%s", path_name);
	
	fclose(USER);

	/*
	char *new_path_name;
	new_path_name = (char *)kmalloc(1000*sizeof(char), GFP_KERNEL);

	if(copy_from_user(new_path_name,buff,len))
		return -EFAULT;
	
	path_name = new_path_name;
	*/
	
	printk(KERN_INFO "write_inode called.\n");

	return len;
}


/***************** Driver init *******************/

static int __init tct_ind_init(void)
{
    	arr = kmalloc(100000, GFP_KERNEL);
    	str = kmalloc(60, GFP_KERNEL);

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