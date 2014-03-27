# Process Management and Scheduling

To support *multitasking*, the kernel need to deal lot of issues, like:

- Applications must not interfere with each others. -> 	Memory Protection

- CPU time shared fairly. -> Scheduling, platfrom-independent

- Make sure the environment is the same as when it last withdrew processor resources. -> 	platfrom-dependent

## Content

- [Process Priorities][Process Priorities]
- [Process Life Cycle][Process Life Cycle]
	- [Preemptive Multitasking][Preemptive Multitasking]
- [Process Representation][Process Representation]
	- [Process Types][Process Types]
	- [Namespaces][Namespaces]
		- [Introduction][Introduction]
		- [Implementation][Implementation]
			- [UTS namespaces][UTS namespaces]
			- [User Namespace][User Namespace]
	- [Process Indentification Number][Process Indentification Number]
		- [Managing PIDs][Managing PIDs]
		- [Overview Of Namespace][Overview Of Namespace]

## Process Priorities

[Process Priorities]: #process-priorities

At least, we can devide the processes into :

- Real-time process (Hard/Soft)
	
	- Linux does not support hard real-time processing, at least not in the vanilla kernel.
	- An example of a soft real-time process is a write operation to a CD. Data must be received by the CD writer at a **certain rate** because data are written to the medium in a continuous stream. If system loading is too high, the data stream may be interrupted briefly, and this may result in an unusable CD, far less drastic than a plane crash(hard real-time process). Nevertheless, the write process should always be granted CPU time when needed — before all other normal processes.

- Non-Real-time process(normal processes)

	- Assigning priorities to them
	- For example, a long compiler run or numerical calculations need only very low priority because it is of **little consequence** if computation is interrupted occasionally for a second or two — users are unlikely to notice. In contrast, **interactive applications** should respond as quickly as possible to user commands because users are notoriously impatient.

Classify with scheme:

- non-preemptive multitasking

	Its runtime environment should be saved so that results are not lost and the process environment is fully reinstated when its turn comes around again. (essentially, the contents of all CPU registers and the page tables).

	The structure of Linux process management requires two further process state options — **user** mode and **kernel** mode. These reflect the fact that all modern CPUs have (at least) two different execution modes, one of which has unlimited rights while the other is subject to various restrictions.

	Normally the kernel is in user mode in which it may access only its own data and cannot therefore inter- fere with other applications in the system — it usually doesn’t even notice that there are other programs besides itself.

- preemptive multitasking

The scheduler code has seen two complete rewrites in recent years:

- O(1) scheduler

	One particular property of this scheduler was that it could perform its work in constant time independent of the number of processes that were running on a system.

- completely fair scheduler

## Process Life Cycle

[Process Life Cycle]: #process-life-cycle

Key idea : A process is not always ready to run. 

