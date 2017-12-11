#include<iostream>
#include<string>
#include <algorithm>
#include <fstream>
#include <stack>
#include "ExpTree.h"
using namespace std;

int wirecnt = 1;
int namecnt = 1;

struct node;
int priority(char);
node* makeNode(char, bool inverted = false);
void attachOperator(StackType<node*>&, StackType<node*>&);
string traverse(node* TreeNode, string src, string drain, string network, int l, float& n, float& p,bool mode);
void DeMorgan(node* TreeNode);
void size(node*TreeNode, float& s);
void inverters(string Expr, ofstream& Write);

bool Validate(string Expr);

struct node {
	char Gate;
	node* left;
	node* right;
	float w;
	bool inverted = false;
};

int main() {

	string expr;
	StackType<char> input;
	StackType<node*> operators;
	StackType<node*> treenodes;
	char temp, again = ' ';
	char inversion = char(39);
	string srcinit = "VDD";
	string draininit = "Out";
	float n, p;
	int l;
	bool mode;
	ofstream Write;
	string Out;
	Write.open("CMOS Circuit.txt");
	Write.clear();

	
	
	cout << "Please Enter a Boolean Expression in the InFix Notation(With no Spaces): \nY = ";
	cin >> expr;

		while (!Validate(expr))
		{
			cout << "Wrong Expression\n Please Re-enter Correct Boolean Expression : \nY = ";
			cin >> expr;
		}

		cout << "Please Enter The Mode of Operation You'd like : \n0: Simple Mode\n1: Transistor Scaling Mode\n";
		cin >> mode;

		if (mode)
		{
			cout << "Please Enter n : ";
			cin >> n;

			cout << "Please Enter p : ";
			cin >> p;

			cout << "Please Enter L : ";
			cin >> l;
		}
		else
			n = p = l = 1;

		for (int i = 0; i<expr.length(); i++) {
			input.Push(expr[i]);
		}

		while (!input.IsEmpty()) {
			temp = input.Top();
			input.Pop();

			if (isalpha(temp))
				treenodes.Push(makeNode(temp));

			if (temp == inversion)
			{
				temp = input.Top();
				input.Pop();
				treenodes.Push(makeNode(temp, true));

			}
			if (temp == ')')
				operators.Push(makeNode(temp));
			if ((temp == '&') || (temp == '|')) {
				bool pushed = false;
				while (!pushed) {
					if (operators.IsEmpty()) {
						operators.Push(makeNode(temp));
						pushed = true;
					}
					else if (operators.Top()->Gate == ')') {
						operators.Push(makeNode(temp));
						pushed = true;
					}
					else if ((priority(temp)>priority(operators.Top()->Gate)) || (priority(temp) == priority(operators.Top()->Gate))) {
						operators.Push(makeNode(temp));
						pushed = true;
					}
					else {
						attachOperator(treenodes, operators);
					}
				}
			}
			if (temp == '(') {
				while (operators.Top()->Gate != ')')
				{
					attachOperator(treenodes, operators);
				}
				operators.Pop();
			}
		}
		while (!operators.IsEmpty()) {
			attachOperator(treenodes, operators);
		}
		Write << "Inverters Used : \n";
		inverters(expr,Write);

		cout << "\n";
		size(treenodes.Top(), p);

		Out = traverse(treenodes.Top(), srcinit, draininit, "PUN", l, n, p,mode);
		Write << "Pull Up Network : \n" << Out;
		cout << "Pull Up Network is :\n" <<  Out << endl;
		DeMorgan(treenodes.Top());
		size(treenodes.Top(), n);

		srcinit = "Out";
		draininit = "GND";
		Out = traverse(treenodes.Top(), srcinit, draininit, "PDN", l, n, p,mode);
		Write << "Pull Down Network : \n" << Out;
		cout << "Pull Down Network is :\n" << Out << endl;
		
	Write.close();
	system("pause");
	return 0;
}

