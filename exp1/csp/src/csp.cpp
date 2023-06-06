#include <bits/stdc++.h>
# define TwoDIntVector     vector<vector<int>>
# define PiiVector      vector<pair<int,int>>
using namespace std;


int N;//N：宿管阿姨数量  ；
int D;//D：值班天数
int S; //S：每日轮班次数     每天分为轮班次数个值班班次

int times;  //⾄少被分配到 D·S/N(向下取整) 次值班。
int cnt = 0;

/**
 * 结构体：阿姨
 */
struct Aunt {
    PiiVector request;
    Aunt() {
        request = PiiVector();
    }
};

/**
 * 结构体：某天的排班信息
 */
struct DayShift{
    int auntNo; //值班的阿姨编号
    vector<int> remainValue; //剩余可选的阿姨
    /**
     * 无参构造函数
     */
    DayShift(){
        auntNo = -1;
        remainValue  = vector<int>();
    }
};

// 将第i天 第j班 的 剩余值即阿姨编号   按 阿姨剩余的请求数 的值从大到小排序
bool cmp_request(int no1, int no2, const vector<Aunt>& Aunts) {
    return Aunts[no1].request.size() < Aunts[no2].request.size();
}
// 将第i天 第j班 的 剩余值即阿姨编号   按 阿姨已排班数 的值从小到大排序
bool cmp_times(int no1, int no2, const vector<int>& aunt_times) {
    return aunt_times[no1] < aunt_times[no2];
}

#pragma region ##约束处理##

/**
 *
 * @param ShiftMatrix 待检查的方案
 * @return true：每一班都有阿姨值班
 */
bool Complete(const vector<vector<DayShift>>& ShiftMatrix){
    for (const auto &shiftMatrix : ShiftMatrix) {
        for (const auto &item : shiftMatrix) {
            if(item.auntNo == -1)
                return false;
        }
    }
    return true;
}

/**
 *3. 公平起⻅，每个宿管阿姨在整个排班周期中，应⾄少被分配到 D·S/N(向下取整) 次值班。
 * @param ShiftMatrix  待检查方案
 * @param times  最少值班次数
 * @return
 */
bool DutyTimes(const vector<vector<DayShift>>& ShiftMatrix){//,const int& times
    vector<int> DutyCnt(N);
    for (const auto &shiftMatrix : ShiftMatrix) {
        for (const auto &item : shiftMatrix) {
            if(item.auntNo >= 0) {
                DutyCnt[item.auntNo]++;
            }
        }

    }

    for (int i = 0; i < N; ++i) {
        if(DutyCnt[i] < times) {
            return false;
        }
    }

    return true;
}

/**
 *
 * @param AuntMatrix   引用类型  允许直接修改对应的值
 * @param remainValue   在回溯算法中选择的值，对应的阿姨编号
 * @param i      某一天
 * @param j     某一班次
 * @return  是否满足 不存在连续上班的情况
 */
bool Consistent(const vector<vector<DayShift>> ShiftMatrix,const vector<Aunt>& Aunts,int auntNo,const int i, const int  j){
    auto row = ShiftMatrix.size();
    auto col = ShiftMatrix[0].size();
    vector<int> aunt_time(Aunts.size(),0);

    for (const auto &row : ShiftMatrix) {
        for (const auto &value : row) {
            if(value.auntNo >= 0){
                aunt_time[value.auntNo]++;//对应阿姨已排班次数
            }
        }
    }

    if(aunt_time[auntNo] > times){
        return false;
    }

    if(i > 0 && i <= row-1 && j == 0 && auntNo == ShiftMatrix[i-1][col-1].auntNo
       && auntNo != -1 ){//(i,j)为第二天第一班  两天交界处 或者 后一班连续
        return false;
    }

    if(i>= 0 && i < row-1 && j == col - 1 && auntNo == ShiftMatrix[i+1][0].auntNo
       && auntNo != -1){       //(i,j)为前一天最后一班  两天交界处
        return false;
    }

    if(i>= 0 && i <= row-1 && j > 0 &&  j <= col - 1 && auntNo == ShiftMatrix[i][j-1].auntNo
       && auntNo != -1){       //同一天连续两班  前一班和当前相同
        return false;
    }

    if(i>= 0 && i <= row-1 && j >= 0 &&  j < col - 1 && auntNo == ShiftMatrix[i][j+1].auntNo
       && auntNo != -1){       //同一天连续两班  后一班和当前相同
        return false;
    }

    return true;
}

