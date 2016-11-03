//
//  practice1.cpp
//  formal_languages
//
//  Created by Vlad on 19/10/2016.
//  Copyright © 2016 Codelovin. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <stack>
#include <queue>
#include <sstream>

static const char eps = '1';
static const long INF = 1e7+7;

class FSA {
    
    std::vector<long> states;
    long initialState;
    std::vector<long> finalStates;
    std::vector<std::pair<std::pair<long, char>, long>> transitions;
    
    void removeAllEpsilonTransitions();
    void DFS(long state, char x, std::map<long, bool> used, long currentK);
    
    long answer;
    
    void incrementStateIds(long shift);
    
public:
    static bool checkRegex(std::string s);
    
    FSA ();
    FSA (char ch);
    FSA (std::string s);
    
    void calculateMaximumK(char ch);
    long getAnswer();
    
    long getMaxStateId() const;
    size_t getStateCount() const;
    
    static FSA concatenate (FSA first, FSA second);
    static FSA alternate (FSA first, FSA second);
    static FSA kleene (FSA other);
    
    void addTransition (long from, long to, char c);
    void addEpsilonTransition (long from, long to);
};

FSA::FSA() {
    states = {0};
    initialState = 0;
    finalStates = {0};
}

FSA::FSA(char ch) {
    states = {0, 1};
    initialState = 0;
    finalStates = {1};
    transitions = {std::make_pair(std::make_pair(0, ch), 1)};
}

bool FSA::checkRegex(std::string s) {
    int operands = 0;
    for (int i = 0; i < s.length(); i++) {
        switch (s[i]) {
            case 'a':
                operands++;
                break;
                
            case 'b':
                operands++;
                break;
                
            case 'c':
                operands++;
                break;
                
            case eps:
                operands++;
                break;
                
            case '.':
                operands--;
                break;
                
            case '*':
                break;
                
            case '+':
                operands--;
                break;
                
            default:
                return 0;
                break;
        }
        if (operands < 1)
            return false;
    }
    return operands == 1;
}

FSA::FSA(std::string s) {
    assert(FSA::checkRegex(s));
    
    if (s == "")
        return;
    
    std::stack<FSA> machines;
    for (int i = 0; i < s.length(); i++) {
        char ch = s[i];
        if (ch == 'a' || ch == 'b' || ch == 'c' || ch == eps) {
            machines.push(FSA(ch));
        } else if (ch == '*') {
            FSA r = machines.top(); machines.pop();
            machines.push(FSA::kleene(r));
        } else if (ch == '+') {
            FSA r = machines.top(); machines.pop();
            FSA l = machines.top(); machines.pop();
            machines.push(FSA::alternate(l, r));
        } else if (ch == '.') {
            FSA r = machines.top(); machines.pop();
            FSA l = machines.top(); machines.pop();
            machines.push(concatenate(l, r));
        } else {
            assert(0);
        }
    }
    assert(machines.size() == 1);
    *this = machines.top();
}

void FSA::DFS(long state, char x, std::map<long, bool> used, long currentK) {
    used[state] = 1;
    answer = std::max(answer, currentK);
    for (auto t: transitions) {
        long from = t.first.first;
        char ch = t.first.second;
        long to = t.second;
        if (from != state)
            continue;
        if (ch == x) {
            if (used[to] == 1) {
                answer = INF;
                return;
            }
            DFS(to, x, used, currentK+1);
        } else if (ch == eps) {
            if (used[to] == 1) {
                if (currentK > 0) {
                    answer = INF;
                    return;
                }
            } else {
                DFS(to, x, used, currentK);
            }
        } else {
            continue;
        }
    }
}


void FSA::incrementStateIds(long shift) {
    initialState += shift;
    for (auto it = states.begin(); it != states.end(); it++)
        *it += shift;
    for (auto it = finalStates.begin(); it != finalStates.end(); it++)
        *it += shift;
    for (auto it = transitions.begin(); it != transitions.end(); it++) {
        it->first.first += shift;
        it->second += shift;
    }
}

long FSA::getMaxStateId() const {
    long res = 0;
    for (auto it: states)
        res = std::max(res, it);
    return res;
}

size_t FSA::getStateCount() const {
    return states.size();
}

void FSA::addTransition(long from, long to, char c) {
    transitions.push_back(std::make_pair(std::make_pair(from, c), to));
}

void FSA::addEpsilonTransition(long from, long to) {
    addTransition(from, to, eps);
}

FSA FSA::alternate(FSA first,FSA second) {
    
    FSA machine = FSA();
    
    first.incrementStateIds(1);
    long firstInitial = first.initialState;
    
    second.incrementStateIds(first.getMaxStateId()+1);
    long secondInitial = second.initialState;
    
    machine.states.insert(machine.states.end(), first.states.begin(), first.states.end());
    machine.states.insert(machine.states.end(), second.states.begin(), second.states.end());
    
    machine.finalStates = {};
    machine.finalStates.insert(machine.finalStates.end(), first.finalStates.begin(), first.finalStates.end());
    machine.finalStates.insert(machine.finalStates.end(), second.finalStates.begin(), second.finalStates.end());
    
    machine.transitions.insert(machine.transitions.end(), first.transitions.begin(), first.transitions.end());
    machine.transitions.insert(machine.transitions.end(), second.transitions.begin(), second.transitions.end());
    
    machine.addEpsilonTransition(0, firstInitial);
    machine.addEpsilonTransition(0, secondInitial);
    
    machine.initialState = 0;
    
    return machine;
}

FSA FSA::concatenate(FSA first,FSA second) {
    
    FSA machine = first;
    second.incrementStateIds(first.getMaxStateId()+1);
    
    machine.states.insert(machine.states.end(), second.states.begin(), second.states.end());
    machine.transitions.insert(machine.transitions.end(), second.transitions.begin(), second.transitions.end());
    
    for (auto from: first.finalStates)
        machine.addEpsilonTransition(from, second.initialState);
    
    machine.finalStates = second.finalStates;
    return machine;
}

FSA FSA::kleene(FSA other) {
    other.incrementStateIds(1);
    FSA machine = other;
    if (other.finalStates.size() == 1 && other.initialState == other.finalStates[0])
        return machine;
    machine.initialState = 0;
    machine.addEpsilonTransition(0, other.initialState);
    for (auto from: other.finalStates)
        machine.addEpsilonTransition(from, 0);
    machine.finalStates = {0};
    return machine;
}

long FSA::getAnswer() {
    return answer;
}

void FSA::calculateMaximumK(char x) {
    answer = 0;
    for (auto st: states)
        DFS(st, x, std::map<long, bool>(), 0);
}

int main() {
    
    //    std::string s = "bc.ab.+*";
    std::string s;
    std::cin >> s;
    char x;
    
    if (!FSA::checkRegex(s)) {
        std::cout << "Invalid regular expression.\n";
        return 0;
    }
    
    std::cin >> x;
    FSA a = *new FSA(s);
    a.calculateMaximumK(x);
    long answer = a.getAnswer();
    if (answer == INF)
        std::cout << "INF" << std::endl;
    else
        std::cout << answer << std::endl;
    return 0;
}
