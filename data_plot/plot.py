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
x1 = a[:int(len(a)/2),0]  # 取第一列数据
y1 = a[:int(len(a)/2),1]  # 取第二列数据

x2 = a[int(len(a)/2):,0]  # 取第一列数据
y2 = a[int(len(a)/2):,1]  # 取第二列数据
# # 进行样条插值
# tck = interpolate.splrep(x1,y1)
# xx = np.linspace(min(x1),max(x1),100)
# yy = interpolate.splev(xx,tck,der=0)
# print(xx)
# plt.plot(x1,y1,'0',xx,yy)
# 画图
plt.figure(1)
plt.subplot()
plt.plot(x1,y1)
# plt.savefig('data_plot/out1.jpg')


# tck = interpolate.splrep(x2,y2)
# xx = np.linspace(min(x2),max(x2),100)
# yy = interpolate.splev(xx,tck,der=0)
# print(xx)

plt.plot(x2,y2)
plt.legend(['true','Cubic-Spline'])
plt.xlabel('信噪比(snr)', fontproperties=chinese_font) #注意后面的字体属性
plt.ylabel('%')
plt.title('SC译码器的BER-FER', fontproperties=chinese_font)  
# 保存图片  
plt.savefig('data_plot/out.jpg')
plt.show()