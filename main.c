
#include <ncurses.h>
#define CTRL(x) ((x) & 0x1F)

#include "editor.h"

int main() 
{
        init_ncurses();
        editor editor;
        init_editor(&editor);
        int c;

        while (!editor.should_close) {
                c = getch();
                switch (c) {
                        case CTRL('q'):
                                editor.should_close = 1;
                                break;

                        case KEY_BACKSPACE:
                        case 127:
                                delete_char(&editor);
                                break;

                        case '\n':
                                insert_new_line(&editor);
                                break;

                        case 'p':
                                {
                                        line *current = editor.head;
                                        while (current) {
                                                printf("%s ", current->c);
                                                printf("%d\n", current->len);
                                                current = current->next;
                                        }
                                }
                                break;

                        case KEY_UP:
                        case KEY_DOWN:
                        case KEY_LEFT:
                        case KEY_RIGHT:
                                update_cursor_position(&editor, c);
                                break;

                       default:
                                insert_char(&editor, c);
                                break;
                }
                clear();
                print_rows(&editor);
                move(editor.cy, editor.cx);
                refresh();
        }

        end_ncurses();
        return 0;
}
