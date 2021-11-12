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


const int N = 1000, M = 20, LARGE=1, SMALL=0;
int K, R, Kdiv2, randMa;
vector<vector<int>> d(N), skill(M); // 問題のd, s
vector<vector<int>> v(N), rev(N); // 入力のDAGと逆DAG
int day = 0; // 現在の日数

////////////// Task //////////
int taskLSThreshold;
priority_queue<array<int,2>> taskQue[2]; // [LARGE/SMALL]{重要度, task index}
priority_queue<array<int,2>> freeTaskQue; // (v.size()==0),下のタスクがないもの {重要度, task index}
vector<int> rCnt(N); // 依存のカウント(自分より下の個数)
vector<array<int,2>> taskWeight(N); // タスクの重み, {子孫の数, L2ノルム}
vector<vector<array<int,2>>> doneTask(M); // doneTask[person] = vector<{taskIdx, かかった日数}>
int doneTaskCount = 0, doneTaskThreshold = 900, attenuate=0.7; // 終わったタスクの数
int notReleased = N; // まだ開始することができない残りの仕事の数
///////////// Worker /////////
priority_queue<array<int,2>> workerQue; // {L2 norm of skill, person}
vector<array<int,3>> working(M, {-1, -1, -1}); // {task, 開始したday, estimateDay}
vector<int> WorkerNorm(M);
MedianManager<int> WorkerNormMedian;
vector<vector<int>> minimumSkill(M);
int doLargeTask = -900; // doLargeTask < doneTaskCountの間だけやる
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

void estimateSkill(const int person, Timer &time){
    // TODO: ずれの方向から、焼きなましの方向を決める。

    // 今のday, working[person]の情報から更新
    int past = day - working[person][1] + 1;
    // doneTask[person].push_back({working[person][0], past});
    int gap = abs(past - working[person][2]);
    bool changed = false;
    for(int r=-1;r<=1;r++){
        if(past + r <= 0) continue;
        doneTask[person].push_back({working[person][0], past + r});
    }
    for(int i=0;i<K;i++){
        chmax(minimumSkill[person][i], d[working[person][0]][i] - past);
    }
    if(gap > 15){
        changed = true;
        for(int i=0;i<K;i++) {
            double x = 0;
            for(auto dt:doneTask[person]) {
                auto [tsk,da] = dt;
                x += d[tsk][i] * (8.0 - da) / 8.0;
            }
            x = (x + (int)doneTask.size() - 1) / (int)doneTask.size();
            skill[person][i] = int(x);
            chmax(skill[person][i], 1);
        }
    }
    for(int i=0;i<K;i++) chmax(skill[person][i], minimumSkill[person][i]);
    // K個パラメータがあって、全ての条件を満たすようにskill[person]を変更する
    int loss = calcLoss(person);
    int notZero = 0;
    for(int i=0;i<K;i++){
        if(skill[person][i] != 0) {
            notZero++;
            break;
        }
    }
    const int yakiR = 1500;
    vector<int> bestSkill = skill[person];
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
        WorkerNormMedian.erase(norm);
        skill[person] = bestSkill;
        norm = calcL1norm(skill[person]);
        WorkerNormMedian.insert(norm);
        WorkerNorm[person] = norm;
        cout << "#s " << person + 1 << " " << skill[person] << endl;
    }
    workerQue.push({norm, person});
}

