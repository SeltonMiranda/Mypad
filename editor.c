
#include "editor.h"

#define MAX_CONTENT 1024

void init_editor(editor *editor)
{
        getmaxyx(stdscr, editor->height, editor->width);
        editor->cx = editor->cy = 0;
        editor->num_rows = 0;
        editor->current_row = 0;
        editor->should_close = 0;
        editor->head = NULL;
}

void init_ncurses()
{
        initscr();
        raw();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
}

void end_ncurses() 
{
        endwin();
}

static line *create_new_node()
{
        line *new = malloc(sizeof(line));
        if (!new) {
                fprintf(stderr, "couldnt alloc new row\n");
                exit(1);
        }
        new->c = malloc(sizeof(char) * MAX_CONTENT);
        if (!new->c) {
                fprintf(stderr, "couldnt alloc new buffer\n");
                exit(1);
        }
        new->len = 0;
        new->next = NULL;

        return new;
}

void insert_char(editor *editor, int c)
{
        if (editor->head == NULL) {
                editor->head = create_new_node();
        }

        line *current = editor->head;
        int row = 0;
        while (current != NULL && row < editor->current_row) {
                current = current->next;
                row++;
        }

        if (editor->cx > MAX_CONTENT) {
                current->c = realloc(current->c, MAX_CONTENT * 2);
        }

        memmove(&current->c[editor->cx + 1], &current->c[editor->cx], current->len - editor->cx);
        current->c[editor->cx] = c;
        current->len++;

        editor->cx++;
}

void insert_new_line(editor *editor)
{
        if (editor->head == NULL) {
                editor->head = create_new_node();
        }

        line *current = editor->head;
        int row = 0;
        while (current != NULL && row < editor->current_row) {
                current = current->next;
                row++;
        }

        line *newLine = create_new_node();

        if (editor->cx < current->len) {
                strncpy(newLine->c, &current->c[editor->cx], sizeof(newLine->c) - 1);
                newLine->c[strlen(newLine->c)] = '\0'; 
                newLine->len = strlen(newLine->c);
                current->c[editor->cx] = '\0';
                current->len = strlen(current->c);
        }

        newLine->next = current->next;
        current->next = newLine;

        editor->current_row++;
        editor->num_rows++;
        editor->cx = 0;
        editor->cy++;

        move(editor->cy, editor->cx);
}

void delete_char(editor *editor)
{
        line *current = editor->head;
        line *prev = NULL;
        int row = 0;
        while (current != NULL && row < editor->current_row) {
                prev = current;
                current = current->next;
                row++;
        }
        
        if (current == NULL || editor->cx <= 0 || editor->cx > current->len)
                return;

        memmove(&current->c[editor->cx - 1], &current->c[editor->cx],
                current->len - editor->cx);
        current->len--;
        
        if (current->len == 0 && row > 0) {
                free(current->c);
                free(current);
                editor->num_rows--;
                editor->cy--;
                editor->current_row--;
                editor->cx = prev->len;
                prev->next = NULL;

                move(editor->cy, editor->cx);
                return;
        }

        current->c[current->len] = '\0';
        editor->cx--;
        move(editor->cy, editor->cx);
}


void update_cursor_position(editor *editor, int key)
{
        switch (key) {
                case KEY_UP:
                        if (editor->cy > 0) {
                                editor->cy--;
                                editor->current_row--;
                        }
                        break;

                case KEY_DOWN:
                        if (editor->cy < editor->num_rows) {
                                editor->cy++;
                                editor->current_row++;
                        }
                        break;

                case KEY_LEFT:
                        if (editor->cx > 0) {
                                editor->cx--;
                        }
                        break;

                case KEY_RIGHT:
                        {
                                line *current = editor->head;
                                int row = 0;
                                while (current != NULL && row < editor->current_row) {
                                        current = current->next;
                                        row++;
                                }

                                if (editor->cx < current->len) {
                                        editor->cx++;
                                }
                        }
                        break;
        }
}

void print_rows(editor *editor)
{
        line *current = editor->head;
        int row = 0;
        while (current != NULL) {
                for (int i = 0; i < current->len; i++) {
                        mvaddch(row, i, current->c[i]);
                }
                row++;
                current = current->next;
        }
        move(editor->cy, editor->cx);
        refresh();
}
