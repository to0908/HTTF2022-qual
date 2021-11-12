#include<bits/stdc++.h> 
using namespace std;
#define all(x) (x).begin(),(x).end()
template<typename T1,typename T2> bool chmin(T1 &a,T2 b){if(a<=b)return 0; a=b; return 1;}
template<typename T1,typename T2> bool chmax(T1 &a,T2 b){if(a>=b)return 0; a=b; return 1;}
template< typename T > ostream &operator<<(ostream &os, const vector< T > &v) {
    for(int i = 0; i < (int) v.size(); i++) os << v[i] << (i + 1 != (int) v.size() ? " " : "");
    return os;
}
#include<chrono>
struct Timer {
    chrono::high_resolution_clock::time_point st;

    Timer() { reset(); }

    void reset() {
        st = chrono::high_resolution_clock::now();
    }

    chrono::milliseconds::rep elapsed() {
        auto ed = chrono::high_resolution_clock::now();
        return chrono::duration_cast< chrono::milliseconds >(ed - st).count();
    }
};
unsigned int randint() {
    static unsigned int tx = 123456789, ty=362436069, tz=521288629, tw=88675123;
    unsigned int tt = (tx^(tx<<11));
    tx = ty; ty = tz; tz = tw;
    return ( tw=(tw^(tw>>19))^(tt^(tt>>8)) );
}

struct NN {
    void print_skill(){ cout << "#s " << skill << endl; }

    NN(){}
    NN(int k){
        K = k;
        d = 0;
        sig_d = 0;
        grad_d = 0;
        S = vector<double>(k, 0);
        s = vector<double>(k, 0);
        grad_s = vector<double>(k, 0);
        sig_s = vector<double>(k, 0);
        skill = vector<int> (k, 1);
        update_Ss();
        D = compute_D();
        Q = compute_Q();
        update_skill();
    }
    void clear_grad(){
        for(int i=0;i<K;i++) grad_s[i] = 0;
        grad_d = 0;
    }
    double compute_D(){
        return 20 + 40 * sigmoid(d);
    }
    double compute_Q(){
        double ret = 0;
        for(int i=0;i<K;i++) ret += S[i] * S[i];
        return D / sqrt(ret);
    }

    void update_Ss(){
        for(int i=0;i<K;i++){
            sig_s[i] = sigmoid(s[i]);
            S[i] = sig_s[i] * 4.0;
        }
    }
    void update_skill(){
        for(int i=0;i<K;i++) skill[i] = round(Q * s[i]);
    }
    int predict(vector<int> &v){
        assert((int)v.size() == K);
        int ret = 0;
        for(int i=0;i<K;i++) ret += max(0, v[i] - skill[i]);
        return max(1, ret);
    }

    void train(vector<vector<int>> &X, vector<int> &y, double lr=12){
        clear_grad();
        sig_d = sigmoid(d);
        D = compute_D();
        update_Ss();
        Q = compute_Q();
        update_skill();

        int n = X.size();
        for(int i=0;i<n;i++){
            double der_E = predict(X[i]) - y[i];
            for(int j=0;j<K;j++){
                // double der_Q = S[i];
                double der_D = der_E * Q / D;
                double der_d = der_D * 40 * inv_sig(sig_d);
                double der_S = der_E * - S[i] * Q / D * Q / D* Q;
                double der_s = der_S * inv_sig(sig_s[i]) * 4.0;
                grad_d += der_d;
                grad_s[i] += der_s;
            }
        }
        double mul = lr / (double)X.size();
        d -= grad_d * mul;
        for(int i=0;i<K;i++) s[i] -= grad_s[i] * mul;
    }

private:
    int K;
    vector<int> skill;

    double D, Q;
    vector<double> S;

    double sig_d;
    vector<double> sig_s;

    double d;
    vector<double> s;

    double grad_d;
    vector<double> grad_s;

    static double sigmoid(double x){
        return 1.0 / (1.0 + exp(-x));
    }
    static double inv_sig(double sig_v){
        return sig_v * (1 - sig_v);
    }
};

const int N = 1000, M = 20;
int K, R, Kdiv2, randMa;
vector<vector<int>> d(N);
vector<vector<int>> v(N), rev(N); // 入力のDAGと逆DAG
int day = 0; // 現在の日数
const int INF = 1e9;
////////////// Task //////////
const int LARGE=1, SMALL=0;
int freeTaskNromTh, remainNonFreeTaskCount; // 下にタスクが何もないタスクの大小の閾値と数
priority_queue<array<int, 2>> taskQue; // {順番, task index}
priority_queue<array<int, 2>> freeTaskQue[2]; // [Large/Small]下のタスクがないもの {L2norm, task index}
vector<int> rCnt(N); // 依存のカウント(自分より下の個数)
vector<array<int,2>> taskWeight(N); // タスクの重み, {順番, L2norm}
int doneTaskCount = 0, doneTaskThreshold = 900, attenuate=0.7; // 終わったタスクの数
int notReleased = N; // まだ開始することができない残りの仕事の数
///////////// Worker /////////
vector<NN> Nets(M);
void setNeuralNet() {
    for(int i=0;i<M;i++) Nets[i] = NN(K);
    cerr << "OK" << endl;
}
int remainWorker = M;
vector<array<int,3>> working(M, {-1, -1, -1}); // {task, 開始したday, estimateDay}
vector<vector<array<int,2>>> doneTask(M); // doneTask[person] = vector<{taskIdx, かかった日数}>
vector<vector<int>> doneTaskDay(M); // かかった日数
vector<vector<vector<int>>> doneTaskSkill(M); // d
/////////////////////////////////////////////////////////

