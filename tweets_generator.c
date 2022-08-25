#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "markov_chain.h"
#define ARG_NUM_ERROR "Usage: num of arguments must be 3 or 4"
#define PATH_ERROR "Error: invalid input file"
#define BASE 10
#define VALID_ARGC_4 4
#define VALID_ARGC_5 5
#define MAX_LEN_TWEET 20
#define MAX_CHRS_LEN 1000


static int compare_string(void* data_v1, void* data_v2)
{
  char * data_s1 = data_v1;
  char * data_s2 = data_v2;
  int num = strcmp (data_s1,data_s2);
  data_s2 = NULL;
  data_s1 = NULL;
  return num;

}
static bool is_last_word_char(void* data)
{
  char * word = data;
  int len_string = strlen (word);
  if(word[len_string-1] == '.')
  {
    return true;
  }
  return false;
}
static void* copy_str(void * data)
{
  char * data_s = data;
  char * new_data = malloc (strlen (data_s)+1);
  if(new_data == NULL)
  {
    return NULL;
  }
  strcpy(new_data, data_s);
  return new_data;
}
static void print_string(void* data)
{
  char * str = data;
  printf ("%s", str);
  if(!is_last_word_char (data))
  {
    printf (" ");
  }

}
static void free_string(void* data)
{
  free(data);
}
static int check_path(char * path)
{
  FILE *input_f;
  input_f= fopen (path, "r");
  if(input_f == NULL)
  {
    return 1;
  }
  fclose (input_f);
  return 0;
}


static bool update_word_in_chain(MarkovChain * markov_chain, char * word,
                                 char *pre_word)
{
  Node * word_node = add_to_database (markov_chain, word);
  if(word_node == NULL)
  {
    // free
    return false;
  }
  if(pre_word == NULL)
  {
    return true;
  }
  if(!is_last_word_char(pre_word))
  {
    Node* node_word = get_node_from_database (markov_chain, word);
    Node* node_pre_word = get_node_from_database (markov_chain,
                                                  pre_word);
    MarkovNode * markov_word = node_word->data;
    MarkovNode * markov_pre_word = node_pre_word->data;
    bool check = add_node_to_counter_list (markov_pre_word,
                                     markov_word, markov_chain);
    if(!check)
    {
      return false;
    }
  }
  return true;

}
static int fill_database(FILE *fp, int words_to_read, MarkovChain*
markov_chain)
{
  char line[MAX_CHRS_LEN];
  int word_counter = 0;
  while(fgets (line, sizeof (line), fp)!=NULL)
  {
    char * previous_word = NULL;
    char *word = NULL;
    word = strtok(line, " \n\r ");
    while(word != NULL)
    {
      if(words_to_read!= -1 && word_counter >= words_to_read)
      {break;}
      if(!update_word_in_chain (markov_chain,word,previous_word))
      {
        return EXIT_FAILURE;
      }
      previous_word = word;
      word = strtok (NULL, " \n\r ");
      word_counter += 1;
    }
  }
  fclose (fp);
  return EXIT_SUCCESS;
}

static void create_tweets(int num, MarkovChain * markov_chain)
{
  for(int i =0; i<num; i++)
  {
    printf ("Tweet %d: ", i+1);
    MarkovNode *first_node = get_first_random_node (markov_chain);
    generate_random_sequence (markov_chain, first_node,
                              MAX_LEN_TWEET);
    printf ("\n");
  }
}

int main (int argc, char* argv[])
{
  if(argc != VALID_ARGC_4 && argc!= VALID_ARGC_5)
  {
    printf(ARG_NUM_ERROR);
    return EXIT_FAILURE;
  }
  int seed = (int)strtol(argv[1], NULL, BASE);
  int num_tweets =(int) strtol(argv[2], NULL, BASE);
  srand (seed);
  char * path = argv[3];
  int words_to_read;
  if(argc ==4)
  {
  words_to_read =-1;
  }
  else
  {
    words_to_read = strtol(argv[4], NULL, BASE);;
  }

  if(check_path (path) == 1)
  {
    printf (PATH_ERROR);
    return EXIT_FAILURE;
  }
  MarkovChain * markov_chain= create_markov_chain(print_string,
    compare_string,free_string, copy_str,
    is_last_word_char );
  if(markov_chain == NULL)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    return EXIT_FAILURE;
  }
  FILE *input_file = fopen (argv[3], "r");
  if(fill_database (input_file,
                    words_to_read, markov_chain) == EXIT_FAILURE)
  {
    printf (ALLOCATION_ERROR_MASSAGE);
    free_markov_chain (&markov_chain);
    return EXIT_FAILURE;
  }
  create_tweets (num_tweets, markov_chain);
  free_markov_chain (&markov_chain);
  return EXIT_SUCCESS;

}