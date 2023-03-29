import matplotlib.pyplot as plt
import csv
import sys

workload = "1" if sys.argv[1] == "workload1" else "2"

x = []
y = []
y2=[]
oof = False
# Read data from CSV file
with open("experiments/phase2_workload_" + workload + "_runtimes.csv") as csvfile:
    reader = csv.reader(csvfile)
    for row in reader:
        if oof:
            x.append(float(row[0]))
            y.append(float(row[1]))
            y2.append(float(row[2]))
        oof = True
oof = False

# Create a new figure and axis object
fig, ax = plt.subplots()

# Plot the data
ax.plot(x, y, 'o-', label='LRU')
ax.plot(x, y2, 'o-', label='Clock')

# Set the axis labels and title
ax.set_xlabel('Maximum Buffer Pool Size (bytes)')
ax.set_ylabel('Runtime (secs)')
ax.set_title('Phase 2 Clock vs LRU Workload '  + workload)

# Add a legend
ax.legend()

# Display the plot
plt.savefig('experiments/phase2_workload_' + workload + '_plot.png')
plt.show()