#pragma endregion

#pragma region ##选择变量的方式##
/**
 * 顺序获取对应的 第i天，第j班进行赋值
 * @param ShiftMatrix   排班表，引用类型
 * @param Aunts   阿姨的数组
 * @param mi   第i天 引用类型
 * @param mj   第j班 引用类型
 * @return
 */
bool Get_ij(vector<vector<DayShift>>& ShiftMatrix,const vector<Aunt>& Aunts,int& mi,int& mj){
    for (int i = 0; i < ShiftMatrix.size(); i++) { // 某一天
        for (int j = 0; j < ShiftMatrix[i].size(); j++) { // 某一班
            int remainValueSize = ShiftMatrix[i][j].remainValue.size();
            int auntno = -1;
            if (ShiftMatrix[i][j].auntNo == -1 && remainValueSize > 0) {//这一天还没有分配阿姨并且有可选值，直接返回 i j
                mi = i;
                mj = j;
                return true;
            }
            if (ShiftMatrix[i][j].auntNo == -1 && remainValueSize <= 0) {//没有可选阿姨，需要判断是否还有阿姨有请求但未分配给她值班
                int maxsize = 0;
                for (int k = 0; k < Aunts.size(); k++) {//遍历所有阿姨
                    int currsize = Aunts[k].request.size();
                    if (currsize > maxsize) {//优先选择请求最多的阿姨
                        auntno = k;//记录阿姨编号
                        maxsize = currsize;
                    }
                }
                if(auntno >= 0){
                    ShiftMatrix[i][j].remainValue.push_back(auntno);
                    mi = i;
                    mj = j;
                    return true;
                }
            }
        }
    }
    return false;
}

/**
 * 最小剩余值：优先选最少可选阿姨的（i，j）排班  ，因为可选剩余值是阿姨对当天当班次的请求，
 * 所以剩余值为0时要从还有请求未满足阿姨中选，赋给该班次，
 * 在这里的赋值方式为将最多请求的阿姨附给该变量，但优化不明显
 * @param ShiftMatrix 排班表
 * @param mi  第 i 天  引用类型
 * @param mj  第j班 引用类型
 * @return
 */
bool MinRemainVal_ij(vector<vector<DayShift>>& ShiftMatrix,const vector<Aunt>& Aunts,int& mi,int& mj){
    int minRemainValue = 1e5+10; // 初始化最小值为一个较大的数
    for (int i = 0; i < ShiftMatrix.size(); i++) { // 某一天
        for (int j = 0; j < ShiftMatrix[i].size(); j++) { // 某一班
            int remainValueSize = ShiftMatrix[i][j].remainValue.size();
            int auntno = -1;
            if(ShiftMatrix[i][j].auntNo== -1 && remainValueSize <= 0){//没有可选阿姨，需要判断是否还有阿姨有请求但未分配给她值班
                int maxsize = 0;
                for (int k = 0; k < Aunts.size(); k++) {//遍历所有阿姨
                    int currsize = Aunts[k].request.size();
                    if (currsize > maxsize) {//优先选择请求最多的阿姨
                        auntno = k;//记录阿姨编号
                        maxsize = currsize;
                    }
                }
                if (auntno >= 0) {
                    mi = i;
                    mj = j;
                    ShiftMatrix[mi][mj].remainValue.push_back(auntno);
                    return true;
                }
            }
            // 更新最小值和对应的i、j
            if (ShiftMatrix[i][j].auntNo== -1 && remainValueSize > 0&& remainValueSize < minRemainValue  ) {
                minRemainValue = remainValueSize;
                mi = i;
                mj = j;
            }
        }
    }

    if(minRemainValue == 1e5+10) {
        return false;
    }

    return true;
}
/** 加入最少约束值Least Constraining Value
 *
 * 首先将(i,j)对应的remain value按照阿姨的已满足数从小到大排序
 * 即在MinRemainVal_ij基础上添加了判断阿姨是否已经达到指定次数，优先选没有指定次数的阿姨
 *
 */
