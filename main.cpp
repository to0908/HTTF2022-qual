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
template< typename T > struct MedianManager {
    T median;
    int n;
    multiset<T> r;
    multiset<T, greater<T>> l;

    MedianManager(){
        median = 0;
        n = 0;
    }

    T get(){
        // 偶数の時は小さい方を返す
        return median;
    }

    T rsize(){ return r.size(); }
    T lsize() { return l.size(); }

    T insert(T x){
        n++;
        if(n == 1) {
            median = x;
            return median;
        }
        if(median < x) {
            r.insert(x);
            if(n % 2) {
                l.insert(median);
                rpop();
            }
        }
        else{
            l.insert(x);
            if(n % 2 == 0) {
                r.insert(median);
                lpop();
            }
        }
        return median;
    }

    T erase(T x){
        assert(n > 0);
        n--;
        if(n == 0) {
            median = 0;
            return 0;
        }
        if(x == median) {
            if(n % 2) {
                rpop();
            }
            else {
                lpop();
            }
        }
        else if(x < median) {
            auto itr = l.find(x);
            assert(itr != l.end());
            l.erase(itr);
            if(n % 2) {
                l.insert(median);
                rpop();
            }
        }
        else if(x > median) {
            auto itr = r.find(x);
            assert(itr != r.end());
            r.erase(itr);
            if(n % 2 == 0) {
                r.insert(median);
                lpop();
            }
        }
        return median;
    }

private:
    void rpop() {
        median = *r.begin();
        r.erase(r.begin());
    }
    void lpop() {
        median = *l.begin();
        l.erase(l.begin());
    }
};


const int N = 1000, M = 20;
int K, R, Kdiv2, randMa;
vector<vector<int>> d(N);
vector<vector<double>> skill(M); // 問題のd, s
vector<vector<int>> v(N), rev(N); // 入力のDAGと逆DAG
int day = 0; // 現在の日数

////////////// Task //////////
priority_queue<array<int, 2>> taskQue; // {順番, task index}
priority_queue<array<int, 2>> freeTaskQue; // 下のタスクがないもの {L2norm, task index}
vector<int> rCnt(N); // 依存のカウント(自分より下の個数)
vector<array<int,2>> taskWeight(N); // タスクの重み, {順番, L2norm}
int doneTaskCount = 0, doneTaskThreshold = 900, attenuate=0.7; // 終わったタスクの数
int notReleased = N; // まだ開始することができない残りの仕事の数
///////////// Worker /////////
int remainWorker = M;
vector<vector<int>> minimumSkill(M);
vector<array<int,3>> working(M, {-1, -1, -1}); // {task, 開始したday, estimateDay}
vector<double> WorkerNorm(M);
MedianManager<double> WorkerNormMedian;
vector<vector<array<int,2>>> doneTask(M); // doneTask[person] = vector<{taskIdx, かかった日数}>
int doLargeTask = -900; // doLargeTask < doneTaskCountの間だけやる
/////////////////////////////////////////////////////////


double estimateDay(int person, int task){
    double est = 0;
    for(int i=0;i<K;i++) {
        est += max(0.0, d[task][i] - skill[person][i]);
    }
    return max(1.0, est);
}
double calcLoss(int person){
    double loss = 0;
    // Mean Square Error
    for(auto [task, past]: doneTask[person]){
        double est = estimateDay(person, task);
        loss += (past - est) * (past - est);
    }
    // Ridge
    // for(int i=0;i<K;i++){
    //     loss += skill[person][i] * skill[person][i] * (int)doneTask[person].size() * 0.5;
    // }
    return loss;
}

double calcL2norm(vector<double> &v, bool isSqrt=true){
    double sum = 0;
    for(auto i:v) sum += i * i;
    if(isSqrt) sum = sqrt(sum);
    return sum;
}
int calcL2norm(vector<int> &v, bool isSqrt=true){
    double sum = 0;
    for(auto i:v) sum += i * i;
    if(isSqrt) sum = sqrt(sum);
    return sum;
}

