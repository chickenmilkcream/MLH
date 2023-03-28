import matplotlib.pyplot as plt
import csv

x = []
y = []
y2=[]
oof = False
# Read data from CSV file
with open("experiments/phase2_LRU_workload_1_runtimes.csv") as csvfile:
    reader = csv.reader(csvfile)
    for row in reader:
        if oof:
            x.append(float(row[0]))
            y.append(float(row[1]))
        oof = True
oof = False
with open("experiments/phase2_clock_workload_1_runtimes.csv") as csvfile:
    reader = csv.reader(csvfile)
    for row in reader:
        if oof:
            y2.append(float(row[1]))
        oof = True



# Create a new figure and axis object
fig, ax = plt.subplots()

# Plot the data
ax.plot(x, y, 'o-', label='Data')
ax.plot(x, y2, 'o-', label='Data')

# Set the axis labels and title
ax.set_xlabel('X-axis')
ax.set_ylabel('Y-axis')
ax.set_title('Plot Example')

# Add a legend
ax.legend()

# Display the plot
plt.savefig('experiments/phase2_plot.png')
plt.show()
