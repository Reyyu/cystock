#include "updateData.h"
#include <sstream>
using namespace std;

bool updateStockDataFromYahoo(string strId, int months){
	if (strId.empty()){
		cout << strId << "updateStockDataFromYahoo error: 股票代码为空" << endl;
		return false;
	}

	//更新数据
	ifstream infile;
	infile.open(strId);
	if (infile.is_open()){
		bool bAlreadyDownloaded = false;

		//比较当前时间与文件最后更新时间
		WIN32_FIND_DATA ffd;
		SYSTEMTIME timeUpdate = { 0 };
		SYSTEMTIME timeCurrent;
		FILETIME time_fileLocal;
		GetLocalTime(&timeCurrent);
		HANDLE hFind = FindFirstFile(strId.c_str(), &ffd);
		FileTimeToLocalFileTime(&ffd.ftLastWriteTime, &time_fileLocal);
		FileTimeToSystemTime(&time_fileLocal, &timeUpdate);

		if (timeUpdate.wYear == timeCurrent.wYear && timeUpdate.wMonth == timeCurrent.wMonth){
			if (timeUpdate.wDay == timeCurrent.wDay && abs(timeUpdate.wHour - timeCurrent.wHour) <= TIME_UPDATEDATA){
				bAlreadyDownloaded = true;
			}
		}
		infile.close();

		if (bAlreadyDownloaded && (ffd.nFileSizeLow != 0 || ffd.nFileSizeHigh != 0)){
			return true;
		} 
	}

	//合成URL
	//上证综指代码：000001.ss，深证成指代码：399001.SZ，沪深300代码：000300.ss
	//http://table.finance.yahoo.com/table.csv?s=000001.sz&g=d&a=8&b=20&c=2015&ignore=.csv
	//20130905至今的数据
	//http://table.finance.yahoo.com/table.csv?s=ibm&d=8&e=5&f=2013&g=d&a=11&b=16&c=1991&ignore=.csv
	//上面的链接可以抓取IBM股票从1991年10月16日起到2013年9月5日的数据。
	stringstream ss;
	ss << "http://table.finance.yahoo.com/table.csv?s=" << strId << ".";
	if (strId[0] == '3' || strId[0] == '0')
		ss << "sz";
	else if (strId[0] == '6')
		ss << "ss";
	else{
		cout << strId << " 股票代码不合法" << endl;
		return false;
	}

	SYSTEMTIME timeCurrent;
	GetLocalTime(&timeCurrent);
	int y = timeCurrent.wYear, m = timeCurrent.wMonth - months, d = 1;
	if (m <= 0){
		y--;
		m = m + 12; //不用太过精确，取个大一点的值
	}
	// "&g=d&a=11&b=1&c=2015&ignore=";
	ss << "&g=d&a=" << m-1 << "&b=" << d << "&c=" << y << "&ignore="; //m-1


	Sleep(1000);
	ofstream outfile;
	outfile.open(strId);
	if (outfile.is_open()){
		string strUrl = ss.str();
		char *url = new char[strUrl.size() + 1];
		strcpy_s(url, strUrl.size() + 1, strUrl.c_str());
		GetInterNetURLText(url, outfile);
		outfile.close();
	}

	return true;
}