bool MRVwithLCV_ij(vector<vector<DayShift>>& ShiftMatrix,const vector<Aunt>& Aunts,int& mi,int& mj){
/**
 * 最少约束值
 * 排班次数已经满足D*S/N的阿姨优先级最低
 */
    vector<int> aunt_time(Aunts.size());
    vector<int> satisfied_aunts;
    for (const auto &row : ShiftMatrix) {
        for (const auto &value : row) {
            if(value.auntNo >= 0){
                aunt_time[value.auntNo]++;//对应阿姨已排班次数
            }
        }
    }

    for (int k = 0; k < Aunts.size(); ++k) {
        //找出排班次数已经超过times的阿姨k
        if( aunt_time[k] < times){
            satisfied_aunts.push_back(k);
        }
    }

    int minRemainValue = 1e5+10; // 初始化最小值为一个较大的数
    for (int i = 0; i < ShiftMatrix.size(); i++) { // 某一天
        for (int j = 0; j < ShiftMatrix[i].size(); j++) { // 某一班
            int remainValueSize = ShiftMatrix[i][j].remainValue.size();
            int auntno = -1;
            //实际测试发现除了input0.txt，其余不会出现remainValueSize <= 0的情况
            if(ShiftMatrix[i][j].auntNo== -1 && remainValueSize <= 0){//没有可选阿姨，需要判断是否还有阿姨有请求但未分配给她值班
                int maxsize = 0;
                for (int k = 0; k < Aunts.size(); k++) {//遍历所有阿姨
                    int currsize = Aunts[k].request.size();
                    bool is_k_in_satisfied_aunts = find(satisfied_aunts.begin(), satisfied_aunts.end(), k) != satisfied_aunts.end();
                    if (currsize > maxsize && !is_k_in_satisfied_aunts) {//优先选择请求最多且未满足times的
                        auntno = k;//记录阿姨编号
                        maxsize = currsize;
                    }
                }
                if (auntno >= 0) {
                    mi = i;
                    mj = j;
                    ShiftMatrix[mi][mj].remainValue.push_back(auntno);
                    return true;
                }
            }

            // 更新最小值和对应的i、j
            if (ShiftMatrix[i][j].auntNo== -1 && remainValueSize > 0&& remainValueSize < minRemainValue  ) {
                minRemainValue = remainValueSize;
                mi = i;
                mj = j;

                //将每天排班的可选剩余值按照阿姨的已满足排班数从小到大排序
                sort(ShiftMatrix[i][j].remainValue.begin(), ShiftMatrix[i][j].remainValue.end(), [&](int a, int b) {
                    return cmp_times(a, b, aunt_time);  // 使用 lambda 表达式传递数组 arr
                });

            }
        }
    }

    if(minRemainValue == 1e5+10) {
        return false;
    }

    return true;
}
/**
 * MRV + Constraint Propagation
 * 尝试不考虑阿姨请求数，仅考虑阿姨排班数
 */
bool MRVwithLCVwithCP_ij(vector<vector<DayShift>>& ShiftMatrix,const vector<Aunt>& Aunts,int& mi,int& mj){

    vector<int> unsatisfied_aunts;
    vector<int> satisfied_aunts;

    vector<int> aunt_time(N);
    for (const auto &shiftMatrix : ShiftMatrix) {
        for (const auto &item : shiftMatrix) {
            if(item.auntNo >= 0) {
//                cout << "auntno:" << item.auntNo << endl;
                aunt_time[item.auntNo]++;
            }
        }
    }

    for (int k = 0; k < Aunts.size(); ++k) {
        //找出排班次数还没到times的阿姨k
        if( aunt_time[k] < times){
            unsatisfied_aunts.emplace_back(k);
        }
    }

    for (int k = 0; k < Aunts.size(); ++k) {
        //找出排班次数到times的阿姨k
        if( aunt_time[k] >= times){
            satisfied_aunts.emplace_back(k);
        }
    }

    int minRemainValue = 1e5+10; // 初始化最小值为一个较大的数
    //找出剩余可选值最少的班次
    for (int i = 0; i < ShiftMatrix.size(); i++) { // 某一天
        for (int j = 0; j < ShiftMatrix[i].size(); j++) { // 某一班
            int remainValueSize = ShiftMatrix[i][j].remainValue.size();

            if (ShiftMatrix[i][j].auntNo== -1 && remainValueSize < minRemainValue )  {
                minRemainValue = remainValueSize;
                mi = i;
                mj = j;
            }
        }
    }

    if(minRemainValue == 1e5+10) {
        return false;
    }

    int auntno = -1;
    if(ShiftMatrix[mi][mj].auntNo== -1 && ShiftMatrix[mi][mj].remainValue.size() <= 0){//没有可选阿姨，需要判断是否还有阿姨有请求但未分配给她值班
        int mintimes = 1e5+10;
        for (int k = 0; k < Aunts.size(); k++) {//遍历所有阿姨
            bool is_k_in_satisfied_aunts = find(satisfied_aunts.begin(), satisfied_aunts.end(), k) != satisfied_aunts.end();
            if ( aunt_time[k] < mintimes  && Aunts[k].request.size() > 0 &&  !is_k_in_satisfied_aunts) {//优先选择排班数较少的阿姨
                auntno = k;//记录阿姨编号
                mintimes = aunt_time[k];
            }
        }
        if (auntno >= 0) {
            ShiftMatrix[mi][mj].remainValue.emplace_back(auntno);
            return true;
        }
    }

    if (ShiftMatrix[mi][mj].auntNo== -1 && ShiftMatrix[mi][mj].remainValue.size() > 0) {

        //将每天排班的可选剩余值按照阿姨的已满足排班数从小到大排序
        sort(ShiftMatrix[mi][mj].remainValue.begin(), ShiftMatrix[mi][mj].remainValue.end(), [&](int a, int b) {
            return cmp_times(a, b, aunt_time);  // 使用 lambda 表达式传递数组 arr
        });
    }


    return true;
}

