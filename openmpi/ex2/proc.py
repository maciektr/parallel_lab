f = open("res.txt", "r")
l = ''
for line in f.readlines():
    if line.startswith('n '):
        l = line[2:-1]
    else:
        if l:
            line = l + '; '+ line
            l = ''
        print(line, sep='', end='')


