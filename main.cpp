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
        n = 0, median=0;
        for(int i=0;i<4;i++) insert(-1e9);
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
const int INF = 1e9 + 7;

////////////// Task //////////
int taskLSThreshold, taskLSThresholdf;
priority_queue<array<int,2>> taskQueLARGE; // [LARGE/SMALL]{重要度, task index}
priority_queue<array<int,2>,vector<array<int,2>>,greater<array<int,2>>> taskQueSMALL; // [LARGE/SMALL]{重要度, task index}
vector<int> rCnt(N); // 依存のカウント(自分より下の個数)
vector<array<int,2>> taskWeight(N); // タスクの重み, {子孫の数, L2ノルム}
vector<vector<array<int,2>>> doneTask(M); // doneTask[person] = vector<{taskIdx, かかった日数}>
int doneTaskCount = 0, doneTaskThreshold = 900, attenuate=0.7; // 終わったタスクの数
int notReleased = N; // まだ開始することができない残りの仕事の数
int remainTask = N;
deque<int> freetask;
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
    if(gap > 10){
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
    int iter=2000;
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

void assignTask(Timer &time){

    vector<int> ans;
    int sz = 0;
    auto addAns = [](vector<int> &ans, int &sz, int &person, int &task) {
        working[person] = {task, day, estimateDay(person, task)};
        sz++;
        ans.emplace_back(person+1);
        ans.emplace_back(task+1);
    };

    bool ok = 1;
    if(!taskQueLARGE.empty()){
        auto [weight, task] = taskQueLARGE.top();
        if(weight > 0) ok = false;
    }
    if(!taskQueSMALL.empty()) {
        auto [weight, task] = taskQueSMALL.top();
        if(weight < 1e8) ok = false;
    }
    if(ok) {
        while(taskQueSMALL.size()) {
            auto [weight, task] = taskQueSMALL.top(); taskQueSMALL.pop();
            taskQueLARGE.push({-weight, task});
        }
        while(taskQueLARGE.size()) {
            auto [weight, task] = taskQueLARGE.top(); taskQueLARGE.pop();
            freetask.push_back(task);
        }
    }

    while(workerQue.size()){
        auto [norm, person] = workerQue.top();
        if(norm >= WorkerNormMedian.get()) {
            if(!taskQueLARGE.empty()) {
                auto [weight, task] = taskQueLARGE.top(); taskQueLARGE.pop();
                addAns(ans, sz, person, task);
            }
            else if(!taskQueSMALL.empty()){
                auto [weight, task] = taskQueSMALL.top(); taskQueSMALL.pop();
                addAns(ans, sz, person, task);
            }
            else if(freetask.size()) {
                int task = freetask.front(); freetask.pop_front();
                addAns(ans, sz, person, task);
            }
            else break;
        }
        else{
            if(day > 10 and day < 60) {
                int t = randint() % 2;
                if(t && !taskQueLARGE.empty()) {
                    auto [weight, task] = taskQueLARGE.top(); taskQueLARGE.pop();
                    addAns(ans, sz, person, task);
                    workerQue.pop();
                    continue;
                }
            }
            if(!taskQueSMALL.empty()) {
                auto [weight, task] = taskQueSMALL.top(); taskQueSMALL.pop();
                addAns(ans, sz, person, task);
            }
            else if(!taskQueLARGE.empty()) {
                if(R > 2000 && day > 100) {
                    int t = randint() % 2;
                    if(t == 0) break;
                }
                // if(R <= 2000 && day > 100) {
                //     int t = randint() % 3;
                //     if(t == 0) break;
                // }
                auto [weight, task] = taskQueLARGE.top();
                if(remainTask < 15) {
                    int est = day + estimateDay(person, task);
                    bool makasu = false;
                    for(int i=0;i<M;i++) {
                        if(WorkerNorm[i] <= WorkerNorm[person]) continue;
                        int nx = working[i][1] + working[i][2] + estimateDay(i, task);
                        if(nx < est) {
                            makasu = true;
                            break;
                        }
                    }
                    if(makasu) break;
                }
                addAns(ans, sz, person, task);
                taskQueLARGE.pop();
            }
            else if(freetask.size()) {
                int task = freetask.back(); freetask.pop_back();
                addAns(ans, sz, person, task);
            }
            else break;
        }
        workerQue.pop();
    }
    remainTask -= sz;

    // if(sz <= 1) {
    //     cout << sz << " ";
    //     cout << ans << endl;
    //     return;
    // }
    // if(R < 2000) {
    //     int loss = 0;
    //     for(int i=0;i<sz;i++) {
    //         loss += estimateDay(ans[i*2]-1, ans[i*2+1]-1);
    //     }
    //     vector<int> tasks(sz);
    //     for(int i=0;i<sz;i++) tasks[i] = ans[i*2 + 1];
    //     vector<int> best = tasks;
    //     const int yakiR = 1500;
    //     int iter=1000;
    //     while(iter--){
    //         int x = randint() % sz;
    //         int y = randint() % sz;
    //         while(y == x) y = randint() % sz;
    //         swap(tasks[x], tasks[y]);
    //         int lossNext = 0;
    //         for(int i=0;i<sz;i++) {
    //             lossNext += estimateDay(ans[i*2]-1, tasks[i]-1);
    //         }
    //         if(lossNext < loss){
    //             loss = lossNext;
    //             best = tasks;
    //             continue;
    //         }
    //         else if(yakiR * (2000 - time.elapsed()) > 2000*(randint()%yakiR)){
    //             // force Next
    //             continue;
    //         }
    //         else {
    //             swap(tasks[x], tasks[y]);
    //         }
    //     }
    //     for(int i=0;i<sz;i++){
    //         ans[i * 2 + 1] = best[i];
    //         working[ans[i * 2] - 1] = {best[i] - 1, day, estimateDay(ans[i * 2] - 1, best[i] - 1)};
    //     }
    // }
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
                    bool la = taskWeight[x][1] > taskLSThresholdf;
                    if(la) taskQueLARGE.push({taskWeight[x][1] - INF, x});
                    else taskQueSMALL.push({-taskWeight[x][1] + INF, x});
                }
                else{
                    bool la = taskWeight[x][0] + taskWeight[x][1] > taskLSThreshold;
                    if(la) taskQueLARGE.push({taskWeight[x][0] + taskWeight[x][1] * (1 - attenuate * (doneTaskCount <= doneTaskThreshold)), x});
                    else taskQueSMALL.push({-taskWeight[x][0] + taskWeight[x][1] * (1 - attenuate * (doneTaskCount <= doneTaskThreshold)), x});
                }
            }
        }
        working[person] = {-1, -1, -1};
    }
    return false;
}

