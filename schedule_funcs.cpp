// Luan Machado Bernardt | GRR20190363

#include <set>
#include <vector>
#include <algorithm>
using namespace std;

#include "types.h"
#include "schedule_funcs.h"

static bool find_loop (vector<vector<int>> &adj_matrix, set<int> &open_vertex, int i) {
    
    open_vertex.insert(i);

    for(int j = 0; j < adj_matrix[i].size(); ++j)
        if (adj_matrix[i][j] == 1) {
            if (open_vertex.find(j) != open_vertex.end())
                return 1;
            if (find_loop(adj_matrix, open_vertex, j) == 1)
                return 1;
        }
    
    open_vertex.erase(i);
    return 0;

}

bool is_serial (schedule_t *schedule) {

    operation_t Ti, Tj;
    set<int>::iterator iter = schedule->s_tids.begin();
    int vertex_num = schedule->s_tids.size();

    vector<vector<int>> adj_matrix = vector<vector<int> >(vertex_num, vector<int>(vertex_num, 0));

    for (int i = 0; i < schedule->s_transactions.size(); ++i) {

        Ti = schedule->s_transactions[i];

        if (Ti.o_op == 'R') {

            for (int j = 0; j < i; ++j) {
                
                Tj = schedule->s_transactions[j];

                if ((Tj.o_op == 'W') &&
                    (Tj.o_attr == Ti.o_attr) &&
                    (Tj.o_id != Ti.o_id))
                    adj_matrix[Tj.o_id - *iter][Ti.o_id - *iter] = 1;
            }
        }

        else if (Ti.o_op == 'W' ) {

            for (int j = 0; j < i; ++j) {
                
                Tj = schedule->s_transactions[j];

                if (((Tj.o_op == 'W') || (Tj.o_op == 'R')) &&
                    (Tj.o_attr == Ti.o_attr) &&
                    (Tj.o_id != Ti.o_id))
                    adj_matrix[Tj.o_id - *iter][Ti.o_id - *iter] = 1;
            }

        }
        
    }

    set<int> open_vertex;

    for (int i = 0; i < vertex_num; ++i)
        if (find_loop(adj_matrix, open_vertex, i))
            return 0;

    return 1;
}

static bool less_comparison (int i, int j) {
    return (i < j);
}

static bool equal_comparison (int i, int j) {
    return (i == j);
}

bool equivalent_view (schedule_t *schedule) {

    vector<int> t_ids;
    
    set<t_constraint> constraints;
    set<t_constraint>::iterator iter;

    set<pair<char, int>> write_on;
    set<char> checked_attrs;

    for (int i = (schedule->s_transactions.size() -1); i >= 0; --i) {
        
        if (schedule->s_transactions[i].o_op == 'W') {
            if (checked_attrs.find(schedule->s_transactions[i].o_attr) == checked_attrs.end()) {
                checked_attrs.insert(schedule->s_transactions[i].o_attr);
                for (int k = 0; k < i; ++k) {
                    if ((schedule->s_transactions[k].o_op == 'W') &&
                        (schedule->s_transactions[i].o_attr == schedule->s_transactions[k].o_attr) &&
                        (schedule->s_transactions[i].o_id != schedule->s_transactions[k].o_id)) {

                            t_constraint new_constraint = make_pair(schedule->s_transactions[k].o_id,
                                                                    schedule->s_transactions[i].o_id);
                            constraints.insert(new_constraint);
                        }
                }
            }
            write_on.insert(make_pair(schedule->s_transactions[i].o_attr, schedule->s_transactions[i].o_id));
        }
        
        else if (schedule->s_transactions[i].o_op == 'R') {
            
            bool found_write = false;
            
            for (int k = (i-1); k >= 0; --k) {
                if (schedule->s_transactions[k].o_op == 'W') {
                    if (schedule->s_transactions[k].o_attr == schedule->s_transactions[i].o_attr) {
                        if (schedule->s_transactions[k].o_id == schedule->s_transactions[i].o_id) {
                            break;
                        }
                        else {
                            t_constraint new_constraint = make_pair(schedule->s_transactions[k].o_id,
                                                                    schedule->s_transactions[i].o_id);
                            found_write = true;
                            constraints.insert(new_constraint);
                            break;
                        }
                    }
                }
            }

            if (!found_write) {
                
                set<pair<char, int>>::iterator it;
                
                for (it = write_on.begin(); it != write_on.end(); ++it) {
                    if ((it->first == schedule->s_transactions[i].o_attr) &&
                        (it->second != schedule->s_transactions[i].o_id)) {
                        t_constraint new_constraint = make_pair(schedule->s_transactions[i].o_id,
                                                                it->second);
                        constraints.insert(new_constraint);
                    }
                }
            }
        }
    
        t_ids.push_back(schedule->s_transactions[i].o_id);
    } 

    unique(t_ids.begin(), t_ids.end(), equal_comparison);
    sort(t_ids.begin(), t_ids.end(), less_comparison);

    do {

        bool equivalent_view = true;

        for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
            if (find(t_ids.begin(), t_ids.end(), iter->first) > find(t_ids.begin(), t_ids.end(), iter->second)) {
                equivalent_view = false;
                break;
            }
        }

        if (equivalent_view)
            return 1;

    } while (next_permutation(t_ids.begin(), t_ids.end()));

    return 0;

}