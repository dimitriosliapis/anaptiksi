#include "thread.h"
#include <string.h>

int toID(char *str, uint32_t *N1, uint32_t *N2) {
    uint32_t i, j, id;
    if (str == NULL)
        return 0;
    if ((str[0] < '0') || (str[0] > '9'))
        i = 2;
    else
        i = 0;
    for (j = 0; j < 2; j++) {
        id = 0;
        while ((str[i] >= '0') && (str[i] <= '9')) {
            id = 10 * id + (str[i] - '0');
            i++;
        }
        if (j == 0)
            *N1 = id;
        else
            *N2 = id;
        i++;
    }
    return 1;
}

void place_to_buffer(char *query, Buffer *buffer, int line) {

    int i = 0;
    int realloc_size;
    char *new_ptr = NULL;
    B_Node *new = NULL, *tmp = NULL;

  //  printf("INSERTING QUERY: %s\n", query);

    if(query == NULL || buffer == NULL) return;

    new = malloc(sizeof(B_Node));
    strcpy(new->query, query);
    new->next = NULL;
    new->line = line;

    if(buffer->first == NULL) {
        buffer->first = new;
        buffer->last = new;
        return;
    }

    tmp = buffer->last;
    if(tmp != NULL) tmp->next = new;
    buffer->last = new;
    /*
    if(buffer->count == buffer->size) {
        realloc_size = 2*buffer->size;
        new_ptr = realloc(buffer->query, realloc_size*sizeof(char*));
        for(i = buffer->size; i < realloc_size; i++){
            new_ptr[i] = malloc(64*sizeof(char));
        }
        if(new_ptr != NULL) {
            buffer->query = new_ptr;
            buffer->size = realloc_size;
        }
        else printf("Buffer full.\n");
    }
    buffer->end = (buffer->end + 1)%buffer->size;
    strcpy(buffer->query[buffer->end], query);
    buffer->count++;
    buffer->line = line;*/
}

int remove_from_buffer(Buffer *buffer, int *line, char **query) {

    B_Node *tmp = NULL;

    //query = malloc(64*sizeof(char));
    memset(*query, '\0', 64);

    if(buffer->first != NULL){
        strcpy(*query,buffer->first->query);
        *line = buffer->first->line;

       // printf("GOT QUERY: %s\n", query);

        tmp = buffer->first;
        buffer->first = tmp->next;
        free(tmp);
        return 1;
    }
    else return -1;

    /*query = malloc(64*sizeof(char));
    memset(query, '\0', 64);


    if(buffer->count == 0)
        return NULL;

    strcpy(query, buffer->query[buffer->start]);
    buffer->start = (buffer->start + 1)%buffer->size;
    buffer->count--;
    *line = buffer->line;
    return query;*/
}

void *master_thread_function(void *ptr) {
    arg *local = ptr;
    char str[64];
    int line = 1, a;
    int realloc_size = 0;
    int start = 1;

    pthread_t *worker_threads = (pthread_t *)malloc(THREAD_POOL_SIZE*sizeof(pthread_t));


    //local->res_size = 0;
    finished = 0;
    status = START;
    max_id = 1;


    pthread_mutex_lock(&vmutex);

    for(a = 0 ; a < THREAD_POOL_SIZE ; a++) pthread_create(&worker_threads[a], NULL, worker_thread_function, local);

    fgets(str,sizeof(str), local->file);
    fgets(str,sizeof(str), local->file);



    pthread_mutex_unlock(&vmutex);

    while (!feof(local->file)) {

        pthread_mutex_lock(&mutex);

        while(str[0] != 'F'){

            place_to_buffer(str, local->buffer, line);

            if(start) {
                pthread_mutex_unlock(&mutex);
                start = 0;
            }

            //if(status == START) status = CONTINUE;
            line++;
 /*           if(line == 32708){
                printf("Asgaeg\n");
            }*/
            if(line == local->res_size){

                realloc_size = 2*local->res_size;
                //while(line >= realloc_size) realloc_size *=2;
                local->results = realloc(local->results, realloc_size*sizeof(int));
                for(a = local->res_size; a < realloc_size; a++) local->results[a] = EMPTY;
                local->res_size = realloc_size;
            }

            fgets(str,sizeof(str), local->file);
        }
        // status = CONTINUE;
        //while(status != START) {
        pthread_mutex_lock(&mutex);
        pthread_cond_wait(&cond_next, &mutex);
        pthread_mutex_unlock(&mutex);
        start = 1;

        //pthread_cond_broadcast(&cond_start);




        //}
        fgets(str,sizeof(str), local->file);

    }
    finished = 1;
    local->res_size = line;


    for(a = 0 ; a < THREAD_POOL_SIZE ; a++) pthread_join(worker_threads[a], NULL);

    pthread_exit(&finished);
}

void *worker_thread_function(void *ptr){

    arg *local = ptr;
    char *query;
    uint32_t N1, N2;
    int line, i, ret;
    uint32_t local_version = 0;
    Queue *frontierF = NULL, *frontierB = NULL;
    int thread_id;

    pthread_mutex_lock(&vmutex);
    query = malloc(64*sizeof(char));

    thread_id = max_id;
    max_id++;
    pthread_mutex_unlock(&vmutex);


    frontierF = createQueue();  // synoro tou bfs apo thn arxh pros ton stoxo
    frontierB = createQueue();


    while(1) {

        if(finished) break;

        pthread_mutex_lock(&mutex);
        //pthread_cond_wait(&cond_start, &mutex);

/*        while(status == START){
            if(finished) break;
        }

        if(finished) break;

        pthread_mutex_lock(&mutex);

        if(status == START){
            pthread_mutex_unlock(&mutex);
            continue;
        }*/
        ret = remove_from_buffer(local->buffer, &line, &query);

        if(ret == -1){
            //status = START;
            pthread_cond_broadcast(&cond_next);
            pthread_mutex_unlock(&mutex);
            if(finished) break;
            continue;
        }
        pthread_mutex_unlock(&mutex);



        toID(query, &N1, &N2);


        if (lookup(global_index_out, N1, global_index_size_out) == ALR_EXISTS &&
            lookup(global_index_in, N2, global_index_size_in) == ALR_EXISTS &&
            isReachableGrailIndex(global_grail, N1, N2, global_scc) == MAYBE) {
            /*local->data->version++;
            local_version = local->data->version;*/
            local_version++;
            //pthread_mutex_unlock(&vmutex);
            local->results[line] = bBFS(global_index_in, global_index_out, global_buffer_in,
                                        global_buffer_out, N1, N2, frontierF, frontierB,
                                        local_version,thread_id);

            //printf("%d\n", local->data->steps);//den tupwnei ta vazei ston pinaka me ta apotelesmata
            //local->results[i] = local->data->steps;
        } else {
            local->results[line] = -1;
           // pthread_mutex_unlock(&vmutex);
        }//printf("-1\n");//den tupwnei ta vazei ston pinaka me ta apotelesmata

        //pthread_mutex_lock(&vmutex);
       // local->data->version++;
        //pthread_mutex_unlock(&vmutex);


    }
    free(query);

    empty(frontierB);
    empty(frontierF);

    i = 0;

    pthread_exit(&i);
}