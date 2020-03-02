#ifndef CBUFF_H_
#define CBUFF_H_
#pragma once

#include <stdbool.h>
#include <stdlib.h>

#define BUF_A_KEY 1112
#define CBUF_A_KEY 1113

#define BUF_B_KEY 2223
#define CBUF_B_KEY 2224

#define BUF_C_KEY 3334
#define CBUF_C_KEY 3335

// Opaque circular buffer structure
typedef struct circular_buf_t circular_buf_t;

// Handle type, the way users interact with the API
typedef circular_buf_t *cbuf_handle_t;

// User provides struct
cbuf_handle_t circular_buf_init(cbuf_handle_t cbuf, int *buffer, size_t size);

// Free a circular buffer structure.
// Does not free data buffer; owner is responsible for that
void circular_buf_clear(cbuf_handle_t cbuf);

// Reset the circular buffer to empty, head == tail
void circular_buf_reset(cbuf_handle_t cbuf);

// Put version 1 continues to add data if the buffer is full
// Old data is overwritten
void circular_buf_put(cbuf_handle_t cbuf, int data);

// Put Version 2 rejects new data if the buffer is full
// Returns 0 on success, -1 if buffer is full
int circular_buf_put2(cbuf_handle_t cbuf, int data);

// Put_tail add data at the tail
// Old data is overwritten
int circular_buf_put_tail(cbuf_handle_t cbuf, int data);

// Retrieve a value from the buffer
// Returns 0 on success, -1 if the buffer is empty
char *circular_buf_get(cbuf_handle_t cbuf);

// Returns true if the buffer is empty
bool circular_buf_empty(cbuf_handle_t cbuf);

// Returns true if the buffer is full
bool circular_buf_full(cbuf_handle_t cbuf);

// Returns the maximum capacity of the buffer
size_t circular_buf_capacity(cbuf_handle_t cbuf);

// Returns the current number of elements in the buffer
size_t circular_buf_size(cbuf_handle_t cbuf);

#endif