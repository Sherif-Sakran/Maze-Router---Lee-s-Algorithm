#include <iostream>
#include <queue>
#include <fstream>
#include <sstream>
using namespace std;
const int N = 200;
struct pin {
	int layer;
	int x;
	int y;
};
struct mazeCell {
	int num = -2;
	bool visited = 0;
};

void initializeMaze(mazeCell(&M)[N][N]);
void printMaze(mazeCell M[][N]);
void writeMaze(mazeCell M[][N], string fileName);
vector<pin> leesAlgorithm(mazeCell (&M1)[N][N], mazeCell (&M2)[N][N], pin src, pin end);
vector<pin> trackPath(mazeCell(&M1)[N][N], mazeCell(&M2)[N][N], int start_x, int start_y, int start_layer, int end_x, int end_y, int end_layer);
vector<pin> moveHorizontal(vector<pin> &path, mazeCell(&M)[N][N], int end_x, int end_y, int end_layer);
vector<pin> moveVertical(vector<pin>& path, mazeCell(&M)[N][N], int end_x, int end_y, int end_layer);
vector<pin> connectAnotherPin(mazeCell(&M1)[N][N], mazeCell(&M2)[N][N], vector<pin>& path, pin dest);
void refineMaze(mazeCell(&M)[N][N]);

vector<vector<pin>> readPins(string fileName);
void removeSpaces(string str, string& net);
vector<string> parseCSV(string net);
vector<string> readAllNets(string fileName);


int main() {
	mazeCell M1[N][N];
	mazeCell M2[N][N];

	//initializing the maze with -2 as (empty)
	//-1 will be for obstacles and 0 for the starting point
	initializeMaze(M1);
	initializeMaze(M2);

	ofstream outFileMaze;
	outFileMaze.open("output.txt");


	string fileName = "input.txt";
	vector<vector<pin>> netsVector = readPins(fileName);
	outFileMaze << "Reading pins\n";
	for (int j = 0; j < netsVector.size(); j++)
	{
		outFileMaze << "Net " << j << ":\n";
		for (int i = 0; i < netsVector[j].size(); i++)
			outFileMaze << "(" << netsVector[j][i].layer << ", " << netsVector[j][i].x << ", " << netsVector[j][i].y << ")\n";
		outFileMaze << endl;
	}
	outFileMaze << "Connecting pins\n";
	for (int i = 0; i < netsVector.size(); i++) {
		outFileMaze << "Net " << i << ":\n";
		vector<pin> tempNet = netsVector[i];
		vector<pin> path = leesAlgorithm(M1, M2, tempNet[0], tempNet[1]);
		refineMaze(M1); //remove marks other than -1 and -2
		refineMaze(M2);

		for (int i = 0; i < path.size(); i++)
			outFileMaze << "(" << path[i].layer << ", " << path[i].x << ", " << path[i].y << ")\n";

		for (int j = 2; j < tempNet.size(); j++) {
			outFileMaze << "Connecting another pin...\n";
			vector<pin> path_2 = connectAnotherPin(M1, M2, path, tempNet[j]); //refines the maze on its own
			outFileMaze << "Found shortest path of length : " << path_2.size() << endl;

		for (int i = 0; i < path_2.size(); i++)
			outFileMaze << "(" << path_2[i].layer << ", " << path_2[i].x << ", " << path_2[i].y << ")\n";
		}
		outFileMaze << endl;


	}

	return 0;
}

void initializeMaze(mazeCell(&M)[N][N]){
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			M[i][j].num = -2;
			M[i][j].visited = 0;
		}
	return;
}
void printMaze(mazeCell M[][N]) {
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			cout << M[i][j].num << "\t";
		cout << endl;
	}
}
void writeMaze(mazeCell M[][N], string fileName) {
	ofstream outFileMaze;
	outFileMaze.open(fileName);
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
			outFileMaze << M[i][j].num << "\t";
		outFileMaze << endl;
	}
}

