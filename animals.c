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

static struct animal {
   char* type;
   unsigned long count;
};

static struct animals {
   struct animal *node;
   struct list_head list;  /* List of animals */
};

static unsigned long num_animals;   /* Number of animals in ecosystem */

static struct animals *ecosystem;

/* Comparison function to sort list alphabetically using list_sort()
 * Returns: -1 if @a should sort before @b
 *          +1 if @a should sort after @b
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
 * Returns: 0 if animal does not exist
 */
static int animals_exist(struct animals *animals_list, int curr)
{
   struct animals *a;
   list_for_each_entry(a, &animals_list->list, list)
   {
      if( 0 == strcmp(seed_array[curr], a->node->type) )
      {
         printk(KERN_DEBUG "animals_ecosystem(): %s already in ecosystem.\n",
               a->node->type);
         /* Already have animal in list - increment counter for that type of animal */
         a->node->count++;
         return 1;
      }
   }
   return 0;
}

/* Function to initialize data structure for ecosystem
 * Returns total amount of memory dynamically allocated for nodes
 * Return 0 if error is unsuccessful */
static int animals_ecosystem(void)
{
   printk(KERN_DEBUG "Entered: animals_ecosystem()\n");
   /* Initialize Linked List for Ecosystem */
   unsigned long NUM_ENTRIES = sizeof(seed_array) / sizeof(char*);  /* Number of entries - duplicates included */
   INIT_LIST_HEAD(&ecosystem->list);

   int i = 0;
   size_t size = sizeof(*ecosystem);
   while( i != NUM_ENTRIES )
   {
      if( animals_exist(ecosystem, i) )
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
      num_animals++;
      /* Add allocated memory to running sum of size */
      size = size + sizeof(*new_node);

      /* Call list_add macro to add new node to Kernel list struct */
      list_add(&a->list, &ecosystem->list);
      printk(KERN_DEBUG "animals_ecosystem(): on iter[%u] added %s to ecosystem list.\n",
               i, new_node->type);
      i++;
   }
   printk(KERN_DEBUG "Exiting: animals_ecosystem()\n");
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
      ret_eco = animals_ecosystem();
      if( ret_eco == 0 )
      {
         return -1;
      }
   }

   /* Sort List alphabetically and report contents */
   list_sort(NULL, &ecosystem->list, cmp);
   printk(KERN_INFO "There are a total of %lu types of animals in ecosystem.\n",
         num_animals);
   printk(KERN_INFO "Allocated a total of %u bytes for ecosystem data structure.\n",
         ret_eco);
   struct animals *a;
   list_for_each_entry(a, &ecosystem->list, list)
   {
      printk(KERN_INFO "Animal %s appears %lu times.\n",
            a->node->type, a->node->count);
   }


#if 0
   /* Initialize Filtered Linked List */
   struct animals_list *filtered;
   filtered = kmalloc(sizeof(*filtered), GFP_KERNEL);
   int ret_filt = animals_filtered(&filtered);
   if( ret_filt == 0 )
   {
      return -1;
   }
#endif

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
















