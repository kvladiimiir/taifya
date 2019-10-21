// Moore_Mealy.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>

using namespace std;

const string TYPE_MOORE = "moore";
const string TYPE_MEALY = "mealy";
const int MAX_NUM = 100;
const int MIN_NUM = 0;

ofstream fileOutput("output.txt");

struct ZParameter
{
	int S;
	int Y;
};

struct VertexProps
{
	string label;
};

struct EdgeProps
{
	string label;
};

void PrintMatrixInFile(vector<vector<string>>& matrix, int numRows, int numCol)
{
	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numCol; j++)
		{
			fileOutput << matrix[i][j] << " ";
		}
		fileOutput << '\n';
	}
}



void WriteGraphMoore(vector<vector<string>>& matrix, int numRows, int numCol)
{
	using Edge = pair<int, int>;
	using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps, EdgeProps>;

	ofstream fout("mooreGraph.dot");

	Graph graph;
	vector<Graph::vertex_descriptor> vertices;
	for (int j = 0; j < numCol; ++j)
	{
		string vertexLabel = 'q' + to_string(j) + matrix[0][j];
		vertices.push_back(boost::add_vertex({ vertexLabel }, graph));
	}
	for (int i = 1; i < numRows; i++)
	{
		for (int j = 0; j < numCol; j++)
		{
			string argument = matrix[i][j];
			char state = argument[1];
			int param = (int)state - 48;
			string weights = 'x' + to_string(i);
			boost::add_edge(vertices[j], vertices[param], { weights }, graph);
		}
	}

	boost::dynamic_properties dp;
	dp.property("label", boost::get(&VertexProps::label, graph));
	dp.property("label", boost::get(&EdgeProps::label, graph));
	dp.property("node_id", boost::get(boost::vertex_index, graph));

	write_graphviz_dp(fout, graph, dp);
}

void WriteGraphMealy(vector<vector<string>>& matrix, int numRows, int numCol)
{
	using Edge = pair<int, int>;
	using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps, EdgeProps>;

	ofstream fout("mealyGraph.dot");

	Graph graph;
	vector<Graph::vertex_descriptor> vertices;
	for (int j = 0; j < numCol; ++j)
	{
		string vertexLabel = 'q' + to_string(j);
		vertices.push_back(boost::add_vertex({ vertexLabel }, graph));
	}

	for (int i = 0; i < numRows; i++)
	{
		for (int j = 0; j < numCol; j++)
		{
			string argument = matrix[i][j];
			char stateS = argument[1];
			char stateY = argument[3];
			int paramS = (int)stateS - 48;
			string weights = 'x' + to_string(i) + 'y' + stateY;
			boost::add_edge(vertices[j], vertices[paramS], { weights }, graph);
		}
	}

	boost::dynamic_properties dp;
	dp.property("label", boost::get(&VertexProps::label, graph));
	dp.property("label", boost::get(&EdgeProps::label, graph));
	dp.property("node_id", boost::get(boost::vertex_index, graph));

	write_graphviz_dp(fout, graph, dp);
}

void MealyIntoMoore(ifstream &fileInput, int nX, int nY, int nC)
{
	int numberArguments = nX * nC;
	vector<string> inputMealyMatrix;
	inputMealyMatrix.resize(numberArguments);
	vector<vector<ZParameter>> inputMealy;
	inputMealy.resize(numberArguments);
	int k = 0;
	for (int i = 0; i < nX; i++)
	{
		for (int j = 0; j < nC; j++)
		{
			inputMealy[i].resize(numberArguments);
			string newMealyArgument;
			fileInput >> newMealyArgument;
			inputMealyMatrix[k] = newMealyArgument;
			k++;
			char stateS = newMealyArgument[1];
			int paramS = (int)stateS - 48;
			char stateY = newMealyArgument[3];
			int paramY = (int)stateY - 48;
			inputMealy[i][j].S = paramS;
			inputMealy[i][j].Y = paramY;
		}
	}

	sort(inputMealyMatrix.begin(), inputMealyMatrix.end());
	inputMealyMatrix.erase(unique(inputMealyMatrix.begin(), inputMealyMatrix.end()), inputMealyMatrix.end());
	size_t sizeZVector = inputMealyMatrix.size();

	vector<ZParameter> zParam;
	zParam.resize(numberArguments);
	for (size_t i = 0; i < sizeZVector; i++)
	{
		string mealyArg = inputMealyMatrix[i];
		char stateS = mealyArg[1];
		int zParamS = (int)stateS - 48;
		char stateY = mealyArg[3];
		int zParamY = (int)stateY - 48;
		zParam[i].S = zParamS;
		zParam[i].Y = zParamY;
	}

	vector<vector<string>> resultMatrix;
	resultMatrix.resize(numberArguments);

	for (size_t i = 0; i < sizeZVector; i++)
	{
		resultMatrix[0].resize(numberArguments);
		resultMatrix[0][i] = 'y' + to_string(zParam[i].Y);
	}

	for (int i = 1; i <= nX; i++)
	{
		for (size_t j = 0; j < sizeZVector; j++)
		{
			resultMatrix[i].resize(numberArguments);
			for (size_t k = 0; k < zParam.size(); k++)
			{
				if ((inputMealy[i - 1][zParam[j].S].S == zParam[k].S) && (inputMealy[i - 1][zParam[j].S].Y == zParam[k].Y))
				{
					resultMatrix[i][j] = 'z' + to_string(k);
					break;
				}
			}
		}
	}

	PrintMatrixInFile(resultMatrix, nX + 1, sizeZVector);
	WriteGraphMoore(resultMatrix, nX + 1, sizeZVector);
}

void MooreIntoMealy(ifstream &fileInput, int nX, int nY, int nC)
{
	vector<string> stateY;
	stateY.resize(MAX_NUM);
	vector<vector<string>> outputMealyMatrix;
	outputMealyMatrix.resize(MAX_NUM);

	for (int j = 0; j < nC; j++)
	{
		string newY;
		fileInput >> newY;
		stateY[j] = newY;
	}

	for (int i = 0; i < nX; i++)
	{
		for (int j = 0; j < nC; j++)
		{
			outputMealyMatrix[i].resize(MAX_NUM);
			string newQ;
			fileInput >> newQ;
			char state = newQ[1];
			int numState = (int)state - 48;
			outputMealyMatrix[i][j] = newQ + stateY[numState];
		}
	}

	PrintMatrixInFile(outputMealyMatrix, nX, nC);
	WriteGraphMealy(outputMealyMatrix, nX, nC);
}

bool CheckNum(int num)
{
	if ((num <= MIN_NUM) || (num >= MAX_NUM))
	{
		cout << "Argument out of range!" << endl;
		return false;
	}
	return true;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "Arguments error!" << endl;
		return 1;
	}
	string fileName = argv[1];
	ifstream fileInput(fileName);
	if (!fileInput.is_open())
	{
		cout << "The file is not open!" << endl;
		return 1;
	}

	int numX;
	int numY;
	int numConditions;
	fileInput >> numX >> numY >> numConditions;

	if (!CheckNum(numX) || !CheckNum(numY) || !CheckNum(numConditions))
	{
		return 1;
	}

	string type;
	fileInput >> type;

	if (type == TYPE_MEALY)
	{
		MealyIntoMoore(fileInput, numX, numY, numConditions);
	}
	else if (type == TYPE_MOORE)
	{
		MooreIntoMealy(fileInput, numX, numY, numConditions);
	}
	else
	{
		cout << "Input type is not" << endl;
		return 1;
	}

	return 0;
}
