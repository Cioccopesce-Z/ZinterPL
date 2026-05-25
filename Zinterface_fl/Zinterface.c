#include <ncurses.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

#define LINE_CODE 512
#define LENGHT_LINE_CODE 256

typedef struct{
    int text_y;
    int text_x;
    int title_x;
    int distance_from_top_wd;
    int base_distance_from_option;
} text_coordinate;

text_coordinate tx;

int highlight = 1;
int end = 1;
int option_selected = 0;

int y_win = 30;
int x_win = 80;
int option_number = 6;

const char *windows_text[] = {
	"write code",
	"clear code",
    "save code in file",
	"execute code",
    "settings",
	"exit",
};

char buffer_code[LINE_CODE][LENGHT_LINE_CODE] = {'\0'}; //lo riempillo di rigozze vuotozze
int code_line_lenght = 0;
int total_line = 0;

char ffile[32]; //file to save the buffer

void save_code_onto(char filename[]){

    FILE *file = fopen(filename, "w");
    if (!file) return;

    for(int i =0; i < total_line; i++){
        fprintf(file,"%s\n",buffer_code[i]);
    }

    fclose(file);
    
}

void settings() {
    WINDOW *set_wd = newwin(y_win, x_win, LINES/2 - y_win/2, COLS/2 - x_win/2);
    keypad(set_wd, TRUE);
    wbkgd(set_wd, COLOR_PAIR(2));
    box(set_wd, 0, 0);
    echo();
    mvwprintw(set_wd,tx.distance_from_top_wd,tx.title_x,"WRITE THE FILE YOU WANT THE CODE TO BE SAVED DEFENITLY");

    mvwprintw(set_wd, 7, 2, "Enter file name: ");
    wrefresh(set_wd);

    curs_set(1);

    // Let user type (max 20 chars)
    wgetnstr(set_wd, ffile, 20);

    for(int i = 0; i < strlen(ffile); i++){
        if(ffile[i]== ' ') ffile[i] = '_'; //elimina spazi
    }

    // Add extension
    strcat(ffile, ".Zinter");

    curs_set(0);
}

void setup_content(){

    tx.title_x = 10;

    tx.text_x = 20;
    tx.text_y = 2;

    tx.distance_from_top_wd = 1;
    tx.base_distance_from_option = 4;
}

void draw_screen(WINDOW *temp_win,const char *text[],char title[]){


    werase(temp_win);

    setup_content();
    int distance_bt_option = 2, x =0;

    mvwprintw(temp_win,tx.distance_from_top_wd,tx.title_x,"%s",title);
    for(int option = 0; option < option_number; option++){

        if(highlight == option + 1){
            wattron(temp_win,A_REVERSE);
            mvwprintw(
                temp_win,
                tx.text_y + tx.base_distance_from_option + x,
                tx.text_x,
                "%d. %s",
                option+1, text[option]
            );
            wattroff(temp_win,A_REVERSE);
        }

            
        else{
            mvwprintw(
                temp_win,
                tx.text_y + tx.base_distance_from_option + x,
                tx.text_x,
                "%s",
                text[option]
            );
        }
        x += distance_bt_option;

    }
    box(temp_win, 0, 0);     // bordo finestra
    wrefresh(temp_win);      // aggiorna finestra
        
}

