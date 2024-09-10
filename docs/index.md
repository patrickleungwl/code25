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

*** Update in progress ***

1. Resource leaks?  Show valgrind output.
2. Less repetitive code.  Fewer steps with CAS.

 
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

