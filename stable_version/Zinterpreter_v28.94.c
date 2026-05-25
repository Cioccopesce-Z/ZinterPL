#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
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
#define error_int -99
#define error_char §§


int line_idx_program = 0;
int place_holder = 0;
int deb = tru;

static char return_type = 0; 
static void *return_value = NULL;
static int   return_hit   = 0;

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

// ==== PREDEFINED FUNCTION =========
void parse(int idx_line_temp, int  eventual_end_line);
int starts_with(const char *str, const char *prefix);
void* exec_funarg(char *name_plus_args, int is_return);

/* 1 int 2 char 3 float 0 not*/
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
            return 3; // è una variabile float
        }
    }
    return 0; // non è una variabile
}
/* 1 int 2 char 3 float 0 not*/
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
            return 3; // è un arr float
        }
    }
    return 0; // non è un array
}
/* 1 int 2 char 3 float 0 not*/
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
            return 3; // è una matrice float
        }
    }
    return 0; // non è una matrice
}

/* 1 yes 0 not*/
int is_function_(const char *name_or_declaration){

    int i=0;
    char name[16] = {0};
    char args[64] = {0};
    if(deb) printf("DEBUG: is function called with: %s \n",name_or_declaration);

    if(strstr(name_or_declaration,"od_")){ // od_ name(args){ } being declared 
        sscanf(name_or_declaration,"od_%15[^(](%64[^)])",name,args); //name == "name"  args == "args" 
    } 
         

    else if(starts_with(name_or_declaration,"__")){ //__name() being called
        if(deb) printf("found start with __ in is_function \n");
        sscanf(name_or_declaration,"__%15[^(](%64[^)])",name,args); //name == "name"  args == "args"
    } 

    else{
        strcpy(name,name_or_declaration);
    }

    if(deb) printf("name = %s, args = %s \n",name,args);  

    char buffer[24];
    sprintf(buffer,"od_%s",name);
        
    if(deb) printf("DEBUG: name for function search: %s \n",buffer);
    if(deb) printf("DEBUG: return_state: %d \n",return_state);
    
    i = 0;
    while(strcmp( state_stack[i].nome_function,buffer) != 0){
        if(deb) printf("DEBUG: searching for function search in loop: %s \n",state_stack[i].nome_function);
        if(i >= return_state) return fal;
        i++;
    }
    if(deb) printf("function %s found %s\n\n",buffer,state_stack[i].nome_function);
    return tru; // non è una function
}

/* variable array matrix function n c , n i c/s/k l*/
void is_what(char name_result[]) {

    int base_type;
    if(deb) printf("is what generally called with: %s \n",name_result);

    if ((base_type = is_var_(name_result)) != 0) {
        if (base_type == 1) { strcpy(name_result, "variable.i"); return; }
        else if (base_type == 2) { strcpy(name_result, "variable.c"); return; }
        else if (base_type == 3) { strcpy(name_result, "variable.l"); return; }
    }
    else if ((base_type = is_arr_(name_result)) != 0) {
        if (base_type == 1) { strcpy(name_result, "array.i"); return; }
        else if (base_type == 2) { strcpy(name_result, "array.s"); return; }
        else if (base_type == 3) { strcpy(name_result, "array.l"); return; }
    }
    else if ((base_type = is_matrix_(name_result)) != 0) {
        if (base_type == 1) { strcpy(name_result, "matrix.i"); return; }
        else if (base_type == 2) { strcpy(name_result, "matrix.s"); return; }
        else if (base_type == 3) { strcpy(name_result, "matrix.l"); return; }
    }
    else if ((base_type = is_function_(name_result)) != 0) {
        if(deb) printf("is what for function called with: %s after is_function calling\n",name_result);
        strcpy(name_result, "function.v"); return; 
    }
    else if( isdigit(name_result[0]) ){
        if(deb) printf("is what for number called with: %s after isdigit calling\n",name_result);
        strcpy(name_result, "n.n"); return;
    }
    else if( isalpha(name_result[0]) ){
        if(deb) printf("is what for character called with: %s after isalpha calling\n",name_result);
        strcpy(name_result, "c.k"); return;
    }
    strcpy(name_result, "-1.-1");
}

//function to get the index of variable, array and matrix 
void* get_index(char data_sruct_name[]) {

    int borrow;
    char var_name[max_name_lettere];
    char type;
    char var_type;

    static int temp_int;
    static char temp_char;
    
    
    //NUMERO
    if(sscanf(data_sruct_name, "&%c&%d&", &type, &temp_int) == 2 && type == 'n'){
        return &temp_int; //serve ma non va modificato
    }
    //SE CARATTERE
    else if(sscanf(data_sruct_name, "&%c&%c&", &type, &temp_char) == 2 && type == 'k'){
        return &temp_char; //serve ma non va modificato
    }

    // MATRICE (es: &i[idx1][idx2]&name&)
    else if (strstr(data_sruct_name, "][") != NULL) {

        char mat_name[max_name_lettere];
        char idx1_str[50];
        char idx2_str[50];
        char mat_type;

        int parsed = sscanf(data_sruct_name,
            "&%c[%49[^]]][%49[^]]]&%15[^&]&",
            &mat_type, idx1_str, idx2_str, mat_name);

        if(parsed != 4) {
            printf("get_index: errore parsing matrice: %s\n", data_sruct_name);
            return NULL;
        }

        if(deb) printf("get_index matrice %s [%s][%s] tipo %c\n",
                    mat_name, idx1_str, idx2_str, mat_type);

        // Risolvi idx1
        int row = 0, col = 0;
        if(sscanf(idx1_str, "%d", &row) != 1) {
            int *pr = (int *)get_index(idx1_str);
            if(!pr) { printf("get_index: indice riga non risolto: %s\n", idx1_str); return NULL; }
            row = *pr;
        }

        // Risolvi idx2
        if(sscanf(idx2_str, "%d", &col) != 1) {
            int *pc = (int *)get_index(idx2_str);
            if(!pc) { printf("get_index: indice colonna non risolto: %s\n", idx2_str); return NULL; }
            col = *pc;
        }

        if(mat_type == 'i') {
            for(int i = 0; i < matrix_count; i++) {
                if(strcmp(matrix[i].name, mat_name) == 0) {
                    if(row >= 0 && row < matrix[i].size_first &&
                    col >= 0 && col < matrix[i].size_sec)
                        return &matrix[i].matrix_int[row][col];
                    printf("OUT OF BOUNDS matrice int '%s' [%d][%d]\n", mat_name, row, col);
                    return NULL;
                }
            }
        }
        else if(mat_type == 'l') {
            for(int i = 0; i < fl_matrix_count; i++) {
                if(strcmp(fl_matrix[i].name, mat_name) == 0) {
                    if(row >= 0 && row < fl_matrix[i].size_first &&
                    col >= 0 && col < fl_matrix[i].size_sec)
                        return &fl_matrix[i].matrix_fl_int[row][col];
                    printf("OUT OF BOUNDS matrice float '%s' [%d][%d]\n", mat_name, row, col);
                    return NULL;
                }
            }
        }
        else if(mat_type == 's') {
            for(int i = 0; i < char_matrix_count; i++) {
                if(strcmp(char_matrix[i].name, mat_name) == 0) {
                    if(row >= 0 && row < char_matrix[i].size_first &&
                    col >= 0 && col < char_matrix[i].size_sec)
                        return &char_matrix[i].matrix_str_int[row][col];
                    printf("OUT OF BOUNDS matrice char '%s' [%d][%d]\n", mat_name, row, col);
                    return NULL;
                }
            }
        }

        printf("get_index: matrice '%s' non trovata\n", mat_name);
        return NULL;
    }


    // ARRAY (esempio &i[something]&array& )
    else if (strchr(data_sruct_name, '[') != NULL) {

        char arr_name[max_name_lettere];
        char index_str[50];

        int check_str_print = sscanf(data_sruct_name, "&%c[%15[^]]]&%15[^&]&", &type, index_str, arr_name);

        if(deb) printf("get_index riconosciuto array %s, indice: %s \n", arr_name, index_str);

        int index = 0;

        // Se l'indice è numero diretto
        if (sscanf(index_str, "%d", &index) == 1 && check_str_print == 3) {

            

            if(type =='s'){
                for (int i = 0; i < char_array_count; i++) {
                    if (strcmp(char_array[i].name, arr_name) == 0 && index >= 0 && index < char_array[i].size) {
                        return &char_array[i].array_int[index]; 
                    }
                }
                printf("OUT OF BOUNDS for char_array: %s, with delcaration: %s \n",arr_name,data_sruct_name);
                    return NULL;
            }

            else if(type == 'i'){
                for (int i = 0; i < array_count; i++) {
                    if (strcmp(array[i].name, arr_name) == 0 && index >= 0 && index < array[i].size) {
                        return &array[i].array_int[index];
                    }
                }
                printf("OUT OF BOUNDS for int_array: %s, with delcaration: %s \n",arr_name,data_sruct_name);
                        return NULL;
            }

            else if(type == 'l'){
                for (int i = 0; i < fl_array_count; i++) {
                    if (strcmp(fl_array[i].name, arr_name) == 0 && index >= 0 && index < fl_array[i].size) {
                        return &fl_array[i].array_int[index];
                    }
                }
                printf("OUT OF BOUNDS for fl_array: %s, with delcaration: %s\n",arr_name,data_sruct_name);
                        return NULL;
            }

            
                

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
                printf("OUT OF BOUNDS for char_array: %s, with delcaration: %s",arr_name,data_sruct_name);
                    return NULL;
            }

            else if(type == 'i'){
                for (int i = 0; i < array_count; i++) {
                    if (strcmp(array[i].name, arr_name) == 0 && *index_value >= 0 && *index_value < array[i].size) {
                        return &array[i].array_int[*index_value];
                    }
                }
                printf("OUT OF BOUNDS for int_array: %s, with delcaration: %s",arr_name,data_sruct_name);
                return NULL;
            }

            else if(type == 'l'){
                for (int i = 0; i < fl_array_count; i++) {
                    if (strcmp(fl_array[i].name, arr_name) == 0 && *index_value >= 0 && *index_value < fl_array[i].size) {
                        return &fl_array[i].array_int[*index_value];
                    }
                }
                printf("OUT OF BOUNDS for fl_array: %s, with delcaration: %s",arr_name,data_sruct_name);
                return NULL;
            }
                
            
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

        if(type =='c'){
            for (int i = 0; i < char_variable_count; i++) {
                if (strcmp(char_variable[i].name, var_name) == 0) {
                    return &char_variable[i].value; 
                }
            }
        }

        if(type =='i'){
            for (int i = 0; i < variable_count; i++) {
                if (strcmp(variable[i].name, var_name) == 0) {
                    
                    return &variable[i].value;
                }
            }
        }

        if(type =='l'){
            for (int i = 0; i < fl_variable_count; i++) {
                if (strcmp(fl_variable[i].name, var_name) == 0) {
                    
                    return &fl_variable[i].value;
                }
            }
        }
    }

    else if(strstr(data_sruct_name,"__") || starts_with(data_sruct_name,"od_")){
        return exec_funarg(data_sruct_name,fal);
    }

    return NULL; // numero immediato o errore
}

