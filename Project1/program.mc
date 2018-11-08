import gdb

class StatePrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        regVal = self.val['reg']
        return regVal


def lookup_type(val):
    if str(val.type) == 'stateType':
        return StatePrinter(val)


gdb.pretty_printers.append(lookup_type)
