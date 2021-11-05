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

const int N = 1000, M = 20;
int K, R;
vector<vector<int>> d(N), skill(M); // 問題のd, s
vector<vector<int>> v(N), rev(N); // 入力のDAGと逆DAG
priority_queue<array<int,2>> taskQue; // {依存カウント, task}
vector<int> rCnt(N); // 依存のカウント(自分より下の個数)
vector<array<int,2>> working(M, {-1, -1}); // {task, day}
int day = 0; // 現在の日数

void estimateSkill(int person){
    // 返ってきた情報から、スキルを推定(推定値を更新)する
    // 最尤推定とかやりたいね

    // 今のday, working[person]の情報から更新
}

void assignTask(){
    // 現在のスキルの推定値から、最適なタスクの割り当てを行う
    // 将来的なものも考えるべき？(今空いているけど数日後に帰ってくるであろう完了タスクにより開放されるタスクに割り当てる方が良いなど)
    // とりあえず今は考えずに、そのターンで最も良いものを割り当てることにする。

    vector<int> ans;
    int sz = 0;
    for(int i=0;i<M;i++){
        if(taskQue.empty()) break;
        if(working[i][0] != -1) continue;
        ans.emplace_back(i + 1);
        ans.emplace_back(taskQue.top()[1] + 1);
        working[i] = {taskQue.top()[1], day};
        taskQue.pop();
        sz++;
    }
    cout << sz << " ";
    cout << ans << endl;
}

bool dayEnd(){
    int n; cin>>n;
    if(n == -1) return true;
    for(int i=0;i<n;i++){
        int a; cin>>a; a--;
        int task = working[a][0];
        estimateSkill(a);
        for(auto x:v[task]){
            rCnt[x]--;
            if(rCnt[x] == 0) {
                taskQue.push({(int)v[x].size(), x});
            }
        }
        working[a] = {-1, -1};
    }
    return false;
}

void solve(){
    for(auto v1:v){
        for(auto i:v1){
            rCnt[i]++;
        }
    }
    for(int i=0;i<N;i++){
        if(rCnt[i] == 0){
            taskQue.push({(int)v[i].size(), i});
        }
    }
    while(true){
        assignTask();
        if(dayEnd()) return;
        day++;
    }
}

signed main(){
    ios::sync_with_stdio(false);
    cin.tie(0);

    Timer time;

    cin>>K>>K>>K>>R;
    for(int i=0;i<M;i++){
        skill[i].resize(K);
        for(int j=0;j<K;j++){
            skill[i][j] = 0;
        }
    }
    for(int i=0;i<N;i++){
        d[i].resize(K);
        for(int j=0;j<K;j++) cin>>d[i][j];
    }
    for(int i=0;i<R;i++){
        int a,b;
        cin>>a>>b;
        a--,b--;
        v[a].emplace_back(b);
        rev[b].emplace_back(a);
    }

    solve();

    cerr << "[time] " << time.elapsed() << " ms"<< endl;
}