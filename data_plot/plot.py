# code:utf-8  	Ubuntu
import matplotlib.pyplot as plt
from scipy import interpolate
import numpy as np
import matplotlib.font_manager as mpt

chinese_font=mpt.FontProperties(fname=r'C:/Windows/Fonts/msyh.ttc') #显示中文字体
#导入数据
file = 'data_plot/data.txt'
a = np.loadtxt(file)
# 数组切片
x = a[:,0]  # 取第一列数据
y = a[:,1]  # 取第二列数据
# 进行样条插值
tck = interpolate.splrep(x,y)
xx = np.linspace(min(x),max(x),100)
yy = interpolate.splev(xx,tck,der=0)
print(xx)
# 画图
plt.plot(x,y,'o',xx,yy)
plt.legend(['true','Cubic-Spline'])
plt.xlabel('距离(cm)', fontproperties=chinese_font) #注意后面的字体属性
plt.ylabel('%')
plt.title('管线仪实测剖面图', fontproperties=chinese_font)  
# 保存图片  
plt.savefig('data_plot/out.jpg')
plt.show()