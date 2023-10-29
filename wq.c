/**
 * @file wq.c
 * @brieff Implementation of a work queu to store client sockets.
 *
 * This file provides functions to initialize, push to, and pop from a work
 * queue. It leverages teh utlist.h macros for doubly-linked list manipulations.
 */
#include "wq.h"
#include "utlist.h"
#include <pthread.h>
#include <stdlib.h>

/**
 * @brief Initializes a work queue.
 *
 * Sets up the necessary components for thw work queue such as
 * its size, head pointer, mutex, and condition variable.
 *
 * @param wq Pointer to the work queue to be initialized.
 */
void wq_init(wq_t *wq) {
  wq->size = 0;
  wq->head = NULL;

  pthread_mutex_init(&(wq->mutex), NULL);
  pthread_cond_init(&(wq->cond), NULL);
}

/**
 * @brief Pops a client socket descriptor from the work queue.
 *
 * Removes and returns the socket descriptor from the front of the queue.
 * If the queue is empty, the calling thread will block until a socket is
 * available.
 *
 * @param wq Pointer to the work queue.
 * @return The client socket descriptor from the fron of the queue.
 */
int wq_pop(wq_t *wq) {

  pthread_mutex_lock(&(wq->mutex));

  /* Wait until there's an item in the queue. */
  while (wq->size == 0) {
    pthread_cond_wait(&(wq->cond), &(wq->mutex));
  }

  wq_item_t *wq_item = wq->head;
  int client_socket_fd = wq->head->client_socket_fd;
  wq->size--;
  DL_DELETE(wq->head, wq->head);

  pthread_mutex_unlock(&(wq->mutex));

  free(wq_item);
  return client_socket_fd;
}

/**
 * @brief Pushes a new client socket descriptor onto the work queue.
 *
 * Adds the socket descriptor to the end of teh queue and
 * signals any waiting threads.
 *
 * @param wq Pointer to the work queue.
 * @param client_socket_fd Client socket descriptor to be added to the queue.
 */
void wq_push(wq_t *wq, int client_socket_fd) {
  pthread_mutex_lock(&(wq->mutex));

  wq_item_t *wq_item = calloc(1, sizeof(wq_item_t));
  wq_item->client_socket_fd = client_socket_fd;
  DL_APPEND(wq->head, wq_item);
  wq->size++;

  pthread_cond_signal(&(wq->cond));
  pthread_mutex_unlock(&(wq->mutex));
}
