# Instruction For Use

- 点击运行`1.copy_source_file.bat`，将编译输出的.hex文件拉取到当前目录

  - `tuya_ble_sdk_Demo_Project.hex` 				 [Merge cm0+cm4]
  - `tuya_ble_sdk_Demo_Project_signed.hex`   [only cm4]

- 用编辑器打开`tuya_ble_sdk_Demo_Project_signed.hex`文件，删除末尾5行并保存，例如：

  `:0200000490303A`
  `:02000000A563F6`
  `:0200000490501A`
  `:0C0000000005E20721002101E2F0C663C8`
  `:00000001FF`

- 点击运行`2.make_hex2bin.bat`，生成.bin升级文件



## 生产文件、升级文件

- 生产文件：`tuya_ble_sdk_Demo_Project.hex`
- 升级文件：`tuya_ble_sdk_Demo_Project_signed.bin`

