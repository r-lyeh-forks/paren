# Paren: The Paren Programming Language #

(C) 2013 Kim, Taegyoon

Paren is a dialect of Lisp. It is designed to be an embedded language.

## Run ##
```
Usage: paren [OPTIONS...] [FILES...]

OPTIONS:
    -h    print this screen.
    -v    print version.
```

## Reference ##
```
Predefined Symbols:
 E PI false true
Functions:
 ! != % && * + ++ - -- /
 < <= == > >= ^ apply begin ceil char-at
 chr dec double eval exit filter floor fn for if
 inc int length list ln log10 map nth pr prn
 quote rand range read-string set sqrt strcat string strlen system
 type when while ||
Etc.:
 (list) "string" ; end-of-line comment
```

## Files ##
* libparen.h libparen.cpp: Paren language library
* paren.cpp: Paren REPL executable

## Examples ##
### Hello, World! ###
```
(prn "Hello, World!")
```

### Function ###

In a function, [lexical scoping](http://en.wikipedia.org/wiki/Lexical_scoping#Lexical_scoping) is used.

```
> ((fn (x y) (+ x y)) 1 2)
3 : int
> ((fn (x) (* x 2)) 3)
6 : int
> (set sum (fn (x y) (+ x y)))
 : nil
> (sum 1 2)
3 : int
> (set even? (fn (x) (== 0 (% x 2))))
 : nil
> (even? 3)
false : bool
> (even? 4)
true : bool
> (apply + (list 1 2 3))
6 : int
> (map sqrt (list 1 2 3 4))
(1 1.4142135623730951 1.7320508075688772 2) : list
> (filter even? (list 1 2 3 4 5))
(2 4) : list
> (set x 1) ((fn (x) (prn x) (set x 3) (prn x)) 4) (prn x) ; lexical scoping
4
3
1
 : nil
> (set adder (fn (amount) (fn (x) (+ x amount)))) (set add3 (adder 3)) (add3 4); lexical scoping
7 : int
```

#### Recursion ####
```
> (set factorial (fn (x) (if (<= x 1) x (* x (factorial (dec x))))))
 : nil
> (for i 1 5 1 (prn i (factorial i)))
1 1
2 2
3 6
4 24
5 120
 : nil
```

### List ###
```
> (nth 1 (list 2 4 6))
4 : int
> (length (list 1 2 3))
3 : int
```

### System Command (Shell) ###
```
(system "notepad" "a.txt") ; compatible with Parenj
(system "notepad a.txt") ; same as above; not compatible with Parenj
```

### Embedding ###
Compile libparen.cpp
```
#include "libparen.h"

using namespace libparen;

int main() {
	paren p;    
    cout << p.eval_string("(+ 1 2)").v_int << endl; // evaluate code and get return value
    p.eval_string("(set a 1)"); // evaluate code
    cout << p.get("a").v_int << endl; // get variable
    p.set("a", node(string("Hello"))); // set variable
    cout << p.get("a").v_string << endl; // get variable
}
```
=>
```
3
1
Hello
```

### [Project Euler Problem 1](http://projecteuler.net/problem=1) ###
```
(set s 0)
(for i 1 999 1
    (when (|| (== 0 (% i 3)) (== 0 (% i 5)))
        (set s (+ s i))))
(prn s)
```
=> 233168

```
(apply + (filter (fn (x) (|| (== 0 (% x 3)) (== 0 (% x 5)))) (range 1 999 1)))
```
=> 233168

### [Project Euler Problem 2](http://projecteuler.net/problem=2) ###
```
(set a 1)
(set b 1)
(set sum 0)
(while (<= a 4000000)
  (set c (+ a b))
  (set a b)
  (set b c)
  (when (== 0 (% a 2))
    (set sum (+ sum a))))
(prn sum)
```
=> 4613732

### [Project Euler Problem 4](http://projecteuler.net/problem=4) ###
```
(set maxP 0)
(for i 100 999 1
  (for j 100 999 1	
    (set p (* i j))
    (set ps (string p))
    (set len (strlen ps))
    (set to (/ len 2))
    (set pal true)
    (set k 0)
    (set k2 (dec len))
    (while
      (&& (< k to) pal)
	  (when (!= (char-at ps k) (char-at ps k2))
		(set pal false))
	  (++ k)
	  (-- k2))
	(when pal
	  (when (> p maxP)
		(set maxP p)))))
(prn maxP)
```
=> 906609

[More solutions of Project Euler in Paren](https://bitbucket.org/ktg/euler-paren) (Some of them are for [Parenj](https://bitbucket.org/ktg/parenj).)

### [99 Bottles of Beer](http://en.wikipedia.org/wiki/99_Bottles_of_Beer) ###
```
(for i 99 1 -1
  (prn i "bottles of beer on the wall," i "bottles of beer.")
  (prn "Take one down and pass it around," (dec i) "bottle of beer on the wall."))
```

## Alternative Implementations ##
* [Paren](https://bitbucket.org/ktg/paren) (Paren running natively)
* [Parenj](https://bitbucket.org/ktg/parenj) (Paren running on the Java Virtual Machine)

## License ##

   Copyright 2013 Kim, Taegyoon

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   [http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0)

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
