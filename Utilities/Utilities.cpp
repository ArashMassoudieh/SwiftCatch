#include "Utilities.h"
#ifdef _WINDOWS
#include <cctype>
#include <algorithm>
#include < time.h >
#include <windows.h> 
#endif // _WINDOWS


//using namespace std;

namespace aquiutils
{

    int lookup(const std::vector<std::string>& s, const std::string& s1)
    {
        for (unsigned int i = 0; i < s.size(); i++)
            if (s[i] == s1)
                return i;
        return -1;
    }

    int lookup(const std::vector<int>& s, const int& s1)
    {
        for (unsigned int i = 0; i < s.size(); i++)
            if (s[i] == s1)
                return i;
        return -1;
    }

    int lookup(const std::vector<std::vector<int> >& s, const std::vector<int>& s1)
    {
        for (unsigned int i = 0; i < s.size(); i++)
            if (s[i] == s1)
                return i;
        return -1;
    }

    int corresponding_parenthesis(std::string S, int i)
    {
        std::string s = S;
        if (S.at(i) == '(')
        {
            int paranthesis_level = 1;
            for (unsigned int j = i + 1; j < S.size(); j++)
            {
                if (S.at(j) == '(')
                    paranthesis_level++;
                if (S.at(j) == ')')
                    paranthesis_level--;

                if (paranthesis_level == 0)
                    return j;
            }
            return -1;
        }


        if (S.at(i) == ')')
        {
            int paranthesis_level = 1;
            for (int j = i - 1; j > 0; j--)
            {
                if (S.at(j) == ')')
                    paranthesis_level++;
                if (S.at(j) == '(')
                    paranthesis_level--;

                if (paranthesis_level == 0)
                    return j;
            }
            return -1;
        }
        return -1;
    }

    int count(const std::string& s, const std::string& s1)
    {
        int out = 0;
        for (unsigned int i = 0; i < s.size() - s1.size() + 1; i++)
            if (s.substr(i, s1.size()) == s1)
                out++;
        return out;
    }

    bool parantheses_balance(std::string S)
    {
        if (count(S, "(") == count(S, ")"))
            return true;
        else
            return false;
    }

    bool contains(const std::string& s, const std::string& s1)
    {
        for (unsigned int i = 0; i < s.size() - s1.size() + 1; i++)
            if (s.substr(i, s1.size()) == s1)
                return true;
        return false;
    }

    std::string left(const std::string& s, int i)
    {
        return s.substr(0, i);
    }
    std::string right(const std::string& s, int i)
    {
        return s.substr(s.size() - i, i);
    }

    void remove(std::string& s, unsigned int i)
    {
        std::string S;
        for (unsigned int j = 0; j < s.size(); j++)
            if (i != j)
                S = S + s[j];
        s = S;
    }

    void replace(std::string& s, unsigned int i, std::string p)
    {
        std::string S;
        for (unsigned int j = 0; j < s.size(); j++)
            if (i != j)
                S = S + s[j];
            else
                S = S + p;
        s = S;
    }

    void remove(std::string& s, unsigned int pos, unsigned int len)
    {
        for (unsigned int i = pos; i < pos + len; i++)
            remove(s, pos);
    }


    bool remove(std::string& s, std::string s1)
    {
        bool found = false;
        for (unsigned int j = 0; j < s.size() - s1.size() + 1; j++)
            if (s.substr(j, s1.size()) == s1)
            {
                remove(s, j, s1.size());
                found = true;
            }
        return found;
    }

    void insert(std::string& s, unsigned int pos, std::string s1)
    {
        std::string S;

        for (unsigned int i = 0; i < s.size(); i++)
        {
            if (i == pos)
                S = S + s1;
            S = S + s[i];
        }
        if (pos == s.size()) S = S + s1;
        s = S;
    }


    bool replace(std::string& s, std::string s1, std::string s2)
    {

        bool found = false;
        std::vector<int> pos;
        unsigned int j = 0;
        while (j < s.size() - s1.size() + 1)
        {
            if (s.substr(j, s1.size()) == s1)
            {
                pos.push_back(j);
                remove(s, j, s1.size());
                found = true;
            }
            j++;
        }
        for (unsigned int j = 0; j < pos.size(); j++)
        {
            insert(s, pos[j] + j * s2.size(), s2);
        }

        return found;
    }




    void replace(std::string& s, unsigned int i, unsigned int j, std::string p)
    {
        std::string S;
        for (unsigned int k = 0; k < s.size(); k++)
            if (k == i)
                S = S + p;
            else if (k < i || k >= i + j)
                S = S + s[j];

        s = S;
    }



