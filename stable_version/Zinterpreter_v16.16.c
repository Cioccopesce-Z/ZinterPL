#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define max_name_lettere 16
#define max_number_of_array 64
#define max_number_of_var 64
#define max_number_of_cell 64
#define max_lenght_of_string 256
#define max_number_of_reg 16
#define max_number_of_concatened_condition 64
#define tru 1
#define fal 0
#define base_memory 256


int line_idx_program = 0;
int place_holder = 0;
int deb = tru;

//pre dichiarazione
typedef struct decl_var decl_var;
typedef struct decl_fl_var decl_fl_var;
typedef struct decl_var_char decl_var_char;

//struct for int var
struct decl_var {
    char name[max_name_lettere];
    int value;
    decl_var* pointer;
};

//struct for float var
struct decl_fl_var {
    char name[max_name_lettere];
    float value;
    decl_fl_var* pointer;
};

//struct for char var
struct decl_var_char {
    char name[max_name_lettere];
    char value;
    decl_var_char* pointer;
};


//struct for int array declaration
typedef struct {
    char name[max_name_lettere];
    int array_int[max_number_of_cell]; // Array(simulato) dentro la struct
    int size;
} decl_arr;

//struct for float array declaration
typedef struct {
    char name[max_name_lettere];
    float array_int[max_number_of_cell]; // Array(simulato) dentro la struct
    int size;
} decl_fl_arr;

//struct for char array declaration
typedef struct {
    char name[max_name_lettere];
    char array_int[max_lenght_of_string]; // Array(simulato) dentro la struct
    int size;
} decl_char_arr;


//struct for int matrix declaration
typedef struct {
    char name[max_name_lettere];
    int matrix_int[max_number_of_cell][max_number_of_cell]; // matrice(simulato) dentro la struct
    int size_first;
    int size_sec;
} decl_matr;

//struct for float matrix declaration
typedef struct {
    char name[max_name_lettere];
    float matrix_fl_int[max_number_of_cell][max_number_of_cell]; // matrice(simulato) dentro la struct
    int size_first;
    int size_sec;
} decl_fl_matr;

//struct for char matrix declaration
typedef struct {
    char name[max_name_lettere];
    char matrix_str_int[max_number_of_cell][max_number_of_cell]; // matrice(simulato) dentro la struct
    int size_first;
    int size_sec;
} decl_char_matr;


//redacted program
typedef struct {
    int line_number; //di solito è uguale all'indice in program[idx]
    char instruction[512];
} program_line;

//salva stato
typedef struct {
    char nome_function[16];
    int posizione_ritorno;
    int codice; //per concatenamenti
    int posizione_skip; //per if e while e for
} program_state;

//evita conflitti se si hanno piu intepreti aperti
typedef struct {

    // ===== variabili =====
    decl_var variable[max_number_of_var]; 
    int variable_count;

    decl_fl_var fl_variable[max_number_of_var];
    int fl_variable_count;

    decl_var_char char_variable[max_number_of_var];
    int char_variable_count;

    // ===== array =====
    decl_arr array[max_number_of_array];
    int array_count;

    decl_fl_arr fl_array[max_number_of_array];
    int fl_array_count;

    decl_char_arr char_array[max_number_of_array];
    int char_array_count;

    // ===== matrix =====
    decl_matr matrix[max_number_of_array];
    int matrix_count;

    decl_fl_matr fl_matrix[max_number_of_array];
    int fl_matrix_count;

    decl_char_matr char_matrix[max_number_of_array];
    int char_matrix_count;


    // ===== registri =====
    int r[max_number_of_reg]; 
    char cr[max_number_of_reg];

    // ===== programma =====
    program_line program[512];
    int line_idx_program;

    // ===== stato =====
    program_state state_stack[max_number_of_concatened_condition];
    int return_state;

    // ===== esecuzione =====
    int global_ip;

    // ===== debug / extra =====
    int deb;
    int place_holder;
    char char_place_holder;


    int *pt_place_holder;
    char *pt_char_place_holder;

} VM;

VM vm;

// ===== variabili =====
#define variable vm.variable
#define variable_count vm.variable_count

#define fl_variable vm.fl_variable
#define fl_variable_count vm.fl_variable_count

#define char_variable vm.char_variable
#define char_variable_count vm.char_variable_count

// ===== array =====
#define array vm.array
#define array_count vm.array_count

#define fl_array vm.fl_array
#define fl_array_count vm.fl_array_count

#define char_array vm.char_array
#define char_array_count vm.char_array_count

// ===== matrix =====
#define matrix vm.matrix
#define matrix_count vm.matrix_count

#define fl_matrix vm.fl_matrix
#define fl_matrix_count vm.fl_matrix_count

#define char_matrix vm.char_matrix
#define char_matrix_count vm.char_matrix_count

// ===== registri =====
#define r vm.r
#define cr vm.cr

// ===== programma =====
#define program vm.program
#define line_idx_program vm.line_idx_program

// ===== stato =====
#define state_stack vm.state_stack
#define return_state vm.return_state

// ===== esecuzione =====
#define global_ip vm.global_ip

// ===== debug =====
#define deb vm.deb
#define place_holder vm.place_holder
#define char_place_holder vm.char_place_holder
#define pt_place_holder vm.pt_place_holder
#define pt_char_place_holder vm.pt_char_place_holder

//check for variabile int or char
int is_var_(const char *name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variable[i].name, name) == 0) {
            return 1; // è una variabile int
        }
    }
    for (int i = 0; i < char_variable_count; i++) {
        if (strcmp(char_variable[i].name, name) == 0) {
            return 2; // è una variabile char
        }
    }
    for (int i = 0; i < fl_variable_count; i++) {
        if (strcmp(fl_variable[i].name, name) == 0) {
            return 2; // è una variabile char
        }
    }
    return 0; // non è una variabile
}

int is_arr_(const char *name) {
    for (int i = 0; i < array_count; i++) {
        if (strcmp(array[i].name, name) == 0) {
            return 1; // è una arr int
        }
    }
    for (int i = 0; i < char_array_count; i++) {
        if (strcmp(char_array[i].name, name) == 0) {
            return 2; // è un arr char
        }
    }
    for (int i = 0; i < fl_array_count; i++) {
        if (strcmp(fl_array[i].name, name) == 0) {
            return 3; // è un arr char
        }
    }
    return 0; // non è una variabile
}