void solve(Timer &time){
    while(true){
        assignTask(time);
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
    vector<int> nfTask;
    vector<int> fTask;
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
        if(cnt[i] != 0) nfTask.emplace_back(taskWeight[i][0] + taskWeight[i][1]);
        if(cnt[i] == 0) fTask.emplace_back(taskWeight[i][0] + taskWeight[i][1]);
        if(rCnt[i] == 0){
            initialTask.emplace_back(-taskWeight[i][0] + taskWeight[i][1] * 2);
        }
        // cerr << i << " " << taskWeight[i][0] << " " << taskWeight[i][1] << endl;
    }

    sort(all(initialTask));
    sort(all(nfTask));
    sort(all(fTask));
    int itcnt = initialTask.size();
    int th = initialTask[min(30, itcnt / 3)];
    int th2 = nfTask[(int)nfTask.size() / 2];
    int th3 = fTask[(int)fTask.size() / 2];
    for(int i=0;i<N;i++) {
        if(rCnt[i] == 0 && cnt[i] == 0) {
            notReleased--;
            int sum = taskWeight[i][0] + taskWeight[i][1];
            bool la = sum > th3;
            if(la) taskQueLARGE.push({taskWeight[i][1] - INF, i});
            else taskQueSMALL.push({-taskWeight[i][1] + INF, i});
        }
        else if(rCnt[i] == 0) {
            notReleased--;
            int sum = taskWeight[i][0] + taskWeight[i][1];
            bool la = sum > th2;
            if(-taskWeight[i][0] + taskWeight[i][1] < th){
                if(la) taskQueLARGE.push({INF + taskWeight[i][0] - taskWeight[i][1], i});
                else taskQueSMALL.push({-INF - taskWeight[i][0] + taskWeight[i][1], i});
            }
            else{
                if(la) taskQueLARGE.push({taskWeight[i][0] + taskWeight[i][1], i});
                else taskQueSMALL.push({-taskWeight[i][0] + taskWeight[i][1], i});
            }
        }
    }
    taskLSThreshold = th2;
    taskLSThresholdf = th3;
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