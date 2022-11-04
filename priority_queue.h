#ifndef PRIORITY_QUEUE
#define PRIORITY_QUEUE

#include "pcb.h"

// DECLARATIONS

struct priority_queue
{
    int currentSize;
    int maxSize;
    Process_Control_Block *heap;
}

void init_queue(int maxSize);

void heapRebuild(struct priority_queue queue, int root);

void insert_pcb(struct Process_Control_Block pcb);

void delete_pcb(struct priority_queue queue);

struct Process_Control_Block get_min_pcb(struct priority_queue queue);

// IMPLEMETATION

void init_queue(struct priority_queue queue, int maxSize)
{
    queue.currentSize = 0;
    queue.maxSize = maxSize;
    queue.heap = malloc(sizeof(struct Process_Control_Block) * maxSize);
}

void heapRebuild(struct priority_queue queue, int root)
{
    int child = 2 * root + 1;

    if (child < queue.currentSize)
    {
        int rightChild = child + 1;

        if (rightChild < queue.size && queue.heap[rightChild].virtual_runtime < queue.heap[child].virtual_runtime)
        {
            child = rightChild;
        }

        if (queue.heap[root].virtual_runtime > queue.heap[child].virtual_runtime)
        {
            struct Process_Control_Block temp = queue.heap[root];
            queue.heap[root] = queue.heap[child];
            queue.heap[child] = temp;

            heapRebuild(queue, child);
        }
    }
}

void insert_pcb(struct priority_queue queue, struct Process_Control_Block pcb)
{
    if (queue.currentSize >= queue.maxSize)
    {
        return;
    }

    queue.heap[queue.currentSize] = pcb;

    int place = queue.currentSize;
    int parent = (place - 1) / 2;

    while ( (place > 0) && (queue.heap[place].virtual_runtime < queue.heap[parent].virtual_runtime))
    {
        struct Process_Control_Block temp = queue.heap[parent];
        queue.heap[parent] = queue.heap[place];
        queue.heap[place] = temp;

        place = parent;
        parent = (place - 1) / 2;
    }
    queue.currentSize++;
}

void delete_pcb(struct priority_queue queue)
{
    if (queue.currentSize <= 0)
    {
        return;
    }

    queue.heap[0] = queue.heap[--queue.size];
    heapRebuild(queue, 0);
}

struct Process_Control_Block get_min_pcb(struct priority_queue queue)
{
    return queue.heap[0];
}


#endif