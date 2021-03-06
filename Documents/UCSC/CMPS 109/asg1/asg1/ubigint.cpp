// $Id: ubigint.cpp,v 1.16 2019-04-02 16:28:42-07 - - $
#include "ubigint.h"

bool is_neg(const vector<unsigned char> &a)
{
    if (a.size() > 1)
        return *a.begin() == '_';//negative sign
    else
        return false;
}

bool is_digit(unsigned char val) { return (val <= '9' && val >= '0'); }
int exp(int value, int power)
{
    int num = value;
    while (power > 1)
    {
        num *= value;
        power--;
    }
    return num;
}

int int_size(int value) //get number of digits in value
{
    int size = 1;
    if (value < 0) //negative value
        value *= -1;
    for (int i = 9; i < value; i *= 10)
        size++;
    return size;
}

ostream& operator <<(ostream& outs, const vector<unsigned char> & a)
{
    vector<unsigned char>::const_iterator start = a.begin();
    for (vector<unsigned char>::const_iterator i = a.end()-1; i != start; i--)
        outs<<*i;
    outs<<*start;
    return outs;
}

vector<unsigned char> int2vec(int total) //stores each digit of int as char in vector
{
    vector<unsigned char> final; //final vector
    bool negative = false;
    if (total < 0)
    {
        total *= -1;
        negative = true;
    }
    if (total < 10) //only a one digit number
    {
        if (negative)
            final.push_back('-');
        final.push_back('0'+total); //insert digit into vector
        return final;
    }
    vector<unsigned char> temp;
    int size = int_size(total)-1; //get num of digits
    int subtract = exp(10,size); //get highest tens place
    int digit = 1;
    while (size >= 0)
    {
        digit = total/subtract; //get digit
        temp.push_back('0'+(total/subtract)); //store digit as char in vector
        total -= (digit*subtract); //remove largest digit from integer
        subtract /= 10; //go to next tens place
        size--;
    }
    for (vector<unsigned char>::iterator i = temp.end()-1; i != temp.begin(); i--) //reverse vector
        final.push_back(*i);
    final.push_back(*temp.begin()); //push first element of vector
    if (negative)
        final.push_back('_'); //push negative sign
    return final;
}

int vec2int(vector<unsigned char> a)
{
    int final = 0;
    int count = 1;

    for (vector<unsigned char>::iterator i = a.begin(); i != a.end(); i++)
    {
        if (isdigit(*i))
        {
            final += (*i-'0')*count;
            count *= 10;
        }
    }
    if (is_neg(a))
        final *= -1;
    return final;
}

vector<unsigned char> vector_mul(vector<unsigned char> a, vector<unsigned char> b) // multiplies two vectors
{
    int total = vec2int(a) * vec2int(b); //multiply both vector int values
    return int2vec(total); //turn total into new vector
}

vector<unsigned char> vector_add(vector<unsigned char> a, vector<unsigned char> b) //adds two vectors
{
    int total = vec2int(a) + vec2int(b);
    return int2vec(total);
}

vector<unsigned char> vector_sub(vector<unsigned char> a, vector<unsigned char> b) //adds two vectors
{
    int total = vec2int(a) - vec2int(b);
    return int2vec(total);
}

//num = numerator, den = denominator, rem = remainder
vector<unsigned char> vector_div(vector<unsigned char>num, vector<unsigned char> den)
{
    int n = vec2int(num); //get numerator
    int d = vec2int(den); //get denominator

    int whole = n/d; //get whole number
    return int2vec(whole); //return whole number as vector
}

vector<unsigned char> vector_rem(vector<unsigned char>num,vector<unsigned char>den)
{
    int n = vec2int(num); //get numerator and denominator as integers
    int d = vec2int(den);

    int rem = n%d; // get remainder
    return int2vec(rem); //return remainder as vector
}

vector<unsigned char> operator *(const vector<unsigned char> & v, int num)
{
    return vector_mul(v,int2vec(num));
}

vector<unsigned char> operator /(const vector<unsigned char> & v, int num)
{
    return vector_div(v,int2vec(num));
}

vector<unsigned char> operator %(const vector<unsigned char> & v, int num)
{
    return vector_div(v, int2vec(num));
}

vector <unsigned char> operator *(const vector<unsigned char> & lhs, const vector<unsigned char> & rhs)
{
    return vector_mul(lhs, rhs);
}

vector <unsigned char> operator %(const vector<unsigned char> & lhs, const vector<unsigned char> &rhs)
{
    return vector_rem(lhs, rhs);
}

vector<unsigned char> operator +(const vector<unsigned char> & lhs, const vector<unsigned char> & rhs)
{
    return vector_add(lhs, rhs);
}

vector<unsigned char> operator -(const vector<unsigned char> & lhs, const vector<unsigned char> & rhs)
{
    return vector_sub(lhs, rhs);
}


/*=======================UBIGINT=========================*/

ubigint::ubigint (unsigned long that): ubig_value (that) {
   //DEBUGF ('~', this << " -> " << ubig_value)
}

ubigint::ubigint (const string& that): ubig_value(0) {
   DEBUGF ('~', "that = \"" << that << "\"");

   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
      ubig_value = ubig_value * 10 + int2vec(digit - '0');
   }
}

ubigint::ubigint(ubigvalue_t a){
    ubig_value = a;
}

ubigint ubigint::operator+ (const ubigint& that) const {
   return ubigint (ubig_value + that.ubig_value);
}

ubigint ubigint::operator- (const ubigint& that) const {
   //if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   return ubigint (ubig_value - that.ubig_value);
}

ubigint ubigint::operator* (const ubigint& that) const {
   return ubigint (ubig_value * that.ubig_value);
}

void ubigint::multiply_by_2() {
   ubig_value = ubig_value * 2;
}

void ubigint::divide_by_2() {
   ubig_value = ubig_value/2;
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
   return vector_div(ubig_value,that.ubig_value);
}

ubigint ubigint::operator% (const ubigint& that) const {
   return ubig_value%that.ubig_value;
}

bool ubigint::operator== (const ubigint& that) const {
   return ubig_value == that.ubig_value;
}

bool ubigint::operator< (const ubigint& that) const {
   return vec2int(ubig_value) < vec2int(that.ubig_value);
}

ostream& operator<< (ostream& out, const ubigint& that) {
    return out<<that.ubig_value;
   //return out << "ubigint(" << that.ubig_value << ")";
}

