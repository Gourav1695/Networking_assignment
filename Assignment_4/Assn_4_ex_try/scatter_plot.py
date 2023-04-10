import matplotlib.pyplot as plt
# Load data from file
s = input("File name ")
with open(s, 'r') as f:
    data = f.readlines()

x = []
y = []
for d in data:
    d = d.strip().split()
    x.append(float(d[1]))
    y.append(float(d[2]))
# Create scatter plot
plt.scatter(x, y)
plt.title('Scatter plot')
plt.xlabel('Payload')
plt.ylabel('Cumulative RTT')
plt.show()