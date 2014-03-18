#include <linux/list.h>

int main(int argc, char const *argv[])
{
	LIST_HEAD(hi);
	int a = 3;
	list_entry(&hi, struct list_head, hi);
	return 0;
}