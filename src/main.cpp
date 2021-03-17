#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iterator>

using namespace std;
static const string BLANK_TAPE_SYMBOL = "ε";
static const string FINAL_TAPE_DELIMITER = "| ";
static const char *const FINAL_TAPE_DELIMITER_PTR = "| ";
static const string RULE_WILDCARD_SYMBOL = "ANY";
static const string PLACEHOLDER_FOR_ANY_AS_VAL = "β";
static const string SENTINEL_SCHWA = "ə";

static const char SD_CHAR_SEMICOLON = ';';
static const char SD_CHAR_SYMBOL_C = 'C';
static const char SD_CHAR_MCONFIG_A = 'A';
static const char SD_CHAR_DELIM_D = 'D';
static const char SD_CHAR_OP_L = 'L';
static const char SD_CHAR_OP_R = 'R';
static const char SD_CHAR_OP_NO_MOVE = 'N';

static const char UTM_PRINTABLE_A = 'A';
static const char UTM_PRINTABLE_C = 'C';
static const char UTM_PRINTABLE_D = 'D';
static const char UTM_PRINTABLE_0 = '0';
static const char UTM_PRINTABLE_1 = '1';
static const char UTM_PRINTABLE_u = 'u';
static const char UTM_PRINTABLE_v = 'v';
static const char UTM_PRINTABLE_w = 'w';
static const char UTM_PRINTABLE_x = 'x';
static const char UTM_PRINTABLE_y = 'y';
static const char UTM_PRINTABLE_z = 'z';
static const char NOP_OPERATOR = 'N';
static const char OPERATOR_PRINT = 'P';
static const char OPERATOR_ERASE = 'E';
static const char OPERATOR_RIGHT = 'R';
static const char OPERATOR_LEFT = 'L';
const char *const ruleNotFoundString = "RULE_NOT_FOUND";

static const bool g_debug = true;
static const char OPERATION_SUBST_CHAR = '#';
char rule_section_delim = '|';
char operation_section_delim = ',';

vector<int> findLocations(string string, char findIt) {
    vector<int> characterLocations;
    for (char i : string) {
        if (i == findIt)
            characterLocations.push_back(i);
    }

    return characterLocations;
}

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
    vector<string> tape;
    long int currTapeIdx = 0L;

    vector<string> m_configurations;

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
                cout
                        << "The curently negated symbol(OPERATION_SUBST_CHAR) is/are: "
                        << negatedSymbol + "\n";
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

        performOperations(operations);

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
            cout << "curr mConfig is: " << currMConfig << "\n";
            cout << "currTapeIdx: " << currTapeIdx << "\n";
            cout << "currTapeSymbol is: " << tape[currTapeIdx] << "\n";
            cout << "currRule is: " << currRule + "\n";
        }
        if (currRule != ruleNotFoundString) {
            parseAndExecuteRule(currRule);
        }
    }

protected:
    string currMConfig;

    void performOperations(vector<string> &operations) {
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
             ostream_iterator<string>(implodedTape, FINAL_TAPE_DELIMITER_PTR));
        cout << implodedTape.str() + "\n";
    }
};

class MConfig {
    string name;
public:
    MConfig(string new_name) {
        name = new_name;
    }

    string getName() {
        return name;
    }
};

enum MfunctionVarType {
    var_enum_mFunction,
    var_enum_mConfig,
    var_enum_char
};

class MFunctionVar {
public:
    MFunctionVar(MfunctionVarType type, const string &value) : type(type),
                                                               value(value) {}

private:
    MfunctionVarType type;
public:
    string value;
};

class MFunctionResult {
public:
    MFunctionResult(vector<MFunctionVar *> *newVars, const string &name)
            : vars(newVars), name(name) {}

    vector<MFunctionVar *> *vars;
    string name;
};

class MFunction : public MConfig {
    vector<MFunctionVar *> *currVars{};
    vector<string> *rules;

public:
    MFunction(const string &newName, vector<MFunctionVar *> *currVars,
              vector<string> *rules) : MConfig(newName), currVars(currVars),
                                       rules(rules) {}

    MFunction(const string &newName, vector<string> *rules) : MConfig(newName),
                                                              rules(rules) {}

