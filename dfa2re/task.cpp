#include "api.hpp"
#include <string>
#include <map>
#include <vector>
#include <iostream>

typedef std::map<int, std::map<int, std::string>> STATES;

void
update_DFA(DFA& d, STATES& update_states, std::vector<int>& final_states) {
	Alphabet alp = d.get_alphabet();
	std::set<std::string> all_states = d.get_states();
	std::vector<std::string> all_st;
	all_st.resize(all_states.size());

	int idx_state, counter_states = 1;
	for (auto st : all_states) {
		if (d.is_initial(st)) {
			idx_state = 0;
			update_states[0] = {};
			all_st[0] = st;
		}
		else {
			idx_state = counter_states;
			update_states[counter_states] = {};
			all_st[counter_states] = st;
			++counter_states;
		}
		if (d.is_final(st)) {
			final_states.push_back(idx_state);
		}
	}

	int num_st = 0, num_to_st = 0;
	std::string to_st;
	for (auto &st : all_st) {
		for (auto ch : alp) {
			if (d.has_trans(st, ch)) {
				to_st = d.get_trans(st, ch);
				num_to_st = std::distance(all_st.begin(), std::find(all_st.begin(), all_st.end(), to_st));
				if (update_states[num_st].find(num_to_st) != update_states[num_st].end()) {
					update_states[num_st][num_to_st] += '|';
					update_states[num_st][num_to_st] += ch;
				}
				else {
					std::string s(1, ch);
					update_states[num_st].emplace(num_to_st, s);
				}
			}
		}
		++num_st;
	}
	int new_final = d.size();
	for (auto st : final_states) {
		update_states[st].emplace(new_final, "");
	}
}

std::string
rv_concat(std::string a, std::string b, std::string c) {
	return "(" + a + ")(" + b + ")*(" + c + ")";
}

std::string
rv_or(std::string a, std::string b) {
	return a + "|" + b;
}


void
state_exclusion(int st, STATES& update_states) {
	std::string between = "";
	if (update_states[st].find(st) != update_states[st].end()) {
		between = update_states[st][st];
		update_states[st].erase(st);
	}
	std::string by;
	for (auto& it_cur_st : update_states) {
		if (it_cur_st.first != st) {
			if (update_states[it_cur_st.first].find(st) != update_states[it_cur_st.first].end()) {
				for (auto& it_to_st : update_states[st]) {
					by = rv_concat(update_states[it_cur_st.first][st], between, it_to_st.second);
					if (update_states[it_cur_st.first].find(it_to_st.first) != update_states[it_cur_st.first].end()) {
						by = rv_or(update_states[it_cur_st.first][it_to_st.first], by);
					}
					update_states[it_cur_st.first][it_to_st.first] = by;
				}
				update_states[it_cur_st.first].erase(st);
			}
		}
	}
	update_states.erase(st);
	return;
}

void
print_states(STATES& update_states) {
	for (auto& st : update_states) {
		std::cout << st.first << " : " << std::endl;
		for (auto& to_st : st.second) {
			std::cout << "    " << to_st.first << " : ";
			for (auto& trans : to_st.second) {
				std::cout << trans << ' ';
			}
			std::cout << std::endl;
		}
	}
}


std::string dfa2re(DFA &d) {
	STATES update_states;
	std::vector<int> final_states;
	update_DFA(d, update_states, final_states);

	int new_final = d.size();
	update_states[new_final] = {};
	for (int i = 1; i < new_final; ++i) {
		state_exclusion(i, update_states);
	}

	if (update_states[0].find(0) != update_states[0].end()) {
		return "(" + update_states[0][0] + ")*(" + update_states[0][new_final] + ")";
	}
	return update_states[0][new_final];
}
