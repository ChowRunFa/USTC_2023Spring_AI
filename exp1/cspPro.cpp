#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <queue>
#include <set>

using pint = std::pair<int, int>;
template<typename T>
using vec = std::vector<T>;
template<typename T>
using vec2 = vec<vec<T>>;
template<typename T>
using vec3 = vec<vec2<T>>;

class CSP {
public:
    explicit CSP(const std::string &);

    bool backtrack();

    void printSolution(const std::string &);

    bool check();

private:
    vec<int> sum, times;
    vec2<int> shifts;
    vec3<bool> req;
    vec2<std::set<int>> domain;
    vec2<pint> count;
    vec2<std::set<pint>> ordered;
    std::set<std::pair<pint, pint>> assignments;
    int N{}, D{}, S{}, FAIR_LIMIT{}, REQ_LIMIT{}, notReqCnt{};

    void updateSum(int);

    void backSum(int);

    bool checkFairness();

    inline void updateCount(int, int, int, bool);

    bool updateDomain(int, int, int, bool);

    bool inferDomain(int, int, int, bool);

    void orderDomain(int d, int s);
};

CSP::CSP(const std::string &file) {
    std::ifstream infile(file);
    if (!infile.is_open()) {
        std::cout << "Error opening metadata file!\n";
        exit(1);
    }

    int num;
    char c;
    infile >> N >> c >> D >> c >> S;
    FAIR_LIMIT = D * S - ((D * S) / N);
    REQ_LIMIT = 1;
    notReqCnt = 0;
    sum.resize(N);
    times.resize(N);
    shifts.resize(D, vec<int>(S));
    count.resize(D, vec<pint>(S));
    req.resize(N, vec2<bool>(D, vec<bool>(S)));
    domain.resize(D, vec<std::set<int>>(S));
    ordered.resize(D, vec<std::set<pint>>(S));

    for (int n = 1; n <= N; ++n) {
        for (int d = 0; d < D; ++d) {
            for (int s = 0; s < S; ++s) {
                infile >> num;
                infile.ignore(1, ',');
                req[n - 1][d][s] = num;
                domain[d][s].insert(n);
                count[d][s].first += num;
            }
        }
    }
    for (int d = 0; d < D; ++d) {
        for (int s = 0; s < S; ++s) {
            count[d][s].second = N - count[d][s].first;
            assignments.insert({count[d][s], {d, s}});
        }
    }
    infile.close();
}

void CSP::updateSum(int n) {
    for (int i = 0; i < N; ++i) {
        if (i != (n - 1)) {
            ++sum[i];
        }
    }
    ++times[n - 1];
}

void CSP::backSum(int n) {
    for (int i = 0; i < N; ++i) {
        if (i != (n - 1)) {
            --sum[i];
        }
    }
    --times[n - 1];
}

bool CSP::checkFairness() {
    for (int i = 0; i < N; ++i) {
        if (sum[i] > FAIR_LIMIT) {
            return false;
        }
    }
    return notReqCnt <= REQ_LIMIT;
}

inline void CSP::updateCount(int d, int s, int n, bool flag) {
    if (req[n - 1][d][s]) {
        count[d][s].first += flag ? -1 : 1;
    } else {
        count[d][s].second += flag ? -1 : 1;
    }
}

bool CSP::updateDomain(int d, int s, int n, bool flag) {
    auto it = assignments.find({count[d][s], {d, s}});
    if (flag) {
        updateCount(d, s, n, flag);
        domain[d][s].erase(n);
        if (domain[d][s].empty()) {
            return false;
        }
    } else {
        updateCount(d, s, n, flag);
        domain[d][s].insert(n);
        if (domain[d][s].size() == 1) {
            return false;
        }
    }
    assignments.erase(it);
    assignments.insert({count[d][s], {d, s}});
    return true;
}

