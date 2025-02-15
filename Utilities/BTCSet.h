#pragma once
#include "BTC.h"
#include <vector>
#include "Vector.h"

using namespace arma;

template <class T>
class CTimeSeriesSet
{
public:
	CTimeSeriesSet(void); //default constructor
	CTimeSeriesSet(int n); //construction with number of variables (timeseries)
	CTimeSeriesSet(int numberofBTCs, int sizeofBTCvector);
    CTimeSeriesSet(const CTimeSeriesSet<T> &BTC);
    CTimeSeriesSet(const CTimeSeries<T> &BTC);
	CTimeSeriesSet(std::string filename, bool varytime);
    bool ReadFromFile(std::string _filename, bool varytime);
    void SetNumberofColumns(int numberofBTCs);
    int nvars;
    std::string filename;
    std::vector<CTimeSeries<T>> BTC;
	void writetofile(char outputfile[]);
	int maxnumpoints();
	CTimeSeriesSet& operator = (const CTimeSeriesSet &C);
	std::vector<std::string> names;
	bool unif = false;
    bool writetofile(std::string outputfile, bool writeColumnHeaders = false);
	void writetofile(std::string outputfile, int writeinterval);
    std::vector<T> interpolate(T t);
    std::vector<T> interpolate(T t, int fnvars);
	void getfromfile(std::string filename, bool varytime);
    T maxtime() const;
    T mintime() const;
    T maxval() const;
    T minval() const;
    std::vector<T> getrandom();
    std::vector<T> percentile(T x);
    std::vector<T> mean(int limit);
    std::vector<T> mean(int limit, std::vector<int> index);
    std::vector<T> std(int limit);
    std::vector<T> std(int limit, std::vector<int> index);
	CMatrix correlation(int limit, int n);
    std::vector<T> integrate();
    std::vector<T> average();
    std::vector<T> percentile(T x, int limit);
    std::vector<T> percentile(T x, int limit, std::vector<int> index);
    std::vector<T> getrandom(int burnin);
    void append(T t, std::vector<T> c);
    CTimeSeries<T> add(std::vector<int> ii);
    CTimeSeries<T> add_mult(std::vector<int> ii, std::vector<T> mult);
    CTimeSeries<T> add_mult(std::vector<int> ii, CTimeSeriesSet &mult);
    CTimeSeries<T> divide(int ii, int jj);
    CTimeSeriesSet make_uniform(T increment, T t0=-999, bool assgn_d=true);
    CTimeSeriesSet getpercentiles(std::vector<T> percents);
    CVector out_of_limit(T limit);
	CTimeSeriesSet distribution(int n_bins, int n_columns, int limit);
    CTimeSeriesSet add_noise(std::vector<T> std, bool logd);
	void clear();
    std::vector<T> max_wiggle();
    std::vector<T> max_wiggle_corr(int _n = 10);
    std::vector<int> max_wiggle_sl(int ii, T tol);
    CTimeSeriesSet getflow (T A);
    void knockout(T t);
	int lookup(std::string S);
    std::vector<T> getrow(int a);
	void setname(int i, std::string name);
    void resize(unsigned int _size);
    void ResizeIfNeeded(unsigned int _increment);
    void adjust_size();
    bool file_not_found=false;
    CTimeSeries<T> &operator[](int index);
    CTimeSeries<T> &operator[](std::string BTCName);
    bool Contains(std::string BTCName);
    CTimeSeriesSet<T> ConverttoNormalScore();
    CTimeSeriesSet<T> AutoCorrelation(const double &span, const double &increment);
    bool SetRow(int i, const T &t, const std::vector<T> &c);
#ifdef _ARMA
    arma::mat ToArmaMat(const vector<string> &columns = vector<string>());
    arma::mat ToArmaMat(const vector<int> &columns);
    CTimeSeriesSet(const mat &m, const double &dt, const vector<vector<int>> &lag = vector<vector<int>>());
    static CTimeSeriesSet OutputShifter(const mat &m, const double &dt, const vector<vector<int>> &lag);
    arma::mat ToArmaMatShifter(const vector<int> &columns, const vector<vector<int>> &lag);
    arma::mat ToArmaMatShifterOutput(const vector<int> &columns, const vector<vector<int>> &lag);
    static vector<CTimeSeriesSet<T>> GetFromArmaMatandSplit(const arma::mat &Mat, const double &dt, const vector<vector<int>> &lag, const vector<int> &splitsizes);
#endif
    vector<CTimeSeriesSet<T>> Split(const vector<int> &splitsizes);
#ifdef QT_version
	CTimeSeries &operator[](Qstd::string BTCName) {
		return operator[](BTCName.toStdString());}
#endif // QT_version
    CTimeSeriesSet(std::vector < std::vector<T>> &, int writeInterval = 1);
	int indexOf(const std::string& name) const;
	void pushBackName(std::string name);
    void append(const CTimeSeries<T> &BTC, std::string name = "");
    void append(const CTimeSeriesSet<T> &TS);
    bool merge(CTimeSeriesSet<T> &TS, bool continous_time=false);
	CTimeSeriesSet sort(int burnOut = 0);
#ifdef QT_version
	void compact(QDataStream &data) const;
	static CTimeSeriesSet unCompact(QDataStream &data);
#endif // QT_version
	~CTimeSeriesSet(void);
};

template <class T> T diff(CTimeSeriesSet<T> B1, CTimeSeriesSet<T> B2);
template <class T> CTimeSeriesSet<T> operator * (const CTimeSeriesSet<T> &BTC, const double &C);
template <class T> CVector norm2dif(CTimeSeriesSet<T> &A, CTimeSeriesSet<T> &B);
template <class T> CTimeSeriesSet<T> merge(CTimeSeriesSet<T> A, const CTimeSeriesSet<T> &B);
template <class T> CTimeSeriesSet<T> merge(std::vector<CTimeSeriesSet<T>> &A);
template <class T> CVector sum_interpolate(std::vector<CTimeSeriesSet<T>> &BTC, double t);
template <class T> T sum_interpolate(std::vector<CTimeSeriesSet<T>> &BTC, T t, std::string name);
template <class T> int max_n_vars(std::vector<CTimeSeriesSet<T>> &BTC);

#include "BTCSet.hpp"
