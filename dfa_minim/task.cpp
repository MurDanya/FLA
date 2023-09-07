#include "api.hpp"
#include <string>
#include <vector>
#include <map>
#include <iostream>

void
delete_unreacheble_states(DFA& d) {
	Alphabet alp(d.get_alphabet());
	std::set<std::string> states = d.get_states();
	std::vector<std::string> reach_states;
	reach_states.push_back(d.get_initial_state());
	int idx = 0;
	while (idx < reach_states.size()) {
		for (auto ch : alp) {
			if (d.has_trans(reach_states[idx], ch) && std::find(reach_states.begin(), reach_states.end(), d.get_trans(reach_states[idx], ch)) == reach_states.end()) {
				reach_states.push_back(d.get_trans(reach_states[idx], ch));
			}
		}
		++idx;
	}
	for (auto st : states) {
		if (std::find(reach_states.begin(), reach_states.end(), st) == reach_states.end()) {
			d.delete_state(st);
		}
	}
}

int
get_new_int(std::set<std::string>& states) {
	int ans = 0;
	bool flag;
	while (1) {
		flag = true;
		for (auto st : states) {
			if (st.find(std::to_string(ans)) != std::string::npos) {
				flag = false;
				break;
			}
		}
		if (flag) {
			break;
		}
		++ans;
	}
	return ans;
}

int
addition_trans(DFA& d) {
	Alphabet alp(d.get_alphabet());
	std::set<std::string> states = d.get_states();
	int new_int = get_new_int(states);
	std::string new_st = std::to_string(new_int);
	d.create_state(new_st);
	states.emplace(new_st);
	for (auto st : states) {
		for (auto ch : alp) {
			if (!d.has_trans(st, ch)) {
				d.set_trans(st, ch, new_st);
			}
		}
	}
	return new_int;
}

std::string
set_to_str(std::set<std::string>& set_st) {
	std::string ans = "";
	for (auto st : set_st) {
		ans += st;
	}
	return ans;
}

std::string
get_to_st(std::vector<std::set<std::string>>& classes, std::string st) {
	for (auto set_st : classes) {
		if (set_st.find(st) != set_st.end()) {
			return set_to_str(set_st);
		}
	}
	return "";
}

DFA dfa_minim(DFA &d) {
	delete_unreacheble_states(d);
	int new_int = addition_trans(d);

	Alphabet alp(d.get_alphabet());
	std::set<std::string> states = d.get_states();
	std::vector<std::set<std::string>> classes;
	classes.push_back({});
	classes.push_back({});
	for (auto st : states) {
		if (d.is_final(st)) {
			classes[0].emplace(st);
		}
		else {
			classes[1].emplace(st);
		}
	}
	if (classes[1].size() == 0) {
		classes.pop_back();
	}
	std::vector<std::set<std::string>> queue = classes;
	std::set<std::string> class_1, class_2;
	int idx = 0, idx_classes = 0;
	while (idx < queue.size()) {
		if (std::find(classes.begin(), classes.end(), queue[idx]) != classes.end()) {
			for (char ch : alp) {
				idx_classes = 0;
				for (auto class_0 : classes) {
					class_1.clear();
					class_2 = class_0;
					for (auto st : class_0) {
						if (d.has_trans(st, ch) && queue[idx].find(d.get_trans(st, ch)) != queue[idx].end()) {
							class_1.emplace(st);
							class_2.erase(st);
						}
					}
					if (class_1.size() != 0 && class_2.size() != 0) {
						classes[idx_classes] = class_1;
						classes.push_back(class_2);
						queue.push_back(class_1);
						queue.push_back(class_2);
					}
					++idx_classes;
				}
			}
		}
		++idx;
	}


	DFA min_d(alp);
	std::string st;
	std::string one_of_st;
	for (auto set_st : classes) {
		st = set_to_str(set_st);
		one_of_st = *(set_st.begin());
		min_d.create_state(st, d.is_final(one_of_st));
		for (auto tmp_st : set_st) {
			if (d.is_initial(tmp_st)) {
				min_d.set_initial(st);
			}
		}
	}
	std::string new_st = std::to_string(new_int);
	for (auto set_st : classes) {
		st = set_to_str(set_st);
		for (auto tmp_st : set_st) {
			if (tmp_st == new_st) {
				new_st = st;
			}
		}
		one_of_st = *(set_st.begin());
		for (char ch : alp) {
			if (d.has_trans(one_of_st, ch)) {
				min_d.set_trans(st, ch, get_to_st(classes, d.get_trans(one_of_st, ch)));
			}
		}
	}
	min_d.delete_state(new_st);
	
	return min_d;
}
