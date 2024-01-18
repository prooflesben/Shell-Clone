/**
 * Vector implementation.
 *
 * - Implement each of the functions to create a working growable array (vector).
 * - Do not change any of the structs
 * - When submitting, You should not have any 'printf' statements in your vector 
 *   functions.
 *
 * IMPORTANT: The initial capacity and the vector's growth factor should be 
 * expressed in terms of the configuration constants in vect.h
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vect.h"

/** Main data structure for the vector. */
struct vect {
  char **data;             /* Array containing the actual data. */
  unsigned int size;       /* Number of items currently in the vector. */
  unsigned int capacity;   /* Maximum number of items the vector can hold before growing. */
};

/** Construct a new empty vector. */
vect_t *vect_new() {


  vect_t *v = malloc(sizeof(vect_t));
  v->size = 0;
  v->capacity = VECT_INITIAL_CAPACITY;
  v->data = malloc(v->capacity * sizeof(char*));
  
  return v;
}

/** Delete the vector, freeing all memory it occupies. */
void vect_delete(vect_t *v) {
  for (int i = 0; i < v->size; i++) {
    free(v->data[i]);
  }
  free(v->data);
  free(v);
}

/** Get the element at the given index. */
const char *vect_get(vect_t *v, unsigned int idx) {
  assert(v != NULL);
  assert(idx < v->size);

  return v->data[idx];
}

/** Get a copy of the element at the given index. The caller is responsible
 *  for freeing the memory occupied by the copy. */
char *vect_get_copy(vect_t *v, unsigned int idx) {
  assert(v != NULL);
  assert(idx < v->size);
  
  char *copy = malloc((strlen(v->data[idx]) + 1) * sizeof(char));
  strncpy(copy, v->data[idx], strlen(v->data[idx]) + 1); 
  return copy;
}

/** Set the element at the given index. */
void vect_set(vect_t *v, unsigned int idx, const char *elt) {
  assert(v != NULL);
  assert(idx < v->size);
  
  v->data[idx] = realloc(v->data[idx], (strlen(elt) + 1) * sizeof(char));
  strncpy(v->data[idx], elt, strlen(elt) + 1);
}

/** Add an element to the back of the vector. */
void vect_add(vect_t *v, const char *elt) {
  assert(v != NULL);

  if (v->size == v->capacity) {
    v->capacity = v->capacity * VECT_GROWTH_FACTOR;
    v->data = realloc(v->data, v->capacity * sizeof(char*));
  }
  // Allocate memory for the new data
  v->data[v->size] = malloc(strlen(elt) + 1);

  // Copies the element to add to the allocated memory
  strncpy(v->data[v->size], elt, strlen(elt) + 1);
  v->size++;
}

/** Remove the last element from the vector. */
void vect_remove_last(vect_t *v) {
  assert(v != NULL);

  if (v->size > 0) {
    free(v->data[v->size - 1]);
    v->size--;
  }
}

/** The number of items currently in the vector. */
unsigned int vect_size(vect_t *v) {
  assert(v != NULL);
  return v->size;
}

/** The maximum number of items the vector can hold before it has to grow. */
unsigned int vect_current_capacity(vect_t *v) {
  assert(v != NULL);
  return v->capacity;
}

/** Copies the vector up to end and starting from start and return the copied vector */
vect_t *copy_vect_abstract(vect_t *prev, vect_t *new_vect, int start, int end) {
  assert(new_vect != NULL);
  if(prev != NULL){
    vect_delete(prev);
  }
  prev = vect_new();

  for (int i = start; i < end; i++) {
    vect_add(prev, vect_get(new_vect, i));
  }

  return prev;
}

/** Copies the whole vector and return the copied vector */
vect_t *copy_vect(vect_t *prev, vect_t *new_vect) {
  return copy_vect_abstract(prev,new_vect,0,vect_size(new_vect));
}

/** Copies the vector after inclusive and return the copied vector */
vect_t *copy_vect_after(vect_t *prev, vect_t *new_vect, int start) {
  return copy_vect_abstract(prev,new_vect,start,vect_size(new_vect));
}

/** Copies the vector until exclusive specified index and return the copied vector */
vect_t *copy_vect_until(vect_t *prev, vect_t *new_vect,int end) {
  return copy_vect_abstract(prev,new_vect,0,end);
}

// Finds the first occurence of the string and if not found returns -1
int indexOf(vect_t *v, const char *target){
   assert(v != NULL);
   for (int i = 0; i < vect_size(v); i++) {
    if(strcmp(vect_get(v, i), target) == 0){
      return i;
    }
  }

   return -1;
}
