#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define HASH_TABLE_SIZE 32768

typedef struct DictionaryEntry {
    char* str;
    int code;
    struct DictionaryEntry* next;
} DictionaryEntry;

typedef struct {
    DictionaryEntry** table;
    int size;
    int max_size;
} Dictionary;

//hash table to improve dictionary lookup
unsigned int hash_function(char* str) {
    unsigned int hash = 0;
    int c;

    while ((c = *str++)) {
        hash = (hash * 31) + c;
    }

    return hash;
}

void insert_str(Dictionary* dict, char* str) {
    if (dict->size == dict->max_size) {
        return;
    }
    DictionaryEntry* entry = malloc(sizeof(DictionaryEntry));
    entry->str = malloc(sizeof(char) * (strlen(str) + 1));
    strcpy(entry->str, str);
    entry->code = dict->size;
    entry->next = NULL;

    int index = hash_function(str) % HASH_TABLE_SIZE;

    if (dict->table[index] == NULL) {
        dict->table[index] = entry;
    } else {
        DictionaryEntry* current = dict->table[index];
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = entry;
    }

    dict->size++;
}

void init_dictionary(Dictionary* dict, int max_size) {
    dict->max_size = max_size;
    dict->size = 0;
    dict->table = malloc(sizeof(DictionaryEntry*) * HASH_TABLE_SIZE);
    memset(dict->table, 0, sizeof(DictionaryEntry*) * HASH_TABLE_SIZE);
}

int get_str_code(Dictionary* dict, char* str) {
    int index = hash_function(str) % HASH_TABLE_SIZE;
    DictionaryEntry* entry = dict->table[index];

    while (entry != NULL) {
        if (strcmp(entry->str, str) == 0) {
            return entry->code;
        }
        entry = entry->next;
    }

    return -1;
}

void lzw_encode(Dictionary* dict, char* text, FILE *output) {
    char current[1000];
    char next;
    int code;
    int i = 0;
    int length = 1;

    while (i < strlen(text)) {

        sprintf(current, "%c", text[i]);
        next = text[i + 1];

        //the last digit
        if (i + 1 == strlen(text)) {
            fputs(current, output);
            return;
        }

        //the last two digits
        if (i + 2 == strlen(text)) {

            fputs(current, output);
            fputc(next, output);

            return;
        }

        //if single character or already inside the dictionary,
        //add the next character together with current
        while (length == 1 || get_str_code(dict, current) != -1) {
            sprintf(current, "%s%c", current, next);
            length++;
            i++;
            next = text[i + 1];
        }

        //cut the last digit, the strlen of current is (length - 1)
        current[strlen(current) - 1] = '\0';
        next = text[i];

        //get code only if the strlen(current) is greater than 1
        if (length == 2) {
            code = (int)current[0];
        } else {
            code = get_str_code(dict, current);
        }

        //output the dictionary position only if the length is larger than 3
        if (length <= 3) {
            fputs(current, output);
        } else {
            fputc(128 + (code >> 8), output);
            fputc(code, output);
        }

        //add the next digit and put into the dictionary
        sprintf(current, "%s%c", current, next);
        insert_str(dict, current);

        length = 1;
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

    lzw_encode(&dict, line, output);
    
    //free memory
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        DictionaryEntry* entry = dict.table[i];
        while (entry != NULL) {
            DictionaryEntry* next = entry->next;
            free(entry->str);
            free(entry);
            entry = next;
        }
    }

    free(dict.table);
    free(line);

    fclose(file);
    fclose(output);

    return 0;




}