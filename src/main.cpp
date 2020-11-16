using namespace std;

class TuringMachine {
    char OPERATOR_PRINT = "P";
    char OPERATOR_ERASE = "E";
    char OPERATOR_RIGHT = "R";
    char OPERATOR_LEFT = "L";

    long int tape_size;
    char tape[];
    long int currTapeIdx = 0L;

    long int num_m_configurations = 3L;
    char m_configurations[num_m_configurations];
    char curr_m_configuration;

    char rule_section_delim = "|";
    long int num_rules;
    string rules[];
    
    void performExecutionCycle() {
        string currRule = this.getRule();
        this.parseAndExecuteRule(currRule);    
    }
    public:


}

int main() {
}
