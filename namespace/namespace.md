## Content

- [nsproxy][nsproxy]
- [UTS namespaces][UTS namespaces]
- [User Namespace][User Namespace]
- [pid_namespace][pid_namespace]
- [pid][pid]

## nsproxy

[nsproxy]:#nsproxy
Each kernel subsystem that is aware of namespaces must provide a data structure that collects all objects that must be available on a per-namespace basis. **`struct nsproxy`**, which can be found in [*/include/linux/nsproxy.h*](nsproxy.h), is used to collect pointers to the subsystem-specific namespace wrappers.

In *sched.h*, we find `task_struct` has a property `struct nsproxy *nsproxy;`. Because a pointer is used, a collection of sub-namespaces can be shared among multiple processes. This way, changes in a given namespace will be visible in all processes that belong to this namespace.

The initial global namespace is defined by `init_nsproxy` in *nsproxy.c* , which keeps pointers to the initial objects of the per-subsystem namespaces

## UTS namespaces  

[UTS namespaces]:#uts-namespaces
Recall : 

- Relative system call : `uname()`, `sethostname()`, `setdomainname()`
- Each container have own : **hostname**, **NIS domain name**.
- The corresponding code can be found in `utsnae.h`, in which `uts_namespace` is the structure saving releant information.

	```c
	struct uts_namespace {
		struct kref kref;
		struct new_utsname name;
		struct user_namespace *user_ns;
		unsigned int proc_inum;
	};

	struct new_utsname {
		char sysname[__NEW_UTS_LEN + 1];
		char nodename[__NEW_UTS_LEN + 1];
		char release[__NEW_UTS_LEN + 1];
		char version[__NEW_UTS_LEN + 1];
		char machine[__NEW_UTS_LEN + 1];
		char domainname[__NEW_UTS_LEN + 1];
	};
	```
- We can use `cat /proc/sys/kernel/xxx` to see these values, which is initial settings stored in `init_uts_ns` in `init/version.c`.

	```c
	struct uts_namespace init_uts_ns = {
		.kref = {
			.refcount	= ATOMIC_INIT(2),
		},
		.name = {
			.sysname	= UTS_SYSNAME,
			.nodename	= UTS_NODENAME,
			.release	= UTS_RELEASE,
			.version	= UTS_VERSION,
			.machine	= UTS_MACHINE,
			.domainname	= UTS_DOMAINNAME,
		},
		.user_ns = &init_user_ns,
		.proc_inum = PROC_UTS_INIT_INO,
	};
	```
- Above constants are set in `utsrelease.h` which is dynamically built by top-level *Makefile*.
- Now, question is how to create a new UTS namespace? Information can be found via `man 2 clone`: 
	
	```   
	/* Prototype for the glibc wrapper function */

	#include <sched.h>

	int clone(int (*fn)(void *), void *child_stack,
     	int flags, void *arg, ...
     	/* pid_t *ptid, struct user_desc *tls, pid_t *ctid */ );

	/* Prototype for the raw system call */

	long clone(unsigned long flags, void *child_stack,
		void *ptid, void *ctid,
		struct pt_regs *regs);


	CLONE_NEWUTS (since Linux 2.6.19)
	If CLONE_NEWUTS is set, then create the process  in  a  new  UTS
	namespace,  whose identifiers are initialized by duplicating the
	identifiers from the UTS namespace of the calling  process.   If
	this  flag  is  not  set, then (as with fork(2)), the process is
	created in the same UTS namespace as the calling process.   This
	flag is intended for the implementation of containers.

	A  UTS namespace is the set of identifiers returned by uname(2);
	among these, the domain name and the host name can  be  modified
	by  setdomainname(2) and  sethostname(2), respectively.  Changes
	made to the identifiers in a UTS namespace are  visible  to  all
	other  processes  in  the same namespace, but are not visible to
	processes in other UTS namespaces.

	Use of this flag requires: a kernel  configured  with  the  CON‐
	FIG_UTS_NS   option   and   that   the   process  be  privileged
	(CAP_SYS_ADMIN).
	```

