#include <iostream>
#include <vector>

using namespace std;

class TuringMachine {
    char OPERATOR_PRINT = "P";
    char OPERATOR_ERASE = "E";
    char OPERATOR_RIGHT = "R";
    char OPERATOR_LEFT = "L";

    vector<char> tape;
    long int currTapeIdx = 0L;

    vector<char> m_configurations;
    char curr_m_configuration;

    char rule_section_delim = "|";
    vector<string> rules;

    string getRule() {
    }

    void parseAndExecuteRule(string rule) {
    }
    
    void performExecutionCycle() {
        string currRule = this.getRule();
        this.parseAndExecuteRule(currRule);    
    }

    public:
        TuringMachine(vector<char>, vector<char>, vector<string>);
        void run(int maxIterations) {
            int numIterations = 0;
            while (numIterations < maxIterations) {
                performExecutionCycle();
                numIterations++;
            }
        }
}

TuringMachine::TuringMachine (vector<char> new_tape, 
                              vector<char> new_m_configs, 
                              vector<string> new_rules) {
    tape = new_tape;
    m_configurations = new_m_configs;
    rules = new_rules;
}

int main() {
}