int is_matrix_(const char *name) {
    for (int i = 0; i < matrix_count; i++) {
        if (strcmp(matrix[i].name, name) == 0) {
            return 1; // è una matrice int
        }
    }
    for (int i = 0; i < char_matrix_count; i++) {
        if (strcmp(char_matrix[i].name, name) == 0) {
            return 2; // è una matrice char
        }
    }
    for (int i = 0; i < fl_matrix_count; i++) {
        if (strcmp(fl_matrix[i].name, name) == 0) {
            return 3; // è una matrice char
        }
    }
    return 0; // non è una variabile
}

int is_reg_(const char *name) {
    int bin;
    int n = sscanf(name,"r%d",&bin);
    if(n == 0) return 0; // non è un reg
    return 1; //reg
}

//function to get the index of variable, array and register

int temp_int;
char temp_char;

void* get_index(char data_sruct_name[]) {

    int borrow;
    char var_name[max_name_lettere];
    char type;
    char var_type;
    
    
    //NUMERO
    if(sscanf(data_sruct_name, "&%c&%d&", &type, &temp_int) == 2 && type == 'n'){
        return &temp_int; //serve ma non va modificato
    }
    //SE CARATTERE
    else if(sscanf(data_sruct_name, "&%c&%c&", &type, &temp_char) == 2 && type == 'k'){
        return &temp_char; //serve ma non va modificato
    }
    

    // REGISTRO (r3)
    if (sscanf(data_sruct_name, "&%c&r%d&", &type, &borrow) == 2 && type == 'i' && borrow >= 0 && borrow < max_number_of_reg) {
        return &r[borrow];
    }
    else if (sscanf(data_sruct_name, "&%c&cr%d&", &type, &borrow) == 2 && type == 'c' && borrow >= 0 && borrow < max_number_of_reg) {
        return &cr[borrow];
    }

    // ARRAY (esempio &i[something]&array& )
    else if (strchr(data_sruct_name, '[') != NULL) {

        char arr_name[max_name_lettere];
        char index_str[50];

        int check_str_print = sscanf(data_sruct_name, "&%c[%15[^]]]&%15[^&]&", &type, index_str, arr_name);

        if(deb) printf("get_index riconosciuto array %s, indice: %s \n", arr_name, index_str);

        int index;

        // Se l'indice è numero diretto
        if (sscanf(index_str, "%d", &index) == 1 && check_str_print == 3) {

            

            if(type =='s'){
                for (int i = 0; i < char_array_count; i++) {
                    if (strcmp(char_array[i].name, arr_name) == 0 && index >= 0 && index < char_array[i].size) {
                        return &char_array[i].array_int[index]; 
                    }
                }
                printf("OUT OF BOUNDS for array: %s, with delcaration: %s",arr_name,data_sruct_name);
                    return NULL;
            }

            for (int i = 0; i < array_count; i++) {
                if (strcmp(array[i].name, arr_name) == 0 && index >= 0 && index < array[i].size) {
                    return &array[i].array_int[index];
                }
            }
            printf("OUT OF BOUNDS for array: %s, with delcaration: %s",arr_name,data_sruct_name);
                    return NULL;


        }

        // Se l'indice è un registro (es: r2)
        else if (sscanf(index_str,"r%d",&borrow) == 1 && borrow >= 0 && borrow < max_number_of_reg && check_str_print == 3) {

            int index_value = r[borrow];

            if(type =='s'){
                for (int i = 0; i < char_array_count; i++) {
                    if (strcmp(char_array[i].name, arr_name) == 0 && index_value >= 0 && index_value < char_array[i].size) {
                        return &char_array[i].array_int[index_value]; 
                    }
                }
                printf("OUT OF BOUNDS for array: %s, with delcaration: %s",arr_name,data_sruct_name);
                    return NULL;
            }

            for (int i = 0; i < array_count; i++) {
                if (strcmp(array[i].name, arr_name) == 0 && index_value >= 0 && index_value < array[i].size) {
                    return &array[i].array_int[index_value];
                }
            }
            printf("OUT OF BOUNDS for array: %s, with delcaration: %s",arr_name,data_sruct_name);
                    return NULL;
        }

        // Se l'indice è una var (es: &i&var&)
        else if (sscanf(index_str, "&%c&%15[^&]&", &var_type, var_name) == 2 && check_str_print == 3) {


            if(is_var_(var_name)!=1){ 
                printf("Errore: la variabile '%s' non è di tipo int. e non puo essere usata come indice per l'array %s\n", var_name, arr_name); 
                return NULL; 
            } // se la variabile non è un int, ritorna NULL
            
            int *index_value = get_index(index_str); // Ottieni il valore della variabile;
            
            if (index_value == NULL)
                return NULL;

            if(type =='s'){
                for (int i = 0; i < char_array_count; i++) {
                    if (strcmp(char_array[i].name, arr_name) == 0 && *index_value >= 0 && *index_value < char_array[i].size) {
                        return &char_array[i].array_int[*index_value]; 
                    }
                }
                printf("OUT OF BOUNDS for array: %s, with delcaration: %s",arr_name,data_sruct_name);
                    return NULL;
            }

            for (int i = 0; i < array_count; i++) {
                if (strcmp(array[i].name, arr_name) == 0 && *index_value >= 0 && *index_value < array[i].size) {
                    return &array[i].array_int[*index_value];
                }
            }
            printf("OUT OF BOUNDS for array: %s, with delcaration: %s",arr_name,data_sruct_name);
            return NULL;
        }

        //se si vuole stampare l'intera stringa (indice vuoto)
        else if (check_str_print == 2 && type == 's') {
            for (int i = 0; i < char_array_count; i++) {
                if (strcmp(char_array[i].name, arr_name) == 0) {
                    return char_array[i].array_int; // ritorna puntatore al primo elemento
                }
            }
            return NULL;
        }
    }

    //VARIABILE (es: &i&var&)
    else if(sscanf(data_sruct_name, "&%c&%15[^&]&",&type, var_name) == 2){
        if(deb) printf("get_index variabile %s di tipo %c \n", var_name, type);

        if(type =='i'){
            for (int i = 0; i < variable_count; i++) {
                if (strcmp(variable[i].name, var_name) == 0) {
                    
                    return &variable[i].value;
                }
            }
        }

        if(type =='c'){
            for (int i = 0; i < char_variable_count; i++) {
                if (strcmp(char_variable[i].name, var_name) == 0) {
                    return &char_variable[i].value; 
                }
            }
        }

    }

    return NULL; // numero immediato o errore
}

