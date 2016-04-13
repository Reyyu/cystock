#include"common.h"
#include"updateData.h"

class CStockInfo{
public:
	string strId;
	string strName;
	bool bFlagSkip; //是否跳过
	CStockInfo() :bFlagSkip(false){}
};

int main(){
	//------------------------------------------------
	//加载股票池中的所有股票信息
	ifstream in("..\\stockData\\stockPool.txt");
	//ifstream in("D:\\stockPool.txt");
	if (!in.is_open()){
		cout << "cannot open stockPool.txt" << endl;
	}
	string strLine;
	vector<CStockInfo> vecStock;
	while (in >> strLine){
		vector<string> vec = splitEx(strLine, ",");
		if (vec.size() != 3){
			continue;
		}
		CStockInfo temp;
		temp.strId = vec[0];
		temp.strName = vec[1];
		temp.bFlagSkip = (vec[2] == "0")? 0:1;
		vecStock.push_back(temp);
	}

	//-------------------------------------------------------------------------------------------
	//由于目前只找到雅虎的历史股票下载接口，而它的更新会慢一天，所以先从雅虎上下载一天之前的历史数据，再下载当天的数据
	for (vector<CStockInfo>::iterator iter = vecStock.begin(); iter != vecStock.end(); ++iter){
		if (!iter->bFlagSkip){
			updateStockDataFromYahoo(iter->strId, 2);
			
			updateStockDataToday(iter->strId);
		}
	}

	return 0;
}
