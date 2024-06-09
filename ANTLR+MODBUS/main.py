from antlr4 import *
from GCodeLexer import GCodeLexer
from GCodeParser import GCodeParser
from GCodeListener import GCodeListener

class WalkListener(GCodeListener):
    def enterStatement(self, ctx):
        if ctx.codfunc() is not None:
            print(ctx.codfunc().getText())
        if ctx.coordx() is not None:
            print(ctx.coordx().coord().getText())
        if ctx.coordy() is not None:
            print(ctx.coordy().coord().getText())


def main():
    with open("GCode-example") as file:
        data = f'{file.read()}'
    lexer = GCodeLexer(InputStream(data))
    stream = CommonTokenStream(lexer)
    parser = GCodeParser(stream)
    tree = parser.gcode()
    print(tree.toStringTree(recog=parser))
    listener = WalkListener()
    walker = ParseTreeWalker()
    walker.walk(listener, tree)

if __name__ == '__main__':
    main()