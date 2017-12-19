class Node:
    def __init__(self):
        self.data = None
        self.right = None
        self.left = None

ops = {'**','*','/','%','+','-','=','(',')'}
prec = {'**':0,'*':1,'/':1,'%':1,'+':2,'-':2,'=':3}
right = {'**', '='}

def infix_to_postfix(stream):
    stack = []
    output = ''
    for token in stream:
        if token in ops:
            if token == '(':
                stack.append(token)
            elif token == ')':
                while stack[-1]!='(':
                    output += ' '+stack.pop()
                stack.pop()
            elif (not stack) or prec[token]<prec[stack[-1]]:
                stack.append(token)
            else:
                if token in right:
                    while stack and prec[token]>prec[stack[-1]]:
                        output += ' '+stack.pop()
                else:
                    while stack and prec[token]>=prec[stack[-1]]:
                        output += ' '+stack.pop()
                stack.append(token)
                    
        else:
            output += ' '+token
        print token, '---', stack,'---', output
    if stack:
        while stack:
            output += ' '+stack.pop()
    return output[1:]

def parse_tree(stream):
    stack = []
    for token in stream:
        temp = Node()
        if not (token in ops):
            temp.data = token
        else:
            temp.right = stack.pop()
            temp.left = stack.pop()
            temp.data = token
        stack.append(temp)
    return stack.pop()

def print_tree(tree,level=0):
    print '   '*level+str(level)+')',tree.data
    if tree.data in ops:
        print_tree(tree.left,level+1)
        print_tree(tree.right,level+1)
