#!/usr/bin/env python

# Packrat PEG (parsing expression grammar) generator
# http://pdos.csail.mit.edu/~baford/packrat/

next_var = 0
def nextVar():
    global next_var;
    next_var += 1;
    return next_var

def indent(s):
    space = '    '
    return s.replace('\n', '\n%s' % space)

class Pattern:
    def __init__(self):
        pass

    def generate(self, result):
        pass

class PatternNot(Pattern):
    def __init__(self, next):
        Pattern.__init__(self)
        self.next = next

    def generate(self, result):
        my_result = "result_%d" % nextVar()
        data = """
Result %s = 0;
%s
%s = ! %s;
        """ % (my_result, self.next.generate(my_result).strip(), result, my_result)

        return data

class PatternRule(Pattern):
    def __init__(self, rule):
        Pattern.__init__(self)
        self.rule = rule

    def generate(self, result):
        data = """
%s = rule_%s();
        """ % (result, self.rule)

        return data

class PatternRepeatOnce(Pattern):
    def __init__(self, next):
        Pattern.__init__(self)
        self.next = next

    def generate(self, result):
        data = """
        """

        return data

class PatternRepeatMany(Pattern):
    def __init__(self, next):
        Pattern.__init__(self)
        self.next = next

    def generate(self, result):
        data = """
        """
        return data

class PatternVerbatim(Pattern):
    def __init__(self, letters):
        Pattern.__init__(self)
        self.letters = letters

    def generate(self, result):
        data = """
%s = "%s";
        """ % (result, self.letters)
        return data

class Rule:
    def __init__(self, name, patterns):
        self.name = name
        self.patterns = patterns

    def generate(self):
        result = "result_%d" % nextVar()
        data = """
static Result rule_%s(){
    Result %s = 0;
    %s
    return %s;
}
        """ % (self.name, result, indent('\n'.join([pattern.generate(result).strip() for pattern in self.patterns])), result)

        return data
    
class Peg:
    def __init__(self, start, rules):
        self.start = start
        self.rules = rules

    def generate(self):
        namespace = "Peg"
        data = """
namespace %s{
    %s

Result main(){
    return rule_%s();
}

}
        """ % (namespace, '\n'.join([rule.generate() for rule in self.rules]), self.start)

        return data

def generate(peg):
    print peg.generate()

def test():
    rules = [
        Rule("s", [PatternNot(PatternVerbatim("hello"))]),
        Rule("blah", [PatternRule("s")])
    ]
    peg = Peg("s", rules)
    generate(peg)

test()
