Let us appriciate the [`list`][source] API.

## Outline

- [Preface](#preface)
- [source code of list.h][source]

## Preface

A recurring task in C programs is the handling of doubly linked lists. 
The kernel too is required to handle such lists. 
[List][source] is standard list implementation of the kernel.

When I read this code, I found `struct list_head` is used without being defined. I think it's hiden in macro. Therefore, I write a `try.c` :

```c

	//try.c
	#include <linux/list.h>

	int main(int argc, char const *argv[])
	{
		LIST_HEAD(hi);
		return 0;
	}
```

Expand the macro with 

	gcc -E try.c -I /usr/src/linux-headers-3.11.0-12-generic/include/ -o try_expand.c

Bingo, in `try_expand.c` is:

```c
	struct list_head {
	 struct list_head *next, *prev;
	};
```

```c
	int main(int argc, char const *argv[])
	{
	 struct list_head hi = { &(hi), &(hi) };
	 return 0;
	}
```

The code of `main` show one way to initialize struct.



## source code

`list.h` can be found in `include/linux/list.h`: [link](list.h)

`try_expand.c` : [link](try_expand.c)

[source]: #source-code