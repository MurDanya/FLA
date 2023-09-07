#include "api.hpp"
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>

class Node {
public:
	Node(char new_op, bool new_n = false, std::set<int> new_f = {}, std::set<int> new_l = {});
	Node(char new_op, Node* new_left, Node* new_right);

	char get_op();
	void set_left(Node& new_node);
	Node* get_left();
	void set_right(Node& new_node);
	Node* get_right();
	void set_nullable(bool new_n);
	bool get_nullable();
	void set_firstpos(std::set<int>& new_f);
	std::set<int>& get_firstpos();
	void set_lastpos(std::set<int>& new_l);
	std::set<int>& get_lastpos();
private:
	char op;
	Node* left;
	Node* right;
	bool nullable;
	std::set<int> firstpos;
	std::set<int> lastpos;
};

Node::Node(char new_op, bool new_n, std::set<int> new_f, std::set<int> new_l)
	: op(new_op), left(NULL), right(NULL), nullable(new_n), firstpos(new_f), lastpos(new_l) {}

Node::Node(char new_op, Node* new_left, Node* new_right)
	: op(new_op), left(new_left), right(new_right), nullable(false), firstpos({}), lastpos({}) {}

char
Node::get_op() {
	return op;
}

void
Node::set_left(Node& new_node) {
	left = &new_node;
}

Node*
Node::get_left() {
	return left;
}

void
Node::set_right(Node& new_node) {
	right = &new_node;
}

Node*
Node::get_right() {
	return right;
}

void
Node::set_nullable(bool new_n) {
	nullable = new_n;
}

bool
Node::get_nullable() {
	return nullable;
}

void
Node::set_firstpos(std::set<int>& new_f) {
	firstpos = new_f;
}

std::set<int>&
Node::get_firstpos() {
	return firstpos;
}

void
Node::set_lastpos(std::set<int>& new_l) {
	lastpos = new_l;
}

std::set<int>&
Node::get_lastpos() {
	return lastpos;
}


class RegexParser
{
public:
	RegexParser();
	~RegexParser();
	Node* parse(const std::string& s);
	std::vector<char>& get_indexes();
	void print_tree(Node* tree);
private:
	Node* tree;
	std::string parse_str;
	size_t pos;
	std::vector<char> indexes;
	int index;
	Node* parsing_or();
	Node* parsing_concat();
	Node* parsing_iteration();
	Node* parsing_sym();
	void delete_tree(Node*);
};

RegexParser::RegexParser() : tree(NULL), parse_str(), pos(0), indexes(), index(0) {}

RegexParser::~RegexParser() {
	delete_tree(tree);
}

void
RegexParser::delete_tree(Node* tree) {
	if (tree != NULL) {
		delete_tree(tree->get_left());
		delete_tree(tree->get_right());
		delete tree;
	}
}

std::vector<char>&
RegexParser::get_indexes() {
	return indexes;
}

Node*
RegexParser::parse(const std::string& s)
{
	parse_str = '(' + s + ')' + '#';
	pos = 0;
	indexes.clear();
	index = 0;
	tree = parsing_or();
	return tree;
}

Node*
RegexParser::parsing_or()
{
	Node* left, * right;
	left = parsing_concat();
	while (parse_str[pos] == '|') {
		++pos;
		right = parsing_concat();
		left = new Node('|', left, right);
	}
	return left;
}

Node*
RegexParser::parsing_concat()
{
	Node* left, * right;
	if (std::isalpha(parse_str[pos]) || std::isdigit(parse_str[pos]) || parse_str[pos] == '#'
		|| parse_str[pos] == '(') {
		left = parsing_iteration();
		while (std::isalpha(parse_str[pos]) || std::isdigit(parse_str[pos])
			|| parse_str[pos] == '#' || parse_str[pos] == '(') {
			right = parsing_iteration();
			left = new Node('&', left, right);
		}
	}
	else {
		left = new Node('+', true);
	}
	return left;
}

Node*
RegexParser::parsing_iteration()
{
	Node* it_node;
	it_node = parsing_sym();
	if (parse_str[pos] == '*') {
		++pos;
		it_node = new Node('*', it_node, NULL);
	}
	return it_node;
}

Node*
RegexParser::parsing_sym()
{
	Node* sym_node;
	if (parse_str[pos] == '(') {
		++pos;
		sym_node = parsing_or();
		++pos;
	}
	else {
		sym_node = new Node(parse_str[pos], false, std::set<int> {index}, std::set<int> {index});
		indexes.push_back(parse_str[pos]);
		++pos;
		++index;
	}
	return sym_node;
}

void
RegexParser::print_tree(Node* tree) {
	if (tree != NULL) {
		std::cout << tree->get_op() << " ( ";
		print_tree(tree->get_left());
		std::cout << " , ";
		print_tree(tree->get_right());
		std::cout << " ) ";
	}
}

