# Project 3 - Course Performance and Academic Records Analyzer

## Features implemented
- Dynamic storage with `malloc`, `realloc`, `free`
- Student structure with ID, name, course, age, grades, GPA
- CRUD operations (add, display, update, delete)
- Manual search (by ID, by name)
- Manual sorting (by GPA, name, ID)
- Analytics: average/highest/lowest/median GPA, top N, best per course, course averages
- File persistence with text file (`records.txt`)
- Validation for ID uniqueness and grade ranges

## Compile and run
```bash
gcc -Wall -Wextra -std=c11 academic_records.c -o academic_records
./academic_records
```