void* resolve(char *type, char *name){
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "&%s&%s&", type, name);
    if(type[0] == 'i' || type[0] == 'n')
    return (int *)get_index(buffer);
    if(type[0] == 's' || type[0] == 'k' || type[0] == 'c')
    return (char *)get_index(buffer);
}

//declare matrix variable and array
void declare_matrix(char name[], char type, int size_one, int size_two) {

    //matrix int
    if(type =='i'){
        strcpy(matrix[matrix_count].name, name);
        matrix[matrix_count].size_first = size_one;
        matrix[matrix_count].size_sec = size_two;

        matrix_count++;

        if(matrix_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di matrici int raggiunto.\n");
        }
    }

    //matrix float
    if(type =='l'){
        strcpy(fl_matrix[matrix_count].name, name);
        fl_matrix[matrix_count].size_first = size_one;
        fl_matrix[matrix_count].size_sec = size_two;

        fl_matrix_count++;

        if(fl_matrix_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di matrici fl raggiunto.\n");
        }
    }

    //matrix char
    if(type =='s'){
        strcpy(char_matrix[char_matrix_count].name, name);
        char_matrix[matrix_count].size_first = size_one;
        char_matrix[matrix_count].size_sec = size_two;

        char_matrix_count++;

        if(char_matrix_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di matrici char raggiunto.\n");
        }
    }
}

void declare_variable(char name[], char type) {

    //var int
    if(type =='i'){
        strcpy(variable[variable_count].name, name);
        variable[variable_count].value = 0;
        variable[variable_count].pointer = &variable[variable_count];

        variable_count++;

        if(variable_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di variabili int raggiunto.\n");
        }
    }

    //var fl
    if(type =='l'){
        strcpy(fl_variable[fl_variable_count].name, name);
        fl_variable[fl_variable_count].value = 0;
        fl_variable[fl_variable_count].pointer = &fl_variable[variable_count];
        
        fl_variable_count++;

        if(fl_variable_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di variabili float raggiunto.\n");
        }
    }

    //var char
    if(type =='c'){
        strcpy(char_variable[char_variable_count].name, name);
        char_variable[char_variable_count].value = 0;
        char_variable[char_variable_count].pointer = &char_variable[variable_count];

        char_variable_count++;

        if(char_variable_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di variabili char raggiunto.\n");
        }
    }
}

void declare_array(char name[], char type, int size) {

    if(type =='i'){

        strcpy(array[array_count].name, name);

        if(size > max_number_of_cell) {
            size = max_number_of_cell; // Limita la dimensione dell'array
            printf("Attenzione: la dimensione dell'array '%s' è stata limitata a %d.\n", name, max_number_of_cell);
        }

        array[array_count].size = size;

        for (int i = 0; i < size; i++) {
            array[array_count].array_int[i] = 0; // Inizializza a 0
        }

        array_count++;
    }

    if(type =='s'){

        strcpy(char_array[char_array_count].name, name);

        if(size > max_number_of_cell) {
            size = max_number_of_cell; // Limita la dimensione dell'array
            printf("Attenzione: la dimensione dell'array '%s' è stata limitata a %d.\n", name, max_number_of_cell);
        }

        char_array[char_array_count].size = size;

        for (int i = 0; i < size; i++) {
            char_array[char_array_count].array_int[i] = 0; // Inizializza a 0
        }

        char_array_count++;
    }
}

void clean_memory() {
    variable_count = 0;
    array_count = 0;
    char_array_count = 0;
    for (int i = 0; i < max_number_of_reg; i++) {
        r[i] = 0;
    }
}

//add to array and variable
void set_to_array(char name[], char type, int index, int value, char cvalue) {

    if(type =='i') {
        for (int i = 0; i < array_count; i++) {

            if (strcmp(array[i].name, name) == 0) {

                if (index >= 0 && index < array[i].size) {
                    array[i].array_int[index] = value;
            }
                else {
                printf("Errore: indice fuori dai limiti per l'array '%s'.\n", name);
                }
                return;
            }
        }
    }

    if(type =='s') {
        for (int i = 0; i < char_array_count; i++) {

            if (strcmp(char_array[i].name, name) == 0) {

                if (index >= 0 && index < char_array[i].size) {
                    char_array[i].array_int[index] = cvalue;
                }
                else {
                printf("Errore: indice fuori dai limiti per l'array '%s'.\n", name);
                }
                return;
            }
        }
    }
    printf("Errore: array '%s' non trovato.\n", name);
}

void set_to_variable(char name[], char type, int value, char cvalue) {

    char buffer[64];
    
    sprintf(buffer,"&%c&%s&",type,name);    

    if(type == 'i'){
        int *temp = (int*)get_index(buffer);

        if (temp == NULL) {
            printf("Errore: variabile '%s' non trovata.\n", name);
            return;
        }
        *temp = value;
    }

    if(type == 'c'){
        char *temp = (char*)get_index(buffer);

        if (temp == NULL) {
            printf("Errore: variabile '%s' non trovata.\n", name);
            return;
        }
        *temp = cvalue;
    }
}

//read code from file
char* read_code_from_file(const char *filename) {

    int is_string = fal; // tru = 1 fal = 0

    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    
    size_t total_size = 0;
    size_t capacity = 1024;

    char *result = malloc(capacity);
    if (!result) {
        fclose(file);
        return NULL;
    }
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), file)) {

        size_t len = strlen(buffer); // lunghezza della riga letta

        for (size_t i = 0; i < len; i++) {

            // salta newline e tab
            if (buffer[i] == '\n' || buffer[i] == '\t')
                continue; //ritorna all'inizio del ciclo senza eseguire il resto del codice

            // se vuoi anche rimuovere spazi extra:
            if (buffer[i] == '"' && (i == 0 || buffer[i-1] != '\\')){
                is_string = !is_string;
                i++;
            }
            if (!is_string && buffer[i] == ' ') continue;

            // se serve più memoria, rialloca
            if (total_size + 1 >= capacity) {
                capacity *= 2;
                char *temp = realloc(result, capacity); // raddoppia la capacità
                if (!temp) {
                    free(result);
                    fclose(file);
                    return NULL;
                }
                result = temp;
            }

            result[total_size++] = buffer[i]; //copia carattere per carattere da buffer a resault
            //total_size viene incrementato dopo lassegnazione, 
            //quindi punta sempre alla posizione successiva disponibile in result
        }
    }

    result[total_size] = '\0';  // chiude la stringa

    fclose(file);
    return result;
}