#pragma endregion

/**
 *
 * @param ShiftMatrix  排班的矩阵，每个元素包括剩余可选值和已选阿姨号  未选为-1
 * @param Aunts   阿姨的数组，每个元素包含阿姨的请求列表（i，j）第i天第j班
 * @param AnsMatrix   答案矩阵，最终每一天每一班的阿姨编号
 * @return
 */
bool RecursiveBackTracing(const vector<vector<DayShift>>& ShiftMatrix,const vector<Aunt>& Aunts,TwoDIntVector& AnsMatrix,string function){
//    clock_t start = clock();
    /**
     * 检查约束
     */
    if(DutyTimes(ShiftMatrix)  && Complete(ShiftMatrix)){
        cout<<"Finish"<<endl;
        for (std::size_t i = 0; i < ShiftMatrix.size(); ++i) {
            for (std::size_t j = 0; j < ShiftMatrix[i].size(); ++j) {
                AnsMatrix[i][j] = ShiftMatrix[i][j].auntNo;
            }
        }
        return true;
    }


    int i,j;//第i天 第j班

    vector<vector<DayShift>> OptionShift(ShiftMatrix);//每个分支传入对应相同的ShiftMatrix，相当于copy
    /**
     * 获取下一个待选的排班
     */
    if(function=="naive"){
        if(!Get_ij(OptionShift,Aunts,i,j)){
            return false;
        }
    } else if(function == "mrv"){
        if(!MinRemainVal_ij(OptionShift,Aunts,i,j)){
            return false;
        }
    } else if(function == "mrv_lcv"){
        if(!MRVwithLCV_ij(OptionShift,Aunts,i,j)){
            return false;
        }
    }else if(function == "mrv_lcv_cp"){
        if(!MRVwithLCVwithCP_ij(OptionShift,Aunts,i,j)){
            return false;
        }
    }
    cnt++;

    for (const int &auntNo : OptionShift[i][j].remainValue) {//遍历每个班次的剩余可选值，
        vector<vector<DayShift>> tempShift(OptionShift);//每个分支传入矩阵的copy
        vector<Aunt> tempAunts(Aunts);//同理 copy
        if(Consistent(tempShift,tempAunts,auntNo,i,j)){//检查是否相容  即连续上班
//            clock_t end = clock();
//            double duration = (double)(end - start) / CLOCKS_PER_SEC;
//            if (duration > 10) {
//                return false;
//            }
            tempShift[i][j].auntNo = auntNo;

            //                        第i天第j班 选定阿姨后删除 该阿姨对应的请求
            // 使用 find 函数查找要删除的阿姨请求(i,j)
            auto iter_aunt = find(tempAunts[auntNo].request.begin(), tempAunts[auntNo].request.end(), make_pair(i,j));
            // 如果找到了要删除的元素，使用 erase 函数将其从向量中删除
            if (iter_aunt != tempAunts[auntNo].request.end())
                tempAunts[auntNo].request.erase(iter_aunt);

            //第i天第j班选定阿姨后将 对应第i天第j班的剩余值删去该阿姨编号
            // 使用 find 函数查找要删除的元素
            auto iter_shift = find(tempShift[i][j].remainValue.begin(), tempShift[i][j].remainValue.end(), auntNo);
            // 如果找到了要删除的元素，使用 erase 函数将其从向量中删除
            if (iter_shift != tempShift[i][j].remainValue.end())
                tempShift[i][j].remainValue.erase(iter_shift);
            //递归
            if(RecursiveBackTracing(tempShift,tempAunts,AnsMatrix,function))
                return true;
        }
    }
    return false;
}



