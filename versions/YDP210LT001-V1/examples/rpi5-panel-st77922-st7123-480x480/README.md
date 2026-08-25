# 1. 准备工作

```
# 更新软件包列表
sudo apt update

# 安装编译工具链与匹配的内核头文件
sudo apt install build-essential linux-headers-$(uname -r)

# 创建文件夹并进入
mkdir vc4-kms-dsi-st77922-st7123 && cd vc4-kms-dsi-st77922-st7123
```

# 2. 驱动源码（panel-st77922-480x480.c）

```
sudo nano panel-st77922-480x480.c
```

# 3. 驱动源码（st7123_touch.c）

```
sudo nano st7123_touch.c
```



# 4. Makefile

```
sudo nano Makefile
```

```
obj-m += panel-st77922-480x480.o
obj-m += st7123_touch.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
```

> 编译：

```
make clean
make
```

```
sudo mkdir -p /lib/modules/$(uname -r)/kernel/drivers/gpu/drm/panel/
sudo cp panel-st77922-480x480.ko /lib/modules/$(uname -r)/kernel/drivers/gpu/drm/panel/

sudo mkdir -p /lib/modules/$(uname -r)/kernel/drivers/input/touchscreen/
sudo cp st7123_touch.ko /lib/modules/$(uname -r)/kernel/drivers/input/touchscreen/

sudo depmod -a
```

设置开机自动加载模块：
编辑 /etc/modules 文件（sudo nano /etc/modules），在末尾加上这两行：

```
panel-st77922-480x800
st7123_touch
```


# 5. 设备树 Overlay（vc4-kms-dsi-st77922-st7123.dts）

```
sudo nano vc4-kms-dsi-st77922-st7123.dts
```

> 编译并安装：

```
dtc -I dts -O dtb -o vc4-kms-dsi-st77922-st7123.dtbo vc4-kms-dsi-st77922-st7123.dts

sudo cp vc4-kms-dsi-st77922-st7123.dtbo /boot/firmware/overlays/
```


# 5. 启用

> 编辑 /boot/firmware/config.txt，添加：

```
sudo nano  /boot/firmware/config.txt
```



```
# 关闭自动检测，避免和手动 overlay 冲突
display_auto_detect=0

dtoverlay=vc4-kms-v3d

# 启用 ST77922 显示与 ST7123 触控 Overlay
dtoverlay=vc4-kms-dsi-st77922-st7123

# 忽略官方 LCD
ignore_lcd=1
```

> 重启：

```
sudo reboot
```