vector<pin> leesAlgorithm(mazeCell (&M1)[N][N], mazeCell (&M2)[N][N], pin start, pin end){
	int dl[] = { -1, 0, 1, 0 };
	int dc[] = { 0, 1, 0, -1 };
	queue<int> X, Y; 
	M1[start.x][start.y].num = 0;
	M1[start.x][start.y].visited = 1;
	M2[start.x][start.y].num = 0;
	M2[start.x][start.y].visited = 1;
	X.push(start.x);
	Y.push(start.y);
	int x, y, xx, yy;
	while (!X.empty())
	{
		x = X.front(); 
		y = Y.front();
		for (int i = 0; i < 4; i++)
		{
			xx = x + dl[i];
			yy = y + dc[i];

			if (xx < N && yy < N && xx>=0 &&yy>=0 && ((!(M1[xx][yy].visited))|| M1[xx][yy].num == -1)&& !(xx == end.x && yy == end.y))
			{
				X.push(xx);
				Y.push(yy);
				M1[xx][yy].num = M1[x][y].num + 1; 
				M1[xx][yy].visited = true;
				M2[xx][yy].num = M1[x][y].num + 1;
				M2[xx][yy].visited = true;

			}
			if (xx < N && yy < N && xx >= 0 && yy >= 0 && xx == end.x && yy == end.y)
			{
				M1[xx][yy].num = 999;
				//M1[xx][yy].num = M1[x][y].num + 1;
				M2[xx][yy].num = M1[x][y].num + 1;
				//M1[xx][yy].visited = true;
				vector<pin> path = trackPath(M1, M2, start.x, start.y, start.layer, xx, yy, end.layer);
				return path;
			}
		}
		X.pop();
		Y.pop();
	}


}
vector<pin> trackPath(mazeCell(&M1)[N][N], mazeCell(&M2)[N][N], int start_x, int start_y, int start_layer, int end_x, int end_y, int end_layer){
	//layer 1 -> vertical.
	//layer 2 -> horizontal.
	vector<pin> path;
	pin src, dest;
	src.layer = start_layer;
	src.x = start_x;
	src.y = start_y;
	dest.layer = end_layer;
	dest.x = end_x;
	dest.y = end_y;
	path.insert(path.begin(), dest);
	bool loopFlag = 1;
	while (loopFlag) {
		if (end_layer == 1)
		{
			moveVertical(path, M1, end_x, end_y, end_layer);
			end_layer = 2; //to move horizontal next
			end_x = path[0].x;
		}
		else
		{
			moveHorizontal(path, M2, end_x, end_y, end_layer);
			end_layer = 1; //to move vertical next
			end_y = path[0].y;
		}
		//via
		if (path[0].layer == src.layer && path[0].x == src.x && path[0].y == src.y)
			loopFlag = 0;
		else if (path.size())
		{
			pin temp = path[0];	
			temp.layer = end_layer;
			path.insert(path.begin(), temp);
		}
		if (path[0].layer == src.layer && path[0].x == src.x && path[0].y == src.y)
			loopFlag = 0;
	};
	return path;
}
vector<pin> moveHorizontal(vector<pin>& path, mazeCell (&M)[N][N], int end_x, int end_y, int end_layer) {
	pin temp;
	int i = end_x;
	int j = end_y;
	//horizontal
	if (M[i][j + 1].num < M[end_x][end_y].num && M[i][j + 1].num != -2 && M[i][j + 1].num != -1)
		//loop to the right
	{
		while (M[i][j + 1].num < M[i][j].num && M[i][j + 1].num != -2 && M[i][j + 1].num != -1)
		{
			temp.layer = end_layer;
			temp.x = i;
			temp.y = j+1;
			path.insert(path.begin(), temp);
			M[i][j].num = -1;
			M[i][j].visited = true;

			j++;
		}
	}

	else
		//loop to the left
	{
		while (M[i][j - 1].num < M[i][j].num && M[i][j - 1].num != -2 && M[i][j - 1].num != -1)
		{
			temp.layer = end_layer;
			temp.x = i;
			temp.y = j-1;
			path.insert(path.begin(), temp);
			M[i][j].num = -1;
			M[i][j].visited = true;
			j--;
		}
	}
	M[i][j].num = -1;
	return path;
}

