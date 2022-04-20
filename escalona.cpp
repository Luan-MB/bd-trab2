// Luan Machado Bernardt | GRR20190363

#include <iostream>
#include <string>
#include <set>
using namespace std;

#include "types.h"
#include "schedule_funcs.h"

#define MAX_SIZE 20

int main () {

    char transaction_input[MAX_SIZE];
    
    t_schedule schedule;
    int schedules_id = 1;
    
    set<int> open_transactions;
    
    int num, id;
    char op, attr;

    // while there is a line to read
    while (cin.getline(transaction_input, MAX_SIZE, '\n')) {

        // parse the arguments
        sscanf(transaction_input, "%d %d %c %c", &num, &id, &op, &attr);

        // insert operation id on open trasactions set
        open_transactions.insert(id);

        // if operation operation is a Commit
        if (op == 'C')
            // remove operation id from open transactions
            open_transactions.erase(id);
        else {

            operation_t new_t;

            new_t.o_timestamp = num;
            new_t.o_id = id;
            new_t.o_op = op;
            new_t.o_attr = attr;

            schedule.s_tids.insert(id);
            schedule.s_transactions.push_back(new_t);

        }
        
        // if there are no open transactions
        if (open_transactions.empty()) {
            cout << schedules_id++ << ' ';

            set<int>::iterator o_ids = schedule.s_tids.begin();

            // print the ids of the operations on the schedule
            for (int i = 0; i < (schedule.s_tids.size() - 1); ++i)
                cout << *o_ids++ << ',';
            cout << *o_ids; 

            // check wether schedule is serializable or not
            if (is_serializable(&schedule))
                cout << " SS";
            else
                cout << " NS";
            
            // same for equivalent view
            if (equivalent_view(&schedule))
                cout << " SV" << endl;
            else    
                cout << " NV" << endl;

            // clear the schedule arrays and sets
            schedule.s_tids.clear();
            schedule.s_transactions.clear();
        }
    }

    return 0;
}