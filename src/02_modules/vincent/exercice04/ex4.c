/* skeleton.c */
#include <linux/module.h>	/* needed by all modules */
#include <linux/init.h>		/* needed for macros */
#include <linux/kernel.h>	/* needed for debugging */

#include <linux/moduleparam.h>	/* needed for module parameters */

#include <linux/slab.h>		/* needed for dynamic memory allocation */
#include <linux/list.h>		/* needed for linked list processing */
#include <linux/string.h>	/* needed for string handling */

static char* text = "dummy text";
module_param(text, charp, 0);
static int  elements = 0;
module_param(elements, int, 0);

struct element {
	char text[64];
	int number;
	struct list_head list;
};

static LIST_HEAD(elem_list);

static int __init ex4_init(void)
{
	int i;
	struct element* elem;
	pr_info("Hello exercice04\n");
	for (i = 0; i < elements; i++) {
		elem = kzalloc(sizeof(*elem), GFP_KERNEL);
		if (elem != NULL) {
			strncpy(elem->text, text, sizeof(elem->text) - 1);
			elem->number = i+1;
			list_add_tail(&elem->list, &elem_list);
		}
	}
	return 0;
}

static void __exit ex4_exit(void)
{
	struct element* elem;
	pr_info("Goodbye exercice04\n");
	while (!list_empty(&elem_list)) {
		elem = list_entry(elem_list.next, struct element, list);
		pr_info("Element %d: %s\n", elem->number, elem->text);
		list_del(&elem->list);
		kfree(elem);
	}
}

module_init (ex4_init);
module_exit (ex4_exit);

MODULE_AUTHOR ("Vincent Audergon <vincent.audergon@master.hes-so.ch>");
MODULE_DESCRIPTION ("Module ex4");
MODULE_LICENSE ("GPL");

