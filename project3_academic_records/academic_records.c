#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 64
#define MAX_COURSE 64
#define MAX_GRADES 10
#define INITIAL_CAPACITY 4
#define DATA_FILE "records.txt"

typedef struct {
    int id;
    char name[MAX_NAME];
    char course[MAX_COURSE];
    int age;
    int gradeCount;
    float grades[MAX_GRADES];
    float gpa;
} Student;

typedef struct {
    Student *items;
    size_t count;
    size_t capacity;
} Database;

static void trim_newline(char *text) {
    size_t len = strlen(text);
    if (len > 0 && text[len - 1] == '\n') {
        text[len - 1] = '\0';
    }
}

static int read_line(char *buffer, size_t size) {
    if (fgets(buffer, (int)size, stdin) == NULL) {
        return 0;
    }
    trim_newline(buffer);
    return 1;
}

static int read_int(const char *prompt, int min, int max, int *out) {
    char line[128];
    char *end = NULL;
    long value;

    while (1) {
        printf("%s", prompt);
        if (!read_line(line, sizeof(line))) {
            return 0;
        }

        if (line[0] == '\0') {
            printf("Input cannot be empty.\n");
            continue;
        }

        value = strtol(line, &end, 10);
        if (*end != '\0') {
            printf("Please enter a valid integer.\n");
            continue;
        }

        if (value < min || value > max) {
            printf("Value must be between %d and %d.\n", min, max);
            continue;
        }

        *out = (int)value;
        return 1;
    }
}

static int read_float(const char *prompt, float min, float max, float *out) {
    char line[128];
    char *end = NULL;
    float value;

    while (1) {
        printf("%s", prompt);
        if (!read_line(line, sizeof(line))) {
            return 0;
        }

        if (line[0] == '\0') {
            printf("Input cannot be empty.\n");
            continue;
        }

        value = strtof(line, &end);
        if (*end != '\0') {
            printf("Please enter a valid number.\n");
            continue;
        }

        if (value < min || value > max) {
            printf("Value must be between %.2f and %.2f.\n", min, max);
            continue;
        }

        *out = value;
        return 1;
    }
}

static void compute_gpa(Student *s) {
    if (s->gradeCount <= 0) {
        s->gpa = 0.0f;
        return;
    }

    float sum = 0.0f;
    for (int i = 0; i < s->gradeCount; i++) {
        sum += s->grades[i];
    }

    float avg100 = sum / s->gradeCount;
    s->gpa = avg100 / 25.0f;
    if (s->gpa > 4.0f) s->gpa = 4.0f;
}

static int init_database(Database *db) {
    db->items = (Student *)malloc(INITIAL_CAPACITY * sizeof(Student));
    if (db->items == NULL) {
        return 0;
    }
    db->count = 0;
    db->capacity = INITIAL_CAPACITY;
    return 1;
}

static void free_database(Database *db) {
    free(db->items);
    db->items = NULL;
    db->count = 0;
    db->capacity = 0;
}

static int ensure_capacity(Database *db) {
    if (db->count < db->capacity) {
        return 1;
    }

    size_t newCap = db->capacity * 2;
    Student *newItems = (Student *)realloc(db->items, newCap * sizeof(Student));
    if (newItems == NULL) {
        return 0;
    }

    db->items = newItems;
    db->capacity = newCap;
    return 1;
}

static int find_index_by_id(const Database *db, int id) {
    for (size_t i = 0; i < db->count; i++) {
        if (db->items[i].id == id) {
            return (int)i;
        }
    }
    return -1;
}

static int contains_case_insensitive(const char *text, const char *pattern) {
    if (pattern[0] == '\0') return 1;

    size_t n = strlen(text);
    size_t m = strlen(pattern);
    if (m > n) return 0;

    for (size_t i = 0; i <= n - m; i++) {
        size_t j = 0;
        while (j < m && tolower((unsigned char)text[i + j]) == tolower((unsigned char)pattern[j])) {
            j++;
        }
        if (j == m) return 1;
    }
    return 0;
}

static void print_student(const Student *s) {
    printf("ID:%d | Name:%s | Course:%s | Age:%d | GPA:%.2f | Grades:",
           s->id, s->name, s->course, s->age, s->gpa);
    for (int i = 0; i < s->gradeCount; i++) {
        printf(" %.1f", s->grades[i]);
    }
    printf("\n");
}

