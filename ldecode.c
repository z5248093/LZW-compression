#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    char** str;
    int* code;
    int size;
    int max_size;
} Dictionary;

void insert_str(Dictionary* dict, char* str) {
    int i = dict->size;
    if (i >= dict->max_size) {
        return;
    }
    dict->str[i] = malloc(sizeof(char) * strlen(str) + 1);
    dict->code[i] = (int)i;
    dict->size++;
    strcpy(dict->str[i], str);
}

void init_dictionary(Dictionary* dict, int max_size) {
    dict->max_size = max_size;
    dict->size = 0;
    dict->str = malloc(sizeof(char*) * max_size);
    dict->code = malloc(sizeof(int) * max_size);
}

int get_str_code(Dictionary* dict, char* str) {
    for (int i = 0; i < dict->size; i++) {
        if (strncmp(dict->str[i], str, strlen(str)) == 0) {
            return dict->code[i];
        }
    }
    return -1;
}

char* get_code_str(Dictionary* dict, int code, int next) {
    //must be a code in dictionary, with 2 bytes
    if (code < 0) {
        if (next < 0) {
            next += 256;
            
        }
        int to_search = ((code + 128) << 8) + next;
        return dict->str[to_search];
    }

    //single digit
    if (code >= 0 && code <= 127) {
        char* str = malloc(2 * sizeof(char));
        str[0] = code;
        str[1] = '\0';
        return str;
    }
    
    if (code >= dict->size) {
        return NULL;
    }

    return NULL;
}

void lzw_decode(Dictionary* dict, char *line, int n, FILE *output) {
    int code;
    int next;
    char prev[1000];
    char* text;
    int i;
   
    code = line[0];
    text = get_code_str(dict, code, next);
    fputs(text, output);

    for (i = 1; i < n; i++) {

        code = line[i];
        next = line[i + 1];
        
        strcpy(prev, text);
        text = get_code_str(dict, code, next);

        //if code in dictionary, read two bytes at a time, therefore i++
        if (code < 0) {
            i++;
        }
        
        //if text is NULL
        //it's a special case when code_to_search is not yet generated in dictionary
        //here (text = prev + prev[0])
        if (text != NULL) {
            sprintf(prev, "%s%c", prev, text[0]);
            fputs(text, output);
        } else {
            text = malloc(sizeof(char) * (strlen(prev) + 2));
            sprintf(text, "%s%c", prev, prev[0]);
            insert_str(dict, text);
            fputs(text, output);
            continue;
        }
        
        //if two bytes code is already in the dictionary
        //skip insertion process
        //prev = this string
        if (strlen(prev) == 2 && get_str_code(dict, prev) != -1) {
            strcpy(text, prev);
        } else {
            insert_str(dict, prev);
        }
    }    
}

int main(int argc, char *argv[]) {
    FILE *file;
    FILE *output;
    char* line = NULL;
    int length = 0;
    char ch;

    if (argc != 3) {
        printf("Usage: %s <input> <output>\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Error opening the file.\n");
        return 1;
    }

    output = fopen(argv[2], "w");

    Dictionary dict;
    init_dictionary(&dict, 32768);
    
    //read file
    while (!feof(file)) {
        ch = fgetc(file);
        
        line = (char*)realloc(line, (length + 1) * sizeof(char));
        line[length] = ch;
        length++;
    }
    line[length - 1] = '\0';

    lzw_decode(&dict, line, length - 1, output);

    //free memory
    for (int i = 0; i < dict.size; i++) {
    free(dict.str[i]);
    }

    free(dict.str);
    free(dict.code);
    free(line);

    fclose(file);
    fclose(output);
    return 0;
}