import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('test.csv')
df['Sequential'] = df['rwrsize']/df['oldsize']
df['Simulated Annealing'] = df['sasize']/df['oldsize']
ax = df.plot(style = '.', x = 'file', y = 'Sequential', color = 'blue', xticks = range(len(df.file)), rot = 90, legend = True)
df.plot(style = '.', x = 'file', y = 'Simulated Annealing', color = 'red', xticks = range(len(df.file)), rot = 90, legend = True, ax = ax)

plt.show()