    bool isnumber(char S)
    {
        if ((((int)S > 47) && ((int)S < 58)) || (S == '.') || (S == '-'))
            return true;
        else
            return false;
    }

    bool isnumber(std::string S)
    {
        bool res = true;
        for (unsigned int i = 0; i < S.size(); i++)
            if (!isnumber(S[i]))
                res = false;

        return res;
    }


    bool isintegernumber(std::string S)
    {
        bool out = true;
        for (unsigned int i = 0; i < S.size(); i++)
        {
            if (((int)S[i] <= 47) || ((int)S[i] >= 58))
                out = false;
        }
        return out;
    }

    double atof(const std::string& S)
    {
        return std::atof(S.c_str());
    }
    double atoi(const std::string& S)
    {
        return std::atoi(S.c_str());
    }

    std::string trim(const std::string& s)
    {
        if (s.find_first_not_of(' ') == std::string::npos) return "";

        return s.substr(s.find_first_not_of(' '), s.find_last_not_of(' ') + 1);
    }

    void push_back(std::vector<std::string>& s, const std::vector<std::string>& s1)
    {
        for (unsigned int i = 0; i < s1.size(); i++)
            s.push_back(s1[i]);
    }

    bool isnegative(const double& x)
    {
        if (x < -SMALLNUMBER)
            return true;
        else
            return false;
    }

    bool ispositive(const double& x)
    {
        if (x > SMALLNUMBER)
            return true;
        else
            return false;
    }

    bool iszero(const double& x)
    {
        if (fabs(x) < SMALLNUMBER)
            return true;
        else
            return false;
    }



    std::vector<std::string> split(const std::string& s, char del)
    {
        unsigned int lastdel = 0;
        std::vector<std::string> strings;
        for (unsigned int i = 0; i < s.size(); i++)
        {
            if (s[i] == del)
            {
                strings.push_back(s.substr(lastdel, i - lastdel));
                lastdel = i + 1;
            }
        }
        if (lastdel < s.size() && trim(s.substr(lastdel, s.size() - lastdel)) != "\r" && trim(s.substr(lastdel, s.size() - lastdel)) != "") strings.push_back(trim(s.substr(lastdel, s.size() - lastdel)));  // works w/o trim- Trim can be deleted
        for (unsigned int i = 0; i < strings.size(); i++) strings[i] = trim(strings[i]);					// Trim can be deleted
        if (strings.size() == 1)
            if (strings[0] == "")
                strings.pop_back();
        return strings;

    }

    std::vector<std::string> getline(std::ifstream& file)
    {
        std::string line;

        while (!file.eof())
        {
            std::getline(file, line);
            return split(line, ',');
        }
        std::vector<std::string> x;
        return x;
    }

    std::vector<std::string> getline(std::ifstream& file, char del1)
    {
        std::string line;

        while (!file.eof())
        {
            std::getline(file, line);
            return split(line, del1);
        }
        std::vector<std::string> x;
        return x;
    }

    std::vector<std::vector<std::string>> getline_op(std::ifstream& file, char del1)
    {
        std::string line;
        std::vector<std::vector<std::string>> s;
        std::vector<std::string> ss;
        while (file.good())
        {
            getline(file, line);
            ss = split(line, ',');
            for (unsigned int i = 0; i < ss.size(); i++)
                s.push_back(split(ss[i], del1));
        }
        return s;

    }

    std::vector<std::string> split(const std::string& s, const std::vector<char>& del)
    {
        unsigned int lastdel = 0;
        unsigned int j = 0;
        std::vector<std::string> strings;
        for (unsigned int i = 0; i < s.size(); i++)
        {
            for (unsigned int jj = 0; jj < del.size(); jj++)
                if (s[i] == del[jj])
                {
                    strings.push_back(s.substr(lastdel, i - lastdel));
                    lastdel = i + 1;
                    j++;
                }
        }
        if (lastdel < s.size()) strings.push_back(trim(s.substr(lastdel, s.size() - lastdel)));
        for (unsigned int i = 0; i < strings.size(); i++) strings[i] = trim(strings[i]);
        return strings;

    }

    std::vector<std::vector<std::string>> getline_op(std::ifstream& file, std::vector<char> del1)
    {
        std::string line;
        std::vector<std::vector<std::string>> s;
        std::vector<std::string> ss;
        while (file.good())
        {
            getline(file, line);
            ss = split(line, ',');
            for (unsigned int i = 0; i < ss.size(); i++)
                s.push_back(split(ss[i], del1));
        }
        return s;
    }




