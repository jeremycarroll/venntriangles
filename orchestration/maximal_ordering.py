all_orders = [[a,b,c,d,e,f]
for a in range(3,7)
for b in range(3,7)
for c in range(3,7)
for d in range(3,7)
for e in range(3,7)
for f in range(3,7)
]

def all_rotations(cycle):
    result = []
    for i in range(len(cycle)):
        result.append(cycle)
        cycle = cycle[1:] + [cycle[0]]
    return result

def is_maximal(order):
    """
    Return a pair, maximal, strictly maximal
    """

    reversed_rotations = all_rotations(list(reversed(order)))
    reversed_rotations.append(order)
    all_rotations0 = all_rotations(order)
    all_rotations1 = set(tuple(x) for x in all_rotations0)
    return order == max(all_rotations0) and order == max(reversed_rotations), len(all_rotations0) == len(all_rotations1)

strictly_canonical = []
equivocal6 = []
for order in all_orders:
    maximal, strictly = is_maximal(order)
    if maximal and sum(order) == 27:
        if strictly:
            strictly_canonical.append(order)
        else:
            equivocal6.append(order)

all_orders = [[a,b,c]
for a in range(3,7)
for b in range(3,7)
for c in range(3,7)
]

strictly_canonical3 = []
equivocal3 = []

for order in all_orders:
    maximal, strictly = is_maximal(order)
    # There are 15 of these, and we have 52 edges in total, so only 7 spare over the 3 minimum.
    if sum(order) - 9 > 7:
        continue

    if maximal:
        if strictly:
            strictly_canonical3.append(order)
        else:
            equivocal3.append(order)
            
for order in equivocal6:
    for order3 in strictly_canonical3:
        strictly_canonical.append(order + order3)
    for order3 in equivocal3:
        for order3a in strictly_canonical3 + equivocal3:
            if sum(order3) + sum(order3a) - 18 <= 7:
                strictly_canonical.append(order + order3 + order3a)

"""
Output are either 6 tuples or 9 tuples or 12 tuples.
6s are size of ~A, ~B, ~C, ~D, ~E, ~F
9s are size of ~A, ~B, ~C, ~D, ~E, ~F, ~AB, ~CD, ~EF
12s are size of ~A, ~B, ~C, ~D, ~E, ~F, ~AB, ~CD, ~EF, ~BC, ~DE, ~FA

12s include some with rotational symmetries, 6s and 9s are all strictly canonical.
"""
for order in strictly_canonical:
    print(order)