    void setVars(vector<MFunctionVar *> *newVars) {
        currVars = newVars;
    }

    vector<MFunctionVar *> *reorderVars(string newMConfigName,
                                        vector<MFunctionVar *> *currVars) {
        vector<MFunctionVar *> *result = new vector<MFunctionVar *>();
        vector<string> vars = splitString(newMConfigName.substr
                (newMConfigName.find_first_of('(', 0) + 1,
                 newMConfigName.find_first_of(')', 0) - 1), ',');
        for (int i = 0; i < vars.size(); i++) {
            int newVarIndex = atoi(&vars[i][1]);
            result->push_back((*currVars)[newVarIndex - 1]);
        }
        return result;
    }

    MFunctionResult *evaluateFunction(vector<string> *tape, int *currTapeIdx) {
        string foundRule = getRule((*tape)[*currTapeIdx]);
        vector<string> ruleParts = splitString(foundRule, rule_section_delim);

        vector<string> operations = splitString(ruleParts[2],
                                                operation_section_delim);
        vector<string> substituted_ops = substituteSymbols(operations);
        performOps(substituted_ops, tape, currTapeIdx);

        string newMConfigName = ruleParts[3];
        vector<MFunctionVar *> *newVars = reorderVars(newMConfigName, currVars);
        return new MFunctionResult(newVars, newMConfigName);
    }

private:
    bool caseMatches(string currCase, string currTapeSymbol) {
        vector<string> ruleParts = splitString(currCase, rule_section_delim);

        string rulePrefix = currTapeSymbol;
        string wildcardRulePrefix = RULE_WILDCARD_SYMBOL;

        string exactMatchResult = "";
        string wildcardMatchResult = "";

        string mConfig = ruleParts[0];
        string symbol = ruleParts[1];

        bool ruleHasOperatorOnSymbol = symbol.find_first_of('(') != string::npos
                && symbol.find_first_of(')') != string::npos;

        if (ruleHasOperatorOnSymbol) {
            vector<string> symbolParts = splitString(symbol, '(');
            string operation = symbolParts[0];
            // Note: currently only supporting unary operators for matching
            string arg = symbolParts[1];
            //DUMMY FOR NOW
            return false;
        } else {
            auto prefixResult = mismatch(rulePrefix.begin(),
                                         rulePrefix.end(),
                                         symbol.begin());
            if (prefixResult.first == rulePrefix.end()) {
                exactMatchResult = currCase;
            }

            auto prefixWildcardResult = mismatch(wildcardRulePrefix.begin(),
                                                 wildcardRulePrefix.end(),
                                                 symbol.begin());
            if (prefixWildcardResult.first == wildcardRulePrefix.end()) {
                wildcardMatchResult = currCase;
            }

            return !exactMatchResult.empty() || !wildcardMatchResult.empty();
        }
    }

    string getRule(string currTapeSymbol) {
        string currCase;
        for (int i = 0; i < rules->size(); i++) {
            currCase = (*rules)[i];
            if (caseMatches(currCase, currTapeSymbol)) {
                return currCase;
            }
        }
        return currCase;
    }

    vector<string> substituteSymbols(vector<string> operations) {
        vector<string> result = {};
        for (string op: operations) {
            if (op[0] == 'P' && op[1] == '#') {
                string opCopy = op;
                opCopy.replace(1, 2, (*currVars)[op[3]]->value);
                result.push_back(opCopy);
            } else {
                result.push_back(op);
            }
        }
        return vector<string>{};
    }

    void performOps(vector<string> ops, vector<string> *tape, int *tapeIdx) {
        cout << "Got into performOps\n";
        for (string operation : ops) {
            if (operation[0] == OPERATOR_PRINT) {
                if (g_debug) {
                    cout << "Printing: " << operation[1] << " at: "
                         << *tapeIdx
                         << "\n";
                }
                (*tape)[*tapeIdx] = operation[1];
            } else if (operation[0] == OPERATOR_ERASE) {
                (*tape)[*tapeIdx] = BLANK_TAPE_SYMBOL;
            } else if (operation[0] == OPERATOR_RIGHT) {
                if (g_debug) {
                    cout << "Going Right at: " << *tapeIdx <<
                         "\n";
                }
                if (*tapeIdx < (*tape).size()) {
                    (*tapeIdx)++;
                }
            } else if (operation[0] == OPERATOR_LEFT) {
                if (g_debug) {
                    cout << "Going Left at: " << *tapeIdx <<
                         "\n";
                }
                if (*tapeIdx > 0) {
                    (*tapeIdx)--;
                }
            } else if (operation[0] != NOP_OPERATOR) {
                if (g_debug) {
                    cout << "Weird operator: " << operation[0] << "\n";
                }
            }
        }
    }
};