vector<pin> moveVertical(vector<pin>& path, mazeCell(&M)[N][N], int end_x, int end_y, int end_layer) {
	pin temp;
	int i = end_x;
	int j = end_y;
	//vertical
	if (M[i + 1][j].num < M[end_x][end_y].num && M[i + 1][j].num != -2 && M[i + 1][j].num != -1)
		//loop downwards
	{
		while (M[i + 1][j].num < M[i][j].num && M[i + 1][j].num != -2 && M[i + 1][j].num != -1)
		{
			temp.layer = end_layer;
			temp.x = i+1;
			temp.y = j;
			path.insert(path.begin(), temp);
			M[i][j].num = -1;
			M[i][j].visited = true;
			i++;
		}
	}

	else
		//loop upwards
	{
		while (M[i-1][j].num < M[i][j].num && M[i-1][j].num != -2 && M[i-1][j].num != -1)
		{
			temp.layer = end_layer;
			temp.x = i-1;
			temp.y = j;
			path.insert(path.begin(), temp);
			M[i][j].num = -1;
			M[i][j].visited = true;
			i--;
		}
	}
	M[i][j].num = -1;
	return path;
}
vector<pin> connectAnotherPin(mazeCell(&M1)[N][N], mazeCell(&M2)[N][N], vector<pin>& path, pin dest) {

	vector<vector<pin>> allPaths;
	//cout << "Connecting another pin...\n";
	for (int i = 0; i < path.size(); i++)
	{
		initializeMaze(M1);
		initializeMaze(M2);
		vector<pin> potentialPath = leesAlgorithm(M1, M2, path[i], dest);
		//cout << endl << "potential path's size: " << potentialPath.size() << endl;
		allPaths.push_back(potentialPath);
	}

	int min = 999;
	int minIndex = -1;
	for (int i =0; i< allPaths.size(); i++)
		if (allPaths[i].size() < min)
		{
			min = allPaths[i].size();
			minIndex = i;
		}
	initializeMaze(M1);
	initializeMaze(M2);
	for (int i = 0; i < path.size(); i++) //since we initializated it again, we need to return the obstacles (-1s) back
	{
		if (path[i].layer == 1)
		{
			M1[path[i].x][path[i].y].num = -1;
			M1[path[i].x][path[i].y].visited = true;
		}
		else
		{
			M2[path[i].x][path[i].y].num = -1;
			M2[path[i].x][path[i].y].visited = true;
		}
	}
	vector <pin> minPath = allPaths[minIndex];
	//cout << "Found shortest path with length: " << minPath.size() << endl;
	for (int i = 0; i < minPath.size(); i++) //since we initializated it again, we need to return the obstacles (-1s) back
	{
		if (minPath[i].layer == 1)
		{
			M1[minPath[i].x][minPath[i].y].num = -1;
			M1[minPath[i].x][minPath[i].y].visited = true;
		}
		else
		{
			M2[minPath[i].x][minPath[i].y].num = -1;
			M2[minPath[i].x][minPath[i].y].visited = true;
		}
	}
	//writeMaze(M1, "M1_pin3.txt");
	//writeMaze(M2, "M2_pin3.txt");
	path.insert(path.end(), minPath.begin(), minPath.end());
	return minPath;
}

void refineMaze(mazeCell(&M)[N][N]) {
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
		{
			if (M[i][j].num != -1)
			{
				M[i][j].num = -2;
				M[i][j].visited = 0;
			}
		}
	return;
}


//utility functions to read the pins
vector<vector<pin>> readPins(string fileName) {
	vector<string> netsAsStrings = readAllNets(fileName);

	vector<vector<pin>> netsVector;
	for (int j = 0; j < netsAsStrings.size(); j++) {
		vector<pin> pins;
		vector<string> pinString = parseCSV(netsAsStrings[j]);
		for (int i = 1; i < pinString.size(); i += 3)
		{
			pin temp;
			stringstream pinLayer((pinString[i]));
			pinLayer >> temp.layer;

			stringstream pinX((pinString[i + 1]));
			pinX >> temp.x;

			stringstream pinY((pinString[i + 2]));
			pinY >> temp.y;
			pins.push_back(temp);
		}
		netsVector.push_back(pins);
	}
	return netsVector;
}
vector<string> parseCSV(string net) {
	replace(net.begin(), net.end(), ',', ' ');
	removeSpaces(net, net);
	replace(net.begin(), net.end(), ' ', ',');
	stringstream ss;
	ss.str(net);
	string substr;
	vector<string> result;
	while (ss.good()) {
		getline(ss, substr, ',');
		result.push_back(substr);
	}
	return result;
}

void removeSpaces(string str, string& net) {
	string nstr;
	for (int i = 0; i < str.length(); ) {
		if (str[i] == ' ') {
			if (i == 0 || i == str.length() - 1) {
				i++;
				continue;
			}
			while (str[i + 1] == ' ')
				i++;
		}
		nstr += str[i++];
	}
	net = nstr;
}

vector<string> readAllNets(string fileName) {
	vector<string> nets;
	ifstream myfile(fileName);
	string net;
	if (!myfile.is_open())
		cout << "Unable to open file";
	else
	{
		while (getline(myfile, net))
			nets.push_back(net);
	}
	myfile.close();
	return nets;
}