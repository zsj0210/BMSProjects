# BMSProjects
项目描述：本项目基于STM32F103主控芯片、BQ7692003PWR专用AFE芯片和5串三元锂电池的BMS控制板。实现了对电池电压、电流、温度的实时高精度监控，并具备被动均衡、充放电管理及完备的硬件保护功能。通过串口命令进行人机交互，旨在保障电池组的安全、稳定与长寿命运行。 主要工作内容：负责驱动BQ7692003PWR芯片，完成多节电芯的电压、电流采集及硬件保护电路的配置。基于RT-Thread操作系统开发固件，实现了项目移植到FreeRots操作系统中、SOC估算、被动均衡策略及充放电状态管理功能，并通过串口Shell命令实现系统控制与数据交互。
锂电池管理系统（BMS）控制板

基于 STM32F103C8T6 微控制器和 BQ7692003PWR AFE 芯片开发的5串三元锂电池管理系统（BMS）。具备高精度数据采集、充放电管理、被动均衡及完备的硬件保护功能，并通过串口命令行提供交互控制。
🧰 主要技术栈

    主控芯片: STM32F103C8T6 (ARM Cortex-M3)

    电池监控芯片: TI BQ7692003PWR

    实时操作系统: RT-Thread

    开发环境: Keil MDK5, STM32CubeMX, VS Code

    通信协议: I2C, UART, Modbus-RTU（预留CAN/RS485）

    调试工具: 串口调试助手、Xshell、Postman

⚙️ 功能特性

    ✅ 实时监测电池电压、电流、温度

    ✅ 支持5串三元锂电池（可软件调整）

    ✅ 硬件级保护：过压(OV)、欠压(UV)、过流(OC)、短路(SC)、过温(OT)

    ✅ 被动均衡功能，支持手动/自动均衡

    ✅ 基于安时积分法与开路电压的SOC估算

    ✅ 串口Shell命令行交互，支持充放电MOS控制

    ✅ 预留CAN总线与RS485通信接口

📁 项目结构
text

BMS-STM32-BQ7692003/
├── Docs/                           # 数据手册、参考资料
├── Hardware/                       # 原理图与PCB文件(若开源)
├── Software/
│   ├── Core/                       # STM32核心驱动
│   ├── BSP/                        # 板级支持包
│   ├── Drivers/
│   │   ├── drv_bq769x0.c           # BQ769X0驱动
│   │   └── drv_soft_i2c.c          # 软件I2C驱动
│   ├── Applications/
│   │   ├── bms_core.c              # BMS核心逻辑
│   │   ├── bms_config.h            # 系统参数配置
│   │   └── modbus_rtu.c            # Modbus通信协议
│   └── RT-Thread/                  # RT-Thread相关配置及组件
└── README.md

🔧 硬件连接
所需设备：

    5串18650电池板

    支持Modbus的充电器或电子负载

    USB转TTL模块（用于Shell通信）

    12V以上电源

接线说明：

    将电池板BAT+/-分别接至BMS控制板BAT接线柱

    充电器/负载仪接至PACK端口

    USB转TTL模块连接板载串口1（TX/RX交叉连接）

    使用6P排线连接电池板与BMS板（用于电压采集与均衡）

🚀 快速开始
编译与下载：

    使用 Keil MDK5 打开工程文件

    根据实际电池参数调整 bms_config.h 中的保护阈值

    编译工程并通过ST-Link下载至STM32

系统调试：

    连接串口工具至板载串口1（波特率：115200）

    输入 help 查看支持的命令列表：

text

BMS_CmdOpenDSG      - 开启放电MOS
BMS_CmdCloseDSG     - 关闭放电MOS
BMS_CmdOpenCHG      - 开启充电MOS
BMS_CmdCloseCHG     - 关闭充电MOS
BMS_CmdOpenBalan    - 开启均衡
BMS_CmdCloseBala    - 关闭均衡
BMS_CmdLoadDetec    - 负载检测
BMS_CmdOpenInfo     - 开启数据打印
BMS_CmdCloseInfo    - 关闭数据打印

📷 系统演示
数据监控界面：

系统可通过命令输出实时电池数据：
text

[BMS INFO] Cell Voltage: 3.91V, 3.90V, 3.89V, 3.88V, 3.87V
[BMS INFO] Battery Current: 0.00A, SOC: 95%
[BMS INFO] Max Voltage Difference: 0.04V

均衡功能：

支持电压偏差大于设定阈值时自动开启被动均衡，也可通过命令行手动控制。
