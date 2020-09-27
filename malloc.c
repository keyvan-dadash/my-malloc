#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define META_SIZE sizeof(block_meta)

typedef struct {
    size_t size;
    struct block_meta *next;
    int free;
    int magic;
    int id;
} block_meta;

void *global_base = NULL;

block_meta *find_free_space(block_meta **last, size_t size)
{
    block_meta *curr = global_base;
    while (curr && !(curr->size >= size && curr->free))
    {
        *last = curr;
        curr = curr->next;
    }
    return curr;
};

block_meta *request_space(block_meta *last, size_t size)
{
    block_meta *block;
    block = sbrk(0);
    void * request = sbrk(size + META_SIZE);
    assert((void*)block == request);
    if (request == (void *)-1) {
        return NULL;
    }

    if (last) {
        last->next = block;
    }

    block->size = size;
    block->free = 0;
    block->magic = 0x12345678;
    block->next = NULL;
    return block;
}

void *my_malloc(size_t size)
{
    block_meta *block;
    if (size <= 0) {
        return NULL;
    }

    if (!global_base) {
        block = request_space(NULL, size);
        if (!block) {
            return NULL;
        }
        block->id = 85;
        global_base = block;
    } else {
        block_meta *last = global_base;
        printf("id of global is %d\n", last->id);
        block = find_free_space(&last, size);
        if (!block) {
            block = request_space(last, size);
            if (!block) {
                return NULL;
            }
        } else {
            block->free = 0;
            block->magic = 0x77777777;
        }
    }
    return (block + 1);
}

block_meta *get_block_ptr(void * blockPtr) {
    return (block_meta *)blockPtr - 1;
}

void my_free(void *ptr)
{
    if (!ptr) {
        return;
    }

    block_meta *blockPtr = get_block_ptr(ptr);
    assert(blockPtr->free == 0);
    assert(blockPtr->magic == 0x77777777 || blockPtr->magic == 0x12345678);
    blockPtr->free =1;
    blockPtr->magic = 0x55555555;
}











int main(int argc, char const *argv[])
{
    int *ptr = my_malloc(sizeof(int));

    *ptr = 5;
    printf("ptr before free is %d\n", *ptr);

    my_free(ptr);
    printf("ptr after free is %d\n", *ptr);

    int *anotherPtr = my_malloc(sizeof(int));
    *anotherPtr = 7;

    printf("ptr after free and reassign is %d\n", *ptr);
    printf("anotherPtr before free is %d\n", *anotherPtr);

    int *anotherPtr1 = my_malloc(sizeof(int));
    *anotherPtr1 = 17;

    printf("ptr after free and reassign is %d\n", *ptr);
    printf("anotherPtr before free is %d\n", *anotherPtr);
    printf("anotherPtr1 before free is %d\n", *anotherPtr1);

    my_free(ptr);
    printf("checking duble free id debugger\n");

    return 0;
}
