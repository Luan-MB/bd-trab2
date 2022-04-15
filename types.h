// Luan Machado Bernardt | GRR20190363

#ifndef __TYPES__
#define __TYPES__

#include <set>
#include <vector>
using namespace std;

typedef struct operation_t {

    int o_timestamp;
    int o_id;
    char o_op;
    char o_attr;

} operation_t;

typedef struct schedule_t {

    set<int> s_tids;
    vector<operation_t> s_transactions;

} t_schedule;

typedef pair<int, int> t_constraint;

#endif