## User Namespace
[User Namespace]:#user-namespace

- I found there is no user-namespace related code in *nsproxy.h*, based on *kernel v3.11.10*, but with *user_namespace.h* and *user_namespace.c*.

	Therefore, I use below to find the associated files.
		
		find -name '*.h' | xargs grep -l '#include <linux/user_namespace.h>'

	Results:

		./include/linux/sock_diag.h
		./include/linux/init_task.h

- How to coperate with each others is omitted here temporaity. I read *user_namespace.h* and *user_namespace.c* first.
- The `user_namespace` struct :

	```c
	struct user_namespace {
		struct uid_gid_map	uid_map;
		struct uid_gid_map	gid_map;
		struct uid_gid_map	projid_map;
		atomic_t		count;
		struct user_namespace	*parent;
		int			level;
		kuid_t			owner;
		kgid_t			group;
		unsigned int		proc_inum;

		/* Register of per-UID persistent keyrings for this namespace */
	#ifdef CONFIG_PERSISTENT_KEYRINGS
		struct key		*persistent_keyring_register;
		struct rw_semaphore	persistent_keyring_register_sem;
	#endif
	};
	```

## pid_namespace
[pid_namespace]:#pid_namespace

- `struct pid_namespace` defined in *linux/pid_namespace.h*:

	```c
	struct pid_namespace {
		struct kref kref;
		struct pidmap pidmap[PIDMAP_ENTRIES];
		struct rcu_head rcu;
		int last_pid;
		unsigned int nr_hashed;
		struct task_struct *child_reaper;
		struct kmem_cache *pid_cachep;
		unsigned int level;
		struct pid_namespace *parent;
	#ifdef CONFIG_PROC_FS
		struct vfsmount *proc_mnt;
		struct dentry *proc_self;
	#endif
	#ifdef CONFIG_BSD_PROCESS_ACCT
		struct bsd_acct_struct *bacct;
	#endif
		struct user_namespace *user_ns;
		struct work_struct proc_work;
		kgid_t pid_gid;
		int hide_pid;
		int reboot;	/* group exit code if this pidns was rebooted */
		unsigned int proc_inum;
	};
	```
- `child_reaper` :

	Every PID namespace is equipped with a task that assumes the role taken by `init` in the global picture. One of the purposes of `init` is to call `wait4` for orphaned tasks, and this must likewise be done by the namespace-specific init variant. A pointer to the task structure of this task is stored in `child_reaper`.
- `parent`, `level` : 

	`parent` is pointer to the parent namespace, and `level` denotes the depth in the namespace hierarchy. The initial namespace has level 0. 

	Counting the levels is important because IDs in higher levels must be visible in lower levels. From a given level setting, the kernel can infer how many IDs must be associated with a task.

## pid
[pid]:#pid

PID management is centered around two data structures in *linux/pid.h*.

- `struct pid`
	
	The kernel-internal representation of a *PID*

	```c
	struct pid
	{
		atomic_t count;
		unsigned int level;
		/* lists of tasks that use this pid */
		struct hlist_head tasks[PIDTYPE_MAX];
		struct rcu_head rcu;
		struct upid numbers[1];
	};	
	```

	The definition of `struct pid` is headed by a reference counter `count` . `tasks` is an array with a *hash list head* for every **ID type**. This is necessary because an ID can be used for several processes. All `task_struct` instances that share a given ID are linked on this list.
- `struct upid`
	
	Store the information that is visible in a specific namespace.

	```c
	struct upid {
		/* Try to keep pid_chain in the same cacheline as nr for find_vpid */
		int nr;
		struct pid_namespace *ns;
		struct hlist_node pid_chain;
	};
	```

	`nr` represents the numerical value of an ID, and `ns` is a pointer to the namespace to which the value belongs. All upid instances are kept on a *hash table*, and `pid_chain` allows for implementing hash overflow lists with standard methods of the kernel. 
- `enum pid_type`

	```c
	enum pid_type
	{
		PIDTYPE_PID,
		PIDTYPE_PGID,
		PIDTYPE_SID,
		PIDTYPE_MAX
	};
	```