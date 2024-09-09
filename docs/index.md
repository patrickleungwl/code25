---
layout: post
title: "Canonical C++ Class With CAS"
date: 20240908
categories: cpp
tags: canonical
archive: true
---

## Canonical C++ Class

For a c++ class that manages its own acquired resource (think memory or file 
handles), the class should contain its own version of ctr, dtr, copy ctr and 
copy assignment.  Since c++ 2010, this list includes move ctr and move 
assignment.  This is all fine.  

### Noisy copy assignment

One slight annoyance I noticed is that the the copy assignment operator 
typical implementation of copy assignment is its usual book-keeping steps:
1) check if it's passed in itself for preventing self-copies, 
2) clean up the source object's contained resource for preventing resource leaks
and then 3) finally copying over the source object's resource.  
These steps to copy over a source object are a bit noisy to the eyes.

### Problem

Perhaps we can do better with a copy-and-swap idiom for reducing this noise? 
I gave this a try and immediately received a 'ambiguous overload' compiler error.
The problem is this- the copy assignment's and move assignment's signatures are usually
this:

	Thing& operator=(Thing& other);   	// copy assignment
	Thing& operator=(Thing&& other);	// move assignment

But when I make the copy assignment use copy-and-swap, this requires setting 
its signature like this:

	Thing& operator=(Thing other);   	// copy assignment
	Thing& operator=(Thing&& other);	// move assignment

This immediately confused the compiler because an rvalue could qualify for 
both methods.  

### Solution

The solution is to remove the move assignment implementation.  Both lvalues 
and rvalues would work with the CAS-version of the copy assignment.

Below is a sample implementation of a canoncial c++ class with CAS for its
combined copy assignment and move assignment.  Runtime output is shown as well.

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

class Thing {
    int* t; // just example, would never do this in production code
 
 public:
    Thing()      : t(new int(0))    { log("ctr"); }
    Thing(int i) : t(new int(i))    { log("ctr_p"); }
    ~Thing()                        { delete t; log("dtr"); };

    // copy ctr
    Thing(const Thing& other) : t(new int(*other.t)) { log("copy ctr"); }

    // copy assignment using CAS, notice use of copy ctr in parameter
    Thing& operator=(Thing other)   {
        std::swap(t, other.t);  // other was just created by copy ctr
        log("copy assignment");
        return *this;
    }

    // move ctr
    Thing(Thing&& other) noexcept :  // noexcept for optimisation
        t(other.t) {        // steal the other's innards
        other.t = nullptr;  // leave the other's blank
        log("move ctr");
    }

    // move assignment
    // Thing& operator=(Thing&& other) noexcept {
    //      this fails compiling with a ambigious overload for
    //      operator=, because rvalue fits for both the copy assignment
    //      operator and the move assignment.
    // }
    //
    // Thing& operator=(Thing);   // Lead to ambiguous call
    // Thing& operator=(Thing&&); // Lead to ambiguous call
    //
    //  to fix- do this:
    //  Thing& operator=(Thing);
    //
    //  or
    //  Thing& operator=(const Thing&);
    //  Thing& operator=(Thing&&);
};

Thing get_thing() {
    log("creating b");
    Thing b(2);
    return b; 
}

int main() {
    log("creating a");
    Thing a(1);
    log("returning b to a, overwrite a");
    a = get_thing();

    log("creating c");
    Thing c(1);
    log("copying c to a, overwrite a with copy assignment");
    a = c;
}

// output:
// creating a
// ctr_p
// returning b to a, overwrite a
// creating b
// ctr_p
// copy assignment
// dtr
// creating c
// ctr_p
// copying c to a, overwrite a with copy assignment
// copy ctr
// copy assignment
// dtr
// dtr
// dtr

~~~
