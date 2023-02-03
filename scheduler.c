#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

// TODO: Add more fields to this struct
struct job {
    int id;
    int arrival;
    int duration;
    struct job *next;
};

/*** Globals ***/
int seed = 100;

//This is the start of our linked list of jobs, i.e., the job list
struct job *head = NULL;

/*** Globals End ***/

/*Function to append a new job to the list*/
void append(int id, int arrival, int duration) {
    // create a new struct and initialize it with the input data
    struct job *tmp = (struct job *) malloc(sizeof(struct job));

    //tmp->id = numofjobs++;
    tmp->id = id;
    tmp->duration = duration;
    tmp->arrival = arrival;

    // the new job is the last job
    tmp->next = NULL;

    // Case: job is first to be added, linked list is empty
    if (head == NULL) {
        head = tmp;
        return;
    }

    struct job *prev = head;

    //Find end of list
    while (prev->next != NULL) {
        prev = prev->next;
    }

    //Add job to end of list
    prev->next = tmp;
    return;
}


/*Function to read in the workload file and create job list*/
void read_workload_file(char *filename) {
    int id = 0;
    FILE *fp;
    size_t len = 0;
    ssize_t read;
    char *line = NULL,
            *arrival = NULL,
            *length = NULL;

//    struct job **head_ptr = malloc(sizeof(struct job *));

    if ((fp = fopen(filename, "r")) == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) > 1) {
        arrival = strtok(line, ",\n");
        length = strtok(NULL, ",\n");

        // Make sure neither arrival nor duration are null.
        assert(arrival != NULL && length != NULL);

        append(id++, atoi(arrival), atoi(length));
    }

    fclose(fp);

    // Make sure we read in at least one job
    assert(id > 0);

    return;
}


void policy_FIFO(struct job *head) {
    puts("Execution trace with FIFO:");
    int cumTime = 0;
    struct job *curJob = head;
    while (curJob != NULL) {
        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", cumTime, curJob->id, curJob->arrival,
               curJob->duration);
        cumTime += curJob->duration;
        curJob = curJob->next;
    }
    puts("End of execution with FIFO.");

    return;
}

void analyze_FIFO(struct job *head) {
    // TODO: Fill this in

    return;
}

int compare_Jobs(const void *left, const void *right) {
    if (((struct job *) left)->duration + ((struct job *) left)->arrival <
        ((struct job *) right)->duration + ((struct job *) right)->arrival)
        return -1;
    else
        return 1;
}

void policy_SJF(struct job *head) {
    puts("Execution trace with SJF:");
//    struct job *curJob = head;
    struct job *curJob;
    //sort the job list prior to while looping:
    // to sort, add arrival and duration.

    int jobArraySize = 0;
    for (curJob = head; curJob != NULL; curJob = curJob->next) {
        ++jobArraySize;
    }
    struct job *jobArray[jobArraySize];
    for (curJob = head; curJob != NULL; curJob = curJob->next) {
        //ids are in order and 0 indexed
        jobArray[curJob->id] = curJob;
    }
//
    qsort(jobArray,jobArraySize,sizeof(struct job*),compare_Jobs);

//    while (curJob != NULL) {
//        printf("[Job %d] arrived at [%d], ran for: [%d]\n", curJob->id, curJob->arrival,
//               curJob->duration);
//        curJob = curJob->next;
//    }
    for(int i = 0; i < jobArraySize; i++){
        printf("[Job %d] arrived at [%d], ran for: [%d]\n", jobArray[i]->id, jobArray[i]->arrival,
               jobArray[i]->duration);
    }
    puts("End of execution with SJF.");
}

void analyze_SJF(struct job *pJob) {

}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "missing variables\n");
        fprintf(stderr, "usage: %s analysis-flag policy workload-file slice-duration\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int analysis = atoi(argv[1]);
    char *policy = argv[2],
            *workload = argv[3];
//    int slice_duration = atoi(argv[4]);

    // Note: we use a global variable to point to
    // the start of a linked-list of jobs, i.e., the job list
    read_workload_file(workload);

    if (strcmp(policy, "FIFO") == 0) {
        policy_FIFO(head);
        if (analysis) {
            printf("Begin analyzing FIFO:\n");
            analyze_FIFO(head);
            printf("End analyzing FIFO.\n");
        }
        exit(EXIT_SUCCESS);
    }
    if (strcmp(policy, "SJF") == 0) {
        policy_SJF(head);
        if (analysis) {
            printf("Begin analyzing SJF:\n");
            analyze_SJF(head);
            printf("End analyzing SJF.\n");
        }
        exit(EXIT_SUCCESS);
    }
    // TODO: Add other policies

    exit(EXIT_SUCCESS);
}