class TuringMachineWithFunctions {
    MFunction DUMMY_MFUNCTION = MFunction("DUMMY", nullptr,
                                          nullptr);
    long int numIterationsRun = 0L;
    vector<string> *tape;
    int currTapeIdx = 0;
    vector<MFunctionVar *> *currMFunctionVars = new vector<MFunctionVar *>
            ();
    vector<MFunction *> *functionObjects;
    vector<MConfig *> *mconfigs;
    MFunction currFunction = DUMMY_MFUNCTION;

    MFunction getFunctionByName(string basicString) {
        for (MFunction *function : *functionObjects) {
            if (function->getName() == basicString) {
                return *function;
            }
        }
        return DUMMY_MFUNCTION;
    }

    void performExecutionCycle() {
        cout << "GOT HERE\n";
        currFunction.setVars(currMFunctionVars);
        cout << "GOT HERE2\n";
        MFunctionResult *result = currFunction.evaluateFunction(tape,
                                                                &currTapeIdx);
        cout << "GOT HERE3\n";
        string newFunctionName = result->name;
        cout << "newFunctionName is: " + newFunctionName + " \n";
        cout << "GOT HERE4\n";
        currFunction = getFunctionByName(newFunctionName);
        cout << "GOT HERE5\n";
        vector<MFunctionVar *> *newVars = result->vars;
        currFunction.setVars(newVars);
        cout << "GOT HERE6\n";
    }

public:
    TuringMachineWithFunctions(vector<string> *newTape,
                               vector<MConfig *> *newMConfigs,
                               vector<MFunctionVar *> *newMFunctionVars,
                               vector<MFunction *> *newMFunctions) {
        tape = newTape;
        mconfigs = newMConfigs;
        functionObjects = newMFunctions;
        currFunction = *((*functionObjects)[0]);
        currMFunctionVars = newMFunctionVars;
    };

    void run(long int numIterations) {
        while (numIterationsRun < numIterations) {
            cout << "starting cycle: \n";
            performExecutionCycle();
            numIterationsRun++;
            cout << "currTapeIdx is " + to_string(currTapeIdx) + " \n";
            cout << "Ran a cycle.\n";
            cout << "Num iterations run is: " + to_string(numIterationsRun) +
                    " \n";
            cout << "Num iterations to be run is: " + to_string
                    (numIterations) +
                    " \n";
        }
        ostringstream implodedTape;
        copy((*tape).begin(), (*tape).end(),
             ostream_iterator<string>(implodedTape,
                                      FINAL_TAPE_DELIMITER_PTR));
        cout << implodedTape.str() + "\n";
    }
};

