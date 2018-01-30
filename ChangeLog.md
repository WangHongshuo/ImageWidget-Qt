# Change Log: #

- 2018.01.20:

Added resizing selected rect frame function. And now, I realize all the function I wanted in this class. But it still have some performance issues, I will optimize or rewrite it when I can.

添加了改变选中框尺寸的功能，至此该类已经达到了我所需的所有功能，但是还有一些性能上的问题。将来有能力和时间的话会进行优化或重写。

- 2018.01.25 #2:

Added dragging selected rect area funciton, but this function needs a lot computing resources, so it can be optimized in the future. Preparing to reduce variables and code.

添加了拖拽选定框功能，但是该功能有些占用CPU，未来可优化。准备优化成员变量，减少代码数量。

- 2018.01.25 #1:

Added a new method to crop image, you can crop the zoomed image (the image you actually seen in the widget) or the original image with the selected rect area     

添加了新的截图方式。可以截取选定区域的缩放后的图像(实际看到的)或原始图像(选中区域在原始图像上的部分)。     

- 2018.01.23:     

Fixed a processing sequence error in zoom in and out funciton, this error may lead to unexpected zoom in and out result (The image should be zoomed in and out center on the cursor postion).     

修复了缩放图像过程中一个顺序错误，该错误会导致图像不会按照预期的效果一样以指针处为中心进行缩放。      

- 2018.01.17:     

Added macro to fix Chinese character gibberish problem when using MSVC.     

添加宏修复MSVC下中文乱码问题。     

- 2018.01.16:

Updated zoom in and out image function and now the image will be zoomed in and out center on the cursor postion.      

更新了缩放图片功能，现在以光标处为中心进行缩放。     

- 2018.01.15:

Optimized `paintEvent`, `QImage::scaled` will not be called when dragging image.      
Optimized zoom in and out image funciton.     

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

Fixed zoom in and zoom out limitation, this function needs huge computer memories when the scale is big, so the max scale is fixed to 12.

修正放大缩小限制，此功能当放大倍数过大时会占用很大内存，故将最大放大倍数限制为12。

- 2017.11.11:

Improved the SelectRect class & the ImageWidget class, removed useless parameters.

Improved interaction logic.

改进了selectrect类和ImageWidget类，去除不必要参数。在ImageWidget对象中单击另存为时会自己创建selectrect子对象，不用在外部用信号槽手动链接两个类。

改进交互逻辑。








