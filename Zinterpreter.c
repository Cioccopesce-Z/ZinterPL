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
#define error_char "§§"


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
};

//struct for float var
struct decl_fl_var {
    char name[max_name_lettere];
    float value;
};

//struct for char var
struct decl_var_char {
    char name[max_name_lettere];
    char value;
};


//struct for int array declaration
typedef struct {
    char name[max_name_lettere];
    int *array_int; // Array(simulato) dentro la struct
    int size;
} decl_arr;

//struct for float array declaration
typedef struct {
    char name[max_name_lettere];
    float *array_int; // Array(simulato) dentro la struct
    int size;
} decl_fl_arr;

//struct for char array declaration
typedef struct {
    char name[max_name_lettere];
    char *array_int; // Array(simulato) dentro la struct
    int size;
} decl_char_arr;


//struct for int matrix declaration
typedef struct {
    char name[max_name_lettere];
    int *data;
    int size_first;
    int size_sec;
} decl_matr;

//struct for float matrix declaration
typedef struct {
    char name[max_name_lettere];
    float *data;
    int size_first;
    int size_sec;
} decl_fl_matr;

//struct for char matrix declaration
typedef struct {
    char name[max_name_lettere];
    char *data;
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

    char param_names[8][max_name_lettere];  // nomi parametri formali
    char param_types[8];                    // tipo: 'i', 'c', 'l'
    int  param_count;
} program_state;

typedef struct {
    int base_variable_idx;
    int base_fl_variable_idx;
    int base_char_variable_idx;
} scope_frame;

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

    scope_frame scope_stack[32];
    int scope_depth;

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
#define scope_stack vm.scope_stack
#define scope_depth vm.scope_depth

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

void push_scope() {
    scope_stack[scope_depth].base_variable_idx      = variable_count;
    scope_stack[scope_depth].base_fl_variable_idx   = fl_variable_count;
    scope_stack[scope_depth].base_char_variable_idx = char_variable_count;
    scope_depth++;
    if(deb) printf("[SCOPE] push -> depth=%d base_i=%d base_c=%d base_l=%d\n",
        scope_depth, variable_count, fl_variable_count, char_variable_count);
}

