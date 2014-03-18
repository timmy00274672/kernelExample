### Source Code 
`kobject.h` can be found in `include/linux/kobject.h`: [link](kobject.h)

#### kobject.h

```c
struct kobject
{
    /*
		the name is exported to userspace using `sysfs`
    */
    const char      *name;
    struct list_head    entry;
    struct kobject      *parent;
    struct kset     *kset;
    struct kobj_type    *ktype;
    struct sysfs_dirent *sd;
    /*
		for simplify reference management	
    */
    struct kref     kref;
    unsigned int state_initialized: 1;
    unsigned int state_in_sysfs: 1;
    unsigned int state_add_uevent_sent: 1;
    unsigned int state_remove_uevent_sent: 1;
    unsigned int uevent_suppress: 1;
};

```

Sysfs is a virtual filesystem that allows for exporting various properties of the system into userspace.