#pragma region ##读取与初始化数据##
/**
 *
 * @param filename
 * @param N  阿姨个数  比如有3个阿姨
 * @param D  需要排班的天数  比如总共有7天需要安排
 * @param S  每天排班的次数 比如一天要排3次班
 * @return  TwoDIntVector 二维矩阵 ：原始数据
 */
TwoDIntVector readRequestMatrix(const std::string& filename,int& N,int& D,int& S) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cout << "无法打开文件 " << filename << std::endl;
        return {};
    }
    std::string line;
    std::getline(file, line); // 读取文件中的第一行

    std::stringstream ss(line); // 将字符串转换为 stringstream 对象
    char comma; // 用于读取逗号

    ss >> N >> comma >> D >> comma >> S; // 从 stringstream 中解析出三个整数和两个逗号

    TwoDIntVector requestMatrix(N*D, std::vector<int>(S, 0));

    for (int i = 0; i < N*D; i++) {
        std::string line;
        std::getline(file, line); // 从文件中读取一行
        std::stringstream ss(line); // 将字符串转换为 stringstream 对象
        for (int j = 0; j < S; j++) {
            std::string token;
            std::getline(ss, token, ','); // 使用逗号分隔符从 stringstream 中读取一个 token
            requestMatrix[i][j] = std::stoi(token); // 将 token 转换为整数并存储在 requestMatrix 中
        }
    }
    file.close();
    return requestMatrix;
}

/**
 *
 * @param arr 传入原始矩阵
 * @param n     阿姨个数
 * @param d     排班天数
 * @param s     值班次数
 * @return      三维矩阵，n个阿姨的dxs请求矩阵
 */
vector<vector<vector<int>>> splitVector(vector<vector<int>> arr, int n, int d, int s) {
    vector<vector<vector<int>>> splitArr(n, vector<vector<int>>(d, vector<int>(s)));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < d; j++) {
            for (int k = 0; k < s; k++) {
                splitArr[i][j][k] = arr[i * d + j][k];
            }
        }
    }
    return splitArr;
}


void writeSolution(const string &file,vector<vector<int>> Ans,const int& satisfiedNum ){
    ofstream outfile(file);
    if (!outfile.is_open()) {
        std::cout << "Cannot open file: " << file << std::endl;
        return;
    }

    for (int d = 0; d < D; ++d) {
        for (int s = 0; s < S; ++s) {
            int n = Ans[d][s];
            outfile << n+1;
            if (s < S - 1) {
                outfile << ",";
            }
        }
        outfile << endl;
    }
    outfile << satisfiedNum;
    outfile.close();
}

#pragma endregion

