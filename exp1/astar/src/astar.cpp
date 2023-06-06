#include <bits/stdc++.h>
#define TwoDMatrix  std::vector<std::vector<int>>
#define LockMatrixQueue priority_queue<LockMatrix*,vector<LockMatrix*>, cmp>

using namespace std;
#pragma region ##初始化定义##
enum IterationType {
    top_right = 1,
    top_left,
    down_left,
    down_right
};

/**
 * @Init 定义结构体:锁盘  LockMatrix
 *      属性包括：
 *                      1. int N  ：表示锁盘的大小
 *                      2. vector <vector<int>> matrix  ：表示NxN的拨轮锁盘
 *                      3. int g： 对应g(n)  表示开始状态到 当前状态的耗散(代价)
 *                      4. int h：对应启发式函数 h(n) 表示当前状态到目标状态的最低耗散路径的耗散估计值(需要满足可采纳性 即不会高估耗散)
 *                      5. pair<pair<int,int>,int> pre：存储上一个状态选择的 (i, j)，以及对应的朝向
 *                      6. LockMatrix(): 无参构造方法，初始化上述属性，初始化为空
 *                      7. LockMatrix(const int& g,const int& h,const vector <vector<int>>&  matrix)
 */
struct LockMatrix {
    int N;
    TwoDMatrix matrix;
    int g;
    int h;
    LockMatrix* parent;
    pair<pair<int, int>, int> pre;

    // 无参构造函数
    LockMatrix() {
        N = 0;
        matrix =     TwoDMatrix();
        g = 0;
        h = 0;
        pre = make_pair(make_pair(-1, -1), -1);
        parent = NULL;
    }

/**常量引用传递方式 ：
 * 将参数的引用传递到函数内部
 * 并且保证函数内部不会修改参数的值。
 * 常量引用传递方式与传值方式相比，避免在函数调用过程中对参数进行拷贝
 * 提高程序效率。*/
    //带参构造函数
    LockMatrix(const int& g, const int& h, const  TwoDMatrix& matrix) {
        this->N = matrix.size();
        this->matrix = matrix;
        this->g = g;
        this->h = h;
        this->pre = make_pair(make_pair(-1, -1), -1);
    }
};
#pragma endregion

#pragma region ##转动四种'L'型##
/**
 * @Function 根据'L'的朝向改变matrix:  TypeTurn
 * @param[in] type   朝向类型
 * @param[in] matrix   LockMatrix的状态矩阵
 * @param[in] i   点的横坐标
 * @param[in] j   点的纵坐标
 * @return     TwoDMatrix  改变后的矩阵
 */
TwoDMatrix TypeTurn(int type, const TwoDMatrix& matrix, int i, int j) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    TwoDMatrix newMatrix = matrix;  // 创建一个与原矩阵相同的新矩阵

    switch (type) {
        case 1:  // s == 1: (i,j), (i,j+1), (i-1,j)
            if (i - 1 >= 0 && j + 1 < cols) {
                newMatrix[i][j] = !matrix[i][j];
                newMatrix[i][j + 1] = !matrix[i][j + 1];
                newMatrix[i - 1][j] = !matrix[i - 1][j];
            } else {
                return TwoDMatrix();  // 越界，返回空矩阵
            }
            break;

        case 2:  // s == 2: (i,j), (i-1,j), (i,j-1)
            if (i - 1 >= 0 && j - 1 >= 0) {
                newMatrix[i][j] = !matrix[i][j];
                newMatrix[i - 1][j] = !matrix[i - 1][j];
                newMatrix[i][j - 1] = !matrix[i][j - 1];
            } else {
                return TwoDMatrix();  // 越界，返回空矩阵
            }
            break;

        case 3:  // s == 3: (i,j), (i,j-1), (i+1,j)
            if (i + 1 < rows && j - 1 >= 0) {
                newMatrix[i][j] = !matrix[i][j];
                newMatrix[i][j - 1] = !matrix[i][j - 1];
                newMatrix[i + 1][j] = !matrix[i + 1][j];
            } else {
                return TwoDMatrix();  // 越界，返回空矩阵
            }
            break;

        case 4:  // s == 4: (i,j), (i+1,j), (i,j+1)
            if (i + 1 < rows && j + 1 < cols) {
                newMatrix[i][j] = !matrix[i][j];
                newMatrix[i + 1][j] = !matrix[i + 1][j];
                newMatrix[i][j + 1] = !matrix[i][j + 1];
            } else {
                return TwoDMatrix();  // 越界，返回空矩阵
            }
            break;
    }

    return newMatrix;
}

