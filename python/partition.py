import matplotlib.pyplot as plt
from math import sqrt, floor

# rs = [2, 1, 2, 2]
# rs = [3, 1, 1, 2]
# s = 0
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
# upc = [('guard', [1, 1, 1]),
#        ('mid', [1, 1, 1, 1, 1])]
eps = 1e-6

def feq(p1, p2):
    if type(p1) in [list, tuple]:
        return sqrt((p1[0]-p2[0])**2+(p1[1]-p2[1])**2)<eps
    else:
        return abs(p1-p2)<eps

# for i in range(len(rs)):
#     s += rs[i]
#     for q in range(1, s+1):
#         plt.plot([q/s, (q-1)/s], [0, 1])
#         s2 = 0
#         for j in range(i+1, len(rs)):
#             s2 += rs[j]
#             for q2 in range(q, q+s2):
#                 if s2*q >= s*(q2-q) and s2*q <= s2+s*(q2-q):
#                     print(i, j, s, s2, q, q2, (q2-q)/s2, q-s/s2*(q2-q))
#                     plt.plot([(q2-q)/s2], [q-s/s2*(q2-q)], 'o')

def doPoint(cells, a):
    for cell, data in cells:
        found = False
        for p in cell:
            if feq(a, p):
                data += [('point', p)]
                found = True
                break
        if found:
            continue

        for p1, p2 in zip(cell, cell[1:]+[cell[0]]):
            if p1[0] == p2[0]:
                if p1 == p2:
                    print('\n!!!!!!!!1\n', p1, cell)
                t2 = (a[1]-p1[1])/(p2[1]-p1[1])
                if feq(p1[0], a[0]) and 0 <= t2 and t2 <= 1:
                    data += [('edge', p1, p2, a)]
            elif p1[1] == p2[1]:
                t1 = (a[0]-p1[0])/(p2[0]-p1[0])
                if feq(p1[1], a[1]) and 0 <= t1 and t1 <= 1:
                    data += [('edge', p1, p2, a)]
            else:
                t1 = (a[0]-p1[0])/(p2[0]-p1[0])
                t2 = (a[1]-p1[1])/(p2[1]-p1[1])
                if 0<=t1 and t1<=1 and abs(t1-t2)<eps:
                    data += [('edge', p1, p2, a)]

def updateCells(cells):
    newcells = []
    for cell, data in cells:
        if len(data) == 2:
            cell1 = []
            cell2 = []
            curcell = cell1
            switch = False
            for p1, p2 in zip(cell, cell[1:]+[cell[0]]):
                if (data[0][0] == 'point' and feq(data[0][1], p1)) or (data[1][0] == 'point' and feq(data[1][1], p1)):
                    cell1 += [p1]
                    cell2 += [p1]
                    if switch:
                        curcell = cell1
                        switch = False
                    else:
                        curcell = cell2
                        switch = True
                else:
                    found = False
                    for d in data:
                        if d[0] == 'edge' and feq(p1, d[1]) and feq(p2, d[2]):
                            found = True
                            if switch:
                                cell1 += [d[3]]
                                cell2 += [p1, d[3]]
                                curcell = cell1
                                switch = False
                            else:
                                cell1 += [p1, d[3]]
                                cell2 += [d[3]]
                                curcell = cell2
                                switch = True
                            break
                    if not found:
                        curcell += [p1]

            newcells += [(cell1, []), (cell2, [])]
        else:
            newcells += [(cell, [])]
    return newcells

colors = ['r', 'b', 'g', 'm', 'c', 'y', 'k']
colors = 3*colors
for d,rs in upc:
    rs = list(reversed(rs))
    rs = [1,1,1]+rs
    print(rs)
    rs += [1]
    cells = [([(0,0),(1,0),(1,1),(0,1)],[])]
    plt.plot([0, 1], [0, 1], 'black')
    doPoint(cells, (0,0))
    doPoint(cells, (1,1))
    cells = updateCells(cells)
    s = 0
    for i in range(len(rs)):
        s += rs[i]
        for q in range(1, s+1):
            # lines += [([q/s, (q-1)/s], [0, 1])]
            plt.plot([q/s, (q-1)/s], [0, 1], colors[i])
            doPoint(cells, (q/s,0))
            doPoint(cells, ((q-1)/s,1))

            plt.plot([q/(1+s)], [q/(1+s)], 'o')
            doPoint(cells, (q/(1+s), q/(1+s)))
            s2 = 0
            newpts = []
            for j in range(i,0,-1):
                s2 += rs[j]
                for q2 in range(q-s2+1, q):
                    if s2*q > s*(q-q2) and s2*q < s2+s*(q-q2) and (1+s-s2)*q != (1+s)*q2:
                        # print(i, j, s, s2, q, q2, (q2-q)/s2, q-s/s2*(q2-q))
                        found = False
                        for oldq, olds in newpts:
                            if  s2*(q-oldq) == olds*(q-q2):
                                found = True
                                break
                        if not found:
                            plt.plot([(q-q2)/s2], [q-s/s2*(q-q2)], 'o')
                            doPoint(cells, ((q-q2)/s2, q-s/s2*(q-q2)))
                            newpts += [(q2, s2)]

            cells = updateCells(cells)

    cellres = []
    j = 0
    outf = open(str(d)+'.dat', 'w')
    outf.write('nr, area, centroidx, centroidy, numBars, pixPerBar0, ..., pixPerBarN, vertex1x, vertex1y, vertex2x, ...\n')
    for cell,_ in cells:
        mx = 0
        my = 0
        for x, y in cell:
            mx += x
            my += y
        mx /= len(cell)
        my /= len(cell)
        # plt.plot([mx], [my], 'x')

        s = 0
        qs = []
        for i in range(len(rs)):
            s += rs[i]
            qs += [floor(my+s*mx)]

        pixoffs = [floor(mx-my)+1, qs[0]]
        for q1, q2 in zip(qs, qs[1:]):
            pixoffs += [q2-q1]

        area = 0
        centx = 0
        centy = 0
        for p1, p2 in zip(cell, cell[1:]+[cell[0]]):
            area += p1[0]*p2[1]-p1[1]*p2[0]
            centx += (p1[0]+p2[0]) * (p1[0]*p2[1]-p2[0]*p1[1])
            centy += (p1[1]+p2[1]) * (p1[0]*p2[1]-p2[0]*p1[1])
        area /= 2
        centx /= 6*area
        centy /= 6*area
        plt.plot([centx], [centy], 'x')
        plt.annotate('{:.4f}\n'.format(area)+str(pixoffs), xy=(centx, centy))
        cellres += [(cell, area, pixoffs)]
        outf.write(str(j) + ', ' + str(area) + ', ' + str(centx) + ', ' + str(centy) + ', ' + str(len(pixoffs)) + ', ')
        for p in pixoffs:
            outf.write(str(p) + ', ')
        for v in cell[:-1]:
            outf.write(str(v[0]) + ', ' + str(v[1]) + ', ')
        outf.write(str(cell[-1][0]) + ', ' + str(cell[-1][1]) + '\n')
        j += 1
    print(cellres)
    # plt.show()
