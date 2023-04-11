import matplotlib.pyplot as plt
import csv
import sys


def plot_pls(input_file_name, title, output_file_name):
    x = []
    y = []
    oof = False
    # Read data from CSV file
    with open(input_file_name) as csvfile:
        reader = csv.reader(csvfile)
        for row in reader:
            if oof:
                x.append(float(row[0]))
                y.append(float(row[1]))
            oof = True
    oof = False

    # Create a new figure and axis object
    fig, ax = plt.subplots()

    # Plot the data removing outliers
    ax.plot(x, y, label='Runtime')


    # Set the axis labels and title
    ax.set_xlabel('Volume of data inserted (bytes)')
    ax.set_ylabel('Runtime (ms)')
    ax.set_title(title)

    # Add a legend
    ax.legend()

    # Display the plot
    plt.savefig(output_file_name)
    plt.show()

plot_pls("experiments/phase3_put_runtimes.csv", 'Phase 3 Put Runtime', 'experiments/phase3_put_plot.png')
plot_pls("experiments/phase3_scan_runtimes.csv", 'Phase 3 Scan Runtime', 'experiments/phase3_scan_plot.png')
plot_pls("experiments/phase3_get_runtimes.csv", 'Phase 3 Get Runtime', 'experiments/phase3_get_plot.png')