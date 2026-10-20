# 使用方法
- 使用git bash 执行下面命令
```shell
git clone --recurse-submodules ssh://git@support.glorz.top:61322/WestBerryTech/qmk_firmware.git

cd qmk_firmware

git clone ssh://git@support.glorz.top:61322/WestBerryTech/multimode.git keyboards/multimode

```
- 在`keyboards`目录下新建`<keyboard>`目录，参考qmk开发流程开发自己的键盘