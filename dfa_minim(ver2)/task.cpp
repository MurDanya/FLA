#include "api.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <queue>

using namespace std;

void
dfs(string state, DFA &d, set <string> &reachable_states) {
	if (reachable_states.find(state) == reachable_states.end()) {
		reachable_states.insert(state);
		for (auto trans : d.get_alphabet()) {
			if (d.has_trans(state, trans)) {
				dfs(d.get_trans(state, trans), d, reachable_states);
			}
		}
	}
}

pair<set <string>, set <string>>
split(DFA &d, set <string> &R, set <string> &C, char ch) {
	set <string> R1, R2;
	for (auto state : R) {
		if (d.has_trans(state, ch) && C.find(d.get_trans(state, ch)) != C.end())
			R1.insert(state);
		else
			R2.insert(state);
	}
	return make_pair(R1, R2);
}

set <set <string>>
combining_states(DFA& d) {
	set <string> F, Q_F, C;
	for (auto state : d.get_states()) {
		if (d.is_final(state))
			F.insert(state);
		else
			Q_F.insert(state);
	}
	set <set <string>> P = { F, Q_F };
	queue <set <string>> S;
	S.push(F);
	S.push(Q_F);
	while (!S.empty()) {
		C = S.front();
		S.pop();
		for (auto ch : d.get_alphabet()) {
			for (auto R : P) {
				auto [R1, R2] = split(d, R, C, ch);
				if (!R1.empty() && !R2.empty()) {
					P.erase(R);
					P.insert(R1);
					P.insert(R2);
					S.push(R1);
					S.push(R2);
				}
			}
		}
	}
	return P;
}

DFA dfa_minim(DFA& d) {
	set <string> reachable_states;
	dfs(d.get_initial_state(), d, reachable_states);
	set <string> diff;
	for (auto state : d.get_states()) {
		if (reachable_states.find(state) == reachable_states.end()) {
			d.delete_state(state);
		}
	}

	string dead_state = "DEAD_STATE";
	d.create_state(dead_state);
	for (auto state : d.get_states()) {
		for (auto trans : d.get_alphabet()) {
			if (!d.has_trans(state, trans)) {
				d.set_trans(state, trans, dead_state);
			}
		}
	}

	set <set <string>> comb_states = combining_states(d);
	DFA minim_d(d.get_alphabet());
	bool is_first_state;
	string first_state;
	for (auto states : comb_states) {
		is_first_state = true;
		for (auto state : states) {
			if (is_first_state) {
				minim_d.create_state(state, d.is_final(state));
				is_first_state = false;
				first_state = state;
			}
			if (d.is_initial(state)) {
				minim_d.set_initial(first_state);
			}
		}
	}

	string to_state;
	for (auto states : comb_states) {
		for (auto ch : d.get_alphabet()) {
			for (auto state : states) {
				to_state = d.get_trans(state, ch);
				for (auto to_states : comb_states) {
					if (to_states.find(to_state) != to_states.end()) {
						for (auto tmp : to_states) {
							to_state = tmp;
							break;
						}
						break;
					}
				}
				minim_d.set_trans(state, ch, to_state);
				break;
			}
		}
	}

	for (auto states : comb_states) {
		if (states.find(dead_state) != states.end()) {
			for (auto tmp : states) {
				minim_d.delete_state(tmp);
			}
		}
	}
	return minim_d;
}
