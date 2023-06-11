#include <stdio.h>
#include <curses.h>
#include <string.h>

int main()
{
    initscr();

    WINDOW *input_border = newwin(3, 50, 0, 0);
    wborder(input_border, '|', '|', '-', '-', '+', '+', '+', '+');
    wrefresh(input_border);

    WINDOW *log_border = newwin(16, 50, 3, 0);
    wborder(log_border, '|', '|', '-', '-', '+', '+', '+', '+');
    wrefresh(log_border);

    WINDOW *input_win = newwin(1, 48, 1, 1);
    wrefresh(input_win);

    WINDOW *log_win = newwin(14, 48, 4, 1);
    scrollok(log_win, TRUE);
    wrefresh(log_win);

    char str[256];

    while (1)
    {
        wclear(input_win);
        wrefresh(input_win);
        
        wgetstr(input_win, str);
        if (strcmp(str, "exit") == 0) break;

        wprintw(log_win, "%s\n", str);
        wrefresh(log_win);
    }

    delwin(input_border);
    delwin(log_border);
    delwin(input_win);
    delwin(log_win);

    endwin();
    return 0;
}