int main() {
    for (int i = 0; i < 10; ++i) {

        string input = "../input/input" + std::to_string(i) + ".txt";
        string output = "../output/output" + std::to_string(i) + ".txt";

        TwoDIntVector requestMatrix = readRequestMatrix(input,N,D,S);
        times  = D*S/N;
        vector<vector<vector<int>>> result(N); // 创建一个包含 N 个向量的向量，用于存储分割后的结果
        result = splitVector(requestMatrix,N,D,S);//result的每个元素是一个二维矩阵表示每个阿姨的请求

#pragma region ##初始化输入数据##
        //初始化Aunts
        vector<Aunt> Aunts(N);
        for (int i = 0; i < N; i++) {
            for(int j = 0; j < D; j++) {
                for(int k= 0; k < S; k++) {
                    if (result[i][j][k] == 1) {
                        Aunts[i].request.emplace_back(j, k);
                    }
                }
            }
        }

        //初始化DayShift
        vector<vector<DayShift>>  ShiftMatrix(D, vector<DayShift>(S));
        for(int j = 0; j < ShiftMatrix.size(); j++) {//某一天
            for(int k= 0; k < ShiftMatrix[0].size(); k++) {//某一班
                for (int i = 0; i < Aunts.size(); i++) {//遍历所有阿姨
                    for (auto duty : Aunts[i].request) {//获取阿姨的请求列表
                        if (duty.first == j && duty.second == k) {//如果阿姨的请求列表和当前班次对应
                            ShiftMatrix[j][k].remainValue.push_back(i);//将阿姨的编号加入剩余值列表
                        }
                    }
                }
            }
        }

        //记录每一天每一班的请求阿姨编号，用于测试解决方案能够满足多少请求
//    vector<vector<vector<int>>> DayRequest(D*S);
//
//    for (int i = 0; i < ShiftMatrix.size(); i++) {
//        for (int j = 0; j < ShiftMatrix[0].size(); j++) {
//            DayRequest[i][j] = ShiftMatrix[i][j].remainValue;
//        }
//    }

        //存放最终结果
        TwoDIntVector AnsMatrix(D,vector<int>(S));
#pragma endregion



#pragma region ##测试##
        bool testRead = false;
        if(testRead){
            /**
             * 输出读入原始矩阵
             */
            for (const auto &row : requestMatrix) {
                for (const auto &value : row) {
                    std::cout << value << " ";
                }
                std::cout << std::endl;
            }

            /**
             * 输出分割后每个阿姨的矩阵
             */

            for (int i = 0; i < N; i++) {
                cout<<i<<":"<<endl;
                for (const auto &row : result[i]) {
                    for (const auto &value : row) {
                        std::cout << value << " ";
                    }
                    std::cout << std::endl;
                }
            }
        }

        bool testAunts = false;
        if (testAunts){
// 输出每个 Aunt 的 request 和 requestNum
            for (int i = 0; i < N; i++) {
                cout << "Aunt " << i << ":" << endl;
                cout << "Request: ";
                for (auto p : Aunts[i].request) {
                    cout << "(" << p.first << "," << p.second << ") ";
                }
                cout << endl;
//        cout << "RequestNum: " << Aunts[i].requestNum << endl;
                cout << endl;
            }
        }

        bool testShift = false;
        if(testShift) {
            // 遍历 ShiftMatrix
            for(int j = 0; j < ShiftMatrix.size(); j++) {//某一天
                for(int k= 0; k < ShiftMatrix[0].size(); k++) {//某一班
                    // 输出 remainValue
                    std::cout <<"第"<<j<<"天第"<<k<< "班的remainValue: ";
                    for (const auto &value : ShiftMatrix[j][k].remainValue) {
                        std::cout << value +1<< " ";
                    }
                    std::cout << std::endl;
                }
            }
        }

#pragma endregion


        vector<string> functions {"naive","mrv","mrv_lcv","mrv_lcv_cp"};
        clock_t startTime, endTime;

        startTime = clock();
        if (RecursiveBackTracing(ShiftMatrix, Aunts, AnsMatrix, functions[3])) {
//            for (const auto &row : AnsMatrix) {
//                for (const auto &value : row) {
//                    cout << value + 1 << " ";
//                }
//                cout << endl;
//            }
            endTime = clock();
            cout<< input+" Find A Solution !" << "运行时间：" << (double) (endTime - startTime) / CLOCKS_PER_SEC << "s 遍历的方案数："<<cnt << endl;
        } else {
//            endTime = clock();
//            cout<< input+"Run timed out !" << "运行时间：" << (double) (endTime - startTime) / CLOCKS_PER_SEC << "s 遍历的方案数："<<cnt << endl;
            cout << "FALSE ";
        }

        int satisfiedNum = 0;
// 遍历AnsMatrix的每个元素
        for (int i = 0; i < AnsMatrix.size(); i++) {
            for (int j = 0; j < AnsMatrix[0].size(); j++) {
                // 遍历DayRequest[i][j]中的每个元素，统计AnsMatrix[i][j]在其中出现的次数
                for (int k = 0; k < ShiftMatrix[i][j].remainValue.size(); k++) {
                    if (AnsMatrix[i][j] == ShiftMatrix[i][j].remainValue[k]) {//阿姨存在(i天,j班)的请求中
                        satisfiedNum++;
                        continue;
                    }
                }
            }
        }

        cout << satisfiedNum << "/" << D * S << endl;
        writeSolution(output,AnsMatrix,satisfiedNum);

    }
}