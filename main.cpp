#include <iostream>
#include <ncurses.h>
#include <shared_mutex>
#include <condition_variable>
#include <deque>
#include "common.cpp"

std::shared_mutex mx;   // the mutex for reader-writer lock
int reader_counter = 0;
int writer_counter = 0;
std::deque<Reader*> readers;
std::deque<Writer*> writers;

void process_key(char key) {
    if (key == 'r') {
        // start a reader
        auto reader = new Reader(reader_counter++, mx);
        reader->start();
        readers.push_back(reader);
    } else if (key == 'w') {
        // start a writer
        auto writer = new Writer(writer_counter++, mx);
        writer->start();
        writers.push_back(writer);
    } else if (key == 'x') {
        // exit reader
        if (!readers.empty()) {
            auto reader = readers.front();
            readers.pop_front();
            reader->finish();
        }        
    } else if (key == 'q') {
        // exit writer
        if (!writers.empty()) {
            auto writer = writers.front();
            writers.pop_front();
            writer->finish();
        }
    } else {
        std::cout << "invalid key pressed";
    }
}

int main() {
    char key;
    
    // Initialize ncurses
    initscr();
    cbreak(); // Line buffering disabled, pass on everything to me
    noecho(); // Don't echo pressed characters to the screen
    keypad(stdscr, TRUE); // Enable special keys, e.g., arrow keys

    printw("Welcome to reader-writer concurrency demo.\n");
    // Prompt the user to press any key
    printw("Please press any key:\n");
    refresh();

    while (true) {
        // Read a single character without requiring Enter
        key = getch();
        process_key(key);
        refresh();
    }

    // Clean up ncurses
    getch(); // Wait for another key press before exiting
    endwin();

    return 0;
}
