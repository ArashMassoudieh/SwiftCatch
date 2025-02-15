
#pragma once

#include <map>
#include <string>
#include <vector>
#include "QuickSort.h"
#ifdef _arma
#ifdef _mlpack
#include "mlpack/core.hpp"
#endif
#include "armadillo"
#endif
//GUI
#ifdef QT_version
#include "qlist.h"
#include "qmap.h"
#include "qvariant.h"
#endif // QT_version

class CDistribution;

struct RegressionParameters {
    std::vector<double> parameters; 
    double MSE = 0; 
    double R2 = 0; 
    enum class _regress_type {linear, power, exponential} regress_type;
};

//using namespace std;

template<class T>
class CTimeSeries
{
public:
    bool structured=true;
	CTimeSeries();
	CTimeSeries(int n);
	virtual ~CTimeSeries();
	int n;
    std::string filename;
	std::string name = "";
	std::string unit = "";
	std::string defaultUnit = "";
	std::vector<std::string> unitsList;
    T interpol(const T &x) const; //interpolate at location x
	CTimeSeries MA_smooth(int span); //Moving average smoothing with span of 1+2*span
    T interpol_D(const T &x); //interpolate the distance to the next non-zero data point
    CTimeSeries interpol(std::vector<T> x); //interpolate at each value in vector x
    CTimeSeries interpol(CTimeSeries &x) const; //interpolate at times in the time axis of x
    CTimeSeries interpol(CTimeSeries *x) const; //interpolate at times in the time axis of x
	CTimeSeries(const CTimeSeries &C);
	CTimeSeries(std::string Filename); //create BTC based on the filename
    bool SetRow(int i, const double &_t, const double &value)
    {
        if (i<n)
        {
            t[i] = _t;
            C[i] = value;
            return true;
        }
        else
            return false;
    }
#ifdef _arma
    CTimeSeries(arma::mat &x, arma::mat &y); //build timeseries from arma::mat
#endif
	CTimeSeries& operator = (const CTimeSeries &C);
    CTimeSeries& operator = (const double &value);
    bool readfile(std::string); //read the values from a text file
    bool writefile(const std::string &Filename); //writes the BTC contets into a fild
    T maxC() const; //returns the maximum value
    T minC() const; //returns the minimum value
    T maxt() const;
    T mint() const;
	void setnumpoints(int); //resize the timeseries
	CTimeSeries Log(); //take the log of all the data points
    CTimeSeries Log(T min); //log(min(min,C))
    T std(); //standard deviation of C
    T mean(); //mean of C
    T percentile(T x); //x^th percentile of C
    T percentile(T x, int limit); //x^th percentile with the exception of the first "limit" data points
    T mean(int limit); // mean of the data after excluding "limit" data points
    T std(int nlimit); // standard deviation of the data after excluding "limit" data points
    T mean_log(int limit); //mean of log transformed data after excluding "limit" data points
    T integrate(); // integral of the time series
    T variance(); //calculates the variance of the value of the timeseries
    T integrate(T t); //integral from the begining to time t
    T integrate(T t1, T t2); //integral between time t1 and t2
    int lookupt(T t); // finds the index of the datapoint with time t
    T average(); //integral of time-series devided by the domail length
    T average(T t); // integral to time t devided by domain length
    T slope(); //slope of time-series at its end
	CTimeSeries<T> distribution(int n_bins, int limit); //extract the histogram of values
    bool append(T x); //appends a data point with value x
    bool append(T tt, T xx); //appends a datapoint with value xx at time tt
    void append(CTimeSeries &CC, bool continous_time=false);// appends a time-series to the time-series
    void ResizeIfNeeded(int _increment); //increases the size of the vectors more capacity is needed
	CTimeSeries& operator+=(CTimeSeries &v); //adds another time-series to the existing one
	CTimeSeries& operator%=(CTimeSeries &v); //adds another time-series by corresponding indexes
    CTimeSeries make_uniform(T increment, T t0=-999, bool asn_D=false); //create a new time-series with uniformly distributed time-axis
    CTimeSeries extract(T t1, T t2); //extracts a sub time-series from t1 to t2.
    std::vector<T> trend(); //calculate the slope based on regression
    T mean_t(); //mean of t values of data point
    CTimeSeries add_noise(T std, bool); //adds Gaussian noise to values
	void assign_D(); //Assign distances to the next non-zero values
	void clear(); // clears the time-series
    T wiggle(); //calculate oscillation
    T wiggle_corr(int _n=10);
    bool wiggle_sl(T tol);
    T maxfabs() const;
    T max_fabs;
    void knock_out(T t);
    T AutoCor1(int i=0);
	bool file_not_found = false;
    CTimeSeries getcummulative();
	CTimeSeries Exp();
	CTimeSeries fabs();
    void adjust_size();
    int t_size() {return t.size();}
    int C_size() {return C.size();}
    int D_size() {return D.size();}
	//GUI
	//QList <QMap <QVariant, QVariant>> compact() const;
    bool resize(unsigned int _size);
    unsigned int Capacity();
    CTimeSeries(T a, T b, const std::vector<T>&x);
    CTimeSeries(T a, T b, const CTimeSeries &btc);
    CTimeSeries(const std::vector<T> &t, const std::vector<T> &C);
    CTimeSeries(std::vector<T>&, int writeInterval = 1);
	bool error = false;
    T GetLastItemValue();
    T GetLastItemTime();
    T &lastD();
    T &lastC();
    T &lastt();
    bool SetT(int i, const T &value);
    bool SetC(int i, const T &value);
    bool SetD(int i, const T& value);
    T GetT(int i) const;
    T GetC(int i) const;
    T GetD(int i) const;
    unsigned int CSize() {return C.size();}
    unsigned int tSize() {return t.size();}
    unsigned int DSize() {return D.size(); }
    void AppendD(const T &value) { D.push_back(value); }
    CTimeSeries<T> inverse_cumulative_uniform(int ninitervals=100);
    CTimeSeries<T> LogTransformX();
    CTimeSeries<T> distribution(int n_bins = 40, double smoothing_span=0, int limit=0);
    CTimeSeries<T> derivative();
    std::vector<double> tToStdVector() {return t;}
    std::vector<double> ValuesToStdVector() {return C;}
    CTimeSeries<T> KernelSmooth(CDistribution *dist, int span=100);
    CTimeSeries<T> KernelSmooth(CDistribution* dist, const double &span = 1);
    RegressionParameters LinearRegress(const CTimeSeries<T> othertimeseries);
    RegressionParameters PowerRegress(const CTimeSeries<T> othertimeseries);
    CTimeSeries<T> Predict(const RegressionParameters& regression_parameters);
    T sum(); 
    T sum_squared(); 
    void CreatePeriodicStepFunction(const T &t_start, const T &t_end, const T &duration, const T &gap, const T &magnitude);
private:
    std::vector<T> t;
    std::vector<T> C;
    std::vector<T> D;
#ifdef QT_version
	CTimeSeries(QList <QMap <QVariant, QVariant>> data);
	void compact(QDataStream &data) const;
	static CTimeSeries unCompact(QDataStream &data);
#endif // QT_version

};