double estimateDay(int person, int task){
    return Nets[person].predict(d[task]);
}
int calcLoss(int person){
    int loss = 0;
    // Mean Square Error
    for(auto [task, past]: doneTask[person]){
        int est = estimateDay(person, task);
        loss += (past - est) * (past - est);
    }
    return loss;
}
double calcL1norm(vector<double> &v){
    int sum = 0;
    for(auto i:v) sum += i;
    return sum;
}
int calcL1norm(vector<int> &v){
    int sum = 0;
    for(auto i:v) sum += i;
    return sum;
}

const int eps = 0.5;
bool debug = true;
int epoch = 2;
int batch = 8;
void estimateSkill(const int person, Timer &time){
    remainWorker++;
    // TODO: ずれの方向から、焼きなましの方向を決める。

    // 今のday, working[person]の情報から更新
    int past = day - working[person][1] + 1;
    int task = working[person][0];
    int estpast = working[person][2];
    if(debug) cerr << "true/est/dif: " << past << " " << estpast << " " << past - estpast << (abs(past - estpast) <= 5 ? " ------ok------":"") << endl;
    // doneTask[person].push_back({task, past});
    doneTaskDay[person].push_back(past);
    doneTaskSkill[person].push_back(d[task]);
    if(past - estpast == 0) return;
    for(int epo=0;epo<epoch;epo++){
        vector<vector<int>> trainX;
        vector<int> trainY;
        while((int)trainX.size() < batch){
            int x = randint() % (int)doneTaskDay[person].size();
            trainX.emplace_back(doneTaskSkill[person][x]);
            trainY.emplace_back(doneTaskDay[person][x]);
        }
        Nets[person].train(trainX, trainY);
    }
    if(debug) cerr << "est - past: " << Nets[person].predict(doneTaskSkill[person].back()) - past << endl;
}

int next_combination(int sub) {
    int x = sub & -sub, y = sub + x;
    return (((sub & ~y) / x) >> 1) | y;
}

void assignTask(){

    vector<int> ans;
    int sz = 0;
    auto addAns = [](vector<int> &ans, int &sz, int &person, int &task) {
        sz++;
        ans.emplace_back(person+1);
        ans.emplace_back(task+1);
    };

    int workerCount = remainWorker;
    vector<int> tasks;
    int cnt = 0;
    while(workerCount--) {
        if(!taskQue.empty()){
            auto [we, task] = taskQue.top(); taskQue.pop();
            tasks.emplace_back(task);
            remainNonFreeTaskCount--;
        }
        else if(!freeTaskQue[LARGE].empty()) {
            auto [we, task] = freeTaskQue[LARGE].top(); freeTaskQue[LARGE].pop();
            tasks.emplace_back(task);
        }
        else if(!freeTaskQue[SMALL].empty()){
            auto [we, task] = freeTaskQue[SMALL].top(); freeTaskQue[SMALL].pop();
            tasks.emplace_back(task);
        }
        else break;
        cnt++;
    }
    if(cnt == 0) {
        cout << 0 << endl;
        return;
    }
    vector<int> worker;
    vector<vector<int>> cost;
    for(int i=0;i<M;i++) {
        if(working[i][0] == -1) {
            worker.emplace_back(i);
            vector<int> t;
            for(auto j:tasks){
                t.emplace_back(estimateDay(i, j));
            }
            cost.emplace_back(t);
        }
    }
    int m = worker.size();
    vector<int> dp(1<<m, INF);
    vector<array<int,3>> pos(1<<m, {-1,-1,-1});
    dp[0] = 0;
    int mi = INF, mibit = -1;
    
    for(int i=0; i<cnt; i++){
        for(int bit=(1<<i)-1; bit < (1<<m); bit = next_combination(bit)){
            for(int j=0; j<m; j++){
                if(bit & (1<<j)) continue;
                if(chmin(dp[bit | (1<<j)], dp[bit] + cost[j][i])){
                    pos[bit | (1<<j)] = {bit, j, i};
                }
                if(i == cnt - 1 && chmin(mi, dp[bit | (1<<j)])){
                    mibit = (bit | (1<<j));
                }
            }
            if(bit == 0) break;
        }
    }
    while(mibit != 0) {
        remainWorker--;
        auto [nb, person_i, task_i] = pos[mibit];
        mibit = nb;
        int person = worker[person_i];
        int task = tasks[task_i];
        working[person] = {task, day, cost[person_i][task_i]};
        addAns(ans, sz, person, task);
    }
    cout << sz << " ";
    cout << ans << endl;
}



