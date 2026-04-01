# Project 5 - Multi-threaded Web Scraper

## Features implemented
- Uses POSIX threads (`pthreads`) to fetch multiple URLs in parallel
- Reads URL list from file (`urls.txt` by default)
- Saves each thread output into a separate file: `output_thread_X.html`
- Handles unreachable URLs and command failures with clear error messages

## Compile and run
```bash
gcc -Wall -Wextra -std=c11 web_scraper.c -o web_scraper -lpthread
./web_scraper
```

Use a custom URL file:
```bash
./web_scraper my_urls.txt
```
