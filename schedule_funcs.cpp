// Luan Machado Bernardt | GRR20190363

#include <set>
#include <vector>
#include <algorithm>
using namespace std;

#include "types.h"
#include "schedule_funcs.h"

// Function that finds a loop in an adjacency matrix
static bool find_loop (vector<vector<int>> &adj_matrix, set<int> &open_vertex, int i) {
    
    // save current vertex i in open vertexes set
    open_vertex.insert(i);

    // call find_loop recursively for every vertex i has an edge to
    for(int j = 0; j < adj_matrix[i].size(); ++j)
        if (adj_matrix[i][j] == 1) {
            if ((open_vertex.find(j) != open_vertex.end()) ||
                (find_loop(adj_matrix, open_vertex, j)) == 1)
                return 1;
        }
    
    // erase current vertex i from open set
    open_vertex.erase(i);
    return 0;
}

// Function that checks wether a schedule is serializable or not
bool is_serializable (schedule_t *schedule) {

    operation_t Ti, Tj;
    set<int>::iterator iter = schedule->s_tids.begin();
    int vertex_num = schedule->s_tids.size();

    // create an adjacency matrix filled with 0s
    vector<vector<int>> adj_matrix = vector<vector<int> >(vertex_num, vector<int>(vertex_num, 0));

    // for every operation in schedule
    for (int i = 0; i < schedule->s_transactions.size(); ++i) {

        Ti = schedule->s_transactions[i];

        // if Ti is a Read operation
        if (Ti.o_op == 'R') {

            for (int j = 0; j < i; ++j) {
                
                Tj = schedule->s_transactions[j];

                // if Tj is a Write operation
                // create an unidirectional edge Tj -> Ti
                if ((Tj.o_op == 'W') &&
                    (Tj.o_attr == Ti.o_attr) &&
                    (Tj.o_id != Ti.o_id))
                    adj_matrix[Tj.o_id - *iter][Ti.o_id - *iter] = 1;
            }
        }

        // if Ti is a Write operation
        else if (Ti.o_op == 'W' ) {

            for (int j = 0; j < i; ++j) {
                
                Tj = schedule->s_transactions[j];

                // if Tj is a Write or Read operation
                // create an unidirectional edge Tj -> Ti
                if (((Tj.o_op == 'W') || (Tj.o_op == 'R')) &&
                    (Tj.o_attr == Ti.o_attr) &&
                    (Tj.o_id != Ti.o_id))
                    adj_matrix[Tj.o_id - *iter][Ti.o_id - *iter] = 1;
            }

        }
        
    }

    set<int> open_vertex;

    // call find_loop for every vertex in graph
    for (int i = 0; i < vertex_num; ++i)
        if (find_loop(adj_matrix, open_vertex, i))
            return 0;

    return 1;
}

// compares if i is less than j
static bool less_comparison (int i, int j) {
    return (i < j);
}

// compares if i and j are equal
static bool equal_comparison (int i, int j) {
    return (i == j);
}

// Function that checks if the schedule has any equivalent view
bool equivalent_view (schedule_t *schedule) {

    vector<int> t_ids;
    
    set<t_constraint> constraints;
    set<t_constraint>::iterator iter;

    set<pair<char, int>> writes_on;
    set<char> checked_attrs;

    // for every operation in schedule starting from the last
    for (int i = (schedule->s_transactions.size() -1); i >= 0; --i) {
        
        // if current operation is a Write
        if (schedule->s_transactions[i].o_op == 'W') {
            // checks if the operation[i] is the last write of attribute
            if (checked_attrs.find(schedule->s_transactions[i].o_attr) == checked_attrs.end()) {
                checked_attrs.insert(schedule->s_transactions[i].o_attr);
                // for all the operations before current
                for (int k = 0; k < i; ++k) {
                    // if operation[k] is a write on the same attr
                    // and is from different transaction
                    if ((schedule->s_transactions[k].o_op == 'W') &&
                        (schedule->s_transactions[i].o_attr == schedule->s_transactions[k].o_attr) &&
                        (schedule->s_transactions[i].o_id != schedule->s_transactions[k].o_id)) {

                            // create a constraint <k, i>
                            t_constraint new_constraint = make_pair(schedule->s_transactions[k].o_id,
                                                                    schedule->s_transactions[i].o_id);
                            constraints.insert(new_constraint);
                        }
                }
            }
            // save the operation transaction id and the attribute it writes on
            writes_on.insert(make_pair(schedule->s_transactions[i].o_attr, schedule->s_transactions[i].o_id));
        }
        
        // if current operation is a Read
        else if (schedule->s_transactions[i].o_op == 'R') {
            
            bool found_write = false;
            
            // for every transaction before current operation
            for (int k = (i-1); k >= 0; --k) {
                // if operation[k] is a Write
                if (schedule->s_transactions[k].o_op == 'W') {
                    // if the write operation is on the same attribute
                    if (schedule->s_transactions[k].o_attr == schedule->s_transactions[i].o_attr) {
                        // but from same transaction
                        if (schedule->s_transactions[k].o_id == schedule->s_transactions[i].o_id) {
                            break;
                        }
                        // if from another transaction
                        else {
                            // create a constraint <k, i>
                            t_constraint new_constraint = make_pair(schedule->s_transactions[k].o_id,
                                                                    schedule->s_transactions[i].o_id);
                            found_write = true;
                            constraints.insert(new_constraint);
                            break;
                        }
                    }
                }
            }

            // if there is no write on attribute before read
            if (!found_write) {
                
                set<pair<char, int>>::iterator it;
                
                // for every pair in writes_on set
                for (it = writes_on.begin(); it != writes_on.end(); ++it) {
                    // if a transaction writes on attribute
                    if ((it->first == schedule->s_transactions[i].o_attr) &&
                        (it->second != schedule->s_transactions[i].o_id)) {
                        // create constraint <i, transaction id>
                        t_constraint new_constraint = make_pair(schedule->s_transactions[i].o_id,
                                                                it->second);
                        constraints.insert(new_constraint);
                    }
                }
            }
        }
        // save the current operation transaction id
        t_ids.push_back(schedule->s_transactions[i].o_id);
    } 

    // purge copies inside t_ids array
    unique(t_ids.begin(), t_ids.end(), equal_comparison);
    // sort the values inside t_ids array in ascendant order
    sort(t_ids.begin(), t_ids.end(), less_comparison);

    // for every permutation of t_ids
    do {

        bool equivalent_view = true;

        // for all contraints
        for (iter = constraints.begin(); iter != constraints.end(); ++iter) {
            // if constraint is not satisfied on current permutation
            if (find(t_ids.begin(), t_ids.end(), iter->first) > find(t_ids.begin(), t_ids.end(), iter->second)) {
                equivalent_view = false;
                break;
            }
        }

        // if all constraints were satisfied
        if (equivalent_view)
            return 1;

    // get next permutation of t_ids
    } while (next_permutation(t_ids.begin(), t_ids.end()));

    return 0;
}