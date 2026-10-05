import curses
from curses.textpad import Textbox, rectangle
import sys

def main(stdscr, min_mb):
    curses.start_color()
    curses.init_pair(1, curses.COLOR_WHITE, curses.COLOR_BLUE)
    curses.init_pair(2, curses.COLOR_BLACK, curses.COLOR_WHITE)
    curses.init_pair(3, curses.COLOR_WHITE, curses.COLOR_BLACK)
    curses.init_pair(4, curses.COLOR_WHITE, curses.COLOR_CYAN)

    curses.curs_set(0)
    sh, sw = stdscr.getmaxyx()
    stdscr.bkgd(' ', curses.color_pair(1))

    box_h, box_w = 10, 50
    box_y, box_x = (sh - box_h) // 2, (sw - box_w) // 2

    stdscr.attron(curses.color_pair(3))
    for y in range(box_y + 1, box_y + box_h + 1):
        stdscr.addstr(y, box_x + 2, " " * box_w)
    stdscr.attroff(curses.color_pair(3))

    stdscr.attron(curses.color_pair(2))
    for y in range(box_y, box_y + box_h):
        stdscr.addstr(y, box_x, " " * box_w)
    
    rectangle(stdscr, box_y, box_x, box_y + box_h - 1, box_x + box_w - 1)
    
    title = " Persistent Partition "
    stdscr.addstr(box_y, box_x + (box_w - len(title)) // 2, title)
    
    msg = f"Enter size for PERSIST in MB (min {min_mb}):"
    stdscr.addstr(box_y + 2, box_x + 2, msg)
    stdscr.attroff(curses.color_pair(2))

    inp_y, inp_x = box_y + 4, box_x + 2
    inp_w = box_w - 4
    inp_win = curses.newwin(1, inp_w, inp_y, inp_x)
    inp_win.bkgd(' ', curses.color_pair(4))
    
    stdscr.refresh()
    curses.curs_set(1)
    box = Textbox(inp_win)
    
    def validator(ch):
        if ch in (10, 13): return 7
        if ch in (curses.KEY_BACKSPACE, 127, 8): return curses.KEY_BACKSPACE
        if not (48 <= ch <= 57): return None
        return ch

    while True:
        inp_win.clear()
        inp_win.refresh()
        result = box.edit(validator).strip()
        
        if result.isdigit() and int(result) >= min_mb:
            return result
        else:
            curses.init_pair(5, curses.COLOR_WHITE, curses.COLOR_RED)
            err = f" Minimum size is {min_mb}MB! Press any key... "
            stdscr.attron(curses.color_pair(5))
            stdscr.addstr(box_y + 7, box_x + (box_w - len(err)) // 2, err)
            stdscr.attroff(curses.color_pair(5))
            stdscr.refresh()
            
            curses.curs_set(0)
            stdscr.getch()
            curses.curs_set(1)
            
            stdscr.attron(curses.color_pair(2))
            stdscr.addstr(box_y + 7, box_x + 1, " " * (box_w - 2))
            stdscr.attroff(curses.color_pair(2))

if __name__ == "__main__":
    min_mb = int(sys.argv[1]) if len(sys.argv) > 1 else 8
    target_file = sys.argv[2] if len(sys.argv) > 2 else None

    try:
        output = curses.wrapper(main, min_mb)
        
        if target_file:
            with open(target_file, "w") as f:
                f.write(str(output))
        else:
            print(output)
            
    except KeyboardInterrupt:
        sys.exit(1)