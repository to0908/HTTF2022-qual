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
vector<vector<int>> skill(M); // 問題のd, s
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
int remainWorker = M;
MedianManager<int> median;
vector<vector<int>> minimumSkill(M);
vector<array<int,3>> working(M, {-1, -1, -1}); // {task, 開始したday, estimateDay}
vector<int> WorkerNorm(M);
vector<vector<array<int,2>>> doneTask(M); // doneTask[person] = vector<{taskIdx, かかった日数}>
/////////////////////////////////////////////////////////

int estimateDay(int person, int task){
    int est = 0;
    for(int i=0;i<K;i++) {
        est += max(0, d[task][i] - skill[person][i]);
    }
    return max(1, est);
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

int calcL1norm(vector<int> &v){
    int sum = 0;
    for(auto i:v) sum += i;
    return sum;
}

const int eps = 0.5;
void estimateSkill(const int person, Timer &time){
    remainWorker++;
    // TODO: ずれの方向から、焼きなましの方向を決める。

    // 今のday, working[person]の情報から更新
    int past = day - working[person][1] + 1;
    doneTask[person].push_back({working[person][0], past});
    int gap = abs(past - working[person][2]);
    for(int i=0;i<K;i++){
        chmax(minimumSkill[person][i], d[working[person][0]][i] - past);
    }
    working[person] = {-1, -1, -1};
    if(gap == 0) return;
    bool changed = false;
    changed=1;
    for(int i=0;i<K;i++) skill[person][i] = minimumSkill[person][i];

    // K個パラメータがあって、全ての条件を満たすようにskill[person]を変更する
    int loss = calcLoss(person);
    int notZero = 0;
    for(int i=0;i<K;i++){
        if(skill[person][i] != 0) {
            notZero++;
            break;
        }
    }
    vector<int> bestSkill = skill[person];

    const int yakiR = 1500;
    int iter=1000;
    while(iter--){
        int p = randint() % K;
        int inc = randint() % 2;
        bool dec = false;
        if(notZero < Kdiv2 or inc==0 or skill[person][p] == minimumSkill[person][p]) {
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
        median.erase(WorkerNorm[person]);
        skill[person] = bestSkill;
        norm = calcL1norm(skill[person]);
        WorkerNorm[person] = norm;
        median.insert(WorkerNorm[person]);
        // cout << "#s " << person + 1 << " " << skill[person] << endl;
    }
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
        else if(!freeTaskQue[SMALL].empty()){
            auto [we, task] = freeTaskQue[SMALL].top(); freeTaskQue[SMALL].pop();
            tasks.emplace_back(task);
        }
        else if(!freeTaskQue[LARGE].empty()) {
            auto [we, task] = freeTaskQue[LARGE].top(); freeTaskQue[LARGE].pop();
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


void assignTask2(){

    vector<int> ans;
    int sz = 0;
    auto addAns = [](vector<int> &ans, int &sz, int &person, int &task) {
        sz++;
        ans.emplace_back(person+1);
        ans.emplace_back(task+1);
    };

    vector<int> tasks;
    int cnt = 0;
    int LaMid = 0, SmMid = 0;
    for(int i=0;i<M;i++) {
        if(working[i][0] == -1) {
            if(WorkerNorm[i] < median.get()) SmMid++;
            else LaMid++;
        }
    }
    while(LaMid){
        if(!freeTaskQue[LARGE].empty()) {
            auto [we, task] = freeTaskQue[LARGE].top(); freeTaskQue[LARGE].pop();
            tasks.emplace_back(task);
        }
        else break;
        LaMid--;
        cnt++;
    }
    while(SmMid){
        if(!freeTaskQue[SMALL].empty()){
            auto [we, task] = freeTaskQue[SMALL].top(); freeTaskQue[SMALL].pop();
            tasks.emplace_back(task);
        }
        else break;
        SmMid--;
        cnt++;
    }
    int rem = LaMid + SmMid;
    while(rem--) {
        if(!freeTaskQue[LARGE].empty()) {
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
    rem = (int)freeTaskQue[0].size() + (int)freeTaskQue[1].size();
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

    auto f = [](int &dp, int &cost, int &rem) {
        if(rem < 10) return max(dp, cost);
        return dp + cost;
    };

    for(int i=0; i<cnt; i++){
        for(int bit=(1<<i)-1; bit < (1<<m); bit = next_combination(bit)){
            for(int j=0; j<m; j++){
                if(bit & (1<<j)) continue;
                if(chmin(dp[bit | (1<<j)], f(dp[bit], cost[j][i], rem))){
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
        if(remainNonFreeTaskCount) assignTask();
        else assignTask2();
        if(dayEnd(time)) return;
        day++;
    }
}

void initSkill(){
    // skillの初期化
    for(int i=0;i<M;i++){
        skill[i].resize(K);
        minimumSkill[i].resize(K);
        int sum = 0;
        for(int j=0;j<K;j++){
            skill[i][j] = (int)randint() % randMa + 1;
            sum += skill[i][j] * skill[i][j];
        }
        WorkerNorm[i] = sum;
        median.insert(sum);
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
    // randMa /= 2;
    for(int i=0;i<R;i++){
        int a,b;
        cin>>a>>b;
        a--,b--;
        v[a].emplace_back(b);
        rev[b].emplace_back(a);
    }
    initSkill();
    initTask();
    solve(time);

    cerr << "[time] " << time.elapsed() << " ms"<< endl;
}