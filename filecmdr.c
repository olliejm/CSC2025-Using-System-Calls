#include <stdio.h>
#include <stdlib.h>

#include "filecmdrlib.h"

/*
    To compile:
    # cc -o filecmdr filecmdrlib.c filecmdr.c 
    To run:
    # ./filecmdr <pathname>
*/

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <pathname>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    int status = EXIT_SUCCESS;
    int ftype = FTYPE_ERR;  
    
    if ((ftype = printfinf(argv[1])) == FTYPE_ERR) {
        perror("printfinf error");
        status = EXIT_FAILURE;
    } else if (useraction(ftype, argv[1]) < 0) {
        perror("useraction error");
        status = EXIT_FAILURE;
    }
        
    return status;
}