template<class T> T diff(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d);
template<class T> T diff_abs(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d);
template<class T> T diff_log(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d, double lowlim);
template<class T> T diff_norm(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d);
template<class T> T diff(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d, int scale);
template<class T> T diff(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d, CTimeSeries<T> Q);
template<class T> T diff2(CTimeSeries<T> *BTC_p, CTimeSeries<T> BTC_d);
template<class T> T diff2(CTimeSeries<T> BTC_p, CTimeSeries<T> *BTC_d);
template<class T> T diff2(const CTimeSeries<T> &BTC_p, const CTimeSeries<T> &BTC_d);
template<class T> T diff2(const CTimeSeries<T>* BTC_p, const CTimeSeries<T>* BTC_d);
template<class T> T diff_mixed(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d, double lowlim, double std_n, double std_ln);
template<class T> T ADD(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d);
template<class T> T diff_relative(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d, double m);
template<class T> T R2(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d);
template<class T> T Covariance(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d);
template<class T> T R2(const CTimeSeries<T> *BTC_p, const CTimeSeries<T> *BTC_d);
template<class T> T R(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d, int nlimit);
template<class T> CTimeSeries<T> operator*(T, CTimeSeries<T>&);
template<class T> CTimeSeries<T> operator*(CTimeSeries<T>&, double);
template<class T> CTimeSeries<T> operator*(CTimeSeries<T>&, CTimeSeries<T>&);
template<class T> CTimeSeries<T> operator/(CTimeSeries<T>&, CTimeSeries<T>&);
template<class T> CTimeSeries<T> operator+(CTimeSeries<T>&, CTimeSeries<T>&);
template<class T> CTimeSeries<T> operator-(CTimeSeries<T>&, CTimeSeries<T>&);
template<class T> CTimeSeries<T> operator%(CTimeSeries<T>&, CTimeSeries<T>&);
template<class T> CTimeSeries<T> operator&(CTimeSeries<T>&, CTimeSeries<T>&);
template<class T> CTimeSeries<T> operator/(CTimeSeries<T> &CTimeSeries_T, T alpha);
template<class T> CTimeSeries<T> operator>(CTimeSeries<T>& BTC1, CTimeSeries<T>& BTC2);
template<class T> T XYbar(CTimeSeries<T>& BTC_p, CTimeSeries<T> &BTC_d);
template<class T> T X2bar(CTimeSeries<T>& BTC_p, CTimeSeries<T> &BTC_d);
template<class T> T Y2bar(CTimeSeries<T>& BTC_p, CTimeSeries<T> &BTC_d);
template<class T> T Ybar(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d);
template<class T> T Xbar(CTimeSeries<T> &BTC_p, CTimeSeries<T> &BTC_d);
template<class T> CTimeSeries<T> operator+(CTimeSeries<T> &v1, CTimeSeries<T> &v2);
template<class T> T prcntl(std::vector<T> C, T x);
template<class T> std::vector<T> prcntl(std::vector<T> &C, std::vector<T> &x);
template<class T> T sgn(T val);
template<class T> T sum_interpolate(std::vector<CTimeSeries<T>>, double t);
template<class T> T R2_c(CTimeSeries<T> BTC_p, CTimeSeries<T> BTC_d);
template<class T> T norm2(CTimeSeries<T> BTC1);
template<class T> CTimeSeries<T> max(CTimeSeries<T> A, T b);
//GUI
template<class T> std::map<std::string, T> regression(std::vector<T> &x, std::vector<T> &y);

#include "BTC.hpp"