void* resolve(char type, char *name){
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "&%c&%s&", type, name);
    
    if(type == 'i' || type == 'n')
        return (int *)get_index(buffer);
    if(type == 's' || type == 'k' || type == 'c')
        return (char *)get_index(buffer);
    if(type == 'l')
        return (float *)get_index(buffer); 
    return NULL;
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
        strcpy(fl_matrix[fl_matrix_count].name, name);
        fl_matrix[fl_matrix_count].size_first = size_one;
        fl_matrix[fl_matrix_count].size_sec = size_two;

        fl_matrix_count++;

        if(fl_matrix_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di matrici fl raggiunto.\n");
        }
    }

    //matrix char
    if(type =='s'){
        strcpy(char_matrix[char_matrix_count].name, name);
        char_matrix[char_matrix_count].size_first = size_one;
        char_matrix[char_matrix_count].size_sec = size_two;

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
            printf("Attenzione: la dimensione dell'int_array '%s' è stata limitata a %d.\n", name, max_number_of_cell);
        }

        array[array_count].size = size;

        for (int i = 0; i < size; i++) {
            array[array_count].array_int[i] = 0; // Inizializza a 0
        }

        array_count++;
    }

    if(type =='l'){

        strcpy(fl_array[fl_array_count].name, name);

        if(size > max_number_of_cell) {
            size = max_number_of_cell; // Limita la dimensione dell'array
            printf("Attenzione: la dimensione dell'fl_array '%s' è stata limitata a %d.\n", name, max_number_of_cell);
        }

        fl_array[fl_array_count].size = size;

        for (int i = 0; i < size; i++) {
            fl_array[fl_array_count].array_int[i] = 0; // Inizializza a 0
        }

        fl_array_count++;
    }

    if(type =='s'){

        strcpy(char_array[char_array_count].name, name);

        if(size > max_number_of_cell) {
            size = max_number_of_cell; // Limita la dimensione dell'array
            printf("Attenzione: la dimensione dell'char_array '%s' è stata limitata a %d.\n", name, max_number_of_cell);
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
void set_to_array(char name[], char type, int index, float value, char cvalue) {

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

    if(type =='l') {
        for (int i = 0; i < fl_array_count; i++) {

            if (strcmp(fl_array[i].name, name) == 0) {

                if (index >= 0 && index < fl_array[i].size) {
                    fl_array[i].array_int[index] = value;
            }
                else {
                printf("Errore: indice fuori dai limiti per l'fl_array '%s'.\n", name);
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
                printf("Errore: indice fuori dai limiti per l'char_array '%s'.\n", name);
                }
                return;
            }
        }
    }
    printf("Errore: array '%s' non trovato.\n", name);
}

void set_to_variable(char name[], char type, float value, char cvalue) {

    char buffer[64];
    
    sprintf(buffer,"&%c&%s&",type,name);    

    if(type == 'i'){
        int *temp = (int*)get_index(buffer);

        if (temp == NULL) {
            printf("Errore: int_variabile '%s' non trovata.\n", name);
            return;
        }
        *temp = value;
    }

    if(type == 'l'){
        float *temp = (float*)get_index(buffer);

        if (temp == NULL) {
            printf("Errore: fl_variabile '%s' non trovata.\n", name);
            return;
        }
        *temp = value;
    }

    if(type == 'c'){
        char *temp = (char*)get_index(buffer);

        if (temp == NULL) {
            printf("Errore: char_variabile '%s' non trovata.\n", name);
            return;
        }
        *temp = cvalue;
    }
}

void set_to_matrix(char name[], char type, int row, int col, float value, char cvalue) {

    if(type == 'i') {
        for(int i = 0; i < matrix_count; i++) {
            if(strcmp(matrix[i].name, name) == 0) {
                if(row >= 0 && row < matrix[i].size_first &&
                   col >= 0 && col < matrix[i].size_sec) {
                    matrix[i].matrix_int[row][col] = (int)value;
                } else {
                    printf("Errore: indici [%d][%d] fuori dai limiti per la matrice int '%s'.\n", row, col, name);
                }
                return;
            }
        }
    }

    else if(type == 'l') {
        for(int i = 0; i < fl_matrix_count; i++) {
            if(strcmp(fl_matrix[i].name, name) == 0) {
                if(row >= 0 && row < fl_matrix[i].size_first &&
                   col >= 0 && col < fl_matrix[i].size_sec) {
                    fl_matrix[i].matrix_fl_int[row][col] = value;
                } else {
                    printf("Errore: indici [%d][%d] fuori dai limiti per la matrice float '%s'.\n", row, col, name);
                }
                return;
            }
        }
    }

    else if(type == 's') {
        for(int i = 0; i < char_matrix_count; i++) {
            if(strcmp(char_matrix[i].name, name) == 0) {
                if(row >= 0 && row < char_matrix[i].size_first &&
                   col >= 0 && col < char_matrix[i].size_sec) {
                    char_matrix[i].matrix_str_int[row][col] = cvalue;
                } else {
                    printf("Errore: indici [%d][%d] fuori dai limiti per la matrice char '%s'.\n", row, col, name);
                }
                return;
            }
        }
    }

    printf("Errore: matrice '%s' non trovata.\n", name);
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
    if(strcmp(type,"i") == 0) declare_variable(name,'i');

    char name_of_type[16];
    char type_of_type;

    if(strchr(type,'[')){

        /* MATRICE: i[N][M] */
        if(strstr(type,"][")){
            char size1_str[16], size2_str[16];
            if(sscanf(type,"%c[%15[^]]][%15[^]]]",&type_of_type,size1_str,size2_str) == 3 && type_of_type == 'i'){
                int s1, s2;

                if(sscanf(size1_str,"%d",&s1) != 1){
                    int *p = get_index(size1_str);
                    if(!p){ printf("ERROR: cannot resolve matrix size1 in: %s\n",text); return; }
                    s1 = *p;
                }
                if(sscanf(size2_str,"%d",&s2) != 1){
                    int *p = get_index(size2_str);
                    if(!p){ printf("ERROR: cannot resolve matrix size2 in: %s\n",text); return; }
                    s2 = *p;
                }
                declare_matrix(name,'i',s1,s2);
            } else {
                printf("ERROR: errore nella dichiarazione MATRIX INT nella riga: %s\n",text);
            }
        }
        /* ARRAY: i[N] */
        else if(sscanf(type,"%c[%15[^]]]",&type_of_type,name_of_type) == 2 && type_of_type == 'i'){
            int size;
            if(sscanf(name_of_type,"%d",&size) == 1){
                declare_array(name,'i',size);
            } else {
                int *ptr0 = get_index(name_of_type);
                if(!ptr0){ printf("ERROR: segmentation error for ptr0 in exec_int on line: %s\n",text); return; }
                declare_array(name,'i',*ptr0);
            }
        } else {
            printf("ERROR: errore nella dichiarazione INT nella riga: %s\n",text);
        }
    }
}

void exec_char(char *text){
    char type[16];
    char name[16];

    sscanf(text,"char_&%15[^&]&%15[^&]&",type,name);
    if(strcmp(type,"c") == 0) declare_variable(name,'c');

    char name_of_type[16];
    char type_of_type;

    if(strchr(type,'[')){

        /* MATRICE: s[N][M] */
        if(strstr(type,"][")){
            char size1_str[16], size2_str[16];
            if(sscanf(type,"%c[%15[^]]][%15[^]]]",&type_of_type,size1_str,size2_str) == 3 && type_of_type == 's'){
                int s1, s2;

                if(sscanf(size1_str,"%d",&s1) != 1){
                    int *p = get_index(size1_str);
                    if(!p){ printf("ERROR: cannot resolve matrix size1 in: %s\n",text); return; }
                    s1 = *p;
                }
                if(sscanf(size2_str,"%d",&s2) != 1){
                    int *p = get_index(size2_str);
                    if(!p){ printf("ERROR: cannot resolve matrix size2 in: %s\n",text); return; }
                    s2 = *p;
                }
                declare_matrix(name,'s',s1,s2);
            } else {
                printf("ERROR: errore nella dichiarazione MATRIX CHAR nella riga: %s\n",text);
            }
        }
        /* ARRAY: s[N] */
        else if(sscanf(type,"%c[%15[^]]]",&type_of_type,name_of_type) == 2 && type_of_type == 's'){
            int size;
            if(sscanf(name_of_type,"%d",&size) == 1){
                declare_array(name,'s',size);
            } else {
                int *ptr0 = get_index(name_of_type);
                if(!ptr0){ printf("ERROR: segmentation error for ptr0 in exec_char on line: %s\n",text); return; }
                declare_array(name,'s',*ptr0);
            }
        } else {
            printf("ERROR: errore nella dichiarazione CHAR nella riga: %s\n",text);
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

    /* SINTASSI SENZA TOKEN: print_ [idx]name oppure print_ name */
    if(n < 1){

        char raw[128] = {0};
        if( sscanf(text,"print_%127s", raw) != 1 ){
            if(raw[0] != '\0') printf("ERROR: parsing failed for print_%s \n",text);

            return;
        }

        if(deb) printf("DEBUG PRINT raw=%s\n", raw);

        // Separa indici dal nome base: "[2]myarr" → indices="[2]" base_name="myarr"
        char base_name[128] = {0};
        char indices[64] = {0};
        char *last_bracket = strrchr(raw, ']');
        if(last_bracket){
            strncpy(base_name, last_bracket + 1, sizeof(base_name) - 1);
            strncpy(indices, raw, (int)(last_bracket + 1 - raw));
        } else {
            strncpy(base_name, raw, sizeof(base_name) - 1);
        }

        char what[128];
        strcpy(what, base_name);
        is_what(what);

        if(deb) printf("DEBUG PRINT base_name=%s what=%s\n", base_name, what);

        if(strcmp(what,"-1.-1") == 0){
            printf("Errore: '%s' non e' nota\n", base_name);
            return;
        }

        char rb[256] = {0};

        if(strcmp(what,"variable.i") == 0){
            snprintf(rb,sizeof(rb),"&i&%s&", base_name);
            int *p = (int *)get_index(rb); if(p) printf("%d",*p);
        }
        else if(strcmp(what,"variable.c") == 0){
            snprintf(rb,sizeof(rb),"&c&%s&", base_name);
            char *p = (char *)get_index(rb); if(p) printf("%c",*p);
        }
        else if(strcmp(what,"variable.l") == 0){
            snprintf(rb,sizeof(rb),"&l&%s&", base_name);
            float *p = (float *)get_index(rb); if(p) printf("%f",*p);
        }
        else if(strcmp(what,"array.i") == 0){
            if(!indices[0]){ printf("WARNING: usa print_ [idx]%s\n", base_name); return; }
            snprintf(rb,sizeof(rb),"&i%s&%s&", indices, base_name); // &i[idx]&name&
            int *p = (int *)get_index(rb); if(p) printf("%d",*p);
        }
        else if(strcmp(what,"array.s") == 0){
            if(!indices[0]){ printf("WARNING: usa print_ [idx]%s\n", base_name); return; }
            snprintf(rb,sizeof(rb),"&s%s&%s&", indices, base_name); // &s[idx]&name&
            char *p = (char *)get_index(rb); if(p) printf("%c",*p);
        }
        else if(strcmp(what,"array.l") == 0){
            if(!indices[0]){ printf("WARNING: usa print_ [idx]%s\n", base_name); return; }
            snprintf(rb,sizeof(rb),"&l%s&%s&", indices, base_name); // &l[idx]&name&
            float *p = (float *)get_index(rb); if(p) printf("%f",*p);
        }
        else if(strcmp(what,"matrix.i") == 0){
            if(!indices[0]){ printf("WARNING: usa print_ [r][c]%s\n", base_name); return; }
            snprintf(rb,sizeof(rb),"&i%s&%s&", indices, base_name); // &i[r][c]&name&
            int *p = (int *)get_index(rb); if(p) printf("%d",*p);
        }
        else if(strcmp(what,"matrix.s") == 0){
            if(!indices[0]){ printf("WARNING: usa print_ [r][c]%s\n", base_name); return; }
            snprintf(rb,sizeof(rb),"&s%s&%s&", indices, base_name); // &s[r][c]&name&
            char *p = (char *)get_index(rb); if(p) printf("%c",*p);
        }
        else if(strcmp(what,"matrix.l") == 0){
            if(!indices[0]){ printf("WARNING: usa print_ [r][c]%s\n", base_name); return; }
            snprintf(rb,sizeof(rb),"&l%s&%s&", indices, base_name); // &l[r][c]&name&
            float *p = (float *)get_index(rb); if(p) printf("%f",*p);
        }
        else if(strcmp(what,"function.v") == 0){
            printf("WARNING: per chiamare una funzione usa la sintassi _name(args)\n");
        }

        return;
    }

    if(n == 1) name[0] = '\0';

    char rebuilt[256];
    snprintf(rebuilt, sizeof(rebuilt), "&%s&%s&",type,name);

    /*TESTO VUOTO*/
    if(strcmp(type,"s") == 0 && name[0] == '\0'){ //&s&& --------> " "
        printf(" ");
        return;
    }

    /*TESTO PURO - anche con virgolette: &s&"ciao"& → ciao */
    else if(strcmp(type,"s") == 0){
        if(name[0] == '"'){
            // rimuove virgolette iniziale e finale
            char stripped[128] = {0};
            strncpy(stripped, name + 1, sizeof(stripped) - 1);
            int len = strlen(stripped);
            if(len > 0 && stripped[len-1] == '"') stripped[len-1] = '\0';
            printf("%s", stripped);
        } else {
            printf("%s", name); //&s&name& -----> name
        }
    }

    /*NUMERO PURO*/
    else if(strcmp(type,"n") == 0) printf("%s",name); //&n&99& -----> 99

    /*VARIABILE*/
    else if(strcmp(type,"i") == 0){
        int *ptr0 = (int *)get_index(rebuilt); //&i&name& ------> 3
        if(ptr0 == NULL) return;
        printf("%d",*ptr0);
    }
    else if(strcmp(type,"l") == 0){
        float *ptr0 = (float *)get_index(rebuilt); //&l&name& ------> 3.14
        if(ptr0 == NULL) return;
        printf("%f",*ptr0);
    }
    else if(strcmp(type,"c") == 0){
        char *ptr0 = (char *)get_index(rebuilt); //&c&name& ------> k
        if(ptr0 == NULL) return;
        printf("%c",*ptr0);
    }

    /*MATRIX*/
    if(strstr(type,"][") && name[0] != '\0'){

        if( starts_with(type,"i[") ){
            int *ptr0 = (int *)get_index(rebuilt); //&i[r][c]&name& ------> 3
            if(ptr0 == NULL) return;
            printf("%d",*ptr0);
        }
        else if( starts_with(type,"l[") ){
            float *ptr0 = (float *)get_index(rebuilt); //&l[r][c]&name& ------> 3.14
            if(ptr0 == NULL) return;
            printf("%f",*ptr0);
        }

        char bin[max_lenght_of_string];
        n = sscanf(type,"s[%15[^]]]",bin);
        if( n == 1 ){
            char *ptr0 = (char *)get_index(rebuilt); //&s[r][c]&name& ------> k
            if(ptr0 == NULL || *ptr0 == '\0') return;
            printf("%c",*ptr0);
        }
        else if(strcmp(type,"s[][]") == 0){
            printf("WARNING: U cant expect me to write a function for printing a whole matrix \n");
        }
    }

    /*ARRAY*/
    else if(!strstr(type,"][")){

        if( starts_with(type,"i[") ){
            int *ptr0 = (int *)get_index(rebuilt); //&i[idx]&name& ------> 3
            if(ptr0 == NULL) return;
            printf("%d",*ptr0);
        }
        else if( starts_with(type,"l[") ){
            float *ptr0 = (float *)get_index(rebuilt); //&l[idx]&name& ------> 3.14
            if(ptr0 == NULL) return;
            printf("%f",*ptr0);
        }

        char bin[max_lenght_of_string];
        n = sscanf(type,"s[%15[^]]]",bin);
        if( n == 1 ){
            char *ptr0 = (char *)get_index(rebuilt); //&s[idx]&name& ------> k
            if(ptr0 == NULL || *ptr0 == '\0') return;
            printf("%c",*ptr0);
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
    if(deb) printf("entered in println\n");
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
int system_setup(){

    if(deb) printf("system_setup called\n");


    int st_ip = 0;

    while( strcmp(state_stack[st_ip].nome_function,"#") != 0 ){
        if(deb) printf("DEBUG: searching # in state_stack[st_ip].nome_function: %s st_ip: %d with return_state: %d\n",
                                    state_stack[st_ip].nome_function,st_ip,return_state);
        
        if(st_ip <= return_state){ //non è presente un sys function
            
            if(deb) printf("DEBUG: startfunction found as: %d ->st_ip\n",st_ip);
            printf("WARNING: no #{} present\n");

                while( strcmp(state_stack[st_ip].nome_function,"__start") != 0){
                    st_ip++;
                }

                int end_ip = state_stack[st_ip].posizione_skip;
                st_ip = state_stack[st_ip].posizione_ritorno;

                return st_ip;
            }

        st_ip++;
    }

    


    int end_ip = state_stack[st_ip].posizione_skip;
    st_ip = state_stack[st_ip].posizione_ritorno;
    int current_ip = st_ip;
    st_ip++;

    if(deb) printf("#{ } start line found as: %d\n",st_ip);
    if(deb) printf("#{ } finish line found as: %d\n",end_ip);


    while(st_ip < end_ip){

        restart: ;
        char function[16] = {0};
        char arguments[50] = {0};

        int n = sscanf(program[st_ip].instruction,"%15[^_]_%49s",function,arguments);

        if( starts_with(function,"//") ){ st_ip++; goto restart; }  //caso commento

        if(deb) printf("analyzed line: %s\n",program[st_ip].instruction);
        if(deb) printf("function: %s args: %s\n",function,arguments);
        
        if(n < 2){
            if(n == 1 && strcmp(function,"exec") == 0){
                if(deb) printf("found exec_ \n");

                st_ip = 0;
                while( strcmp(state_stack[st_ip].nome_function,"__start") != 0 ){
                    st_ip++;
                }
                st_ip = state_stack[st_ip].posizione_ritorno;

                if(deb) printf("start ip found as: %d\n",st_ip);

                if(current_ip > st_ip){
                    printf("WARNING: used # (sys) function after function start\n");
                    
                }

                else{
                    global_ip = st_ip;
                }
                return st_ip;

            } 
            printf("ERROR: parsing error in system function #: %s \n",function); 
            return -1;
        }

        else if(strcmp(function,"debug") == 0){

            if(deb) printf("debug function in #{ } found \n");

            if(strcmp(arguments,"-df") == 0) deb = fal;
            else if(strcmp(arguments,"-dt") == 0) deb = tru;
            else printf("WARNING: wrong argument for sys function: debug in #{ }: %s\nthe default setting will be preserved \n",arguments);
        }

        else if(strcmp(function,"import") == 0){
            printf("WARNING: function import are no longer supported in this build, if you want to add a .Zlib file call it with the interpreter -filename.Zlib\nwont be added any library\n");
        }
        st_ip ++;
    }
    
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

//end condizioni logiche


int check_if_same_type(char arg1[], char arg2[]){

    if(deb) printf("type checked: %s %s\n",arg1,arg2);
    /* Allow immediate number 'n' to match integer 'i' */
    if (    (arg1[0] == 'n' && arg2[0] == 'i') || (arg1[0] == 'i' && arg2[0] == 'n') 
         || (arg1[0] == 'i' && arg2[0] == 'f') || (arg1[0] == 'f' && arg2[0] == 'i') ||
            (arg1[0] == 'c' && arg2[0] == 'k') || (arg1[0] == 's' && arg2[0] == 'c') 
         || (arg1[0] == 'c' && arg2[0] == 's') || (arg1[0] == 's' && arg2[0] == 'k') || 
            (arg1[0] == 's' && arg2[0] == 'v') || (arg1[0] == 'i' && arg2[0] == 'v') 
         || (arg1[0] == 'c' && arg2[0] == 'v')) return tru;

    else if ( (arg1[0] == 'v' && arg2[0] == 'v') || (arg1[0] != arg2[0]) ) return fal;

    else if ( arg1[0] == arg2[0] ) return tru;

    return fal;
}


/*  set_to_datatype_ name,type,ivalue,cvalue   set_to_datatype_ name,type,index,ivalue,cvalue   set_to_datatype_ name,type,idx1,idx2,ivalue,cvalue */
void exec_set_to(char *text){
    char _set_to_args[256];
    char _set_to_what[64];
    int n = sscanf(text,"set_to_%63[^_]_%s",_set_to_what,_set_to_args);
    if(n < 2){ printf("ERROR: parsing error in exec_set_to \n"); return; }
    if(deb) printf("exec_set_to chiamata con: %s, %s\n",_set_to_what,_set_to_args);
    
    if(strcmp(_set_to_what,"variable") == 0){
        /*set_to_variable_ var0,i,55,@*/ /*set_to_variable_ var0,i,&i&var1&,@*/ /*set_to_variable_ var0,i,@,&c&var1&*/
        char name[64], cvalue[64];
        char value[64];
        char type;
        sscanf(_set_to_args,"%63[^,],%c,%63[^,],%63s",name,&type,value,cvalue);
        if((type == 'i' || type == 'l') && cvalue[0] != '@'){ printf("ERROR: cant set to an int_var a char (use as cplaceholder @\n"); return;}
        else if(type == 'c' && value[0] != '@'){ printf("ERROR: cant set to a char_var an int (use as placeholder @\n"); return;}
        else if(type == 'k' || type == 'n'){ printf("ERROR: use valid type for set to function %s\n",text); return;}

        if(type == 'l'){
            //float nvalue = atof(value);
            float *nvalue= (float *)get_index(value);
            if(nvalue == NULL){ printf("ERROR: cant get %s fl_value in %s\n",value,text); return;}
            set_to_variable(name,type,*nvalue,char_place_holder);
        }
        
        else if(type == 'i'){
            //int nvalue = atoi(value);
            int *nvalue= (int *)get_index(value);
            if(nvalue == NULL){ printf("ERROR: cant get %s int_value in %s\n",value,text); return;}
            set_to_variable(name,type,*nvalue,char_place_holder);
        }

        else if(type == 'c'){
            char *nvalue= (char *)get_index(cvalue);
            if(nvalue == NULL){ printf("ERROR: cant get %s char_value in %s\n",cvalue,text); return;}
            set_to_variable(name,type,place_holder,*nvalue);
        }
    }

    else if(strcmp(_set_to_what,"array") == 0){
        /*set_to_array_ arr0,i,idx1,55,@*/
        char name[64], cvalue[64];
        char value[64];
        char type;
        char index[64];
        sscanf(_set_to_args,"%63[^,],%c,%63[^,],%63[^,],%63s",name,&type,index,value,cvalue);
        if((type == 'i' || type == 'l') && cvalue[0] != '@'){ printf("ERROR: cant set to an int_arr a char (use as cplaceholder @\n"); return;}
        else if(type == 's' && value[0] != '@'){ printf("ERROR: cant set to a char_arr an int (use as placeholder @\n"); return;}
        else if(type == 'k' || type == 'n'){ printf("ERROR: use valid type for set to function %s\n",text);  return;}

        if(type == 'l'){
            //float nvalue = atof(value);
            float *nvalue_ptr = (float *)get_index(value);
            if(nvalue_ptr == NULL){ printf("ERROR: cant get %s fl_value in %s\n",value,text); return;}
            float nvalue = *nvalue_ptr;

            int *nindex_ptr = (int *)get_index(index);
            if(nindex_ptr == NULL){ printf("ERROR: cant get %s index in %s\n",value,text); return;}
            int nindex = *nindex_ptr;
            set_to_array(name,type,nindex,nvalue,char_place_holder);
        }
        
        else if(type == 'i'){
            //int nvalue = atoi(value);
            int *nvalue_ptr = (int *)get_index(value);
            if(nvalue_ptr == NULL){ printf("ERROR: cant get %s int_value in %s\n",value,text); return;}
            int nvalue = *nvalue_ptr;

            int *nindex_ptr = (int *)get_index(index);
            if(nindex_ptr == NULL){ printf("ERROR: cant get %s index in %s\n",value,text); return;}
            int nindex = *nindex_ptr;
            set_to_array(name,type,nindex,nvalue,char_place_holder);
        }

        else if(type == 's'){
            char *nvalue_ptr = (char *)get_index(cvalue);
            if(nvalue_ptr == NULL){ printf("ERROR: cant get %s char_value in %s\n",cvalue,text); return;}
            char nvalue = *nvalue_ptr;
           
            int *nindex_ptr = (int *)get_index(index);
            if(nindex_ptr == NULL){ printf("ERROR: cant get %s index in %s\n",value,text); return;}
            int nindex = *nindex_ptr;
            set_to_array(name,type,nindex,char_place_holder,nvalue);
        }
    }

    else if(strcmp(_set_to_what,"matrix") == 0){
        /*set_to_matrix_ matr0,i,idx1,idx2,55,@*/
        char name[64], cvalue[64];
        char value[64];
        char type;
        char idx1[64], idx2[64];
        sscanf(_set_to_args,"%63[^,],%c,%63[^,],%63[^,],%63[^,],%63s",name,&type,idx1,idx2,value,cvalue);
        if((type == 'i' || type == 'l') && cvalue[0] != '@'){ printf("ERROR: cant set to an int_matrix a char (use as cplaceholder @\n"); return;}
        else if(type == 's' && value[0] != '@'){ printf("ERROR: cant set to a char_matrix an int (use as placeholder @\n"); return;}
        else if(type == 'k' || type == 'n'){ printf("ERROR: use valid type for set to function %s\n",text);  return;}

        if(type == 'l'){
            //float nvalue = atof(value);
            float *nvalue= (float *)get_index(value);
            if(nvalue == NULL){ printf("ERROR: cant get %s fl_value in %s\n",value,text); return;}
            float nnvalue = *nvalue;

            int *nindex1= (int *)get_index(idx1);
            if(nindex1 == NULL){ printf("ERROR: cant get %s index1 in %s\n",idx1,text); return;}
            int nnindex1 = *nindex1;
            int *nindex2= (int *)get_index(idx2);
            if(nindex2 == NULL){ printf("ERROR: cant get %s index2 in %s\n",idx2,text); return;}
            int nnindex2 = *nindex2;

            set_to_matrix(name,type,nnindex1,nnindex2,nnvalue,char_place_holder);
        }
        
        else if(type == 'i'){
            //int nvalue = atoi(value);
            int *nvalue= (int *)get_index(value);
            if(nvalue == NULL){ printf("ERROR: cant get %s int_value in %s\n",value,text); return;}
            int nnvalue = *nvalue;

            int *nindex1= (int *)get_index(idx1);
            if(nindex1 == NULL){ printf("ERROR: cant get %s index1 in %s\n",idx1,text); return;}
            int nnindex1 = *nindex1;
            int *nindex2= (int *)get_index(idx2);
            if(nindex2 == NULL){ printf("ERROR: cant get %s index2 in %s\n",idx2,text); return;}
            int nnindex2 = *nindex2;

            set_to_matrix(name,type,nnindex1,nnindex2,nnvalue,char_place_holder);
        }

        else if(type == 's'){
            char *nvalue= (char *)get_index(cvalue);
            if(nvalue == NULL){ printf("ERROR: cant get %s char_value in %s\n",cvalue,text);}
            char nnvalue = *nvalue;

            int *nindex1= (int *)get_index(idx1);
            if(nindex1 == NULL){ printf("ERROR: cant get %s index1 in %s\n",idx1,text); return;}
            int nnindex1 = *nindex1;
            int *nindex2= (int *)get_index(idx2);
            if(nindex2 == NULL){ printf("ERROR: cant get %s index2 in %s\n",idx2,text); return;}
            int nnindex2 = *nindex2;

            set_to_matrix(name,type,nnindex1,nnindex2,place_holder,nnvalue);
        }
    }

    else{
        printf("ERROR: no type available for set_to_%s_ \n",_set_to_what);
    }
}

static int resolve_index(const char *idx_str) {
    int val;
    if (sscanf(idx_str, "%d", &val) == 1) return val;
    
    for (int i = 0; i < variable_count; i++)
        if (strcmp(variable[i].name, idx_str) == 0) return variable[i].value;
    printf("ERROR: cannot resolve index '%s'\n", idx_str);
    return -1;
}

static int types_match(char l, char right) {
    char ls[2] = {l, '\0'};
    char rs[2] = {right, '\0'};
    return check_if_same_type(ls, rs);
}

int check_return(int line) {
    if (line-1 == line_idx_program) {
        if(deb) printf("ERROR: return outside a function %s line: %d global_ip: %d line_idx: %d\n",program[global_ip].instruction,line,global_ip,line_idx_program);
        return fal;
    }
    return tru;
}

void* exec_funarg(char *name_plus_args, int is_return) {

    int no_value_to_return = fal; //una funazione base ritorna 1 valore

    //IF START
    if(strstr(name_plus_args,"start")){
        return pt_place_holder;
    }

    // ======= IF RETURN ==========
    if (is_return) {
        if(deb) printf("DEBUG: funarg called with is_return line: %s\n", name_plus_args);

        char buffer[128] = {0};
        sscanf(name_plus_args, "return_%127s", buffer);   // FIX: source corretto

        if (strlen(buffer) == 0 || strcmp(buffer, "NULL") == 0) {
            if(deb) printf("WARNING: no item to return\n");
            return_value = NULL;
            return_hit   = 1;
            return NULL;
        }
        else if (strstr(buffer, "--")) {
            if(deb) printf("WARNING: more than one item to return\n");
            return_value = pt_place_holder;
            return_hit   = 1;
            return pt_place_holder;
        }
        else {
            void *ret = NULL;
            char bin[32] = {0};
            strncpy(bin, buffer, sizeof(bin) - 1);
            is_what(bin);

            char genre[16] = {0};
            char type = 0;
            sscanf(bin, "%15[^.].%c", genre, &type);

            return_type = type;

            if (strchr(buffer, '&')) {
                ret = get_index(buffer);
            } else {
                ret = resolve(type, buffer);
            }

            return_value = ret;
            return_hit   = 1;
            return ret;
        }
    }

    // ======= CALL FUNCTION ==========
    else {
        if(deb) printf("DEBUG: funarg chiamata con name+args: %s\n", name_plus_args);

        char name[32] = {0};
        char args[64] = {0};

        if (strstr(name_plus_args, "__")) {
            sscanf(name_plus_args, "__%31[^(](%63[^)])", name, args);
            no_value_to_return = tru;
        }
        else {
            strncpy(name, name_plus_args, sizeof(name) - 1);
        }

        if(deb) printf("function to start analyse in funarg as: %s, return_state %d\n", name, return_state);

        int st_ip = 0, end_ip = 0;

        int i;

        for (i = 0; i < return_state; i++) {
            if(deb) printf("i: %d, return_state: %d, name: %s, state_stack[%d]: %s\n",i,return_state,name,i,state_stack[i].nome_function);
            if (strcmp(state_stack[i].nome_function + 3, name) == 0) {
                st_ip  = state_stack[i].posizione_ritorno;
                end_ip = state_stack[i].posizione_skip;
                break;
            }
        }

        if (i == return_state) {
            printf("ERROR: function %s not found\n", name);
            return NULL;
        }

        if(deb) printf("function found: %s  start: %d  end: %d\n", name, st_ip, end_ip);

        int svaed_ip = global_ip;

        // reset prima di entrare: ogni chiamata parte pulita
        return_hit   = 0;
        return_value = NULL;

        parse(st_ip, end_ip);

        // dopo parse il valore è in return_value; reset il flag per il chiamante
        void *ret = return_value;
        return_hit   = 0;
        return_value = NULL;

        global_ip = svaed_ip;

        if(deb) printf("correctly went out parse from funarg, return: %p\n", ret);
        return ret;
    }
}

// fix anche in math_plus: stessa firma e stessi cast
int math_plus(char *operation, int called_by_parse) {
    char lop[24] = {0}, rop[24] = {0};
    if(deb) printf("DEBUG: math_plus chiamata con %s\n", operation);
    if(called_by_parse) { printf("ERROR: arithmetic needs = to save result\n"); return error_int; }

    if(sscanf(operation, "%23[^+]+%23s", lop, rop) != 2) {
        printf("ERROR: failed parse in math_plus %s\n", operation); return error_int;
    }

    char junk[16], ltype = 0, rtype = 0;
    int lopv = 0, ropv = 0;

    if(strchr(lop,'&')) {
        sscanf(lop, "&%c&%23[^&]&", &ltype, lop);
    } else {
        char clop[24]; strcpy(clop,lop); is_what(clop);
        sscanf(clop, "%15[^.].%c", junk, &ltype);
    }
    if(ltype=='i')      lopv = *(int   *)resolve(ltype,lop);
    else if(ltype=='l') lopv = (int)*(float*)resolve(ltype,lop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",lop); }

    if(strchr(rop,'&')) {
        sscanf(rop, "&%c&%23[^&]&", &rtype, rop);
    } else {
        char crop[24]; strcpy(crop,rop); is_what(crop);
        sscanf(crop, "%15[^.].%c", junk, &rtype);
    }
    if(rtype=='i')      ropv = *(int   *)resolve(rtype,rop);
    else if(rtype=='l') ropv = (int)*(float*)resolve(rtype,rop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",rop); }

    if(!types_match(ltype,rtype)) { printf("ERROR: type mismatch in math_plus\n"); return error_int; }
    return lopv + ropv;
}

int math_min(char *operation, int called_by_parse) {
    char lop[24] = {0}, rop[24] = {0};
    if(deb) printf("DEBUG: math_min chiamata con %s\n", operation);
    if(called_by_parse) { printf("ERROR: arithmetic needs = to save result\n"); return error_int; }

    if(sscanf(operation, "%23[^-]-%23s", lop, rop) != 2) {
        printf("ERROR: failed parse in math_min %s\n", operation); return error_int;
    }

    char junk[16], ltype = 0, rtype = 0;
    int lopv = 0, ropv = 0;

    if(strchr(lop,'&')) {
        sscanf(lop, "&%c&%23[^&]&", &ltype, lop);
    } else {
        char clop[24]; strcpy(clop,lop); is_what(clop);
        sscanf(clop, "%15[^.].%c", junk, &ltype);
    }
    if(ltype=='i')      lopv = *(int   *)resolve(ltype,lop);
    else if(ltype=='l') lopv = (int)*(float*)resolve(ltype,lop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",lop); }

    if(strchr(rop,'&')) {
        sscanf(rop, "&%c&%23[^&]&", &rtype, rop);
    } else {
        char crop[24]; strcpy(crop,rop); is_what(crop);
        sscanf(crop, "%15[^.].%c", junk, &rtype);
    }
    if(rtype=='i')      ropv = *(int   *)resolve(rtype,rop);
    else if(rtype=='l') ropv = (int)*(float*)resolve(rtype,rop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",rop); }

    if(!types_match(ltype,rtype)) { printf("ERROR: type mismatch in math_min\n"); return error_int; }
    return lopv - ropv;
}

int math_times(char *operation, int called_by_parse) {
    char lop[24] = {0}, rop[24] = {0};
    if(deb) printf("DEBUG: math_times chiamata con %s\n", operation);
    if(called_by_parse) { printf("ERROR: arithmetic needs = to save result\n"); return error_int; }

    if(sscanf(operation, "%23[^*]*%23s", lop, rop) != 2) {
        printf("ERROR: failed parse in math_times %s\n", operation); return error_int;
    }

    char junk[16], ltype = 0, rtype = 0;
    int lopv = 0, ropv = 0;

    if(strchr(lop,'&')) {
        sscanf(lop, "&%c&%23[^&]&", &ltype, lop);
    } else {
        char clop[24]; strcpy(clop,lop); is_what(clop);
        sscanf(clop, "%15[^.].%c", junk, &ltype);
    }
    if(ltype=='i')      lopv = *(int   *)resolve(ltype,lop);
    else if(ltype=='l') lopv = (int)*(float*)resolve(ltype,lop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",lop); }

    if(strchr(rop,'&')) {
        sscanf(rop, "&%c&%23[^&]&", &rtype, rop);
    } else {
        char crop[24]; strcpy(crop,rop); is_what(crop);
        sscanf(crop, "%15[^.].%c", junk, &rtype);
    }
    if(rtype=='i')      ropv = *(int   *)resolve(rtype,rop);
    else if(rtype=='l') ropv = (int)*(float*)resolve(rtype,rop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",rop); }

    if(!types_match(ltype,rtype)) { printf("ERROR: type mismatch in math_times\n"); return error_int; }
    return lopv * ropv;
}

int math_slash(char *operation, int called_by_parse) {
    char lop[24] = {0}, rop[24] = {0};
    if(deb) printf("DEBUG: math_slash chiamata con %s\n", operation);
    if(called_by_parse) { printf("ERROR: arithmetic needs = to save result\n"); return error_int; }

    if(sscanf(operation, "%23[^/]/%23s", lop, rop) != 2) {
        printf("ERROR: failed parse in math_slash %s\n", operation); return error_int;
    }

    char junk[16], ltype = 0, rtype = 0;
    int lopv = 0, ropv = 0;

    if(strchr(lop,'&')) {
        sscanf(lop, "&%c&%23[^&]&", &ltype, lop);
    } else {
        char clop[24]; strcpy(clop,lop); is_what(clop);
        sscanf(clop, "%15[^.].%c", junk, &ltype);
    }
    if(ltype=='i')      lopv = *(int   *)resolve(ltype,lop);
    else if(ltype=='l') lopv = (int)*(float*)resolve(ltype,lop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",lop); }

    if(strchr(rop,'&')) {
        sscanf(rop, "&%c&%23[^&]&", &rtype, rop);
    } else {
        char crop[24]; strcpy(crop,rop); is_what(crop);
        sscanf(crop, "%15[^.].%c", junk, &rtype);
    }
    if(rtype=='i')      ropv = *(int   *)resolve(rtype,rop);
    else if(rtype=='l') ropv = (int)*(float*)resolve(rtype,rop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",rop); }

    if(!types_match(ltype,rtype)) { printf("ERROR: type mismatch in math_slash\n"); return error_int; }
    if(ropv == 0) { printf("ERROR: division by zero\n"); return error_int; }
    return lopv / ropv;
}

int is_math(char *operand) {
    if(deb) printf("DEBUG: is_math chiamata con %s\n", operand);

    if(strchr(operand,'+')) return math_plus (operand, fal);
    if(strchr(operand,'-')) return math_min  (operand, fal);
    if(strchr(operand,'*')) return math_times(operand, fal);
    if(strchr(operand,'/')) return math_slash(operand, fal);

    return error_int;  // non è math
}

void exec_equal(char *text) {

    char left_operand[32] = {0};
    char right_operand[32] = {0};

    if (text[0] == '=') {
        printf("ERROR: im crying operation not allowed\n"); return;
    }
    else if (text[strlen(text)-1] == '=') {
        printf("ERROR: im crying operation not allowed\n"); return;
    }
    else {
        int n = sscanf(text, "%23[^=]=%23s", left_operand, right_operand);
        if(deb) printf("\ntext: %s, left op: %s, right op: %s\n", text, left_operand, right_operand);
        if (n < 2) { printf("ERROR: parse error in x=x: %s\n", text); return; }
    }

    char left_name[64]  = {0};
    char right_name[64] = {0};
    char left_i[16]     = {0};
    char right_i[16]    = {0};
    char left_j[16]     = {0};
    char right_j[16]    = {0};

    /*===========INTERNAL PARSE==========*/

    /*MATRIX*/
    if (strstr(left_operand, "][") && strstr(right_operand, "][")) { /*matr x matr*/
        sscanf(left_operand,  "[%[^]]][%[^]]]%63s", left_i,  left_j,  left_name);
        sscanf(right_operand, "[%[^]]][%[^]]]%63s", right_i, right_j, right_name);
        if(deb) printf("[CHECK] matr x matr | left: [%s][%s]%s | right: [%s][%s]%s\n",
               left_i, left_j, left_name, right_i, right_j, right_name);

        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        char rtdata[16] = {0}; char rtype = 0;

        strcpy(bin, left_name);  is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "matrix") != 0) { printf("ERROR: %s is not a matrix\n", left_name); return; }

        strcpy(bin, right_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", rtdata, &rtype);
        if (strcmp(rtdata, "matrix") != 0) { printf("ERROR: %s is not a matrix\n", right_name); return; }

        if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return; }

        int li = resolve_index(left_i),  lj = resolve_index(left_j);
        int ri = resolve_index(right_i), rj = resolve_index(right_j);
        if (li < 0 || lj < 0 || ri < 0 || rj < 0) return;

        char lenc[128], renc[128];
        snprintf(lenc, sizeof(lenc), "&%c[%d][%d]&%s&", ltype, li, lj, left_name);
        snprintf(renc, sizeof(renc), "&%c[%d][%d]&%s&", rtype, ri, rj, right_name);

        if (ltype == 'i') {
            int *dest = (int *)get_index(lenc);
            int *src  = (int *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
            *dest = *src;
        } else if (ltype == 'l') {
            float *dest = (float *)get_index(lenc);
            float *src  = (float *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
            *dest = *src;
        } else if (ltype == 's') {
            char *dest = (char *)get_index(lenc);
            char *src  = (char *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
            *dest = *src;
        }
    }

    else if (strstr(left_operand, "][") && !strstr(right_operand, "][")) { /*matr x boh*/
        sscanf(left_operand, "[%[^]]][%[^]]]%63s", left_i, left_j, left_name);

        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        strcpy(bin, left_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "matrix") != 0) { printf("ERROR: %s is not a matrix\n", left_name); return; }

        int li = resolve_index(left_i), lj = resolve_index(left_j);
        if (li < 0 || lj < 0) return;

        char lenc[128];
        snprintf(lenc, sizeof(lenc), "&%c[%d][%d]&%s&", ltype, li, lj, left_name);

        int math_res = is_math(right_operand);
        if(math_res != error_int) {
            if(deb) printf("[CHECK] matr x math | left: [%s][%s]%s | right: %s\n", left_i, left_j, left_name, right_operand);
            if(ltype=='i'){
                int *dest = (int*)get_index(lenc);
                if(!dest){ printf("ERROR: null pointer\n"); return; }
                *dest = math_res;
            } else if(ltype=='l'){
                float *dest = (float*)get_index(lenc);
                if(!dest){ printf("ERROR: null pointer\n"); return; }
                *dest = (float)math_res;
            }
            return;
        }

        else if (right_operand[0] == '\'') { /*matr x k*/
            if(deb) printf("[CHECK] matr x k | left: [%s][%s]%s | right: %s\n",
                   left_i, left_j, left_name, right_operand);
            if (!types_match(ltype, 'k')) { printf("ERROR: type mismatch matrix/char\n"); return; }
            char *dest = (char *)get_index(lenc);
            if (!dest) { printf("ERROR: null pointer\n"); return; }
            *dest = right_operand[1];
        }
        else if (isdigit((unsigned char)right_operand[0])) { /*matr x n*/
            if(deb) printf("[CHECK] matr x n | left: [%s][%s]%s | right: %s\n",
                   left_i, left_j, left_name, right_operand);
            if (!types_match(ltype, 'n')) { printf("ERROR: type mismatch matrix/number\n"); return; }
            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = atoi(right_operand);
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = (float)atof(right_operand);
            }
        }
        else if (strchr(right_operand, ']')) { /*matr x arr*/
            sscanf(right_operand, "[%[^]]]%63s", right_i, right_name);
            if(deb) printf("[CHECK] matr x arr | left: [%s][%s]%s | right: [%s]%s\n",
                   left_i, left_j, left_name, right_i, right_name);

            char rtdata[16] = {0}; char rtype = 0;
            strcpy(bin, right_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", rtdata, &rtype);
            if (strcmp(rtdata, "array") != 0) { printf("ERROR: %s is not an array\n", right_name); return; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return; }

            int ri = resolve_index(right_i);
            if (ri < 0) return;
            char renc[128];
            snprintf(renc, sizeof(renc), "&%c[%d]&%s&", rtype, ri, right_name);

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                int *src  = (int *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                float *src  = (float *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                char *src  = (char *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            }
        }
        else if (strstr(right_operand, "__")) { /*matr x func*/
            if(deb) printf("[CHECK] matr x func | left: [%s][%s]%s | right: %s\n",
                left_i, left_j, left_name, right_operand);
           if (!types_match(ltype, 'v')) { printf("ERROR: type mismatch matrix/function\n"); return; }

            void *ret = get_index(right_operand);
            if (!ret) { printf("ERROR: null pointer from function\n"); return; }

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = *(int *)ret;
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = *(float *)ret;
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = *(char *)ret;
            }
        }
        else { /*matr x var*/
            sscanf(right_operand, "%63s", right_name);
            if(deb) printf("[CHECK] matr x var | left: [%s][%s]%s | right: %s\n",
                   left_i, left_j, left_name, right_name);

            char rtdata[16] = {0}; char rtype = 0;
            strcpy(bin, right_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", rtdata, &rtype);
            if (strcmp(rtdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", right_name); return; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return; }

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                int *src  = (int *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                float *src  = (float *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                char *src  = (char *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            }
        }
    }

    else if (!strstr(left_operand, "][") && strstr(right_operand, "][")) { /*boh x matr*/
        sscanf(right_operand, "[%[^]]][%[^]]]%63s", right_i, right_j, right_name);

        char bin[16];
        char rtdata[16] = {0}; char rtype = 0;
        strcpy(bin, right_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", rtdata, &rtype);
        if (strcmp(rtdata, "matrix") != 0) { printf("ERROR: %s is not a matrix\n", right_name); return; }

        int ri = resolve_index(right_i), rj = resolve_index(right_j);
        if (ri < 0 || rj < 0) return;
        char renc[128];
        snprintf(renc, sizeof(renc), "&%c[%d][%d]&%s&", rtype, ri, rj, right_name);

        if (strchr(left_operand, ']')) { /*arr x matr*/
            sscanf(left_operand, "[%[^]]]%63s", left_i, left_name);
            if(deb) printf("[CHECK] arr x matr | left: [%s]%s | right: [%s][%s]%s\n",
                   left_i, left_name, right_i, right_j, right_name);

            char ltdata[16] = {0}; char ltype = 0;
            strcpy(bin, left_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", ltdata, &ltype);
            if (strcmp(ltdata, "array") != 0) { printf("ERROR: %s is not an array\n", left_name); return; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return; }

            int li = resolve_index(left_i);
            if (li < 0) return;
            char lenc[128];
            snprintf(lenc, sizeof(lenc), "&%c[%d]&%s&", ltype, li, left_name);

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                int *src  = (int *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                float *src  = (float *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                char *src  = (char *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            }
        }
        else { /*var x matr*/
            sscanf(left_operand, "%63s", left_name);
            if(deb) printf("[CHECK] var x matr | left: %s | right: [%s][%s]%s\n",
                   left_name, right_i, right_j, right_name);

            char ltdata[16] = {0}; char ltype = 0;
            strcpy(bin, left_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", ltdata, &ltype);
            if (strcmp(ltdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", left_name); return; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return; }

            if (ltype == 'i') {
                int *dest = (int *)resolve(ltype, left_name);
                int *src  = (int *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 'l') {
                float *dest = (float *)resolve(ltype, left_name);
                float *src  = (float *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 'c') {
                char *dest = (char *)resolve(ltype, left_name);
                char *src  = (char *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            }
        }
    }

    /*ARRAY*/
    else if (strchr(left_operand, ']') && strchr(right_operand, ']')) { /*arr x arr*/
        sscanf(left_operand,  "[%[^]]]%63s", left_i,  left_name);
        sscanf(right_operand, "[%[^]]]%63s", right_i, right_name);
        if(deb) printf("[CHECK] arr x arr | left: [%s]%s | right: [%s]%s\n",
               left_i, left_name, right_i, right_name);

        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        char rtdata[16] = {0}; char rtype = 0;

        strcpy(bin, left_name);  is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "array") != 0) { printf("ERROR: %s is not an array\n", left_name); return; }

        strcpy(bin, right_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", rtdata, &rtype);
        if (strcmp(rtdata, "array") != 0) { printf("ERROR: %s is not an array\n", right_name); return; }

        if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return; }

        int li = resolve_index(left_i);
        int ri = resolve_index(right_i);
        if (li < 0 || ri < 0) return;

        char lenc[128], renc[128];
        snprintf(lenc, sizeof(lenc), "&%c[%d]&%s&", ltype, li, left_name);
        snprintf(renc, sizeof(renc), "&%c[%d]&%s&", rtype, ri, right_name);

        if (ltype == 'i') {
            int *dest = (int *)get_index(lenc);
            int *src  = (int *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
            *dest = *src;
        } else if (ltype == 'l') {
            float *dest = (float *)get_index(lenc);
            float *src  = (float *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
            *dest = *src;
        } else if (ltype == 's' || ltype == 'c') {
            char *dest = (char *)get_index(lenc);
            char *src  = (char *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
            *dest = *src;
        }
    }

    else if (strchr(left_operand, ']') && !strchr(right_operand, ']')) { /*arr x boh*/
        sscanf(left_operand, "[%[^]]]%63s", left_i, left_name);

        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        strcpy(bin, left_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "array") != 0) { printf("ERROR: %s is not an array\n", left_name); return; }

        int li = resolve_index(left_i);
        if (li < 0) return;
        char lenc[128];
        snprintf(lenc, sizeof(lenc), "&%c[%d]&%s&", ltype, li, left_name);

        int math_res = is_math(right_operand);
        if(math_res != error_int) {
            if(deb) printf("[CHECK] arr x math | left: [%s]%s | right: %s\n", left_i, left_name, right_operand);
            if(ltype=='i'){
                int *dest = (int*)get_index(lenc);
                if(!dest){ printf("ERROR: null pointer\n"); return; }
                *dest = math_res;
            } else if(ltype=='l'){
                float *dest = (float*)get_index(lenc);
                if(!dest){ printf("ERROR: null pointer\n"); return; }
                *dest = (float)math_res;
            }
            return;
        }
        else if (right_operand[0] == '\'') { /*arr x k*/
            if(deb) printf("[CHECK] arr x k | left: [%s]%s | right: %s\n", left_i, left_name, right_operand);
            if (!types_match(ltype, 'k')) { printf("ERROR: type mismatch array/char\n"); return; }
            char *dest = (char *)get_index(lenc);
            if (!dest) { printf("ERROR: null pointer\n"); return; }
            *dest = right_operand[1];
        }
        else if (isdigit((unsigned char)right_operand[0])) { /*arr x n*/
            if(deb) printf("[CHECK] arr x n | left: [%s]%s | right: %s\n", left_i, left_name, right_operand);
            if (!types_match(ltype, 'n')) { printf("ERROR: type mismatch array/number\n"); return; }
            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = atoi(right_operand);
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = (float)atof(right_operand);
            }
        }
        else if (strstr(right_operand, "__")) { /*arr x func*/
            if(deb) printf("[CHECK] arr x func | left: [%s]%s | right: %s\n",
                left_i, left_name, right_operand);
                if (!types_match(ltype, 'v')) { printf("ERROR: type mismatch array/function\n"); return; }

            void *ret = get_index(right_operand);
            if (!ret) { printf("ERROR: null pointer from function\n"); return; }

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = *(int *)ret;
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = *(float *)ret;
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = *(char *)ret;
            }
        }
        else { /*arr x var*/
            sscanf(right_operand, "%63s", right_name);
            if(deb) printf("[CHECK] arr x var | left: [%s]%s | right: %s\n", left_i, left_name, right_name);

            char rtdata[16] = {0}; char rtype = 0;
            strcpy(bin, right_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", rtdata, &rtype);
            if (strcmp(rtdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", right_name); return; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return; }

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                int *src  = (int *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                float *src  = (float *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 's' || ltype == 'c') {
                char *dest = (char *)get_index(lenc);
                char *src  = (char *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            }
        }
    }

    else if (!strchr(left_operand, ']') && strchr(right_operand, ']')) { /*var x arr*/
        sscanf(left_operand,  "%63s",        left_name);
        sscanf(right_operand, "[%[^]]]%63s", right_i, right_name);
        if(deb) printf("[CHECK] var x arr | left: %s | right: [%s]%s\n", left_name, right_i, right_name);

        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        char rtdata[16] = {0}; char rtype = 0;

        strcpy(bin, left_name);  is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", left_name); return; }

        strcpy(bin, right_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", rtdata, &rtype);
        if (strcmp(rtdata, "array") != 0) { printf("ERROR: %s is not an array\n", right_name); return; }

        if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return; }

        int ri = resolve_index(right_i);
        if (ri < 0) return;
        char renc[128];
        snprintf(renc, sizeof(renc), "&%c[%d]&%s&", rtype, ri, right_name);

        if (ltype == 'i') {
            int *dest = (int *)resolve(ltype, left_name);
            int *src  = (int *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
            *dest = *src;
        } else if (ltype == 'l') {
            float *dest = (float *)resolve(ltype, left_name);
            float *src  = (float *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
            *dest = *src;
        } else if (ltype == 's' || ltype == 'c') {
            char *dest = (char *)resolve(ltype, left_name);
            char *src  = (char *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
            *dest = *src;
        }
    }

    /*VARIABILI*/
    else if (!strchr(left_operand, ']') && (!strchr(right_operand, ']') || strstr(right_operand,"__")) ) {

        int math_res = is_math(right_operand);
        if(math_res != error_int) {
            sscanf(left_operand, "%63s", left_name);
            if(deb) printf("[CHECK] var x math | left: %s | right: %s\n", left_name, right_operand);
            char bin[16]; char ltdata[16]={0}; char ltype=0;
            strcpy(bin,left_name); is_what(bin);
            sscanf(bin,"%15[^.].%c",ltdata,&ltype);
            if(strcmp(ltdata,"variable")!=0){ printf("ERROR: %s is not a variable\n",left_name); return; }
            if(ltype=='i'){
                int *dest = (int*)resolve(ltype,left_name);
                if(!dest){ printf("ERROR: null pointer\n"); return; }
                *dest = math_res;
            } else if(ltype=='l'){
                float *dest = (float*)resolve(ltype,left_name);
                if(!dest){ printf("ERROR: null pointer\n"); return; }
                *dest = (float)math_res;
            }
            return;
        }
        else if (right_operand[0] == '\'') { /*var x k*/
            sscanf(left_operand, "%63s", left_name);
            if(deb) printf("[CHECK] var x k | left: %s | right: %s\n", left_name, right_operand);

            char bin[16];
            char ltdata[16] = {0}; char ltype = 0;
            strcpy(bin, left_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", ltdata, &ltype);
            if (strcmp(ltdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", left_name); return; }
            if (!types_match(ltype, 'k')) { printf("ERROR: type mismatch var/char\n"); return; }

            char *dest = (char *)resolve(ltype, left_name);
            if (!dest) { printf("ERROR: null pointer\n"); return; }
            *dest = right_operand[1];
        }

        else if (isdigit((unsigned char)right_operand[0])) { /*var x n*/
            sscanf(left_operand, "%63s", left_name);
            if(deb) printf("[CHECK] var x n | left: %s | right: %s\n", left_name, right_operand);

            char bin[16];
            char ltdata[16] = {0}; char ltype = 0;
            strcpy(bin, left_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", ltdata, &ltype);
            if (strcmp(ltdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", left_name); return; }
            if (!types_match(ltype, 'n')) { printf("ERROR: type mismatch var/number\n"); return; }

            if (ltype == 'i') {
                int *dest = (int *)resolve(ltype, left_name);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = atoi(right_operand);
            } else if (ltype == 'l') {
                float *dest = (float *)resolve(ltype, left_name);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = (float)atof(right_operand);
            }
        }

        else if (strstr(right_operand,"__")) { /*var x __function*/
            sscanf(left_operand, "%63s", left_name);
            sscanf(right_operand, "%63s", right_name);
            if(deb) printf("[CHECK] var x func | left: %s | right: %s\n", left_name, right_operand);

            char bin[16];
            char rtdata[16] = {0}; char rtype = 0;
            strcpy(bin, right_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", rtdata, &rtype);
            if (strcmp(rtdata, "function") != 0) { printf("ERROR: %s is not a function\n", right_name); return; }

            char ltdata[16] = {0}; char ltype = 0;
            strcpy(bin, left_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", ltdata, &ltype);
            if (strcmp(ltdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", left_name); return; }

            if (!types_match(ltype, 'v')) { printf("ERROR: type mismatch var/function\n"); return; }

            if (ltype == 'i') {
                int *dest = (int *)resolve(ltype, left_name);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = *(int *)get_index(right_name);
            } 
            else if (ltype == 'l') {
                float *dest = (float *)resolve(ltype, left_name);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = *(float *)get_index(right_name);
            } 
            else if (ltype == 'c') {
                char *dest = (char *)resolve(ltype, left_name);
                if (!dest) { printf("ERROR: null pointer\n"); return; }
                *dest = *(char *)get_index(right_name);
            }
        }

        else { /*var x var*/
            sscanf(left_operand,  "%63s", left_name);
            sscanf(right_operand, "%63s", right_name);
            if(deb) printf("[CHECK] var x var | left: %s | right: %s\n", left_name, right_name);

            char bin[16];
            char ltdata[16] = {0}; char ltype = 0;
            char rtdata[16] = {0}; char rtype = 0;

            strcpy(bin, left_name);  is_what(bin);
            sscanf(bin, "%15[^.].%c", ltdata, &ltype);
            if (strcmp(ltdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", left_name); return; }

            strcpy(bin, right_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", rtdata, &rtype);
            if (strcmp(rtdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", right_name); return; }

            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return; }

            if (ltype == 'i') {
                int *dest = (int *)resolve(ltype, left_name);
                int *src  = (int *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 'l') {
                float *dest = (float *)resolve(ltype, left_name);
                float *src  = (float *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            } else if (ltype == 'c') {
                char *dest = (char *)resolve(ltype, left_name);
                char *src  = (char *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return; }
                *dest = *src;
            }
        }
    }
}

void parse(int start_line, int  eventual_end_line){

    if(eventual_end_line == -1) eventual_end_line = line_idx_program;
    global_ip = start_line;
    if(deb) printf("parse chiamato con global_ip: %d e line_idx_program: %d\n",global_ip,eventual_end_line);


    while( !return_hit && (global_ip <= eventual_end_line) ){

        if(deb) printf("\nlinea analizzata: %d %s\n",global_ip, program[global_ip].instruction);

        if( starts_with (program[global_ip].instruction, "//") ) { /*comment do nothing*/ }
        else if( starts_with (program[global_ip].instruction, "print_") ){ exec_print(program[global_ip].instruction);/*stampa carattere a terminale */ }
        else if( starts_with (program[global_ip].instruction, "lnprint_") ){ exec_lnprint(program[global_ip].instruction);/*va a capo stampa carattere a terminale */ }
        else if( starts_with (program[global_ip].instruction, "println_") ){ exec_println(program[global_ip].instruction);/*stampa carattere a terminale e va a capo */ }
        else if( starts_with (program[global_ip].instruction, "lnprintln_") ){ exec_lnprintln(program[global_ip].instruction);/*va a capo stampa carattere a terminale e va a capo */ }
        else if( strstr (program[global_ip].instruction, "==") ){ /*see older build*/ }
        else if( strchr (program[global_ip].instruction, '=') ){ exec_equal(program[global_ip].instruction);}
        else if( starts_with (program[global_ip].instruction, "#") ){ /*funzioni di sistema e interprete tipo import o kill*/ }
        else if( starts_with (program[global_ip].instruction, "od") ){ /*dichiara funzione od (open door) ma non ha gran senso*/ }
        else if( starts_with (program[global_ip].instruction, "__") ){ exec_funarg(program[global_ip].instruction, fal); }
        else if( starts_with (program[global_ip].instruction, "int_") ){ exec_int(program[global_ip].instruction); /*dichiara var int  */ }
        else if( starts_with (program[global_ip].instruction, "char_") ){ exec_char(program[global_ip].instruction); /*dichiara var char */ }
        else if( starts_with (program[global_ip].instruction, "if_") ){/*inizia if_ */ }
        else if( starts_with (program[global_ip].instruction, "else_") ){/*else */ }
        else if( starts_with (program[global_ip].instruction, "for_") ){/*inizia for */ }
        else if( starts_with (program[global_ip].instruction, "while_") ){/*inizia while */ }
        else if ( starts_with(program[global_ip].instruction, "return_") && check_return(eventual_end_line) == tru ) {
            exec_funarg(program[global_ip].instruction, tru); /* return_hit è ora 1 quindi il while esce al prossimo controllo */}
        else if( starts_with (program[global_ip].instruction, "set_to_") ){ exec_set_to(program[global_ip].instruction); }
        else if( strstr (program[global_ip].instruction, "++") ){ /*operazione ++ di sicuro perche cicli gia parsati*/}
        else if( strstr (program[global_ip].instruction, "--") ){ /*operazione -- di sicuro perche cicli gia parsati*/}
        else if( strchr (program[global_ip].instruction, '+') ){ /*operazione + di sicuro perche cicli gia parsati*/}
        else if( strchr (program[global_ip].instruction, '-') ){ /*operazione - di sicuro perche cicli gia parsati*/}
        else if( strchr (program[global_ip].instruction, '*') ){ /*operazione * di sicuro perche cicli gia parsati*/}
        else if( strchr (program[global_ip].instruction, '/') ){ /*operazione / di sicuro perche cicli gia parsati*/}
        else if( starts_with (program[global_ip].instruction, "scan_") ){/*legge carattere da schermo */ }
        global_ip++;
    }

    return_hit = fal;
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
            if(deb) printf("program[%d].instruction: %s\n",i,program[i].instruction);
            state_stack[return_state].codice = return_state;
            state_stack[return_state].posizione_ritorno = i;
            if( starts_with(program[i].instruction,"if_") ) strcpy(state_stack[return_state].nome_function, "if_");
            else if( starts_with(program[i].instruction,"for_") ) strcpy(state_stack[return_state].nome_function, "for_");
            else if( starts_with(program[i].instruction,"while_") ) strcpy(state_stack[return_state].nome_function, "while_");
            else if( starts_with(program[i].instruction,"else_") ) strcpy(state_stack[return_state].nome_function, "else_");
            else if( starts_with(program[i].instruction,"__start") ) strcpy(state_stack[return_state].nome_function, "__start");
            else if( starts_with(program[i].instruction,"od_") ){ 
                     char bin[64], bin_name[16], name[24]; 
                     sscanf(program[i].instruction, "od_%15[^ (]", bin_name);
                     sprintf(name,"od_%s",bin_name); //SINGOLA
                     strcpy(state_stack[return_state].nome_function, name); 
                     }
            else if( starts_with(program[i].instruction,"#") ) strcpy(state_stack[return_state].nome_function, "#"); 
            state_stack[return_state].posizione_skip = j;

            return_state++;
        }

        i++;
    }
}


void run_test(){
 
    // ===== VARIABILI =====
    declare_variable("var0",'i');
    declare_variable("var1",'c');
    declare_variable("var2",'l');
    printf("LOG: decl_var ok \n");
 
    set_to_variable("var0",'i', 3, place_holder);
    set_to_variable("var1",'c', place_holder, 'F');
    set_to_variable("var2",'l', 2.71, char_place_holder);
    printf("LOG: set_to_var ok \n");
 
    // ===== ARRAY =====
    declare_array("arr0", 's', 7);
    declare_array("arr1", 'i', 5);
    declare_array("arr2", 's', 4);
    declare_array("arr3", 'l', 6);
    printf("LOG: decl_array ok \n");
 
    set_to_array("arr0",'s', 0, place_holder, 'f');
    set_to_array("arr1", 'i', 1, 77, place_holder);
    set_to_array("arr2", 's', 2, place_holder, 'c');
    set_to_array("arr2", 's', 3, place_holder, 'i');
    set_to_array("arr3", 'l', 0, 7.21 , char_place_holder);
    printf("LOG: set_to_array ok \n");
 
    // ===== MATRICI =====
    declare_matrix("matr0", 'i', 5, 5);
    declare_matrix("matr1", 's', 3, 3);
    declare_matrix("matr2", 'l', 4, 8);
    printf("LOG: declare_matrix ok \n");
 
    set_to_matrix("matr0", 'i', 0, 0,  42,  '\0');
    set_to_matrix("matr0", 'i', 1, 2,  99,  '\0');
    set_to_matrix("matr0", 'i', 4, 4, -7,   '\0');
 
    set_to_matrix("matr1", 's', 0, 0,  0,   'X');
    set_to_matrix("matr1", 's', 1, 1,  0,   'Y');
    set_to_matrix("matr1", 's', 2, 2,  0,   'Z');
 
    set_to_matrix("matr2", 'l', 0, 0,  3.14, '\0');
    set_to_matrix("matr2", 'l', 3, 7,  9.99, '\0');
    printf("LOG: set_to_matrix ok \n");
 
    // ===== GET_INDEX - VARIABILI =====
    int   *ptr0 = (int   *)get_index("&i&var0&");
    char  *ptr1 = (char  *)get_index("&c&var1&");
    float *ptr2 = (float *)get_index("&l&var2&");
    printf("LOG: get_index var ok \n");
 
    // ===== GET_INDEX - ARRAY =====
    char  *ptr3 = (char  *)get_index("&s[0]&arr0&");
    int   *ptr4 = (int   *)get_index("&i[1]&arr1&");
    float *ptr5 = (float *)get_index("&l[0]&arr3&");
    char  *ptr6 = (char  *)get_index("&s[2]&arr2&");
    char  *ptr7 = (char  *)get_index("&s[&i&var0&]&arr2&");  // var0=3 -> arr2[3]='i'
    printf("LOG: get_index array ok \n");
 
    // ===== GET_INDEX - MATRICI =====
    int   *mptr0 = (int   *)get_index("&i[0][0]&matr0&");    // 42
    int   *mptr1 = (int   *)get_index("&i[1][2]&matr0&");    // 99
    int   *mptr2 = (int   *)get_index("&i[4][4]&matr0&");    // -7
 
    char  *mptr3 = (char  *)get_index("&s[0][0]&matr1&");    // X
    char  *mptr4 = (char  *)get_index("&s[1][1]&matr1&");    // Y
    char  *mptr5 = (char  *)get_index("&s[2][2]&matr1&");    // Z
 
    float *mptr6 = (float *)get_index("&l[0][0]&matr2&");    // 3.14
    float *mptr7 = (float *)get_index("&l[3][7]&matr2&");    // 9.99
 
    // var0(=3) come indice di riga -> matr0[3][1] = 55
    set_to_matrix("matr0", 'i', 3, 1, 55, '\0');
    int   *mptr8 = (int   *)get_index("&i[&i&var0&][1]&matr0&");  // 55
    printf("LOG: get_index matrice ok \n");
 
    // ===== OUTPUT =====
    printf("\n=== VARIABILI ===\n");
    if (ptr0) printf("var0 (int)   : %d    [atteso: 3]\n",  *ptr0);
    if (ptr1) printf("var1 (char)  : %c    [atteso: F]\n",  *ptr1);
    if (ptr2) printf("var2 (float) : %f  [atteso: 2.71]\n", *ptr2);
    else       printf("var2 (float) : NULL\n");
 
    printf("\n=== ARRAY ===\n");
    if (ptr3) printf("arr0[0] (char)      : %c    [atteso: f]\n",  *ptr3);
    if (ptr4) printf("arr1[1] (int)       : %d    [atteso: 77]\n", *ptr4);
    if (ptr5) printf("arr3[0] (float)     : %f  [atteso: 7.21]\n", *ptr5);
    else       printf("arr3[0] (float)     : NULL\n");
    if (ptr6) printf("arr2[2] (char)      : %c    [atteso: c]\n",  *ptr6);
    if (ptr7) printf("arr2[var0=3] (char) : %c    [atteso: i]\n",  *ptr7);
 
    printf("\n=== MATRICI INT ===\n");
    if (mptr0) printf("matr0[0][0]        : %d    [atteso:  42]\n", *mptr0);
    if (mptr1) printf("matr0[1][2]        : %d    [atteso:  99]\n", *mptr1);
    if (mptr2) printf("matr0[4][4]        : %d    [atteso:  -7]\n", *mptr2);
    if (mptr8) printf("matr0[var0=3][1]   : %d    [atteso:  55]\n", *mptr8);
    else        printf("matr0[var0][1]     : NULL\n");
 
    printf("\n=== MATRICI CHAR ===\n");
    if (mptr3) printf("matr1[0][0]        : %c    [atteso: X]\n", *mptr3);
    if (mptr4) printf("matr1[1][1]        : %c    [atteso: Y]\n", *mptr4);
    if (mptr5) printf("matr1[2][2]        : %c    [atteso: Z]\n", *mptr5);
 
    printf("\n=== MATRICI FLOAT ===\n");
    if (mptr6) printf("matr2[0][0]        : %f  [atteso: 3.14]\n", *mptr6);
    else        printf("matr2[0][0]        : NULL\n");
    if (mptr7) printf("matr2[3][7]        : %f  [atteso: 9.99]\n", *mptr7);
    else        printf("matr2[3][7]        : NULL\n");
 
}

void copy_file(FILE *src, FILE *dst){

    char buffer[1024];
    size_t n;

    while((n = fread(buffer, 1, sizeof(buffer), src)) > 0){

        for(size_t i = 0; i < n; i++){

            // ignora eventuali '\0'
            if(buffer[i] != '\0'){
                fputc(buffer[i], dst);
            }
        }
    }
}

void importlib(const char *libfile, const char *codefile){

    FILE *lib = fopen(libfile, "rb");
    if(!lib){
        printf("ERROR: impossibile aprire libreria %s\n", libfile);
        return;
    }

    FILE *code = fopen(codefile, "rb");
    if(!code){
        printf("ERROR: impossibile aprire codice %s\n", codefile);
        fclose(lib);
        return;
    }

    FILE *tmp = fopen(".__temp__.Zinter", "wb");
    if(!tmp){
        printf("ERROR: impossibile creare file temporaneo\n");
        fclose(lib);
        fclose(code);
        return;
    }

    copy_file(code, tmp);
    fputc('\n', tmp);
    copy_file(lib, tmp);

    fclose(lib);
    fclose(code);
    fclose(tmp);

    remove(codefile);
    rename(".__temp__.Zinter", codefile);
}

int main(int argc, char *argv[]) {
    memset(&vm, 0, sizeof(VM));

    if(argc >= 3){

        //DEBUG FUNCTION SETUP
        if(strcmp("-df",argv[1]) == 0) deb = fal;
        else if(strcmp("-dt",argv[1]) == 0) deb = tru;

        //SOURCEFILE SETUP
        const char *dot = strrchr(argv[2], '.');
        if (!dot || strcmp(dot, ".Zinter") != 0) {
            printf("ERROR: formato file non supportato\n");
            return 0;
        }
        
        //SETTING UP LIBRARIES
        if(argc >=4){
            for(int i = 3; i<argc; i++){
                if(strstr(argv[i],".Zlib")){
                    importlib(argv[i]+1,argv[2]);
                }
            }
        }
        

        
        //STARTING THE FLOW
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

        int st = system_setup();
        int ed = -1;
        for(int i = 0; i<return_state; i++){
            if(strcmp(state_stack[i].nome_function,"__start") == 0){
                ed = state_stack[i].posizione_skip;
            }
        }
        if(deb) printf("\n\nendline: %d\n\n",ed);
        if(st == -1){ printf("ERROR: see preview error from system_setup \n"); return 0;}
        else{ parse(st,ed); }
        


        free(code);
        return 0;
    }

    if(argc<3){
        printf("need to add arguments \n");
        printf("in order: ./Zinterpreter -debug file_name.Zinter -library \n");
        printf("                 ^          ^                        ^\n");
        printf("                 |          |                        |\n");
        printf("           or your compiler |                    eventual file_name.Zlib \n");
        printf("              version       |        \n");
        printf("                           -df ==debug false  \n");
        printf("                           -dt ==debug true  \n");
        printf("                           -int ==interface   \n");
        
        printf("\n");
        printf("           A test will now be executed:\n");
        run_test();
        // =====================FINE SET TEST===================
    
    }


    
    return 0;
}