#pragma endregion

#pragma region ##记录已扩展结点##
/**
 * @Init 已访问节点集合:  VisitedList
 *      在扩展节点时，先检查该节点是否已经在已访问节点集合中
 *      如果是，则跳过该节点的扩展
 *      确保每个节点只被扩展一次，避免重复计算
 */
static vector<TwoDMatrix> VisitedList;


/**
 * @Function 封闭列表:  isMatrixVisited
 *
 * 检查当前状态是否已被扩展过
 *
 * @param[in] matrix   状态对应的矩阵
 * @return bool值  ： 当前状态是否已扩展过
 */
bool isMatrixVisited(const TwoDMatrix& matrix) {
    for (const auto& visitedMatrix : VisitedList) {
        if (visitedMatrix == matrix) {
            return true;
        }
    }
    return false;
}
#pragma endregion

#pragma region ##启发式函数##

string heuristic ;
int AnsCnt;
vector<pair<pair<int, int>, int>> AnsList;

/**
 * @Function 有效点检查:  isValidPosition
 *
 * 启发式函数Heuristic的子函数
 *
 * @param[in] matrix   状态对应的矩阵
 * @param[in]  i   点的横坐标
 * @param[in]  j   点的纵坐标
 * @return bool值  ： 当前点是否在矩阵内且为1
 */
bool isValidPosition(const TwoDMatrix& matrix, int i, int j) {
    int rows = matrix.size();
    int cols = matrix[0].size();
    return (i >= 0 && i < rows && j >= 0 && j < cols && matrix[i][j] == 1);
}

/**
 * @Function 启发式函数:  Heuristic
 *
 * 函数详细描述，包括输入输出参数等。
 *
 * @param[in] matrix   状态对应的矩阵
 * @return  int count  不相交的1构成的‘L’的个数
 */
int Heuristic(const TwoDMatrix& matrix) {
    int count = 0;
    int N = matrix.size();
    TwoDMatrix tempMatrix = matrix;  // Create a copy of the matrix
    if(heuristic  == "NumOf_1") {
        for (int i = 0; i < tempMatrix.size(); i++) {
            for (int j = 0; j < tempMatrix[i].size(); j++) {
                if (tempMatrix[i][j] == 1) {
                    count++;
                }
            }
        }
    }if(heuristic  == "Manhattan"){
        for (int i = 0; i < tempMatrix.size(); i++) {
            for (int j = 0; j < tempMatrix[i].size(); j++) {
                if (tempMatrix[i][j] == 1) {
                    for (int k = i; k < tempMatrix.size(); k++) {
                        for (int l = 0; l < tempMatrix[k].size(); l++) {
                            if (tempMatrix[k][l] == 1) {
                                int distance = std::abs(i - k) + std::abs(j - l);
                                count += distance;
                            }
                        }
                    }
                }
            }
        }
    } if(heuristic  == "DisjointL") {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (tempMatrix[i][j] == 1) {
                    if (isValidPosition(tempMatrix, i, j + 1) && isValidPosition(tempMatrix, i - 1, j)) {
                        count++;
                        tempMatrix[i][j] = 0;
                        tempMatrix[i][j + 1] = 0;
                        tempMatrix[i - 1][j] = 0;
                        continue;
                    }
                    if (isValidPosition(tempMatrix, i - 1, j) && isValidPosition(tempMatrix, i, j - 1)) {
                        count++;
                        tempMatrix[i][j] = 0;
                        tempMatrix[i - 1][j] = 0;
                        tempMatrix[i][j - 1] = 0;
                        continue;
                    }
                    if (isValidPosition(tempMatrix, i, j - 1) && isValidPosition(tempMatrix, i + 1, j)) {
                        count++;
                        tempMatrix[i][j] = 0;
                        tempMatrix[i][j - 1] = 0;
                        tempMatrix[i + 1][j] = 0;
                        continue;
                    }
                    if (isValidPosition(tempMatrix, i + 1, j) && isValidPosition(tempMatrix, i, j + 1)) {
                        count++;
                        tempMatrix[i][j] = 0;
                        tempMatrix[i + 1][j] = 0;
                        tempMatrix[i][j + 1] = 0;
                        continue;
                    }
                }
            }
        }
