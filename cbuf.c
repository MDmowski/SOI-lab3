#include "cbuf.h"
#include <assert.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>

#pragma once

struct circular_buf_t
{
    int *buffer;
    size_t head;
    size_t tail;
    size_t max; // of the buffer
    bool full;
};

cbuf_handle_t circular_buf_init(cbuf_handle_t cbuf, int *buffer, size_t size)
{
    assert(buffer && size);
    assert(cbuf);

    cbuf->buffer = buffer;
    cbuf->max = size;
    circular_buf_reset(cbuf);

    assert(circular_buf_empty(cbuf));

    return cbuf;
}

void circular_buf_reset(cbuf_handle_t cbuf)
{
    assert(cbuf);

    cbuf->head = 0;
    cbuf->tail = 0;
    cbuf->full = false;
}

void circular_buf_clear(cbuf_handle_t cbuf)
{
    assert(cbuf);
    if (!circular_buf_empty(cbuf))
    {
        for (int i = cbuf->tail; i < cbuf->tail + circular_buf_size(cbuf); i++)
        {
            int id = cbuf->buffer[i % cbuf->max];
            shmctl(id, IPC_RMID, NULL);
        }
    }
    puts("BUFFER CLEARED");
}

bool circular_buf_full(cbuf_handle_t cbuf)
{
    assert(cbuf);

    return cbuf->full;
}

bool circular_buf_empty(cbuf_handle_t cbuf)
{
    assert(cbuf);

    return (!cbuf->full && (cbuf->head == cbuf->tail));
}

size_t circular_buf_capacity(cbuf_handle_t cbuf)
{
    assert(cbuf);

    return cbuf->max;
}

size_t circular_buf_size(cbuf_handle_t cbuf)
{
    assert(cbuf);

    size_t size = cbuf->max;

    if (!cbuf->full)
    {
        if (cbuf->head >= cbuf->tail)
        {
            size = (cbuf->head - cbuf->tail);
        }
        else
        {
            size = (cbuf->max + cbuf->head - cbuf->tail);
        }
    }

    return size;
}

static void advance_pointer(cbuf_handle_t cbuf)
{
    assert(cbuf);

    if (cbuf->full)
    {
        cbuf->tail = (cbuf->tail + 1) % cbuf->max;
    }

    cbuf->head = (cbuf->head + 1) % cbuf->max;
    cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer(cbuf_handle_t cbuf)
{
    assert(cbuf);

    cbuf->full = false;
    cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

int circular_buf_put_tail(cbuf_handle_t cbuf, int data)
{
    int r = -1;

    assert(cbuf && cbuf->buffer);

    if (!circular_buf_full(cbuf))
    {
        size_t newTail;
        if (cbuf->tail != 0)
            newTail = cbuf->tail - 1;
        else
            newTail = cbuf->max - 1;
        cbuf->buffer[newTail] = data;
        cbuf->tail = newTail;
        cbuf->full = (cbuf->head == cbuf->tail);
        r = 0;
    }

    return r;
}

int circular_buf_put2(cbuf_handle_t cbuf, int data)
{
    int r = -1;
    assert(cbuf && cbuf->buffer);
    if (!circular_buf_full(cbuf))
    {
        cbuf->buffer[cbuf->head] = data;
        advance_pointer(cbuf);
        r = 0;
    }

    return r;
}

char *circular_buf_get(cbuf_handle_t cbuf)
{
    assert(cbuf && cbuf->buffer);

    char *data;

    if (!circular_buf_empty(cbuf))
    {
        int id = cbuf->buffer[cbuf->tail];
        data = (char *)shmat(id, NULL, 0);
        shmctl(id, IPC_RMID, NULL);
        retreat_pointer(cbuf);
    }

    return data;
}

void circular_buf_print(cbuf_handle_t cbuf)
{
    assert(cbuf);
    if (!circular_buf_empty(cbuf))
    {
        printf("|");
        for (int i = cbuf->tail; i < cbuf->tail + circular_buf_size(cbuf); i++)
        {
            int id = cbuf->buffer[i % cbuf->max];
            char *s = (char *)shmat(id, NULL, 0);
            printf(" %s |", s);
            //shmctl(id, IPC_RMID, NULL);
        }
        printf("\n");
    }
    else
    {
        printf("EMPTY\n");
    }
}