- More details in [sched](sched/sched.md#task-state).
- The system saves all processes in a process table — regardless their states.

### Preemptive Multitasking

[Preemptive Multitasking]: #preemptive-multitasking
If a process wants to access system data or functions (the latter manage the resources shared between all processes, e.g., filesystem space), it must switch to **kernel** mode. Methods:

- **System Call**
- **Interrupt**

The preemptive scheduling model of the kernel establishes a hierarchy that determines which process states may be interrupted by which other states.

Priority: (From Low to High)

1. Normal Process
2. Processing a system call

	One option known as kernel preemption (kernel 2.5) supports switches to another process, if this is urgently required, even during the execution of system calls in kernel mode (but not during interrupts), because the time for processing a system call may be too long for some applications that are reliant on constant data streams

3. Processing a interrupt

## Process Representation

[Process Representation]: #process-representation

All algorithms of the Linux kernel concerned with processes and programs are built around a data structure named `task_struct` and defined in `include/sched.h`. Disscss [here](sched).

### Process Types

[Process Types]: #process-types

The processes are generated using the `fork`, `exec`, `clone` system calls:

- `fork` : copy the current process as *child process*. 

	All resources are copied(memory data, open files, working directory)

- `exec` : using current process to running another process
- `clone` : it's used to implement **thread**

### Namespaces

[Namespaces]: #namespaces

What is **namespace**? If a compnay provide IaaS, it wants to provide lots of custumer for each hardware. There are methods like vitualization. Each user is in their own kernel. **Namespaces** is another method, by which users share one kernel. 

Namespaces provide a lightweight form of virtualization by allowing us to view the global properties of a running system under different aspects.

[Lwn.net][1] has a introduction:

The Linux 3.8 merge window saw the acceptance of Eric Biederman's sizeable series of user namespace and related patches. Although there remain some details to finish—for example, a number of Linux filesystems are **not** yet user-namespace aware—the implementation of **user namespaces** is now functionally complete.

The completion of the user namespaces work is something of a milestone, for a number of reasons. 

1. 	This work represents the completion of one of the most complex namespace implementations to date, as evidenced by the fact that it has been around five years since the first steps in the implementation of user namespaces (in Linux 2.6.23). 

2. 	The namespace work is currently at something of a "stable point", with the implementation of most of the existing namespaces being more or less complete. This does not mean that work on namespaces has finished: other namespaces may be added in the future, and there will probably be further extensions to existing namespaces, such as the addition of namespace isolation for the kernel log. 

3.	The recent changes in the implementation of user namespaces are something of a game changer in terms of how namespaces can be used: starting with Linux 3.8, *unprivileged processes* can create user namespaces in which they have full privileges, which in turn allows any other type of namespace to be created inside a user namespace. Thus, the present moment seems a good point to take an overview of namespaces and a practical look at the namespace API. 

[1]: http://lwn.net/Articles/531114/

#### Introduction

[Introduction]:#introduction

Currently, Linux implements six different types of namespaces. The purpose of each namespace is to wrap a particular global system resource in an abstraction that makes it appear to the processes within the namespace that they have their own **isolated** instance of the global resource. One of the overall goals of namespaces is to support the implementation of **containers**, a tool for *lightweight virtualization* (as well as other purposes) that provides a group of processes with the illusion that they are the only processes on the system.

In the discussion below, we present the namespaces in the order that they were implemented. The `CLONE_NEW*` identifiers listed in parentheses are the names of the constants used to identify namespace types when employing the namespace-related APIs (`clone()`, `unshare()`, and `setns()`).

-	Mount namespaces (CLONE_NEWNS, Linux 2.4.19) 

	- 	Relative system call : `mount()`, `umount()`
	- 	Each container have own : different views of the filesystem hierarchy

- 	UTS namespaces (CLONE_NEWUTS, Linux 2.6.19) : "UNIX Time-sharing System"
	
	- 	Relative system call : `uname()`, `sethostname()`, `setdomainname()`
	- 	Each container have own : **hostname**, **NIS domain name**.

-	IPC namespaces (CLONE_NEWIPC, Linux 2.6.19) 

	-	Each container have own : System V **IPC identifiers**, POSIX **message queue filesystem**.

-	PID namespaces (CLONE_NEWPID, Linux 2.6.24) 

	-	Each container have own : process ID number space, **init** (PID 1)
	-	Benefits : 

		-	containers can be migrated between hosts while keeping the same process IDs for the processes inside the container.
		-	 the "ancestor of all processes" manages various system initialization tasks and reaps orphaned child processes when they terminate.

	-	A process will have one PID for each of the layers of the hierarchy starting from the PID namespace in which it resides through to the root PID namespace.
	-	A process can see (e.g., view via /proc/PID and send signals with kill()) only processes contained in its own PID namespace and the namespaces nested below that PID namespace.

-	Network namespaces (CLONE_NEWNET, started in Linux 2.4.19 2.6.24 and largely completed by about Linux 2.6.29) 

	-	Each container have own : network devices, IP addresses, IP routing tables, `/proc/net` directory, port numbers, and so on
	-	It is possible to have multiple containerized web servers on the same host system, with each server bound to port 80 in its (per-container) network namespace.

-	User namespaces (CLONE_NEWUSER, started in Linux 2.6.23 and completed in Linux 3.8) 
	
	-	Each container have own : UID, GID
	-	A process can have a normal unprivileged user ID outside a user namespace while at the same time having a user ID of 0 inside the namespace.
	-	Starting in Linux 3.8, unprivileged processes can create user namespaces, which opens up a raft of interesting new possibilities for applications: since an otherwise unprivileged process can hold root privileges inside the user namespace, unprivileged applications now have access to functionality that was formerly limited to root.

#### Implementation

[Implementation]:#implementation

![](img/2_namespace.png)

Each kernel subsystem that is aware of namespaces must provide a data structure that collects all objects that must be available on a per-namespace basis. struct **`nsproxy`** is used to collect pointers to the subsystem-specific namespace wrappers:

It can be found in [*/include/linux/nsproxy.h*](namespace/nsproxy.h).

In `sched.h`, we find `task_struct` has a property `struct nsproxy *nsproxy;`. Because a pointer is used, a collection of sub-namespaces can be shared among multiple processes. This way, changes in a given namespace will be visible in all processes that belong to this namespace.

The initial global namespace is defined by `init_nsproxy` in `nsproxy.c` , which keeps pointers to the initial objects of the per-subsystem namespaces

##### UTS namespaces

[UTS namespaces]:#uts-namespaces

- Relative system call : `uname()`, `sethostname()`, `setdomainname()`
- Each container have own : **hostname**, **NIS domain name**.
- More details in [namespace](namespace/namespace.md#uts-namespaces). 

##### User Namespace

[User Namespace]:#user-namespace

Recall : 
	
- Each container have own : UID, GID
- A process can have a normal unprivileged user ID outside a user namespace while at the same time having a user ID of 0 inside the namespace.
- Starting in Linux 3.8, unprivileged processes can create user namespaces, which opens up a raft of interesting new possibilities for applications: since an otherwise unprivileged process can hold root privileges inside the user namespace, unprivileged applications now have access to functionality that was formerly limited to root.
- Details in [namespace](namespace/namespace.md#uts-namespaces). 

### Process Indentification Number

[Process Indentification Number]: #process-identification-number

- It's called PID here. However, each process characterized also by other identifiers than PID. Examples:

	- *PID* 
		- PID namespaces are organized in a hierarchy.
		- We have to distinguish between **local** and **global** IDs
			- Global IDs : 
				valid within the kernel itself and in the initial namespace to which the `init` tasks started during *boot* belongs.
			- Local IDs :
				belong to a specific namespace and are not globally valid. For each ID type, they are valid within the namespace to which they belong, but identifiers of identical type may appear with the same ID number in a different namespace.
		- `task_struct->pid` 
	- *TGID* : thread group ID
		- `task_struct->tgid`
	- *PGID* : process group
		- relative *system call* : `setpgrp`
		- Process groups facilitate the sending of signals to all members of the group
		- `task_struct->signal->__pgrp` denoted the global PGID
	- *SID* : session ID
		- relative *system call* : `setsid`
		- Several process groups can be combined in a session.
		- It is used in terminal programming 
		- `task_struct->signal->__session` denotes the global SID
- Recall the `task_struct` in *sched.h* : 

	```c
	struct task_struct{
	...
		pid_t pid;
		pid_t tgid;
	...
	}
	```

#### Managing PIDs

[Managing PIDs]: #managing-pids

In addition to these two fields(`pid`, `tgid`), the kernel needs to find a way to manage all local per-namespace quantities, as well as the other identifiers like TID and SID. This requires several interconnected data structures and numerous auxiliary functions.

- The data structures required to represent IDs themselves is `struct pid_namespace` defined in *linux/pid_namespace.h*. Details in [namespace](namespace/namespace.md#pid_namespace). 
- Relative files:
	
	I use below	command:

	```bash
	find -name '*.h' | xargs grep -l '#include <linux/pid_namespace.h>'
	```			

	Results:
		
		./include/linux/perf_event.h
		./include/linux/init_task.h
- PID management is centered around two data structures in *linux/pid.h*. Details in [namespace](namespace/namespace.md#pid).

#### Overview Of Namespace

[Overview Of Namespace]:#overview-of-namespace

![](img/2_Overview_NS.png)

# Linux philosophy:

Since Linux is optimized for throughput and tries to handle common cases as fast as possible, guaranteed response times are only very hard to achieve.