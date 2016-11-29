#include "scc.h"
#include "cc.h"

void tarjan(ind *index_out, list_node *buffer_out, uint32_t size_out) {

    Stack stack;
    ht_Node *explored = createHashtable(HT_BIG);
    list_node *neighbors_out;
    ind *cur_ind = NULL;
    ptrdiff_t offset_out;
    uint32_t size = size_out;
    uint32_t i = 0, v = 0, k = 0;

    stack.last = NULL;

    for(i = 0 ; i < size ; i++) {
        if(lookup(index_out, i, size_out) == NOT_EXIST) continue;
        if(search(explored, i, HT_BIG) == FOUND) continue;

        push(&stack, i);
        while(!stackIsEmpty(&stack)) {
            v = pop(&stack);

            if(search(explored, v, HT_BIG) == NOT_FOUND) {
                insert(explored, v, HT_BIG);
                offset_out = getListHead(index_out, v);
                neighbors_out = buffer_out + offset_out;

                if(offset_out >= 0) {
                    k = 0;
                    while(k < N) {
                        if(neighbors_out->neighbor[k] == DEFAULT) break;
                        push(&stack, neighbors_out->neighbor[k]);
                        k++;
                        if(k == N && neighbors_out->nextListNode != -1) {
                            neighbors_out = buffer_out + neighbors_out->nextListNode;
                            k = 0;
                        }
                    }
                }
            }
        }
    }






}