const double eps = 0.5;
void estimateSkill(const int person, Timer &time){
    remainWorker++;
    // TODO: ずれの方向から、焼きなましの方向を決める。

    // 今のday, working[person]の情報から更新
    int past = day - working[person][1] + 1;
    doneTask[person].push_back({working[person][0], past});
    int gap = abs(past - working[person][2]);
    if(past == 1) {
        for(int i=0;i<K;i++){
            chmax(minimumSkill[person][i], d[working[person][0]][i]);
        }
    }
    working[person] = {-1, -1, -1};
    if(gap == 0) return;
    
    bool changed = false;
    int hoge = 100;
    while(hoge--){
        changed = true;
        for(auto [task, cost]:doneTask[person]){
            double est = estimateDay(person, task);
            double gap = est - cost;
            if(gap < eps) continue;
            // bool over = gap > 0;
            gap /= 100;
            for(int i=0;i<K;i++){
                // d>sか、over/under estimateで場合分けをして変える方が良い？
                skill[person][i] += (d[task][i] - skill[person][i]) * gap;
                // if(d[task][i] > skill[person][i]) {
                //     if(over) skill[person][i] += gap;
                //     else skill[person][i] -= gap;
                // }
                // else {
                //     if(over) {
                //         int k = randint() % 3;
                //         if(k == 2) k = -1;
                //         skill[person][i] += gap * k;
                //     }
                //     else skill[person][i] -= gap;
                // }
                chmax(skill[person][i], minimumSkill[person][i]);
            }
        }
    }
    // K個パラメータがあって、全ての条件を満たすようにskill[person]を変更する
    int loss = calcLoss(person);
    int notZero = 0;
    for(int i=0;i<K;i++){
        if(skill[person][i] != 0) {
            notZero++;
            break;
        }
    }
    vector<double> bestSkill = skill[person];

    const int yakiR = 1500;
    int iter=1000;
    while(iter--){
        int p = randint() % K;
        int inc = randint() % 2;
        bool dec = false;
        if(notZero < Kdiv2 or inc==0) {
            if(skill[person][p] == 0) notZero++;
            skill[person][p]++;
        }
        else{
            dec = 1;
            while(skill[person][p] == 0) p = randint() % K;
            skill[person][p]--;
            if(skill[person][p] == 0) notZero--;
        }
        int lossNext = calcLoss(person);
        if(lossNext < loss){
            changed = true;
            loss = lossNext;
            bestSkill = skill[person];
            if(loss == 0) break;
            continue;
        }
        else if(yakiR * (2000 - time.elapsed()) > 2000*(randint()%yakiR)){
            // force Next
            continue;
        }
        else {
            if(dec) skill[person][p]++;
            else skill[person][p]--;
        }
    }
    for(int i=0;i<K;i++) {
        chmax(bestSkill[i], minimumSkill[person][i]);
    }
    
    int norm = WorkerNorm[person];
    if(changed) {
        WorkerNormMedian.erase(norm);
        skill[person] = bestSkill;
        norm = calcL2norm(skill[person], false);
        WorkerNormMedian.insert(norm);
        WorkerNorm[person] = norm;
        cout << "#s " << person + 1 << " " << skill[person] << endl;
    }
}

void assignTask(){

    vector<int> ans;
    int sz = 0;
    auto addAns = [](vector<int> &ans, int &sz, int &person, int &task) {
        working[person] = {task, day, (int)estimateDay(person, task)};
        sz++;
        ans.emplace_back(person+1);
        ans.emplace_back(task+1);
    };

    int workerCount = remainWorker;
    priority_queue<array<int,2>> pq;
    while(workerCount--) {
        if(!taskQue.empty()){
            auto [idx, task] = taskQue.top(); taskQue.pop();
            pq.push({taskWeight[task][1] ,task});
        }
        else if(!freeTaskQue.empty()){
            auto [we, task] = freeTaskQue.top(); freeTaskQue.pop();
            pq.push({we ,task});
        }
        else break;
    }
    while(pq.size()) {
        auto [we, task] = pq.top(); pq.pop();
        remainWorker--;
        double mi = 1e9;
        int idx = -1;
        for(int i=0;i<M;i++){
            if(working[i][0] != -1) continue;
            if(chmin(mi, estimateDay(i, task))) idx = i;
        }
        working[idx] = {task, day, (int)mi};
        addAns(ans, sz, idx, task);
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
                if(v[x].size() == 0) freeTaskQue.push({taskWeight[x][1], x});
                else{
                    taskQue.push({taskWeight[x][0], x});
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

void init(){
    // skillの初期化
    for(int i=0;i<M;i++){
        skill[i].resize(K);
        minimumSkill[i].resize(K);
        int sum = 0;
        for(int j=0;j<K;j++){
            skill[i][j] = max(1, (int)randint() % randMa);
            sum += skill[i][j] * skill[i][j];
        }
        WorkerNorm[i] = sum;
        WorkerNormMedian.insert(sum);
    }
    int cnt[N]={};
    queue<int> q;
    for(int i=0;i<N;i++){
        cnt[i] = v[i].size();
        rCnt[i] = rev[i].size();
        taskWeight[i][1] = calcL2norm(d[i], false);
        if(v[i].size() == 0) {
            q.push(i);
            taskWeight[i][0] = -1;
            if(rCnt[i] == 0) freeTaskQue.push({taskWeight[i][1], i});
            continue;
        }
    }
    int now = 0;
    while(q.size()){
        int p = q.front();
        q.pop();
        if(v[p].size() != 0) {
            taskWeight[p][0] = now;
            if(rev[p].size() == 0) taskQue.push({now, p});
            now++;
        }
        for(auto i:rev[p]) {
            cnt[i]--;
            if(cnt[i] == 0){
                q.push(i);
            }
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
    randMa /= 2;
    for(int i=0;i<R;i++){
        int a,b;
        cin>>a>>b;
        a--,b--;
        v[a].emplace_back(b);
        rev[b].emplace_back(a);
    }
    init();
    solve(time);

    cerr << "[time] " << time.elapsed() << " ms"<< endl;
}