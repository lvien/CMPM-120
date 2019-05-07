#include <iostream>
#include <cstdlib>
#include <vector>
#include <cassert>
#include <deque>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <unistd.h>
#include <cmath>
#include "test.h"

using namespace std;
void print_vector(vector<unsigned char> a);
void print_vector(vector<unsigned char> a)
{
    vector<unsigned char>::iterator end = a.end();
    for (vector<unsigned char>::iterator i = a.begin(); i != end; i++)
        cout<<*i;
}

int main(int argc, char *argv[])
{
    bigint A ("33");
    bigint B ("_33");
    cout<<"A: "<<A<<endl;
    cout<<"B: "<<B<<endl;
    cout<<"A / B: "<<(A * B)<<endl;

    cout<<5*10+6-1<<endl;
}

