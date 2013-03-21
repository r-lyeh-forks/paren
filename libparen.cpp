// (C) 2013 Kim, Taegyoon
// Paren language core

#include "libparen.h"

using namespace std;

namespace libparen {
    paren::paren() {
        init();
    }

    node::node(): type(T_NIL) {}
    node::node(int a): type(T_INT), v_int(a) {}
    node::node(double a): type(T_DOUBLE), v_double(a) {}
    node::node(bool a): type(T_BOOL), v_bool(a) {}    
    node::node(const string a): type(T_STRING), v_string(a) {}
    node::node(const vector<node> &a): type(T_LIST), v_list(a) {}
    node nil;

    inline int node::to_int() {
        switch (type) {
        case T_INT:
            return v_int;
        case T_DOUBLE:
            return (int) v_double;
        case T_BOOL:
            return (int) v_bool;
        case T_STRING:
            return atoi(v_string.c_str());
        default:
            return 0;
        }
    }

    inline double node::to_double() {
        switch (type) {
        case T_INT:
            return v_int;
        case T_DOUBLE:
            return v_double;
        case T_BOOL:
            return v_bool;
        case T_STRING:
            return atof(v_string.c_str());
        default:
            return 0.0;
        }
    }
    inline string node::to_str() {
        stringstream ss;
        ss.precision(20);
        switch (type) {
        case T_NIL: break;
        case T_INT:
            ss << v_int; break;
        case T_BUILTIN:
            ss << "builtin." << v_int; break;
        case T_DOUBLE:
            ss << v_double; break;
        case T_BOOL:
            return (v_bool ? "true" : "false");
        case T_STRING:
        case T_SYMBOL:
            return v_string;
        case T_FN:
        case T_LIST:
            {
                ss << '(';
                for (vector<node>::iterator iter = v_list.begin(); iter != v_list.end(); iter++) {
                    if (iter != v_list.begin()) ss << ' ';
                    ss << iter->to_str();
                }
                ss << ')';
                break;
            }
        }
        return ss.str();
    }
    inline string node::type_str() {
        switch (type) {
        case T_NIL:
            return "nil";
        case T_INT:
            return "int";
        case T_DOUBLE:
            return "double";
        case T_BOOL:
            return "bool";
        case T_STRING:
            return "string";
        case T_SYMBOL:
            return "symbol";
        case T_LIST:
            return "list";
        case T_BUILTIN:
            return "builtin";
        case T_FN:
            return "fn";
        default:
            return "invalid type";
        }
    }
    inline string node::str_with_type() {
        return to_str() + " : " + type_str();
    }

    inline double paren::rand_double() {
        return (double) rand() / ((double) RAND_MAX + 1.0);
    }

    class tokenizer {
    private:
        vector<string> ret;
        string acc; // accumulator
        string s;        
        void emit() { // add accumulated string to token list
            if (acc.length() > 0) {ret.push_back(acc); acc = "";}
        }
    public:
        int unclosed; // number of unclosed parenthesis ( or quotation "
        tokenizer(const string &s): s(s), unclosed(0) {}
        