void format_code(const char *code) {
    int i = 0;
    int j = 0;   // indice nella riga corrente

    line_idx_program = 0;

    while(code[i] != '\0') {

        if(code[i] == ':' || code[i] == '{' || code[i] == '}') {
            if(code[i] != ':') program[line_idx_program].instruction[j] = code[i]; 
            j++;
            program[line_idx_program].instruction[j] = '\0';
            program[line_idx_program].line_number = line_idx_program; // assegna numero di linea
            line_idx_program++;
            j = 0;  // reset indice riga

        } else {

            program[line_idx_program].instruction[j] = code[i];
            j++;
        }

        i++;
    }

    // chiudi ultima riga se necessario
    if(j > 0) {
        program[line_idx_program].instruction[j] = '\0';
        line_idx_program++;
    }
    line_idx_program--; // corregge l'ultimo incremento in eccesso
}

// =============pre parser function ====================

int starts_with(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}



//funzioni di dichiarazione
void exec_int(char *text){
    char type[16];
    char name[16];

    sscanf(text,"int_&%15[^&]&%15[^&]&",type,name);
    if(strcmp(type,"i") == 0) declare_variable(name,'i'); //&i&name&

    /*if array*/
    char name_of_type[16];
    char type_of_type;

    if(strchr(type,'[')){   //&i[ qualcosa ]&name&
        
        if( sscanf(type,"%c[%15[^]]]",&type_of_type,name_of_type) == 2 && type_of_type == 'i' ){
            
            int size;
            if(sscanf(name_of_type, "%d", &size) == 1){
                declare_array(name,'i',size);
            }
            else{
                int *ptr0 = get_index(name_of_type);
                if(ptr0 == NULL){
                    printf("segmentation error for ptr0 in exec_int on line: %s \n",text);
                    return;
                    }
                declare_array(name,'i',*ptr0);
            }
        }
        else{
            printf("errore nella dichiarazione INT nella riga: %s \n",text);
            return;
            
        }
    }
    
}

void exec_char(char *text){
    char type[16];
    char name[16];

    sscanf(text,"char_&%15[^&]&%15[^&]&",type,name);
    if(strcmp(type,"c") == 0) declare_variable(name,'c'); //&i&name&

    /*if array*/
    char name_of_type[16];
    char type_of_type;
    if(strchr(type,'[')){   //&i[ qualcosa dimensione ]&name&
        if( sscanf(type,"%c[%15[^]]]",&type_of_type,name_of_type) == 2 && type_of_type == 's' ){
            
            int size;
            if(sscanf(name_of_type, "%d", &size) == 1){
                declare_array(name,'s',size);
            }
            else{
                int *ptr0 = get_index(name_of_type);
                if(ptr0 == NULL){
                    printf("segmentation error for ptr0 in exec_int on line: %s \n",text);
                    return;
                    }
                declare_array(name,'s',*ptr0);
            }
        }
        else{
            printf("errore nella dichiarazione CHAR nella riga: %s \n",text);
            return;
            
        }
    }
}

int get_array_size(char arr_name[], char type){

    if(type == 's')
    for(int i=0; i < char_array_count; i++){
        if(strcmp(char_array[i].name,arr_name) == 0)
        return char_array[i].size;
    }

    else if(type == 'i')
    for(int i=0; i < array_count; i++){
        if(strcmp(array[i].name,arr_name) == 0)
        return array[i].size;
    }
    return -1;
}

//funzion input output

void exec_print(char *text){

    char type[64] = {0};
    char name[128] = {0};

    // & i & nome &
    // & i[] & nome & 
    // & i[][] & nome & 
    
    int n = sscanf(text,"print_&%15[^&]&%127[^&]&",type,name);
    if(deb) printf("DEBUG PRINT: type=%s name=%s\n", type, name);

    if(n < 1){
        printf("Errore parsing print: %s\n", text);
        return;
    }

    if(n == 1){
        name[0] = '\0'; // stringa vuota sicura
    }


    char rebuilt[256];
    snprintf(rebuilt, sizeof(rebuilt), "&%s&%s&",type,name);

    /*TESTO VUOTO*/ 
    if(strcmp(type,"s") == 0 && name[0] == '\0'){ //&s&& --------> " "
        printf(" "); 
        return;
    }

    /*TESTO PURO*/
    else if(strcmp(type,"s") == 0) printf("%s",name); //&s&name& -----> name

    /*NUMERO PURO*/
    else if(strcmp(type,"n") == 0) printf("%s",name); //&n&99& -----> 99

    /*VARIABILE*/
    else if(strcmp(type,"i") == 0){
        int *ptr0 = (int *)get_index(rebuilt); //&i&name& ------> 3    (se name contiene 3)
        if(ptr0 == NULL) return;    
        printf("%d",*ptr0);
    }
    
    else if(strcmp(type,"l") == 0){
        float *ptr0 = (float *)get_index(rebuilt); //&i&name& ------> 3.14    (se name contiene 3)
        if(ptr0 == NULL) return;    
        printf("%f",*ptr0);
    }
    

    else if(strcmp(type,"c") == 0){
        char *ptr0 = (char *)get_index(rebuilt); //&c&name& ------> k    (se name contiene il car k)
        if(ptr0 == NULL) return;
        printf("%c",*ptr0);
    }

    /*ARRAY*/
    else if( starts_with(type,"i[") ){
        int *ptr0 = (int *)get_index(rebuilt); //&i[num/var/reg]&name& ------> 3    (se name[some] contiene 3)
        if(ptr0 == NULL) return;
        printf("%d",*ptr0);
    }
    /*se fl array*/
    else if( starts_with(type,"l[") ){
        float *ptr0 = (float *)get_index(rebuilt); //&i[num/var/reg]&name& ------> 3.14    (se name[some] contiene 3)
        if(ptr0 == NULL) return;
        printf("%f",*ptr0);
    }

    /*se stringa con indice*/
    char bin[max_lenght_of_string];
    n = sscanf(type,"s[%15[^]]]",bin);
    if( n == 1 ){
        char *ptr0 = (char *)get_index(rebuilt); //&s[num/var/reg]&name& ------> k    (se name contiene il car k)
        if(ptr0 == NULL || *ptr0 == '\0') return;
        printf("%c",*ptr0);
    }

    /*MATRIX*/
    if(strstr(type,"][") && name[0] != '\0'){

        if( starts_with(type,"i[") ){
            int *ptr0 = (int *)get_index(rebuilt); //&i[num/var/reg][num/var/reg]&name& ------> 3    (se name[some] contiene 3)
            if(ptr0 == NULL) return;
            printf("%d",*ptr0);
        }
        /*se fl matrix*/
        else if( starts_with(type,"l[") ){
            float *ptr0 = (float *)get_index(rebuilt); //&i[num/var/reg][num/var/reg]&name& ------> 3.14    (se name[some] contiene 3)
            if(ptr0 == NULL) return;
            printf("%f",*ptr0);
        }

        /*se matrice con indice*/
        n = sscanf(type,"s[%15[^]]]",bin);
        if( n == 1 ){
            char *ptr0 = (char *)get_index(rebuilt); //&s[num/var/reg][num/var/reg]&name& ------> k    (se name contiene il car k)
            if(ptr0 == NULL || *ptr0 == '\0') return;
            printf("%c",*ptr0);
        }

        /*se stampa intera matrice*/
        else if(strcmp(type,"s[][]") == 0){
            printf("U cant expect me to write a function for printing a whole matrix \n");
        }
    }
    
}

