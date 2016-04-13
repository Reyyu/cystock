#ifndef UPDATE_DATA_H
#define UPDATE_DATA_H
#include "common.h"
LPSTR GetInterNetURLText(LPSTR lpcInterNetURL, ofstream &outfile);
LPSTR GetInterNetURLText_string(LPSTR lpcInterNetURL, string &outstring);
bool updateStockDataFromYahoo(std::string strId, int months = 3);
bool updateStockDataToday(std::string strId);



#endif //UPDATE_DATA_H
