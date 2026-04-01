#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "dataset.txt"

typedef struct {
    double *data;
    size_t size;
    size_t capacity;
} Dataset;

typedef int (*MenuAction)(Dataset *);
typedef int (*FilterCallback)(double, double);
typedef double (*TransformCallback)(double, double);
typedef int (*CompareCallback)(double, double);

typedef struct {
    int option;
    const char *label;
    MenuAction action;
} MenuItem;

static int init_dataset(Dataset *ds) {
    ds->capacity = 8;
    ds->size = 0;
    ds->data = (double *)malloc(ds->capacity * sizeof(double));
    return ds->data != NULL;
}

static void free_dataset(Dataset *ds) {
    free(ds->data);
    ds->data = NULL;
    ds->size = 0;
    ds->capacity = 0;
}

static int ensure_capacity(Dataset *ds, size_t needed) {
    if (needed <= ds->capacity) return 1;

    size_t newCap = ds->capacity;
    while (newCap < needed) newCap *= 2;

    double *newData = (double *)realloc(ds->data, newCap * sizeof(double));
    if (newData == NULL) return 0;

    ds->data = newData;
    ds->capacity = newCap;
    return 1;
}

static int read_int(const char *prompt, int min, int max, int *out) {
    char line[128];
    char *end;
    long value;

    while (1) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) return 0;

        value = strtol(line, &end, 10);
        if (*end != '\n' && *end != '\0') {
            printf("Please enter a valid integer.\n");
            continue;
        }

        if (value < min || value > max) {
            printf("Value must be in range [%d, %d].\n", min, max);
            continue;
        }

        *out = (int)value;
        return 1;
    }
}

static int read_double(const char *prompt, double *out) {
    char line[128];
    char *end;
    double value;

    while (1) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) return 0;

        value = strtod(line, &end);
        if (*end != '\n' && *end != '\0') {
            printf("Please enter a valid number.\n");
            continue;
        }

        *out = value;
        return 1;
    }
}

static void print_dataset(const Dataset *ds) {
    if (ds->size == 0) {
        printf("Dataset is empty.\n");
        return;
    }

    printf("Dataset (%zu items): ", ds->size);
    for (size_t i = 0; i < ds->size; i++) {
        printf("%.2f", ds->data[i]);
        if (i + 1 < ds->size) printf(", ");
    }
    printf("\n");
}

static int op_create_dataset(Dataset *ds) {
    int n;
    if (!read_int("How many values? ", 1, 100000, &n)) return 0;

    if (!ensure_capacity(ds, (size_t)n)) {
        printf("Memory allocation failed.\n");
        return 1;
    }

    ds->size = (size_t)n;
    for (int i = 0; i < n; i++) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "Value %d: ", i + 1);
        if (!read_double(prompt, &ds->data[i])) return 0;
    }

    printf("Dataset created.\n");
    return 1;
}

static int op_display_dataset(Dataset *ds) {
    (void)ds;
    return 1;
}

static int op_sum_avg(Dataset *ds) {
    if (ds->size == 0) {
        printf("Dataset is empty.\n");
        return 1;
    }

    double sum = 0.0;
    for (size_t i = 0; i < ds->size; i++) sum += ds->data[i];

    printf("Sum: %.2f\n", sum);
    printf("Average: %.2f\n", sum / ds->size);
    return 1;
}

static int op_min_max(Dataset *ds) {
    if (ds->size == 0) {
        printf("Dataset is empty.\n");
        return 1;
    }

    double min = ds->data[0];
    double max = ds->data[0];

    for (size_t i = 1; i < ds->size; i++) {
        if (ds->data[i] < min) min = ds->data[i];
        if (ds->data[i] > max) max = ds->data[i];
    }

    printf("Min: %.2f\n", min);
    printf("Max: %.2f\n", max);
    return 1;
}

static int filter_above_threshold(double value, double threshold) {
    return value > threshold;
}

static double transform_scale(double value, double factor) {
    return value * factor;
}

