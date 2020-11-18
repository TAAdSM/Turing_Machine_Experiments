#include <string>
#include <vector>
#include <sstream>
#include <iostream>

static const char *const BLANK_TAPE_SYMBOL = "ε";
static const char *const FINAL_TAPE_DELIMITER = "| ";
static const char *const RULE_WILDCARD_SYMBOL = "ANY";

using namespace std;

vector<string> splitString(string toSplit, char delim) {
    stringstream sstream(toSplit);
    string item;
    vector<string> splitted;
    while(getline(sstream, item, delim)) {
        splitted.push_back(item);
    }
    return splitted;
}

class TuringMachine {
    const char *const ruleNotFoundString = "RULE_NOT_FOUND";
    const char OPERATOR_PRINT = 'P';
    const char OPERATOR_ERASE = 'E';
    const char OPERATOR_RIGHT = 'R';
    const char OPERATOR_LEFT = 'L';

    vector<string> tape;
    long int currTapeIdx = 0L;

    vector<string> m_configurations;
    string currMConfig;

    char rule_section_delim = '|';
    char operation_section_delim = ',';
    vector<string> rules;

    string getRule() {
        string rulePrefix =  currMConfig +
            rule_section_delim + tape[currTapeIdx];
        string wildcardRulePrefix = currMConfig + rule_section_delim + RULE_WILDCARD_SYMBOL;
        string exactMatchResult = "";
        string wildcardMatchResult = "";
        for (string rule: rules) {
            auto prefixResult = mismatch(rulePrefix.begin(), rulePrefix.end(), rule.begin());
            if (prefixResult.first == rulePrefix.end()) {
                exactMatchResult = rule;
                break;
            }
            auto prefixWildcardResult = mismatch(wildcardRulePrefix.begin(), wildcardRulePrefix.end(), rule.begin());
            if (prefixWildcardResult.first == wildcardRulePrefix.end()) {
                wildcardMatchResult = rule;
            }
        }
        return !exactMatchResult.empty() ? exactMatchResult :
            !wildcardMatchResult.empty() ? wildcardMatchResult : ruleNotFoundString;
    }

    void parseAndExecuteRule(string rule) {
        vector<string> ruleParts = splitString(rule, rule_section_delim);
        // string mConfig = ruleParts[0];
        // string symbol = ruleParts[1];
        vector<string> operations = splitString(ruleParts[2], operation_section_delim);
        string newMConfig = ruleParts[3];

        for (string operation : operations) {
            if (operation[0] == OPERATOR_PRINT) {
                tape[currTapeIdx] = operation[1];
            } else if (operation[0] == OPERATOR_ERASE) {
                tape[currTapeIdx] = BLANK_TAPE_SYMBOL;
            } else if (operation[0] == OPERATOR_RIGHT) {
                if (currTapeIdx < tape.size()) {
                    currTapeIdx++;
                }
            } else if (operation[0] == OPERATOR_LEFT) {
                if (currTapeIdx > 0) {
                    currTapeIdx--;
                }
            }
        }

        currMConfig = newMConfig;
//        cout << "mConfig updated to " + currMConfig + "\n";
    }
    
    void performExecutionCycle() {
        string currRule = getRule();
        if (currRule != ruleNotFoundString) {
            parseAndExecuteRule(currRule);
        }
    }

    public:
        TuringMachine(vector<string> new_tape,
                      vector<string> new_m_configs,
                      vector<string> new_rules) {
            tape = new_tape;
            m_configurations = new_m_configs;
            rules = new_rules;
        };
        void run(int maxIterations) {
            currMConfig = m_configurations[0];
            int numIterations = 0;
            while (numIterations < maxIterations) {
                performExecutionCycle();
                numIterations++;
            }
            ostringstream implodedTape;
            copy(tape.begin(), tape.end(), ostream_iterator<string>(implodedTape, FINAL_TAPE_DELIMITER));
            cout << implodedTape.str() + "\n";
        }
};

int main() {
    vector<string> simpleTMInitialTape(20);
    fill(simpleTMInitialTape.begin(), simpleTMInitialTape.end(), BLANK_TAPE_SYMBOL);
    TuringMachine simpleTM(simpleTMInitialTape,
                           vector<string> {"A", "B", "C", "D"},
                           vector<string> {"A|ANY|P0,R|B",
                                                     "B|ANY|R|C",
                                                     "C|ANY|P1,R|D",
                                                     "D|ANY|R|A"});
    simpleTM.run(15);
    return 0;
}
