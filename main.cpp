// 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98
//  main.cpp
//  p3-281bank
//
//  Created by Pablo Rodriguez on 6/2/25.
//
#include "xcode_redirect.hpp"
#include <iostream>

#include <getopt.h>

#include <cassert>
#include <cstdio>

#include <fstream>
#include <string>

#include <unordered_map>
#include <unordered_set>

#include <queue>
#include <vector>

#include <algorithm>

#include <sstream>

using namespace std;

struct Options {
    bool verbose = false;
    bool help = false;
    bool file = false;
    string filename;
};

struct Bankaccount{
    string name, pin;
    uint64_t timestamp, balance;
 
    unordered_set<string> ipadress;
    bool login = false;
};
uint32_t next_id = 0;
struct Transaction{
    
    uint64_t timestamp;
    string ip;
    string sender;
    string recipient;
    uint64_t amount;
    uint64_t exec_date;
    string transfee;
    uint64_t unique_id;
    uint64_t fee;
    
};

struct TransactionCompare {
    bool operator()(const Transaction& a, const Transaction& b) const {
        if (a.exec_date != b.exec_date)
            return a.exec_date > b.exec_date;  // min-heap by exec_date
        return a.unique_id > b.unique_id;      // tiebreaker by unique_id
    }
};


void printHelp() {
cout << "Options:\n"
         << "  -h, --help         Show this help message\n"
         << "  -v, --verbose      This causes the program to print certain log messages\n"
         << "  -f, --file         This is followed by a filename for the registration file\n";
       
}


void getOptions(int argc, char **argv, Options &options) {
    // These are used with getopt_long()
    opterr = static_cast<int>(false);  // Let us handle all error output for command line options
    int choice;
    int index = 0;

    // NOLINTBEGIN: getopt is old C code predating C++ style, this usage is from `man getopt`
    option longOptions[] = {
  // Insert two lines for the "mode" ('m') and the "help" ('h') options.
        {"verbose", no_argument, nullptr, 'v' },
        {"help", no_argument, nullptr, 'h' },
        {"file", required_argument, nullptr, 'f' },
        {nullptr, 0, nullptr, '\0'},
    };  // longOptions[]
    // NOLINTEND

    // Fill in the double quotes, to match the mode and help options.
    while ((choice = getopt_long(argc, argv, "vhf:", static_cast<option *>(longOptions), &index)) != -1) {
        switch (choice) {
            case 'v':
                options.verbose = true;
                break;
            case 'h':
                options.help = true;
                exit(0);
                break;
        
            case 'f':
                options.file = true;
                options.filename = string(optarg);
                if(options.filename.empty()){
                    cerr << "No filename after file \n";
                    exit(1);
                }
                break;
            

           
        default:
            cerr << "Unknown command line option" << '\n';
            exit(1);

        }  // switch ..choice
    }  // while
    
    if(options.file == false){
            cerr << "filename not specified \n";
        exit(1);
        }
    
}

uint64_t fixtime(const string &timestamp){;
        string digits;

        for (char ch : timestamp) {
            if (isdigit(ch)) {
                digits += ch;  // append digit to new string
            }
        }

    uint64_t value = stoull(digits);
    return value;
}

