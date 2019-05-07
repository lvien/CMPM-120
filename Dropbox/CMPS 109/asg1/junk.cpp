#include <iostream>
// $Id: scanner.cpp,v 1.21 2019-04-05 14:36:05-07 - - $

#include <cassert>
#include <locale>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <string>
#include <climits>
#include <vector>
#include <deque>
#include <unistd.h>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <limits>

using namespace std;

//iterstack

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

enum class tsymbol {SCANEOF, NUMBER, OPERATOR};


vector<bool> debugflags::flags (UCHAR_MAX + 1, false);

void debugflags::setflags (const string& initflags) {
   for (const unsigned char flag: initflags) {
      if (flag == '@') flags.assign (flags.size(), true);
                  else flags[flag] = true;
   }
   if (getflag ('x')) {
      string flag_chars;
      for (size_t index = 0; index < flags.size(); ++index) {
         if (getflag (index)) flag_chars += static_cast<char> (index);
      }
   }
}

//
// getflag -
//    Check to see if a certain flag is on.
//

bool debugflags::getflag (char flag) {
   return flags[static_cast<unsigned char> (flag)];
}

void debugflags::where (char flag, const char* file, int line,
                        const char* pretty_function) {
   cout << "DEBUG(" << flag << ") " << file << "[" << line << "] "
          << "   " << pretty_function << endl;
}

struct token {
   tsymbol symbol;
   string lexinfo;
   token (tsymbol sym, const string& lex = string()):
          symbol(sym), lexinfo(lex){
   }
};

class scanner {
   private:
      istream& instream;
      int nextchar {instream.get()};
      bool good() { return nextchar != EOF; }
      char get();
   public:
      scanner (istream& instream_ = cin): instream(instream_) {}
      token scan();
};

ostream& operator<< (ostream&, tsymbol);
ostream& operator<< (ostream&, const token&);

char scanner::get() {
   if (not good()) throw runtime_error ("scanner::get() past EOF");
   char currchar = nextchar;
   nextchar = instream.get();
   return currchar;
}

token scanner::scan() {
   while (good() and isspace (nextchar)) get();
   if (not good()) return {tsymbol::SCANEOF};
   if (nextchar == '_' or isdigit (nextchar)) {
      token result {tsymbol::NUMBER, {get()}};
      while (good() and isdigit (nextchar)) result.lexinfo += get();
      return result;
   }
   return {tsymbol::OPERATOR, {get()}};
}

ostream& operator<< (ostream& out, tsymbol symbol) {
   struct hasher {
      auto operator() (tsymbol sym) const {
         return static_cast<underlying_type<tsymbol>::type> (sym);
      }
   };
   static const unordered_map<tsymbol,string,hasher> map {
      {tsymbol::NUMBER  , "NUMBER"  },
      {tsymbol::OPERATOR, "OPERATOR"},
      {tsymbol::SCANEOF , "SCANEOF" },
   };
   return out << map.at(symbol);
}

ostream& operator<< (ostream& out, const token& token) {
   out << "{" << token.symbol << ", \"" << token.lexinfo << "\"}";
   return out;
}


using bigint_stack = iterstack<bigint>;

void do_arith (bigint_stack& stack, const char oper) {
   if (stack.size() < 2) throw ydc_exn ("stack empty");
   bigint right = stack.top();
   stack.pop();
   DEBUGF ('d', "right = " << right);
   bigint left = stack.top();
   stack.pop();
   DEBUGF ('d', "left = " << left);
   bigint result;
   switch (oper) {
      case '+': result = left + right; break;
      case '-': result = left - right; break;
      case '*': result = left * right; break;
      case '/': result = left / right; break;
      case '%': result = left % right; break;
      case '^': result = pow (left, right); break;
      default: throw invalid_argument ("af");
   }
   DEBUGF ('d', "result = " << result);
   stack.push (result);
}

void do_clear (bigint_stack& stack, const char) {
   DEBUGF ('d', "");
   stack.clear();
}


void do_dup (bigint_stack& stack, const char) {
   bigint top = stack.top();
   DEBUGF ('d', top);
   stack.push (top);
}

void do_printall (bigint_stack& stack, const char) {
   for (const auto& elem: stack) cout << elem << endl;
}

void do_print (bigint_stack& stack, const char) {
   if (stack.size() < 1) throw ydc_exn ("stack empty");
   cout << stack.top() << endl;
}

