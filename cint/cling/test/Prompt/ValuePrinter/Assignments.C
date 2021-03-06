// RUN: cat %s | %cling | FileCheck %s
// XFAIL: *
int a = 12;
a // CHECK: (int) 12

const char* b = "b" // CHECK: (const char *) "b"

struct C {int d;} E = {22};
E // CHECK: (struct C) @0x{{[0-9A-Fa-f].*}}
E.d // CHECK: (int) 22

#include <string>
std::string s("xyz") 
// CHECK: (std::string) @0x{{[0-9A-Fa-f]{8,}.}}
// CHECK: c_str: "xyz"

#include <limits.h>
class Outer { 
public: 
  struct Inner { 
    enum E{ 
      A = INT_MAX,
      B = 2, 
      C = 2,
      D = INT_MIN
    } ABC; 
  }; 
};
Outer::Inner::C
// CHECK: (enum Outer::Inner::E const) @0x{{[0-9A-Fa-f].*}}
// CHECK: (Outer::Inner::E::B) ? (Outer::Inner::E::C) : (int) {{[0-9].*}} 
Outer::Inner::D
// CHECK: (enum Outer::Inner::E const) @0x{{[0-9A-Fa-f].*}}
// CHECK: (Outer::Inner::E::D) : (int) -{{[0-9].*}}

// Put an enum on the global scope
enum E{ e1 = -12, e2, e3=33, e4, e5 = 33};
e2
// CHECK: (E::e2) : (int) -11
::e1
// CHECK: (E::e1) : (int) -12
.q