void write_code(){

    WINDOW *code_wd = newwin(y_win, x_win, LINES/2-y_win/2, COLS/2-x_win/2);
    keypad(code_wd, TRUE);
    wbkgd(code_wd, COLOR_PAIR(2));
    box(code_wd, 0, 0);
    curs_set(1);
    
    mvwprintw(code_wd,tx.distance_from_top_wd,tx.title_x,"WRITE DOWN THE CODE BUDDY & hope the editor dont crash");

    

    int y = 5; //pos y cursore
    int x = 15; //pos x cursore
    int char_pos = 0;
    int ch;
    int shift_pressed = false;

    code_line_lenght = 0;
    total_line = 0;

    wmove(code_wd, y, x);
    wrefresh(code_wd);

    while((ch = wgetch(code_wd)) != 27){ // ESC to exit

        if(ch == '\n' && y< y_win - 2){ // new line
            buffer_code[code_line_lenght][char_pos] = '\0';
            code_line_lenght++;
            y++;
            char_pos = 0; //resetta posizione primo carattere nel buffer
            x = 15;
            total_line++;
        }
        else if(ch == KEY_BACKSPACE || ch == 127){ //cancella
            if(char_pos > 0){
                x--;
                char_pos--;

                for(int i = char_pos; buffer_code[code_line_lenght][i] != '\0' ; i++){
                    buffer_code[code_line_lenght][i] = buffer_code[code_line_lenght][i+1];
                }

                for(int i = 1; i < x_win-2; i++){
                    mvwaddch(code_wd,y,i,' ');
                }
                
                mvwprintw(code_wd,y,15,"%s",buffer_code[code_line_lenght]);
            }
            else if(char_pos<=0){
                if(code_line_lenght > 0){
                y--;
                code_line_lenght--;
                int len = strlen(buffer_code[code_line_lenght]);
                x = 15 + len;
                char_pos = len;
                }
            }
        }
        else if(ch == KEY_DOWN){
            if(code_line_lenght+1 <= total_line){
                int len = strlen(buffer_code[code_line_lenght+1]);
                char_pos = len;
                x = char_pos + 15;
                y++;
                code_line_lenght++;
            }
        }
        else if(ch == KEY_UP){
            if(code_line_lenght > 0){
                int len = strlen(buffer_code[code_line_lenght-1]);
                char_pos = len;
                x = char_pos + 15;
                y--;
                code_line_lenght--;
            }
        }   
        else if(ch == KEY_LEFT){
            if(x > 15){
                x--;
                char_pos--;
            }
            else if(x <= 15){
                if(code_line_lenght > 0){
                    y--;
                    code_line_lenght--;
                    int len = strlen(buffer_code[code_line_lenght]);
                    x = 15 + len;
                    char_pos = len;
                }
            }
        }
        else if(ch == KEY_RIGHT){
            if(char_pos < strlen(buffer_code[code_line_lenght])){
                x++;
                char_pos++;
            }
            else if(char_pos >= strlen(buffer_code[code_line_lenght]) && code_line_lenght+1 <= total_line){
                int len = strlen(buffer_code[code_line_lenght+1]);
                y++;
                code_line_lenght++;
                x = len + 15;
                char_pos = len;
            }
        }
        else if(ch == BUTTON_SHIFT || ch == TRACE_SHIFT){
                mvwprintw(code_wd,tx.distance_from_top_wd+1,15,"SHIFT PRESSED WAIT FOR SECOND KEY (-S FOR SAVE)");
                shift_pressed = true;
            }
        else if(ch == 's' && shift_pressed == true){
                mvwprintw(code_wd,tx.distance_from_top_wd+1,15,"SHIFT PRESSED WAIT FOR SECOND KEY (-S FOR SAVE)");
                shift_pressed = false;
            }
        else{ //hai scritto qualcosa

            if(ch == ' '){
                int len = strlen(buffer_code[code_line_lenght]);

                for(int i = len; i >= char_pos; i--){
                buffer_code[code_line_lenght][i+1] = buffer_code[code_line_lenght][i];

                }
                buffer_code[code_line_lenght][char_pos] = ' ';
                //mvwaddch(code_wd, y, x, ' ');
                mvwprintw(code_wd, y, 15, "%s", buffer_code[code_line_lenght]);

                x++;
                char_pos++;

            }
            else if(ch == '('){
                buffer_code[code_line_lenght][char_pos] = '(';
                buffer_code[code_line_lenght][char_pos + 1] = ')';
                buffer_code[code_line_lenght][char_pos + 2] = '\0';

                mvwaddch(code_wd, y, x, '(');
                mvwaddch(code_wd, y, x + 1, ')');

                x++;              // cursore tra le parentesi
                char_pos++;
            }
            else if(ch == '{'){
                buffer_code[code_line_lenght][char_pos] = '{';
                buffer_code[code_line_lenght][char_pos + 1] = '}';
                buffer_code[code_line_lenght][char_pos + 2] = '\0';

                mvwaddch(code_wd, y, x, '{');
                mvwaddch(code_wd, y, x + 1, '}');

                x++;
                char_pos++;
            }
            else if(ch == '&'){
                buffer_code[code_line_lenght][char_pos] = '&';
                buffer_code[code_line_lenght][char_pos + 1] = '&';
                buffer_code[code_line_lenght][char_pos + 2] = '\0';

                mvwaddch(code_wd, y, x, '&');
                mvwaddch(code_wd, y, x + 1, '&');

                x += 2;
                char_pos += 2;
            } 
            else{
                int len = strlen(buffer_code[code_line_lenght]);

                for(int i = len; i >= char_pos; i--){
                    buffer_code[code_line_lenght][i+1] = buffer_code[code_line_lenght][i];
                }

                buffer_code[code_line_lenght][char_pos] = ch;

                mvwprintw(code_wd, y, 15, "%s", buffer_code[code_line_lenght]);

                x++;
                char_pos++;
            }
        }

        wmove(code_wd, y, x);
        wrefresh(code_wd);
        save_code_onto("Zbuffer.Zbak");
    }
    mvwprintw(code_wd,tx.distance_from_top_wd+1,15,"CODE SAVED NO CHECK EXECUTE");
    mvwprintw(code_wd,tx.base_distance_from_option,15,"press any button to get back to menu......");
    wrefresh(code_wd);
    getch();
    
    curs_set(0);
    delwin(code_wd); // destroy widow
}


