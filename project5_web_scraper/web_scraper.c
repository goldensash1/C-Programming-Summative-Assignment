#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_URL_LEN 512
#define MAX_CMD_LEN 1024

typedef struct {
    int index;
    char url[MAX_URL_LEN];
} ThreadTask;

static void *download_page(void *arg) {
    ThreadTask *task = (ThreadTask *)arg;
    char outFile[64];
    char cmd[MAX_CMD_LEN];

    snprintf(outFile, sizeof(outFile), "output_thread_%d.html", task->index + 1);

    // curl is used for simplicity and portability in this assignment context.
    snprintf(cmd, sizeof(cmd), "curl -L --max-time 15 -sS \"%s\"", task->url);

    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        fprintf(stderr, "[Thread %d] Failed to start curl for URL: %s\n", task->index + 1, task->url);
        return NULL;
    }

    FILE *out = fopen(outFile, "w");
    if (!out) {
        fprintf(stderr, "[Thread %d] Failed to open output file: %s\n", task->index + 1, outFile);
        pclose(pipe);
        return NULL;
    }

    char buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        fwrite(buffer, 1, bytesRead, out);
    }

    fclose(out);

    int status = pclose(pipe);
    if (status == -1) {
        fprintf(stderr, "[Thread %d] Error while waiting for curl: %s\n", task->index + 1, task->url);
    } else if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        fprintf(stderr, "[Thread %d] URL unreachable or download failed: %s\n", task->index + 1, task->url);
    } else {
        printf("[Thread %d] Downloaded: %s -> %s\n", task->index + 1, task->url, outFile);
    }

    return NULL;
}

static int load_urls_from_file(const char *fileName, ThreadTask **tasksOut, int *countOut) {
    FILE *fp = fopen(fileName, "r");
    if (!fp) {
        return 0;
    }

    int capacity = 8;
    int count = 0;
    ThreadTask *tasks = (ThreadTask *)malloc((size_t)capacity * sizeof(ThreadTask));
    if (!tasks) {
        fclose(fp);
        return 0;
    }

    char line[MAX_URL_LEN];
    while (fgets(line, sizeof(line), fp)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[len - 1] = '\0';
        if (line[0] == '\0') continue;

        if (count == capacity) {
            capacity *= 2;
            ThreadTask *newTasks = (ThreadTask *)realloc(tasks, (size_t)capacity * sizeof(ThreadTask));
            if (!newTasks) {
                free(tasks);
                fclose(fp);
                return 0;
            }
            tasks = newTasks;
        }

        tasks[count].index = count;
        strncpy(tasks[count].url, line, MAX_URL_LEN - 1);
        tasks[count].url[MAX_URL_LEN - 1] = '\0';
        count++;
    }

    fclose(fp);

    if (count == 0) {
        free(tasks);
        return 0;
    }

    *tasksOut = tasks;
    *countOut = count;
    return 1;
}

int main(int argc, char *argv[]) {
    const char *urlFile = "urls.txt";
    ThreadTask *tasks = NULL;
    int urlCount = 0;

    if (argc > 1) {
        urlFile = argv[1];
    }

    if (!load_urls_from_file(urlFile, &tasks, &urlCount)) {
        fprintf(stderr, "Failed to load URLs from %s\n", urlFile);
        fprintf(stderr, "Create a text file with one URL per line.\n");
        return 1;
    }

    pthread_t *threads = (pthread_t *)malloc((size_t)urlCount * sizeof(pthread_t));
    if (!threads) {
        fprintf(stderr, "Failed to allocate thread handles.\n");
        free(tasks);
        return 1;
    }

    for (int i = 0; i < urlCount; i++) {
        int rc = pthread_create(&threads[i], NULL, download_page, &tasks[i]);
        if (rc != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i + 1);
        }
    }

    for (int i = 0; i < urlCount; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    free(tasks);

    printf("All downloads completed.\n");
    return 0;
}