bool dayEnd(Timer &time){
    int n; cin>>n;
    if(n == -1) return true;
    doneTaskCount += n;
    for(int i=0;i<n;i++){
        int person; cin>>person; person--;
        int task = working[person][0];
        estimateSkill(person, time);
        for(auto x:v[task]){
            rCnt[x]--;
            if(rCnt[x] == 0) {
                notReleased--;
                if(v[x].size() == 0){
                    freeTaskQue[taskWeight[x][1] >= freeTaskNromTh].push({taskWeight[x][1], x});
                }
                else{
                    taskQue.push({taskWeight[x][0] + taskWeight[x][1], x});
                }
            }
        }
        working[person] = {-1, -1, -1};
    }
    return false;
}


void solve(Timer &time){
    while(true){
        assignTask();
        if(dayEnd(time)) return;
        day++;
    }
}


void initTask(){
    queue<array<int,2>> q;
    vector<int> ftn;
    for(int i=0;i<N;i++){
        taskWeight[i][1] = calcL1norm(d[i]);
        if(v[i].size() == 0) {
            ftn.push_back(taskWeight[i][1]);
        }
        else remainNonFreeTaskCount++;
    }
    sort(all(ftn));
    int x = (int)ftn.size();
    freeTaskNromTh = (ftn[(x-1) / 2] + ftn[x/2]) / 2;
    for(int i=0;i<N;i++){
        rCnt[i] = rev[i].size();
        if(v[i].size() == 0) {
            q.push({0, i});
            if(rCnt[i] == 0) {
                freeTaskQue[taskWeight[i][1] >= freeTaskNromTh].push({taskWeight[i][1], i});
            }
            continue;
        }
    }

    for(int i=0;i<N;i++){
        // タスクの重み計算
        queue<int> q;
        bool used[N]={};
        q.push(i);
        while(q.size()){
            int p = q.front(); q.pop();
            taskWeight[i][0] += taskWeight[p][1];
            for(auto x:v[p]){
                if(!used[x]){
                    used[x] = 1;
                    q.push(x);
                }
            }
        }
        taskWeight[i][0] -= taskWeight[i][1];
        if(rev[i].size() == 0 && v[i].size()) {
            taskQue.push({taskWeight[i][0] + taskWeight[i][1], i});
        }
        // cerr << i << " " << taskWeight[i][0] << " " << taskWeight[i][1] << endl;
    }
    // 最初に軽いタスクで実力チェック
    if(R < 2000)
    {
        priority_queue<array<int,3>,vector<array<int,3>>, greater<array<int,3>>> pq;
        while(taskQue.size()){
            auto [we, task] = taskQue.top(); taskQue.pop();
            pq.push({we, task, 0});
        }
        while(freeTaskQue[LARGE].size()) {
            auto [we, task] = freeTaskQue[LARGE].top(); freeTaskQue[LARGE].pop();
            pq.push({we, task, 1});
            remainNonFreeTaskCount++;
        }
        while(freeTaskQue[SMALL].size()) {
            auto [we, task] = freeTaskQue[SMALL].top(); freeTaskQue[SMALL].pop();
            pq.push({we, task, 2});
            remainNonFreeTaskCount++;
        }
        int ma = 40;
        while(ma && pq.size()){
            auto [we, task, p] = pq.top(); pq.pop();
            ma--;
            taskQue.push({(int)1e9+7 + we, task});
        }
        while(pq.size()){
            auto [we, task, p] = pq.top(); pq.pop();
            if(p == 0) {
                taskQue.push({we, task});
                continue;
            }
            else if(p == 1) freeTaskQue[LARGE].push({we, task});
            else if(p == 2) freeTaskQue[SMALL].push({we, task});
            remainNonFreeTaskCount--;
        }
    }
}

signed main(){
    ios::sync_with_stdio(false);
    cin.tie(0);

    Timer time;

    cin>>K>>K>>K>>R;
    Kdiv2 = K / 2;
    for(int i=0;i<N;i++){
        d[i].resize(K);
        for(int j=0;j<K;j++){
            cin>>d[i][j];
            chmax(randMa, d[i][j]);
        }
    }
    chmax(randMa, 30);
    for(int i=0;i<R;i++){
        int a,b;
        cin>>a>>b;
        a--,b--;
        v[a].emplace_back(b);
        rev[b].emplace_back(a);
    }
    initTask();
    setNeuralNet();
    solve(time);

    cerr << "[time] " << time.elapsed() << " ms"<< endl;
}