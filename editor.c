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



void insert_new_line(editor *editor)

{
        if (editor->head == NULL) {
                editor->head = create_new_node();
        }

        line *current = get_current_line_position(editor);
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



void delete_char(editor *editor) {
        line *current = editor->head;
        line *prev = NULL;
        int row = 0;

        // Navegar até a linha atual
        while (current != NULL && row < editor->current_row) {
                prev = current;
                current = current->next;
                row++;
        }

        // Verifica se estamos fora dos limites ou se a posição do cursor é inválida
        if (current == NULL || editor->cx <= 0 || editor->cx > current->len) {
                return;
        }

        // Caso contrário, deletar um único caractere
        memmove(&current->c[editor->cx - 1], &current->c[editor->cx], current->len - editor->cx);
        current->len--;

        if (current->len == 0 && row > 0) {
                // Deletar a linha se ela estiver vazia após a remoção
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
                editor->cx = prev->len;
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
                                line *current = get_current_line_position(editor);
                                if (editor->cy > 0 && editor->cx < current->len) {
                                        editor->cy--;
                                        editor->current_row--;
                                }

                        }
                        break;

                case KEY_DOWN:
                        {
                                line * current = get_current_line_position(editor);
                                if (editor->cy < editor->num_rows && editor->cx < current->next->len) {
                                        editor->cy++;
                                        editor->current_row++;
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

void save_file(editor *editor)
{
        if (editor->filename == NULL) {
                editor->filename = strdup("Untitled.txt");
        }

        FILE *f = fopen(editor->filename, "w");
        if (f == NULL) {
                perror("Could not open file\n");
                return;
        }

        line *current = editor->head;
        while (current != NULL) {
                if (fwrite(current->c, sizeof(char), current->len, f) != (size_t)current->len) {
                        perror("could not write to file");
                        fclose(f);
                        return;
                }
                current = current->next;
        }

        fclose(f);
        mvprintw(editor->height - 1, 0, "File saved");
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
