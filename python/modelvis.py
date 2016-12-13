import matplotlib.pyplot as plt

upc = [(0, [3, 2, 1, 1]),
       (1, [2, 2, 2, 1]),
       (2, [2, 1, 2, 2]),
       (3, [1, 4, 1, 1]),
       (4, [1, 1, 3, 2]),
       (5, [1, 2, 3, 1]),
       (6, [1, 1, 1, 4]),
       (7, [1, 3, 1, 2]),
       (8, [1, 2, 1, 3]),
       (9, [3, 1, 1, 2])]

d = 0
w = 0.92
o = 0.96
x = [o-w, o]
y = [-1, -1]
par = 1
for s in upc[d][1]:
    x += [x[-1], x[-1]+w*s]
    y += [par, par]
    par *= -1

x += [o+7*w, o+8*w]
y += [par, par]
plt.plot(x, y)
ax = plt.gca()
ax.set_ylim([-1.5, 1.5])
plt.show()
