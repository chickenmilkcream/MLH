import matplotlib.pyplot as plt
import csv
import sys

x = []
y = []
y2=[]
oof = False
# Read data from CSV file
with open("experiments/phase2_search_runtimes.csv") as csvfile:
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
ax.plot(x, y, 'o-', label='Binary Search')
ax.plot(x, y2, 'o-', label='B Tree')

# Set the axis labels and title
ax.set_xlabel('Data Size (key-value pairs)')
ax.set_ylabel('Runtime (secs)')
ax.set_title('Phase 2 Binary Search vs B Tree Runtim')

# Add a legend
ax.legend()

# Display the plot
plt.savefig('experiments/phase2_search_plot.png')
plt.show()
