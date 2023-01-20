#导入数据
file = 'data_plot/test.txt'
with open(file) as f:
    num = f.read()
    global l
    l = list(num)
    for x in range(len(l)):
        if l[x] == ' ':
            l[x] = ','
s=''.join(l);
with open(file,'w') as f:
    f.write(s)
