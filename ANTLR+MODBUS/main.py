from antlr4 import *
from GCodeLexer import GCodeLexer
from GCodeParser import GCodeParser
from GCodeListener import GCodeListener
#from serial import setup_serial, send_coordinates_via_serial


class WalkListener(GCodeListener):
    def __init__(self):
        self.coordenadas = []
        self.set = False
        self.x = 0
        self.y = 0

    def enterStatement(self, ctx):
        if ctx.coordx() is not None:
            self.x = int(ctx.coordx().coord().getText())
            set = True
        if ctx.coordy() is not None:
            self.y = int(ctx.coordy().coord().getText())
            set = True
        if set:
            self.coordenadas.append((self.x, self.y))

def main():
    with open("ANTLR+MODBUS/GCode-example") as file:
        data = f'{file.read()}'
    lexer = GCodeLexer(InputStream(data))
    stream = CommonTokenStream(lexer)
    parser = GCodeParser(stream)
    tree = parser.gcode()
    listener = WalkListener()
    walker = ParseTreeWalker()
    walker.walk(listener, tree)
    print(listener.coordenadas)

if __name__ == '__main__':
    main()