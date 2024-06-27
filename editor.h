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
        int scroll_offset;
        char *filename;
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
line *get_current_line_position(editor *editor);
void save_file(editor *editor);
void load_file(editor *editor, const char *name);
void free_lines(line *head);
void scroll_rows(editor *editor);
void free_editor(editor *editor);

#endif
