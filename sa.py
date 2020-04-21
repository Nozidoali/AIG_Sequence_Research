import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('data.csv')
df['improve'] = df.size_before - df.size_after
ax = df[df.type == 'seq'].plot( kind = 'line', x = 'runtime', y = 'improve', color = 'Blue', grid = True, yticks = range(80,120,2) )
df[df.type == 'sa'].plot( kind = 'line', x = 'runtime', y = 'improve', color = 'red', ax = ax )
plt.show()