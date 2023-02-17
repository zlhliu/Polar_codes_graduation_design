import numpy as np
import matplotlib as ma
import matplotlib.pyplot as plt

x=[i-500 for i in range(10)]
y=[]
z={}
for i in range(10):
    z[i-5]=0
    y.append(i)
for item in range(10000):
    U1=np.random.rand()
    U2=np.random.rand()
    X2=np.sqrt(-2*np.log(U1)) * np.sin(2 * 3.14 * U2)
    z[int(X2)]+=1
for key,value in z.items():
    x[key+5]=value
plt.plot(y,x)
plt.savefig('data_plot/out.jpg')

plt.show()
