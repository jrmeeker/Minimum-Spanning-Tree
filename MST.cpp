#include <iostream>
#include <numeric>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iterator>
#include <algorithm>
#include <fstream>
#include <string>

using namespace std;

class Graph
{
private:
	int size, rando, maxWeight, index;
	int **gArray;
	int **wgArray;
	int currentRow, sumEdges = 0;
	float gen, prob, density;
	string line;
	vector <int> sourceRow, MST, Weights, closedSet, values;
	vector < vector <int> > openSet, graph, weightedGraph;

public:

	Graph(string file) //Default constructor for calling Graph class
	{
		//READ NEEDED DATA FROM FILE, STARTING WITH SIZE//
		ifstream infile(file); // Create input stream for reading file
		istream_iterator <int> start(infile), end; // Create i stream iterator to read file
		vector <int> fromFile(start, end); // Read file contents into fromFile vector
		
		this->size = fromFile[0]; //Set size of adjacency matrix to the size passed in from file - 20 in the case of the example
		graph.resize(size); //Resize the graph & weighted graph to the same size
		weightedGraph.resize(size); 
		values.resize(3); 

		index = 1; // We will start reading from the fromFile vector at index 1

		for (int i = 0; i < size; ++i)
		{
			graph[i] = vector <int> (size); //Set up graph vectors
			weightedGraph[i] = vector <int> (size);
		}

		for (int i = 0; i < size; ++i) //This nested loop will set up an adjacency matrix representing a random graph
		{
			for (int j = 0; j < size; ++j)
			{
				graph[i][j] = graph[j][i] = false; //Create a zero by default
				weightedGraph[i][j] = weightedGraph[j][i] = false;
				
				if (i == j) //Create zeros along diagonal
				{
					graph[i][j] = graph[j][i] = false; 
					weightedGraph[i][j] = weightedGraph[j][i] = false;
				}

				else
				{
					for (int x = 0; x < 3; ++x)
					{
						values[x] = fromFile[index]; //Read in the 3 values for starting node, ending node, and cost
						index++;
					};
					
					if (values[1] == j) // If there is a connection between the current node and the node read in:
					{
						graph[i][j] = graph[j][i] = 1; // Set the adjacency matrix value to 1
						weightedGraph[i][j] = weightedGraph[j][i] = values[2]; //Set the weighted graph's value to cost
					}
					else //If there is no connection a zero has already been placed in by default and we will read in the values again on the next pass
					{
						index -= 3; //Reset the index to recapture the current node
					}
				}
			}
		}
	};

	void addToOpenSet(vector <int> node) //Created this function just so when it's called later its purpose is more clear
	{
		openSet.push_back(node);
	};

	void analyzeEdges(vector <int> sourceRow) //This function analyzes the possible paths from the current node.
	{
		vector <int> adjNode(2);
		for (int i = 0; i < size; ++i) //This loop will find the non-zero edges and add the corresponding node/edge pairs to the open set
		{
			if (sourceRow[i] != 0)
			{
				adjNode = { i, sourceRow[i] }; //Create node/edge pair
				if (find(closedSet.begin(), closedSet.end(), i) == closedSet.end())//If i is not in closed set:
				{
					if (find(openSet.begin(), openSet.end(), adjNode) == openSet.end())//If adjNode is not in open set:
						addToOpenSet(adjNode); //Add node/edge pair to open set
				}
			}
		}
	};

	vector <int> rowToVector(int row) //This function takes a given row from the adjacency matrix and converts it into a vector
	{
		sourceRow.resize(size); //Allocate memory for populating the row which represents the current node's possible edges
		for (int i = 0; i < size; ++i)
		{
			sourceRow[i] = weightedGraph[row][i]; // Loop generates a vector containing the values of the first row of the adjacency matrix
		}
		return sourceRow;
	};

