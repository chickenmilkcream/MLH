import matplotlib.pyplot as plt
import csv
import sys


def load_data(input_file_name):
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
    return x, y


num_bits_per_entry = [5, 10, 15]

fig, ax = plt.subplots()

for i in num_bits_per_entry:
    f = "phase3_get_runtimes_numbits_" + str(i) + ".csv"
    x, y = load_data(f)
    # Plot the data removing outliers
    ax.plot(x, y, label=str(i) + " bits per entry")


    # Set the axis labels and title
    ax.set_xlabel('Volume of data inserted (bytes)')
    ax.set_ylabel('Runtime (ms)')
    ax.set_title("Runtime vs Volume of data inserted for different number of bits per entry in Bloom Filter")

    # Add a legend
    ax.legend()

    # Display the plot
    plt.savefig("phase3_get_runtimes_2.png")
    plt.show()