static int cmp_asc(double a, double b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static int cmp_desc(double a, double b) {
    return -cmp_asc(a, b);
}

static int apply_filter(Dataset *ds, FilterCallback cb, double param) {
    if (cb == NULL) {
        printf("Filter callback is NULL.\n");
        return 1;
    }

    size_t writeIndex = 0;
    for (size_t i = 0; i < ds->size; i++) {
        if (cb(ds->data[i], param)) {
            ds->data[writeIndex++] = ds->data[i];
        }
    }

    ds->size = writeIndex;
    printf("Filter applied. New size: %zu\n", ds->size);
    return 1;
}

static int apply_transform(Dataset *ds, TransformCallback cb, double param) {
    if (cb == NULL) {
        printf("Transform callback is NULL.\n");
        return 1;
    }

    for (size_t i = 0; i < ds->size; i++) {
        ds->data[i] = cb(ds->data[i], param);
    }

    printf("Transformation applied.\n");
    return 1;
}

static int sort_with_callback(Dataset *ds, CompareCallback cmp) {
    if (cmp == NULL) {
        printf("Compare callback is NULL.\n");
        return 1;
    }

    for (size_t i = 0; i + 1 < ds->size; i++) {
        for (size_t j = 0; j + 1 < ds->size - i; j++) {
            if (cmp(ds->data[j], ds->data[j + 1]) > 0) {
                double tmp = ds->data[j];
                ds->data[j] = ds->data[j + 1];
                ds->data[j + 1] = tmp;
            }
        }
    }

    printf("Sorting completed.\n");
    return 1;
}

static int op_filter_dataset(Dataset *ds) {
    if (ds->size == 0) {
        printf("Dataset is empty.\n");
        return 1;
    }

    double threshold;
    if (!read_double("Keep values above threshold: ", &threshold)) return 0;
    return apply_filter(ds, filter_above_threshold, threshold);
}

static int op_transform_dataset(Dataset *ds) {
    if (ds->size == 0) {
        printf("Dataset is empty.\n");
        return 1;
    }

    double factor;
    if (!read_double("Scale factor: ", &factor)) return 0;
    return apply_transform(ds, transform_scale, factor);
}

static int op_sort_dataset(Dataset *ds) {
    if (ds->size == 0) {
        printf("Dataset is empty.\n");
        return 1;
    }

    int order;
    if (!read_int("Sort order (1=ASC, 2=DESC): ", 1, 2, &order)) return 0;
    return sort_with_callback(ds, order == 1 ? cmp_asc : cmp_desc);
}

static int op_search_value(Dataset *ds) {
    if (ds->size == 0) {
        printf("Dataset is empty.\n");
        return 1;
    }

    double target;
    if (!read_double("Value to search: ", &target)) return 0;

    for (size_t i = 0; i < ds->size; i++) {
        if (ds->data[i] == target) {
            printf("Found %.2f at index %zu\n", target, i);
            return 1;
        }
    }

    printf("Value %.2f not found.\n", target);
    return 1;
}

static int op_reset_dataset(Dataset *ds) {
    ds->size = 0;
    printf("Dataset reset.\n");
    return 1;
}

static int op_load_file(Dataset *ds) {
    FILE *fp = fopen(DATA_FILE, "r");
    if (!fp) {
        printf("Could not open %s\n", DATA_FILE);
        return 1;
    }

    ds->size = 0;
    double value;
    while (fscanf(fp, "%lf", &value) == 1) {
        if (!ensure_capacity(ds, ds->size + 1)) {
            fclose(fp);
            printf("Memory allocation failed while loading file.\n");
            return 1;
        }
        ds->data[ds->size++] = value;
    }

    fclose(fp);
    printf("Loaded %zu values from %s\n", ds->size, DATA_FILE);
    return 1;
}

static int op_save_file(Dataset *ds) {
    FILE *fp = fopen(DATA_FILE, "w");
    if (!fp) {
        printf("Could not write to %s\n", DATA_FILE);
        return 1;
    }

    for (size_t i = 0; i < ds->size; i++) {
        fprintf(fp, "%.6f\n", ds->data[i]);
    }

    fclose(fp);
    printf("Saved %zu values to %s\n", ds->size, DATA_FILE);
    return 1;
}

static int op_exit_program(Dataset *ds) {
    (void)ds;
    return 0;
}

static MenuItem menu[] = {
    {1, "Create dataset", op_create_dataset},
    {2, "Display dataset", op_display_dataset},
    {3, "Compute sum and average", op_sum_avg},
    {4, "Find minimum and maximum", op_min_max},
    {5, "Filter dataset (callback)", op_filter_dataset},
    {6, "Transform dataset (callback)", op_transform_dataset},
    {7, "Sort dataset (comparison callback)", op_sort_dataset},
    {8, "Search value", op_search_value},
    {9, "Load dataset from file", op_load_file},
    {10, "Save dataset to file", op_save_file},
    {11, "Reset dataset", op_reset_dataset},
    {12, "Exit", op_exit_program}
};

int main(void) {
    Dataset ds;
    if (!init_dataset(&ds)) {
        printf("Failed to initialize dataset memory.\n");
        return 1;
    }

    while (1) {
        int choice;
        int shouldContinue = 1;
        size_t menuCount = sizeof(menu) / sizeof(menu[0]);

        printf("\n===== Data Analysis Toolkit =====\n");
        for (size_t i = 0; i < menuCount; i++) {
            printf("%d. %s\n", menu[i].option, menu[i].label);
        }

        if (!read_int("Choose option: ", 1, (int)menuCount, &choice)) {
            break;
        }

        MenuAction action = NULL;
        for (size_t i = 0; i < menuCount; i++) {
            if (menu[i].option == choice) {
                action = menu[i].action;
                break;
            }
        }

        if (action == NULL) {
            printf("Invalid option.\n");
            continue;
        }

        shouldContinue = action(&ds);
        if (choice == 2) {
            print_dataset(&ds);
        }

        if (!shouldContinue) {
            break;
        }
    }

    free_dataset(&ds);
    return 0;
}