	void selectNode(vector <vector <int>> openSet)
	{
		vector <int> edgeCosts(openSet.size()); //Convert the current open set to a vector 
		for (int i = 0; i < openSet.size(); ++i)
		{
			edgeCosts[i] = openSet[i][1]; //Create a vector to keep track of the edge costs to each node in the open set
		};
		if (openSet.size() != 0) //If open set is empty, there are no possible edges and the code below will crash
		{
			int indexOfLeast = distance(edgeCosts.begin(), min_element(edgeCosts.begin(), edgeCosts.end())); // Returns the index of the smallest edge cost
			vector <int> nextNode = openSet[indexOfLeast]; //Choose the next node out of the open set, located at the index of the smallest next edge cost

			MST.push_back(nextNode[0]); //Add node to MST and therefore the closed set
			Weights.push_back(nextNode[1]); //Add edge cost to the list of weights comprising the MST
			openSet.erase(openSet.begin() + indexOfLeast); //Remove current node from open set
			this->openSet = openSet; //Update global open set with the current removed
			currentRow = nextNode[0]; //Update value of current row to represent traveling to the next node
		}
	};

	bool isConnected() //This function will check if the graph is connected. It will detect any grouping of nodes that are an island.
	{
		bool connected = 1; //Assume graph is connected. If algorithm detects graph is disconnected the value of connected will be set to zero.
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				sumEdges += graph[i][j]; //Add up all of the non-weighted edges in the graph. These will show up as 1's in the adjacency matrix.

			}
		};
		sumEdges = sumEdges / 2; //Each edge is counted twice in the adjacency matrix, so we divide by 2 to find the actual number of edges

		if (sumEdges >= size - 1) //For n bodes, if there are less than n-1 edges the graph is disconnected and we can set connected equal to 0
		{
			sourceRow.resize(size); //Allocate memory for sourceRow
			for (int i = 0; i < size; ++i)
			{
				for (int j = 0; j < size; ++j)
				{
					sourceRow[j] = graph[i][j];
				}
				if (accumulate(sourceRow.begin(), sourceRow.end(), 0) == 0) //If there are no edges, the sum will be zero and the node is an island.
				{
					connected = 0; //Set the value of connected to zero
					break; //Break out of isConnected() function
				}
			};
		}
		else
		{
			connected = 0;
		}
		return connected;
	};


	void runPrim()
	{
		bool connected = isConnected(); //Checks if graph is connected
		if (connected) //If the graph is connected, we will run Prim. If not, exit and tell user graph is not connected.
		{
			MST.push_back(0); //Add node zero to the minimum spanning tree
			while (closedSet.size() < size) //Continue algorithm until all nodes are accounted for
			{
				closedSet.push_back(currentRow); //Add current node to closed set. Starts at 0.
				sourceRow = rowToVector(currentRow);//Convert current row in matrix to a vector containg the adjacent edge costs to next nodes
				analyzeEdges(sourceRow); //Add adjacent edges to open set
				selectNode(openSet); //Remove least cost next node from open set and add to closed set
			}

			if (MST.size() >= size) //If the MST vector has less elements than nodes, the algorithm did not complete and the graph is disconnected
			{
				cout << "\n\n" << endl;
				cout << "Minimum spanning tree: ";
				for (int i = 0; i < size; ++i)
				{
					cout << closedSet[i];
					if (i < size - 1)
					{
						cout << " -> ";
					}
				};
				cout << endl;
				int sum = accumulate(Weights.begin(), Weights.begin() + (size - 1), 0); //Sum up all of edge costs
				cout << "\nTotal cost = " << sum << endl; //Print total cost
			}
			else
			{
				cout << "\nGraph is not connected. No minimum spanning tree exists." << endl;
			}
		}
		else
		{
			cout << "\nGraph is not connected. No minimum spanning tree exists." << endl;
		};
	};

	void printGraph()
	{
		cout << "Adjacency Matrix Representation:\n" << endl;
		for (int x = 0; x < size; ++x)
		{
			for (int y = 0; y < size; ++y)
			{
				cout << graph[x][y];
				if (y == size - 1)
					cout << endl;
			}
		}
	};

	void printWeightedGraph()
	{
		cout << "\nMatrix with weights added to edges:\n" << endl;
		for (int x = 0; x < size; ++x)
		{
			for (int y = 0; y < size; ++y)
			{
				cout << weightedGraph[x][y] << " ";
				if (y == size - 1)
					cout << endl;
			}
		}
	};
};
	

int main()
{
	Graph myGraph("TestData.txt");

	myGraph.printGraph();
	cout << endl;
	myGraph.printWeightedGraph();
	myGraph.runPrim();
	getchar();
}