# ImageWidget
- This project based on Qt.
- This project is a submodule, check `.gitignore` before use it.
## Introduction: ##
It's my first time to write a class and this class inherit from QWidget. It can zoom, drag, crop loaded image in the widget, can also return the point position in widget or image your mouse clicked. But it still has some problems with zoom function, huge memory will be occupied when zoom out image.      

这是我第一次写的类，该类继承QWidget，在此基础上改写成了显示图像的ImageWidget，有缩放、拖拽、截取、获取左键点击处的Widget坐标和图像内坐标。缩放功能不是很完善，图像的放大会带来更多的内存占用。    
   
## How to use: ##
- Add those four files in project, add a Widget and promote to ImageWidget.
- 将四个文件加入工程，并对将Widget提升为ImageWidget即可。
## Screenshot: ##
![](https://github.com/WangHongshuo/Readme_Images_Repository/blob/master/ImageWidget-Qt/ImageWidget-Qt_1.jpg)   
![](https://github.com/WangHongshuo/Readme_Images_Repository/blob/master/ImageWidget-Qt/ImageWidget-Qt_2.gif)
## Change Log: ##

- 2018.01.15:

Optimized `paintEvent`, `QImage::scaled` will not be called when dragging image.      
Optimized zoom image funciton.     

优化`paintEvent`，在拖拽图片时不调用`QImage::scaled`。     
优化了缩放图片。       

- 2018.01.14:

When your mouse clicked at the widget, it will return the point position in the widget or the image.

加入返回点击处的Widget坐标和图像内的坐标的功能。      

- 2018.01.12:

Rewrote this class, renamed function names and variable names and removed some unnecessary variables.  

重写了该类，重新规划了变量和函数名，以及减少了变量的数量。     

- 2018.01.09: 
           
Added more funciton in contextmenu, and this class will be set to a submodule.           

右键菜单加入了更多功能，将次类单独出来作为子模块。        

- 2017.12.12:

Updated ImageWidget, rewrote load image function, you can set image by the pointer(not copy the memory) or the data(copy the memory), and added other small options.

更新了ImageWidget,重写了载入图像函数，可以用指针传入（不复制内存）和数据传入（复制内存），添加了载入图像时的一些额外选项。

- 2017.12.03:

Updated ImageWidget

更新了ImageWidget控件

- 2017.11.29:

Fixed a pointer bug

修复了当建立ImageWidget子对象时，父对象销毁后mp_img指针被多次delete的问题。

- 2017.11.17:

Fixed zoom in & zoom out limitation, this function needs huge computer memories when the scale is big, so the max scale is fixed to 12.

修正放大缩小限制，此功能当放大倍数过大时会占用很大内存，故将最大放大倍数限制为12。

- 2017.11.11:

Improved the SelectRect class & the ImageWidget class, removed useless parameters.

Improved interaction logic.

改进了selectrect类和ImageWidget类，去除不必要参数。在ImageWidget对象中单击另存为时会自己创建selectrect子对象，不用在外部用信号槽手动链接两个类。

改进交互逻辑。








