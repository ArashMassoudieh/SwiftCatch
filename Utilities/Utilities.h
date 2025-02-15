#pragma once
#ifndef UTILITIES_H
#define UTILITIES_H

#undef _HAS_STD_BYTE

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <sstream>

#define SMALLNUMBER 1e-23
#define PI 3.14159265359

/*#ifdef  _WINDOWS
struct timezone
{
    int  tz_minuteswest; 
    int  tz_dsttime;     
};
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

#endif //  _WINDOWS
*/


//using namespace std;

namespace aquiutils
{

    int lookup(const std::vector<std::string> &s, const std::string &s1);
    int lookup(const std::vector<int> &s, const int &s1);
    int lookup(const std::vector<std::vector<int> > &s, const std::vector<int> &s1);
    int corresponding_parenthesis(std::string S, int i);
    int count(const std::string &s, const std::string &s1);
    bool parantheses_balance(std::string S);
    bool contains(const std::string &s, const std::string &s1);
    std::string left(const std::string &s, int i);
    std::string right(const std::string &s, int i);
    void remove(std::string &s,unsigned int i);
    void replace(std::string &s,unsigned int i,std::string p);
    void remove(std::string &s,unsigned int pos, unsigned int len);
    bool remove(std::string &s, std::string s1);
    void insert(std::string &s, unsigned int pos, std::string s1);
    bool replace(std::string &s,std::string s1, std::string s2);
    void replace(std::string &s,unsigned int i, unsigned int j, std::string p);
    bool isnumber(char S);
    bool isnumber(std::string S);
    bool isintegernumber(std::string S);
    double atof(const std::string &S);
    double atoi(const std::string &S);
    std::string trim(const std::string &s);
    void push_back(std::vector<std::string> &s, const std::vector<std::string> &s1);
    bool isnegative(const double &x);
    bool ispositive(const double &x);
    bool iszero(const double &x);
    std::vector<std::string> split(const std::string &s, char del);
    std::vector<std::string> getline(std::ifstream& file);
    std::vector<std::string> getline(std::ifstream& file, char del1);
    std::vector<std::vector<std::string>> getline_op(std::ifstream& file,char del1);
    std::vector<std::string> split(const std::string &s, const std::vector<char> &del);
    std::vector<std::vector<std::string>> getline_op(std::ifstream& file,std::vector<char> del1);
    std::vector<std::vector<std::string>> getline_op_eqplus(std::ifstream& file);
    std::vector<std::string> split_curly_semicolon(std::string s);
    std::vector<int> look_up(std::string s, char del);  //Returns a vector with indices of "del"
    std::vector<int> ATOI(std::vector<std::string> ii);
    std::vector<double> ATOF(std::vector<std::string> ii);
    std::string tolower(const std::string &S);
    std::vector<std::string> tolower(const std::vector<std::string> &S);
    void writeline(std::ofstream& f, std::vector<std::string> s, std::string del=",");
    void writeline(std::ofstream& f, std::vector<std::vector<std::string>> s, std::string del=",", std::string del2="&");
    void writestring(std::ofstream& f, std::string s);
    void writestring(std::string filename, std::string s);
    void writenumber(std::ofstream& f, double s);
    void writeendl(std::ofstream& f);
    double Heavyside(double x);
    double Pos(double x);
    double Neg(double x);
    std::string numbertostring(const double &x, bool scientific=false);
    std::string numbertostring(std::vector<double> &x, bool scientific=false);
    std::string numbertostring(int x);
    std::string numbertostring(unsigned int x);
    std::string numbertostring(unsigned int x, int number_of_digits);
    std::string numbertostring(std::vector<int> &x, bool scientific=false);
    std::string tail(std::string const& source, size_t const length);
    std::string tabs(int i);
    bool And(std::vector<bool> x);
    // max(const std::vector<double> &x);
    //int max(const std::vector<int> &x);
    double Max(const std::vector<double>&);
    double Min(const std::vector<double>&);
    int MaxElement(const std::vector<double> &x);
    int MinElement(const std::vector<double> &x);

    int Max(std::vector<int>);
    std::string remove_backslash_r(const std::string &ss);
    std::string GetOnlyFileName(const std::string &fullfilename);
    double avg(double x, double y, std::string type="arithmetic");
    template<class T>
    T randompick(const std::vector<T> &vec)
    {
        unsigned int i=rand()%vec.size();
        return vec[i];
    };
    template<typename T> bool isfinite(T arg);
    template<typename T> T sum(std::vector<T> vec)
    {
        T out=0;
        for (int i=0; i<vec.size(); i++)
        {
            out+=vec[i];
        }
        return out;
    }
    template<typename T> bool isfinite(T arg)
    {
        if (arg==arg)
            return true;
        else
            return false;
    }
    std::vector<unsigned int> Rank(const std::vector<double> &v);
    template<typename T, typename T1, typename T2>
    int CountLessThan(const std::vector<T> &v, const T1 &x, T2 boundary=0, bool fromend=false)
    {
        int out = 1;
        if (boundary==0 && !fromend)
        {
            boundary = v.size();
        }
        if (!fromend)
            for (unsigned int i=0; i<boundary; i++)
            {
                if (v[i]<x)
                    out++;
            }
        else
            for (unsigned int i=boundary; i<v.size(); i++)
            {
                if (v[i]<x)
                    out++;
            }
        return out;
    }
    template<typename T, typename T1, typename T2>
    int CountGreaterThan(const std::vector<T> &v, const T1 &x, T2 boundary=0, bool fromend=false)
    {
        int out = 0;
        if (boundary==0 && !fromend)
        {
            boundary = v.size();
        }
        if (!fromend)
            for (unsigned int i=0; i<boundary; i++)
            {
                if (v[i]>x)
                    out++;
            }
        else
            for (unsigned int i=boundary; i<v.size(); i++)
            {
                if (v[i]>x)
                    out++;
            }
        return out;
    }
//#ifdef _WINDOWS
//    int gettimeofday(struct timeval* tv, struct timezone* tz);
//#endif


}


#endif // UTILITIES_H
