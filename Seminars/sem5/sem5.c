#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

typedef struct {
    int position;
    int length;
} LINE_STRUCT;

int count_lines(FILE *f, LINE_STRUCT *array_lines) {
    int count_l = 0, position;
    char buffer[MAX_LINE_LENGTH];

    while ((position = ftell(f)) >= 0 && fgets(buffer, sizeof(buffer), f)) {
        int len = strlen(buffer);
        array_lines[count_l].position = position;
        array_lines[count_l].length = len;
        count_l++;
    }
    if (count_l > 0 && buffer[strlen(buffer) - 1] != '\n') {
        array_lines[count_l].position = ftell(f);
        array_lines[count_l].length = 0;
        count_l++;
    }

    return count_l;
}

int compare_length(const LINE_STRUCT *a, const LINE_STRUCT *b) {
    return (b->length - a->length);
}

void print_line(FILE *f, LINE_STRUCT * array_lines, int number_to_print, int count_lines)
{
    if (number_to_print <= 0 || number_to_print > count_lines) {
        printf("Error! There are no line with requared number %d in the file.\n", number_to_print);
        return;
    }
    char buf[MAX_LINE_LENGTH];
    fseek(f, array_lines[number_to_print-1].position, SEEK_SET);
    fgets(buf, sizeof(buf), f);
    printf("Line %d: ", number_to_print);
    puts(buf);
}

void bubblesort_lines(LINE_STRUCT *array_lines, int lines_count)
{
    for (int i = 0; i < lines_count - 1; i++){
        for (int j = 0; j < lines_count - i - 1; j++){
            if (compare_length(&array_lines[i], &array_lines[j])){
                LINE_STRUCT temp = array_lines[j];
                array_lines[j] = array_lines[i];
                array_lines[i] = temp;
               }
        }
    }
}
void print_full_file(FILE *f, LINE_STRUCT *array_lines, int lines_count) {
    FILE *output_f = fopen("sorted_final.txt", "w");
    if (!output_f) {
        printf("Error with output.txt.\n");
        return;
    }
    
    bubblesort_lines(array_lines, lines_count);

    for (int i = lines_count-1; i >=0 ; i--) {
        fseek(f, array_lines[i].position, SEEK_SET);
        char buffer[MAX_LINE_LENGTH];
        fgets(buffer, sizeof(buffer), f);
        fprintf(output_f, "%s", buffer);
    }
    fclose(output_f);
    
     FILE *output_f1 = fopen("sorted_final.txt", "r");
    char buffer[MAX_LINE_LENGTH];
    while(fgets(buffer, sizeof(buffer), output_f1))
        printf("%s", buffer);
    
    fclose(output_f1);
    
}

int main(int argc, char ** argv)
{
    if (argc != 2) {
        printf("Error with arguments\n");
        return 1;}
    FILE * f = fopen(argv[1], "r+");
    LINE_STRUCT array_lines[100];
    int lines_count = count_lines(f, array_lines);
    printf("Lines count in the file: %d\n", lines_count);
/*
    int number_to_print;
    while (1) {
        printf("Input line nunber to print the line: ");
        scanf("%d", &number_to_print);
        if (number_to_print == 0) 
            break;
         print_line(f, array_lines, number_to_print, lines_count);
    }
*/
    printf("Sorted lines file: ");
    print_full_file(f, array_lines, lines_count);
    fclose(f);
    
    return 0;
}