void exec_lnprint(char *text){
    printf("\n");
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "print_%s", text + 8);
    exec_print(buffer);
}

void exec_println(char *text){
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "print_%s", text + 8);
    exec_print(buffer);
    printf("\n");
}

void exec_lnprintln(char *text){
    printf("\n");
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "print_%s", text + 10);
    exec_print(buffer);
    printf("\n");
}

void exec_scan(char *text){
    
}

//system fun
void exec_fence(char *text){
    
}

//condizioni logiche USE STATE_STACK
int exec_if(char *text){
    int current_line = global_ip;

}

void exec_else(char *text){
    int current_line = global_ip;

}

void exec_for(char *text){
    int current_line = global_ip;

}

void exec_while(char *text){
    int current_line = global_ip;

}

int test_return_value = 67;
void* exec_funarg(char *text, char *name, char *args, int op_code){
    
    /*QUI VENGONO DISTINTI I VARI TIPI DI CHIAMATA DI UNA FUNZIONE: NON RITORNA ARGOMENTI __NAME
                                                                    RITORNA UN ARGOMENTO CON = &I&VAR& __NAME
                                                                    RITORNA + ARGOMENTI __NAME() --VAR --VAR*/
    //E PER OGNUNA CASISTICA SI DA UN CODICE IN ORDINE 0 = NO RETURN
                                                    // 1 = UN RETURN CON =
                                                    // 2 = RITORNO CON + VAR
    return &test_return_value;                                                      
}

int check_if_same_type(char *arg1, char *arg2){

    if(deb) printf("type checked: %s %s\n",arg1,arg2);
    /* Allow immediate number 'n' to match integer 'i' */
    if ((arg1[0] == 'n' && arg2[0] == 'i') || (arg1[0] == 'i' && arg2[0] == 'n') || (arg1[0] == 'i' && arg2[0] == 'f') || (arg1[0] == 'f' && arg2[0] == 'i'))
        return tru;

    if ((arg1[0] == 'c' && arg2[0] == 'k') || (arg1[0] == 's' && arg2[0] == 'c')|| (arg1[0] == 'c' && arg2[0] == 's') || (arg1[0] == 's' && arg2[0] == 'k') )
        return tru;

    /* Check same base type */
    if(arg1[0] != arg2[0]) return fal;
    if(arg1[0] == arg2[0]) return tru;


    return fal;
}