bool CSP::inferDomain(int d, int s, int n, bool flag) {
    bool ret = true;
    if (s > 0 && !shifts[d][s - 1]) {
        ret = updateDomain(d, s - 1, n, flag);
    }
    if (!ret) return false;
    if (s < S - 1 && !shifts[d][s + 1]) {
        ret = updateDomain(d, s + 1, n, flag);
    }
    if (!ret) return false;
    if (s == S - 1 && d < D - 1 && !shifts[d + 1][0]) {
        ret = updateDomain(d + 1, 0, n, flag);
    }
    if (!ret) return false;
    if (d > 0 && s == 0 && !shifts[d - 1][S - 1]) {
        ret = updateDomain(d - 1, S - 1, n, flag);
    }
    return ret;
}

void CSP::orderDomain(int d, int s) {
    ordered[d][s].clear();
    for (auto n: domain[d][s]) {
        int tag = times[n - 1];
        tag += req[n - 1][d][s] ? 0 : N;
        ordered[d][s].insert({tag, n});
    }
}

bool CSP::backtrack() {
    if (assignments.empty()) {
        return true;
    }

    auto assignment = *assignments.begin();
    int d = assignment.second.first, s = assignment.second.second;
    assignments.erase(assignments.begin());
    orderDomain(d, s);

    for (auto &p: ordered[d][s]) {
        int n = p.second;
        shifts[d][s] = n;
        notReqCnt += req[n - 1][d][s] ? 0 : 1;
        updateSum(n);
        if (checkFairness()) {
            auto ret = inferDomain(d, s, n, true);
            if (ret && backtrack()) {
                return true;
            }
            inferDomain(d, s, n, false);
        }
        backSum(n);
        notReqCnt -= req[n - 1][d][s] ? 0 : 1;
    }

    shifts[d][s] = 0;
    assignments.insert(assignment);
    return false;
}

void CSP::printSolution(const std::string &file) {
    std::ofstream outfile(file);
    if (!outfile.is_open()) {
        std::cout << "Cannot open file: " << file << std::endl;
        return;
    }
    int cnt = 0;
    for (int d = 0; d < D; ++d) {
        for (int s = 0; s < S; ++s) {
            int n = shifts[d][s];
            outfile << n;
            if (s < S - 1) {
                outfile << ",";
            }
            cnt += req[n - 1][d][s];
        }
        outfile << std::endl;
    }
    outfile << cnt;
    outfile.close();
    std::cout << "Total number of requests satisfied: " << cnt << "/" << D * S << std::endl;
}

bool CSP::check() {
    // check constraint 1
    for (int d = 0; d < D; ++d) {
        for (int s = 0; s < S; ++s) {
            if (s > 0 && shifts[d][s] == shifts[d][s - 1]) {
                std::cout << "Constraint 1 violated at (" << d << ", " << s << ")" << std::endl;
                return false;
            }
            if (s < S - 1 && shifts[d][s] == shifts[d][s + 1]) {
                std::cout << "Constraint 1 violated at (" << d << ", " << s << ")" << std::endl;
                return false;
            }
            if (s == S - 1 && d < D - 1 && shifts[d][s] == shifts[d + 1][0]) {
                std::cout << "Constraint 1 violated at (" << d << ", " << s << ")" << std::endl;
                return false;
            }
            if (d > 0 && s == 0 && shifts[d][s] == shifts[d - 1][S - 1]) {
                std::cout << "Constraint 1 violated at (" << d << ", " << s << ")" << std::endl;
                return false;
            }
        }
    }
    // check constraint 2
    for (int n = 1; n <= N; ++n) {
        int cnt = 0;
        for (int d = 0; d < D; ++d) {
            for (int s = 0; s < S; ++s) {
                cnt += shifts[d][s] == n;
            }
        }
        if (cnt < D * S / N) {
            std::cout << "Constraint 2 violated for " << n << std::endl;
            return false;
        }
    }
    return true;
}

int main() {

    for (int i = 0; i < 10; ++i) {
        std::string input = "../input/input" + std::to_string(i) + ".txt";
        std::string output = "../output/output" + std::to_string(i) + ".txt";
        CSP csp(input);
        auto start = std::chrono::high_resolution_clock::now();
        if (csp.backtrack()) {
            std::cout << "Solution found!\n";
            csp.printSolution(output);
            if (csp.check()) {
                std::cout << "Solution correct!\n";
            } else {
                std::cout << "Solution incorrect!\n";
            }
        } else {
            std::cout << "No solution found!\n";
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
    }
    return 0;
}


