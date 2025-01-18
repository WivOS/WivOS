#!/usr/bin/env python2
# coding: utf-8
"""
	Generate a symbol table from nm output.
"""

import fileinput

def formatLine(line):
    stripLength = len(line.strip().split(" "))
    if stripLength < 3 or stripLength > 4:
        return

    #  _, _, name = line.strip().split(" ")
    temp_list = line.strip().split(" ")
    size = 0
    if len(temp_list) >= 4:
        name, _, _, size = temp_list
    else:
        name, _, _ = temp_list
    if name == "abs":
        # abs is a keyword, so we'll just pretend we don't have that
        return
    if name == "kernel_symbols_start" or name == "kernel_symbols_end":
        # we also don't want to include ourselves...
        return
    zeroes = ", 0" # * (1 + 4 - ((len(name) + 1) % 4))
    print """
    extern %s
    align 8
    dq %s
    dq 0x%s
    db '%s'%s""" % (name, name, size, name, zeroes)


print "SECTION .symbols"

print "global kernel_symbols_start"
print "kernel_symbols_start:"
for line in fileinput.input():
    formatLine(line)

print "global kernel_symbols_end"
print "kernel_symbols_end: ;"