bool Validate(string expr)
{
	stack<char>  Symbols;
	int cntOperands = 0;
	int cntNeg = 0;
	int cntOperators = 0;
	if ((expr.find("Y") != -1) || (expr.find("y") != -1))
		return false;
	if (expr.find(" ") != -1)
		return false;
	
	for (int i = 0; i < expr.length(); i++)
	{
		if (isalnum(expr[i]))
			cntOperands++;
		else if ((expr[i] == '&') || (expr[i] == '|'))
			cntOperators++;
		else if (expr[i] == '\'')
			cntNeg++;
	}
	if (cntOperators != cntOperands - 1)
		return false;
	if (cntNeg > cntOperands)
		return false;

	for (int i = 0; i<expr.length(); i++)
	{
		if (expr[i] == '(')
			Symbols.push(expr[i]);
		else if (expr[i] == ')')
		{
			if (Symbols.empty())
				return false;
			else
				Symbols.pop();
		}
	}

	if (!Symbols.empty())
		return false;
	else
		return true;
		
}
int priority(char op) {
	if (op == '|')
		return 1;
	if (op == '&')
		return 2;
}
node* makeNode(char Gate, bool inverted) {
	node* childnode;
	childnode = new node;
	childnode->Gate = Gate;
	childnode->left = NULL;
	childnode->right = NULL;
	childnode->inverted = inverted;
	return childnode;
}
void attachOperator(StackType<node*>& treenodes, StackType<node*>& operators) {
	node* operatornode = operators.Top();
	operators.Pop();
	operatornode->left = treenodes.Top();
	treenodes.Pop();
	operatornode->right = treenodes.Top();
	treenodes.Pop();
	treenodes.Push(operatornode);
}
string traverse(node* TreeNode, string src, string drain, string network, int l, float& n, float& p, bool mode)
{
	string temp;
	string wire = "Node";

	if (TreeNode->Gate == '&')
	{
		string w = wire + to_string(wirecnt);
		wirecnt++;
		temp = traverse(TreeNode->left, src, w, network, l, n, p, mode);
		temp += traverse(TreeNode->right, w, drain, network, l, n, p, mode);
		return temp;
	}
	else if (TreeNode->Gate == '|')
	{
		temp = traverse(TreeNode->left, src, drain, network, l, n, p, mode);
		temp += traverse(TreeNode->right, src, drain, network, l, n, p, mode);
		return temp;
	}
	else
	{
		//Mname drain gate source body type[W = x L = y]

		if (network == "PUN")
		{
			if (mode)
				return "M" + to_string(namecnt++) + " " + drain + " " + TreeNode->Gate + ((!TreeNode->inverted) ? "_inv" : "") + " " + src + " " + src + " PMOS W=" + to_string(TreeNode->w) + " L=" + to_string(l) + "\n";
			else
				return "M" + to_string(namecnt++) + " " + drain + " " + TreeNode->Gate + ((!TreeNode->inverted) ? "_inv" : "") + " " + src + " " + src + " PMOS\n";
		}
		else
		{
			if(mode)
				return "M" + to_string(namecnt++) + " " + src + " " + TreeNode->Gate + ((!TreeNode->inverted) ? "_inv" : "") + " " + drain + " " + drain + " NMOS W=" + to_string(TreeNode->w) + " L=" + to_string(l) + "\n";
			else
				return "M" + to_string(namecnt++) + " " + src + " " + TreeNode->Gate + ((!TreeNode->inverted) ? "_inv" : "") + " " + drain + " " + drain + " NMOS\n";

		}
	}

}
float height(node* TreeNode)
{
	if (isalpha(TreeNode->Gate)) return 1;
	if (TreeNode->Gate == '&')
		return (height(TreeNode->left) + height(TreeNode->right));
	if (TreeNode->Gate == '|')
		return max(height(TreeNode->left), height(TreeNode->right));
	else
		return 0;
}
void size(node*TreeNode, float& s) {
	if (TreeNode->Gate == '|')
	{
		size(TreeNode->left, s);
		size(TreeNode->right, s);
		return;
	}
	else if (TreeNode->Gate == '&')
	{
		float left = height(TreeNode->left);
		float right = height(TreeNode->right);
		float sum = left + right;
		float SendL = (sum*s) / left;
		float SendR = (sum*s) / right;
		size(TreeNode->left, SendL);
		size(TreeNode->right, SendR);
	}
	else if (isalpha(TreeNode->Gate))
	{
		TreeNode->w = s;
		return;
	}
}
void DeMorgan(node* TreeNode)
{
	if (TreeNode == NULL) return;
	if (TreeNode->Gate == '&') TreeNode->Gate = '|';
	else if (TreeNode->Gate == '|') TreeNode->Gate = '&';
	DeMorgan(TreeNode->left);
	DeMorgan(TreeNode->right);
	return;

}
void inverters(string Expr, ofstream& Write)
{

	for (int i = 0; i < Expr.size(); i++)
	{
		if (isalpha(Expr[i]))
			if (Expr[i + 1] != std::string::npos)
			{
				if (Expr[i + 1] != char(39))
				{
					Write << "Minv" << Expr[i] << "1 " << Expr[i] << "_inv " << Expr[i] << " VDD VDD " << "PMOS" << endl;
					Write << "Minv" << Expr[i] << "2 " << Expr[i] << "_inv " << Expr[i] << " GND GND " << "NMOS" << endl;

					cout << "Minv" << Expr[i] << "1 " << Expr[i] << "_inv " << Expr[i] << " VDD VDD " << "PMOS" << endl;
					cout << "Minv" << Expr[i] << "2 " << Expr[i] << "_inv " << Expr[i] << " GND GND " << "NMOS" << endl;
				}
				else if (Expr[i + 1] == char(39))
				{
					Write << "Minv" << Expr[i] << "1 " << Expr[i] << " " << Expr[i] << "_inv " << " VDD VDD " << " PMOS" << endl;
					Write << "Minv" << Expr[i] << "2 " << Expr[i] << " " << Expr[i] << "_inv GND GND " << "NMOS" << endl;
					cout << "Minv" << Expr[i] << "1 " << Expr[i] << " " << Expr[i] << "_inv " << " VDD VDD " << " PMOS" << endl;
					cout << "Minv" << Expr[i] << "2 " << Expr[i] << " " << Expr[i] << "_inv GND GND " << "NMOS" << endl;
				}
			}
	}

}