bool updateStockDataToday(std::string strId){
	if (strId.empty()){
		cout << strId << "updateStockDataToday error: 股票代码为空" << endl;
		return false;
	}

	ifstream infile;
	infile.open(strId);
	int y = 0, m = 0, d = 0, y1 = 0, m1 = 0, d1 = 0;
	if (!infile.is_open()){
		return false;
	}

	string strBuff;
	getline(infile, strBuff);
	getline(infile, strBuff);
	infile.close();
	vector<string> vec = splitEx(strBuff, ",");

	//2016-02-16
	if (vec.size() <= 0 || vec[0].size() != 10){
		return false;
	}
	string strYear(vec[0], 0, 4);
	string strMonth(vec[0], 5, 2);
	string strDay(vec[0], 8, 2);

	stringstream ss;
	ss << strYear;
	ss >> y;
	ss.str("");
	ss.clear();
	ss << strMonth;
	ss >> m;
	ss.str("");
	ss.clear();
	ss << strDay;
	ss >> d;

	//合成URL
	//http://hq.sinajs.cn/list=sh601006
	ss.str("");
	ss.clear();
	ss << "http://hq.sinajs.cn/list=";
	if (strId[0] == '3' || strId[0] == '0')
		ss << "sz";
	else if (strId[0] == '6')
		ss << "sh";
	else{
		cout << strId << "updateStockDataToday error: 股票代码不合法" << " " << strId << endl;
		return false;
	}
	ss << strId;

	string strLatest;
	string strUrl = ss.str();
	char *url = new char[strUrl.size() + 1];
	strcpy_s(url, strUrl.size() + 1, strUrl.c_str());
	GetInterNetURLText_string(url, strLatest);


	//var hq_str_sz300499="高澜股份,39.610,36.010,39.610,39.610,39.610,39.610,0.000,
	//17600,697136.000,4803800,39.610,10900,39.600,600,39.590,1100,39.580,700,39.570,
	//0,0.000,0,0.000,0,0.000,0,0.000,0,0.000,2016-02-17,15:05:57,00";

	//0：”大秦铁路”，股票名字；
	//1：”27.55″，今日开盘价；
	//2：”27.25″，昨日收盘价；
	//3：”26.91″，当前价格；
	//4：”27.55″，今日最高价；
	//5：”26.20″，今日最低价；
	//6：”26.91″，竞买价，即“买一”报价；
	//7：”26.92″，竞卖价，即“卖一”报价；
	//8：”22114263″，成交的股票数，由于股票交易以一百股为基本单位，所以在使用时，通常把该值除以一百；
	//9：”589824680″，成交金额，单位为“元”，为了一目了然，通常以“万元”为成交金额的单位，所以通常把该值除以一万；
	//10：”4695″，“买一”申请4695股，即47手；
	//11：”26.91″，“买一”报价；
	//12：”57590″，“买二”
	//13：”26.90″，“买二”

	//14：”14700″，“买三”
	//15：”26.89″，“买三”
	//16：”14300″，“买四”
	//17：”26.88″，“买四”
	//18：”15100″，“买五”
	//19：”26.87″，“买五”
	//20：”3100″，“卖一”申报3100股，即31手；
	//21：”26.92″，“卖一”报价
	//(22, 23), (24, 25), (26, 27), (28, 29)分别为“卖二”至“卖四的情况”
	//30：”2008 - 01 - 11″，日期；
	//31：”15:05 : 32″，时间；

	vector<string> vec1 = splitEx(strLatest, ",");
	if (vec1.size() < 31){
		return false;
	}

	string strDate1 = vec1[30];
	if (strDate1.size() == 10){
		string strYear(strDate1, 0, 4);
		string strMonth(strDate1, 5, 2);
		string strDay(strDate1, 8, 2);

		stringstream ss;
		ss << strYear;
		ss >> y1;
		ss.str("");
		ss.clear();
		ss << strMonth;
		ss >> m1;
		ss.str("");
		ss.clear();
		ss << strDay;
		ss >> d1;
	}

	bool bUpdated = false;
	if (y == y1 && m == m1 && d == d1){
		bUpdated = true;
	}

	infile.open(strId);
	strBuff.clear();
	ss.str("");
	ss.clear();

	//直接保存第一行
	getline(infile, strBuff);
	ss << strBuff;
	ss << "\n";

	//在第二行写入新的数据
	//Date, Open, High, Low, Close, Volume, Adj Close
	//2016-02-16,9.84,10.03,9.83,10.01,42838600,10.01
	ss << vec1[30];
	ss << ",";
	ss << vec1[1];
	ss << ",";
	ss << vec1[4];
	ss << ",";
	ss << vec1[5];
	ss << ",";
	ss << vec1[3];
	ss << ",";
	ss << vec1[8];
	ss << ",";
	ss << "0";
	ss << "\n";

	if (bUpdated){
		getline(infile, strBuff); //跳过此行
	}

	while (getline(infile, strBuff)){
		ss << strBuff;
		ss << "\n";
	}
	infile.close();

	ofstream outfile;
	outfile.open(strId);
	if (outfile.is_open()){
		outfile << ss.str();
		outfile.close();
	}
	return true;
}



LPSTR GetInterNetURLText(LPSTR lpcInterNetURL, ofstream &outfile)
{
	static const int bufferSize = 10240;
	HINTERNET hSession;
	LPSTR lpResult = NULL;
	hSession = InternetOpen("WinInet", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	__try
	{
		if (hSession != NULL)
		{
			HINTERNET hRequest;
			hRequest = InternetOpenUrlA(hSession, lpcInterNetURL, NULL, 0, INTERNET_FLAG_RELOAD, 0);
			__try
			{
				if (hRequest != NULL)
				{
					DWORD dwBytesRead;//下载的字节数
					do
					{
						char szBuffer[bufferSize] = { 0 };
						InternetReadFile(hRequest, szBuffer, bufferSize, &dwBytesRead);
						outfile << szBuffer;
						outfile.flush();
					} while (dwBytesRead != 0);
				}
			}
			__finally
			{
				InternetCloseHandle(hRequest);
			}
		}
	}
	__finally
	{
		InternetCloseHandle(hSession);
	}
	return lpResult;
}

LPSTR GetInterNetURLText_string(LPSTR lpcInterNetURL, string &outstring)
{
	static const int bufferSize = 10240;
	HINTERNET hSession;
	LPSTR lpResult = NULL;
	hSession = InternetOpen("WinInet", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	__try
	{
		if (hSession != NULL)
		{
			HINTERNET hRequest;
			hRequest = InternetOpenUrlA(hSession, lpcInterNetURL, NULL, 0, INTERNET_FLAG_RELOAD, 0);
			__try
			{
				if (hRequest != NULL)
				{
					DWORD dwBytesRead;//下载的字节数
					do
					{
						char szBuffer[bufferSize] = { 0 };
						InternetReadFile(hRequest, szBuffer, bufferSize, &dwBytesRead);
						outstring.append(szBuffer);
					} while (dwBytesRead != 0);
				}
			}
			__finally
			{
				InternetCloseHandle(hRequest);
			}
		}
	}
	__finally
	{
		InternetCloseHandle(hSession);
	}
	return lpResult;
}