//matematiche fun
void exec_math(char *text){
    
    char operand1[16] ={0};
    char operand2[16] ={0};
    char type1[15] ={0};
    char type2[15] ={0};

    char func_name[16] ={0};
    char func_args[64] ={0};

    int N = 0;
    char K = 0;

    int *int_src = 0;
    char *char_src = 0;
    
    int *int_dest = 0;
    char *char_dest = 0;

    int error = tru;

    /*CHECK IF ASSEGNATION*/
    if(starts_with(text,"=&")){

        /*!!! CERCA DI TROVARE UN MODO PER FARE CHE SI POSSONO USARE VARIABILI COME INDICI*/
        if( sscanf(text,"=&%14[^&]&%15[^&]&&%14[^&]&%15[^&]&",type1,operand1,type2,operand2) != 4){

            
            if(sscanf(text,"=&%14[^&]&%15[^&]&__%15[^(](%63[^)])",type1,operand1,func_name,func_args) == 4){
                printf("op 2 as a function so called my lord with magestic name: %s and args %s\n",func_name,func_args);
                int_src = (int *)exec_funarg(pt_char_place_holder,func_name,func_args,1); //ONE STAND FOR ASSGNATING ONE VALUE
                error = fal;

                int_dest = resolve(type1, operand1);
                if(!int_dest) return;
                *int_dest = *int_src;
                return;
            }
        }

        if(check_if_same_type(type1,type2)){
            /*SE VAR VAR O SE VAR N*/
            if( (type1[0] == 'i' && type1[1] == '\0') && type2[1] == '\0' ){ //
                
            int_dest = resolve(type1,operand1);
                if(!int_dest) return;

                if(type2[0] == 'n' && type2[1] == '\0'){
                    N = atoi(operand2);
                    int_src = &N;
                    
                }
                else{
                int_src = resolve(type2,operand2);
                if(!int_src) return;
                }
                
                
            }
            /*SE CVAR CVAR O SE CVAR K O SE CVAR CARR[] */
            else if(type1[0] == 'c' && type1[1] == '\0'){

                char_dest = resolve(type1,operand1);
                if(!char_dest) return;

                if(type2[0] == 'k' && type2[1] == '\0'){ // &c&var& &k&c&
                    K = operand2[0];
                    char_src = &K;
                }
                else if(type2[0] == 'c' && type2[1] == '\0'){
                    char_src = resolve(type2,operand2);
                    if(!char_src) return;
                }
                else if(type2[0] == 's' && type2[1] == '[' && type2[3] == ']'){
                    char_src = resolve(type2,operand2);
                    if(!char_src) return;
                }

                
            }
            /*SE ARRAY ARRAY O SE ARRAY VAR O ARRAY N O VAR ARRAY*/
            else if( (type1[0] == 'i' && type1[1] == '[') || (type2[0] == 'i' && type2[1] == '[')){//almeno uno dei 2 è array
                
                char buffer[32];
                
                if(strstr(type1,"i[")){ //&i[]&array&
                    if(deb) printf("type1 array FOUND\n");

                    int_dest = resolve(type1,operand1);
                    if(!int_dest) return;

                    if(strstr(type2,"i[")){ // = &i[]&array& &i[]&array&
                        if(deb) printf("array array type1 main FOUND\n");

                        int_src = resolve(type2,operand2);
                        if(!int_src) return;
                        
                    }
                    else if(strcmp(type2,"i") == 0){ //&i[]&array& &i&array&
                        if(deb) printf("array var FOUND\n");

                        int_src = resolve(type2,operand2);
                        if(!int_src) return;
                        
                    }
                    else if(strcmp(type2,"n") == 0){ //&i[]&array& &n&5&
                        N = atoi(operand2);
                        int_src = &N;
                    }
                    
                }

                else if(strstr(type2,"i[")){ //         &i[]&array&
                    if(deb) printf("type2 is array FOUND\n");

                    int_src = resolve(type2,operand2);
                    if(!int_src) return;

                    if(strstr(type1,"i[")){ // = &i[]&array& &i[]&array&
                        if(deb) printf("array array type2 main FOUND\n");

                        int_dest = resolve(type1,operand1);
                        if(!int_dest) return;
                        
                    }
                    else if(strcmp(type1,"i") == 0){ //&i&array& &[]i&array&
                        if(deb) printf("var array FOUND\n");

                        int_dest = resolve(type1,operand1);
                        if(!int_dest) return;
                    }
                    else if(strcmp(type1,"n") == 0){ //&n&5& &[]i&array& <---errore
                        printf("Error on = you can't copy data into a number, %s \n",text);
                    }
                    
                }
                

            }

            //CASO = &s[]&stringa& &s[]&stringa&    OPPURE  = &s[3]&stringa& &s[2]&stringa&     OPPURE
            else if(type1[0] == 's' && type1[1] == '[' && type1[3] == ']'){ /* = &s[idx]&carr& */
                
                char_dest = resolve(type1,operand1);
                if(type2[0] == 'k'){ // &s[N]&arr& &k&z&
                    K = operand2[0];
                    char_src = &K;
                }
                else if(is_arr_(operand2) == 2 && type2[3] == ']' ){ // &s[N]&arr& &s[N]&arr&
                    char_src = resolve(type2,operand2);
                    if(!char_src) return;
                    
                }
                else if(is_var_(operand2) == 2){ // &s[N]&arr& &c&var&
                    char_src = resolve(type2,operand2);
                    if(!char_src) return;
                } 
                
            }
            else if(type1[0] == 's' && strstr(type1,"[]")){
                printf("ERROR: for string use s= function\n");
                return;
            }

            

        }
        else if(error){
            printf("different type on = in: %s\n",text);
            return;
        }
    }

    if(int_dest  && int_src)  *int_dest  = *int_src;
    if(char_dest && char_src) *char_dest = *char_src;

    return;
}

