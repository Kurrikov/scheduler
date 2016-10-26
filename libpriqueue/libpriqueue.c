/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
  q->size = 0;
  q->front = NULL;
  q->comparer = comparer;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue,
    where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
  // initialize a node
  noodle_t *new_noodle = malloc(sizeof(noodle_t));
  new_noodle->pasta = ptr;
  new_noodle->next_noodle = NULL;
  
  if (q->size == 0) {
    // set the front of the queue to new noodle
    q->front = new_noodle;
  } else {
    // place new_noodle in proper location
    noodle_t *temp_noodle = q->front;
    noodle_t *lastgood_noodle = q->front;

    while (temp_noodle != NULL) {
      if(q->comparer(ptr, temp_noodle->pasta) < 0) {
        if (temp_noodle == q->front) {
          // insert at the front if necessary
          new_noodle->next_noodle = q->front;
          q->front = new_noodle;
        } else {
          // insert between previous and temp
          lastgood_noodle->next_noodle = new_noodle;
          new_noodle->next_noodle = temp_noodle;
        }
      } else {
        // shift down the list
        lastgood_noodle = temp_noodle;
        temp_noodle = temp_noodle->next_noodle;

        if (temp_noodle == NULL) {
          // insert at the end if the list has been exhausted
          lastgood_noodle->next_noodle = new_noodle;
        }
      }
    }
  }
  // q->size++;
	return ++(q->size);
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
  if (q->size == 0) {
    return NULL;
  } else {
    return q->front;
  }
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the data at the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
  if (q->size == 0) {
    return NULL;
  } else {
    // get data stored in first element of the queue
    noodle_t *front = q->front;
    void *ptr = q->front->pasta;

    // set the new front of the queue
    if (q->front->next_noodle == NULL) {
      q->front = NULL;
    } else {
      q->front = q->front->next_noodle;
    }

    free(front);
    --(q->size);
    return ptr;
  }
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
  if (q->size == 0 || index < 0 || index >= (int)q->size) {
  	return NULL;
  }

  int i = 0;
  noodle_t *temp_noodle = q->front;
  while (i <= index) {
    temp_noodle = temp_noodle->next_noodle;
    ++i;
  }

  return temp_noodle;
}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained
  in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
  if (q->size == 0) {
    return 0;
  }

  noodle_t *lastgood = q->front;
  noodle_t *temp_noodle = q->front;
  int count = 0;

  while(temp_noodle != NULL){
    if (temp_noodle->pasta == ptr) {
      // if node should be removed
      if (temp_noodle == q->front) {
        // if the front is being removed
        q->front = lastgood->next_noodle;
        lastgood = q->front;
        free(temp_noodle);
        ++count;
        temp_noodle = lastgood;
      } else {
        // removing non-front element
        lastgood->next_noodle = temp_noodle->next_noodle;
        free(temp_noodle);
        ++count;
        temp_noodle = lastgood;
      }
    } else {
      // node should not be removed
      lastgood = temp_noodle;
      temp_noodle = temp_noodle->next_noodle;
    }
  }
  q->size -= count;
	return count;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	return 0;
}


/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{

}
