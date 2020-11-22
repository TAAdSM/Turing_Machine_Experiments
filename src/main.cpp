#include <string>
#include <vector>
#include <sstream>
#include <iostream>

static const char *const BLANK_TAPE_SYMBOL = "ε";
static const char *const FINAL_TAPE_DELIMITER = "| ";
static const char *const RULE_WILDCARD_SYMBOL = "ANY";

static const bool g_debug = false;
using namespace std;

vector<string> splitString(string toSplit, char delim) {
    stringstream sstream(toSplit);
    string item;
    vector<string> splitted;
    while (getline(sstream, item, delim)) {
        splitted.push_back(item);
    }
    return splitted;
}

class TuringMachine {
    const char *const ruleNotFoundString = "RULE_NOT_FOUND";
    const char NOP_OPERATOR = 'N';
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
        string ruleWithIdxPrefix = currMConfig + rule_section_delim +
                                   tape[currTapeIdx] + "," +
                                   to_string(currTapeIdx);
        string rulePrefix = currMConfig +
                            rule_section_delim + tape[currTapeIdx];
        string wildcardRulePrefix =
                currMConfig + rule_section_delim + RULE_WILDCARD_SYMBOL;

        string exactMatchWithIdxResult = "";
        string exactMatchResult = "";
        string negateOpMatchResult = "";
        string wildcardMatchResult = "";

        for (string rule: rules) {
            vector<string> ruleParts = splitString(rule, rule_section_delim);
            string mConfig = ruleParts[0];
            string symbol = ruleParts[1];

            string notOpSymbolPrefix;
            handleNegatedSymbols(rule, mConfig, symbol, negateOpMatchResult,
                                 notOpSymbolPrefix);

            auto prefixWithIdxResult = mismatch(ruleWithIdxPrefix.begin(),
                                                ruleWithIdxPrefix.end(),
                                                rule.begin());
            if (prefixWithIdxResult.first == ruleWithIdxPrefix.end()) {
                exactMatchWithIdxResult = rule;
                break;
            }

            auto prefixResult = mismatch(rulePrefix.begin(), rulePrefix.end(),
                                         rule.begin());
            if (prefixResult.first == rulePrefix.end() && (symbol.find(",") ==
                                                           string::npos &&
                                                           symbol.find
                                                                   (notOpSymbolPrefix)
                                                           == string::npos)) {
                exactMatchResult = rule;
                break;
            }
            auto prefixWildcardResult = mismatch(wildcardRulePrefix.begin(),
                                                 wildcardRulePrefix.end(),
                                                 rule.begin());
            if (prefixWildcardResult.first == wildcardRulePrefix.end()) {
                wildcardMatchResult = rule;
            }
        }
        return !exactMatchWithIdxResult.empty() ? exactMatchWithIdxResult :
               !exactMatchResult.empty() ? exactMatchResult :
               !negateOpMatchResult.empty() ? negateOpMatchResult :
               !wildcardMatchResult.empty() ? wildcardMatchResult
                                            : ruleNotFoundString;
    }

    void handleNegatedSymbols(const string &rule, const string &mConfig,
                              string &symbol,
                              string &negateOpMatchResult,
                              string &notOpSymbolPrefix) {
        notOpSymbolPrefix = "NOT(";
        auto notOpSymbolPrefixResult = mismatch(notOpSymbolPrefix.begin(),
                                                notOpSymbolPrefix.end(),
                                                symbol.begin());
        if (currMConfig == mConfig && notOpSymbolPrefixResult.first ==
                                      notOpSymbolPrefix.end()) {
            string negatedSymbol = string(notOpSymbolPrefixResult.second,
                                          symbol.end() - 1);
            if (g_debug) {
                cout << "The curently negated symbol(s) is/are: " <<
                     negatedSymbol + "\n";
            }
            vector<string> negatedSymbols = splitString(negatedSymbol, ',');
            bool allSymbolsNegated = true;
            for (string nSymbol : negatedSymbols) {
                if (tape[currTapeIdx] == nSymbol) {
                    allSymbolsNegated = false;
                    break;
                }
            }
            if (allSymbolsNegated) {
                negateOpMatchResult = rule;
            }
        }
    }

    void parseAndExecuteRule(string rule) {
        vector<string> ruleParts = splitString(rule, rule_section_delim);
        vector<string> operations = splitString(ruleParts[2],
                                                operation_section_delim);
        string newMConfig = ruleParts[3];

        for (string operation : operations) {
            if (operation[0] == OPERATOR_PRINT) {
                if (g_debug) {
                    cout << "Printing: " << operation[1] << " at: "
                         << currTapeIdx
                         << "\n";
                }
                tape[currTapeIdx] = operation[1];
            } else if (operation[0] == OPERATOR_ERASE) {
                tape[currTapeIdx] = BLANK_TAPE_SYMBOL;
            } else if (operation[0] == OPERATOR_RIGHT) {
                if (g_debug) {
                    cout << "Going Right at: " << currTapeIdx <<
                         "\n";
                }
                if (currTapeIdx < tape.size()) {
                    currTapeIdx++;
                }
            } else if (operation[0] == OPERATOR_LEFT) {
                if (g_debug) {
                    cout << "Going Left at: " << currTapeIdx <<
                         "\n";
                }
                if (currTapeIdx > 0) {
                    currTapeIdx--;
                }
            } else if (operation[0] != NOP_OPERATOR) {
                if (g_debug) {
                    cout << "Weird operator: " << operation[0] << "\n";
                }
            }
        }

        if (g_debug) {
            cout << "Changing mConfig: " << currMConfig << " -> "
                 << newMConfig
                 << "\n";
        }
        currMConfig = newMConfig;
    }

    void performExecutionCycle() {
        if (g_debug) {
            cout <<
                 "--------------------------------------------\n";
        }
        string currRule = getRule();
        if (g_debug) {
            cout << "New cycle.\n";
            cout << "currRule is: " << currRule + "\n";
            cout << "currTapeIdx: " << currTapeIdx << "\n";
        }
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
        copy(tape.begin(), tape.end(),
             ostream_iterator<string>(implodedTape, FINAL_TAPE_DELIMITER));
        cout << implodedTape.str() + "\n";
    }
};

