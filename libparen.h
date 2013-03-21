// (C) 2013 Kim, Taegyoon
// Paren language core

#pragma once
#ifndef LIBPAREN_H
#define LIBPAREN_H

#include <iostream>
#include <sstream>
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <unordered_map>
#include <map>
#include <ctime>
#include <memory>

#define PAREN_VERSION "1.4.2"

namespace libparen {
    using namespace std;

    struct node {
        enum {T_NIL, T_INT, T_DOUBLE, T_BOOL, T_STRING, T_SYMBOL, T_LIST, T_BUILTIN, T_FN} type;
        enum builtin {PLUS, MINUS, MUL, DIV, CARET, PERCENT, SQRT, INC, DEC, PLUSPLUS, MINUSMINUS, FLOOR, CEIL, LN, LOG10, RAND,
            EQEQ, NOTEQ, LT, GT, LTE, GTE, ANDAND, OROR, NOT,
            IF, WHEN, FOR, WHILE,
            STRLEN, STRCAT, CHAR_AT, CHR,
            INT, DOUBLE, STRING, READ_STRING, TYPE, SET,
            EVAL, QUOTE, FN, LIST, APPLY, MAP, FILTER, RANGE, NTH, LENGTH, BEGIN,
            PR, PRN, EXIT, SYSTEM};
        union {
            int v_int;
            double v_double;
            bool v_bool;
        };
        string v_string;
        vector<node> v_list;
        shared_ptr<void> env; // if T_FN, actually (environment *). to avoid mutual reference

        node();
        node(int a);
        node(double a);
        node(bool a);
        node(const string a);
        node(const vector<node> &a);

        int to_int(); // convert to int
        double to_double(); // convert to double
        string to_str(); // convert to string
        string type_str();
        string str_with_type();
    };

    struct environment {
        unordered_map<string, node> env;
        environment *outer;
        environment();
        environment(environment *outer);
        node &get(const string &name);
    };

    struct paren {
        paren();

        inline double rand_double();
        vector<string> tokenize(const string &s);
        vector<node> parse(const string &s);

        unordered_map<string, int> builtin_map;
        environment global_env; // variables

        node eval(node &n, environment &env);
        node eval_all(vector<node> &lst);
        void print_symbols();
        void print_functions();
        void print_logo();
        void prompt();
        void prompt2();
        inline void init();
        node eval_string(string &s);
        node eval_string(const char* s);
        inline void eval_print(string &s);
        void repl(); // read-eval-print loop

        node &get(const char* name);
        void set(const char* name, node value);
    }; // struct paren
} // namespace libparen
#endif
