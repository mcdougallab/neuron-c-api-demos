import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("netcon.csv")
plt.plot(data.t, data.v)
plt.xlabel("t (ms)")
plt.ylabel("v (mV)")
plt.savefig("netcon.png")

print("saved as netcon.png")