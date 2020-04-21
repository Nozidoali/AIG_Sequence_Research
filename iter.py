import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('iter.csv')
df['iter'] = range( len(df['size']) )
ax = df.plot( x = 'iter', y = 'size', title = 'flexibility = 10, iter = 100' )
plt.show()