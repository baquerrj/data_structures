/*
 * Animal Array Sorting Kernel Module
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/list_sort.h>
#include <linux/timer.h>

/* Jiffies to calculate elaphsed time */
static unsigned long start, end, elapsed;

/* Static Seed Array for Animal Types */
static char* seed_array[] = {
   "frog", "spider", "shark", "tiger", "lion", "seal", "snake", "shark", "hawk", "dog",
   "cat", "spider", "snake", "lizard", "fish", "toad", "toad", "snake", "shark", "cat",
   "elephant", "shark", "rabbit", "bird", "chicken", "frog", "shark", "lion", "deer", "mouse",
   "shark", "seal", "fish", "rat", "hare", "bull", "cow", "parrot", "baboon", "ape",
   "monkey", "shark", "lion", "rat", "elephant", "dog", "worm", "possum", "monkey", "moose"
};

/* Struct for animal information at each node */
static struct animal {
   char* type;             /* Type of animal */
   unsigned long count;    /* Number of occurrences for this type of animal */
};

/* Linked-list struct for animals */
static struct animals {
   struct animal *node;
   struct list_head list;  /* List of animals */
};

static struct animals *ecosystem;   /* Data structure for ecosystem information */

static struct animals *filtered;    /* Data structure for filtered ecosystem information */

/* Param Definitions */
static char* animal_type = "all";               /* Filter for animal type - default display all */
static unsigned long count_greater_than = 0;    /* Filter for number of occurrences - default display all */


/* Comparison function to sort list alphabetically using list_sort()
 * Returns: -1 if @a should sort before @b,
 *          +1 if @a should sort after @b,
 *           0 if ordering should be preserved
 */
static int cmp(void *priv, struct list_head *a, struct list_head *b)
{
   int ret = 0;
   struct animals *ela, *elb;
   ela = container_of(a, struct animals, list);
   elb = container_of(b, struct animals, list);
   ret = strcmp(ela->node->type, elb->node->type);
   return ret;
}

/* Function to check if animal at curr position is already in ecosystem
 * Returns:  0 if animal does not exist,
 *          +1 otherwise.
 */
static int animals_exist(struct animals *animals_list, int curr)
{
   struct animals *a;
   list_for_each_entry(a, &animals_list->list, list)
   {
      if( 0 == strcmp(seed_array[curr], a->node->type) )
      {
         printk(KERN_INFO "animals_ecosystem(): %s already in ecosystem.\n",
               a->node->type);
         /* Already have animal in list - increment counter for that type of animal */
         a->node->count++;
         return 1;
      }
   }
   return 0;
}

/* Function to initialize unfiltered linked-list for ecosystem
 * Returns: total amount of memory dynamically allocated for nodes,
 *          0 if error is unsuccessful
 */
static int animals_ecosystem(struct animals *animals_list)
{
   printk(KERN_DEBUG "Entered: animals_ecosystem()\n");

   /* Initialize Linked List for Ecosystem */
   INIT_LIST_HEAD(&animals_list->list);

   int i = 0;
   size_t size = sizeof(*ecosystem);
   unsigned long NUM_ENTRIES = sizeof(seed_array) / sizeof(char*); 
   while( i != NUM_ENTRIES )
   {
      if( animals_exist(animals_list, i) )
      {
         i++;
         continue;
      }
      
      /* Have not seen current animal - add it to ecosystem */
      struct animal *new_node;
      new_node = kmalloc(sizeof(*new_node), GFP_KERNEL);
      new_node->type = seed_array[i];
      new_node->count = 1;
      struct animals *a;
      a = kmalloc(sizeof(*a), GFP_KERNEL);
      a->node = new_node;
      /* Add allocated memory to running sum of size */
      size = size + sizeof(*new_node);

      /* Call list_add macro to add new node to Kernel list struct */
      list_add(&a->list, &animals_list->list);
      printk(KERN_DEBUG "animals_ecosystem(): on iter[%u] added %s to ecosystem list.\n",
               i, new_node->type);
      i++;
   }
   printk(KERN_DEBUG "Exiting: animals_ecosystem().\n");
   return size;
}

/* Function to apply animal count filter. Iterates through list
 * and deletes entries that do not match filter.
 * Also frees memory allocated for deleted node.
 * Returns total size of freed memory
 */
static int animals_apply_count_filter(struct animals *animals_list)
{
   struct animals *f;
   struct animals *next;
   size_t size = 0;
   list_for_each_entry_safe(f, next, &animals_list->list, list)
   {
      if( f->node->count < count_greater_than )
      {
         printk(KERN_DEBUG "Removing %s from filtered list.\n",
               f->node->type);
         size = size + sizeof(*f->node);
         list_del(&f->list);
         struct animal *node = f->node;
         kfree(node);
      }
   }
   return size;
}

/* Function to apply animal type filter. Iterates through list
 * and deletes entries that do not match filter.
 * Also frees memory allocated for deleted node.
 * Returns total size of freed memory
 */ 
static int animals_apply_type_filter(struct animals *animals_list)
{
   struct animals *f;
   struct animals *next;
   size_t size = 0;
   list_for_each_entry_safe(f, next, &animals_list->list, list)
   {
      if( 0 != strcmp(f->node->type, animal_type) )
      {
         /* Node type does not match filter - remove it */
         printk(KERN_DEBUG "Removing %s from filtered list.\n",
               f->node->type);
         size = size + sizeof(*f->node);
         list_del(&f->list);
         struct animal *node = f->node;
         kfree(node);
      }
   }
   return size;
}