//        if (count != 0)
//            return count * 2 + count / 3;
//        else {
//            for (int i = 0; i < N; i++)
//                for (int j = 0; j < N; j++) {
//                    if (matrix[i][j] == 1)
//                        count++;
//                }
//        }
//        return count * 2 + count / 3;
    }
    return count;
//    return 0;
}
//你想划分的部分代码

#pragma endregion

#pragma region ##扩展边缘队列##
/**
 * @Function 自定义比较函数:  cmp
 *
 * 优先队列根据LockMatrix的g+h值，较低的优先级较高
 *
 * @param[in] lm1   LockMatrix1
 * @param[in] lm2   LockMatrix2
 * @return bool值
 */
struct cmp {
    bool operator()(const LockMatrix* lm1, const LockMatrix* lm2) {
        return (lm1->g + lm1->h) > (lm2->g + lm2->h);
    }
};


/**
 * @Function 扩展边缘队列:  ExtendFrontier
 *
 * 优先队列根据LockMatrix的g+h值，较低的优先级较高
 *
 * @param[in] MatrixQueue   优先队列，存放边缘节点
 * @param[in] PreMatrix      父结点
 * @return 在函数内修改优先队列的内容，即插入节点
 */
void ExtendFrontier(LockMatrixQueue& MatrixQueue,
                    LockMatrix* PreMatrix){
    int TempN = PreMatrix->N;
    for(int i = 0; i < TempN; i++)
        for (int j = 0; j < TempN; j++) {
            for (int k : {1, 2, 3, 4}) {
                LockMatrix* CurrMatrix = new LockMatrix;
                CurrMatrix->matrix = TypeTurn(k,PreMatrix->matrix,i,j);
                CurrMatrix->pre = make_pair(make_pair(i, j), k);
                if(isMatrixVisited(CurrMatrix->matrix)||CurrMatrix->matrix.empty()){
                    delete CurrMatrix;
                    continue;
                } else{
//            std::cout<<MatrixQueue.size()<<std::endl;
                    CurrMatrix->g = PreMatrix->g + 1;
                    CurrMatrix->h = Heuristic(CurrMatrix->matrix);
                    CurrMatrix->N = PreMatrix->N;
                    CurrMatrix->parent = PreMatrix;
                    MatrixQueue.push(CurrMatrix);
                }
            }
        }
}