int main() {
    vector<string> simpleTMInitialTape(20);
    fill(simpleTMInitialTape.begin(), simpleTMInitialTape.end(),
         BLANK_TAPE_SYMBOL);
    TuringMachine simpleTM(simpleTMInitialTape,
                           vector<string>{"A", "B", "C", "D"},
                           vector<string>{"A|ε|P0,R|B",
                                          "B|ε|R|C",
                                          "C|ε|P1,R|D",
                                          "D|ε|R|A"});
    simpleTM.run(20);

    vector<string> simplifiedSimpleTMInitialTape(20);
    fill(simplifiedSimpleTMInitialTape.begin(),
         simplifiedSimpleTMInitialTape.end(), BLANK_TAPE_SYMBOL);
    TuringMachine simplifiedSimpleTM(simplifiedSimpleTMInitialTape,
                                     vector<string>{"B"},
                                     vector<string>{"B|ε|P0|B",
                                                    "B|0|R,R,P1|B",
                                                    "B|1|R,R,P0|B"});
    simplifiedSimpleTM.run(20);

    vector<string> bigSimpleTMInitialTape(100);
    fill(bigSimpleTMInitialTape.begin(), bigSimpleTMInitialTape.end(),
         BLANK_TAPE_SYMBOL);
    TuringMachine bigSimpleTM(bigSimpleTMInitialTape,
                              vector<string>{"A", "B", "C", "D"},
                              vector<string>{"A|ε|P0,R|B",
                                             "B|ε|R|C",
                                             "C|ε|P1,R|D",
                                             "D|ε|R|A"});
    bigSimpleTM.run(100);

    vector<string> singleOneTMInitialTape(20);
    fill(singleOneTMInitialTape.begin(), singleOneTMInitialTape.end(),
         BLANK_TAPE_SYMBOL);
    TuringMachine singleOneTM(singleOneTMInitialTape,
                              vector<string>{"B", "C", "D", "E", "F"},
                              vector<string>{"B|ε|P0,R|C",
                                             "C|ε|R|D",
                                             "D|ε|P1,R|E",
                                             "E|ε|R|F",
                                             "F|ε|P0,R|E"});
    singleOneTM.run(20);


    vector<string> transcendentalNumTMInitialTape(100);
    fill(transcendentalNumTMInitialTape.begin(),
         transcendentalNumTMInitialTape.end(), BLANK_TAPE_SYMBOL);
    TuringMachine transcendentalNumTM(transcendentalNumTMInitialTape,
                                      vector<string>{"B", "O", "Q", "P", "F"},
                                      vector<string>{
                                              "B|ε|Pə,R,Pə,R,P0,R,R,P0,L,L|O",
                                              "O|1|R,Px,L,L,L|O",
                                              "O|0||Q",
                                              "Q|1|R,R|Q",
                                              "Q|0|R,R|Q",
                                              "Q|ε|P1,L|P",
                                              "P|x|E,R|Q",
                                              "P|ə|R|F",
                                              "P|ε|L,L|P",
                                              "F|1|R,R|F",
                                              "F|0|R,R|F",
                                              "F|ε|P0,L,L|O"
                                      });
    transcendentalNumTM.run(100);

    vector<string> successiveIntsTMInitialTape(20);
    fill(successiveIntsTMInitialTape.begin(), successiveIntsTMInitialTape.end(),
         BLANK_TAPE_SYMBOL);
    TuringMachine successiveIntsTM(successiveIntsTMInitialTape,
                                   vector<string>{"BEGIN", "INCREMENT",
                                                  "REWIND", "REWIND_ADDZERO"},
                                   vector<string>{"BEGIN|ε|P0|INCREMENT",
                                                  "INCREMENT|0|P1|REWIND",
                                                  "INCREMENT|1,0|N|REWIND_ADDZERO",
                                                  "INCREMENT|1|P0,L|INCREMENT",
                                                  "INCREMENT|ε|P1|REWIND",
                                                  "REWIND|ε|L|INCREMENT",
                                                  "REWIND|0|R|REWIND",
                                                  "REWIND|1|R|REWIND",
                                                  "REWIND_ADDZERO|ε|P1|INCREMENT",
                                                  "REWIND_ADDZERO|0|R|REWIND_ADDZERO",
                                                  "REWIND_ADDZERO|1|R|REWIND_ADDZERO",
                                   });
    successiveIntsTM.run(3100);

    vector<string> squareRootTwoTMInitialTape(100);
    fill(squareRootTwoTMInitialTape.begin(),
         squareRootTwoTMInitialTape.end(), BLANK_TAPE_SYMBOL);
    TuringMachine squareRootTwoTM(squareRootTwoTMInitialTape,
                                  vector<string>{"begin", "new",
                                                 "markdigits",
                                                 "findx",
                                                 "firstr",
                                                 "lastr",
                                                 "finddigits",
                                                 "find1stdigit",
                                                 "found1stdigit",
                                                 "find2nddigit",
                                                 "found2nddigit",
                                                 "addzero",
                                                 "addone",
                                                 "carry",
                                                 "add-finished",
                                                 "eraseoldx",
                                                 "printnewx",
                                                 "eraseoldy",
                                                 "printnewy",
                                                 "resetnewx",
                                                 "flagresultdigits",
                                                 "unflagresultdigits",
                                                 "newdigitiszero",
                                                 "printzerodigit",
                                                 "newdigitisone",
                                                 "printonedigit",
                                                 "cleanup"
                                  },
                                  vector<string>{"begin|ε|P@,R,P1|new",
                                                 "new|@|R|markdigits",
                                                 "new|NOT(@)|L|new",
                                                 "markdigits|0|R,Px,R|markdigits",
                                                 "markdigits|1|R,Px,R|markdigits",
                                                 "markdigits|ε|R,Pz,R,R,Pr|findx",
                                                 "findx|x|E|firstr",
                                                 "findx|@|N|finddigits",
                                                 "findx|NOT(@,x)|L,L|findx",
                                                 "firstr|r|R,R|lastr",
                                                 "firstr|NOT(r)|R,R|firstr",
                                                 "lastr|r|R,R|lastr",
                                                 "lastr|ε|Pr,R,R,Pr|findx",
                                                 "finddigits|@|R,"
                                                 "R|find1stdigit",
                                                 "finddigits|NOT(@)|L,"
                                                 "L|finddigits",
                                                 "find1stdigit|x|L|found1stdigit",
                                                 "find1stdigit|y|L|found1stdigit",
                                                 "find1stdigit|z|L|found2nddigit",
                                                 "find1stdigit|ε|R,"
                                                 "R|find1stdigit",
                                                 "found1stdigit|0|R|addzero",
                                                 "found1stdigit|1|R,R,"
                                                 "R|find2nddigit",
                                                 "find2nddigit|x|L|found2nddigit",
                                                 "find2nddigit|y|L|found2nddigit",
                                                 "find2nddigit|none|R,"
                                                 "R|find2nddigit",
                                                 "found2nddigit|0|R|addzero",
                                                 "found2nddigit|1|R|addone",
                                                 "found2nddigit|ε|R|addone",
                                                 "addzero|r|Ps|addfinished",
                                                 "addzero|u|Pv|addfinished",
                                                 "addzero|NOT(r,u)|R,R|addzero",
                                                 "addone|r|Pv|addfinished",
                                                 "addone|u|Ps,R,R|carry",
                                                 "addone|NOT(r,u)|R,R|addone",
                                                 "carry|r|Pu|addfinished",
                                                 "carry|ε|Pu|newdigitiszero",
                                                 "carry|u|Pr,R,R|carry",
                                                 "addfinished|@|R,R|eraseoldx",
                                                 "addfinished|NOT(@)|L,"
                                                 "L|addfinished",
                                                 "eraseoldx|x|E,L,L|printnewx",
                                                 "eraseoldx|z|Py,L,L|printnewx",
                                                 "eraseoldx|NOT(x,z)|R,"
                                                 "R|eraseoldx",
                                                 "printnewx|@|R,R|eraseoldy",
                                                 "printnewx|y|Pz|finddigits",
                                                 "printnewx|ε|Px|finddigits",
                                                 "eraseoldy|y|E,L,L|printnewy",
                                                 "eraseoldy|NOT(y)|R,"
                                                 "R|eraseoldy",
                                                 "printnewy|@|R|newdigitisone",
                                                 "printnewy|NOT(@)|Py,"
                                                 "R|resetnewx",
                                                 "resetnewx|ε|R,"
                                                 "Px|flagresultdigits",
                                                 "resetnewx|else|R,R|resetnewx",
                                                 "flagresultdigits|s|Pt,R,"
                                                 "R|unflagresultdigits",
                                                 "flagresultdigits|v|Pw,R,"
                                                 "R|unflagresultdigits",
                                                 "flagresultdigits|NOT(s,v)"
                                                 "|R,R|flagresultdigits",
                                                 "unflagresultdigits|s|Pr,R,"
                                                 "R|unflagresultdigits",
                                                 "unflagresultdigits|v|Pu,R,R|unflagresultdigits",
                                                 "unflagresultdigits|NOT(s,v)"
                                                 "|N|findigits",
                                                 "newdigitiszero|@|R|printzerodigit",
                                                 "newdigitiszero|NOT(@)"
                                                 "|L|newdigitiszero",
                                                 "printzerodigit|0|R,E,"
                                                 "R|printzerodigit",
                                                 "printzerodigit|1|R,E,"
                                                 "R|printzerodigit",
                                                 "printzerodigit|ε|P0,R,R,"
                                                 "R|cleanup",
                                                 "newdigitisone|@|R|printonedigit",
                                                 "newdigitisone|NOT(@)"
                                                 "|L|newdigitisone",
                                                 "printonedigit|0|R,E,"
                                                 "R|printonedigit",
                                                 "printonedigit|1|R,E,"
                                                 "R|printonedigit",
                                                 "printonedigit|ε|P1,R,R,"
                                                 "R|cleanup",
                                                 "cleanup|ε|N|new",
                                                 "cleanup|NOT(ε)|E,R,R|cleanup",
                                  });
    squareRootTwoTM.run(56);
    return 0;
}