static void add_student(Database *db) {
    Student s;

    if (!ensure_capacity(db)) {
        printf("Memory allocation failed while adding a student.\n");
        return;
    }

    if (!read_int("Student ID: ", 1, 1000000, &s.id)) return;
    if (find_index_by_id(db, s.id) >= 0) {
        printf("Duplicate student ID is not allowed.\n");
        return;
    }

    printf("Student Name: ");
    if (!read_line(s.name, sizeof(s.name))) return;
    if (s.name[0] == '\0') {
        printf("Name cannot be empty.\n");
        return;
    }

    printf("Course Name: ");
    if (!read_line(s.course, sizeof(s.course))) return;
    if (s.course[0] == '\0') {
        printf("Course name cannot be empty.\n");
        return;
    }

    if (!read_int("Age: ", 10, 100, &s.age)) return;
    if (!read_int("Number of grades (1-10): ", 1, MAX_GRADES, &s.gradeCount)) return;

    for (int i = 0; i < s.gradeCount; i++) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "Grade %d (0-100): ", i + 1);
        if (!read_float(prompt, 0.0f, 100.0f, &s.grades[i])) return;
    }

    compute_gpa(&s);
    db->items[db->count++] = s;
    printf("Student added successfully.\n");
}

static void display_all(const Database *db) {
    if (db->count == 0) {
        printf("No records found.\n");
        return;
    }

    for (size_t i = 0; i < db->count; i++) {
        print_student(&db->items[i]);
    }
}

static void update_student(Database *db) {
    int id;
    if (!read_int("Enter ID to update: ", 1, 1000000, &id)) return;

    int idx = find_index_by_id(db, id);
    if (idx < 0) {
        printf("Student not found.\n");
        return;
    }

    Student *s = &db->items[idx];
    printf("Updating student: %s\n", s->name);

    printf("New Name (current: %s): ", s->name);
    if (!read_line(s->name, sizeof(s->name))) return;
    if (s->name[0] == '\0') {
        printf("Name cannot be empty.\n");
        return;
    }

    printf("New Course (current: %s): ", s->course);
    if (!read_line(s->course, sizeof(s->course))) return;
    if (s->course[0] == '\0') {
        printf("Course cannot be empty.\n");
        return;
    }

    if (!read_int("New Age: ", 10, 100, &s->age)) return;
    if (!read_int("New number of grades (1-10): ", 1, MAX_GRADES, &s->gradeCount)) return;

    for (int i = 0; i < s->gradeCount; i++) {
        char prompt[64];
        snprintf(prompt, sizeof(prompt), "New grade %d (0-100): ", i + 1);
        if (!read_float(prompt, 0.0f, 100.0f, &s->grades[i])) return;
    }

    compute_gpa(s);
    printf("Student updated successfully.\n");
}

static void delete_student(Database *db) {
    int id;
    if (!read_int("Enter ID to delete: ", 1, 1000000, &id)) return;

    int idx = find_index_by_id(db, id);
    if (idx < 0) {
        printf("Student not found.\n");
        return;
    }

    for (size_t i = (size_t)idx; i + 1 < db->count; i++) {
        db->items[i] = db->items[i + 1];
    }

    db->count--;
    printf("Student deleted successfully.\n");
}

static void search_by_id(const Database *db) {
    int id;
    if (!read_int("Enter ID to search: ", 1, 1000000, &id)) return;

    int idx = find_index_by_id(db, id);
    if (idx < 0) {
        printf("Student not found.\n");
        return;
    }

    print_student(&db->items[idx]);
}

static void search_by_name(const Database *db) {
    char query[MAX_NAME];
    int found = 0;

    printf("Enter name to search: ");
    if (!read_line(query, sizeof(query))) return;

    for (size_t i = 0; i < db->count; i++) {
        if (contains_case_insensitive(db->items[i].name, query)) {
            print_student(&db->items[i]);
            found = 1;
        }
    }

    if (!found) {
        printf("No students matched '%s'.\n", query);
    }
}

static void swap_students(Student *a, Student *b) {
    Student temp = *a;
    *a = *b;
    *b = temp;
}

static void sort_by_gpa(Database *db) {
    if (db->count < 2) return;

    for (size_t i = 0; i < db->count - 1; i++) {
        for (size_t j = 0; j < db->count - i - 1; j++) {
            if (db->items[j].gpa < db->items[j + 1].gpa) {
                swap_students(&db->items[j], &db->items[j + 1]);
            }
        }
    }

    printf("Sorted by GPA (descending).\n");
}

