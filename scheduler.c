#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

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

struct job *jobCopy(struct job *pJob);

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
    int cumTime = 0;
    struct job *curJob = head;

    int id = 1;
    int response = 0, turnaround = 0, wait = 0;

    while (curJob != NULL) {
        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", curJob->id, cumTime - curJob->arrival,
               (cumTime + curJob->duration) - curJob->arrival, cumTime - curJob->arrival);

        id = curJob->id + 1;
        response += cumTime - curJob->arrival;
        turnaround += (cumTime + curJob->duration) - curJob->arrival;
        wait += cumTime - curJob->arrival;

        cumTime += curJob->duration;
        curJob = curJob->next;
    }
    printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", response / (float) id, turnaround / (float) id,
           wait / (float) id);
}

int compare_Jobs(const void *left, const void *right) {
    //https://stackoverflow.com/questions/35914574/sorting-linked-list-simplest-way
    struct job *leftj = *(struct job **) left;
    struct job *rightj = *(struct job **) right;
    int leftWeight = leftj->arrival + leftj->duration;
    int rightWeight = rightj->arrival + rightj->duration;
    return leftWeight - rightWeight;
}

void policy_SJF(struct job *head) {
    puts("Execution trace with SJF:");
    struct job *curJob;

    int jobArraySize = 0;
    for (curJob = head; curJob != NULL; curJob = curJob->next) {
        ++jobArraySize;
    }
    struct job *jobArray[jobArraySize];
    for (curJob = head; curJob != NULL; curJob = curJob->next) {
        jobArray[curJob->id] = curJob;
    }

    qsort(jobArray, jobArraySize, sizeof(struct job *), compare_Jobs);

    int cumTime = 0;
    for (int i = 0; i < jobArraySize; i++) {
        curJob = jobArray[i];

        if (curJob->arrival > cumTime) {
            cumTime += curJob->arrival - cumTime;
        }
        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", cumTime, curJob->id, curJob->arrival,
               curJob->duration);
        cumTime += curJob->duration;
    }
    puts("End of execution with SJF.");
}

void analyze_SJF(struct job *pJob) {

    struct job *curJob;

    int jobArraySize = 0;
    for (curJob = head; curJob != NULL; curJob = curJob->next) {
        ++jobArraySize;
    }
    struct job *jobArray[jobArraySize];
    for (curJob = head; curJob != NULL; curJob = curJob->next) {
        jobArray[curJob->id] = curJob;
    }

    qsort(jobArray, jobArraySize, sizeof(struct job *), compare_Jobs);

    int cumTime = 0;

    int id = 1;
    int response[jobArraySize];
    int turnaround[jobArraySize];
    int wait[jobArraySize];

    for (int i = 0; i < jobArraySize; i++) {
        curJob = jobArray[i];
        if (curJob->arrival > cumTime) {
            cumTime += curJob->arrival - cumTime;
        }
        if (curJob->id + 1 > id) {
            id = curJob->id + 1;
        }
        response[curJob->id] = cumTime - curJob->arrival;
        turnaround[curJob->id] = (cumTime + curJob->duration) - curJob->arrival;
        wait[curJob->id] = cumTime - curJob->arrival;

        cumTime += curJob->duration;
    }

    //print stats in order
    int responseS = 0, turnaroundS = 0, waitS = 0;
    for (int i = 0; i < jobArraySize; i++) {
        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", i, response[i], turnaround[i], wait[i]);
        responseS += response[i];
        turnaroundS += turnaround[i];
        waitS += wait[i];
    }
    printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", responseS / (float) id, turnaroundS / (float) id,
           waitS / (float) id);
}

void policy_RR(struct job *pJob, int quanta) {
    puts("Execution trace with RR:");
    struct job *curJob = pJob;

    struct job *queue[100] = {NULL};
    struct job *jobs[100] = {NULL};
    int cumTime = 0;
    int runQueueIndex = 0;
    int insertQueueIndex = 0;
    int nextJobIndex = 0;

    while (curJob != NULL) {
        jobs[curJob->id] = curJob;
        curJob = curJob->next;
    }

    while (jobs[nextJobIndex] != NULL || queue[runQueueIndex] != NULL) {
        if (jobs[nextJobIndex] != NULL &&
            jobs[nextJobIndex]->arrival <= cumTime) {
            struct job *currentJob = jobs[nextJobIndex++];
            int runTime;
            if (currentJob->duration >= quanta) {
                runTime = quanta;
            } else {
                runTime = currentJob->duration;
            }

            printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", cumTime, currentJob->id, currentJob->arrival,
                   runTime);

            cumTime += runTime;
            currentJob->duration = currentJob->duration - runTime;

            if (currentJob->duration > 0) {
                queue[insertQueueIndex++] = currentJob;
            }
        } else if (queue[runQueueIndex] != NULL) {
            struct job *currentJob = queue[runQueueIndex++];
            int runTime;
            if (currentJob->duration >= quanta) {
                runTime = quanta;
            } else {
                runTime = currentJob->duration;
            }

            printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", cumTime, currentJob->id, currentJob->arrival,
                   runTime);

            currentJob->duration = currentJob->duration - runTime;

            if (currentJob->duration > 0) {
                queue[insertQueueIndex++] = currentJob;
            }
            cumTime += runTime;
        } else {
            cumTime += jobs[nextJobIndex]->arrival - cumTime;
        }
    }

    puts("End of execution with RR.");
}