void ExtendFrontierWithLimit(LockMatrixQueue& MatrixQueue,
                             LockMatrix* PreMatrix,pair<int,int>& limit){
    int TempN = PreMatrix->N;
    for(int i = 0; i < TempN; i++)
        for (int j = 0; j < TempN; j++) {
            for (int k : {1, 2, 3, 4}) {
                LockMatrix* CurrMatrix = new LockMatrix;
                CurrMatrix->matrix = TypeTurn(k,PreMatrix->matrix,i,j);
                CurrMatrix->pre = make_pair(make_pair(i, j), k);
                if(isMatrixVisited(CurrMatrix->matrix)||CurrMatrix->matrix.empty()){
                    delete CurrMatrix;
                    continue;
                } else{
//            std::cout<<MatrixQueue.size()<<std::endl;
                    CurrMatrix->g = PreMatrix->g + 1;
                    CurrMatrix->h = Heuristic(CurrMatrix->matrix);
                    if(CurrMatrix->g + CurrMatrix->h > limit.first){
                        limit.second = min(limit.second, CurrMatrix->g + CurrMatrix->h);
                        delete CurrMatrix;
                    } else{
                        CurrMatrix->N = PreMatrix->N;
                        CurrMatrix->parent = PreMatrix;
                        MatrixQueue.push(CurrMatrix);
                    }
                }
            }
        }
}

#pragma endregion

#pragma region ##A*算法##



/**
 * @Function A*搜索:  AStar
 *
 * 优先队列根据LockMatrix的g+h值，较低的优先级较高
 *
 * @param[in] InputMatrix   初始输入的锁盘矩阵
 * @return  记录解路径
 */
void AStar(const TwoDMatrix& InputMatrix){
    //边缘队列
    LockMatrixQueue LMQueue;

    //初始化封闭列表
    VisitedList = vector<TwoDMatrix>();
    //初始化输入锁盘
    LockMatrix*  OriginMatrix = new LockMatrix(0, Heuristic(InputMatrix), InputMatrix);
    //加入优先队列
    LMQueue.push(OriginMatrix);
    //加入封闭列表
    VisitedList.push_back(OriginMatrix->matrix);
    //目标锁盘状态
    TwoDMatrix EndMatrix(OriginMatrix->N, std::vector<int>(OriginMatrix->N, 0));
    // 接受出队锁盘
    LockMatrix* TempMatrix;
    clock_t startTime, endTime;
    startTime = clock();

    while (!LMQueue.empty()){
        //出队
        TempMatrix = LMQueue.top();
        LMQueue.pop();
        //加入封闭列表
        endTime = clock();
        //寻找目标状态
        auto duration =  (double) (endTime - startTime) / CLOCKS_PER_SEC;
        if(duration > 100){
            cout << "超过100s ! " << endl;
            break;;
        }


        if(TempMatrix->matrix == EndMatrix){

//               std::cout<<"FIND ANS!"<<std::endl;

            LockMatrix* EndTemp = TempMatrix;
            while (EndTemp->parent != NULL && EndTemp->pre.first.first!=-1  ){
                AnsCnt++;
                std::cout<<"("<<EndTemp->pre.first.first<<","<<EndTemp->pre.first.second<<")"<<EndTemp->pre.second<<std::endl;
//                 std::cout<<EndTemp->h<<","<<EndTemp->g<<std::endl;
                for (const auto &s : EndTemp->matrix) {
                    for (const auto &value : s) {
                        std::cout << value << " ";
                    }
                    std::cout << std::endl;
                }
                AnsList.push_back(EndTemp->pre);
                EndTemp = EndTemp->parent;
            }
            break;
        }

        ExtendFrontier(LMQueue,TempMatrix);
        VisitedList.push_back(TempMatrix->matrix);
    }

}

