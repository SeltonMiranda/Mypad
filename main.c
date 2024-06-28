
#include "editor.h"

#define CTRL(x) ((x) & 0x1F)

int main(int argc, char *argv[]) 
{
        init_ncurses();
        editor editor;
        init_editor(&editor);

        if (argc == 2) {
                load_file(&editor, argv[1]);
                print_rows(&editor);
        }
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

                        case CTRL('s'):
                                save_file(&editor);
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
                scroll_rows(&editor);
                print_rows(&editor);
                move(editor.cy - editor.scroll_offset, editor.cx);
                refresh();
        }

        free_editor(&editor);
        end_ncurses();
        return 0;
}
