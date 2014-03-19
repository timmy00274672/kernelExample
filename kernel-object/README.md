## Source Code 
`kobject.h` can be found in `include/linux/kobject.h`: [link](kobject.h)

### kobject.h

Two structure : 

1.  kobject
2.  kset

#### kobject

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

extern void kobject_init(struct kobject *kobj, struct kobj_type *ktype);
extern void kobject_put(struct kobject *kobj);
extern void kobject_init(struct kobject *kobj, struct kobj_type *ktype);
int kobject_add(struct kobject *kobj, struct kobject *parent, const char *fmt, ...); 
extern int __must_check kset_register(struct kset *kset);
extern void kset_unregister(struct kset *kset);

```

- It is essential that `kobject` s are **not** linked with other data structures by means of **~~pointers~~** but are directly embedded. Managing the kernel object itself amounts to
managing the whole containing object this way. Since struct kobject is **embedded**
into many data structures of the kernel, the developers take care to keep it small.
Adding a single new element to this data structure results in a size increase of many
other data structures. Embedded kernel objects look as follows:
    
    ```c
    struct sample {
        ...
        struct kobject kobj;
        ...
    };
    ```

- `Sysfs` is a virtual filesystem that allows for exporting various properties of the system into userspace.
- `kobj_type` provides an interface to `sysfs` filesystem instead of collecting various kernel objects. (`kset` does collect various kernel objects)

#### kset

In many cases, it is necessary to group different kernel objects into a set — for instance, the set of all
character devices or the set of all PCI-based devices.

```c
/**
 * struct kset - a set of kobjects of a specific type, belonging to a specific subsystem.
 *
 * A kset defines a group of kobjects.  They can be individually
 * different "types" but overall these kobjects all want to be grouped
 * together and operated on in the same manner.  ksets are used to
 * define the attribute callbacks and other common events that happen to
 * a kobject.
 *
 * @list: the list of all kobjects for this kset
 * @list_lock: a lock for iterating over the kobjects
 * @kobj: the embedded kobject for this kset (recursion, isn't it fun...)
 * @uevent_ops: the set of uevent operations for this kset.  These are
 * called whenever a kobject has something happen to it so that the kset
 * can add new environment variables, or filter out the uevents if so
 * desired.
 */
struct kset {
    struct list_head list;
    spinlock_t list_lock;
    struct kobject kobj;
    const struct kset_uevent_ops *uevent_ops;
};

```
-   `uevent_ops` provides several function pointers to methods that relay information about the state of the set to userland. This mechanism is used by the core of the driver model, for instance, to format messages that inform about the addition of new devices.

### kref.h

```c
struct kref {
    atomic_t refcount;
};

/**
 * kref_init - initialize object.
 * @kref: object in question.
 */
static inline void kref_init(struct kref *kref)
{
    atomic_set(&kref->refcount, 1);
}

/**
 * kref_get - increment refcount for object.
 * @kref: object.
 */
static inline void kref_get(struct kref *kref)
{
    /* If refcount was 0 before incrementing then we have a race
     * condition when this kref is freeing by some other thread right now.
     * In this case one should use kref_get_unless_zero()
     */
    WARN_ON_ONCE(atomic_inc_return(&kref->refcount) < 2);
}

/**
 * kref_put - decrement refcount for object.
 * @kref: object.
 * @release: pointer to the function that will clean up the object when the
 *       last reference to the object is released.
 *       This pointer is required, and it is not acceptable to pass kfree
 *       in as this function.  If the caller does pass kfree to this
 *       function, you will be publicly mocked mercilessly by the kref
 *       maintainer, and anyone else who happens to notice it.  You have
 *       been warned.
 *
 * Decrement the refcount, and if 0, call release().
 * Return 1 if the object was removed, otherwise return 0.  Beware, if this
 * function returns 0, you still can not count on the kref from remaining in
 * memory.  Only use the return value if you want to see if the kref is now
 * gone, not present.
 */
static inline int kref_put(struct kref *kref, void (*release)(struct kref *kref))
{
    return kref_sub(kref, 1, release);
}

```

`refcount` is an atomic data type to specify the number of positions in the kernel at which an object is currently being used. When the counter reaches 0, the object is no longer needed and can therefore be removed from memory.

Encapsulation of the single value in a structure was chosen to prevent direct manipulation of the value. `kref_init` must always be used for initialization. If an object is in use, `kref_get` must be invoked **beforehand** to increment the reference counter. `kref_put` decrements the counter when the object is no longer used.
