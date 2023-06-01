#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
// {
//     printf("Number of arguments: %d\n", argc);
//     for(int i=0; i<argc; i++) {
//         printf("Argument %d: %s\n", i, argv[i]);
//     }
// }


    if (argc != 4) {
        printf("Usage: %s [integer] [+|-|*|/] [integer]\n", argv[0]);
        return 1;
    }
    
    int num1 = atoi(argv[1]);
    char op = argv[2][0];
    int num2 = atoi(argv[3]);
    int result;
    
    switch (op) {
        case '+':
            result = num1 + num2;
            break;
        case '-':
            result = num1 - num2;
            break;
        case '*':
            result = num1 * num2;
            break;
        case '/':
            if (num2 == 0) {
                printf("Error: Division by zero\n");
                return 1;
            }
            result = num1 / num2;
            break;
        default:
            printf("Error: Invalid operator %c\n", op);
            return 1;
    }
    
    printf("%d\n", result);
    return 0;
}
