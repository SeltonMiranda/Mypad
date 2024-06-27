#ifndef __EDIT__
#define __EDIT__

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

typedef struct line line;
typedef struct line {
        int len;
        char *c;
        line *next;
} line;

typedef struct editor {
        int cx, cy;
        int height, width;
        int num_rows;
        int current_row;
        int should_close;
        line *head;
} editor;

void init_editor(editor *editor);
void init_ncurses();
void end_ncurses();
void insert_char(editor *editor, int c);
void insert_new_line(editor *editor);
void print_rows(editor *editor);
void update_cursor_position(editor *editor, int key);
void delete_char(editor *editor);


#endif