    std::vector<std::vector<std::string>> getline_op_eqplus(std::ifstream& file)
    {
        std::vector<char> del1;
        del1.push_back('=');
        del1.push_back('+');
        std::string line;
        std::vector<std::vector<std::string>> s;
        std::vector<std::string> ss;
        while (file.good())
        {
            getline(file, line);
            ss = split(line, ',');
            for (unsigned int i = 0; i < ss.size(); i++)
                s.push_back(split(ss[i], del1));
        }
        return s;


    }


    std::vector<std::string> split_curly_semicolon(std::string s)
    {
        std::vector<char> del2; del2.push_back('{'); del2.push_back('}'); del2.push_back(';');
        return split(s, del2);
    }

    std::vector<int> look_up(std::string s, char del)  //Returns a vector with indices of "del"
    {
        std::vector<int> out;
        for (unsigned int i = 0; i < s.size(); i++)
            if (s[i] == del)
                out.push_back(i);

        return out;

    }

    std::vector<int> ATOI(std::vector<std::string> ii)
    {
        std::vector<int> res;
        for (unsigned int i = 0; i < ii.size(); i++)
            res.push_back(atoi(ii[i].c_str()));

        return res;
    }

    std::vector<double> ATOF(std::vector<std::string> ii)
    {
        std::vector<double> res;
        for (unsigned int i = 0; i < ii.size(); i++)
            res.push_back(atof(ii[i].c_str()));

        return res;
    }


    std::string tolower(const std::string& S)
    {
        std::string SS = S;
        for (unsigned int i = 0; i < S.size(); i++)
        {
            SS[i] = std::tolower(S[i]);
        }
        return SS;
    }

    std::vector<std::string> tolower(const std::vector<std::string>& S)
    {
        std::vector<std::string> SS = S;
        for (unsigned int i = 0; i < S.size(); i++)
        {
            SS[i] = tolower(S[i]);
        }
        return SS;
    }

    void writeline(std::ofstream& f, std::vector<std::string> s, std::string del)
    {
        for (unsigned int i = 0; i < s.size() - 1; i++)
            f << s[i] << del;
        f << s[s.size() - 1] << std::endl;
    }

    void writeline(std::ofstream& f, std::vector<std::vector<std::string>> s, std::string del, std::string del2)
    {
        for (unsigned int i = 0; i < s.size() - 1; i++)
        {
            for (unsigned int j = 0; j < s[i].size() - 1; j++)
                f << s[i][j] << del2;
            f << s[i][s[i].size() - 1] << del;
        }
        f << s[s.size() - 1][s[s.size() - 1].size() - 1] << std::endl;
    }
    void writestring(std::ofstream& f, std::string s)
    {
        f << s;
    }

    void writestring(std::string filename, std::string s)
    {
        std::ofstream file(filename);
        file << s + "\n";
        file.close();

    }
    void writenumber(std::ofstream& f, double s)
    {
        f << s;
    }

    void writeendl(std::ofstream& f)
    {
        f << std::endl;
    }

    double Heavyside(double x)
    {
        if (x > 0) return 1; else return 0;
    }

    double Pos(double x)
    {
        if (x > 0) return x; else return 0;
    }

    double Neg(double x)
    {
        return Pos(-x);
    }


    std::string numbertostring(const double& x, bool scientific)
    {
        std::string Result;          // std::string which will contain the result
        std::ostringstream convert;   // stream used for the conversion
        if (scientific)
            convert << std::scientific;
        convert << x;      // insert the textual representation of 'Number' in the characters in the stream
        Result = convert.str();
        return Result;
    }

    std::string numbertostring(std::vector<double>& x, bool scientific)
    {
        std::string Result = "[";
        for (int i = 0; i < x.size() - 1; i++)
            Result += numbertostring(x[i], scientific) + ",";
        Result += numbertostring(x[x.size() - 1], scientific) + "]";
        return Result;
    }

    std::string numbertostring(int x)
    {
        std::string Result;          // std::string which will contain the result
        std::ostringstream convert;   // stream used for the conversion
        convert << x;      // insert the textual representation of 'Number' in the characters in the stream
        Result = convert.str();
        return Result;
    }

    std::string numbertostring(unsigned int x)
    {
        std::string Result;          // std::string which will contain the result
        std::ostringstream convert;   // stream used for the conversion
        convert << x;      // insert the textual representation of 'Number' in the characters in the stream
        Result = convert.str();
        return Result;
    }

    std::string numbertostring(unsigned int x, int number_of_digits)
    {
        std::string Result;          // std::string which will contain the result
        std::ostringstream convert;   // stream used for the conversion
        convert << x;      // insert the textual representation of 'Number' in the characters in the stream

        Result = convert.str();
        if (Result.size() < number_of_digits)
        {
            for (int i = 0; i < Result.size() - number_of_digits; i++)
                Result = "0" + Result;
        }
        return Result;
    }

