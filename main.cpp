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
vector<vector<int>> d(N);
vector<vector<int>> v(N), rev(N);
vector<vector<int>> skill(N);
bool dayEnd(vector<int> &w, vector<int> &cnt, priority_queue<array<int,2>> &que){
    int n;
    cin>>n;
    if(n == -1) return true;
    for(int i=0;i<n;i++){
        int a; cin>>a; a--;
        int task = w[a];
        for(auto x:v[task]){
            cnt[x]--;
            if(cnt[x] == 0) {
                que.push({(int)v[x].size(), x});
            }
        }
        w[a] = -1;
    }
    return false;
}

void solve(){
    priority_queue<array<int,2>> que;
    vector<int> cnt(N);
    for(auto v1:v){
        for(auto i:v1){
            cnt[i]++;
        }
    }
    for(int i=0;i<N;i++){
        if(cnt[i] == 0){
            que.push({(int)v[i].size(), i});
        }
    }
    vector<int> working(M, -1);
    while(true){
        vector<int> ans;
        int sz = 0;
        for(int i=0;i<M;i++){
            if(que.empty()) break;
            if(working[i] != -1) continue;
            ans.emplace_back(i + 1);
            ans.emplace_back(que.top()[1] + 1);
            working[i] = que.top()[1];
            que.pop();
            sz++;
        }
        cout << sz << " ";
        cout << ans << endl;
        if(dayEnd(working, cnt, que)) return;
    }
}

signed main(){
    ios::sync_with_stdio(false);
    cin.tie(0);

    Timer time;

    cin>>K>>K>>K>>R;
    for(int i=0;i<N;i++){
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