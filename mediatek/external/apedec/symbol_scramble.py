import os, sys
import json
import random

symbol_table = []
#
# Convert all include to lower case
#
def parse_symbol_file(infile):
    fin = open(infile,'r')

    for line in fin.readlines():
        tokens = line.split()
        if len(tokens) == 3 and tokens[0] == '00000000' and tokens[1] == 't':
            symbol_table.append(tokens[2])
     
    fin.close()

def do_scramble(seed, outfile):
    fout = open(outfile,'w')
    alphabet = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
    min = 5
    max = 15
    random.seed(seed)

    fout.write('#ifndef __SCRAMBLE_H__\n')
    fout.write('#define __SCRAMBLE_H__\n')
    fout.write('\n')
    fout.write('\n')

    for symbol in symbol_table:   
        
        if '.clone' in symbol:
            b = symbol.split('.clone')  
            symbol = b[0]
        string=''
        for x in random.sample(alphabet,random.randint(min,max)):
            string += x
        space = 50 - len(symbol)
        line = '#define ' + symbol
        while space > 0:
            line += ' '
            space = space - 1
        line += string + '\n'
        fout.write(line)
        
    fout.write('\n')
    fout.write('\n')
    fout.write('#endif')
    fout.close()
        
if __name__ == '__main__':

    if len(sys.argv) != 4:
        print 'wrong usage ' + str(len(sys.argv))
        print ' seed symbol_file out_header_name'
        sys.exit(0)
    try:
       print 'symbol scramble tool v0.1'
       parse_symbol_file(sys.argv[2])
       do_scramble(sys.argv[1], sys.argv[3])
        
    except:
        print 'Error encountered. Press <Enter> to exit.'
        sys.exit(0)