void assignTask(){

    vector<int> ans;
    int sz = 0;
    auto addAns = [](vector<int> &ans, int &sz, int &person, int &task) {
        working[person] = {task, day, estimateDay(person, task)};
        sz++;
        ans.emplace_back(person+1);
        ans.emplace_back(task+1);
    };

    while(workerQue.size()){
        auto [norm, person] = workerQue.top();
        if(norm >= WorkerNormMedian.get()) {
            if(!taskQue[LARGE].empty()) {
                auto [weight, task] = taskQue[LARGE].top(); taskQue[LARGE].pop();
                addAns(ans, sz, person, task);
            }
            else if(!taskQue[SMALL].empty()){
                auto [weight, task] = taskQue[SMALL].top(); taskQue[SMALL].pop();
                addAns(ans, sz, person, task);
            }
            else if(!freeTaskQue.empty()){
                auto [weight, task] = freeTaskQue.top(); freeTaskQue.pop();
                addAns(ans, sz, person, task);
            }
            else break;
        }
        else{
            if(!taskQue[SMALL].empty()) {
                auto [weight, task] = taskQue[SMALL].top(); taskQue[SMALL].pop();
                addAns(ans, sz, person, task);
            }
            else if(!taskQue[LARGE].empty() && notReleased > 20) {
                if(R > 2000 && day > 100) {
                    int t = randint() % 2;
                    if(t == 0) break;
                }
                auto [weight, task] = taskQue[LARGE].top(); taskQue[LARGE].pop();
                addAns(ans, sz, person, task);
            }
            else if(!freeTaskQue.empty()){
                auto [weight, task] = freeTaskQue.top(); freeTaskQue.pop();
                addAns(ans, sz, person, task);
            }
            else break;
        }
        workerQue.pop();
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
                    taskQue[taskWeight[x][0] + taskWeight[x][1] > taskLSThreshold].push(
                        {taskWeight[x][0] + taskWeight[x][1] * (1 - attenuate * (doneTaskCount <= doneTaskThreshold)), x}
                    );
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
        workerQue.push({sum, i});
        WorkerNormMedian.insert(sum);
    }
    int cnt[N] = {};
    queue<int> q;
    // そのタスクをするのに必要な残りタスクの数
    vector<int> initialTask;
    vector<int> allTask;
    for(int i=0;i<N;i++){
        rCnt[i] = rev[i].size();
        cnt[i] = v[i].size();
        if(cnt[i] == 0) q.push(i);

        // タスクの重み計算
        queue<int> q;
        bool used[N]={};
        q.push(i);
        while(q.size()){
            int p = q.front(); q.pop();
            taskWeight[i][0]++;
            for(auto x:v[p]){
                if(!used[x]){
                    used[x] = 1;
                    q.push(x);
                }
            }
        }
        taskWeight[i][0]--;
        taskWeight[i][1] = calcL1norm(d[i]);
        {
            taskWeight[i][0] *= R;
            taskWeight[i][1] *= (4000 - R);
        }
        allTask.emplace_back(taskWeight[i][0] + taskWeight[i][1]);
        if(rCnt[i] == 0){
            initialTask.emplace_back(-taskWeight[i][0] + taskWeight[i][1]);
        }
        // cerr << i << " " << taskWeight[i][0] << " " << taskWeight[i][1] << endl;
    }
    sort(all(initialTask));
    sort(all(allTask));
    int itcnt = initialTask.size();
    int th = initialTask[itcnt / 2];
    int th2 = allTask[itcnt / 2];
    int INF = 1e9 + 7;
    for(int i=0;i<N;i++) {
        if(rCnt[i] == 0 && cnt[i] == 0) {
            notReleased--;
            freeTaskQue.push({taskWeight[i][1], i});
        }
        else if(rCnt[i] == 0) {
            notReleased--;
            int sum = taskWeight[i][0] + taskWeight[i][1];
            if(-taskWeight[i][0] + taskWeight[i][1] < th){
                taskQue[sum > th2].push({INF + taskWeight[i][0] - taskWeight[i][1], i});
            }
            else{
                taskQue[sum > th2].push({taskWeight[i][0] + taskWeight[i][1], i});
            }
        }
    }
    taskLSThreshold = th2;
}

signed main(){
    ios::sync_with_stdio(false);
    cin.tie(0);

    Timer time;

    cin>>K>>K>>K>>R;
    // if(R >= 2000) doneTaskThreshold = 0;
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