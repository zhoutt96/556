//
// Created by zhoutt96 on 3/21/20.
//

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <string>

using namespace std;
int main(int argc, char** argv)
{
    // ./compare -f filename1 filename2
    string t,ans,ans2;
    int i;
    freopen(argv[2],"r",stdin);
    char c;
    while(scanf("%c",&c)!=EOF) ans+=c;
    fclose(stdin);
    freopen(argv[3],"r",stdin);
    while(scanf("%c",&c)!=EOF) ans2+=c;;
    fclose(stdin);
    if(ans.size()!=ans2.size()){cout<<"NO\n";return 0;}
    for(i=0;i<ans.size();i++)
        if(ans[i]!=ans2[i]){cout<<"NO\n";return 0;}
    cout<<"YES\n";
    return 0;
}
