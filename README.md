# ImageWidget
- This project based on Qt.
- This project is a submodule, check `.gitignore` before use it.
## Introduction: ##
It's my first time to write a class and this class inherit from QWidget. It can zoom in and out, drag, crop loaded image in the widget, can also return the point position in widget or image your mouse clicked. But it still has some problems with zoom in and out function, huge memory will be occupied when zoom out image.      

这是我第一次写的类，该类继承QWidget，在此基础上改写成了显示图像的ImageWidget，有缩放、拖拽、截取、获取左键点击处的Widget坐标和图像内坐标。缩放功能不是很完善，图像的放大会带来更多的内存占用。    
   
## How to use: ##
- Add those four files in project, add a Widget and promote to ImageWidget.
- 将四个文件加入工程，并对将Widget提升为ImageWidget即可。
## Screenshot: ##
![](https://github.com/WangHongshuo/Readme_Images_Repository/blob/master/ImageWidget-Qt/ImageWidget-Qt_1.jpg)   
![](https://github.com/WangHongshuo/Readme_Images_Repository/blob/master/ImageWidget-Qt/ImageWidget-Qt_2.gif)
## Last Change: ##

- 2018.01.20:

Added resizing selected rect frame function. And now, I realize all the function I wanted in this class. But it still have some performance issues, I will optimize or rewrite it when I can.

添加了改变选中框尺寸的功能，至此该类已经达到了我所需的所有功能，但是还有一些性能上的问题。将来有能力和时间的话会进行优化或重写。

- Full [Change Log](./ChangeLog.md)