//-1 SE ERRORE 0 SE FALSO 1 SE VERO
int exec_conf(char *text){
    
    char operand1[16] ={0};
    char operand2[16] ={0};
    char type1[15] ={0};
    char type2[15] ={0};

    char func_name[16] ={0};
    char func_args[64] ={0};

    int N = 0;
    char K = 0;

    int *int_src = 0;
    char *char_src = 0;
    
    int *int_dest = 0;
    char *char_dest = 0;

    int error = tru;

    /*CHECK IF ASSEGNATION*/
    if(starts_with(text,"==")){
        /*!!! CERCA DI TROVARE UN MODO PER FARE CHE SI POSSONO USARE VARIABILI COME INDICI*/
        
        if( sscanf(text,"==&%14[^&]&%15[^&]&&%14[^&]&%15[^&]&",type1,operand1,type2,operand2) != 4){

            
            if(sscanf(text,"==&%14[^&]&%15[^&]&__%15[^(](%63[^)])",type1,operand1,func_name,func_args) == 4 || 
                sscanf(text,"==__%15[^(](%63[^)])&%14[^&]&%15[^&]&",func_name,func_args,type1,operand1) == 4){ //l`inversa
                if(deb) printf("op 2 as a function so called my lord with magestic name: %s and args %s\n",func_name,func_args);
                int_src = (int *)exec_funarg(pt_char_place_holder,func_name,func_args,1); //ONE STAND FOR ASSGNATING ONE VALUE
                error = fal;

                int_dest = resolve(type1, operand1);
                if(!int_dest) return -1;
                if(*int_dest == *int_src) return tru;
                return fal;
            }
        }

        if(check_if_same_type(type1,type2)){
            /*SE VAR VAR O SE VAR N*/
            if( (type1[0] == 'i' && type1[1] == '\0') && type2[1] == '\0' ){ //
                
            int_dest = resolve(type1,operand1);
                if(!int_dest) return-1;

                if(type2[0] == 'n' && type2[1] == '\0'){
                    N = atoi(operand2);
                    int_src = &N;
                }
                else{
                int_src = resolve(type2,operand2);
                if(!int_src) return-1;
                }
                
                
            }
            /*SE CVAR CVAR O SE CVAR K O SE CVAR CARR[] */
            else if(type1[0] == 'c' && type1[1] == '\0'){

                char_dest = resolve(type1,operand1);
                if(!char_dest) return-1;

                if(type2[0] == 'k' && type2[1] == '\0'){ // &c&var& &k&c&
                    K = operand2[0];
                    char_src = &K;
                    
                }
                else if(type2[0] == 'c' && type2[1] == '\0'){
                    char_src = resolve(type2,operand2);
                    if(!char_src) return-1;
                }
                else if(type2[0] == 's' && type2[1] == '[' && type2[3] == ']'){
                    char_src = resolve(type2,operand2);
                    if(!char_src) return-1;
                }

                
            }

            /*SE ARRAY ARRAY O SE ARRAY VAR O ARRAY N O VAR ARRAY*/
            else if( (type1[0] == 'i' && type1[1] == '[') || (type2[0] == 'i' && type2[1] == '[')){//almeno uno dei 2 è array
                
                char buffer[32];
                
                if(strstr(type1,"i[")){ //&i[]&array&
                    if(deb) printf("type1 array FOUND\n");

                    int_dest = resolve(type1,operand1);
                    if(!int_dest) return-1;

                    if(strstr(type2,"i[")){ // = &i[]&array& &i[]&array&
                        if(deb) printf("array array type1 main FOUND\n");

                        int_src = resolve(type2,operand2);
                        if(!int_src) return-1;
                        
                    }
                    else if(strcmp(type2,"i") == 0){ //&i[]&array& &i&array&
                        if(deb) printf("array var FOUND\n");

                        int_src = resolve(type2,operand2);
                        if(!int_src) return-1;
                        
                    }
                    else if(strcmp(type2,"n") == 0){ //&i[]&array& &n&5&
                        N = atoi(operand2);
                        int_src = &N;
                    }
                    
                }

                else if(strstr(type2,"i[")){ //         &i[]&array&
                    if(deb) printf("type2 is array FOUND\n");

                    int_src = resolve(type2,operand2);
                    if(!int_src) return-1;

                    if(strstr(type1,"i[")){ // = &i[]&array& &i[]&array&
                        if(deb) printf("array array type2 main FOUND\n");

                        int_dest = resolve(type1,operand1);
                        if(!int_dest) return-1;
                        
                    }
                    else if(strcmp(type1,"i") == 0){ //&i&array& &[]i&array&
                        if(deb) printf("var array FOUND\n");

                        int_dest = resolve(type1,operand1);
                        if(!int_dest) return-1;
                    }
                    else if(strcmp(type1,"n") == 0){ //&n&5& &[]i&array& <---errore
                        printf("Error on = you can't copy data into a number, %s \n",text);
                        return-1;
                    }
                    
                }

            }

            //CASO = &s[]&stringa& &s[]&stringa&    OPPURE  = &s[3]&stringa& &s[2]&stringa&     OPPURE
            else if(type1[0] == 's' && type1[1] == '[' && type1[3] == ']'){ /* = &s[idx]&carr& */
                
                char_dest = resolve(type1,operand1);
                if(type2[0] == 'k'){ // &s[N]&arr& &k&z&
                    K = operand2[0];
                    char_src = &K;
                }
                else if(is_arr_(operand2) == 2 && type2[3] == ']' ){ // &s[N]&arr& &s[N]&arr&
                    char_src = resolve(type2,operand2);
                    if(!char_src) return-1;
                    
                }
                else if(is_var_(operand2) == 2){ // &s[N]&arr& &c&var&
                    char_src = resolve(type2,operand2);
                    if(!char_src) return-1;
                } 
                
            }
            else if(type1[0] == 's' && strstr(type1,"[]")){
                printf("ERROR: for string use s= function\n");
                return-1;
            }

        }
        else if(error){
            printf("different type on = in: %s\n",text);
            return -1;
        }
    }

    if( int_dest  && int_src  && *int_dest  == *int_src)  return tru;
    if( char_dest && char_src && *char_dest == *char_src) return tru;

    return -1;
}


void parse(int idx_line_temp){
    global_ip = idx_line_temp;
    if(deb) printf("parse chiamato con global_ip: %d e line_idx_program: %d\n",global_ip,line_idx_program);
    while(global_ip<=line_idx_program){
        if(deb) printf("\nlinea analizzata: %d %s\n",global_ip, program[global_ip].instruction);
        if( starts_with (program[global_ip].instruction, "=") ){ exec_math(program[global_ip].instruction); /*funzioni: + - * / e assegnazione */}
        else if( starts_with (program[global_ip].instruction, "==") ){ exec_conf(program[global_ip].instruction); }
        else if( starts_with (program[global_ip].instruction, "#") ){/*funzioni di sistema e interprete tipo import o kill*/ }
        else if( starts_with (program[global_ip].instruction, "return_") ){/*ritorna uno o piu valori*/ }
        else if( starts_with (program[global_ip].instruction, "__") ){/*dichiara funzione __ */ }
        else if( starts_with (program[global_ip].instruction, "_") ){/*chiama funzione oppure semplicemente sposta l'esecuizione ad una riga e imposta un ritorno (pipe e fork) */ }
        else if( starts_with (program[global_ip].instruction, "int_") ){ exec_int(program[global_ip].instruction); /*dichiara var int  */ }
        else if( starts_with (program[global_ip].instruction, "char_") ){ exec_char(program[global_ip].instruction); /*dichiara var char */ }
        else if( starts_with (program[global_ip].instruction, "if_") ){/*inizia if_ */ }
        else if( starts_with (program[global_ip].instruction, "else_") ){/*else */ }
        else if( starts_with (program[global_ip].instruction, "for_") ){/*inizia for */ }
        else if( starts_with (program[global_ip].instruction, "while_") ){/*inizia while */ }
        else if( starts_with (program[global_ip].instruction, "scan_") ){/*legge carattere da schermo */ }
        else if( starts_with (program[global_ip].instruction, "print_") ){ exec_print(program[global_ip].instruction);/*stampa carattere a terminale */ }
        else if( starts_with (program[global_ip].instruction, "lnprint_") ){ exec_lnprint(program[global_ip].instruction);/*va a capo stampa carattere a terminale */ }
        else if( starts_with (program[global_ip].instruction, "println_") ){ exec_println(program[global_ip].instruction);/*stampa carattere a terminale e va a capo */ }
        else if( starts_with (program[global_ip].instruction, "lnprintln_") ){ exec_lnprintln(program[global_ip].instruction);/*va a capo stampa carattere a terminale e va a capo */ }
        global_ip++;
    }
}

