
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
                strcpy(newLine->c, &current->c[editor->cx]);
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
        refresh();
}

void delete_char(editor *editor)
{
        line *current = editor->head;
        int row = 0;
        while (current != NULL && row < editor->current_row) {
                current = current->next;
                row++;
        }
        
        if (editor->cx <= 0 || editor->cx > current->len)
                return;

        memmove(&current->c[editor->cx - 1], &current->c[editor->cx],
                current->len - editor->cx);
        current->len--;
        current->c[current->len] = '\0';
        editor->cx--;
        clear();
        move(editor->cy, editor->cx);
        refresh();
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
                mvprintw(row++, 0, "%s", current->c);
                current = current->next;
        }
        move(editor->cy, editor->cx);
        refresh();
}