static int name_compare(const char *a, const char *b) {
    while (*a && *b) {
        int ca = tolower((unsigned char)*a);
        int cb = tolower((unsigned char)*b);
        if (ca != cb) return ca - cb;
        a++;
        b++;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

static void sort_by_name(Database *db) {
    if (db->count < 2) return;

    for (size_t i = 0; i < db->count - 1; i++) {
        for (size_t j = 0; j < db->count - i - 1; j++) {
            if (name_compare(db->items[j].name, db->items[j + 1].name) > 0) {
                swap_students(&db->items[j], &db->items[j + 1]);
            }
        }
    }

    printf("Sorted by name (ascending).\n");
}

static void sort_by_id(Database *db) {
    if (db->count < 2) return;

    for (size_t i = 0; i < db->count - 1; i++) {
        for (size_t j = 0; j < db->count - i - 1; j++) {
            if (db->items[j].id > db->items[j + 1].id) {
                swap_students(&db->items[j], &db->items[j + 1]);
            }
        }
    }

    printf("Sorted by ID (ascending).\n");
}

static void report_summary(const Database *db) {
    if (db->count == 0) {
        printf("No records available for reporting.\n");
        return;
    }

    float sum = 0.0f;
    float min = db->items[0].gpa;
    float max = db->items[0].gpa;

    for (size_t i = 0; i < db->count; i++) {
        float g = db->items[i].gpa;
        sum += g;
        if (g < min) min = g;
        if (g > max) max = g;
    }

    float avg = sum / (float)db->count;

    float *temp = (float *)malloc(db->count * sizeof(float));
    if (temp == NULL) {
        printf("Memory error while calculating median.\n");
        return;
    }

    for (size_t i = 0; i < db->count; i++) {
        temp[i] = db->items[i].gpa;
    }

    for (size_t i = 0; i < db->count - 1; i++) {
        for (size_t j = 0; j < db->count - i - 1; j++) {
            if (temp[j] > temp[j + 1]) {
                float x = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = x;
            }
        }
    }

    float median;
    if (db->count % 2 == 0) {
        size_t mid = db->count / 2;
        median = (temp[mid - 1] + temp[mid]) / 2.0f;
    } else {
        median = temp[db->count / 2];
    }

    free(temp);

    printf("Class average GPA: %.2f\n", avg);
    printf("Highest GPA: %.2f\n", max);
    printf("Lowest GPA: %.2f\n", min);
    printf("Median GPA: %.2f\n", median);
}

static void report_top_n(const Database *db) {
    int n;
    if (db->count == 0) {
        printf("No records available.\n");
        return;
    }

    if (!read_int("Enter N for top students: ", 1, (int)db->count, &n)) return;

    Student *copy = (Student *)malloc(db->count * sizeof(Student));
    if (copy == NULL) {
        printf("Memory error.\n");
        return;
    }

    for (size_t i = 0; i < db->count; i++) copy[i] = db->items[i];

    for (size_t i = 0; i < db->count - 1; i++) {
        for (size_t j = 0; j < db->count - i - 1; j++) {
            if (copy[j].gpa < copy[j + 1].gpa) {
                swap_students(&copy[j], &copy[j + 1]);
            }
        }
    }

    for (int i = 0; i < n; i++) {
        print_student(&copy[i]);
    }

    free(copy);
}

static void report_best_per_course(const Database *db) {
    if (db->count == 0) {
        printf("No records available.\n");
        return;
    }

    int *printed = (int *)calloc(db->count, sizeof(int));
    if (printed == NULL) {
        printf("Memory error.\n");
        return;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (printed[i]) continue;

        size_t best = i;
        for (size_t j = i + 1; j < db->count; j++) {
            if (name_compare(db->items[j].course, db->items[i].course) == 0) {
                printed[j] = 1;
                if (db->items[j].gpa > db->items[best].gpa) {
                    best = j;
                }
            }
        }

        printed[i] = 1;
        printf("Best student in %s:\n", db->items[i].course);
        print_student(&db->items[best]);
    }

    free(printed);
}

static void report_course_averages(const Database *db) {
    if (db->count == 0) {
        printf("No records available.\n");
        return;
    }

    int *used = (int *)calloc(db->count, sizeof(int));
    if (used == NULL) {
        printf("Memory error.\n");
        return;
    }

    for (size_t i = 0; i < db->count; i++) {
        if (used[i]) continue;

        float sum = 0.0f;
        int count = 0;

        for (size_t j = i; j < db->count; j++) {
            if (name_compare(db->items[i].course, db->items[j].course) == 0) {
                used[j] = 1;
                sum += db->items[j].gpa;
                count++;
            }
        }

        printf("Course %s average GPA: %.2f\n", db->items[i].course, sum / count);
    }

    free(used);
}

static void save_to_file(const Database *db) {
    FILE *fp = fopen(DATA_FILE, "w");
    if (fp == NULL) {
        printf("Could not open file for writing: %s\n", DATA_FILE);
        return;
    }

    for (size_t i = 0; i < db->count; i++) {
        const Student *s = &db->items[i];
        fprintf(fp, "%d|%s|%s|%d|%d", s->id, s->name, s->course, s->age, s->gradeCount);
        for (int g = 0; g < s->gradeCount; g++) {
            fprintf(fp, "|%.2f", s->grades[g]);
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    printf("Saved %zu record(s) to %s\n", db->count, DATA_FILE);
}

static void load_from_file(Database *db) {
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL) {
        printf("Could not open file: %s\n", DATA_FILE);
        return;
    }

    db->count = 0;

    char line[512];
    while (fgets(line, sizeof(line), fp) != NULL) {
        Student s;
        char *token;
        int field = 0;

        trim_newline(line);

        token = strtok(line, "|");
        while (token != NULL) {
            if (field == 0) s.id = atoi(token);
            else if (field == 1) strncpy(s.name, token, sizeof(s.name));
            else if (field == 2) strncpy(s.course, token, sizeof(s.course));
            else if (field == 3) s.age = atoi(token);
            else if (field == 4) s.gradeCount = atoi(token);
            else {
                int gradeIndex = field - 5;
                if (gradeIndex >= 0 && gradeIndex < MAX_GRADES) {
                    s.grades[gradeIndex] = strtof(token, NULL);
                }
            }

            field++;
            token = strtok(NULL, "|");
        }

        if (s.gradeCount < 1 || s.gradeCount > MAX_GRADES) {
            continue;
        }

        s.name[MAX_NAME - 1] = '\0';
        s.course[MAX_COURSE - 1] = '\0';
        compute_gpa(&s);

        if (!ensure_capacity(db)) {
            printf("Memory error while loading file.\n");
            fclose(fp);
            return;
        }

        if (find_index_by_id(db, s.id) >= 0) {
            continue;
        }

        db->items[db->count++] = s;
    }

    fclose(fp);
    printf("Loaded %zu record(s) from %s\n", db->count, DATA_FILE);
}

static void print_menu(void) {
    printf("\n===== Academic Records Analyzer =====\n");
    printf("1. Add student\n");
    printf("2. Display all students\n");
    printf("3. Update student\n");
    printf("4. Delete student\n");
    printf("5. Search by ID\n");
    printf("6. Search by name\n");
    printf("7. Sort by GPA\n");
    printf("8. Sort by name\n");
    printf("9. Sort by ID\n");
    printf("10. Report summary\n");
    printf("11. Report top N students\n");
    printf("12. Report best student per course\n");
    printf("13. Report course averages\n");
    printf("14. Save to file\n");
    printf("15. Load from file\n");
    printf("16. Exit\n");
    printf("Choose option: ");
}

int main(void) {
    Database db;
    if (!init_database(&db)) {
        printf("Failed to initialize database.\n");
        return 1;
    }

    while (1) {
        char line[64];
        int choice;

        print_menu();
        if (!read_line(line, sizeof(line))) {
            break;
        }

        choice = atoi(line);
        switch (choice) {
            case 1: add_student(&db); break;
            case 2: display_all(&db); break;
            case 3: update_student(&db); break;
            case 4: delete_student(&db); break;
            case 5: search_by_id(&db); break;
            case 6: search_by_name(&db); break;
            case 7: sort_by_gpa(&db); break;
            case 8: sort_by_name(&db); break;
            case 9: sort_by_id(&db); break;
            case 10: report_summary(&db); break;
            case 11: report_top_n(&db); break;
            case 12: report_best_per_course(&db); break;
            case 13: report_course_averages(&db); break;
            case 14: save_to_file(&db); break;
            case 15: load_from_file(&db); break;
            case 16:
                save_to_file(&db);
                free_database(&db);
                printf("Goodbye.\n");
                return 0;
            default:
                printf("Invalid option. Choose 1-16.\n");
                break;
        }
    }

    free_database(&db);
    return 0;
}
