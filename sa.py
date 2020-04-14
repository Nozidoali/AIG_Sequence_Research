import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('data.csv')
df.plot( x = 'Iteration', y = 'TotalGain', color = 'Blue', title = 'r=0.9, t=5', yticks = range(0,70,5) )
# df.plot( x = 'Iteration', y = 'Temperature', color = 'Red' )
plt.show()