bool valid_transaction(uint64_t timestamp, string &ip, string &sender,  string &recipient,
                         uint64_t exec_date, unordered_map<string, Bankaccount> &myMap, Options &options){
    
    if(sender == recipient){
        //Self transactions are not allowed.
        if(options.verbose == true){
            cout << "Self transactions are not allowed." << "\n";
        }
        return false;
    }
    
    if(exec_date - timestamp > 3000000){
        //Select a time up to three days in the future.
        if(options.verbose == true){
            cout << "Select a time up to three days in the future." << "\n";
        }
        return false;
    }
    
    
    if(myMap.find(sender) == myMap.end()){
        //Sender <SENDER> does not exist.
        if(options.verbose == true){
            cout << "Sender " << sender << " does not exist." << "\n";
        }
        return false;
    }
    if(myMap.find(recipient) == myMap.end()){
        //Recipient <RECIPIENT> does not exist.
        if(options.verbose == true){
            cout << "Recipient " << recipient << " does not exist." << "\n";
        }
        return false;
    }
    if(myMap[sender].timestamp > exec_date){
        //At the time of execution, sender and/or recipient have not registered.
        if(options.verbose){
            cout << "At the time of execution, sender and/or recipient have not registered."
            << "\n";
        }
        return false;
    }
    if(myMap[recipient].timestamp > exec_date){
        //At the time of execution, sender and/or recipient have not registered.
        if(options.verbose){
            cout << "At the time of execution, sender and/or recipient have not registered."
            << "\n";
        }
        return false;
    }
    if(myMap[sender].ipadress.empty()){
        //Sender <SENDER> is not logged in.
        if(options.verbose){
            cout << "Sender " << sender << " is not logged in."
            << "\n";
        }
        return false;
    }
    if(myMap[sender].ipadress.find(ip) == myMap[sender].ipadress.end()){
        //Fraudulent transaction detected, aborting request.
        if(options.verbose){
            cout << "Fraudulent transaction detected, aborting request." << "\n";
        }
        return false;
    }
    
    
    

    return true;
}

void transaction(priority_queue<Transaction, vector<Transaction>, TransactionCompare> &pq,  unordered_map<string, Bankaccount> &myMap, vector<Transaction> &trans_history, Options &options){
    
    //check if they both can afford the fees

    uint64_t fee = 0;
    uint64_t fee_s = 0;
    uint64_t fee_r = 0;
    fee = pq.top().amount;
    fee = fee / 100;
    
    if(fee  > 450){
        fee = 450;
    }//max
    if(fee < 10){
        fee = 10;
    }//min
    

    uint64_t fivemil =  50000000000;
    if(pq.top().exec_date - myMap[pq.top().sender].timestamp > fivemil){
        fee = (fee * 3) / 4;
    }//if older than 5 years apply discount
    
    if(pq.top().transfee == "s"){
        if(fee % 2 != 0){
            fee = fee - 1;
            fee_s = (fee / 2) + 1;//for sender
            fee_r = (fee / 2);
            fee++;//set back to normal
        }
        else{
            fee_s = fee / 2;
            fee_r = fee / 2;
        }
        
    }//split fee
    if(pq.top().transfee == "s"){
        if(fee_s > myMap[pq.top().sender].balance){
            if(options.verbose){
                //Insufficient funds to process transaction <transaction_id>.
                cout << "Insufficient funds to process transaction "
                << pq.top().unique_id << "." << "\n";
            }
            return;
        }
        if(fee_r > myMap[pq.top().recipient].balance){
            if(options.verbose){
                //Insufficient funds to process transaction <transaction_id>.
                cout << "Insufficient funds to process transaction "
                << pq.top().unique_id << "." << "\n";
            }
            return;
        }
    }//fee is bigger than balance
    
    if(pq.top().transfee == "o"){
        if(fee > myMap[pq.top().sender].balance){
            if(options.verbose){
                //Insufficient funds to process transaction <transaction_id>.
                cout << "Insufficient funds to process transaction "
                << pq.top().unique_id << "." << "\n";
            }
            return;
        }
    }//sender can't afford the fee
    if(pq.top().transfee == "o"){
        myMap[pq.top().sender].balance -= fee;
        if(myMap[pq.top().sender].balance < pq.top().amount){
            myMap[pq.top().sender].balance += fee;//give the fee back
            if(options.verbose){
                //Insufficient funds to process transaction <transaction_id>.
                cout << "Insufficient funds to process transaction "
                << pq.top().unique_id << "." << "\n";
            }
            return;//not enough money
            
        }
        
        myMap[pq.top().sender].balance = myMap[pq.top().sender].balance
        - pq.top().amount;
        
        myMap[pq.top().recipient].balance = myMap[pq.top().recipient].balance
        + pq.top().amount;
    }
    
    else{
        myMap[pq.top().sender].balance -= fee_s;
        
        if(myMap[pq.top().sender].balance < pq.top().amount){
            myMap[pq.top().sender].balance += fee_s;
            if(options.verbose){
                //Insufficient funds to process transaction <transaction_id>.
                cout << "Insufficient funds to process transaction "
                << pq.top().unique_id << "." << "\n";
            }
            return;
            
        }
        
        myMap[pq.top().sender].balance = myMap[pq.top().sender].balance
        - pq.top().amount;
        
        myMap[pq.top().recipient].balance -= fee_r;
        
        myMap[pq.top().recipient].balance = myMap[pq.top().recipient].balance
        + pq.top().amount;
        
        
        
    }
    trans_history.push_back(pq.top());
    trans_history.back().fee = fee;
    
    //Transaction <ID> executed at <EXEC_DATE>: $<AMOUNT> from <SENDER> to <RECIPIENT>.
    if(options.verbose){
        cout << "Transaction " << pq.top().unique_id << " executed at "
        << pq.top().exec_date << ": $" << pq.top().amount << " from "
        << pq.top().sender << " to " << pq.top().recipient << "." << "\n";
    }
}




