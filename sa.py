import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('data.csv')
df.plot.scatter( x = 'Iteration', y = 'TotalGain', color = 'Blue' )
df.plot.scatter( x = 'Iteration', y = 'Temperature', color = 'Red' )
plt.show()