        vector<string> tokenize() {
            int last = s.length() - 1;
            for (int pos=0; pos <= last; pos++) {
                char c = s.at(pos);
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
                    emit();
                }
                else if (c == ';') { // end-of-line comment
                    emit();
                    do pos++; while (pos <= last && s.at(pos) != '\n');
                }
                else if (c == '"') { // beginning of string
                    unclosed++;
                    emit();
                    acc += '"';
                    pos++;
                    while (pos <= last) {
                        if (s.at(pos) == '"') {unclosed--; break;}
                        if (s.at(pos) == '\\') { // escape
                            char next = s.at(pos+1);
                            if (next == 'r') next = '\r';
                            else if (next == 'n') next = '\n';
                            else if (next == 't') next = '\t';
                            acc += next;
                            pos += 2;
                        }
                        else {
                            acc += s.at(pos);
                            pos++;
                        }
                    }
                    emit();
                }
                else if (c == '(') {
                    unclosed++;
                    emit();
                    acc += c;
                    emit();
                }
                else if (c == ')') {
                    unclosed--;
                    emit();
                    acc += c;
                    emit();
                }
                else {
                    acc += c;
                }
            }
            emit();
            return ret;
        }
    };

    vector<string> paren::tokenize(const string &s) {
        return tokenizer(s).tokenize();
    }
    
    class parser {        
    private:
        int pos;
        vector<string> tokens;
    public:
        parser(const vector<string> &tokens): pos(0), tokens(tokens) {}
        vector<node> parse() {
            vector<node> ret;
            int last = tokens.size() - 1;
            for (;pos <= last; pos++) {
                string tok = tokens.at(pos);
                if (tok.at(0) == '"') { // double-quoted string
                    ret.push_back(node(tok.substr(1)));
                }
                else if (tok == "(") { // list
                    pos++;
                    ret.push_back(node(parse()));
                }
                else if (tok == ")") { // end of list
                    break;
                }
                else if (isdigit(tok.at(0)) || (tok.at(0) == '-' && tok.length() >= 2 && isdigit(tok.at(1)))) { // number
                    if (tok.find('.') != string::npos || tok.find('e') != string::npos) { // double
                        ret.push_back(node(atof(tok.c_str())));
                    } else {
                        ret.push_back(node(atoi(tok.c_str())));
                    }
                } else { // symbol
                    node n;
                    n.type = node::T_SYMBOL;
                    n.v_string = tok;
                    ret.push_back(n);
                }
            }
            return ret;
        }        
    };
    
    vector<node> paren::parse(const string &s) {
        return parser(tokenize(s)).parse();
    }

    environment::environment(): outer(NULL) {}
    environment::environment(environment *outer): outer(outer) {}    

    node &environment::get(const string &name) {
        auto found = env.find(name);
        if (found != env.end()) {
            return found->second;
        }
        else {
            if (outer != NULL) {
                return outer->get(name);
            }
            else {
                return nil;
            }
        }
    }

    node builtin(int b) {
        node n(b);
        n.type = node::T_BUILTIN;
        return n;
    }

    node fn(node n, environment *outer_env) {
        node n2(n);
        n2.type = node::T_FN;
        n2.env = shared_ptr<void>(new environment(outer_env));
        return n2;
    }

    node paren::eval(node &n, environment &env) {
        switch (n.type) {
        case node::T_NIL:
        case node::T_INT:
        case node::T_DOUBLE:
        case node::T_BOOL:
        case node::T_STRING:
        case node::T_BUILTIN:
            {
                return n;
            }
        case node::T_SYMBOL:
            {
                node &n2 = env.get(n.v_string);
                if (n2.type != node::T_NIL)
                    return n2;
                else {
                    auto found = builtin_map.find(n.v_string);
                    if (found != builtin_map.end()) {
                        n = builtin(found->second); // elementary just-in-time compilation
                        return n;
                    }
                    else {
                        cerr << "Unknown variable: " << n.v_string << endl;
                        return nil;
                    }
                }
                
            }
        case node::T_LIST: // function (FUNCTION ARGUMENT ..)
            {
                if (n.v_list.size() == 0) return node();
                node func = eval(n.v_list[0], env);
                int builtin = -1;
                if (func.type == node::T_BUILTIN) {
                    builtin = func.v_int;
                    switch(builtin) {
                        case node::PLUS: // (+ X ..)
                            {
                                int len = n.v_list.size();
                                if (len <= 1) return node(0);
                                node first = eval(n.v_list[1], env);
                                if (first.type == node::T_INT) {
                                    int sum = first.v_int;
                                    for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                        sum += eval(*i, env).to_int();
                                    }
                                    return node(sum);
                                }
                                else {
                                    double sum = first.v_double;
                                    for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                        sum += eval(*i, env).to_double();
                                    }
                                    return node(sum);
                                }
                            }
                        case node::MINUS: // (- X ..)
                            {
                                int len = n.v_list.size();
                                if (len <= 1) return node(0);
                                node first = eval(n.v_list[1], env);
                                if (first.type == node::T_INT) {
                                    int sum = first.v_int;
                                    for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                        sum -= eval(*i, env).to_int();
                                    }
                                    return node(sum);
                                }
                                else {
                                    double sum = first.v_double;
                                    for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                        sum -= eval(*i, env).to_double();
                                    }
                                    return node(sum);
                                }
                            }
                        case node::MUL: // (* X ..)
                            {
                                int len = n.v_list.size();
                                if (len <= 1) return node(1);
                                node first = eval(n.v_list[1], env);
                                if (first.type == node::T_INT) {
                                    int sum = first.v_int;
                                    for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                        sum *= eval(*i, env).to_int();
                                    }
                                    return node(sum);
                                }
                                else {
                                    double sum = first.v_double;
                                    for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                        sum *= eval(*i, env).to_double();
                                    }
                                    return node(sum);
                                }
                            }
                        case node::DIV: // (/ X ..)
                            {
                                int len = n.v_list.size();
                                if (len <= 1) return node(1);
                                node first = eval(n.v_list[1], env);
                                if (first.type == node::T_INT) {
                                    int acc = first.v_int;
                                    for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                        acc /= eval(*i, env).to_int();
                                    }
                                    return node(acc);
                                }
                                else {
                                    double acc = first.v_double;
                                    for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                        acc /= eval(*i, env).to_double();
                                    }
                                    return node(acc);
                                }
                            }
                        case node::CARET: { // (^ BASE EXPONENT)
                            return node(pow(eval(n.v_list[1], env).to_double(), eval(n.v_list[2], env).to_double()));}
                        case node::PERCENT: { // (% DIVIDEND DIVISOR)
                            return node(eval(n.v_list[1], env).to_int() % eval(n.v_list[2], env).to_int());}
                        case node::SQRT: { // (sqrt X)
                            return node(sqrt(eval(n.v_list[1], env).to_double()));}
                        case node::INC: { // (inc X)
                                int len = n.v_list.size();
                                if (len <= 1) return node(0);
                                node first = eval(n.v_list[1], env);
                                if (first.type == node::T_INT) {
                                    return node(first.v_int + 1);
                                }
                                else {
                                    return node(first.v_double + 1.0);
                                }
                            }
                        case node::DEC: { // (dec X)
                                int len = n.v_list.size();
                                if (len <= 1) return node(0);
                                node first = eval(n.v_list[1], env);
                                if (first.type == node::T_INT) {
                                    return node(first.v_int - 1);
                                }
                                else {
                                    return node(first.v_double - 1.0);
                                }
                            }
                        case node::PLUSPLUS: { // (++ X)
                                int len = n.v_list.size();
                                if (len <= 1) return node(0);
                                node first = eval(n.v_list[1], env);
                                if (first.type == node::T_INT) {
                                    env.get(n.v_list[1].v_string).v_int++;
                                    return node();
                                }
                                else {
                                    env.get(n.v_list[1].v_string).v_double++;
                                    return node();
                                }
                            }
                        case node::MINUSMINUS: { // (-- X)
                                int len = n.v_list.size();
                                if (len <= 1) return node(0);
                                node first = eval(n.v_list[1], env);
                                if (first.type == node::T_INT) {
                                    env.get(n.v_list[1].v_string).v_int--;
                                    return node();
                                }
                                else {
                                    env.get(n.v_list[1].v_string).v_double--;
                                    return node();
                                }
                            }
                        case node::FLOOR: { // (floor X)
                            return node(floor(eval(n.v_list[1], env).to_double()));}
                        case node::CEIL: { // (ceil X)
                            return node(ceil(eval(n.v_list[1], env).to_double()));}
                        case node::LN: { // (ln X)
                            return node(log(eval(n.v_list[1], env).to_double()));}
                        case node::LOG10: { // (log10 X)
                            return node(log10(eval(n.v_list[1], env).to_double()));}
                        case node::RAND: { // (rand)
                            return node(rand_double());}
                        case node::SET: // (set SYMBOL VALUE)
                            {                            
                                env.env[n.v_list[1].v_string] = eval(n.v_list[2], env);
                                return node();
                            }
                        case node::EQEQ: { // (== X ..) short-circuit                    
                            node first = eval(n.v_list[1], env);
                            if (first.type == node::T_INT) {
                                int firstv = first.v_int;
                                for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                    if (eval(*i, env).to_int() != firstv) {return node(false);}
                                }
                            }
                            else {
                                double firstv = first.v_double;
                                for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                    if (eval(*i, env).to_double() != firstv) {return node(false);}
                                }
                            }
                            return node(true);}
                        case node::NOTEQ: { // (!= X ..) short-circuit                    
                            node first = eval(n.v_list[1], env);
                            if (first.type == node::T_INT) {
                                int firstv = first.v_int;
                                for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                    if (eval(*i, env).to_int() == firstv) {return node(false);}
                                }
                            }
                            else {
                                double firstv = first.v_double;
                                for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                    if (eval(*i, env).to_double() == firstv) {return node(false);}
                                }
                            }
                            return node(true);}
                        case node::LT: { // (< X Y)
                            node first = eval(n.v_list[1], env);
                            node second = eval(n.v_list[2], env);
                            if (first.type == node::T_INT) {
                                return node(first.v_int < second.to_int());
                            }
                            else {
                                return node(first.v_double < second.to_double());
                            }}
                        case node::GT: { // (> X Y)
                            node first = eval(n.v_list[1], env);
                            node second = eval(n.v_list[2], env);
                            if (first.type == node::T_INT) {
                                return node(first.v_int > second.to_int());
                            }
                            else {
                                return node(first.v_double > second.to_double());
                            }}
                        case node::LTE: { // (<= X Y)
                            node first = eval(n.v_list[1], env);
                            node second = eval(n.v_list[2], env);
                            if (first.type == node::T_INT) {
                                return node(first.v_int <= second.to_int());
                            }
                            else {
                                return node(first.v_double <= second.to_double());
                            }}
                        case node::GTE: { // (>= X Y)
                            node first = eval(n.v_list[1], env);
                            node second = eval(n.v_list[2], env);
                            if (first.type == node::T_INT) {
                                return node(first.v_int >= second.to_int());
                            }
                            else {
                                return node(first.v_double >= second.to_double());
                            }}
                        case node::ANDAND: { // (&& X ..) short-circuit
                            for (auto i = n.v_list.begin() + 1; i != n.v_list.end(); i++) {
                                if (!eval(*i, env).v_bool) {return node(false);}
                            }
                            return node(true);}
                        case node::OROR: { // (|| X ..) short-circuit
                            for (auto i = n.v_list.begin() + 1; i != n.v_list.end(); i++) {
                                if (eval(*i, env).v_bool) {return node(true);}
                            }
                            return node(false);}
                        case node::NOT: { // (! X)
                            return node(!(eval(n.v_list[1], env).v_bool));}
                        case node::IF: { // (if CONDITION THEN_EXPR ELSE_EXPR)
                            node &cond = n.v_list[1];
                            if (eval(cond, env).v_bool) {
                                return eval(n.v_list[2], env);
                            }
                            else {
                                return eval(n.v_list[3], env);
                            }}
                        case node::WHEN: { // (when CONDITION EXPR ..)
                            node &cond = n.v_list[1];
                            if (eval(cond, env).v_bool) {
                                int len = n.v_list.size();
                                for (int i = 2; i < len - 1; i++) {
                                    eval(n.v_list[i], env);
                                }
                                return eval(n.v_list[len - 1], env); // returns last EXPR
                            }
                            return node();}
                        case node::FOR: // (for SYMBOL START END STEP EXPR ..)
                            {
                                node start = eval(n.v_list[2], env);
                                env.env[n.v_list[1].v_string] = start;
                                int len = n.v_list.size();
                                if (start.type == node::T_INT) {                            
                                    int last = eval(n.v_list[3], env).to_int();
                                    int step = eval(n.v_list[4], env).to_int();                                
                                    int &a = env.get(n.v_list[1].v_string).v_int;
                                    if (step >= 0) {
                                        for (; a <= last; a += step) {
                                            for (int i = 5; i < len; i++) {
                                                eval(n.v_list[i], env);
                                            }
                                        }
                                    }
                                    else {
                                        for (; a >= last; a += step) {
                                            for (int i = 5; i < len; i++) {
                                                eval(n.v_list[i], env);
                                            }
                                        }
                                    }
                                }
                                else {
                                    double last = eval(n.v_list[3], env).to_double();
                                    double step = eval(n.v_list[4], env).to_double();                                
                                    double &a = env.get(n.v_list[1].v_string).v_double;
                                    if (step >= 0) {
                                        for (; a <= last; a += step) {
                                            for (int i = 5; i < len; i++) {
                                                eval(n.v_list[i], env);
                                            }
                                        }
                                    }
                                    else {
                                        for (; a >= last; a += step) {
                                            for (int i = 5; i < len; i++) {
                                                eval(n.v_list[i], env);
                                            }
                                        }
                                    }
                                }
                                return node();
                            }
                        case node::WHILE: { // (while CONDITION EXPR ..)
                            node &cond = n.v_list[1];
                            int len = n.v_list.size();
                            while (eval(cond, env).v_bool) {
                                for (int i = 2; i < len; i++) {
                                    eval(n.v_list[i], env);
                                }
                            }
                            return node(); }
                        case node::STRLEN: { // (strlen X)
                            return node((int) eval(n.v_list[1], env).v_string.size());}
                        case node::STRCAT: { // (strcat X ..)
                            int len = n.v_list.size();
                            if (len <= 1) return node("");
                            node first = eval(n.v_list[1], env);
                            string acc = first.to_str();
                            for (auto i = n.v_list.begin() + 2; i != n.v_list.end(); i++) {
                                acc += eval(*i, env).to_str();
                            }
                            return node(acc);}
                        case node::CHAR_AT: { // (char-at X)
                            return node(eval(n.v_list[1], env).v_string[eval(n.v_list[2], env).v_int]);}
                        case node::CHR: { // (chr X)
                            char temp[2] = " ";
                            temp[0] = (char) eval(n.v_list[1], env).v_int;
                            return node(string(temp));}
                        case node::STRING: { // (string X)
                            return node(eval(n.v_list[1], env).to_str());}
                        case node::DOUBLE: { // (double X)
                            return node(eval(n.v_list[1], env).to_double());}
                        case node::INT: { // (int X)
                            return node(eval(n.v_list[1], env).to_int());}
                        case node::READ_STRING: { // (read-string X)
                            return node(parse(eval(n.v_list[1], env).to_str())[0]);}
                        case node::TYPE: { // (type X)
                            return node(eval(n.v_list[1], env).type_str());}
                        case node::EVAL: { // (eval X)                    
                            node n2 = eval(n.v_list[1], env);
                            return node(eval(n2, env));}
                        case node::QUOTE: { // (quote X)
                            return n.v_list[1];}
                        case node::FN: { // (fn (ARGUMENT ..) BODY) => lexical closure
                            node n2 = fn(n.v_list, &env);
                            return n2;}
                        case node::LIST: { // (list X ..)
                            vector<node> ret;
                            for (unsigned int i = 1; i < n.v_list.size(); i++) {
                                ret.push_back(eval(n.v_list[i], env));
                            }
                            return node(ret);}
                        case node::APPLY: { // (apply FUNC LIST)
                            vector<node> expr;
                            node f = eval(n.v_list[1], env);
                            expr.push_back(f);
                            vector<node> lst = eval(n.v_list[2], env).v_list;
                            for (unsigned int i = 0; i < lst.size(); i++) {
                                expr.push_back(lst.at(i));
                            }
                            node n2 = node(expr);
                            return eval(n2, env);
                        }
                        case node::MAP: { // (map FUNC LIST)
                            node f = eval(n.v_list.at(1), env);
                            vector<node> lst = eval(n.v_list.at(2), env).v_list;
                            vector<node> acc;
                            vector<node> expr; // (FUNC ITEM)
                            expr.push_back(f);
                            expr.push_back(node());
                            for (unsigned int i = 0; i < lst.size(); i++) {
                                expr[1] = lst.at(i);
                                node n2 = node(expr);
                                acc.push_back(eval(n2, env));
                            }                    
                            return node(acc);
                        }
                        case node::FILTER: { // (filter FUNC LIST)
                            node f = eval(n.v_list.at(1), env);
                            vector<node> lst = eval(n.v_list.at(2), env).v_list;
                            vector<node> acc;
                            vector<node> expr; // (FUNC ITEM)
                            expr.push_back(f);
                            expr.push_back(node());
                            for (unsigned int i = 0; i < lst.size(); i++) {
                                node item = lst.at(i);
                                expr[1] = item;
                                node n2 = node(expr);
                                node ret = eval(n2, env);
                                if (ret.v_bool) acc.push_back(item);
                            }                    
                            return node(acc);
                        }
                        case node::RANGE: { // (range START END STEP)
                            node start = eval(n.v_list.at(1), env);                    
                            vector<node> ret;
                            if (start.type == node::T_INT) {
                                int a = eval(n.v_list.at(1), env).v_int;
                                int last = eval(n.v_list.at(2), env).v_int;
                                int step = eval(n.v_list.at(3), env).v_int;
                                if (step >= 0) {
                                    for (; a <= last; a += step) {
                                        ret.push_back(node(a));}}
                                else {
                                    for (; a >= last; a += step) {
                                        ret.push_back(node(a));}}
                            }
                            else {
                                double a = eval(n.v_list.at(1), env).v_double;
                                double last = eval(n.v_list.at(2), env).v_double;
                                double step = eval(n.v_list.at(3), env).v_double;
                                if (step >= 0) {
                                    for (; a <= last; a += step) {
                                        ret.push_back(node(a));}}
                                else {
                                    for (; a >= last; a += step) {
                                        ret.push_back(node(a));}}
                            }
                            return node(ret);
                        }
                        case node::NTH: { // (nth INDEX LIST)
                            int i = eval(n.v_list.at(1), env).v_int;
                            const vector<node> &lst = eval(n.v_list.at(2), env).v_list;
                            return lst.at(i);}
                        case node::LENGTH: { // (length LIST)                    
                            const vector<node> &lst = eval(n.v_list.at(1), env).v_list;
                            return node((int) lst.size());}
                        case node::BEGIN: { // (begin X ..)                    
                            int last = n.v_list.size() - 1;
                            if (last <= 0) return node();
                            for (int i = 1; i < last; i++) {
                                eval(n.v_list[i], env);
                            }
                            return eval(n.v_list[last], env);}
                        case node::PR: // (pr X ..)
                            {
                                auto first = n.v_list.begin() + 1;
                                for (auto i = first; i != n.v_list.end(); i++) {
                                    if (i != first) printf(" ");
                                    printf("%s", eval(*i, env).to_str().c_str());
                                }
                                return node();
                            }
                        case node::PRN: // (prn X ..)
                            {
                                auto first = n.v_list.begin() + 1;
                                for (auto i = first; i != n.v_list.end(); i++) {
                                    if (i != first) printf(" ");
                                    printf("%s", eval(*i, env).to_str().c_str());
                                }
                                puts("");
                                return node();
                            }
                        case node::EXIT: { // (exit X)
                                puts("");
                                exit(eval(n.v_list[1], env).to_int());
                                return node(); }
                        case node::SYSTEM: { // Invokes the command processor to execute a command.
                            string cmd;
                            for (unsigned int i = 1; i < n.v_list.size(); i++) {
                                if (i != 1) cmd += ' ';
                                cmd += eval(n.v_list[i], env).v_string;
                            }
                            return node(system(cmd.c_str()));}
                        default: {
                            cerr << "Not implemented function: [" << func.v_string << "]" << endl;
                            return node();}
                    } // end switch
                }
                else {
                    vector<node> &f = func.v_list;
                    if (func.type == node::T_FN) {
                        // anonymous function application. lexical scoping
                        // (fn (ARGUMENT ..) BODY ..)
                        vector<node> &arg_syms = f[1].v_list;

                        environment *local_env = (environment *)func.env.get();
                        int alen = arg_syms.size();
                        for (int i=0; i<alen; i++) { // assign arguments
                            string &k = arg_syms.at(i).v_string;
                            local_env->env[k] = eval(n.v_list.at(i + 1), env);
                        }

                        int flen = f.size();
                        for (int i=2; i<flen-1; i++) { // body
                            eval(f.at(i), *local_env);
                        }
                        node ret = eval(f.at(flen-1), *local_env);
                        return ret;
                    }
                    else {
                        cerr << "Unknown function: [" << func.to_str() << "]" << endl;
                        return node();
                    }
                }
            }
        default:
            cerr << "Unknown type" << endl;
            return node();
        }
    }

    node paren::eval_all(vector<node> &lst) {
        int last = lst.size() - 1;
        if (last < 0) return node();
        for (int i = 0; i < last; i++) {
            eval(lst[i], global_env);
        }
        return eval(lst[last], global_env);
    }

    void paren::print_symbols() {
        int i = 0;    
        map<string, node> ordered(global_env.env.begin(), global_env.env.end());
        for (auto iter = ordered.begin(); iter != ordered.end(); iter++) {
            printf(" %s", iter->first.c_str());
            i++;
            if (i % 10 == 0) puts("");
        }
        puts("");
    }
    
    void paren::print_functions() {
        int i = 0;
        map<string, int> ordered(builtin_map.begin(), builtin_map.end());
        for (auto iter = ordered.begin(); iter != ordered.end(); iter++) {
            printf(" %s", iter->first.c_str());
            i++;
            if (i % 10 == 0) puts("");
        }
        puts("");
    }
    
    void paren::print_logo() {
        printf("Paren %s (C) 2013 Kim, Taegyoon\n", PAREN_VERSION);        
        puts("Predefined Symbols:");
        print_symbols();
        puts("Functions:");
        print_functions();
        puts("Etc.:");
        puts(" (list) \"string\" ; end-of-line comment");
    } 

    void paren::prompt() {
        printf("> ");
    }

    void paren::prompt2() {
        printf("  ");
    }

    inline void paren::init() {
        srand((unsigned int) time(0));
        global_env.env["true"] = node(true);
        global_env.env["false"] = node(false);
        global_env.env["E"] = node(2.71828182845904523536);
        global_env.env["PI"] = node(3.14159265358979323846);

        builtin_map["+"] = node::PLUS;
        builtin_map["-"] = node::MINUS;
        builtin_map["*"] = node::MUL;
        builtin_map["/"] = node::DIV;
        builtin_map["^"] = node::CARET;
        builtin_map["%"] = node::PERCENT;
        builtin_map["sqrt"] = node::SQRT;
        builtin_map["inc"] = node::INC;
        builtin_map["dec"] = node::DEC;
        builtin_map["++"] = node::PLUSPLUS;
        builtin_map["--"] = node::MINUSMINUS;
        builtin_map["floor"] = node::FLOOR;
        builtin_map["ceil"] = node::CEIL;
        builtin_map["ln"] = node::LN;
        builtin_map["log10"] = node::LOG10;
        builtin_map["rand"] = node::RAND;
        builtin_map["=="] = node::EQEQ;
        builtin_map["!="] = node::NOTEQ;
        builtin_map["<"] = node::LT;
        builtin_map[">"] = node::GT;
        builtin_map["<="] = node::LTE;
        builtin_map[">="] = node::GTE;
        builtin_map["&&"] = node::ANDAND;
        builtin_map["||"] = node::OROR;
        builtin_map["!"] = node::NOT;
        builtin_map["if"] = node::IF;
        builtin_map["when"] = node::WHEN;
        builtin_map["for"] = node::FOR;
        builtin_map["while"] = node::WHILE;
        builtin_map["strlen"] = node::STRLEN;
        builtin_map["strcat"] = node::STRCAT;
        builtin_map["char-at"] = node::CHAR_AT;
        builtin_map["chr"] = node::CHR;
        builtin_map["int"] = node::INT;
        builtin_map["double"] = node::DOUBLE;
        builtin_map["string"] = node::STRING;
        builtin_map["read-string"] = node::READ_STRING;
        builtin_map["type"] = node::TYPE;
        builtin_map["eval"] = node::EVAL;
        builtin_map["quote"] = node::QUOTE;
        builtin_map["fn"] = node::FN;
        builtin_map["list"] = node::LIST;
        builtin_map["apply"] = node::APPLY;
        builtin_map["map"] = node::MAP;
        builtin_map["filter"] = node::FILTER;
        builtin_map["range"] = node::RANGE;
        builtin_map["nth"] = node::NTH;
        builtin_map["length"] = node::LENGTH;
        builtin_map["begin"] = node::BEGIN;
        builtin_map["set"] = node::SET;
        builtin_map["pr"] = node::PR;
        builtin_map["prn"] = node::PRN;
        builtin_map["exit"] = node::EXIT;
        builtin_map["system"] = node::SYSTEM;
    }

    node paren::eval_string(string &s) {
        auto vec = parse(s);
        return eval_all(vec);
    }

    node paren::eval_string(const char* s) {
        string s2(s);
        return eval_string(s2);
    }

    inline void paren::eval_print(string &s) {
        cout << eval_string(s).str_with_type() << endl;
    }

    // read-eval-print loop
    void paren::repl() {
        string code;
        while (true) {
            if (code.length() == 0) prompt(); else prompt2();
            string line;
            if (!getline(cin, line)) { // EOF
                eval_print(code);
                return;
            }
            code += '\n' + line;
            tokenizer t(code);
            t.tokenize();
            if (t.unclosed <= 0) { // no unmatched parenthesis nor quotation
                eval_print(code);
                code = "";
            }
        }
    }

    node &paren::get(const char* name) {
        string s(name);
        return global_env.get(s);
    }

    void paren::set(const char* name, node value) {
        string s(name);
        global_env.env[s] = value;
    }
} // namespace libparen
