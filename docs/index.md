---
layout: post
title: "Canonical C++ Class With CAS"
date: 20240908
categories: cpp
tags: canonical
archive: true
---

Let's discuss the issue in brief.

## Canonical C++ Class

For a c++ class that manages its own acquired resource (think memory or file 
handles), the class should implement its own version of ctr, dtr, copy ctr and 
copy assignment.  Since c++ 2010, this list includes move ctr and move 
assignment.  This is all standard best practices.

### Noisy copy assignment

One slight annoyance I noticed is that the the copy assignment operator 
typical implementation of copy assignment is its usual book-keeping steps:

1. check if it's passed in itself for preventing self-copies, 
2. clean up the source object's contained resource for preventing resource leaks
and 
3. finally copying over the source object's resource.  

These steps to copy over a source object are a bit tedious.  Is there another
way?

### Problem

Perhaps we can simplying the copy assignment by using a copy-and-swap pattern?
I gave this a try and immediately received a 'ambiguous overload' compiler error.
The problem is this- the copy assignment's and move assignment's signatures are usually
this:

	Thing& operator=(Thing& other);   	// copy assignment
	Thing& operator=(Thing&& other);	// move assignment

But when I make the copy assignment use copy-and-swap, this requires setting 
its signature like this:

	Thing& operator=(Thing other);   	// copy assignment
	Thing& operator=(Thing&& other);	// move assignment

This immediately confused the compiler because an rvalue could bind with
both methods.  

### Solution

One solution is to remove the move assignment implementation.  Both lvalues 
and rvalues would work with the CAS-version of the copy assignment.

Below is a sample implementation of a canoncial c++ class with CAS for its
single copy assignment that works with both rvalues and lvalues.  The runtime 
output is shown as well.

~~~cpp
#include <iostream>

// canonical class in c++
//
// default constructor (for arrays)
// constructor - with parameter
// destructor
// copy constructor
// copy assignment operator using CAS
//
// for c++11 and after
// move constructor
// move assignment operator

static void log(const char *msg) { std::cout << msg << '\n'; } // reduce noise
static void log(char id, const char *msg) { std::cout << ' ' << msg << ' ' << id << '\n'; } // reduce noise

class Thing {
    char* t; // just example, would never do this in production code
            //
 
 public:
    Thing()       : t(new char(' '))    { log(*t, "ctr"); }
    Thing(char c) : t(new char(c))      { log(*t, "ctr_p"); }
    ~Thing()                            { log(*t, "dtr"); delete t; };

    // copy ctr
    Thing(const Thing& other) : t(new char(*other.t)) { log(*t, "copy ctr"); }

    // copy assignment using CAS, notice use of copy ctr in parameter
    Thing& operator=(Thing other)   {
        std::swap(t, other.t);  // other was just created by copy ctr
        log(*t, "copy assignment");
        return *this;
    }

    // move ctr
    Thing(Thing&& other) noexcept :  // noexcept for optimisation
        t(other.t) {        // steal the other's innards
        other.t = nullptr;  // leave the other's blank
        log(*t, "move ctr");
    }

};

Thing get_thing() {
    log("creating b");
    Thing b('b');
    return b; 
}

int main() {
    log("*** first test ***");
    log("creating a");
    Thing a('a');
    log("returning b as a temporary rvalue to a, overwrite a with move assignment");
    a = get_thing();

    log("*** second test ***");
    log("creating c");
    Thing c('c');
    log("copying c as an lvalue to a, overwrite a with copy assignment");
    a = c;

    log("end of test");
}

~~~

And the output:

~~~
*** first test ***
creating a
 ctr_p a
returning b as a temporary rvalue to a, overwrite a with move assignment
creating b
 ctr_p b
 copy assignment b
 dtr a
*** second test ***
creating c
 ctr_p c
copying c as an lvalue to a, overwrite a with copy assignment
 copy ctr c
 copy assignment c
 dtr b
end of test
 dtr c
 dtr c
~~~

## Discussion

What did the use of swap-and-copy accomplish for the copy assignment and 
move assignment operations?  It

1. reduces code redundancy, reuses the same code for both copy and move assignments
2. simplifies the tiresome book-keeping assignment steps into one CAS step

### Code Details

The standard copy assignment and move assignment method signatures look like 
this:
    
    Thing& operator=(const Thing&); // copy assignment
    Thing& operator=(Thing&&);      // move assignment

When we replace the copy assignment with a copy-and-swap implementation, these
signatures look like this:

    Thing& operator=(Thing);        // lead to ambiguous call
    Thing& operator=(Thing&&);      // lead to ambiguous call

The compiler immediately complains because an rvalue can bind to both methods.
To fix this problem, we can go back to the original implementation OR ...
simplify to one CAS-powered assignment which binds to both lvalues and rvalues:

    Thing& operator=(Thing);        

### Resource leaks?

Valgrind did not report any resource leaks with the test code.

Why did the rvalue returned by getValue() not produce an extra copy?
These lines:

    Thing a('a');       // create a
    a = get_thing();    // create b and return b, overwriting a

only generated this output:

    ctr_p a
    ctr_p b
    copy assignment b
    dtr a
    
Why did on the return of get_thing(), there was no additional copy created?  
Starting with c++ 17, the compiler is guaranteed to use **return value optimization**
or RVO.  The compiler decides to allocate space in the stack for the return
value- and the return value within the function is created in that space for
return without the need to make an extra copy.  This eliminated one copy.

Then the compiler binded the returned rvalue, a temporary value without a name, 
to the copy assignment method- which, because of its value type argument, 
works for both lvalues and rvalues. 


