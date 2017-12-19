class Lexer:
    def __init__(self, f):
        self.file = open(f,'r',1)
        self.line = ''

    def get_next(self):
        self.line = self.file.readline()
        return self.line

    def close(self):
        self.file.close()
