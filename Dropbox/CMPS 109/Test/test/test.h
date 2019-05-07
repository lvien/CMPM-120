#ifndef TEST_H
#define TEST_H

#include <cassert>
#include <deque>
#include <unordered_map>
#include <unistd.h>
#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <utility>
#include <cctype>
#include <cstdlib>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
/*
int exp(int value, int power);
int int_size(int value);
vector<unsigned char> int2vec(int value);
void print_vector(vector<unsigned char> a);
vector<unsigned char> vector_mul(vector<unsigned char> a, vector<unsigned char> b);
vector<unsigned char> vector_div(vector<unsigned char> num, vector<unsigned char> den, vector<unsigned char> &rem);
int vec2int (vector<unsigned char> a);
bool is_digit(unsigned char val);

*/
/*=======================UBIGINT=========================*/

class ubigint {
   friend ostream& operator<< (ostream&, const ubigint&);
   private:
      using udigit_t = unsigned char;
      using ubigvalue_t = vector<udigit_t>;
      ubigvalue_t ubig_value;
   public:

      void multiply_by_2();
      void divide_by_2();

      ubigint() = default; // Need default ctor as well.
      ubigint (unsigned long);
      ubigint (const string&);
      ubigint (ubigvalue_t a);
      ubigint operator+ (const ubigint&) const;
      ubigint operator- (const ubigint&) const;
      ubigint operator* (const ubigint&) const;
      ubigint operator/ (const ubigint&) const;
      ubigint operator% (const ubigint&) const;


      bool operator== (const ubigint&) const;
      bool operator<  (const ubigint&) const;
};

/*=======================BIGINT=========================*/

class bigint {
   friend ostream& operator<< (ostream&, const bigint&);
   private:
      ubigint uvalue;
      bool is_negative {false};
   public:

      bigint() = default; // Needed or will be suppressed.
      bigint (long);
      bigint (const ubigint&, bool is_negative = false);
      explicit bigint (const string&);

      bigint operator+() const;
      bigint operator-() const;

      bigint operator+ (const bigint&) const;
      bigint operator- (const bigint&) const;
      bigint operator* (const bigint&) const;
      bigint operator/ (const bigint&) const;
      bigint operator% (const bigint&) const;

      bool operator== (const bigint&) const;
      bool operator<  (const bigint&) const;
};

/*=======================RELOPS=========================*/

template <typename value>
inline bool operator!= (const value& left, const value& right) {
   return not (left == right);
}

template <typename value>
inline bool operator>  (const value& left, const value& right) {
   return right < left;
}

template <typename value>
inline bool operator<= (const value& left, const value& right) {
   return not (right < left);
}

template <typename value>
inline bool operator>= (const value& left, const value& right) {
   return not (left < right);
}

/*=======================DEBUG=========================*/



//
// debug -
//    static class for maintaining global debug flags.
// setflags -
//    Takes a string argument, and sets a flag for each char in the
//    string.  As a special case, '@', sets all flags.
// getflag -
//    Used by the DEBUGF macro to check to see if a flag has been set.
//    Not to be called by user code.
//
class debugflags {
   private:
      static vector<bool> flags;
   public:
      static void setflags (const string& optflags);
      static bool getflag (char flag);
      static void where (char flag, const char* file, int line,
                         const char* pretty_function);
};

//
// DEBUGF -
//    Macro which expands into trace code.  First argument is a
//    trace flag char, second argument is output code that can
//    be sandwiched between <<.  Beware of operator precedence.
//    Example:
//       DEBUGF ('u', "foo = " << foo);
//    will print two words and a newline if flag 'u' is  on.
//    Traces are preceded by filename, line number, and function.
//
#define DEBUGF(FLAG,CODE) { \
           if (debugflags::getflag (FLAG)) { \
              debugflags::where (FLAG, __FILE__, __LINE__, \
                                 __PRETTY_FUNCTION__); \
              cerr << CODE << endl; \
           } \
        }
#define DEBUGS(FLAG,STMT) { \
           if (debugflags::getflag (FLAG)) { \
              debugflags::where (FLAG, __FILE__, __LINE__, \
                                 __PRETTY_FUNCTION__); \
              STMT; \
           } \
        }

template <typename value_type>
class iterstack: private vector<value_type> {
   private:
      using stack_t = vector<value_type>;
      using stack_t::crbegin;
      using stack_t::crend;
      using stack_t::push_back;
      using stack_t::pop_back;
      using stack_t::back;
      using const_iterator = typename stack_t::const_reverse_iterator;
   public:
      using stack_t::clear;
      using stack_t::empty;
      using stack_t::size;
      inline const_iterator begin() {return crbegin();}
      inline const_iterator end() {return crend();}
      inline void push (const value_type& value) {push_back (value);}
      inline void pop() {pop_back();}
      inline const value_type& top() const {return back();}
};
#endif // TEST_H
