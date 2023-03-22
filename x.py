import math

        # M = math.ceil(len(layers[j - 1]) / (B * (B + 1))) * B
        # while len(layers[j]) < M:
        #     layers[j].append(None)

# leaves = [10, 20, 40,   50, 70, 80,   90, 110, 120,   140, 160, 192,   240, 260]
# leaves = [10, 20, 40,   50, 70, 80,   90, 110, 120,   140, 160, 192,   240, 260, 270]


P = 4
K = 1
V = 1
B = P // (K + V)
N = B * (B + 1) + 1


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

    terminal_nodes = []
    for pair in pairs:
        terminal_nodes.append(pair[0])
        terminal_nodes.append(pair[1])

    sizes = []
    for i in range(height - 1):
        sizes.append(len(non_terminal_nodes[i]) // K)
    sizes.append(len(terminal_nodes) // (K + V))  
    sizes.append(0)
    
    return height, non_terminal_nodes, terminal_nodes, sizes


# do we even need None None nodes when we have the offset? If the start + offset // sizeof_key + j of this layer
# is the offset of the next layer, then the None None nodes are implicit
# anther way of putting it: there will only ever be at most one None None node at the end of level
# if the end of a level is where you're sent, you know that's a None None node
# 1 None is still needed

height, non_terminal_nodes, terminal_nodes, sizes = SST(pairs)

# print('B =', B)
# print('N =', N)
print('height =', height)
for i in range(len(non_terminal_nodes)):
    while len(non_terminal_nodes[i]) % P:
        non_terminal_nodes[i].append(None)
while len(terminal_nodes) % P:
    terminal_nodes.append(None)
while len(sizes) % P:
    sizes.append(None)

pages = sizes + sum(non_terminal_nodes, []) + terminal_nodes

s = ''
for i in range(0, len(pages), P):
    t = ''
    for j in range(i, i + P):
        t += ', ' + str(pages[j])
    s += '[' + t[2:] + '] '
print(s)



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
    offset = 0 # in bytes

    sizes = []

    page = pages[offset:offset + P]
    offset += P
    i = 0
    while page[i]:
        sizes.append(page[i])
        i += 1
        if i == P // 1:
            page = pages[offset:offset + P]
            offset += P
            i = 0

    start = offset
    offset_j = 0

    page = pages[offset:offset + P]
    i = 0
    while i < height - 1:
        limit = min(offset_j + B, sizes[i] - (offset - start) // K)
        j = offset_j
        while j < limit and page[j] < key:
            j += 1

        end = start + math.ceil(sizes[i] * K / P) * P
        intermediate = end + (((offset - start) // K + offset_j) // B * (B + 1) + (j - offset_j)) * B * (K if i < height - 2 else K + V)
        offset = intermediate // P * P
        offset_j = intermediate % P // K
        start = end

        page = pages[offset:offset + P]
        i += 1

    return page
    


print()
print(find(pages, 5))
        

# B = 2 and look for 8
# [(5, 11), (1, 3), (7, 9), (13, None), (0, 1), (2, 3), (4, 5), (6, 7), (8, 9), (10, 11), (12, 13), (14, None)]


def next(arr, target):
    start = 0;
    end = len(arr) - 1;
 
    ans = -1;
    while (start <= end):
        mid = (start + end) // 2;
        if (arr[mid] < target):
            start = mid + 1;
        else:
            ans = mid;
            end = mid - 1;
 
    return ans;
 
# Driver code
if __name__ == '__main__':
    arr = [1, 2, 3, 5, 8, 12];
    print(next(arr, 4));