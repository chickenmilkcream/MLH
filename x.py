import math

        # M = math.ceil(len(layers[j - 1]) / (B * (B + 1))) * B
        # while len(layers[j]) < M:
        #     layers[j].append(None)

# leaves = [10, 20, 40,   50, 70, 80,   90, 110, 120,   140, 160, 192,   240, 260]
# leaves = [10, 20, 40,   50, 70, 80,   90, 110, 120,   140, 160, 192,   240, 260, 270]

N = 19
P = 4
K = 1
V = 1
B = P // (K + V)
MAGIC = 0xd6c5ac252a571e93f66f4a1a96b827a4


pairs = [(i, i) for i in range(N)]


def SST(pairs):
    height = math.ceil(math.log(N / B, B + 1)) + 1

    non_terminal_nodes = [[] for _ in range(height - 1)]

    for i in range(B - 1, N, B):
        for j in range(height, 0, -1):
            if (i + 1) % (B * (B + 1) ** (j - 1)) == 0:
                if j < height:
                    non_terminal_nodes[height - j - 1].append(pairs[i][0])
                break

    terminal_nodes = pairs

    sizes = []
    for i in range(height - 1):
        sizes.append(len(non_terminal_nodes[i]))
    sizes.append(len(terminal_nodes))

    # omit in C code!
    # for level in levels:
    #     while len(level) % P:
    #         level.append(None)
    #
    
    return height, non_terminal_nodes, terminal_nodes, sizes


# do we even need None None nodes when we have the offset? If the start + offset // sizeof_key + j of this layer
# is the offset of the next layer, then the None None nodes are implicit
# anther way of putting it: there will only ever be at most one None None node at the end of level
# if the end of a level is where you're sent, you know that's a None None node
# 1 None is still needed

height, non_terminal_nodes, terminal_nodes, sizes = SST(pairs)

print('B =', B)
print('N =', N)
print('height =', height)
for level, size in zip(non_terminal_nodes, sizes):
    print(level, size)
print(terminal_nodes)


meta = [MAGIC, height] + sizes
# omit in C code!
while len(meta) % P:
    meta.append(None)
#

# pages = meta + sum(levels, [])

# this is all a struct that fits into 4096 B
# magic
# height
# layer 0 size
# layer 1 size
# layer 2 size
# ...

# you stop reading after size * K
# next level starts at ceil(size * K / P) * P
# offset is (i * (B + 1) + j) * K where i is index of current node and j is index within current node

def binary_search(A, x):
    return _binary_search(A, 0, len(A) - 1, x)


def _binary_search(A, left, right, x):
    if left > right:
        raise KeyError()
    else:
        mid = (left + right) // 2
        if x < A[mid]:
            return _binary_search(A, left, mid - 1, x)
        elif x > A[mid]:
            return _binary_search(A, mid + 1, right, x)
        else:
            return mid


def find(pages, key):
    start = 0 # in bytes
    offset = 0 # in bytes

    page = pages[start:start + P]
    start += P
    meta = page
    assert meta[0] == MAGIC
    height = meta[1]
    sizes = pages[2:height + 2]

    page = pages[start + offset:start + offset + P]
    i = 0
    while i < height - 1:
        keys = page

        # TODO: change to binary search
        j = 0
        while j < min(B, sizes[i] - offset / K) and keys[j] < key:
            j += 1

        start += math.ceil(sizes[i] * K / P) * P
        offset = offset * (B + 1) + j * B * K
        if i == height - 2:
            offset *= 2

        i += 1

        page = pages[start + offset:start + offset + P]

    return page[binary_search(page, key)]
    


        
# print(find(pages, 8))
        

# B = 2 and look for 8
# [(5, 11), (1, 3), (7, 9), (13, None), (0, 1), (2, 3), (4, 5), (6, 7), (8, 9), (10, 11), (12, 13), (14, None)]