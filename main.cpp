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

bool process_key(char key) {
    if (key == 'r') {
        // start a reader
        auto reader = new Reader(reader_counter++, mx);
        reader->start();
        readers.push_back(reader);
        return true;
    } else if (key == 'w') {
        // start a writer
        auto writer = new Writer(writer_counter++, mx);
        writer->start();
        writers.push_back(writer);
        return true;
    } else if (key == 'x') {
        // exit reader
        if (!readers.empty()) {
            auto reader = readers.front();
            readers.pop_front();
            reader->finish();
            delete reader;
        }
        return true;
    } else if (key == 'q') {
        // exit writer
        if (!writers.empty()) {
            auto writer = writers.front();
            writers.pop_front();
            writer->finish();
            delete writer;
        }
        return true;
    } else {
        return false;        
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
    printw("\nInstructions: Press one of the following keys:\n");
    printw("r will start a reader\n");
    printw("w will start a writer\n");
    printw("x will exit a reader\n");
    printw("q will exit a writer\n");
    printw("any other key will exit the program\n");
    printw("\nThe only difference between a reader and writer is that a reader acquires a lock in shared mode (S) whereas a writer acquires a lock in exclusive mode (X)\n");
    printw("Your objective is to verify following assertions:\n");
    printw("Assertion 1: No writer can start writing while a reader is reading. To verify this first start a reader (r) and then try to start a writer (w).\n");
    printw("Assertion 2: No writer can start writing while another writer is writing. To verify this first start a writer (w) and then try to start another writer (w).\n");
    printw("Assertion 3: No reader can start reading while a writer is writing. To verify this first start a writer (w) and then try to start another reader (r).\n");
    printw("Assertion 4: Multiple readers can read concurrently at the same time provided no writer is writing. To verify this first start a reader (r) and then try to start another reader (r).\n");
    // Prompt the user to press any key    
    printw("\nLet's get started now. Please press any key:\n");
    refresh();
    bool cont = true;

    while (cont) {
        // Read a single character without requiring Enter
        key = getch();
        cont = process_key(key);
        refresh();
    }
    
    // exit
    // Clean up ncurses
    endwin();
    return 0;
}
