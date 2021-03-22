#include <string>
#include <utility>
#include <vector>
#include <sstream>
#include <iostream>
#include <iterator>

using namespace std;

static const bool TEST_MFUNCTIONS = true;
static const bool TEST_SIMPLE_TMS = false;

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
static const char *const NONEMPTY_NOT_OPERATOR = "NONEMPTY_NOT";

char rule_section_delim = '|';
char operation_section_delim = ',';

vector<int> findLocations(const string& string, char findIt) {
    vector<int> characterLocations;
    for (char i : string) {
        if (i == findIt)
            characterLocations.push_back(i);
    }

    return characterLocations;
}

vector<string> splitString(const string& toSplit, char delim) {
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

        string exactMatchWithIdxResult;
        string exactMatchResult;
        string negateOpMatchResult;
        string wildcardMatchResult;

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
            if (prefixResult.first == rulePrefix.end() && (symbol.find(',') ==
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
            for (const string& nSymbol : negatedSymbols) {
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

    void parseAndExecuteRule(const string& rule) {
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
        tape = std::move(new_tape);
        m_configurations = std::move(new_m_configs);
        rules = std::move(new_rules);
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
    explicit MConfig(string new_name) {
        name = std::move(new_name);
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
    MFunctionVar(MfunctionVarType type, string value) : type(type),
                                                        value(std::move(
                                                                value)) {}

public:
    MfunctionVarType type;
    string value;
};

class MFunctionResult {
public:
    MFunctionResult(MfunctionVarType type, vector<MFunctionVar *> *vars,
                    string name) : type(type), vars(vars),
                                   name(std::move(name)) {}

    MfunctionVarType type;
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

    static vector<MFunctionVar *> *reorderVars(const string &newMConfigName,
                                        vector<MFunctionVar *> *currVars) {
        auto *result = new vector<MFunctionVar *>();
        vector<string> vars = splitString(newMConfigName.substr
                (newMConfigName.find_first_of('(', 0) + 1,
                 newMConfigName.find_first_of(')', 0) - 1), ',');
        for (auto & var : vars) {
            int newVarIndex = atoi(&var[1]);
            result->push_back((*currVars)[newVarIndex - 1]);
        }
        return result;
    }

    static MfunctionVarType getTypeFromRuleName(const string &ruleMConfigName) {
        if (ruleMConfigName.find_first_of('(') != string::npos) {
            return var_enum_mFunction;
        }
        return var_enum_mConfig;
    }

    MFunctionResult *evaluateFunction(vector<string> *tape, int *currTapeIdx) {
        string foundRule = getRule((*tape)[*currTapeIdx]);
        vector<string> ruleParts = splitString(foundRule, rule_section_delim);

        vector<string> operations = splitString(ruleParts[2],
                                                operation_section_delim);
        vector<string> substituted_ops = substituteSymbols(operations);
        performOps(substituted_ops, tape, currTapeIdx);

        string newMConfigName = ruleParts[3];
        MfunctionVarType newMConfigType = getTypeFromRuleName(newMConfigName);
        vector<MFunctionVar *> *newVars = reorderVars(newMConfigName, currVars);
        return new MFunctionResult(newMConfigType, newVars, newMConfigName);
    }

private:
    bool caseMatches(const string &currCase, const string &currTapeSymbol) {
        vector<string> ruleParts = splitString(currCase, rule_section_delim);

        string rulePrefix = currTapeSymbol;
        string wildcardRulePrefix = RULE_WILDCARD_SYMBOL;

        string exactMatchResult;
        string wildcardMatchResult;

        string mConfig = ruleParts[0];
        string symbol = ruleParts[1];

        bool ruleHasOperatorOnSymbol = symbol.find_first_of('(') != string::npos
                                       && symbol.find_first_of(')') !=
                                          string::npos;

        if (ruleHasOperatorOnSymbol) {
            return handleRuleOperator(currTapeSymbol, symbol);
        } else {
            string searchString;
            if (symbol.find_first_of('#') != string::npos) {
                MFunctionVar chosenVar = getFunctionVarFromSubstString(symbol);
                searchString = chosenVar.value;
            } else {
                searchString = symbol;
            }

            auto prefixResult = mismatch(rulePrefix.begin(),
                                         rulePrefix.end(),
                                         searchString.begin());
            if (prefixResult.first == rulePrefix.end()) {
                exactMatchResult = currCase;
            }

            auto prefixWildcardResult = mismatch(wildcardRulePrefix.begin(),
                                                 wildcardRulePrefix.end(),
                                                 searchString.begin());
            if (prefixWildcardResult.first == wildcardRulePrefix.end()) {
                wildcardMatchResult = currCase;
            }

            return !exactMatchResult.empty() || !wildcardMatchResult.empty();
        }
    }

    bool handleRuleOperator(const string &currTapeSymbol,
                            const string &symbol) const {
        const string &symbolWithoutCloseParen = symbol.substr(0,
                                                              symbol.length() -
                                                              1);
        vector<string> symbolParts = splitString(symbolWithoutCloseParen, '(');
        string operation = symbolParts[0];
        // Note: only supporting unary operator for now
        string arg = symbolParts[1];

        if (operation == NONEMPTY_NOT_OPERATOR) {
            if (arg.find_first_of('#') != string::npos) {
                MFunctionVar chosenVar = getFunctionVarFromSubstString(arg);
                return currTapeSymbol != chosenVar.value && currTapeSymbol !=
                                                            BLANK_TAPE_SYMBOL;
            }
            return currTapeSymbol != arg && currTapeSymbol != BLANK_TAPE_SYMBOL;
        }
        return false;
    }

    MFunctionVar
    getFunctionVarFromSubstString(const string &substString) const {
        int argIdx = stoi(
                substString.substr(1, substString.find_first_of('_')));
        MFunctionVar chosenVar = *(*currVars)[argIdx - 1];
        if (chosenVar.type != MfunctionVarType::var_enum_char) {
            cout << "ERROR, passed chosenVar of type: " + to_string
                    (chosenVar.type) + "as substitution parameter\n";
        }
        return chosenVar;
    }

    string getRule(const string &currTapeSymbol) {
        string currCase;
        for (auto & rule : *rules) {
            currCase = rule;
            if (caseMatches(currCase, currTapeSymbol)) {
                return currCase;
            }
        }
        return currCase;
    }

    vector<string> substituteSymbols(const vector<string> &operations) {
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
        return result;
    }

    static void
    performOps(const vector<string> &ops, vector<string> *tape, int *tapeIdx) {
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

    MFunction getFunctionByName(const string &basicString) {
        for (MFunction *function : *functionObjects) {
            if (equivalentFunctions(basicString, function)) {
                return *function;
            }
        }
        return DUMMY_MFUNCTION;
    }

    static bool equivalentFunctions(const string &basicString,
                                    MFunction *function) {
        return function->getName() == basicString;
    }

    int performExecutionCycle() {
        currFunction.setVars(currMFunctionVars);
        MFunctionResult *result = currFunction.evaluateFunction(tape,
                                                                &currTapeIdx);
        if (result->type == MfunctionVarType::var_enum_mFunction) {
            string newFunctionName = result->name;
            cout << "newFunctionName is: " + newFunctionName + " \n";
            currFunction = getFunctionByName(newFunctionName);
            vector<MFunctionVar *> *newVars = result->vars;
            currFunction.setVars(newVars);
            return 0;
        } else {
            cout << "Got to Mconfig: " + result->name + ", stopping.\n";
            return -1;
        }
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
            int resultCode = performExecutionCycle();
            if (resultCode == -1) {
                break;
            }
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
    if (TEST_SIMPLE_TMS) {
        cout << "simpleTM:\n";
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
        cout << "-------------------------------------\n";

        cout << "simplifiedSimpleTM:\n";
        vector<string> simplifiedSimpleTMInitialTape(20);
        fill(simplifiedSimpleTMInitialTape.begin(),
             simplifiedSimpleTMInitialTape.end(), BLANK_TAPE_SYMBOL);
        TuringMachine simplifiedSimpleTM(simplifiedSimpleTMInitialTape,
                                         vector<string>{"B"},
                                         vector<string>{"B|ε|P0|B",
                                                        "B|0|R,R,P1|B",
                                                        "B|1|R,R,P0|B"});
        simplifiedSimpleTM.run(10);
        cout << "-------------------------------------\n";

        cout << "bigSimpleTM:\n";
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
        cout << "-------------------------------------\n";

        cout << "singleOneTM:\n";
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
        cout << "-------------------------------------\n";


        cout << "transcendentalNumTM:\n";
        vector<string> transcendentalNumTMInitialTape(100);
        fill(transcendentalNumTMInitialTape.begin(),
             transcendentalNumTMInitialTape.end(), BLANK_TAPE_SYMBOL);
        TuringMachine transcendentalNumTM(transcendentalNumTMInitialTape,
                                          vector<string>{"B", "O", "Q", "P",
                                                         "F"},
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
        cout << "-------------------------------------\n";

        cout << "successiveIntsTM:\n";
        vector<string> successiveIntsTMInitialTape(20);
        fill(successiveIntsTMInitialTape.begin(),
             successiveIntsTMInitialTape.end(),
             BLANK_TAPE_SYMBOL);
        TuringMachine successiveIntsTM(successiveIntsTMInitialTape,
                                       vector<string>{"BEGIN", "INCREMENT",
                                                      "REWIND",
                                                      "REWIND_ADDZERO"},
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
        cout << "-------------------------------------\n";

        cout << "squareRootTwoTM:\n";
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
                                                     "finddigits|@|R,R|find1stdigit",
                                                     "finddigits|NOT(@)|L,L|finddigits",
                                                     "find1stdigit|x|L|found1stdigit",
                                                     "find1stdigit|y|L|found1stdigit",
                                                     "find1stdigit|z|L|found2nddigit",
                                                     "find1stdigit|ε|R,R|find1stdigit",
                                                     "found1stdigit|0|R|addzero",
                                                     "found1stdigit|1|R,R,R|find2nddigit",
                                                     "find2nddigit|x|L|found2nddigit",
                                                     "find2nddigit|y|L|found2nddigit",
                                                     "find2nddigit|ε|R,R|find2nddigit",
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
                                                     "addfinished|NOT(@)|L,L|addfinished",
                                                     "eraseoldx|x|E,L,L|printnewx",
                                                     "eraseoldx|z|Py,L,L|printnewx",
                                                     "eraseoldx|NOT(x,z)|R,R|eraseoldx",
                                                     "printnewx|@|R,R|eraseoldy",
                                                     "printnewx|y|Pz|finddigits",
                                                     "printnewx|ε|Px|finddigits",
                                                     "eraseoldy|y|E,L,L|printnewy",
                                                     "eraseoldy|NOT(y)|R,R|eraseoldy",
                                                     "printnewy|@|R|newdigitisone",
                                                     "printnewy|NOT(@)|Py,R|resetnewx",
                                                     "resetnewx|ε|R,Px|flagresultdigits",
                                                     "resetnewx|NOT(ε)|R,R|resetnewx",
                                                     "flagresultdigits|OPERATION_SUBST_CHAR|Pt,R,R|unflagresultdigits",
                                                     "flagresultdigits|v|Pw,R,R|unflagresultdigits",
                                                     "flagresultdigits|NOT(OPERATION_SUBST_CHAR,v)|R,R|flagresultdigits",
                                                     "unflagresultdigits|OPERATION_SUBST_CHAR|Pr,R,R|unflagresultdigits",
                                                     "unflagresultdigits|v|Pu,R,R|unflagresultdigits",
                                                     "unflagresultdigits|NOT(OPERATION_SUBST_CHAR,v)|N|finddigits",
                                                     "newdigitiszero|@|R|printzerodigit",
                                                     "newdigitiszero|NOT(@)|L|newdigitiszero",
                                                     "printzerodigit|0|R,E,R|printzerodigit",
                                                     "printzerodigit|1|R,E,R|printzerodigit",
                                                     "printzerodigit|ε|P0,R,R,R|cleanup",
                                                     "newdigitisone|@|R|printonedigit",
                                                     "newdigitisone|NOT(@)|L|newdigitisone",
                                                     "printonedigit|0|R,E,R|printonedigit",
                                                     "printonedigit|1|R,E,R|printonedigit",
                                                     "printonedigit|ε|P1,R,R,R|cleanup",
                                                     "cleanup|ε|N|new",
                                                     "cleanup|NOT(ε)|E,R,R|cleanup"
                                      });
        squareRootTwoTM.run(3000);
        cout << "-------------------------------------\n";

    }

    if (TEST_MFUNCTIONS) {
        // ============================================================================
        // MFUNCTION EXPERIMENTS
        // ============================================================================

        // f - find
        vector<string> findInitialTape(10);
        fill(findInitialTape.begin(),
             findInitialTape.end(), BLANK_TAPE_SYMBOL);
        findInitialTape[0] = SENTINEL_SCHWA;
        findInitialTape[1] = UTM_PRINTABLE_0;
        findInitialTape[2] = UTM_PRINTABLE_0;
        findInitialTape[3] = UTM_PRINTABLE_0;
        findInitialTape[4] = UTM_PRINTABLE_0;
        findInitialTape[5] = UTM_PRINTABLE_x;
        findInitialTape[6] = UTM_PRINTABLE_0;
        findInitialTape[7] = UTM_PRINTABLE_0;
        findInitialTape[8] = UTM_PRINTABLE_0;
        findInitialTape[9] = UTM_PRINTABLE_0;

        auto *findRules = new vector<string>();
        findRules->push_back(
                "f(#1_MC,#2_MC,#3_SYMB)|" + SENTINEL_SCHWA +
                "|L|f1(#1_MC,#2_MC,"
                "#3_SYMB)");
        findRules->push_back(
                "f(#1_MC,#2_MC,#3_SYMB)|NONEMPTY_NOT(" + SENTINEL_SCHWA + ")|L|"
                                                                          "f(#1_MC,#2_MC,#3_SYMB)");

        auto *find1Rules = new vector<string>();
        find1Rules->push_back("f1(#1_MC,#2_MC,#3_SYMB)|#3_SYMB||#1_MC");
        find1Rules->push_back(
                "f1(#1_MC,#2_MC,#3_SYMB)|NONEMPTY_NOT(#3_SYMB)|R|f1(#1_MC,#2_MC,#3_SYMB)");
        find1Rules->push_back(
                "f1(#1_MC,#2_MC,#3_SYMB)|" + BLANK_TAPE_SYMBOL + "|R|f2(#1_MC,"
                                                                 "#2_MC,#3_SYMB)");

        auto *find2Cases = new vector<string>();
        find2Cases->push_back("f2(#1_MC,#2_MC,#3_SYMB)|#3_SYMB||#1_MC");
        find2Cases->push_back(
                "f2(#1_MC,#2_MC,#3_SYMB)|NONEMPTY_NOT(#3_SYMB)|R|f1(#1_MC,#2_MC,#3_SYMB)");
        find2Cases->push_back("f2(#1_MC,#2_MC,#3_SYMB)|" + BLANK_TAPE_SYMBOL +
                              "|R|#2_MC");

        auto *INIT_MC = new MConfig("MC");
        auto *DID_FIND = new MConfig("DF");
        auto *DID_NOT_FIND = new MConfig("DNF");
        auto *f = new MFunction("f(#1_MC,#2_MC,#3_SYMB)", findRules);
        auto *f1 = new MFunction("f1(#1_MC,#2_MC,#3_SYMB)", find1Rules);
        auto *f2 = new MFunction("f2(#1_MC,#2_MC,#3_SYMB)", find2Cases);
//    vector<MFunction *> *findFunctions = new vector<MFunction *>{f, f1, f2};
//    TuringMachineWithFunctions *testFindTM = new TuringMachineWithFunctions
//            (&findInitialTape,
//             new vector<MConfig *>{
//                     INIT_MC, DID_FIND,
//                     DID_NOT_FIND},
//             new vector<MFunctionVar *>{
//                     new MFunctionVar(MfunctionVarType::var_enum_mConfig,
//                                      "DF"),
//                     new MFunctionVar
//                             (MfunctionVarType::var_enum_mConfig,
//                              "DNF"),
//                     new MFunctionVar
//                             (MfunctionVarType::var_enum_char, "x")},
//             findFunctions);
//    testFindTM->run(12L);

        // e - erase
        vector<string> eraseInitialTape(10);
        fill(eraseInitialTape.begin(),
             eraseInitialTape.end(), BLANK_TAPE_SYMBOL);
        eraseInitialTape[0] = SENTINEL_SCHWA;
        eraseInitialTape[1] = UTM_PRINTABLE_0;
        eraseInitialTape[2] = UTM_PRINTABLE_0;
        eraseInitialTape[3] = UTM_PRINTABLE_0;
        eraseInitialTape[4] = UTM_PRINTABLE_0;
        eraseInitialTape[5] = UTM_PRINTABLE_x;
        eraseInitialTape[6] = UTM_PRINTABLE_0;
        eraseInitialTape[7] = UTM_PRINTABLE_0;
        eraseInitialTape[8] = UTM_PRINTABLE_0;
        eraseInitialTape[9] = UTM_PRINTABLE_0;

        auto *erase3argsrules = new vector<string>();
        erase3argsrules->push_back(
                "e(#1_MC,#2_MC,#3_SYMB)|ANY||f(e1(#1_MC,#2_MC,"
                "#3_SYMB),#2_MC,#3_SYMB)");
        auto *e1Rules = new vector<string>();
        e1Rules->push_back("e1(#1_MC,#2_MC,#3_SYMB)|ANY|E|#2_MC");
        auto *erase2argsrules = new vector<string>();
        erase2argsrules->push_back("e(#1_MC,#2_SYMB)|ANY||e(e(#1_MC,#2_SYMB),"
                                   "#1_MC,#2_SYMB)");

        auto *DID_ERASE = new MConfig("DE");
        auto *DID_NOT_ERASE = new MConfig("DNE");
        auto *erase3args = new MFunction("e(#1_MC,#2_MC,#3_SYMB)",
                                              erase3argsrules);
        auto *e1 = new MFunction("e1(#1_MC,#2_MC,#3_SYMB)", e1Rules);
        auto *erase2args = new MFunction("e(#1_MC,#2_SYMB)",
                                              erase2argsrules);
        auto *eraseFunctions = new vector<MFunction *>{
                erase3args,
                e1,
                erase2args,
                f, f1, f2};
        auto *testEraseTM = new TuringMachineWithFunctions(
                &eraseInitialTape,
                new vector<MConfig *>{
                        INIT_MC, DID_ERASE, DID_NOT_ERASE
                },
                new vector<MFunctionVar *>{
                        new MFunctionVar(MfunctionVarType::var_enum_mConfig,
                                         "DE"),
                        new MFunctionVar(MfunctionVarType::var_enum_mConfig,
                                         "DNE"),
                        new MFunctionVar(MfunctionVarType::var_enum_char, "x"),
                },
                eraseFunctions);
        testEraseTM->run(12L);
    }

    return 0;
}