void pop_scope() {
    if(scope_depth <= 0) { printf("ERROR: pop_scope su scope vuoto\n"); return; }
    scope_depth--;
    variable_count      = scope_stack[scope_depth].base_variable_idx;
    fl_variable_count   = scope_stack[scope_depth].base_fl_variable_idx;
    char_variable_count = scope_stack[scope_depth].base_char_variable_idx;
    if(deb) printf("[SCOPE] pop  -> depth=%d base_i=%d base_c=%d base_l=%d\n",
        scope_depth, variable_count, fl_variable_count, char_variable_count);
}

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
int is_arr_(const char *gname) {

    char name[32];
    strcpy(name,gname);
    char junk[24];

    if(strchr(name,'[')){
        char buffer[24];
        sscanf(name,"%[^]]]%s",junk,buffer);
        strcpy(name,buffer);
    }
    if(deb) printf("DEBUG: is_arr name: %s\n",name);

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
int is_matrix_(const char *gname) {

    char name[32];
    char junk[24];
    strcpy(name,gname);
    if(strchr(name,'[')){
        char buffer[24];
        sscanf(name,"%[^]]]%s",junk,buffer);
        sscanf(buffer,"%[^]]]%s",junk,name);
    }
    if(deb) ("DEBUG: is_matrix name: %s\n",name);

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

    static int temp_int = 0;
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
                        return &matrix[i].data[row * matrix[i].size_sec + col];
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
                        return &fl_matrix[i].data[row * matrix[i].size_sec + col];
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
                        return &char_matrix[i].data[row * matrix[i].size_sec + col];
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

void* resolve(char rtype, char *gname){
    char buffer[64];
    char name[32];
    char type[24];
    char junk[24];
    strcpy(name, gname);
    type[0] = rtype;
    type[1] = '\0';

    if(strchr(name,'[') && is_arr_(name) != 0){
        sscanf(name, "%[^]]]%s", junk, buffer);
        strcat(type, junk);
        strcat(type, "]");
        strcpy(name, buffer);  
    }
    else if(strchr(name,'[') && is_matrix_(name) != 0){
        sscanf(name, "%[^]]]%s", junk, buffer);
        strcat(type, junk);
        strcat(type, "]");     
        sscanf(buffer, "%[^]]]%s", junk, name);
        strcat(type, junk);
        strcat(type, "]");     
    }

    snprintf(buffer, sizeof(buffer), "&%s&%s&", type, name);
    if(deb) printf("DEBUG: resolve resault: start: (%s),(%s) end: %s\n",type,gname,buffer);
    
    if(strchr(type,'i') || strchr(type,'n'))
        return (int *)get_index(buffer);
    if(strchr(type,'s') || strchr(type,'k') || strchr(type,'c'))
        return (char *)get_index(buffer);
    if(strchr(type,'l'))
        return (float *)get_index(buffer); 
    if(rtype == 'v')
        return get_index(gname);
    return NULL;
}

//declare matrix variable and array
void declare_matrix(char name[], char type, int s1, int s2) {

    if (type == 'i') {
        strcpy(matrix[matrix_count].name, name);
        matrix[matrix_count].size_first = s1;
        matrix[matrix_count].size_sec   = s2;
        matrix[matrix_count].data = calloc(s1 * s2, sizeof(int));
        if (!matrix[matrix_count].data) {
            printf("ERROR: malloc fallita per matrice int '%s'\n", name);
            return;
        }
        matrix_count++;
    }

    else if (type == 'l') {
        strcpy(fl_matrix[fl_matrix_count].name, name);
        fl_matrix[fl_matrix_count].size_first = s1;
        fl_matrix[fl_matrix_count].size_sec   = s2;
        fl_matrix[fl_matrix_count].data = calloc(s1 * s2, sizeof(float));
        if (!fl_matrix[fl_matrix_count].data) {
            printf("ERROR: malloc fallita per matrice float '%s'\n", name);
            return;
        }
        fl_matrix_count++;
    }

    else if (type == 's') {
        strcpy(char_matrix[char_matrix_count].name, name);
        char_matrix[char_matrix_count].size_first = s1;
        char_matrix[char_matrix_count].size_sec   = s2;
        char_matrix[char_matrix_count].data = calloc(s1 * s2, sizeof(char));
        if (!char_matrix[char_matrix_count].data) {
            printf("ERROR: malloc fallita per matrice char '%s'\n", name);
            return;
        }
        char_matrix_count++;
    }
}

void declare_variable(char name[], char type) {

    //var int
    if(type =='i'){
        strcpy(variable[variable_count].name, name);
        variable[variable_count].value = 0;

        variable_count++;

        if(variable_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di variabili int raggiunto.\n");
        }
    }

    //var fl
    if(type =='l'){
        strcpy(fl_variable[fl_variable_count].name, name);
        fl_variable[fl_variable_count].value = 0;
        
        fl_variable_count++;

        if(fl_variable_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di variabili float raggiunto.\n");
        }
    }

    //var char
    if(type =='c'){
        strcpy(char_variable[char_variable_count].name, name);
        char_variable[char_variable_count].value = 0;

        char_variable_count++;

        if(char_variable_count >= max_number_of_var) {
            printf("Attenzione: numero massimo di variabili char raggiunto.\n");
        }
    }
}

void declare_array(char name[], char type, int size) {

    if (type == 'i') {
        strcpy(array[array_count].name, name);
        array[array_count].size     = size;
        array[array_count].array_int = calloc(size, sizeof(int));
        if (!array[array_count].array_int) {
            printf("ERROR: malloc fallita per array int '%s'\n", name);
            return;
        }
        array_count++;
    }

    else if (type == 'l') {
        strcpy(fl_array[fl_array_count].name, name);
        fl_array[fl_array_count].size     = size;
        fl_array[fl_array_count].array_int = calloc(size, sizeof(float));
        if (!fl_array[fl_array_count].array_int) {
            printf("ERROR: malloc fallita per array float '%s'\n", name);
            return;
        }
        fl_array_count++;
    }

    else if (type == 's') {
        strcpy(char_array[char_array_count].name, name);
        char_array[char_array_count].size     = size;
        char_array[char_array_count].array_int = calloc(size, sizeof(char));
        if (!char_array[char_array_count].array_int) {
            printf("ERROR: malloc fallita per array char '%s'\n", name);
            return;
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
                    matrix[i].data[row * matrix[i].size_sec + col] = (int)value;
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
                    fl_matrix[i].data[row * matrix[i].size_sec + col] = value;
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
                    char_matrix[i].data[row * matrix[i].size_sec + col] = cvalue;
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
    if (!file){ printf("ERROR: impossibile aprire il file: %s, possibile che non esista",filename); }

    
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
            void *ret = exec_funarg(base_name, fal);
            if(!ret) return;
            char rettype = return_type;
            if(rettype == 'i')      printf("%d", *(int*)ret);
            else if(rettype == 'l') printf("%f", *(float*)ret);
            else if(rettype == 'c') printf("%c", *(char*)ret);
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

int check_deven(int line) {
    if (line-1 == line_idx_program) {
        if(deb) printf("ERROR: deven outside a function %s line: %d global_ip: %d line_idx: %d\n",program[global_ip].instruction,line,global_ip,line_idx_program);
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
        sscanf(name_plus_args, "deven_%127s", buffer);   // FIX: source corretto

        if (strlen(buffer) == 0 || strcmp(buffer, "NULL") == 0) {
            if(deb) printf("WARNING: no item to deven\n");
            return_value = NULL;
            return_hit   = 1;
            return NULL;
        }
        else if (strstr(buffer, "--")) {
            if(deb) printf("WARNING: more than one item to deven\n");
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

            if (type == 'v') {
                void *inner = exec_funarg(buffer, fal);  
                return_type  = return_type;  
                return_value = inner;
                return_hit   = 1;
                return inner;
            }

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

        // ---- NUOVO: push scope ----
        push_scope();

        // ---- NUOVO: trova la entry in state_stack per questa funzione ----
        // (hai già trovato i nell'indice del loop precedente)
        // inietta i parametri come variabili locali
        if(strlen(args) > 0) {
            char args_copy[64];
            strncpy(args_copy, args, sizeof(args_copy)-1);
            char *tok = strtok(args_copy, "!");
            int pi = 0;
            while(tok && pi < state_stack[i].param_count) {
                char pname[max_name_lettere];
                strcpy(pname, state_stack[i].param_names[pi]);
                char ptype = state_stack[i].param_types[pi];

                declare_variable(pname, ptype);

                if(ptype == 'i') {
                    int val = 0;
                    if(isdigit((unsigned char)tok[0]) || tok[0] == '-') {
                        // numero diretto: 10, -5
                        val = atoi(tok);
                    } else {
                        // nome variabile: cerca tra int e float
                        int *p = (int*)resolve('i', tok);
                        if(p) val = *p;
                        else { float *pf = (float*)resolve('l', tok); if(pf) val = (int)*pf; }
                    }
                    set_to_variable(pname, 'i', val, 0);
                }
                else if(ptype == 'l') {
                    float val = 0;
                    if(isdigit((unsigned char)tok[0]) || tok[0] == '-') {
                        val = atof(tok);
                    } else {
                        float *p = (float*)resolve('l', tok);
                        if(p) val = *p;
                        else { int *pi2 = (int*)resolve('i', tok); if(pi2) val = (float)*pi2; }
                    }
                    set_to_variable(pname, 'l', val, 0);
                }
                else if(ptype == 'c') {
                    char val = 0;
                    if(tok[0] == '\'' && tok[2] == '\'') {
                        // char letterale: 'x'
                        val = tok[1];
                    } else {
                        char *p = (char*)resolve('c', tok);
                        if(p) val = *p;
                    }
                    set_to_variable(pname, 'c', 0, val);
                }

                pi++;
                tok = strtok(NULL, "!");
            }
        }
        // ---- parse esistente ----
        return_hit   = 0;
        return_value = NULL;
        parse(st_ip, end_ip);

        // ---- NUOVO: pop scope ----
        pop_scope();

        // dopo parse il valore è in return_value; reset il flag per il chiamante
        void *ret = return_value;
        return_hit   = 0;
        return_value = NULL;

        global_ip = svaed_ip;

        if(deb) printf("correctly went out parse from funarg, return: %p\n", ret);
        return ret;
    }
}
//if(deb) 
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
    if(ltype=='i')      lopv = *(int*)resolve(ltype,lop);
    else if(ltype=='n') lopv = *(int*)resolve(ltype,lop);
    else if(ltype=='l') lopv = (int)*(float*)resolve(ltype,lop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",lop); }

    if(strchr(rop,'&')) {
        sscanf(rop, "&%c&%23[^&]&", &rtype, rop);
    } else {
        char crop[24]; strcpy(crop,rop); is_what(crop);
        sscanf(crop, "%15[^.].%c", junk, &rtype);
    }
    if(rtype=='i')      ropv = *(int   *)resolve(rtype,rop);
    else if(rtype=='n') ropv = *(int*)resolve(rtype,rop);
    else if(rtype=='l') ropv = (int)*(float*)resolve(rtype,rop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",rop); }

    if(!types_match(ltype,rtype)) { printf("ERROR: type mismatch in math_plus\n"); return error_int; }
    if(deb) printf("DEBUG: math_plus return: %d = %d + %d\n",lopv+ropv,lopv,ropv);
    return lopv+ropv;
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
    else if(ltype=='n') lopv = *(int*)resolve(ltype,lop);
    else if(ltype=='l') lopv = (int)*(float*)resolve(ltype,lop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",lop); }

    if(strchr(rop,'&')) {
        sscanf(rop, "&%c&%23[^&]&", &rtype, rop);
    } else {
        char crop[24]; strcpy(crop,rop); is_what(crop);
        sscanf(crop, "%15[^.].%c", junk, &rtype);
    }
    if(rtype=='i')      ropv = *(int   *)resolve(rtype,rop);
    else if(rtype=='n') ropv = *(int*)resolve(rtype,rop);
    else if(rtype=='l') ropv = (int)*(float*)resolve(rtype,rop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",rop); }

    if(!types_match(ltype,rtype)) { printf("ERROR: type mismatch in math_min\n"); return error_int; }
    if(deb) printf("DEBUG: math_min return: %d = %d - %d\n",lopv-ropv,lopv,ropv);
    return lopv-ropv;
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
    else if(ltype=='n') lopv = *(int*)resolve(ltype,lop);
    else if(ltype=='l') lopv = (int)*(float*)resolve(ltype,lop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",lop); }

    if(strchr(rop,'&')) {
        sscanf(rop, "&%c&%23[^&]&", &rtype, rop);
    } else {
        char crop[24]; strcpy(crop,rop); is_what(crop);
        sscanf(crop, "%15[^.].%c", junk, &rtype);
    }
    if(rtype=='i')      ropv = *(int   *)resolve(rtype,rop);
    else if(rtype=='n') ropv = *(int*)resolve(rtype,rop);
    else if(rtype=='l') ropv = (int)*(float*)resolve(rtype,rop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",rop); }

    if(!types_match(ltype,rtype)) { printf("ERROR: type mismatch in math_times\n"); return error_int; }
    if(deb) printf("DEBUG: math_plus return: %d = %d * %d\n",lopv*ropv,lopv,ropv);
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
    else if(ltype=='n') lopv = *(int*)resolve(ltype,lop);
    else if(ltype=='l') lopv = (int)*(float*)resolve(ltype,lop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",lop); }

    if(strchr(rop,'&')) {
        sscanf(rop, "&%c&%23[^&]&", &rtype, rop);
    } else {
        char crop[24]; strcpy(crop,rop); is_what(crop);
        sscanf(crop, "%15[^.].%c", junk, &rtype);
    }
    if(rtype=='i')      ropv = *(int   *)resolve(rtype,rop);
    else if(rtype=='n') ropv = *(int*)resolve(rtype,rop);
    else if(rtype=='l') ropv = (int)*(float*)resolve(rtype,rop);
    else { printf("WARNING: cant operate arithmetically with char %s\n",rop); }

    if(!types_match(ltype,rtype)) { printf("ERROR: type mismatch in math_slash\n"); return error_int; }
    if(ropv == 0) { printf("ERROR: division by zero\n"); return error_int; }
    if(deb) printf("DEBUG: math_plus return: %d = %d / %d\n",lopv/ropv,lopv,ropv);
    return lopv/ropv;
}

int is_math(char *operand) {
    if(deb) printf("DEBUG: is_math chiamata con %s\n", operand);

    if(strchr(operand,'+')) return math_plus (operand, fal);
    if(strchr(operand,'-')) return math_min  (operand, fal);
    if(strchr(operand,'*')) return math_times(operand, fal);
    if(strchr(operand,'/')) return math_slash(operand, fal);

    return error_int;  // non è math
}

int dimconf(char *rname, char *lname, char *datatype, char type, char *op){
    if(deb) printf("DEBUG: confronto di dimensione di array o matrici non inficizzate\n");

    int rdim = 0, ldim = 0;
    //potrei ridurre della meta i cicli ma sono le 2 di mattina e non ne ho voglia
    if(strcmp(datatype,"matrix") == 0){

        if(type == 'i'){
            for(int i = 0; i < matrix_count; i++){ //resolve rightdim
                if(strcmp(rname,matrix[i].name) == 0){
                    rdim = matrix[i].size_first * matrix[i].size_sec;
                    break;
                }
            }
            
            for(int i = 0; i < matrix_count; i++){ //resolve leftdim
                if(strcmp(lname,matrix[i].name) == 0){
                    ldim = matrix[i].size_first * matrix[i].size_sec;
                    break;
                }
            }
        }

        if(type == 'l'){
            for(int i = 0; i < fl_matrix_count; i++){ //resolve rightdim
                if(strcmp(rname,fl_matrix[i].name) == 0){
                    rdim = fl_matrix[i].size_first * fl_matrix[i].size_sec;
                    break;
                }
            }
            
            for(int i = 0; i < fl_matrix_count; i++){ //resolve leftdim
                if(strcmp(lname,fl_matrix[i].name) == 0){
                    ldim = fl_matrix[i].size_first * fl_matrix[i].size_sec;
                    break;
                }
            }
        }

        if(type == 's'){
            for(int i = 0; i < char_matrix_count; i++){ //resolve rightdim
                if(strcmp(rname,char_matrix[i].name) == 0){
                    rdim = char_matrix[i].size_first * char_matrix[i].size_sec;
                    break;
                }
            }
            
            for(int i = 0; i < char_matrix_count; i++){ //resolve leftdim
                if(strcmp(lname,char_matrix[i].name) == 0){
                    ldim = char_matrix[i].size_first * char_matrix[i].size_sec;
                    break;
                }
            }
        }
    }

    else if(strcmp(datatype,"array") == 0){
            if(type == 'i'){
                for(int i = 0; i < array_count; i++){ //resolve rightdim
                    if(strcmp(rname,array[i].name) == 0){
                        rdim = array[i].size;
                        break;
                    }
                }
                
                for(int i = 0; i < array_count; i++){ //resolve leftdim
                    if(strcmp(lname,array[i].name) == 0){
                        ldim = array[i].size;
                        break;
                    }
                }
            }

            if(type == 'l'){
                for(int i = 0; i < fl_array_count; i++){ //resolve rightdim
                    if(strcmp(rname,fl_array[i].name) == 0){
                        rdim = fl_array[i].size;
                        break;
                    }
                }
                
                for(int i = 0; i < fl_array_count; i++){ //resolve leftdim
                    if(strcmp(lname,fl_array[i].name) == 0){
                        ldim = fl_array[i].size;
                        break;
                    }
                }
            }

            if(type == 's'){
                for(int i = 0; i < char_array_count;i++){ //resolve rightdim
                    if(strcmp(rname,char_array[i].name) == 0){
                        rdim = char_array[i].size;
                        break;
                    }
                }
                
                for(int i = 0; i < char_array_count; i++){ //resolve leftdim
                    if(strcmp(lname,char_array[i].name) == 0){
                        ldim = char_array[i].size;
                        break;
                    }
                }
            }
        }
        

        if(strcmp(op,"==") == 0){
            if(ldim == rdim) return tru;
        }
        else if(strcmp(op,"^=") == 0){
            if(ldim != rdim) return tru;
        }
        else if(strcmp(op,"<") == 0){
            if(ldim < rdim) return tru;
        }
        else if(strcmp(op,">") == 0){
            if(ldim > rdim) return tru;
        }
        else if(strcmp(op,"<<") == 0){
            if(ldim <= rdim) return tru;
        }
        else if(strcmp(op,">>") == 0){
            if(ldim >= rdim) return tru;
        }
        return fal;
    
}

int itotalconf(int dest, int src, char *op){

    if(strcmp(op,"==") == 0){
        if(dest == src) return tru;
    }
    else if(strcmp(op,"^=") == 0){
        if(dest != src) return tru;
    }
    else if(strcmp(op,"<") == 0){
        if(dest < src) return tru;
    }
    else if(strcmp(op,">") == 0){
        if(dest > src) return tru;
    }
    else if(strcmp(op,"<<") == 0){
        if(dest <= src) return tru;
    }
    else if(strcmp(op,">>") == 0){
        if(dest >= src) return tru;
    }
    return fal;
}

int ltotalconf(float dest, float src, char *op){

    if(strcmp(op,"==") == 0){
        if(dest == src) return tru;
    }
    else if(strcmp(op,"^=") == 0){
        if(dest != src) return tru;
    }
    else if(strcmp(op,"<") == 0){
        if(dest < src) return tru;
    }
    else if(strcmp(op,">") == 0){
        if(dest > src) return tru;
    }
    else if(strcmp(op,"<<") == 0){
        if(dest <= src) return tru;
    }
    else if(strcmp(op,">>") == 0){
        if(dest >= src) return tru;
    }
    return fal;
}

int stotalconf(char dest, char src, char *op){

    if(strcmp(op,"==") == 0){
        if(dest == src) return tru;
    }
    else if(strcmp(op,"^=") == 0){
        if(dest != src) return tru;
    }
    else if(strcmp(op,"<") == 0){
        if(dest < src) return tru;
    }
    else if(strcmp(op,">") == 0){
        if(dest > src) return tru;
    }
    else if(strcmp(op,"<<") == 0){
        if(dest <= src) return tru;
    }
    else if(strcmp(op,">>") == 0){
        if(dest >= src) return tru;
    }
    return fal;
}

static int split_operands(const char *text, const char *op, char *left, char *right) {
    const char *pos = strstr(text, op);
    if (!pos) return 0;
    int llen = (int)(pos - text);
    strncpy(left,  text, llen); left[llen] = '\0';
    strncpy(right, pos + strlen(op), 31); right[31] = '\0';
    return 1;
}

int exec_conf(char *text,char *op){

    char left_operand[32] = {0};
    char right_operand[32] = {0};

    // NUOVO
    if (!split_operands(text, op, left_operand, right_operand)) {
        printf("ERROR: parse error in exec_conf: op='%s' text='%s'\n", op, text);
        return error_int;
    }
    if(deb) printf("CONF DEBUG: text: %s op: %s, left: %s, right: %s\n",
                text, op, left_operand, right_operand);

    char left_name[64]  = {0};
    char right_name[64] = {0};
    char left_i[16]     = {0};
    char right_i[16]    = {0};
    char left_j[16]     = {0};
    char right_j[16]    = {0};


    /*MATRIX*/
    if (strstr(left_operand, "][") && strstr(right_operand, "][")) { /*matr x matr*/

        if (left_operand[0]=='[' && left_operand[1]==']' && left_operand[2]=='[' && left_operand[3]==']' &&
                right_operand[0]=='[' && right_operand[1]==']' && right_operand[2]=='[' && right_operand[3]==']') {
                char ln[64]={0}, rn[64]={0};
                sscanf(left_operand,  "[][]%63s", ln);
                sscanf(right_operand, "[][]%63s", rn);
                char bin[16]; char ltdata[16]={0}; char ltype=0;
                strcpy(bin, ln); is_what(bin);
                sscanf(bin, "%15[^.].%c", ltdata, &ltype);
                if(strcmp(ltdata,"matrix")!=0){ printf("ERROR: %s is not a matrix\n",ln); return error_int; }
                return dimconf(rn, ln, "matrix", ltype, op);
        }

        sscanf(left_operand,  "[%[^]]][%[^]]]%63s", left_i,  left_j,  left_name);
        sscanf(right_operand, "[%[^]]][%[^]]]%63s", right_i, right_j, right_name);
        if(deb) printf("[CHECK] matr x matr | left: [%s][%s]%s | right: [%s][%s]%s\n",
               left_i, left_j, left_name, right_i, right_j, right_name);
        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        char rtdata[16] = {0}; char rtype = 0;

        strcpy(bin, left_name);  is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "matrix") != 0) { printf("ERROR: %s is not a matrix\n", left_name); return error_int; }

        strcpy(bin, right_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", rtdata, &rtype);
        if (strcmp(rtdata, "matrix") != 0) { printf("ERROR: %s is not a matrix\n", right_name); return error_int; }

        if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return error_int; }

        int li = resolve_index(left_i),  lj = resolve_index(left_j);
        int ri = resolve_index(right_i), rj = resolve_index(right_j);
        if (li < 0 || lj < 0 || ri < 0 || rj < 0) return error_int;

        char lenc[128], renc[128];
        snprintf(lenc, sizeof(lenc), "&%c[%d][%d]&%s&", ltype, li, lj, left_name);
        snprintf(renc, sizeof(renc), "&%c[%d][%d]&%s&", rtype, ri, rj, right_name);

        if (ltype == 'i') {
            int *dest = (int *)get_index(lenc);
            int *src  = (int *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
            return itotalconf(*dest,*src,op);
        } else if (ltype == 'l') {
            float *dest = (float *)get_index(lenc);
            float *src  = (float *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
            return ltotalconf(*dest,*src,op);
        } else if (ltype == 's') {
            char *dest = (char *)get_index(lenc);
            char *src  = (char *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
            return stotalconf(*dest,*src,op); 
        }
    }

    else if (strstr(left_operand, "][") && !strstr(right_operand, "][")) { /*matr x boh*/
        sscanf(left_operand, "[%[^]]][%[^]]]%63s", left_i, left_j, left_name);

        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        strcpy(bin, left_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "matrix") != 0) { printf("ERROR: %s is not a matrix\n", left_name); return error_int; }

        int li = resolve_index(left_i), lj = resolve_index(left_j);
        if (li < 0 || lj < 0) return error_int;

        char lenc[128];
        snprintf(lenc, sizeof(lenc), "&%c[%d][%d]&%s&", ltype, li, lj, left_name);

        int math_res = is_math(right_operand);
        if(math_res != error_int) {
            if(deb) printf("[CHECK] matr x math | left: [%s][%s]%s | right: %s\n", left_i, left_j, left_name, right_operand);
            if(ltype=='i'){
                int *dest = (int*)get_index(lenc);
                if(!dest){ printf("ERROR: null pointer\n"); return error_int; }
                *dest = math_res;
            } else if(ltype=='l'){
                float *dest = (float*)get_index(lenc);
                if(!dest){ printf("ERROR: null pointer\n"); return error_int; }
                *dest = (float)math_res;
            }
            return error_int;
        }

        else if (right_operand[0] == '\'') { /*matr x k*/
            if(deb) printf("[CHECK] matr x k | left: [%s][%s]%s | right: %s\n",
                   left_i, left_j, left_name, right_operand);
            if (!types_match(ltype, 'k')) { printf("ERROR: type mismatch matrix/char\n"); return error_int; }
            char *dest = (char *)get_index(lenc);
            if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
            *dest = right_operand[1];
        }
        else if (isdigit((unsigned char)right_operand[0])) { /*matr x n*/
            if(deb) printf("[CHECK] matr x n | left: [%s][%s]%s | right: %s\n",
                   left_i, left_j, left_name, right_operand);
            if (!types_match(ltype, 'n')) { printf("ERROR: type mismatch matrix/number\n"); return error_int; }
            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
                *dest = atoi(right_operand);
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
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
            if (strcmp(rtdata, "array") != 0) { printf("ERROR: %s is not an array\n", right_name); return error_int; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return error_int; }

            int ri = resolve_index(right_i);
            if (ri < 0) return error_int;
            char renc[128];
            snprintf(renc, sizeof(renc), "&%c[%d]&%s&", rtype, ri, right_name);

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                int *src  = (int *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return itotalconf(*dest, *src, op);
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                float *src  = (float *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return ltotalconf(*dest, *src, op);
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                char *src  = (char *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return stotalconf(*dest, *src, op);
            }
        }
        else if (strstr(right_operand, "__")) { /*matr x func*/
            if(deb) printf("[CHECK] matr x func | left: [%s][%s]%s | right: %s\n",
                left_i, left_j, left_name, right_operand);
           if (!types_match(ltype, 'v')) { printf("ERROR: type mismatch matrix/function\n"); return error_int; }

            void *ret = get_index(right_operand);
            if (!ret) { printf("ERROR: null pointer from function\n"); return error_int; }

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
                *dest = *(int *)ret;
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
                *dest = *(float *)ret;
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
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
            if (strcmp(rtdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", right_name); return error_int; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return error_int; }

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                int *src  = (int *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                if(*dest == *src) return tru; else return fal;
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                float *src  = (float *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                if(*dest == *src) return tru; else return fal;
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                char *src  = (char *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                if(*dest == *src) return tru; else return fal;
            }
        }
    }

    else if (!strstr(left_operand, "][") && strstr(right_operand, "][")) { /*boh x matr*/
        sscanf(right_operand, "[%[^]]][%[^]]]%63s", right_i, right_j, right_name);

        char bin[16];
        char rtdata[16] = {0}; char rtype = 0;
        strcpy(bin, right_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", rtdata, &rtype);
        if (strcmp(rtdata, "matrix") != 0) { printf("ERROR: %s is not a matrix\n", right_name); return error_int; }

        int ri = resolve_index(right_i), rj = resolve_index(right_j);
        if (ri < 0 || rj < 0) return error_int;
        char renc[128];
        snprintf(renc, sizeof(renc), "&%c[%d][%d]&%s&", rtype, ri, rj, right_name);

        if (strchr(left_operand, ']')) { /*arr x matr*/
            sscanf(left_operand, "[%[^]]]%63s", left_i, left_name);
            if(deb) printf("[CHECK] arr x matr | left: [%s]%s | right: [%s][%s]%s\n",
                   left_i, left_name, right_i, right_j, right_name);

            char ltdata[16] = {0}; char ltype = 0;
            strcpy(bin, left_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", ltdata, &ltype);
            if (strcmp(ltdata, "array") != 0) { printf("ERROR: %s is not an array\n", left_name); return error_int; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return error_int; }

            int li = resolve_index(left_i);
            if (li < 0) return error_int;
            char lenc[128];
            snprintf(lenc, sizeof(lenc), "&%c[%d]&%s&", ltype, li, left_name);

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                int *src  = (int *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return itotalconf(*dest, *src, op);
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                float *src  = (float *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return ltotalconf(*dest, *src, op);
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                char *src  = (char *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return stotalconf(*dest, *src, op);
            }
        }
        else { /*var x matr*/
            sscanf(left_operand, "%63s", left_name);
            if(deb) printf("[CHECK] var x matr | left: %s | right: [%s][%s]%s\n",
                   left_name, right_i, right_j, right_name);

            char ltdata[16] = {0}; char ltype = 0;
            strcpy(bin, left_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", ltdata, &ltype);
            if (strcmp(ltdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", left_name); return error_int; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return error_int; }

            if (ltype == 'i') {
                int *dest = (int *)resolve(ltype, left_name);
                int *src  = (int *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return itotalconf(*dest, *src, op);
            } else if (ltype == 'l') {
                float *dest = (float *)resolve(ltype, left_name);
                float *src  = (float *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return ltotalconf(*dest, *src, op);
            } else if (ltype == 'c') {
                char *dest = (char *)resolve(ltype, left_name);
                char *src  = (char *)get_index(renc);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return stotalconf(*dest, *src, op);
            }
        }
    }

    /*ARRAY*/
    else if (strchr(left_operand, ']') && strchr(right_operand, ']')) { /*arr x arr*/

            if (left_operand[0]=='[' && left_operand[1]==']' &&
                right_operand[0]=='[' && right_operand[1]==']') {
                char ln[64]={0}, rn[64]={0};
                sscanf(left_operand,  "[]%63s", ln);
                sscanf(right_operand, "[]%63s", rn);
                char bin[16]; char ltdata[16]={0}; char ltype=0;
                strcpy(bin, ln); is_what(bin);
                sscanf(bin, "%15[^.].%c", ltdata, &ltype);
                if(strcmp(ltdata,"array")!=0){ printf("ERROR: %s is not an array\n",ln); return error_int; }
                return dimconf(rn, ln, "array", ltype, op);
            }

        sscanf(left_operand,  "[%[^]]]%63s", left_i,  left_name);
        sscanf(right_operand, "[%[^]]]%63s", right_i, right_name);
        if(deb) printf("[CHECK] arr x arr | left: [%s]%s | right: [%s]%s\n",
               left_i, left_name, right_i, right_name);

        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        char rtdata[16] = {0}; char rtype = 0;

        strcpy(bin, left_name);  is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "array") != 0) { printf("ERROR: %s is not an array\n", left_name); return error_int; }

        strcpy(bin, right_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", rtdata, &rtype);
        if (strcmp(rtdata, "array") != 0) { printf("ERROR: %s is not an array\n", right_name); return error_int; }

        if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return error_int; }

        int li = resolve_index(left_i);
        int ri = resolve_index(right_i);
        if (li < 0 || ri < 0) return error_int;

        char lenc[128], renc[128];
        snprintf(lenc, sizeof(lenc), "&%c[%d]&%s&", ltype, li, left_name);
        snprintf(renc, sizeof(renc), "&%c[%d]&%s&", rtype, ri, right_name);

        if (ltype == 'i') {
            int *dest = (int *)get_index(lenc);
            int *src  = (int *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
            return itotalconf(*dest, *src, op);
        } else if (ltype == 'l') {
            float *dest = (float *)get_index(lenc);
            float *src  = (float *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
            return ltotalconf(*dest, *src, op);
        } else if (ltype == 's' || ltype == 'c') {
            char *dest = (char *)get_index(lenc);
            char *src  = (char *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
            return stotalconf(*dest, *src, op);
        }   
    }

    else if (strchr(left_operand, ']') && !strchr(right_operand, ']')) { /*arr x boh*/
        sscanf(left_operand, "[%[^]]]%63s", left_i, left_name);

        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        strcpy(bin, left_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "array") != 0) { printf("ERROR: %s is not an array\n", left_name); return error_int; }

        int li = resolve_index(left_i);
        if (li < 0) return error_int;
        char lenc[128];
        snprintf(lenc, sizeof(lenc), "&%c[%d]&%s&", ltype, li, left_name);

        int math_res = is_math(right_operand);
        if(math_res != error_int) {
            if(deb) printf("[CHECK] arr x math | left: [%s]%s | right: %s\n", left_i, left_name, right_operand);
            if(ltype=='i'){
                int *dest = (int*)get_index(lenc);
                if(!dest){ printf("ERROR: null pointer\n"); return error_int; }
                *dest = math_res;
            } else if(ltype=='l'){
                float *dest = (float*)get_index(lenc);
                if(!dest){ printf("ERROR: null pointer\n"); return error_int; }
                *dest = (float)math_res;
            }
            return error_int;
        }
        else if (right_operand[0] == '\'') { /*arr x k*/
            if(deb) printf("[CHECK] arr x k | left: [%s]%s | right: %s\n", left_i, left_name, right_operand);
            if (!types_match(ltype, 'k')) { printf("ERROR: type mismatch array/char\n"); return error_int; }
            char *dest = (char *)get_index(lenc);
            if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
            *dest = right_operand[1];
        }
        else if (isdigit((unsigned char)right_operand[0])) { /*arr x n*/
            if(deb) printf("[CHECK] arr x n | left: [%s]%s | right: %s\n", left_i, left_name, right_operand);
            if (!types_match(ltype, 'n')) { printf("ERROR: type mismatch array/number\n"); return error_int; }
            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
                *dest = atoi(right_operand);
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
                *dest = (float)atof(right_operand);
            }
        }
        else if (strstr(right_operand, "__")) { /*arr x func*/
            if(deb) printf("[CHECK] arr x func | left: [%s]%s | right: %s\n",
                left_i, left_name, right_operand);
                if (!types_match(ltype, 'v')) { printf("ERROR: type mismatch array/function\n"); return error_int; }

            void *ret = get_index(right_operand);
            if (!ret) { printf("ERROR: null pointer from function\n"); return error_int; }

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
                *dest = *(int *)ret;
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
                *dest = *(float *)ret;
            } else if (ltype == 's') {
                char *dest = (char *)get_index(lenc);
                if (!dest) { printf("ERROR: null pointer\n"); return error_int; }
                *dest = *(char *)ret;
            }
        }
        else { /*arr x var*/
            sscanf(right_operand, "%63s", right_name);
            if(deb) printf("[CHECK] arr x var | left: [%s]%s | right: %s\n", left_i, left_name, right_name);

            char rtdata[16] = {0}; char rtype = 0;
            strcpy(bin, right_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", rtdata, &rtype);
            if (strcmp(rtdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", right_name); return error_int; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return error_int; }

            if (ltype == 'i') {
                int *dest = (int *)get_index(lenc);
                int *src  = (int *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return itotalconf(*dest, *src, op);
            } else if (ltype == 'l') {
                float *dest = (float *)get_index(lenc);
                float *src  = (float *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return ltotalconf(*dest, *src, op);
            } else if (ltype == 's' || ltype == 'c') {
                char *dest = (char *)get_index(lenc);
                char *src  = (char *)resolve(rtype, right_name);
                if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
                return stotalconf(*dest, *src, op);
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
        if (strcmp(ltdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", left_name); return error_int; }

        strcpy(bin, right_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", rtdata, &rtype);
        if (strcmp(rtdata, "array") != 0) { printf("ERROR: %s is not an array\n", right_name); return error_int; }

        if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return error_int; }

        int ri = resolve_index(right_i);
        if (ri < 0) return error_int;
        char renc[128];
        snprintf(renc, sizeof(renc), "&%c[%d]&%s&", rtype, ri, right_name);

        if (ltype == 'i') {
            int *dest = (int *)resolve(ltype, left_name);
            int *src  = (int *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
            return itotalconf(*dest, *src, op);
        } else if (ltype == 'l') {
            float *dest = (float *)resolve(ltype, left_name);
            float *src  = (float *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
            return ltotalconf(*dest, *src, op);
        } else if (ltype == 's' || ltype == 'c') {
            char *dest = (char *)resolve(ltype, left_name);
            char *src  = (char *)get_index(renc);
            if (!dest || !src) { printf("ERROR: null pointer\n"); return error_int; }
            return stotalconf(*dest, *src, op);
        }
    }

    /*VARIABILI*/
    else if (!strchr(left_operand, ']') && (!strchr(right_operand, ']') || strstr(right_operand,"__")) ) {

        sscanf(left_operand, "%63s", left_name);

        char bin[16];
        char ltdata[16] = {0}; char ltype = 0;
        strcpy(bin, left_name); is_what(bin);
        sscanf(bin, "%15[^.].%c", ltdata, &ltype);
        if (strcmp(ltdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", left_name); return error_int; }

        /* Risolve il puntatore sinistro una volta sola */
        void *dest_ptr = resolve(ltype, left_name);
        if (!dest_ptr) { printf("ERROR: null pointer\n"); return error_int; }

        int math_res = is_math(right_operand);
        if (math_res != error_int) { /*var x math*/
            if(deb) printf("[CHECK] var x math | left: %s | right: %s\n", left_name, right_operand);
            if (!types_match(ltype, 'n')) { printf("ERROR: type mismatch var/math\n"); return error_int; }
            if      (ltype == 'i') return itotalconf(*(int   *)dest_ptr, (int)math_res,   op);
            else if (ltype == 'l') return ltotalconf(*(float *)dest_ptr, (float)math_res, op);
        }

        else if (right_operand[0] == '\'') { /*var x k*/
            if(deb) printf("[CHECK] var x k | left: %s | right: %s\n", left_name, right_operand);
            if (!types_match(ltype, 'k')) { printf("ERROR: type mismatch var/char\n"); return error_int; }
            char *dest = (char *)dest_ptr;
            return stotalconf(*dest, right_operand[1], op);
        }

        else if (isdigit((unsigned char)right_operand[0])) { /*var x n*/
            if(deb) printf("[CHECK] var x n | left: %s | right: %s\n", left_name, right_operand);
            if (!types_match(ltype, 'n')) { printf("ERROR: type mismatch var/number\n"); return error_int; }
            if      (ltype == 'i') return itotalconf(*(int   *)dest_ptr, atoi(right_operand),          op);
            else if (ltype == 'l') return ltotalconf(*(float *)dest_ptr, (float)atof(right_operand),   op);
        }

        else if (strstr(right_operand, "__")) { /*var x func*/
            sscanf(right_operand, "%63s", right_name);
            if(deb) printf("[CHECK] var x func | left: %s | right: %s\n", left_name, right_name);

            char rtdata[16] = {0}; char rtype = 0;
            strcpy(bin, right_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", rtdata, &rtype);
            if (strcmp(rtdata, "function") != 0) { printf("ERROR: %s is not a function\n", right_name); return error_int; }
            if (!types_match(ltype, 'v')) { printf("ERROR: type mismatch var/function\n"); return error_int; }

            void *ret = get_index(right_name);
            if (!ret) { printf("ERROR: null pointer from function\n"); return error_int; }

            if      (ltype == 'i') return itotalconf(*(int   *)dest_ptr, *(int   *)ret, op);
            else if (ltype == 'l') return ltotalconf(*(float *)dest_ptr, *(float *)ret, op);
            else if (ltype == 'c') return stotalconf(*(char  *)dest_ptr, *(char  *)ret, op);
        }

        else { /*var x var*/
            sscanf(right_operand, "%63s", right_name);
            if(deb) printf("[CHECK] var x var | left: %s | right: %s\n", left_name, right_name);

            char rtdata[16] = {0}; char rtype = 0;
            strcpy(bin, right_name); is_what(bin);
            sscanf(bin, "%15[^.].%c", rtdata, &rtype);
            if (strcmp(rtdata, "variable") != 0) { printf("ERROR: %s is not a variable\n", right_name); return error_int; }
            if (!types_match(ltype, rtype)) { printf("ERROR: type mismatch\n"); return error_int; }

            void *src_ptr = resolve(rtype, right_name);
            if (!src_ptr) { printf("ERROR: null pointer\n"); return error_int; }

            if      (ltype == 'i') return itotalconf(*(int   *)dest_ptr, *(int   *)src_ptr, op);
            else if (ltype == 'l') return ltotalconf(*(float *)dest_ptr, *(float *)src_ptr, op);
            else if (ltype == 'c') return stotalconf(*(char  *)dest_ptr, *(char  *)src_ptr, op);
        }
    }
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


void *exec_plus_plus(char *text) {

    char lop[16] = {0};
    char rop[16] = {0};
    int has_rop = (sscanf(text, "%15[^+]++%15s", lop, rop) == 2);
    if (!has_rop)
        sscanf(text, "%15[^+]++", lop);

    // tipo di lop
    char buffer[16], junk[16]; char type = 0;
    strcpy(buffer, lop);
    is_what(buffer);
    sscanf(buffer, "%15[^.].%c", junk, &type);
    if(deb) printf("DEBUG: plusplus lop=%s type=%c has_rop=%d rop=%s\n", lop, type, has_rop, rop);

    // valore di rop (default 1)
    int ropv = 1;
    if (has_rop) {
        char rbuffer[16]; char rtype = 0;
        strcpy(rbuffer, rop);
        is_what(rbuffer);
        sscanf(rbuffer, "%15[^.].%c", junk, &rtype);
        int *rptr = (int *)resolve(rtype, rop);
        if (!rptr) { printf("ERROR: cannot resolve rop '%s' in ++\n", rop); return pt_place_holder; }
        ropv = *rptr;
    }

    // INT: somma diretta
    if (type == 'i') {
        int *dest = (int *)resolve(type, lop);
        if (!dest) { printf("ERROR: resolve failed for '%s' in ++\n", lop); return pt_place_holder; }
        *dest += ropv;
        return dest;
    }

    // CHAR ARRAY: copia arr[src] in arr[src+ropv]
    else if (type == 's') {
        char idx_str[8] = {0}, arr_name[16] = {0};
        if (sscanf(lop, "[%7[^]]]%15s", idx_str, arr_name) != 2) {
            printf("ERROR: cannot parse array index from '%s' in ++\n", lop);
            return pt_char_place_holder;
        }
        int src_idx = resolve_index(idx_str);
        int dst_idx = src_idx + ropv;

        char buf_src[32], buf_dst[32];
        snprintf(buf_src, sizeof(buf_src), "&s[%d]&%s&", src_idx, arr_name);
        snprintf(buf_dst, sizeof(buf_dst), "&s[%d]&%s&", dst_idx, arr_name);

        char *src = (char *)get_index(buf_src);
        char *dst = (char *)get_index(buf_dst);
        if (!src || !dst) { printf("ERROR: out of bounds in ++ char array '%s' [%d]->[%d]\n", arr_name, src_idx, dst_idx); return pt_char_place_holder; }
        *dst = *src;
        return src;
    }

    else {
        printf("ERROR: type mismatch on ++ operation, line: %s\n", text);
        return pt_char_place_holder;
    }
}

void *exec_min_min(char *text) {

    char lop[16] = {0};
    char rop[16] = {0};
    int has_rop = (sscanf(text, "%15[^-]--%15s", lop, rop) == 2);
    if (!has_rop)
        sscanf(text, "%15[^-]--", lop);

    // tipo di lop
    char buffer[16], junk[16]; char type = 0;
    strcpy(buffer, lop);
    is_what(buffer);
    sscanf(buffer, "%15[^.].%c", junk, &type);
    if(deb) printf("DEBUG: minmin lop=%s type=%c has_rop=%d rop=%s\n", lop, type, has_rop, rop);

    // valore di rop (default 1)
    int ropv = 1;
    if (has_rop) {
        char rbuffer[16]; char rtype = 0;
        strcpy(rbuffer, rop);
        is_what(rbuffer);
        sscanf(rbuffer, "%15[^.].%c", junk, &rtype);
        int *rptr = (int *)resolve(rtype, rop);
        if (!rptr) { printf("ERROR: cannot resolve rop '%s' in --\n", rop); return pt_place_holder; }
        ropv = *rptr;
    }

    // INT: sottrazione diretta
    if (type == 'i') {
        int *dest = (int *)resolve(type, lop);
        if (!dest) { printf("ERROR: resolve failed for '%s' in --\n", lop); return pt_place_holder; }
        *dest -= ropv;
        return dest;
    }

    // CHAR ARRAY: copia arr[src] in arr[src-ropv]
    else if (type == 's') {
        char idx_str[8] = {0}, arr_name[16] = {0};
        if (sscanf(lop, "[%7[^]]]%15s", idx_str, arr_name) != 2) {
            printf("ERROR: cannot parse array index from '%s' in --\n", lop);
            return pt_char_place_holder;
        }
        int src_idx = resolve_index(idx_str);
        int dst_idx = src_idx - ropv;

        if (dst_idx < 0) {
            printf("ERROR: -- underflow su char array '%s' [%d - %d]\n", arr_name, src_idx, ropv);
            return pt_char_place_holder;
        }

        char buf_src[32], buf_dst[32];
        snprintf(buf_src, sizeof(buf_src), "&s[%d]&%s&", src_idx, arr_name);
        snprintf(buf_dst, sizeof(buf_dst), "&s[%d]&%s&", dst_idx, arr_name);

        char *src = (char *)get_index(buf_src);
        char *dst = (char *)get_index(buf_dst);
        if (!src || !dst) { printf("ERROR: out of bounds in -- char array '%s' [%d]->[%d]\n", arr_name, src_idx, dst_idx); return pt_char_place_holder; }
        *dst = *src;
        return src;
    }

    else {
        printf("ERROR: type mismatch on -- operation, line: %s\n", text);
        return pt_char_place_holder;
    }
}

size_t get_memory_usage() {

    size_t total = 0;

    // VM statica
    total += sizeof(VM);

    // heap malloc array int
    for(int i = 0; i < array_count; i++)
        total += array[i].size * sizeof(int);

    // heap malloc array float
    for(int i = 0; i < fl_array_count; i++)
        total += fl_array[i].size * sizeof(float);

    // heap malloc array char
    for(int i = 0; i < char_array_count; i++)
        total += char_array[i].size * sizeof(char);

    // heap malloc matrix int
    for(int i = 0; i < matrix_count; i++)
        total += matrix[i].size_first * matrix[i].size_sec * sizeof(int);

    // heap malloc matrix float
    for(int i = 0; i < fl_matrix_count; i++)
        total += fl_matrix[i].size_first * fl_matrix[i].size_sec * sizeof(float);

    // heap malloc matrix char
    for(int i = 0; i < char_matrix_count; i++)
        total += char_matrix[i].size_first * char_matrix[i].size_sec * sizeof(char);

    // stringa codice sorgente (se la vuoi tracciare passa la size come parametro)
    // total += code_size;

    return total;
}

int exec_status(char *text) {

    size_t mem = get_memory_usage();
    printf("memoria stimata : %zu B | %zu KB | %zu MB\n", mem, mem/1024, mem/1024/1024);

    if(strstr(text,"clear") || strstr(text,"clr") || strstr(text,"cls")) {

        int confirmed = 0;

        if(strchr(text,'y')) {
            confirmed = 1;
        } else {
            char dec;
            printf("WARNING: this operation will erase all variables/arrays/matrices. Are you sure? (y/n) ");
            scanf("%c", &dec);
            if(dec == 'y') confirmed = 1;
        }

        if(!confirmed) return 0;

        for(int i = 0; i < array_count; i++)
            if(array[i].array_int) { free(array[i].array_int); array[i].array_int = NULL; }

        for(int i = 0; i < fl_array_count; i++)
            if(fl_array[i].array_int) { free(fl_array[i].array_int); fl_array[i].array_int = NULL; }

        for(int i = 0; i < char_array_count; i++)
            if(char_array[i].array_int) { free(char_array[i].array_int); char_array[i].array_int = NULL; }

        for(int i = 0; i < matrix_count; i++)
            if(matrix[i].data) { free(matrix[i].data); matrix[i].data = NULL; }

        for(int i = 0; i < fl_matrix_count; i++)
            if(fl_matrix[i].data) { free(fl_matrix[i].data); fl_matrix[i].data = NULL; }

        for(int i = 0; i < char_matrix_count; i++)
            if(char_matrix[i].data) { free(char_matrix[i].data); char_matrix[i].data = NULL; }

        memset(variable,      0, sizeof(variable));
        memset(fl_variable,   0, sizeof(fl_variable));
        memset(char_variable, 0, sizeof(char_variable));
        memset(array,         0, sizeof(array));
        memset(fl_array,      0, sizeof(fl_array));
        memset(char_array,    0, sizeof(char_array));
        memset(matrix,        0, sizeof(matrix));
        memset(fl_matrix,     0, sizeof(fl_matrix));
        memset(char_matrix,   0, sizeof(char_matrix));
        memset(r,             0, sizeof(r));
        memset(cr,            0, sizeof(cr));

        variable_count = 0;    fl_variable_count = 0;   char_variable_count = 0;
        array_count = 0;       fl_array_count = 0;      char_array_count = 0;
        matrix_count = 0;      fl_matrix_count = 0;     char_matrix_count = 0;

        printf("LOG: memory cleared\n");
    }

    return 0;
}

void has_condition(char operation[]){

    if(deb) printf("DEBUG: entrato nell'has_condition\n");
    char text[128] = {0};
    strcpy(text,operation);

    char buffer[64] = {0};
    strcpy(buffer,operation);
    is_what(buffer);

    if( strstr(program[global_ip].instruction,"==") ||
                strstr(program[global_ip].instruction,"^=") ||
                strstr(program[global_ip].instruction,">>") ||
                strstr(program[global_ip].instruction,"<<") ||
                (strchr(program[global_ip].instruction,'>') && !strstr(program[global_ip].instruction,">>")) ||
                (strchr(program[global_ip].instruction,'<') && !strstr(program[global_ip].instruction,"<<")) ){

                    if(strstr(text,">>")) strcpy(operation,">>");
                    else if(strstr(text,"<<")) strcpy(operation,"<<");
                    else if(strstr(text,"==")) strcpy(operation,"==");
                    else if(strstr(text,"^=")) strcpy(operation,"^=");
                    else if(strchr(text,'>'))  strcpy(operation,">");
                    else if(strchr(text,'<'))  strcpy(operation,"<");
                    if(deb) printf("DEBUG: entrato nel ramo con == > < in has condition\n");
                    strcat(operation,".has");
    }

    else if(strcmp(buffer,"-1.-1") != 0){
        strcpy(operation,"dirdata");
        if(deb) printf("DEBUG: entrato nel ramo funzione/dirdata di has_condition\n");
    }

    else{
        if(deb) printf("DEBUG: entrato nel ramo non condition di has_condition\n");
        strcpy(operation,"no_condition_or_data"); return;
    }

    if(strstr(operation,"||") && strstr(operation,"et") ){
        if(deb) printf("DEBUG: entrato nel ramo adding di has_condition\n");
        strcat(operation,".adding");
    }
    if(deb) printf("DEBUG: has_condition return:%s\n",operation);
    return;
}

int is_int(char varname[]){

    char dtype[15] = {0}, type[15] = {0};
    is_what(varname);
    if(sscanf(varname,"%14[^.].%14s",dtype,type) == 2 && strcmp(type,"i") == 0) return tru;
    else return fal;

}

char type_of_var(char text[]){
    char buffer[16];
    strcpy(buffer,text);
    is_what(buffer);
    char junk[8], type[8];
    sscanf(buffer,"%[^.].%s",junk,type);
    if(strcmp(type,"-1") == 0 ){
        printf("ERROR: no type found for data: %s\n",text);
        return 'F';
    }
    else{
        if(deb) printf("DEBUG: type of %s to return is %s (type_of_var)\n",text,type);
        return type[0];
    }
}

void exec_if(char text[]){
    
    if(deb) printf("DEBUG: entrato nell'exec_if %s\n",text);
    char condition[64] = {0};
    int n = sscanf(text,"if(%[^)])",condition);
    if( n == 0) n = sscanf(text,"othif(%[^)])",condition);

    if(deb) printf("DEBUG: condition found as: %s in exec_if\n",condition);

    char buffer[64] = {0};
    strcpy(buffer,condition);
    has_condition(buffer);

    if(strcmp(buffer,"no_condition_or_data") == 0){
        printf("ERROR: invalid argument for if function");
        return;
    }

    int current_ip = global_ip, i = 0, position_in_ret_state = 0;

    
    while(i < return_state){
        if(state_stack[i].posizione_ritorno == current_ip){
            position_in_ret_state = i;
            break;
        }
        i++;
    }
    
    
    i = position_in_ret_state; if(i == 0){ printf("ERROR: condition in line %d: not found check for eventual misspelled in %s\n",global_ip,text); return;}

    if(deb) printf("DEBUG: condition: %s found with start: %d e end %d\n",
            state_stack[i].nome_function,state_stack[i].posizione_ritorno,state_stack[i].posizione_skip);

    //=======cerca se ci sono condizioni successive======
    int nex_st = 0;
    int nex_end = 0;
    int all_condition_end = 0;
    char nex_tx[64] = {0};

    int f = i; f++;
    if(strcmp(state_stack[f].nome_function,"oth") == 0 
        && (state_stack[f].posizione_ritorno == (state_stack[i].posizione_skip)+1) //f = i+1
        &&  strcmp(state_stack[f+1].nome_function,"othif") == 0){               //f= i+2

            printf("ERROR: construction if/othif oth othif obscure %s %s %s with state_stack[%d]\n",
                state_stack[i].nome_function,state_stack[f].nome_function,
                state_stack[f+1].nome_function,i);
            return;
    }

    if(strcmp(state_stack[f].nome_function,"othif") == 0){
        strcpy(nex_tx,program[state_stack[f].posizione_ritorno].instruction);
        nex_st = state_stack[f].posizione_ritorno;
        nex_end = state_stack[f].posizione_skip;

    }
    else if(strcmp(state_stack[f].nome_function,"oth") == 0){
        strcpy(nex_tx,program[state_stack[f].posizione_ritorno].instruction);
        nex_st = state_stack[f].posizione_ritorno;
        nex_end = state_stack[f].posizione_skip;
        all_condition_end = nex_end;
    }

    //========CONTROLLA SE ESEGUIRE=========

    if(strstr(buffer,"dirdata")){
        strcpy(buffer,condition);

        if(is_int(buffer)){
            if(deb) printf("DEBUG: condition is dirdata with state_stack[%d]\n",i);
            //controlla contenutono se = 0 fal se =1 tru
            int *value = (int*)resolve(type_of_var(condition),condition); //use resolve
            if(value == NULL){ printf("ERROR: to find var %s for get_index in exec_if\n",condition); return;}
            

            //vedi se eseguire solo if e saltare l ultima condizione

            if(*value == 1){
                //condizione vera
                if(deb) printf("DEBUG: condizione dirdata %s vera\n",condition);
                parse(state_stack[i].posizione_ritorno+1,state_stack[i].posizione_skip);
                global_ip = state_stack[i].posizione_skip;
                return;

            }

            else{
                if(deb) printf("DEBUG: condizione %s falsa\n",condition);

                if(strstr(nex_tx,"othif")){
                    global_ip = nex_st;
                    exec_if(program[state_stack[f].posizione_ritorno].instruction);
                    return;
                }
                else{
                    global_ip = nex_st;
                    parse(nex_st+1,nex_end);
                    return;
                }
            }
        } 
        //fa comparazioni or e and 
        else if(strstr(buffer,"adding")){
            if(deb) printf("DEBUG: ci sono adding in exec_if dirdata\n");
            //contiene || o nd
        }
        else{
            if(deb) printf("ERROR: if cannot operate with type other than integer %s in %s\n",condition,text); return;
        }
    }

    else if(strstr(buffer,".has")){

        if(deb) printf("DEBUG: condizione da parsare con conf in exec_if buffer:%s \n",buffer);

        if(strstr(buffer,"adding")){
            if(deb) printf("DEBUG: ci sono adding in exec_if complex conf\n");
            //contiene || o nd
        }
        else{
            char op[3] = {0};
            sscanf(buffer,"%2[^.].",op);
            if(deb) printf("DEBUG: condition in exex_if: %s e op: %s\n",condition,op);
            int res = exec_conf(condition,op);
            if(res){

                //condizione
                if(deb) printf("DEBUG: condizione conf %s vera\n",condition);
                parse(state_stack[i].posizione_ritorno+1,state_stack[i].posizione_skip);
                global_ip = state_stack[i].posizione_skip;
                return;
                
            }
            else{
                
                //condizione falsa
                if(strstr(nex_tx,"othif")){
                    global_ip = nex_st;
                    exec_if(program[state_stack[f].posizione_ritorno].instruction);
                    return;
                }
                else{
                    global_ip = nex_st;
                    parse(nex_st+1,nex_end);
                    return;
                }
            }
        }
    }
}

void skip_to_end(){
    int current_ip = global_ip;
    int i = 0;
    while(i < return_state){
        if(state_stack[i].posizione_ritorno == current_ip){
            global_ip = state_stack[i].posizione_skip;
            break;
        }
        i++;
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
        else if( starts_with (program[global_ip].instruction, "status_") ){ exec_status(program[global_ip].instruction);/*stampa carattere a terminale */ }
        else if( starts_with (program[global_ip].instruction, "lnprint_") ){ exec_lnprint(program[global_ip].instruction);/*va a capo stampa carattere a terminale */ }
        else if( starts_with (program[global_ip].instruction, "println_") ){ exec_println(program[global_ip].instruction);/*stampa carattere a terminale e va a capo */ }
        else if( starts_with (program[global_ip].instruction, "lnprintln_") ){ exec_lnprintln(program[global_ip].instruction);/*va a capo stampa carattere a terminale e va a capo */ }
        else if( starts_with (program[global_ip].instruction, "#") ){ /*funzioni di sistema e interprete tipo import o kill*/ }
        else if( starts_with (program[global_ip].instruction, "od_") ){ /*dichiara funzione od (open door) ma non ha gran senso*/ }
        else if( starts_with (program[global_ip].instruction, "__") ){ exec_funarg(program[global_ip].instruction, fal); }
        else if( starts_with (program[global_ip].instruction, "int_") ){ exec_int(program[global_ip].instruction); /*dichiara var int  */ }
        else if( starts_with (program[global_ip].instruction, "char_") ){ exec_char(program[global_ip].instruction); /*dichiara var char */ }
        else if( starts_with (program[global_ip].instruction, "othif") ){ skip_to_end();/*else if*/ }
        else if( starts_with (program[global_ip].instruction, "if") ){ exec_if(program[global_ip].instruction); /*inizia if_ */ }
        else if( starts_with (program[global_ip].instruction, "oth") ){ skip_to_end();/*else */ }
        else if( starts_with (program[global_ip].instruction, "for") ){/*inizia for */ }
        else if( starts_with (program[global_ip].instruction, "while") ){/*inizia while */ }
        else if( strstr(program[global_ip].instruction,"==") ||
                strstr(program[global_ip].instruction,"^=") ||
                strstr(program[global_ip].instruction,">>") ||
                strstr(program[global_ip].instruction,"<<") ||
                (strchr(program[global_ip].instruction,'>') && !strstr(program[global_ip].instruction,">>")) ||
                (strchr(program[global_ip].instruction,'<') && !strstr(program[global_ip].instruction,"<<")) )
                {
                    char detected_op[3] = {0};
                    const char *ins = program[global_ip].instruction;
                    if     (strstr(ins,">>")) strcpy(detected_op,">>");
                    else if(strstr(ins,"<<")) strcpy(detected_op,"<<");
                    else if(strstr(ins,"==")) strcpy(detected_op,"==");
                    else if(strstr(ins,"^=")) strcpy(detected_op,"^=");
                    else if(strchr(ins,'>'))  strcpy(detected_op,">");
                    else if(strchr(ins,'<'))  strcpy(detected_op,"<");
                    exec_conf(program[global_ip].instruction, detected_op);
                }
        else if ( starts_with(program[global_ip].instruction, "deven_") && check_deven(eventual_end_line) == tru ) {
            exec_funarg(program[global_ip].instruction, tru); /* return_hit è ora 1 quindi il while esce al prossimo controllo */}
        else if( starts_with (program[global_ip].instruction, "set_to_") ){ exec_set_to(program[global_ip].instruction); }
        else if( strchr (program[global_ip].instruction, '=') ){ exec_equal(program[global_ip].instruction);}
        else if( strstr (program[global_ip].instruction, "++") ){ exec_plus_plus(program[global_ip].instruction);/*operazione ++ di sicuro perche cicli gia parsati*/}
        else if( strstr (program[global_ip].instruction, "--") ){ exec_min_min(program[global_ip].instruction); /*operazione -- di sicuro perche cicli gia parsati*/}
        else if( starts_with (program[global_ip].instruction, "scan_") ){/*legge carattere da schermo */ }
        global_ip++;
    }

    return_hit = fal;
}

//costruisce un id con inizio fine nome e proprieta di ogni funzione con { }
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
            if( starts_with(program[i].instruction,"othif") ) strcpy(state_stack[return_state].nome_function, "othif");
            else if( starts_with(program[i].instruction,"if") ) strcpy(state_stack[return_state].nome_function, "if");
            else if( starts_with(program[i].instruction,"for") ) strcpy(state_stack[return_state].nome_function, "for_");
            else if( starts_with(program[i].instruction,"while") ) strcpy(state_stack[return_state].nome_function, "while");
            else if( starts_with(program[i].instruction,"oth") ) strcpy(state_stack[return_state].nome_function, "oth");
            else if( starts_with(program[i].instruction,"__start") ) strcpy(state_stack[return_state].nome_function, "__start");
            else if( starts_with(program[i].instruction,"od_") ){
                char bin_name[16] = {0};
                char param_str[64] = {0};
                sscanf(program[i].instruction, "od_%15[^(](%63[^)])", bin_name, param_str);

                char full_name[24];
                sprintf(full_name, "od_%s", bin_name);
                strcpy(state_stack[return_state].nome_function, full_name);

                // parsing parametri formali separati da !
                // formato atteso: i&nome!c&nome2!l&nome3
                int pc = 0;
                char tmp[64];
                strncpy(tmp, param_str, sizeof(tmp)-1);
                char *tok = strtok(tmp, "!");
                while(tok && pc < 8) {
                    char ptype; char pname[max_name_lettere];
                    if(sscanf(tok, "&%c&%15[^&]&", &ptype, pname) == 2) {
                        state_stack[return_state].param_types[pc] = ptype;
                        strcpy(state_stack[return_state].param_names[pc], pname);
                        pc++;
                    }
                    tok = strtok(NULL, "!");
                }
                state_stack[return_state].param_count = pc;
                if(deb) printf("BUILD DEBUG: funzione %s con %d parametri\n", full_name, pc);
            }
            else if( starts_with(program[i].instruction,"#") ) strcpy(state_stack[return_state].nome_function, "#"); 
            else if( starts_with(program[i].instruction,"C") ) strcpy(state_stack[return_state].nome_function, "C");
            state_stack[return_state].posizione_skip = j;

            return_state++;
        }

        if(strstr(program[i].instruction,"//"));
        i++;
    }
}
void run_test(){

    int pass = 0;
    int fail = 0;

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

    // ===== GET_INDEX - ARRAY =====
    char  *ptr3 = (char  *)get_index("&s[0]&arr0&");
    int   *ptr4 = (int   *)get_index("&i[1]&arr1&");
    float *ptr5 = (float *)get_index("&l[0]&arr3&");
    char  *ptr6 = (char  *)get_index("&s[2]&arr2&");
    char  *ptr7 = (char  *)get_index("&s[&i&var0&]&arr2&");  // var0=3 -> arr2[3]='i'

    // ===== GET_INDEX - MATRICI =====
    int   *mptr0 = (int   *)get_index("&i[0][0]&matr0&");
    int   *mptr1 = (int   *)get_index("&i[1][2]&matr0&");
    int   *mptr2 = (int   *)get_index("&i[4][4]&matr0&");

    char  *mptr3 = (char  *)get_index("&s[0][0]&matr1&");
    char  *mptr4 = (char  *)get_index("&s[1][1]&matr1&");
    char  *mptr5 = (char  *)get_index("&s[2][2]&matr1&");

    float *mptr6 = (float *)get_index("&l[0][0]&matr2&");
    float *mptr7 = (float *)get_index("&l[3][7]&matr2&");

    set_to_matrix("matr0", 'i', 3, 1, 55, '\0');
    int   *mptr8 = (int   *)get_index("&i[&i&var0&][1]&matr0&");  // var0=3 -> matr0[3][1]=55

    printf("\n=== VARIABILI ===\n");
    if (ptr0 && *ptr0 == 3)   { printf("var0 (int)   : %d    [ok]\n",  *ptr0); pass++; }
    else                      { printf("var0 (int)   : %d    [FAIL atteso 3]\n", ptr0 ? *ptr0 : -999); fail++; }

    if (ptr1 && *ptr1 == 'F') { printf("var1 (char)  : %c    [ok]\n",  *ptr1); pass++; }
    else                      { printf("var1 (char)  : %c    [FAIL atteso F]\n", ptr1 ? *ptr1 : '?'); fail++; }

    if (ptr2 && *ptr2 > 2.70 && *ptr2 < 2.72) { printf("var2 (float) : %f  [ok]\n", *ptr2); pass++; }
    else                      { printf("var2 (float) : %f  [FAIL atteso 2.71]\n", ptr2 ? *ptr2 : -999.0); fail++; }

    printf("\n=== ARRAY ===\n");
    if (ptr3 && *ptr3 == 'f') { printf("arr0[0] (char)      : %c    [ok]\n",  *ptr3); pass++; }
    else                      { printf("arr0[0] (char)      : %c    [FAIL atteso f]\n", ptr3 ? *ptr3 : '?'); fail++; }

    if (ptr4 && *ptr4 == 77)  { printf("arr1[1] (int)       : %d    [ok]\n",  *ptr4); pass++; }
    else                      { printf("arr1[1] (int)       : %d    [FAIL atteso 77]\n", ptr4 ? *ptr4 : -999); fail++; }

    if (ptr5 && *ptr5 > 7.20 && *ptr5 < 7.22) { printf("arr3[0] (float)     : %f  [ok]\n", *ptr5); pass++; }
    else                      { printf("arr3[0] (float)     : %f  [FAIL atteso 7.21]\n", ptr5 ? *ptr5 : -999.0); fail++; }

    if (ptr6 && *ptr6 == 'c') { printf("arr2[2] (char)      : %c    [ok]\n",  *ptr6); pass++; }
    else                      { printf("arr2[2] (char)      : %c    [FAIL atteso c]\n", ptr6 ? *ptr6 : '?'); fail++; }

    if (ptr7 && *ptr7 == 'i') { printf("arr2[var0=3] (char) : %c    [ok]\n",  *ptr7); pass++; }
    else                      { printf("arr2[var0=3] (char) : %c    [FAIL atteso i]\n", ptr7 ? *ptr7 : '?'); fail++; }

    printf("\n=== MATRICI INT ===\n");
    if (mptr0 && *mptr0 == 42)  { printf("matr0[0][0]        : %d    [ok]\n", *mptr0); pass++; }
    else                        { printf("matr0[0][0]        : %d    [FAIL atteso 42]\n", mptr0 ? *mptr0 : -999); fail++; }

    if (mptr1 && *mptr1 == 99)  { printf("matr0[1][2]        : %d    [ok]\n", *mptr1); pass++; }
    else                        { printf("matr0[1][2]        : %d    [FAIL atteso 99]\n", mptr1 ? *mptr1 : -999); fail++; }

    if (mptr2 && *mptr2 == -7)  { printf("matr0[4][4]        : %d    [ok]\n", *mptr2); pass++; }
    else                        { printf("matr0[4][4]        : %d    [FAIL atteso -7]\n", mptr2 ? *mptr2 : -999); fail++; }

    if (mptr8 && *mptr8 == 55)  { printf("matr0[var0=3][1]   : %d    [ok]\n", *mptr8); pass++; }
    else                        { printf("matr0[var0=3][1]   : %d    [FAIL atteso 55]\n", mptr8 ? *mptr8 : -999); fail++; }

    printf("\n=== MATRICI CHAR ===\n");
    if (mptr3 && *mptr3 == 'X') { printf("matr1[0][0]        : %c    [ok]\n", *mptr3); pass++; }
    else                        { printf("matr1[0][0]        : %c    [FAIL atteso X]\n", mptr3 ? *mptr3 : '?'); fail++; }

    if (mptr4 && *mptr4 == 'Y') { printf("matr1[1][1]        : %c    [ok]\n", *mptr4); pass++; }
    else                        { printf("matr1[1][1]        : %c    [FAIL atteso Y]\n", mptr4 ? *mptr4 : '?'); fail++; }

    if (mptr5 && *mptr5 == 'Z') { printf("matr1[2][2]        : %c    [ok]\n", *mptr5); pass++; }
    else                        { printf("matr1[2][2]        : %c    [FAIL atteso Z]\n", mptr5 ? *mptr5 : '?'); fail++; }

    printf("\n=== MATRICI FLOAT ===\n");
    if (mptr6 && *mptr6 > 3.13 && *mptr6 < 3.15) { printf("matr2[0][0]        : %f  [ok]\n", *mptr6); pass++; }
    else                        { printf("matr2[0][0]        : %f  [FAIL atteso 3.14]\n", mptr6 ? *mptr6 : -999.0); fail++; }

    if (mptr7 && *mptr7 > 9.98 && *mptr7 < 10.0) { printf("matr2[3][7]        : %f  [ok]\n", *mptr7); pass++; }
    else                        { printf("matr2[3][7]        : %f  [FAIL atteso 9.99]\n", mptr7 ? *mptr7 : -999.0); fail++; }

    printf("\n=== CONFRONTO BINARIO (exec_conf) ===\n");
    *ptr6 = 'f';   // rende arr2[2]='f' uguale ad arr0[0]='f' per i test ==
    int c1 = exec_conf("[0]arr0==[2]arr2","==");
    int c2 = exec_conf("[0]arr0^=[2]arr2","^=");
    int c3 = exec_conf("[]arr0>>[]arr2",">>");
    int c4 = exec_conf("[]arr0<<[]arr2","<<");

    if (c1 == 1) { printf("arr0[0] arr2[2] char ==   : %d  [ok]\n", c1); pass++; }
    else         { printf("arr0[0] arr2[2] char ==   : %d  [FAIL atteso 1]\n", c1); fail++; }

    if (c2 == 0) { printf("arr0[0] arr2[2] char ^=   : %d  [ok]\n", c2); pass++; }
    else         { printf("arr0[0] arr2[2] char ^=   : %d  [FAIL atteso 0]\n", c2); fail++; }

    if (c3 == 1) { printf("arr0[] >> arr2[] dimconf  : %d  [ok]\n", c3); pass++; }
    else         { printf("arr0[] >> arr2[] dimconf  : %d  [FAIL atteso 1]\n", c3); fail++; }

    if (c4 == 0) { printf("arr0[] << arr2[] dimconf  : %d  [ok]\n", c4); pass++; }
    else         { printf("arr0[] << arr2[] dimconf  : %d  [FAIL atteso 0]\n", c4); fail++; }

    printf("\n=== CONFRONTO VAR x N ===\n");
    int c5 = exec_conf("var0==3","==");
    int c6 = exec_conf("var0>1",">");
    int c7 = exec_conf("var0<1","<");
    int c8 = exec_conf("var0^=3","^=");

    if (c5 == 1) { printf("var0(3) == 3              : %d  [ok]\n", c5); pass++; }
    else         { printf("var0(3) == 3              : %d  [FAIL atteso 1]\n", c5); fail++; }

    if (c6 == 1) { printf("var0(3) >  1              : %d  [ok]\n", c6); pass++; }
    else         { printf("var0(3) >  1              : %d  [FAIL atteso 1]\n", c6); fail++; }

    if (c7 == 0) { printf("var0(3) <  1              : %d  [ok]\n", c7); pass++; }
    else         { printf("var0(3) <  1              : %d  [FAIL atteso 0]\n", c7); fail++; }

    if (c8 == 0) { printf("var0(3) ^= 3              : %d  [ok]\n", c8); pass++; }
    else         { printf("var0(3) ^= 3              : %d  [FAIL atteso 0]\n", c8); fail++; }

    printf("\n=== ARITMETICA (math_*) ===\n");
    int m1 = math_plus ("3+4",  fal);
    int m2 = math_min  ("9-4",  fal);
    int m3 = math_times("3*4",  fal);
    int m4 = math_slash("10/2", fal);
    // errore atteso: divisione per zero
    int m5 = math_slash("5/0",  fal);
    // errore atteso: numero puro non e' char, math_plus con char non ha senso
    // (queste due righe servono solo a vedere che non crashano)
    int m6 = math_plus("3+0",  fal);

    if (m1 == 7)          { printf("3 + 4                     : %d  [ok]\n", m1); pass++; }
    else                  { printf("3 + 4                     : %d  [FAIL atteso 7]\n", m1); fail++; }

    if (m2 == 5)          { printf("9 - 4                     : %d  [ok]\n", m2); pass++; }
    else                  { printf("9 - 4                     : %d  [FAIL atteso 5]\n", m2); fail++; }

    if (m3 == 12)         { printf("3 * 4                     : %d  [ok]\n", m3); pass++; }
    else                  { printf("3 * 4                     : %d  [FAIL atteso 12]\n", m3); fail++; }

    if (m4 == 5)          { printf("10 / 2                    : %d  [ok]\n", m4); pass++; }
    else                  { printf("10 / 2                    : %d  [FAIL atteso 5]\n", m4); fail++; }

    if (m5 == error_int)  { printf("5 / 0  -> div by zero     : error_int  [ok]\n"); pass++; }
    else                  { printf("5 / 0  -> div by zero     : %d  [FAIL atteso error_int]\n", m5); fail++; }

    if (m6 == 3)          { printf("3 + 0                     : %d  [ok]\n", m6); pass++; }
    else                  { printf("3 + 0                     : %d  [FAIL atteso 3]\n", m6); fail++; }

    printf("\n========================================\n");
    printf("TOTALE: %d ok,  %d falliti  su %d test\n", pass, fail, pass + fail);
    if (fail == 0) printf("build OK\n");
    else           printf("ATTENZIONE: %d test falliti, verificare la build\n", fail);
    printf("========================================\n");
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

    FILE *tmp = fopen(".__temp__.Zim", "wb");
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
    rename(".__temp__.Zim", codefile);
}

int main(int argc, char *argv[]) {
    memset(&vm, 0, sizeof(VM));

    if(argc >= 3){

        FILE *origin = fopen(argv[2],"r");
        FILE *copybflib = fopen("source_no_lib","w");

        //DEBUG FUNCTION SETUP
        if(strcmp("-df",argv[1]) == 0) deb = fal;
        else if(strcmp("-dt",argv[1]) == 0) deb = tru;

        //SOURCEFILE SETUP
        const char *dot = strrchr(argv[2], '.');
        if (!dot || strcmp(dot, ".Zim") != 0) {
            printf("ERROR: formato file non supportato\n");
            return 0;
        }
        
        //SETTING UP LIBRARIES
        int has_libs = 0;  // Flag per sapere se hai usato librerie
        if(argc >= 4){
            copy_file(origin, copybflib);
            has_libs = 1;
            for(int i = 3; i<argc; i++){
                if(strstr(argv[i],".Zlib")){
                    importlib(argv[i]+1, argv[2]);
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

        // CHIUDI I FILE
        fclose(origin);
        fclose(copybflib);

        // CLEANUP SOLO SE HAI USATO LIBRERIE
        if(has_libs) {
            if(remove(argv[2]) == 0) {
                if(rename("source_no_lib", argv[2]) == 0) {
                    if(deb) printf("File aggiornato con successo (librerie integrate e rimosse)\n");
                } else {
                    printf("ERROR: impossibile rinominare source_no_lib\n");
                }
            } else {
                printf("ERROR: impossibile cancellare file originale\n");
            }
        } else {
            remove("source_no_lib");  // Rimuovi il file vuoto
        }

        free(code);
        return 0;
    }

    if(argc<3){
        printf("need to add arguments \n");
        printf("in order: ./Zinterpreter -debug file_name.Zim -library \n");
        printf("                 ^          ^                        ^\n");
        printf("                 |          |                        |\n");
        printf("           or your compiler |                    eventual file_name.Zlib \n");
        printf("              version       |        \n");
        printf("                           -df ==debug false  \n");
        printf("                           -dt ==debug true  \n");
        printf("\n");
        printf("           A test will now be executed:\n");
        run_test();
    }

    return 0;
}