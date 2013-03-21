// (C) 2013 Kim, Taegyoon
// The Paren Programming Language

#include <cstdio>
#include <cstring>
#include "libparen.h"

using namespace libparen;

int main(int argc, char *argv[]) {    
    if (argc <= 1) {
        paren p;
        p.print_logo();
        p.repl();
        puts("");
        return 0;
    }
    else if (argc == 2) {
        char *opt(argv[1]);
        if (strcmp(opt, "-h") == 0) {
            puts("Usage: paren [OPTIONS...] [FILES...]");
            puts("");
            puts("OPTIONS:");
            puts("    -h    print this screen.");
            puts("    -v    print version.");
            return 0;
        } else if (strcmp(opt, "-v") == 0) {
            puts(PAREN_VERSION);
            return 0;
        }
    }

    // execute files, one by one
    for (int i = 1; i < argc; i++) {
        paren p;
        FILE *file = fopen(argv[i], "r");
        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            long size = ftell(file);
            fseek(file, 0, SEEK_SET);
            char *code = (char *) malloc((size + 1) * sizeof(char)); // size + null
            code[size] = 0;
            fread(code, sizeof(char), size, file);
            fclose(file);
            
            string strCode(code);            
            p.eval_string(strCode);
            free(code); code = NULL;
        }
        else {
            fprintf(stderr, "Cannot open file: %s\n", argv[i]);
        }
    }
}