int main() {
//    cout << "simpleTM:\n";
//    vector<string> simpleTMInitialTape(20);
//    fill(simpleTMInitialTape.begin(), simpleTMInitialTape.end(),
//         BLANK_TAPE_SYMBOL);
//    TuringMachine simpleTM(simpleTMInitialTape,
//                           vector<string>{"A", "B", "C", "D"},
//                           vector<string>{"A|ε|P0,R|B",
//                                          "B|ε|R|C",
//                                          "C|ε|P1,R|D",
//                                          "D|ε|R|A"});
//    simpleTM.run(20);
//    cout << "-------------------------------------\n";
//
//    cout << "simplifiedSimpleTM:\n";
//    vector<string> simplifiedSimpleTMInitialTape(20);
//    fill(simplifiedSimpleTMInitialTape.begin(),
//         simplifiedSimpleTMInitialTape.end(), BLANK_TAPE_SYMBOL);
//    TuringMachine simplifiedSimpleTM(simplifiedSimpleTMInitialTape,
//                                     vector<string>{"B"},
//                                     vector<string>{"B|ε|P0|B",
//                                                    "B|0|R,R,P1|B",
//                                                    "B|1|R,R,P0|B"});
//    simplifiedSimpleTM.run(10);
//    cout << "-------------------------------------\n";
//
//    cout << "bigSimpleTM:\n";
//    vector<string> bigSimpleTMInitialTape(100);
//    fill(bigSimpleTMInitialTape.begin(), bigSimpleTMInitialTape.end(),
//         BLANK_TAPE_SYMBOL);
//    TuringMachine bigSimpleTM(bigSimpleTMInitialTape,
//                              vector<string>{"A", "B", "C", "D"},
//                              vector<string>{"A|ε|P0,R|B",
//                                             "B|ε|R|C",
//                                             "C|ε|P1,R|D",
//                                             "D|ε|R|A"});
//    bigSimpleTM.run(100);
//    cout << "-------------------------------------\n";
//
//    cout << "singleOneTM:\n";
//    vector<string> singleOneTMInitialTape(20);
//    fill(singleOneTMInitialTape.begin(), singleOneTMInitialTape.end(),
//         BLANK_TAPE_SYMBOL);
//    TuringMachine singleOneTM(singleOneTMInitialTape,
//                              vector<string>{"B", "C", "D", "E", "F"},
//                              vector<string>{"B|ε|P0,R|C",
//                                             "C|ε|R|D",
//                                             "D|ε|P1,R|E",
//                                             "E|ε|R|F",
//                                             "F|ε|P0,R|E"});
//    singleOneTM.run(20);
//    cout << "-------------------------------------\n";
//
//
//    cout << "transcendentalNumTM:\n";
//    vector<string> transcendentalNumTMInitialTape(100);
//    fill(transcendentalNumTMInitialTape.begin(),
//         transcendentalNumTMInitialTape.end(), BLANK_TAPE_SYMBOL);
//    TuringMachine transcendentalNumTM(transcendentalNumTMInitialTape,
//                                      vector<string>{"B", "O", "Q", "P", "F"},
//                                      vector<string>{
//                                              "B|ε|Pə,R,Pə,R,P0,R,R,P0,L,L|O",
//                                              "O|1|R,Px,L,L,L|O",
//                                              "O|0||Q",
//                                              "Q|1|R,R|Q",
//                                              "Q|0|R,R|Q",
//                                              "Q|ε|P1,L|P",
//                                              "P|x|E,R|Q",
//                                              "P|ə|R|F",
//                                              "P|ε|L,L|P",
//                                              "F|1|R,R|F",
//                                              "F|0|R,R|F",
//                                              "F|ε|P0,L,L|O"
//                                      });
//    transcendentalNumTM.run(100);
//    cout << "-------------------------------------\n";
//
//    cout << "successiveIntsTM:\n";
//    vector<string> successiveIntsTMInitialTape(20);
//    fill(successiveIntsTMInitialTape.begin(), successiveIntsTMInitialTape.end(),
//         BLANK_TAPE_SYMBOL);
//    TuringMachine successiveIntsTM(successiveIntsTMInitialTape,
//                                   vector<string>{"BEGIN", "INCREMENT",
//                                                  "REWIND", "REWIND_ADDZERO"},
//                                   vector<string>{"BEGIN|ε|P0|INCREMENT",
//                                                  "INCREMENT|0|P1|REWIND",
//                                                  "INCREMENT|1,0|N|REWIND_ADDZERO",
//                                                  "INCREMENT|1|P0,L|INCREMENT",
//                                                  "INCREMENT|ε|P1|REWIND",
//                                                  "REWIND|ε|L|INCREMENT",
//                                                  "REWIND|0|R|REWIND",
//                                                  "REWIND|1|R|REWIND",
//                                                  "REWIND_ADDZERO|ε|P1|INCREMENT",
//                                                  "REWIND_ADDZERO|0|R|REWIND_ADDZERO",
//                                                  "REWIND_ADDZERO|1|R|REWIND_ADDZERO",
//                                   });
//    successiveIntsTM.run(3100);
//    cout << "-------------------------------------\n";
//
//    cout << "squareRootTwoTM:\n";
//    vector<string> squareRootTwoTMInitialTape(100);
//    fill(squareRootTwoTMInitialTape.begin(),
//         squareRootTwoTMInitialTape.end(), BLANK_TAPE_SYMBOL);
//    TuringMachine squareRootTwoTM(squareRootTwoTMInitialTape,
//                                  vector<string>{"begin", "new",
//                                                 "markdigits",
//                                                 "findx",
//                                                 "firstr",
//                                                 "lastr",
//                                                 "finddigits",
//                                                 "find1stdigit",
//                                                 "found1stdigit",
//                                                 "find2nddigit",
//                                                 "found2nddigit",
//                                                 "addzero",
//                                                 "addone",
//                                                 "carry",
//                                                 "add-finished",
//                                                 "eraseoldx",
//                                                 "printnewx",
//                                                 "eraseoldy",
//                                                 "printnewy",
//                                                 "resetnewx",
//                                                 "flagresultdigits",
//                                                 "unflagresultdigits",
//                                                 "newdigitiszero",
//                                                 "printzerodigit",
//                                                 "newdigitisone",
//                                                 "printonedigit",
//                                                 "cleanup"
//                                  },
//                                  vector<string>{"begin|ε|P@,R,P1|new",
//                                                 "new|@|R|markdigits",
//                                                 "new|NOT(@)|L|new",
//                                                 "markdigits|0|R,Px,R|markdigits",
//                                                 "markdigits|1|R,Px,R|markdigits",
//                                                 "markdigits|ε|R,Pz,R,R,Pr|findx",
//                                                 "findx|x|E|firstr",
//                                                 "findx|@|N|finddigits",
//                                                 "findx|NOT(@,x)|L,L|findx",
//                                                 "firstr|r|R,R|lastr",
//                                                 "firstr|NOT(r)|R,R|firstr",
//                                                 "lastr|r|R,R|lastr",
//                                                 "lastr|ε|Pr,R,R,Pr|findx",
//                                                 "finddigits|@|R,R|find1stdigit",
//                                                 "finddigits|NOT(@)|L,L|finddigits",
//                                                 "find1stdigit|x|L|found1stdigit",
//                                                 "find1stdigit|y|L|found1stdigit",
//                                                 "find1stdigit|z|L|found2nddigit",
//                                                 "find1stdigit|ε|R,R|find1stdigit",
//                                                 "found1stdigit|0|R|addzero",
//                                                 "found1stdigit|1|R,R,R|find2nddigit",
//                                                 "find2nddigit|x|L|found2nddigit",
//                                                 "find2nddigit|y|L|found2nddigit",
//                                                 "find2nddigit|ε|R,R|find2nddigit",
//                                                 "found2nddigit|0|R|addzero",
//                                                 "found2nddigit|1|R|addone",
//                                                 "found2nddigit|ε|R|addone",
//                                                 "addzero|r|Ps|addfinished",
//                                                 "addzero|u|Pv|addfinished",
//                                                 "addzero|NOT(r,u)|R,R|addzero",
//                                                 "addone|r|Pv|addfinished",
//                                                 "addone|u|Ps,R,R|carry",
//                                                 "addone|NOT(r,u)|R,R|addone",
//                                                 "carry|r|Pu|addfinished",
//                                                 "carry|ε|Pu|newdigitiszero",
//                                                 "carry|u|Pr,R,R|carry",
//                                                 "addfinished|@|R,R|eraseoldx",
//                                                 "addfinished|NOT(@)|L,L|addfinished",
//                                                 "eraseoldx|x|E,L,L|printnewx",
//                                                 "eraseoldx|z|Py,L,L|printnewx",
//                                                 "eraseoldx|NOT(x,z)|R,R|eraseoldx",
//                                                 "printnewx|@|R,R|eraseoldy",
//                                                 "printnewx|y|Pz|finddigits",
//                                                 "printnewx|ε|Px|finddigits",
//                                                 "eraseoldy|y|E,L,L|printnewy",
//                                                 "eraseoldy|NOT(y)|R,R|eraseoldy",
//                                                 "printnewy|@|R|newdigitisone",
//                                                 "printnewy|NOT(@)|Py,R|resetnewx",
//                                                 "resetnewx|ε|R,Px|flagresultdigits",
//                                                 "resetnewx|NOT(ε)|R,R|resetnewx",
//                                                 "flagresultdigits|OPERATION_SUBST_CHAR|Pt,R,R|unflagresultdigits",
//                                                 "flagresultdigits|v|Pw,R,R|unflagresultdigits",
//                                                 "flagresultdigits|NOT(OPERATION_SUBST_CHAR,v)|R,R|flagresultdigits",
//                                                 "unflagresultdigits|OPERATION_SUBST_CHAR|Pr,R,R|unflagresultdigits",
//                                                 "unflagresultdigits|v|Pu,R,R|unflagresultdigits",
//                                                 "unflagresultdigits|NOT(OPERATION_SUBST_CHAR,v)|N|finddigits",
//                                                 "newdigitiszero|@|R|printzerodigit",
//                                                 "newdigitiszero|NOT(@)|L|newdigitiszero",
//                                                 "printzerodigit|0|R,E,R|printzerodigit",
//                                                 "printzerodigit|1|R,E,R|printzerodigit",
//                                                 "printzerodigit|ε|P0,R,R,R|cleanup",
//                                                 "newdigitisone|@|R|printonedigit",
//                                                 "newdigitisone|NOT(@)|L|newdigitisone",
//                                                 "printonedigit|0|R,E,R|printonedigit",
//                                                 "printonedigit|1|R,E,R|printonedigit",
//                                                 "printonedigit|ε|P1,R,R,R|cleanup",
//                                                 "cleanup|ε|N|new",
//                                                 "cleanup|NOT(ε)|E,R,R|cleanup"
//                                  });
//    squareRootTwoTM.run(3000);
//    cout << "-------------------------------------\n";

// ============================================================================
// MFUNCTION EXPERIMENTS
// ============================================================================

// f - find
    vector<string> findInitialTape(10);
    fill(findInitialTape.begin(),
         findInitialTape.end(), BLANK_TAPE_SYMBOL);
    findInitialTape[0] = SENTINEL_SCHWA;
    findInitialTape[5] = "x";

    vector<string> *findRules = new vector<string>();
    findRules->push_back(
            "f(#1_MC,#2_MC,#3_SYMB)|" + SENTINEL_SCHWA + "|L|f1(#1_MC,#2_MC,"
                                                         "#3_SYMB)");
    findRules->push_back(
            "f(#1_MC,#2_MC,#3_SYMB)|NONEMPTY_NOT(" + SENTINEL_SCHWA + ")|L|"
                                                                      "f(#1_MC,#2_MC,#3_SYMB)");

    vector<string> *find1Rules = new vector<string>();
    find1Rules->push_back("f1(#1_MC,#2_MC,#3_SYMB)|#3_SYMB|#1_MC");
    find1Rules->push_back(
            "f1(#1_MC,#2_MC,#3_SYMB)|NONEMPTY_NOT(#3_SYMB)|R|f1(#1_MC,#2_MC,#3_SYMB)");
    find1Rules->push_back(
            "f1(#1_MC,#2_MC,#3_SYMB)|" + BLANK_TAPE_SYMBOL + "|R|f2(#1_MC,"
                                                             "#2_MC,#3_SYMB)");

    vector<string> *find2Cases = new vector<string>();
    find2Cases->push_back("f2(#1_MC,#2_MC,#3_SYMB)|#3_SYMB|#1_MC");
    find2Cases->push_back(
            "f2(#1_MC,#2_MC,#3_SYMB)|NONEMPTY_NOT(#3_SYMB)|R|f1(#1_MC,#2_MC,#3_SYMB)");
    find2Cases->push_back("f2(#1_MC,#2_MC,#3_SYMB)|" + BLANK_TAPE_SYMBOL +
                          "|R|#2_MC");

    MConfig *INIT_MC = new MConfig("MC");
    MConfig *DID_FIND = new MConfig("DF");
    MConfig *DID_NOT_FIND = new MConfig("DNF");
    MFunction *f = new MFunction("f(#1_MC,#2_MC,#3_SYMB)", findRules);
    MFunction *f1 = new MFunction("f1(#1_MC,#2_MC,#3_SYMB)", find1Rules);
    MFunction *f2 = new MFunction("f2(#1_MC,#2_MC,#3_SYMB)", find2Cases);
    vector<MFunction *> *findFunctions = new vector<MFunction *>{f, f1, f2};
    TuringMachineWithFunctions *testFindTM = new TuringMachineWithFunctions
            (&findInitialTape,
             new vector<MConfig *>{
                     INIT_MC, DID_FIND,
                     DID_NOT_FIND},
             new vector<MFunctionVar *>{
                     new MFunctionVar(MfunctionVarType::var_enum_mConfig,
                                      "DF"),
                     new MFunctionVar
                             (MfunctionVarType::var_enum_mConfig,
                              "DNF"),
                     new MFunctionVar
                             (MfunctionVarType::var_enum_char, "x")},
             findFunctions);
    testFindTM->run(50L);


// pcal = 'Print Character As Last'
//vector<string>* pcalCases = new vector<string>();
//
//vector<string> pcalInitialTape(10);
//fill(pcalInitialTape.begin(),
//     pcalInitialTape.end(), BLANK_TAPE_SYMBOL);

// Prints supplied char on the leftmost empty F-square
//MFunction* printCharAsLast = new MFunction("printCharAsLast", new
//    vector<MFunction::MFunctionVar*>(), pcalCases);
//pcalCases->push_back(new MFunctionCase(printCharAsLast, "0", new
//    vector<string> {"R", "E", "R"}, printCharAsLast));

//    cout << "universalTM:\n";
//    vector<string> universalTMInitialTape(200);
//    fill(universalTMInitialTape.begin(), universalTMInitialTape.end(),
//         BLANK_TAPE_SYMBOL);
    // TODO initialize with program contents
    // (NOT WORKING YET)
    // Observation: nesting mfunctions will result in a tree structure similar
    // to the AST a compiler uses... not sure how to parse but at any rate ...
    // current single-vector implementation is not completely correct
    // Alternately I could just expand my mfunctions for the UTM
    // Should be fine, unless the overall number of rules
    // becomes gigantic. I will try tomorrow. Low energy at the moment.
//    TuringMachineWithFunctions universalTM(universalTMInitialTape,
//                                       vector<string>{"f", "f1", "f2"},
//                                       vector<string>{
    // Find
//                                               "f(C,B,α)|ə|L|f1(C,B,α)",
//                                               "f(C,B,α)|NOT()|L|f(C,B,α)",
//                                               "f1(C,B,α)|α||C",
//                                               "f1(C,B,α)|ε|R|f2(C,B,α)",
//                                               "f1(C,B,α)|NOT(α)|R|f1(C,B,α)",
//                                               "f2(C,B,α)|α||C",
//                                               "f2(C,B,α)|ε|R|B",
//                                               "f2(C,B,α)|NOT(α)|R|f1(C,B,α)",
    // Erase
//                                               "q|||e(q,b,x)",
//                                               "e(q,b,x)|||f(e1(q,b,x),b,x)",
//                                               "e1(q,b,x)||E|q",
    // Print-at-left-end
    // AKA print-at-leftmost F-square
    // (Figure square)
//                                               "pe(C,B)|||f(pe1(C,B), C, ə)",
//                                               "pe1(C,B)|ANY|R,R|pe1(C,B)",
//                                               "pe1(C,B)|ə|PB|C",
    // left/right and move after
    // desired char
//                                               "l(C)||L|C",
//                                               "r(C)||R|C",
//                                               "fl(C,B,α)|||f(l(C),B,α)", // called f'
//                                               "fr(C,B,α)|||f(r(C),B,α)", // called
    // f''
    // Copy
//                                               "c(C,B,α)|||fl(c1(C),B,α)",
//                                               "c1(C)|ANY_AS_VAR||pe(C,β)" //
    // Here β=any scanned symbol in
    // ANY_AS_VAR.  Need to
    // implement this.
//                                      });
//    universalTM.run(20);
//    cout << "-------------------------------------\n";

    return 0;
}