void
set_first_last(Node* tree) {
	Node* left, * right;
	std::set<int> left_set, right_set;
	if (tree->get_op() == '|') {
		left = tree->get_left();
		right = tree->get_right();
		set_first_last(left);
		set_first_last(right);

		tree->set_nullable(left->get_nullable() || right->get_nullable());

		left_set = left->get_firstpos();
		right_set = right->get_firstpos();
		left_set.insert(right_set.begin(), right_set.end());
		tree->set_firstpos(left_set);

		left_set = left->get_lastpos();
		right_set = right->get_lastpos();
		left_set.insert(right_set.begin(), right_set.end());
		tree->set_lastpos(left_set);
	}
	else if (tree->get_op() == '&') {
		left = tree->get_left();
		right = tree->get_right();
		set_first_last(left);
		set_first_last(right);

		tree->set_nullable(left->get_nullable() && right->get_nullable());

		left_set = left->get_firstpos();
		right_set = right->get_firstpos();
		if (left->get_nullable()) {
			left_set.insert(right_set.begin(), right_set.end());
		}
		tree->set_firstpos(left_set);

		left_set = left->get_lastpos();
		right_set = right->get_lastpos();
		if (right->get_nullable()) {
			right_set.insert(left_set.begin(), left_set.end());
		}
		tree->set_lastpos(right_set);
	}
	else if (tree->get_op() == '*') {
		left = tree->get_left();
		set_first_last(left);

		tree->set_nullable(true);

		tree->set_firstpos(left->get_firstpos());

		tree->set_lastpos(left->get_lastpos());
	}
}

void
set_follow(Node* tree, std::vector<std::set<int>>& follow_pos) {
	Node* left, * right;
	std::set<int> set_firstpos;
	if (tree->get_op() == '&') {
		left = tree->get_left();
		right = tree->get_right();

		set_follow(left, follow_pos);
		set_follow(right, follow_pos);

		set_firstpos = right->get_firstpos();
		for (int i : left->get_lastpos()) {
			follow_pos[i].insert(set_firstpos.begin(), set_firstpos.end());
		}
	}
	else if (tree->get_op() == '*') {
		left = tree->get_left();

		set_follow(left, follow_pos);

		set_firstpos = left->get_firstpos();
		for (int i : left->get_lastpos()) {
			follow_pos[i].insert(set_firstpos.begin(), set_firstpos.end());
		}
	}
	else if (tree->get_op() == '|') {
		set_follow(tree->get_left(), follow_pos);
		set_follow(tree->get_right(), follow_pos);
	}
}

std::string
set_to_str(std::set<int>& set_pos) {
	std::string str_pos = "";
	for (int num : set_pos) {
		if (str_pos.length() == 0) {
			str_pos = std::to_string(num);
		}
		else {
			str_pos = str_pos + ',' + std::to_string(num);
		}
	}
	return str_pos;
}

std::set<int>
str_to_set(const std::string& str) {
	std::set<int> nums = {};
	int i;
	std::stringstream ss(str);
	while (ss >> i) {
		nums.insert(i);
		if (ss.peek() == ',') {
			ss.ignore();
		}
	}
	return nums;
}

DFA re2dfa(const std::string& s) {
	RegexParser parser;
	Node* tree = parser.parse(s);
	std::vector<char> symbols_indexes = parser.get_indexes();
	std::vector<std::set<int>> follow_positions;
	follow_positions.resize(symbols_indexes.size());
	set_first_last(tree);
	set_follow(tree, follow_positions);
	std::vector<std::string> states;
	std::map<char, std::set<int>> transitions;
	std::string cur_state = set_to_str(tree->get_firstpos());
	states.push_back(cur_state);
	DFA res = DFA(Alphabet(s.length() == 0 ? "a" : s));
	res.create_state("0");
	if ((tree->get_firstpos()).find(symbols_indexes.size() - 1) != (tree->get_firstpos()).end()) {
		res.make_final("0");
	}
	res.set_initial("0");
	for (int i = 0; i < states.size(); ++i) {
		cur_state = states[i];
		transitions.clear();
		for (int idx : str_to_set(cur_state)) {
			if (transitions.find(symbols_indexes[idx]) == transitions.end()) {
				transitions[symbols_indexes[idx]] = {};
			}
			transitions[symbols_indexes[idx]].insert(follow_positions[idx].begin(), follow_positions[idx].end());
		}
		for (auto trans : transitions) {
			if (trans.second.empty()) {
				continue;
			}
			std::string trans_state = set_to_str(trans.second);
			auto it_trans_state = std::find(states.begin(), states.end(), trans_state);
			std::string idx_trans_state = std::to_string(std::distance(states.begin(), it_trans_state));
			if (it_trans_state == states.end()) {
				states.push_back(trans_state);
				res.create_state(idx_trans_state);
				if (trans.second.find(symbols_indexes.size() - 1) != trans.second.end()) {
					res.make_final(idx_trans_state);
				}
			}
			res.set_trans(std::to_string(i), trans.first, idx_trans_state);
		}
	}

	return res;
}
