class Seed:
    def __init__(self,string):
        self.size = 1
        self.states = [[c for c in string]]

class NFA:
    def __init__(self,nfa1,nfa2):
        self.size = nfa1.size + nfa2.size
        self.states = []
        self.states = 