string paddedTimestamp(uint64_t ts) {
    ostringstream oss;
    oss.width(12);
    oss.fill('0');
    oss << ts;
    return oss.str();
}



string formatTimeDifference(const string& s) {
    // s must be 12 digits (yy mm dd hh mm ss)
    int yy = stoi(s.substr(0, 2));
    int mo = stoi(s.substr(2, 2));
    int dd = stoi(s.substr(4, 2));
    int hh = stoi(s.substr(6, 2));
    int mi = stoi(s.substr(8, 2));
    int ss = stoi(s.substr(10, 2));

    ostringstream out;
    bool printed = false;

    auto append = [&](int val, const string& label) {
        if (val > 0) {
            if (printed) out << " ";
            out << val << " " << label;
            if (val != 1) out << "s";
            printed = true;
        }
    };

    append(yy, "year");
    append(mo, "month");
    append(dd, "day");
    append(hh, "hour");
    append(mi, "minute");
    append(ss, "second");

    if (!printed) out << "0 seconds";
    return out.str();
}





bool validColonCount(const std::string &ts) {
    // A well-formed timestamp has 5 colons separating 6 fields
    return std::count(ts.begin(), ts.end(), ':') == 5;
}



int main(int argc, char *argv[]) {
    ios_base::sync_with_stdio(false);
    xcode_redirect(argc, argv);
    Options options;
    getOptions(argc, argv, options);
    unordered_map<string, Bankaccount> myMap;
    
    priority_queue<Transaction, vector<Transaction>, TransactionCompare> transactionPQ;
    
    vector<Transaction> trans_history;
    
    
    
    
    
    
    
    
    ifstream infile(options.filename);
        if (!infile) {
            cerr << "Error: Could not open file.\n";
            return 1;
        }

        string timestamp, name, pin, balance;
        string line;

        while (getline(infile, line)) {
            istringstream iss(line);
            if (getline(iss, timestamp, '|') &&
                getline(iss, name, '|') &&
                getline(iss, pin, '|') &&
                getline(iss, balance)) {
                // Now mac, name, id, and balance are filled
               // cout << "Timestamp: " << timestamp
               //           << ", Name: " << name
               //           << ", Pin: " << pin
               //           << ", Balance: " << balance << '\n';
                Bankaccount bank;
                
                bank.timestamp = fixtime(timestamp);
                bank.name = name;
                bank.pin = pin;
                bank.balance = stoull(balance);
                myMap[bank.name] = bank;
                
                
            } else {
                cerr << "Error: Malformed line -> " << line << '\n';
            }
        }//This is for spec-reg
    
    string temp;
    
    string ip;
    string sender;
    string recipient;
    uint64_t amount;
    string exec_date;
    string transfee;
    
    string prevtimestamp;
    
    timestamp.clear();
   
    while(cin >> temp){
        if(!temp.empty() && temp[0] == '#'){
            getline(cin, temp);
            continue;
        }//this skips if it starts with #
        
        if(!temp.empty() && temp[0] == '$'){
            break;
        }//stops while loop if '$' is found
        
        if(!temp.empty() && temp == "login"){
            cin >> name;
            cin >> pin;
            cin >> ip;
            
            if(myMap.find(name) != myMap.end()){
                if(myMap[name].pin == pin && myMap[name].name == name){
                    myMap[name].ipadress.insert(ip);
                    myMap[name].login = true;
                    if(options.verbose == true){
                        //User <USER_ID> logged in.
                        cout << "User " << name << " logged in." << "\n";
                        continue;
                    }
                    continue;
                }
            }
             if(options.verbose){
                //Login failed for <USER_ID>.
                cout << "Login failed for " << name << "." << "\n";
            }
            continue;
        }//end for login
        if(!temp.empty() && temp == "out"){
            cin >> name;
            cin >> ip;
            
            
            
            auto it = myMap[name].ipadress.find(ip);
            if (it != myMap[name].ipadress.end()) {
                myMap[name].ipadress.erase(ip);
                myMap[name].login = false;
                if(options.verbose == true){
                    //User <USER_ID> logged out.
                    cout << "User " << name << " logged out." << "\n";
                }
            }
            else if(options.verbose == true){
                //Logout failed for <USER_ID>.
                cout << "Logout failed for " << name << "." << "\n";
            }
            
            continue;
        }
        
        if(!temp.empty() && temp == "place"){
            cin >> timestamp;
            cin >> ip;
            cin >> sender;
            cin >> recipient;
            cin >> amount;
            cin >> exec_date;
            cin >> transfee;
            
            //A place command with a timestamp earlier than the previous place.
            //A place command which contains an execution date before its own timestamp.
            //In all of these cases, print an informative error message to standard error (cerr) and call exit(1).
            if(!prevtimestamp.empty()){
                if (fixtime(timestamp) < fixtime(prevtimestamp)){
                    cerr << "Error timestamp less then prev" << "\n";
                    exit(1);
                }
            }
            
            if(fixtime(timestamp) > fixtime(exec_date)){
                cerr << "Error invalid exec and timestamp" << "\n";
                exit(1);
            }
            
            prevtimestamp = timestamp;
            
            if(!valid_transaction(fixtime(timestamp), ip, sender, recipient,
                                  fixtime(exec_date), myMap, options)){
                continue;
            }
            
            
            
            if(!transactionPQ.empty()){
                while(!transactionPQ.empty() && fixtime(timestamp) >= transactionPQ.top().exec_date){
                    transaction(transactionPQ, myMap, trans_history, options);
                    transactionPQ.pop();
                }
            }
            
            //last two need to go through
           
                
                Transaction t = {fixtime(timestamp), ip, sender, recipient, amount,
                fixtime(exec_date), transfee, next_id++, 0};
                transactionPQ.push(t);
                if(options.verbose){
                    //Transaction <ID> placed at <TIMESTAMP>: $<AMOUNT> from <SENDER> to <RECIPIENT> at <EXEC_DATE>.
                    cout << "Transaction " << t.unique_id << " placed at " <<
                    t.timestamp << ": $" << t.amount << " from " << t.sender <<
                    " to " << t.recipient << " at " << t.exec_date << "." << "\n";
                }
                
            
            
            

            
            
            continue;
        }
        
        if(!temp.empty() && temp == "balance"){
            cin >> name;
            cin >> ip;
            
            //i need to check if name and ip are valid
            
            if (myMap.find(name) == myMap.end()){
                //User <USER_ID> does not exist.
                if(options.verbose == true){
                    cout << "User " << name << " does not exist." << "\n";
                }
                continue; // FIXED from break
            }
            
            if(myMap[name].ipadress.empty()){
                if(options.verbose == true){
                    //User <USER_ID> is not logged in.
                    cout << "User " << name << " is not logged in." << "\n";
                }
                continue; // FIXED from break
            }
            
            if(myMap[name].ipadress.find(ip) == myMap[name].ipadress.end()){
                if(options.verbose == true){
                  //Fraudulent balance check detected, aborting request.
                    cout << "Fraudulent balance check detected, aborting request."
                    << "\n";
                }
                continue; // FIXED from break
        }
            //As of <TIMESTAMP>, <USER_ID> has a balance of $<BALANCE>.
            cout << "As of ";
            if (timestamp.empty()){
                cout << myMap[name].timestamp;
            }
            else{
                cout << fixtime(timestamp);
            }
            cout << ", " << name << " has a balance of $";
            cout << myMap[name].balance << "." << "\n";
        
            continue;
    }
        
        
    }
    
    if(!transactionPQ.empty()){
        while(!transactionPQ.empty()){
            transaction(transactionPQ, myMap, trans_history, options);
            transactionPQ.pop();
        }
    }//gotta do all of the place commands
    
    string timestart;
    string timeend;
    while(cin >> temp){
        
        if(temp == "l"){
            cin >> timestart;
            cin >> timeend;
            int transnumber = 0;
            
            if(!validColonCount(timestart)){
                continue;
            }
            if(!validColonCount(timeend)){
                continue;
            }
            
            if(timeend < timestart){
                continue;
            }//y needs to be >= x
            
            if(fixtime(timestart) == fixtime(timeend)){
                cout << "List Transactions requires a non-empty time interval."
                << "\n";
                continue;
            }
            
            
            for(size_t i = 0; i < trans_history.size(); i++){
                if(trans_history[i].exec_date < fixtime(timestart)){
                    continue;
                }//if less then skip
                if(trans_history[i].exec_date >= fixtime(timeend)){
                    break;
                }//if greater then or equal too end loop
                
                // <Transaction ID>: <sender> sent <amount> dollars to <recipient> at <execution_date>.
                cout << trans_history[i].unique_id << ": "
                << trans_history[i].sender << " sent "
                << trans_history[i].amount;
                if(trans_history[i].amount == 1){
                    cout << " dollar to ";
                }
                if(trans_history[i].amount > 1){
                    cout << " dollars to ";
                }
                cout << trans_history[i].recipient << " at "
                << trans_history[i].exec_date << "." << "\n";
                transnumber++;
                
            }
            //There were <total number> transactions that were executed between time x to y.
            if(transnumber != 1){
                cout << "There were " << transnumber
                << " transactions that were executed between time " <<
                fixtime(timestart) << " to " << fixtime(timeend) << "." << "\n";
            }
            //There was 1 transaction that was executed between time x to y.
            else if(transnumber == 1){
                cout << "There was 1 transaction that was executed between time "
                << fixtime(timestart) << " to " << fixtime(timeend) << "." << "\n";
            }
            continue;
        }
        
        if(temp == "r"){
            uint64_t fee = 0;
            cin >> timestart;
            cin >> timeend;
            
            if(!validColonCount(timestart)){
                continue;
            }
            if(!validColonCount(timeend)){
                continue;
            }
            
            if(timeend < timestart){
                continue;
            }//y needs to be >= x
            
            
            
            
            if(fixtime(timestart) == fixtime(timeend)){
                cout << "Bank Revenue requires a non-empty time interval."
                << "\n";
                continue;
            }
            for(size_t i = 0; i < trans_history.size(); i++){
                if(trans_history[i].exec_date < fixtime(timestart)){
                    continue;
                }//if less then skip
                if(trans_history[i].exec_date >= fixtime(timeend)){
                    break;
                }//if greater then or equal too end loop
                
                fee = fee + trans_history[i].fee;
            }
            
            //281Bank has collected <amount> dollars in fees over <time interval formatted(y-x)>.
            uint64_t diff = fixtime(timeend) - fixtime(timestart);
            cout << "281Bank has collected " << fee <<  " dollars in fees over "
                 << formatTimeDifference(paddedTimestamp(diff)) << ".\n";
            continue;
        
        }
        
        if(temp == "h"){
            cin >> name;
            if(myMap.find(name) == myMap.end()){
                cout << "User " << name << " does not exist." << "\n";
                continue;
            }
            cout << "Customer " << name << " account summary:" << "\n";
            cout << "Balance: $" << myMap[name].balance << "\n";
            queue<Transaction> sent;
            queue<Transaction> recived;
            
            
            for(size_t i = 0; i < trans_history.size(); i++){
                if(trans_history[i].sender == name){
                    sent.push(trans_history[i]);
                }
                else if(trans_history[i].recipient == name){
                        recived.push(trans_history[i]);
                }
               
            }
            cout << "Total # of transactions: " << sent.size() + recived.size() << "\n";
            //Incoming 2:
            //2: hjgarcia sent 27 dollars to paoletti at 30001.
            cout << "Incoming " << recived.size() << ":" << "\n";
            
            if(recived.size() > 10){
                size_t popcount = recived.size() - 10;
                while(popcount != 0){
                    recived.pop();
                    popcount--;
                }
            }//the latest 10 if size bigger than 10
            
            while(!recived.empty()){
                auto t = recived.front();
                cout << t.unique_id << ": " << t.sender << " sent " << t.amount
                            << " dollar" << (t.amount == 1 ? "" : "s")
                            << " to " << t.recipient << " at " << t.exec_date << ".\n";
                recived.pop();
                
            }
            cout << "Outgoing " << sent.size() << ":" << "\n";
            
            if(sent.size() > 10){
                size_t popcount = sent.size() - 10;
                while(popcount != 0){
                    sent.pop();
                    popcount--;
                }
            }//the latest 10 if size bigger than 10
            
            while(!sent.empty()){
                auto t = sent.front();
                cout << t.unique_id << ": " << t.sender << " sent " << t.amount
                            << " dollar" << (t.amount == 1 ? "" : "s")
                            << " to " << t.recipient << " at " << t.exec_date << ".\n";
                sent.pop();
            }
            
            
            
            continue;
        }
        
        if(temp == "s"){
            cin >> timestart;
            if(!validColonCount(timestart)){
                continue;
            }
            
            
            uint64_t start = fixtime(timestart) / 1000000 * 1000000;
            uint64_t end = start + 1000000;
            uint64_t fee = 0;
            int count = 0;
            //Summary of [112000000, 113000000):
            cout << "Summary of [" << start << ", "
            << end << "):" << "\n";
            
            for(size_t i = 0; i < trans_history.size(); i++){
                if(trans_history[i].exec_date < start){
                    continue;
                }//if less then skip
                if(trans_history[i].exec_date >= end){
                    break;
                }//if greater then or equal too end loop
                //2: hjgarcia sent 27 dollars to paoletti at 112030001.
                cout << trans_history[i].unique_id << ": " <<
                trans_history[i].sender << " sent " << trans_history[i].amount
                << " dollar" << (trans_history[i].amount == 1 ? "" : "s")
                << " to " << trans_history[i].recipient << " at " << trans_history[i].exec_date << ".\n";
                count++;
                fee = fee + trans_history[i].fee;
                
                
            }
            
            //there were a total of 4 transactions, 281Bank has collected 41 dollars in fees.
            if(count != 1){
                cout << "There were a total of " << count << " transaction";
            }
            if(count == 1){
                cout << "There was a total of " << count << " transaction";
            }
            
            cout << (count == 1 ? "" : "s") << ", 281Bank has collected " << fee
            << " dollars in fees." << "\n";
            
            
            
        }
        
        
    }


    return 0;
}