void IDAStar(const TwoDMatrix& InputMatrix){
    // 接受出队锁盘
    LockMatrix* TempMatrix;
    //耗散值阈值
    pair<int,int> limit;
    limit.first = Heuristic(InputMatrix);
    limit.second = INT_MAX;
    //初始化输入锁盘
    LockMatrix*  OriginMatrix = new LockMatrix(0, Heuristic(InputMatrix), InputMatrix);
    //判断目标状态
    bool flag = false;
    clock_t startTime, endTime;
    startTime = clock();
    while (limit.first < INT_MAX && !flag) {

        limit.second = INT_MAX;

        //边缘队列
        LockMatrixQueue LMQueue;

        //加入优先队列
        LMQueue.push(OriginMatrix);

        //初始化封闭列表
        VisitedList = vector<TwoDMatrix>();
        //加入封闭列表
        VisitedList.push_back(OriginMatrix->matrix);

        //目标锁盘状态
        TwoDMatrix EndMatrix(OriginMatrix->N, std::vector<int>(OriginMatrix->N, 0));

        while (!LMQueue.empty()) {
            //出队
            TempMatrix = LMQueue.top();
            LMQueue.pop();
            //加入封闭列表
            endTime = clock();
            auto duration =  (double) (endTime - startTime) / CLOCKS_PER_SEC;
//            if(duration > 600){
//                cout << "超过600s ! " << endl;
//                break;
//            }
            //寻找目标状态
            if (TempMatrix->matrix == EndMatrix) {
                flag = true;
                std::cout << "FIND ANS!" << std::endl;

                LockMatrix *EndTemp = TempMatrix;
                while (EndTemp->parent != NULL && EndTemp->pre.first.first != -1) {
                    AnsCnt++;
//                    std::cout << "(" << EndTemp->pre.first.first << "," << EndTemp->pre.first.second << ",)"
//                              << EndTemp->pre.second << std::endl;
//                    std::cout << EndTemp->h << "," << EndTemp->g << std::endl;
//                    for (const auto &s : EndTemp->matrix) {
//                        for (const auto &value : s) {
//                            std::cout << value << " ";
//                        }
//                        std::cout << std::endl;
//                    }
                    AnsList.push_back(EndTemp->pre);
                    EndTemp = EndTemp->parent;
                }
                break;
            }

            ExtendFrontierWithLimit(LMQueue, TempMatrix,limit);
            VisitedList.push_back(TempMatrix->matrix);
        }
        limit.first = limit.second;
    }

}
#pragma endregion



#pragma region ##测试函数##
/**
 * @Function 打印一个矩阵的每个点转换为每个类型后的矩阵:  printTypeTurn
 *
 * 函数详细描述，包括输入输出参数等。
 *
 *  @param[in] lockMatrix   原始矩阵
 *  @return vector<std::vector<int>>  lockMatrix 转换后的矩阵
 */
void printTypeTurn(const TwoDMatrix& lockMatrix) {
    int N = lockMatrix.size();

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            std::cout << "-------(" << i << ", " << j << "):" << std::endl;
            for (int k = 1; k < 5; k++) {
                TwoDMatrix tempMatrix = TypeTurn(k, lockMatrix, i, j);
                if (!tempMatrix.empty()) {
                    std::cout << "type: " << k << ":" << std::endl;
                    for (const auto& row : tempMatrix) {
                        for (const auto& value : row) {
                            std::cout << value << " ";
                        }
                        std::cout << std::endl;
                    }
                }
            }
        }
    }
}



/**
 * @Function 测试扩展边缘队列是否正常:PrintMatrixWithHValue
 *
 * 调用ExtendFrontier函数
 *
 *  @param[in] lockMatrix   原始矩阵
 *  @return 输出该矩阵的所有子结点以及启发值
 */
