#include "api.hpp"
#include <string>
#include <map>
#include <vector>

using namespace std;

std::string dfa2re(DFA &d) {
	map <string, map <string, string>> new_dfa;
	for (auto state : d.get_states()) {
		for (auto trans : d.get_alphabet()) {
			if (d.has_trans(state, trans)) {
				string to_state = d.get_trans(state, trans);
				string new_trans(1, trans);
				if (new_dfa[state].find(to_state) != new_dfa[state].end()) {
					new_dfa[state][to_state] = new_dfa[state][to_state] + "|" + new_trans;
				}
				else {
					new_dfa[state][to_state] = new_trans;
				}
			}
		}
		if (d.is_final(state)) {
			new_dfa[state]["NEW_FINAL"] = "";
		}
		if (d.is_initial(state)) {
			new_dfa[state][state] = "";
		}
	}
	for (auto state : d.get_states()) {
		if (!d.is_initial(state)) {
			string self_trans = "";
			vector <string> from_states;
			for (auto [from_st, transitions] : new_dfa) {
				if (transitions.find(state) != transitions.end()) {
					if (from_st == state) {
						self_trans = "(" + transitions[state] + ")*";
					}
					else {
						from_states.push_back(from_st);
					}
				}
			}
			for (auto from_st : from_states) {
				string from_trans = new_dfa[from_st][state];
				for (auto [to_st, to_trans] : new_dfa[state]) {
					string add_trans = "";
					if (new_dfa[from_st].find(to_st) != new_dfa[from_st].end()) {
						add_trans = new_dfa[from_st][to_st] + "|";
					}
					new_dfa[from_st][to_st] = add_trans + "(" + from_trans + ")" + self_trans + "(" + to_trans + ")";
				}
//				new_dfa[from_st].erase(state);
			}
			new_dfa.erase(state);
		}
	}
	string init_state = d.get_initial_state();
	return "(" + new_dfa[init_state][init_state] + ")*(" + new_dfa[init_state]["NEW_FINAL"] + ")";
}