#pragma once
#include <vector>

//using namespace std;
class CQuickSort
{
public:
	CQuickSort(void);
	~CQuickSort(void);
};

std::vector<double> QSort(const std::vector<double> &V);
std::vector<double> QbSort(const std::vector<double> &V);
std::vector<int> QSort(const std::vector<int> &V);
std::vector<int> QbSort(const std::vector<int> &V);
std::vector<double> reverse_order(const std::vector<double> &V);
std::vector<int> reverse_order(const std::vector<int> &V);
std::vector<double> bubbleSort(const std::vector<double> &V);
std::vector<int> bubbleSort(const std::vector<int> &V);
