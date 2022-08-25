#include <stdlib.h>
#include "linked_list.h"
#include "markov_chain.h"
#include <assert.h>
#include <string.h>

int get_random_number(int max_number)
{
  return rand() % max_number;
}

void generate_random_sequence(MarkovChain *markov_chain, MarkovNode *
first_node, int max_length)
{
  int count = 1;
  void *first_word = first_node->data;
  markov_chain->print_func(first_word);
  while(count < max_length)
  {

    MarkovNode * next_markov = get_next_random_node (first_node);
    while (next_markov == NULL)
    {
      next_markov = get_first_random_node (markov_chain);
    }
    void * next_word = next_markov->data;

    markov_chain->print_func (next_word);
    count++;
    first_node = next_markov;
    if(markov_chain->is_last(next_word))
    {
      break;
    }
  }
}
MarkovNode* get_next_random_node(MarkovNode *state_struct_ptr)
{
  NextNodeCounter * start = state_struct_ptr->counter_list;
  int max = state_struct_ptr->sum_freq;
  int i = get_random_number (max);
  int size = state_struct_ptr->counter_size;
  int sum_of_freq = 0;
  for(int j =0; j<size; j++)
  {
    NextNodeCounter temp_next_counter = start[j];
    sum_of_freq += temp_next_counter.frequency;
    if (i < sum_of_freq)
    {
      MarkovNode *markov_node = temp_next_counter.markov_node;
      return markov_node;
    }
  }
  return state_struct_ptr;

}
MarkovNode * get_i_word_in_chain(MarkovChain * chain, int n)
{
  LinkedList * list = chain->database;
  Node * temp = list->first;
  int i = 0;
  while(i < n)
  {
    temp = temp->next;
    i++;
  }
  MarkovNode * markov_node = temp->data;
  return markov_node;
}
void free_markov_chain(MarkovChain ** ptr_chain)
{
  MarkovChain * markov_chain= *ptr_chain;
  if(markov_chain == NULL)
  {
    return;
  }
  LinkedList * list = markov_chain->database;
  Node * temp = list->first;
  while (temp != NULL)
  {
    Node * temp_next =temp->next;
    free_markov_node (temp->data, markov_chain);
    free(temp->data);
    temp->data = NULL;
    free (temp);
    temp = temp_next;
  }
  free (list);
  (*ptr_chain)->database = NULL;
  free (markov_chain);
  *ptr_chain = NULL;
}
MarkovNode* get_first_random_node (MarkovChain *markov_chain)
{
  int chain_size = markov_chain->database->size;
  int i = get_random_number (chain_size);
  MarkovNode * markov_node = get_i_word_in_chain (markov_chain, i);
  void * word = markov_node->data;
  while (markov_chain->is_last(word))
  {
    i = get_random_number (chain_size);
    markov_node = get_i_word_in_chain (markov_chain, i);
    word = markov_node->data;
  }
  return markov_node;
}

MarkovNode * create_markov_node(void * word, copy_func copy)
{
  MarkovNode * new_node = malloc (sizeof (MarkovNode));
  if(new_node == NULL)
  {
    return NULL;
  }
  new_node->data=copy( word);
  if(new_node->data == NULL)
  {
    free(new_node);
    new_node = NULL;
    return NULL;}
  new_node->counter_list = NULL;
  new_node->counter_size =0;
  return new_node;
}

void create_next_counter(NextNodeCounter * node_counter,
                         MarkovNode * markov_node)
{
  node_counter->markov_node = markov_node;
  node_counter->frequency = 1;
}

NextNodeCounter *is_in_counter(MarkovNode *first_node, MarkovNode
*second_node, comp_func compare)
{

  NextNodeCounter *start = first_node->counter_list;
  int size = first_node->counter_size;
  for(int i=0; i<size; i++)
  {
    NextNodeCounter temp_new_counter = start[i];
    MarkovNode * temp_markov_node = temp_new_counter.markov_node;
    void * word = temp_markov_node->data;
    void * word_compare_to = second_node->data;
    if(compare (word, word_compare_to) == 0)
    {return start+i;}
  }
  return NULL;
}

bool add_node_to_counter_list(MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain)
{
  if(first_node->counter_size == 0)
  {
    NextNodeCounter  node_counter ;
    node_counter.markov_node = second_node;
    node_counter.frequency = 1;
    first_node->counter_list = malloc (sizeof (NextNodeCounter));

    if(first_node->counter_list == NULL)
    {
      return false;
    }
    first_node->counter_list[0] = node_counter;
    first_node->counter_size += 1;
    first_node->sum_freq = 1;
    return true;
  }
  NextNodeCounter * node_counter = is_in_counter (first_node,second_node,
                                          markov_chain->comp_func);
  if(node_counter != NULL)
  {
    node_counter->frequency +=1;
    first_node->sum_freq +=1;
    return true;
  }
  NextNodeCounter new_mode_counter;
  create_next_counter (&new_mode_counter ,second_node);
  first_node->counter_size += 1;
  int capacity = first_node->counter_size;
  NextNodeCounter * temp = realloc (first_node->counter_list, capacity*
  sizeof (NextNodeCounter));
  if(temp == NULL)
  {
//    free counter?
    return false;
  }
  first_node->counter_list = temp;
  (first_node->counter_list)[capacity-1] = new_mode_counter;
  first_node->sum_freq +=1;
  return true;

}

MarkovChain * create_markov_chain(print_func print, comp_func compare,
                                  free_func free_f, copy_func copy, is_last
                                  is_last_f)
{
  MarkovChain * chain =NULL;
  chain = malloc (sizeof (MarkovChain));
  if(chain == NULL)
  { return NULL;}
  LinkedList *linked_list = create_linked_list();
  if(linked_list == NULL)
  {
    free (chain);
    return NULL;
  }
  chain->database = linked_list;
  chain->print_func = print;
  chain->comp_func = compare;
  chain->free_data = free_f;
  chain->copy_func = copy;
  chain->is_last = is_last_f;
  return chain;
}
void free_markov_node(MarkovNode * markov_node, MarkovChain * markov_chain)
{

  markov_chain->free_data(markov_node->data);
  free (markov_node->counter_list);
}


Node* add_to_database(MarkovChain *markov_chain, void *data_ptr)
{
  Node *node = get_node_from_database (markov_chain, data_ptr);
  if (node != NULL)
  {
    return node;
  }
  MarkovNode *new_node = create_markov_node (data_ptr ,
                                             markov_chain->copy_func);
  if (new_node == NULL)
  {
    return NULL;
  }
  int check = add (markov_chain->database, new_node);
  if (check == 1)
  {
    free_markov_node (new_node, markov_chain);
    return NULL;
  }
  return markov_chain->database->last;
}

LinkedList * create_linked_list()
{
  LinkedList * linked_list = malloc (sizeof(LinkedList));
  if(linked_list == NULL)
  {return NULL;}
  linked_list->size = 0;
  linked_list->first = NULL;
  linked_list->last = NULL;
  return linked_list;
}


Node* get_node_from_database(MarkovChain *markov_chain, void *data_ptr)
{
  Node *temp = markov_chain->database->first;
  while (temp != NULL)
  {
    if (markov_chain->comp_func(temp->data->data, data_ptr) == 0)
    {
      return temp;
    }
    temp = temp->next;
  }
  return NULL;
}