void build_state() {
    int i = 0;
    

    while (i <= line_idx_program) {

        if (strchr(program[i].instruction, '{') != NULL) {

            int j = i;
            int depth = 0;

            while (j <= line_idx_program) {

                if (strchr(program[j].instruction, '{') != NULL)
                    depth++;

                if (strchr(program[j].instruction, '}') != NULL) {
                    depth--;
                    if (depth == 0)
                        break;
                }

                j++;
            }

            state_stack[return_state].codice = return_state;
            state_stack[return_state].posizione_ritorno = i;
            if( starts_with(program[i].instruction,"if_") ) strcpy(state_stack[return_state].nome_function, "if_");
            else if( starts_with(program[i].instruction,"for_") ) strcpy(state_stack[return_state].nome_function, "for_");
            else if( starts_with(program[i].instruction,"while_") ) strcpy(state_stack[return_state].nome_function, "while_");
            else if( starts_with(program[i].instruction,"else_") ) strcpy(state_stack[return_state].nome_function, "else_");
            else if( starts_with(program[i].instruction,"__start") ) strcpy(state_stack[return_state].nome_function, "__start");
            else if( starts_with(program[i].instruction,"__") ) strcpy(state_stack[return_state].nome_function, "__");
            state_stack[return_state].posizione_skip = j;

            return_state++;
        }

        i++;
    }
}



void run_test(){
    declare_variable("var0",'i');
    declare_variable("var1",'c');
    printf("LOG: decl_var ok \n");

    set_to_variable("var0",'i', 3, place_holder);
    set_to_variable("var1",'c', place_holder, 'F');
    printf("LOG: set_to_var ok \n");

    declare_array("arr0", 's', 7); // Inizializza arr0 char con 7 elementi
    declare_array("arr1", 'i', 5); // Inizializza arr1 int con 5 elementi
    declare_array("arr2", 's', 4); // Inizializza arr2 char con 4 elementi
    printf("LOG: decl_array ok \n");

    set_to_array("arr0",'s', 0, place_holder, 'f'); // arr0[0] = f
    set_to_array("arr1", 'i', 1, 77, place_holder); // arr1[1] = 77
    set_to_array("arr2", 's', 2, place_holder, 'c'); // Inizializza arr2[2] a 'c'
    set_to_array("arr2", 's', 3, place_holder, 'i'); // Inizializza arr2[3] a 'i'
    r[2] = 3; // r2 = 3
    printf("LOG: set_to_array_+_reg ok \n");

    // Test get_index
    int *ptr0 = (int *)get_index("&i&var0&"); // test int var
    char *ptr1 = (char *)get_index("&c&var1&"); // test char var
    printf("LOG: get_index_for_var0_&_var1 ok \n");

    char *ptr2 = (char *)get_index("&s[0]&arr0&"); // test char array con N come indice
    printf("LOG: get_index_arr with N as index ok \n");

    int *ptr3 = (int *)get_index("&i[1]&arr1&"); // test int array con N come indice
    printf("LOG: get_index_arr with N as index ok \n");
    
    char *ptr4 = (char *)get_index("&s[2]&arr2&"); // test char array con N come indice
    printf("LOG: get_index_arr with N as index ok \n");
    
    char *ptr5 = (char *)get_index("&s[r2]&arr2&"); // test char array con r come indice
    printf("LOG: get_index_arr with reg as index ok \n");
    
    char *ptr6 = (char *)get_index("&s[&i&var0&]&arr2&"); // test char array con var come indice
    printf("LOG: get_index_arr with var0 as index ok \n");

    int *ptr7 = (int *)get_index("&i&r2&"); // test reg
    printf("LOG: get_index_reg ok \n");

    int x = exec_conf("==&i&var0&&n&3&"); // test conf VAR N
    printf("LOG: conf VAR N ok \n");

    if (ptr0) printf("Valore di var0: %d\n", *ptr0); // Dovrebbe stampare 3
    if (ptr1) printf("Valore di var1: %c\n", *ptr1); // Dovrebbe stampare F
    if (ptr2) printf("Valore di arr0[0]: %c\n", *ptr2); // Dovrebbe stampare f
    if (ptr3) printf("Valore di arr1[1]: %d\n", *ptr3); // Dovrebbe stampare 77
    if (ptr4) printf("Valore di arr2[2]: %c\n", *ptr4); // Dovrebbe stampare c
    if (ptr5) printf("Valore di arr2[r2]: %c\n", *ptr5); // Dovrebbe stampare i
    if (ptr6) printf("Valore di arr2[var0]: %c\n", *ptr6); // Dovrebbe stampare 2
    if (ptr7) printf("Valore di r2: %d\n", *ptr7); // Dovrebbe stampare 2
    printf("confronto booleano tra var0 e 3: %d\n",x); //dovrebbe stampare 1

    printf("\nrisultato atteso: 3 F f 77 c i i 3 1\n");

    

}


int main(int argc, char *argv[]) {
    memset(&vm, 0, sizeof(VM));

    if(argc >= 3) {
        

        const char *dot = strrchr(argv[2], '.');
        

        if (!dot || strcmp(dot, ".Zinter") != 0) {
            printf("errore: formato file non supportato\n");
            return 0;
        }

        if(strcmp("-df",argv[1]) == 0) deb = fal;
        else if(strcmp("-dt",argv[1]) == 0) deb = tru;

        char *code = read_code_from_file(argv[2]);

        format_code(code);

        if(deb) printf("USCITO DA FORMAT_CODE \n\n\n");

        build_state();

        if(deb) {

        int i=0;

        while(i<=line_idx_program){
            printf("%d, %s",program[i].line_number,program[i].instruction);
            printf("\n");
            i++;
        }

        printf("Stato costruito:\n");

        i=0;
        while(i<return_state){
            printf("riga di inizio condizione n%d: %d nome: %s, posizione skip(chiusura) %d",state_stack[i].codice,state_stack[i].posizione_ritorno,state_stack[i].nome_function,state_stack[i].posizione_skip);
            printf("\n");
            i++;
            }
        }

        
        //=====inizia parse e esecuzione======
        int st_ip = 0;
        while( strcmp(state_stack[st_ip].nome_function,"__start") != 0 ){
            st_ip++;
        }


        parse(state_stack[st_ip].posizione_ritorno);


        free(code);
        return 0;
    }

    if(argc==1){
        printf("need to add arguments \n");
        printf("in order: ./Zinterpreter -debug file_name.Zinter \n");
        printf("                 ^          ^        \n");
        printf("                 |          |        \n");
        printf("           or your compiler |        \n");
        printf("              version       |        \n");
        printf("                           -df ==debug false  \n");
        printf("                           -dt ==debug true  \n");
        printf("                           -int ==interface   \n");
        
        printf("\n");
        printf("           A test will now be executed:\n");
    }


    run_test();
    // =====================FINE SET TEST===================
    
    return 0;
}