void PrintMatrixWithHValue(const TwoDMatrix& lockMatrix) {
    LockMatrix* PreMatrix = new LockMatrix(10, 20, lockMatrix);  // 使用带参构造函数创建对象

    LockMatrixQueue MatrixQueue;
    // 调用 ExtendFrontier 函数
    ExtendFrontier(MatrixQueue, PreMatrix);
    while (!MatrixQueue.empty()) {
        LockMatrix* CurrMatrix = MatrixQueue.top();
        MatrixQueue.pop();
        std::cout << "===============" << std::endl;
        std::cout << "Matrix: " << std::endl;
        for (const auto& row : CurrMatrix->matrix) {
            for (const auto& value : row) {
                std::cout << value << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "h: " << CurrMatrix->h << std::endl;
        auto pre =  CurrMatrix->pre;
        std::cout << "(" <<  pre.first.first << ", " <<  pre.first.second << "), " <<  pre.second << std::endl;
        std::cout << "==============" << std::endl;
        delete CurrMatrix;
    }
}

#pragma endregion

#pragma region  ##main函数##

/**
 * @Function 读取锁盘文件:  readLockFile
 *
 * 函数详细描述，包括输入输出参数等。
 *
 *  @param[in] filename   文件路径/文件名
 *  @return vector<std::vector<int>>  lockMatrix 对应的矩阵
 */
TwoDMatrix readLockFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "无法打开文件 " << filename << std::endl;
        return {};
    }
    int N;
    file >> N;

    TwoDMatrix lockMatrix(N, std::vector<int>(N, 0));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            file >> lockMatrix[i][j];
        }
    }
    file.close();
    return lockMatrix;
}

int main() {

    //选择A*
    vector<string> type = {"AStar","IDAStar"};
    string func = type[0];

    //选择启发式
    vector<string> heuristicFunc = {"DisjointL","NumOf_1","Manhattan"};
    heuristic = heuristicFunc[1];

    std::cout << "使用"+ func << "+" + heuristic + "启发式"<< std::endl;

    for (int u = 5; u < 6; u++) {
        AnsCnt = 0;
        AnsList = vector<pair<pair<int, int>, int>>();
        string input = "../input/input" + std::to_string(u) + ".txt";
//        string input = "../input/input4.txt";
        string output = "../output/output" + std::to_string(u) + ".txt";
        TwoDMatrix lockMatrix = readLockFile(input);

        bool Test = true;
        if (Test) {
#pragma region  ##测试##
//            int N = lockMatrix.size();
//            std::cout << "是否测试模式：1/Y 0/N" << std::endl;
//            std::cout << "锁盘数据:" << std::endl;
//            // 打印读取的锁盘数据
//            for (const auto &row : lockMatrix) {
//                for (const auto &value : row) {
//                    std::cout << value << " ";
//                }
//                std::cout << std::endl;
//            }

//测试启发式函数
//        std::cout << "测试启发式函数:" << std::endl;
//        std::cout << Heuristic(lockMatrix) << std::endl;
//测试根据‘L’朝向转换矩阵函数
//        std::cout << "测试根据‘L’朝向转换矩阵函数:" << std::endl;
//        printTypeTurn(lockMatrix);
//测试拓展边缘队列
//        std::cout << "测试拓展边缘队列:" << std::endl;
//        PrintMatrixWithHValue(lockMatrix);
//测试AStar
#pragma endregion
            //开始时间
            clock_t startTime, endTime;
            startTime = clock();
            if(func == "AStar"){
                AStar(lockMatrix);
            } else if(func == "IDAStar" ){
                IDAStar(lockMatrix);
            }
            //结束时间
            endTime = clock();
            cout <<"input"+ std::to_string(u) <<"运行时间:"<< (double) (endTime - startTime) / CLOCKS_PER_SEC << "s" ;
            cout << "  解步骤数:" << AnsCnt << std::endl;

#pragma region  ##输出##
            // 使用反向迭代器进行倒序输出
            ofstream outfile(output);

            if (!outfile.is_open()) {
                std::cout << "Cannot open file: " << output << std::endl;
                return -1;
            }

//            outfile << AnsList.size() << endl;;
            for (auto it = AnsList.rbegin(); it != AnsList.rend(); ++it) {
                std::cout << "坐标：(" <<it->first.first<< ","<<it->first.second  << ")  方式："<<it->second << std::endl;
//                outfile << it->first.first << "," << it->first.second <<","<<it->second << endl;;
            }

//    }
#pragma endregion

        }
    }
    return 0;
}
//你想划分的部分代码

#pragma endregion