lines = [line.rstrip('\n') for line in open('gb2312.cmm')]


w_str = "static const uint16_t unic[6768] = {"

for line in lines:
    w_str += line[0:6] + ", "

with open("gb2312.c","a") as w:
    w.write(w_str)


