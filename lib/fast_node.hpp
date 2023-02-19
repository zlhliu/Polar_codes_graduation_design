#ifndef _FAST_NODE_
#define _FAST_NODE_

#include<bits/stdc++.h>

#define R0 0
#define R1 1
#define REP 2
#define SPC 3
#define NONE 4

typedef struct fast_node_state
{
    fast_node_state():state(4){}
    int state;
    int num;
}fast_node_state;

std::vector<std::vector<fast_node_state>> calculate_fast_node();


#endif