void exec_interpreter() {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }
    else if (pid == 0) {
        // Child process: redirect stdout/stderr to pipe
        close(pipefd[0]); // Close read end
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        char *args[] = {"./Zinterpreter", "-df", ffile, NULL};
        execvp(args[0], args);

        perror("execvp failed");
        exit(EXIT_FAILURE); // Ensure the child exits on failure
    }
    else {
        // Parent process
        close(pipefd[1]); // Close write end

        WINDOW *out_win = newwin(y_win, x_win, LINES / 2 - y_win / 2, COLS / 2 - x_win / 2);
        wbkgd(out_win, COLOR_PAIR(2));
        box(out_win, 0, 0);
        wrefresh(out_win);

        char buf[256];
        int row = 1;
        int n = 0;
        char *newline;
        char incomplete_line[256] = "";  // Buffer for incomplete lines

        while ((n = read(pipefd[0], buf, sizeof(buf) - 1)) > 0) {
            buf[n] = '\0';  // Null-terminate the new data
            strcat(incomplete_line, buf);  // Concatenate to previous incomplete data

            // Process full lines (split by '\n')
            while ((newline = strchr(incomplete_line, '\n')) != NULL) {
                *newline = '\0';  // Terminate the string at '\n'
                mvwprintw(out_win, row++, 1, "%s", incomplete_line);
                wrefresh(out_win);

                // Move to the next part of the incomplete data
                strcpy(incomplete_line, newline + 1);
            }

            // If there's data left (no newline at the end)
            if (*incomplete_line != '\0') {
                mvwprintw(out_win, row++, 1, "%s", incomplete_line);
                wrefresh(out_win);
            }
        }

        close(pipefd[0]);
        waitpid(pid, NULL, 0);  // Wait for child process to finish

        mvwprintw(out_win, y_win - 2, 1, "Press any key to return...");
        wrefresh(out_win);
        getch();  // Wait for user input
        delwin(out_win);  // Clean up the window
    }
}

void reset_buffer(){
    
    WINDOW *res_buff = newwin(y_win, x_win, LINES/2-y_win/2, COLS/2-x_win/2);
    init_pair(2, COLOR_WHITE, COLOR_BLUE);
    wbkgd(res_buff, COLOR_PAIR(2));

    box(res_buff, 0, 0);
    
    for(int i=0; i <= total_line; i++){
        buffer_code[i][0] = '\0';
    }
    mvwprintw(res_buff,tx.distance_from_top_wd,tx.title_x,"Your buffer has been cleaned you can now write new code");
    wrefresh(res_buff);
    getch();
    delwin(res_buff);
}

void save_code(){
    WINDOW *saving = newwin(y_win,x_win,LINES/2-y_win/2,COLS/2-x_win/2);
    wbkgd(saving, COLOR_PAIR(2));
    box(saving,0,0);
    mvwprintw(saving, tx.base_distance_from_option-1, tx.title_x,"SAVING your code onto file: %s",ffile);
    save_code_onto(ffile);
    
    mvwprintw(saving, tx.base_distance_from_option, tx.title_x,"your code has been SAVED onto: %s",ffile);
    mvwprintw(saving, tx.base_distance_from_option+1, tx.title_x,"press y if you want to delete the backup");
    wrefresh(saving);
    int ch = wgetch(saving);
    if(ch == 'y'){
        remove("Zbuffer.Zbak");
        mvwprintw(saving, tx.base_distance_from_option+2, tx.title_x,"Zbffer.bak got executed sorry for your loss btw | || || |_");
    }
    
    wrefresh(saving);
    getch();
}

void exec_choice(int choice){
    if(choice == 6) end = 0;
    else if(choice == 1) write_code();
    else if(choice == 2) reset_buffer();
    else if(choice == 3) save_code();
    else if(choice == 4) exec_interpreter();
    else if(choice == 5) settings();;
    option_selected = 0;
    return;
}

int main() {

    initscr();          // inizializza ncurses
    start_color();      // abilita i colori
    cbreak();           // input immediato
    noecho();           // non mostra i tasti premuti
    curs_set(0);        // non mostrare il cursore
    
    strcpy(ffile,"Zstd_name.Zinter");
    init_pair(2, COLOR_WHITE, COLOR_BLUE); // definisce coppia colore

    refresh();          // aggiorna stdscr

    WINDOW *windows = newwin(y_win,x_win,LINES/2-y_win/2,COLS/2-x_win/2); // crea finestra

    keypad(windows, TRUE);   // attiva frecce sulla finestra
    wbkgd(windows, COLOR_PAIR(2));   // attiva colore

    int ch;
    

    while(end){
        draw_screen(windows, windows_text,"ZINTER PROGRAMMING LANGUAGE INTERPRETER GUI");    // stampa nella finestra
        
        ch = wgetch(windows);

        switch (ch){

        case KEY_UP:
            if(highlight > 1)
            highlight--;
            break;
        
        case KEY_DOWN:
                if(highlight < option_number)
                    highlight++;
            break;

        case 10:
            option_selected = highlight;
            break;
        }

        exec_choice(option_selected);
    }
    

    endwin();           // chiude ncurses
    return 0;
}
