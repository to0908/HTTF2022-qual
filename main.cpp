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

const int N = 1000, M = 20;
int K, R, Kdiv2;
int randMa;
vector<vector<int>> d(N), skill(M); // 問題のd, s
vector<vector<int>> v(N), rev(N); // 入力のDAGと逆DAG
priority_queue<array<int,2>> taskQue; // {依存カウント, task}
priority_queue<array<int,2>> workerQue; // {L2 norm of skill, person}
vector<int> rCnt(N); // 依存のカウント(自分より下の個数)
vector<array<int,2>> taskWeight(N); // タスクの重み, {子孫の数, L2ノルム}
vector<array<int,3>> working(M, {-1, -1, -1}); // {task, 開始したday, estimateDay}
vector<vector<array<int,2>>> doneTask(M); // doneTask[person] = vector<{taskIdx, かかった日数}>
int day = 0; // 現在の日数
int doneTaskCount = 0, doneTaskThreshold = 900, attenuate=0.7; // 終わったタスクの数

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
    // Ridge
    // for(int i=0;i<K;i++){
    //     loss += skill[person][i] * skill[person][i] * (int)doneTask[person].size() * 0.5;
    // }
    return loss;
}

int calcL2norm(vector<int> &v, bool isSqrt=true){
    int sum = 0;
    for(auto i:v) sum += i * i;
    if(isSqrt) sum = sqrt(sum);
    return sum;
}

void estimateSkill(int person, Timer &time){
    // 返ってきた情報から、スキルを推定(推定値を更新)する
    // 最尤推定とかやりたいね

    // 今のday, working[person]の情報から更新
    int past = day - working[person][1] + 1;
    doneTask[person].push_back({working[person][0], past});
    int gap = abs(past - working[person][2]);
    bool changed = false;
    if(gap > 15){
        changed = true;
        for(int i=0;i<K;i++) skill[person][i] = randint() % randMa;
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
    const int yakiR = 1500;
    vector<int> bestSkill = skill[person];
    int iter=1000;
    while(iter--){
        int p = randint() % K;
        int inc = randint() % 3;
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
    skill[person] = bestSkill;
    workerQue.push({calcL2norm(skill[person],false), person});
    if(changed) cout << "#s " << person + 1 << " " << skill[person] << endl;
}

void assignTask(){

    vector<int> ans;
    int sz = 0;
    while(taskQue.size()){
        if(workerQue.empty()) break;
        auto [norm, person] = workerQue.top(); workerQue.pop();
        auto [weight, task] = taskQue.top(); taskQue.pop();
        working[person] = {task, day, estimateDay(person, task)};
        ans.emplace_back(person+1);
        ans.emplace_back(task+1);
        sz++;
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
                taskQue.push({taskWeight[x][0] + taskWeight[x][1] * (1 - attenuate * (doneTaskCount <= doneTaskThreshold)), x});
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
        int sum = 0;
        for(int j=0;j<K;j++){
            skill[i][j] = max(1, (int)randint() % randMa);
            sum += skill[i][j] * skill[i][j];
        }
        workerQue.push({sum, i});
    }
    int cnt[N] = {};
    queue<int> q;
    // そのタスクをするのに必要な残りタスクの数
    vector<int> initialTask;
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
        taskWeight[i][1] = calcL2norm(d[i]);
        if(rCnt[i] == 0){
            initialTask.emplace_back(-taskWeight[i][0] + taskWeight[i][1]);
        }
        // cerr << i << " " << taskWeight[i][0] << " " << taskWeight[i][1] << endl;
    }
    sort(all(initialTask));
    int itcnt = initialTask.size();
    int th = initialTask[itcnt / 2];
    int INF = 1e9 + 7;
    for(int i=0;i<N;i++){
        if(rCnt[i] == 0){
            if(-taskWeight[i][0] + taskWeight[i][1] < th){
                taskQue.push({INF + taskWeight[i][0] - taskWeight[i][1], i});
            }
            else{
                taskQue.push({taskWeight[i][0] + taskWeight[i][1], i});
            }
        }
    }
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