void analyze_RR(struct job *pJob, int quanta) {
    struct job *curJob = pJob;

    int cumTime = 0;
    int runQueueIndex = 0;
    int insertQueueIndex = 0;
    int nextJobIndex = 0;
    struct job *queue[100] = {NULL};
    struct job *jobs[100] = {NULL};
    int numJobs = 0;

    while (curJob != NULL) {
        jobs[curJob->id] = curJob;
        numJobs = curJob->id + 1;
        curJob = curJob->next;
    }
    int wait[numJobs];
    int runTimeArr[numJobs]; //needed because we mutate this in loop
    memset(runTimeArr, -1, sizeof(int) * numJobs);
    int startTime[numJobs];
    memset(startTime, -1, sizeof(int) * numJobs);
    int turnaround[numJobs];
    int responseTime[numJobs];
    memset(responseTime, -1, sizeof(int) * numJobs);

    int waitS = 0;
    int responseS = 0;
    int turnaroundS = 0;

    while (jobs[nextJobIndex] != NULL || queue[runQueueIndex] != NULL) {
        if (jobs[nextJobIndex] != NULL &&
            jobs[nextJobIndex]->arrival <= cumTime) {
            struct job *currentJob = jobs[nextJobIndex];
            int runTime;
            if (runTimeArr[currentJob->id] == -1){
                runTimeArr[currentJob->id] = currentJob->duration;
            }
            if (currentJob->duration >= quanta) {
                runTime = quanta;
            } else {
                runTime = currentJob->duration;
            }

            if (responseTime[currentJob->id] == -1){
                responseTime[currentJob->id] = cumTime-currentJob->arrival;
            }
            if (startTime[currentJob->id] == -1){
                startTime[currentJob->id] = cumTime;
            }

            cumTime += runTime;

            turnaround[currentJob->id] = cumTime-currentJob->arrival;
            wait[currentJob->id] = (cumTime - currentJob->arrival) - runTimeArr[currentJob->id];

            currentJob->duration = currentJob->duration - runTime;
            if (currentJob->duration > 0) {
                queue[insertQueueIndex++] = currentJob;
            }
            nextJobIndex++;
        } else if (queue[runQueueIndex] != NULL) {
            struct job *currentJob = queue[runQueueIndex];
            int runTime;
            if (currentJob->duration >= quanta) {
                runTime = quanta;
            } else {
                runTime = currentJob->duration;
            }

            currentJob->duration = currentJob->duration - runTime;

            if (currentJob->duration > 0) {
                queue[insertQueueIndex++] = currentJob;
            }
            cumTime += runTime;
            turnaround[currentJob->id] = cumTime - currentJob->arrival;
            wait[currentJob->id] = (cumTime - currentJob->arrival) - runTimeArr[currentJob->id];

            runQueueIndex++;
        } else {
            cumTime += jobs[nextJobIndex]->arrival - cumTime;
        }
    }
    for (int i = 0; i < numJobs; i++) {
        //add into sum array
        printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n", i, responseTime[i],
               turnaround[i], wait[i]);
        turnaroundS += turnaround[i];
        waitS += wait[i];
        responseS += responseTime[i];
    }

    printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n", responseS / (float) numJobs, turnaroundS / (float) numJobs,
           waitS / (float) numJobs);
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
    int slice_duration = atoi(argv[4]);

    // Note: we use a global variable to point to
    // the start of a linked-list of jobs, i.e., the job list
    read_workload_file(workload);

    if (strcmp(policy, "FIFO") == 0) {
        policy_FIFO(head);//mutates jobs
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
    if (strcmp(policy, "RR") == 0) {
        policy_RR(head, slice_duration);
        head = NULL;
        read_workload_file(workload);
        if (analysis) {
            printf("Begin analyzing RR:\n");
            analyze_RR(head, slice_duration);
            printf("End analyzing RR.\n");
        }
        exit(EXIT_SUCCESS);
    }
    exit(EXIT_SUCCESS);
}