/* Function to initialize data structure for ecosystem with
 * filters applied: animal_type and/or count_greater_than.
 * Returns: total amount of memory dynamically allocated for nodes,
 *          0 if unsuccessful
 */
static int animals_filtered(void)
{
   printk(KERN_DEBUG "Entered: animals_filtered().\n");

   /* Initialize Filtered List - applying filters along the way */
   INIT_LIST_HEAD(&filtered->list);

   int i = 0;
   size_t size = sizeof(*filtered);
   unsigned long NUM_ENTRIES = sizeof(seed_array) / sizeof(char*);

   /* Initialize filtered list to unfiltered one - apply filters after */
   size = animals_ecosystem(filtered);
   if( 0 < count_greater_than )
   {
      /* Apply count filter */
      size_t tsize = animals_apply_count_filter(filtered);
      if( !tsize )
      {
         /* Decrement size of memory allocated */
         size = size - tsize; 
      }
   }
   if( 0 != strcmp("all", animal_type) )
   {
      /* Apply animal type filter */
      size_t tsize = animals_apply_type_filter(filtered);
      if( !tsize )
      {
         /* Decrement size of memory allocated */
         size = size - tsize;
      }
   }
   
   printk(KERN_DEBUG "Exiting: animals_filtered().\n");
   return size;
}

static int __init animals_init(void)
{
   start = jiffies;

   printk(KERN_INFO "Animals Module:\tInitializing animal constructs.\n");   

   /* Initialize Linked List for Ecosystem */
   ecosystem = kmalloc(sizeof(struct animals), GFP_KERNEL);
   int ret_eco = 0; 
   if( !ecosystem )
   {
      printk(KERN_WARNING "Initialization of ecosystem list failed.\n");
      return -1;
   }
   else
   {
      ret_eco = animals_ecosystem(ecosystem);
      if( ret_eco == 0 )
      {
         return -1;
      }
   }

   /* Sort List alphabetically */
   list_sort(NULL, &ecosystem->list, cmp);

   /* Initialize Filtered Linked List */
   filtered = kmalloc(sizeof(struct animals), GFP_KERNEL);
   int ret_filt = 0;
   if( !filtered )
   {
      printk(KERN_WARNING "Initialization of filtered list failed.\n");
      return -1;
   }
   else
   {
      ret_filt = animals_filtered();
      if( ret_filt == 0 )
      {
         return -1;
      }
   }

   printk(KERN_INFO "Set 1 Report:\n");
   printk(KERN_INFO "\tAllocated a total of %u bytes for ecosystem data structure.\n", ret_eco);
   struct animals *a;
   int num = 0;
   printk(KERN_INFO "\tContents:");
   list_for_each_entry(a, &ecosystem->list, list)
   {
      num++;
      printk(KERN_INFO "\t\t(%s) appears %lu times.\n",
            a->node->type, a->node->count);
   }
   printk(KERN_INFO "\tThere are a total of %u types of animals in ecosystem.\n",
         num);
   
   printk(KERN_INFO "Set 2 Report:\n");
   printk(KERN_INFO "\tAllocated a total of %u bytes for filtered data structure.\n", ret_filt);
   printk(KERN_INFO "\tFilter Criteria:\n");
   printk(KERN_INFO "\t\tanimal type: (%s)\n", animal_type);
   printk(KERN_INFO "\t\tanimals with more than %lu counts.\n", count_greater_than);
   struct animals *f;
   num = 0;
   printk(KERN_INFO "\tContents:");
   list_for_each_entry(f, &filtered->list, list)
   {
      num++;
      printk(KERN_INFO "\t\t(%s) appears %lu times.\n",
            f->node->type, f->node->count);
   }
   printk(KERN_INFO "\tTotal of number of nodes in filtered list is %u\n", num);

   end = jiffies;
   elapsed = end - start;
   printk(KERN_INFO "Animals Module:\tLoaded after %u msecs.\n",
            jiffies_to_msecs(elapsed));
   return 0;
}

static void __exit animals_exit(void)
{
   start = jiffies;
   /* Traverse list - freeing memory for nodes along the way */
   struct animals *a;
   size_t size = 0;
   list_for_each_entry(a, &ecosystem->list, list)
   {
      if( a->node )
      {
         size = size + sizeof(*a->node);
         kfree(a->node);
      }
   }
   size = size + sizeof(*ecosystem);
   kfree(ecosystem);
   printk(KERN_INFO "Freed a total of %u bytes for ecosystem data structure.\n",
         size);

   /* Traveser filtered list - freeing memory for nodes along the way */
   struct animals *f;
   size = 0;
   list_for_each_entry(f, &filtered->list, list)
   {
      if( f->node )
      {
         size = size + sizeof(*f->node);
         kfree(f->node);
      }
   }
   size = size + sizeof(*filtered);
   kfree(filtered);
   printk(KERN_INFO "Freed a total of %u bytes for filtered data structure.\n",
         size);

   end = jiffies;
   elapsed = end - start;
   printk(KERN_INFO "Animals Module:\tTook %u msecs to unload.\n",
         jiffies_to_msecs(elapsed));
}


module_init(animals_init);
module_exit(animals_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Roberto Baquerizo.");
MODULE_DESCRIPTION("Aninal Array Sorting Kernel Module..");

module_param(animal_type, charp, 0);          
module_param(count_greater_than, ulong, 0);    
MODULE_PARM_DESC(animal_type, "Filter for animal type - default display all.");
MODULE_PARM_DESC(count_greater_than, "Filter for number of occurrences - default display all.");
