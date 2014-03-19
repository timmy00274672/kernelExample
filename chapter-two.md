# Process Management and Scheduling

To support *multitasking*, the kernel need to deal lot of issues, like:

- Applications must not interfere with each others. -> 	Memory Protection

- CPU time shared fairly. -> Scheduling, platfrom-independent

- Make sure the environment is the same as when it last withdrew processor resources. -> 	platfrom-dependent

## Process Priorities

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

Key idea : A process is not always ready to run.

A process may have one of the following states : 'running', 'waiting', 'sleeping', 'zombie'
.

- 'zombie' : 

	As the name suggests, such processes are defunct but are somehow still alive. In reality, they are dead because their resources (RAM, connections to peripherals, etc.) have already been released so that they cannot and never will run again. However, they are still alive because there are still entries for them in the **process table**.

	How do zombies come about? The reason lies in the process creation and destruction structure under Unix. A program terminates when two events occur — first, the program must be killed by another process or by a user (this is usually done by sending a **`SIGTERM`** or **`SIGKILL`** **signal**, which is equivalent to terminating the process regularly); second, the parent process from which the process originates **must** invoke or have already invoked the **`wait4`** (read: wait for) system call when the child process terminates. This confirms to the kernel that the parent process has acknowledged the death of the child. The system call enables the kernel to free resources reserved by the child process.

	A zombie occurs when only the first condition (the program is terminated) applies but not the second ( `wait4` ). A process always switches briefly to the zombie state between termination and removal of its data from the process table. In some cases (if, e.g., the parent process is badly programmed and does not issue a wait call), a zombie can firmly lodge itself in the process table and remain there **until the next reboot**. This can be seen by reading the output of process tools such as ps or top . This is hardly a problem as the residual data take up little space in the kernel.

The system saves all processes in a process table — regardless their states.

### Preemptive Multitasking

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

All algorithms of the Linux kernel concerned with processes and programs are built around a data structure named `task_struct` and defined in `include/sched.h`. Disscss [here](sched).

### Process Types

The processes are generated using the `fork`, `exec`, `clone` system calls:

- `fork` : copy the current process as *child process*. 

	All resources are copied(memory data, open files, working directory)

- `exec` : using current process to running another process
- `clone` : it's used to implement **thread**

# Linux philosophy:

Since Linux is optimized for throughput and tries to handle common cases as fast as possible, guaranteed response times are only very hard to achieve.