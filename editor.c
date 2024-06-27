#include "editor.h"
#include <ncurses.h>

#define MAX_CONTENT 1024

void init_editor(editor *editor)
{
        getmaxyx(stdscr, editor->height, editor->width);
        editor->cx = editor->cy = 0;
        editor->num_rows = 0;
        editor->current_row = 0;
        editor->should_close = 0;
        editor->scroll_offset = 0;
        editor->filename = NULL;
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
        line *current = get_current_line_position(editor);

        if (editor->cx > MAX_CONTENT) {
                current->c = realloc(current->c, MAX_CONTENT * 2);
        }

        memmove(&current->c[editor->cx + 1], &current->c[editor->cx], current->len - editor->cx);
        current->c[editor->cx] = c;
        current->len++;

        editor->cx++;

}

void insert_new_line(editor *editor) {
    if (editor->head == NULL) {
        editor->head = create_new_node();
    }

    line *current = get_current_line_position(editor);
    line *newLine = create_new_node();

    // Handle splitting the current line and inserting a new line
    if (editor->cx < current->len) {
        // Copy the content after the cursor to the new line
        strncpy(newLine->c, &current->c[editor->cx], MAX_CONTENT - 1);
        newLine->c[MAX_CONTENT - 1] = '\0'; // Ensure null-termination
        newLine->len = strlen(newLine->c);
        
        // Trim the current line content at the cursor position
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

void delete_char(editor *editor) {
    line *current = editor->head;
    line *prev = NULL;
    int row = 0;

    // Navigate to the current row
    while (current != NULL && row < editor->current_row) {
        prev = current;
        current = current->next;
        row++;
    }

    // Check if we are out of bounds or the cursor position is invalid
    if (current == NULL || editor->cx <= 0 || editor->cx > current->len) {
        return;
    }

    // Delete a single character
    memmove(&current->c[editor->cx - 1], &current->c[editor->cx], current->len - editor->cx);
    current->len--;

    if (current->len == 0 && row > 0) {
        // Delete the line if it's empty after deletion and not the first line
        if (prev == NULL) {
            editor->head = current->next;
        } else {
            prev->next = current->next;
        }

        free(current->c);
        free(current);
        editor->num_rows--;
        editor->cy--;
        editor->current_row--;
        editor->cx = (prev != NULL) ? prev->len : 0; // Move cursor to end of previous line
        move(editor->cy, editor->cx);
        return;
    }

    current->c[current->len] = '\0';
    editor->cx--;
    move(editor->cy, editor->cx);
}

line *get_current_line_position(editor *editor)
{
        line *current = editor->head;
        int row = 0;
        while (current != NULL && row < editor->current_row) {
                current = current->next;
                row++;
        }

        return current;
}

void update_cursor_position(editor *editor, int key)
{
        switch (key) {
                case KEY_UP:
                        {
                                if (editor->cy > 0) {
                                        editor->cy--;
                                        editor->current_row--;
                                } else {
                                        editor->cy = 0;
                                }

                        }
                        break;

                case KEY_DOWN:
                        {
                                if (editor->cy < editor->num_rows - 1) {
                                        editor->cy++;
                                        editor->current_row++;
                                        
                                        line *current = get_current_line_position(editor);
                                        if (current && editor->cx >= current->len) {
                                                editor->cx = current->len;
                                        }
                                }
                        }
                        break;

                case KEY_LEFT:
                        if (editor->cx > 0) {
                                editor->cx--;
                        }
                        break;

                case KEY_RIGHT:
                        {
                                line *current = get_current_line_position(editor);
                                if (editor->cx < current->len) {
                                        editor->cx++;
                                }

                        }
                        break;
        }
}

void scroll_rows(editor *editor) 
{
        if (editor->cy < editor->scroll_offset) {
                editor->scroll_offset = editor->cy;
        }

        if (editor->cy >= editor->scroll_offset + editor->height) {
                editor->scroll_offset = editor->cy - editor->height + 1;
        }

        if (editor->scroll_offset > editor->num_rows - editor->height) {
        editor->scroll_offset = editor->num_rows - editor->height;
    }

}

void print_rows(editor *editor)
{
        line *current = editor->head;
        int displayed_row = 0;

        // Avança para a linha de scroll_offset
        for (int i = 0; i < editor->scroll_offset && current != NULL; i++) {
                current = current->next;
        }

        // Imprime apenas as linhas visíveis na janela
        while (current != NULL && displayed_row < editor->height) {
                for (int i = 0; i < current->len && i < editor->width; i++) {
                        mvaddch(displayed_row, i, current->c[i]);
                }
                displayed_row++;
                current = current->next;
        }

        move(editor->cy - editor->scroll_offset, editor->cx);
        refresh();
}

void save_file(editor *editor) {
    if (editor->filename == NULL) {
        fprintf(stderr, "Filename is not specified.\n");
        return;
    }

    FILE *file = fopen(editor->filename, "w");
    if (!file) {
        perror("Could not open file for writing");
        return;
    }

    line *current_line = editor->head;
    while (current_line != NULL) {
        fwrite(current_line->c, sizeof(char), current_line->len, file);

        if (strstr(current_line->c, "\n") == NULL) {
                        fputc('\n', file);
                }

        current_line = current_line->next;
    }

    fclose(file);
    mvprintw(editor->height - 1, 0, "File saved successfully.\n");
}

void free_lines(line *head)
{
        line *current = head;
        while (current != NULL) {
                line *next = current->next;
                free(current->c);
                free(current);
                current = next;
        }
}

void load_file(editor *editor, const char *name)
{
        FILE *file = fopen(name, "r");
        if (file == NULL) {
                perror("Could not open file");
                return;
        }

        if (editor->head != NULL) {
                free_lines(editor->head);
                editor->head = NULL;
        }

        char *line_buf = NULL;
        size_t line_buf_size = 0;
        ssize_t line_size;
        line *current_line = NULL;
        while ((line_size = getline(&line_buf, &line_buf_size, file)) != -1) {
                line *new_line = create_new_node();
                new_line->len = line_size;
                memcpy(new_line->c, line_buf, line_size);
                new_line->c[line_size] = '\0';

                if (current_line == NULL) {
                        editor->head = new_line;
                } else {
                        current_line->next = new_line;
                }
                current_line = new_line;
                editor->num_rows++;
        }

        free(line_buf);
        fclose(file);
        editor->filename = strdup(name);
}

void free_editor(editor *editor)
{
        if (editor->head != NULL) {
                free_lines(editor->head);
                editor->head = NULL;
        }
        
        if (editor->filename != NULL) {
                free(editor->filename);
                editor->filename = NULL;
        }
}