void do_debug (bigint_stack&, const char) {
   cout << "Y not implemented" << endl;
}

class ydc_quit: public exception {};
void do_quit (bigint_stack&, const char) {
   throw ydc_quit();
}

void do_function (bigint_stack& stack, const char oper) {
   switch (oper) {
      case '+': do_arith    (stack, oper); break;
      case '-': do_arith    (stack, oper); break;
      case '*': do_arith    (stack, oper); break;
      case '/': do_arith    (stack, oper); break;
      case '%': do_arith    (stack, oper); break;
      case '^': do_arith    (stack, oper); break;
      case 'Y': do_debug    (stack, oper); break;
      case 'c': do_clear    (stack, oper); break;
      case 'd': do_dup      (stack, oper); break;
      case 'f': do_printall (stack, oper); break;
      case 'p': do_print    (stack, oper); break;
      case 'q': do_quit     (stack, oper); break;
      default : throw ydc_exn (octal (oper) + " is unimplemented");
   }
}


//
// scan_options
//    Options analysis:  The only option is -Dflags.
//
void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            error() << "-" << static_cast<char> (optopt)
                    << ": invalid option" << endl;
            break;
      }
   }
   if (optind < argc) {
      error() << "operand not permitted" << endl;
   }
}


//ubigint
// $Id: ubigint.cpp,v 1.16 2019-04-02 16:28:42-07 - - $

class ubigint {
   friend ostream& operator<< (ostream&, const ubigint&);
   private:
      using unumber = unsigned long;
      unumber uvalue {};
   public:
      void multiply_by_2();
      void divide_by_2();

      ubigint() = default; // Need default ctor as well.
      ubigint (unsigned long);
      ubigint (const string&);

      ubigint operator+ (const ubigint&) const;
      ubigint operator- (const ubigint&) const;
      ubigint operator* (const ubigint&) const;
      ubigint operator/ (const ubigint&) const;
      ubigint operator% (const ubigint&) const;

      bool operator== (const ubigint&) const;
      bool operator<  (const ubigint&) const;
};


ubigint::ubigint (unsigned long that): uvalue (that) {
   DEBUGF ('~', this << " -> " << uvalue)
}

ubigint::ubigint (const string& that): uvalue(0) {
   DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      uvalue = uvalue * 10 + digit - '0';
   }
}

ubigint ubigint::operator+ (const ubigint& that) const {
   return ubigint (uvalue + that.uvalue);
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   return ubigint (uvalue - that.uvalue);
}

ubigint ubigint::operator* (const ubigint& that) const {
   return ubigint (uvalue * that.uvalue);
}

void ubigint::multiply_by_2() {
   uvalue *= 2;
}

void ubigint::divide_by_2() {
   uvalue /= 2;
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   return uvalue == that.uvalue;
}

bool ubigint::operator< (const ubigint& that) const {
   return uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const ubigint& that) {
   return out << "ubigint(" << that.uvalue << ")";
}

//bigint
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

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
   ubigint result = uvalue + that.uvalue;
   return result;
}

bigint bigint::operator- (const bigint& that) const {
   ubigint result = uvalue - that.uvalue;
   return result;
}


bigint bigint::operator* (const bigint& that) const {
   bigint result = uvalue * that.uvalue;
   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result = uvalue / that.uvalue;
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result = uvalue % that.uvalue;
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << "bigint(" << (that.is_negative ? "-" : "+")
              << "," << that.uvalue << ")";
}

//
// Main function.
//
int main (int argc, char** argv) {
   exec::execname (argv[0]);
   scan_options (argc, argv);
   bigint_stack operand_stack;
   scanner input;
   try {
      for (;;) {
         try {
            token lexeme = input.scan();
            switch (lexeme.symbol) {
               case tsymbol::SCANEOF:
                  throw ydc_quit();
                  break;
               case tsymbol::NUMBER:
                  operand_stack.push (bigint (lexeme.lexinfo));
                  break;
               case tsymbol::OPERATOR: {
                  char oper = lexeme.lexinfo[0];
                  do_function (operand_stack, oper);
                  break;
                  }
               default:
                  assert (false);
            }
         }catch (ydc_exn& exn) {
            cout << exn.what() << endl;
         }
      }
   }catch (ydc_quit&) {
      // Intentionally left empty.
   }
   return exec::status();
}