    std::string numbertostring(std::vector<int>& x, bool scientific)
    {
        std::string Result = "[";
        if (x.size() > 0)
        {
            for (int i = 0; i < x.size() - 1; i++)
                Result += numbertostring(x[i]) + ",";
            Result += numbertostring(x[x.size() - 1]) + "]";
        }
        else
            Result += "]";
        return Result;
    }

    std::string tail(std::string const& source, size_t const length) {
        if (length >= source.size()) { return source; }
        return source.substr(source.size() - length);
    } // tail

    std::string tabs(int i)
    {
        std::string out;
        for (int j = 0; j < i; j++)
            out += "\t";
        return out;
    }

    bool And(std::vector<bool> x) { bool out = true;  for (int i = 0; i < x.size(); i++) out &= x[i]; return out; }
    //double max(const std::vector<double> &x) { double out = -1e+24;  for (int i = 0; i < x.size(); i++) out=std::max(out, x[i]); return out; }
    /*int max(const std::vector<int>& x)
    {	int out = -37000;
        for (int i = 0; i < x.size(); i++)
            out=std::max(out, x[i]);
        return out;

    }*/
    double Max(const std::vector<double>& x) { double out = -1e+24;  for (int i = 0; i < x.size(); i++) out = std::max(out, x[i]); return out; }
    double Min(const std::vector<double>& x) { double out = 1e+24;  for (int i = 0; i < x.size(); i++) out = std::min(out, x[i]); return out; }
    int MaxElement(const std::vector<double>& x)
    {
        double out = -1e+24;
        int max_elem = -1;
        for (unsigned int i = 0; i < x.size(); i++)
        {
            if (x[i] > out)
            {
                out = x[i];
                max_elem = i;
            }
        }
        return max_elem;
    }
    int MinElement(const std::vector<double>& x)
    {
        double out = 1e+24;
        int min_elem = -1;
        for (unsigned int i = 0; i < x.size(); i++)
        {
            if (x[i] < out)
            {
                out = x[i];
                min_elem = i;
            }
        }
        return min_elem;
    }

    int Max(std::vector<int> x)
    {
        int out = -37000;
        for (int i = 0; i < x.size(); i++)
            out = std::max(out, x[i]);
        return out;

    }

    std::string remove_backslash_r(const std::string& ss)
    {
        std::string s = ss;
        if (!s.empty() && s[s.size() - 1] == '\r')
            s.erase(s.size() - 1);
        return s;

    }

    std::string GetOnlyFileName(const std::string& fullfilename)
    {
        std::vector<char> del;
        del.push_back('/');
        del.push_back('\\');
        std::vector<std::string> splittedbyslash = split(fullfilename, del);
        return splittedbyslash[splittedbyslash.size() - 1];

    }

    double avg(double x, double y, std::string type)
    {
        if (type == "arithmetic")
            return 0.5 * (x + y);
        if (type == "geometric")
            return sqrt(x * y);
        if (type == "harmonic")
            return (2.0 * x * y / (x + y));
        else
            return 0.5 * (x + y);
    }

    std::vector<unsigned int> Rank(const std::vector<double>& v)
    {
        std::vector<unsigned int> out(v.size());
        for (unsigned int i = 0; i < v.size(); i++)
        {
            for (unsigned int j = 0; j < v.size(); j++)
            {
                if (v[i] > v[j])
                    out[i]++;
            }
        }
        return out;
    }

}

////#ifdef _WINDOWS
////
////    int gettimeofday(struct timeval* tv, struct timezone* tz)
////    {
////        FILETIME ft;
////        unsigned __int64 tmpres = 0;
////        static int tzflag;
////
////        if (NULL != tv)
////        {
////            GetSystemTimeAsFileTime(&ft);
////
////            tmpres |= ft.dwHighDateTime;
////            tmpres <<= 32;
////            tmpres |= ft.dwLowDateTime;
////
////            /*converting file time to unix epoch*/
////            tmpres -= DELTA_EPOCH_IN_MICROSECS;
////            tmpres /= 10;  /*convert into microseconds*/
////            tv->tv_sec = (long)(tmpres / 1000000UL);
////            tv->tv_usec = (long)(tmpres % 1000000UL);
////        }
////
////        if (NULL != tz)
////        {
////            if (!tzflag)
////            {
////                _tzset();
////                tzflag++;
////            }
////            tz->tz_minuteswest = _timezone / 60;
////            tz->tz_dsttime = _daylight;
////        }
////
////        return 0;
////    }
////}
////
////#endif

