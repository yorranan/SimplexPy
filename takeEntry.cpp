#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>

#define POSITIVE_SIGNAL 1
#define NEGATIVE_SIGNAL -1
#define EQUAL_SIGNAL 0

using namespace std;

bool isMaximization = false;
bool haveOneBiggerThanZero = false;
typedef struct
{
    string goal;
    vector<string> expressions;
}FileContent;

vector<string> split(const string &s, char delimiter)
{ 
    vector<string> tokens;
    stringstream ss(s);
    string token;
    while (getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}

void tokenParser(string &input, int *index, double &value) {
    value = 1.0;
    *index = 0;

    if (input.empty())
    {
        cerr << "A string da entrada está vazia!" << endl;
        return;
    }

    size_t pos = 0;
    bool isNegative = false;
    if (input[0] == '-')
    {
        isNegative = true;
    } 
    else if (input[0] != '+')
    {
        string aux = "+";
        aux.append(input);
        input = aux;
        pos++;
    }
    else
    {
        pos++;
    }

    size_t xPos = input.find('x');
    string valueStr = input.substr(pos, xPos - pos);
    if (valueStr.empty())
    {
        value = 1.0;
    }
    else if (valueStr == "-")
    {
        value = -1.0;
    }
    else
    {
        value = stod(valueStr);
    }

    int aux_index; 
    *index = stoi(input.substr(xPos + 1));
    if (xPos == string::npos)
    {
        cerr << "Formato Inválido: 'x' não encontrado em " << input << endl;
        return;
    }
}

vector<string> tokenizeExpression(string expression)
{
    int i = 0;
    string newExpression = "";
    while (expression[i] != '\0')
    {
        if (expression[i] == '+' && i > 0)
        {
            newExpression += ";+";
        }
        else if (expression[i] == '-' && i > 0)
        {
            newExpression += ";-";
        }
        else if (expression[i] != ' ')
        {
            newExpression += expression[i];
        }
        i++;
    }
    newExpression += ";";

    vector<string> tokens;
    if (newExpression != "")
        tokens = split(newExpression, ';');
    return tokens;
}
vector<double> parser_goal(
    string str,
    int &max_index
    ){
    int i = 0;
    size_t limitPosition = str.find("=");
    string substr = "", b_value = "";
    string aux = str.substr(0, limitPosition - 1);
    if (aux.find("max") != string::npos || aux.find("Max") != string::npos || aux.find("Maximização") != string::npos || aux.find("Maximize") != string::npos)
    {
        isMaximization = true;
    }
    else if (aux.find("min") != string::npos || aux.find("Min") != string::npos || aux.find("Minimização") != string::npos || aux.find("Minimize") != string::npos)
    {
        isMaximization = false;
    }
    else {
        cerr << "Formato Inválido: limit: " << limitPosition <<" Nenhuma palavra definindo operação encontrada em " << str.substr(0, limitPosition - 1) << endl;
        return vector<double>();
    }
    substr = str.substr(limitPosition + 1);
    vector<string> tokens = tokenizeExpression(substr);
    int index;
    double value;
    vector<pair<double, int>> values;   // valores dos coeficientes
    for (i = 0; i < tokens.size(); i++)
    {
        // Se der errado tirar o if
        if (tokens[i].empty())
        {
            continue;
        }
        printf(" token[%d] = %s\n", i, tokens[i].c_str());
        tokenParser(tokens[i], &index, value);
        values.push_back(make_pair(value, index));
        if (index > max_index)
            max_index = index;
    }

    vector<double> result(max_index, 0.0);
    for (i = 0; i < values.size(); i++)
    {
        result[values[i].second - 1] = values[i].first;
    }

    return result;
}

vector<double> parser(
    string str,
    int &max_index,
    vector<pair<double,int>> &slack_value
    )
{
    int i = 0;
    size_t limitPosition;
    string substr = "", b_value = "";
    bool haveInequantionSignal = false;
    bool oneSimbol = false;
    /**
     * Se for a função objetivo, a expressão é do tipo: 2x1 + 3x2 + 4x3
     * string::npos é o valor retornado quando a substring não é encontrada, ele funciona como um sentinela
     * 
 */
    if (str.find(">=") != string::npos || str.find(">") != string::npos)
    {
        limitPosition = str.find(">=");
        haveOneBiggerThanZero = true;
        haveInequantionSignal = true;
        if (limitPosition == string::npos)
        {
            limitPosition = str.find(">");
            if (limitPosition != string::npos)
            {
                haveOneBiggerThanZero = true;
                haveInequantionSignal = true;
                oneSimbol = true;
            }
        }
        
    }
    else if (str.find("<=") != string::npos || str.find("<") != string::npos)
    {
        limitPosition = str.find("<=");
        if (limitPosition == string::npos)
        {
            limitPosition = str.find("<");
            if (limitPosition != string::npos)
            {
                oneSimbol = true;
            }
        }
        haveInequantionSignal = true;
    }
    else if (str.find("=") != string::npos)
    {
        if (!oneSimbol)
        {
            limitPosition = str.find("=");
        }
        haveInequantionSignal = false;
    }
    else
    {
        cerr << "Formato Inválido: Nenhum sinal de limite encontrado em " << str << endl;
        return vector<double>();
    }
    
    b_value = str.substr(limitPosition + 1);
    if (!oneSimbol)
    {
        b_value = str.substr(limitPosition + 2);
    }
    if (str.find(">=") != string::npos || str.find(">") != string::npos){
        slack_value.push_back(make_pair(stod(b_value), POSITIVE_SIGNAL));
    }
    else if (str.find("<=") != string::npos || str.find("<") != string::npos){
        slack_value.push_back(make_pair(stod(b_value), NEGATIVE_SIGNAL));
    }
    else if (str.find("=") != string::npos){
        slack_value.push_back(make_pair(stod(b_value), EQUAL_SIGNAL));
    }
    else {
        cerr << "Formato Inválido: Nenhum sinal de limite encontrado em " << str << endl;
        return vector<double>();
    }

    substr = str.substr(0, limitPosition);


    vector<string> tokens = tokenizeExpression(substr);
    int index;
    double value;
    vector<pair<double, int>> values; // valores dos coeficientes
    for (i = 0; i < tokens.size(); i++)
    {
        printf(" token[%d] = %s\n", i, tokens[i].c_str());
        tokenParser(tokens[i], &index, value);
        values.push_back(make_pair(value, index));
        if (index > max_index)
            max_index = index;
    }

    vector<double> result(max_index, 0.0);
    for (i = 0; i < values.size(); i++)
    {
        result[values[i].second - 1] = values[i].first;
    }

    return result;
}

FileContent readFile()
{
    FileContent fileContent;
    fileContent.goal = "";
    fileContent.expressions = vector<string>();
    ifstream file("entrada.txt");
    if (!file.is_open())
    {
        cerr << "Erro ao abrir o arquivo." << endl;
    }
    getline(file, fileContent.goal);
    string line;
    while (getline(file, line))
    {
        fileContent.expressions.push_back(line);
    }
    return fileContent;
}

void addSlackVariables(
vector<vector<double>> &expressions,
vector<double> &b,
vector<pair<double, int>> &slack_variables,
int &max_index, vector<double> &goal,
bool &haveOneBiggerThanZero
)
 {
    vector<vector<double>> aux_expression(expressions.size(), vector<double>(slack_variables.size(), 0.0));
    int i;
    for (i = 0; i < expressions.size(); i++)
    {
        if (slack_variables[i].second == POSITIVE_SIGNAL)
        {
            aux_expression[i][i] = -1.0;
            haveOneBiggerThanZero = true;
        }
        else if (slack_variables[i].second == NEGATIVE_SIGNAL)
        {
            aux_expression[i][i] = 1.0;
        }
        else
        {
            aux_expression[i][i] = 0.0;
            haveOneBiggerThanZero = true;
        }
        b.push_back(slack_variables[i].first); 
        // Se no vetor b algum índice for menor que 0, então multiplique toda a restrição por (-1) e inverta o sentido da desigualdade   
        expressions[i].insert(expressions[i].end(), aux_expression[i].begin(), aux_expression[i].end());
        if (slack_variables[i].first < 0)
        {
            b[i] *= -1;
        }
    }
    if (isMaximization)
    {
        for_each(goal.begin(), goal.end(), [](double &value) {
            value *= -1;
        });
    }
    for (int i = 0; i < slack_variables.size(); i++)
    {
        goal.push_back(0.0);
    }
}

FileContent fileContent = readFile();

void writeMatrixAtFile(vector<vector<double>> A){
    ofstream file("A.txt");
    if (!file.is_open())
    {
        cerr << "Erro ao abrir o arquivo." << endl;
    }
    for (int i = 0; i < A.size(); i++)
    {
        for (int j = 0; j < A[i].size(); j++)
        {
            file << A[i][j] << " ";
        }
        file << endl;
    }
    file.close();

}

void writeVectorAtFileB(vector<double> b){
    ofstream file("b.txt"); 
    if (!file.is_open())
    {
        cerr << "Erro ao abrir o arquivo." << endl;
    }
    for (int i = 0; i < b.size(); i++)
    {
        file << b[i] << " ";
    }
    file.close();
}

void writeVectorAtFileC(vector<double> c){
    ofstream file("c.txt"); 
    if (!file.is_open())
    {
        cerr << "Erro ao abrir o arquivo." << endl;
    }
    for (int i = 0; i < c.size(); i++)
    {
        file << c[i] << " ";
    }
    file.close();
}

void writeInfosAtFile(int countSlack, int basic_variables){
    ofstream file("infos.txt");
    if (!file.is_open())
    {
        cerr << "Erro ao abrir o arquivo." << endl;
    }
    if (isMaximization) {
        file << "1 ";
    } else {
        file << "0 ";
    }
    file << countSlack << " " << basic_variables;
    file.close();
}

int main()
{
    vector<vector<double>> expressions;
    vector<double> b;
    vector<double> goal;
    bool haveOneBiggerThanZero;
    int max_index = 0;
    vector<pair<double, int>> slack_variables;
    goal = parser_goal(fileContent.goal, max_index);
    cout << endl;
    int limit_index = max_index;
    for (int i = 0; i < fileContent.expressions.size(); i++)
    {
        expressions.push_back(parser(fileContent.expressions[i], max_index, slack_variables));
    }
    int countSlack = slack_variables.size();    
    int basic_variables = goal.size();
    writeInfosAtFile(countSlack, basic_variables);
    addSlackVariables(expressions, b, slack_variables, limit_index, goal, haveOneBiggerThanZero);
    writeVectorAtFileB(b);
    writeMatrixAtFile(expressions);
    writeVectorAtFileC(goal);
}
