##### 1）BMS业务逻辑层次

- 层次划分：系统分为三个逻辑层次

  - 底层（修路层）：负责硬件通道管理（如I2C通信初始化）

  - 中间层（物流层）：实现数据传输与协议处理

  - 上层（业务层）：执行具体业务功能（如电池监控、保护等）

- 核心思想：底层是手段，上层是目的，所有技术实现最终服务于业务需求

- 业务组成：系统包含6大核心业务模块

  - 电池监控业务(BMS_Monitor)

  - 电池保护业务(BMS_Protect)

  - 电池分析业务(BMS_Analysis)

  - 能量管理业务(BMS_Energy)

  - 信息管理业务(BMS_Info)

  - 通信管理业务(BMS_Comm)

2）电池监控任务分析

- MonitorBattery：负责电池相关参数的监控

  - 监控单体电芯电压，通过Bms_HalMonitorCellVoltage实现

  - 监控电池组电压，通过Bms_HalMonitorBatteryVoltage实现

    - 实现方式：通过I2C通信从BQ769X0芯片采集各电芯电压数据，更新数值后通过for循环遍历所有电芯，将原始数据存入BMS_MonitorData结构体

    - 硬件交互层:

      - 通过I2C总线与BQ769X0芯片通信

      - 使用BQ769X0_ReadBlockWithCRC带CRC校验读取数据块

      - 原始ADC数据需进行位移和运算处理

    - 驱动分层

      - 上层驱动：BQ769X0芯片功能封装

      - 下层驱动：软件模拟I2C通信（基于GPIO操作）

  - 监控电池温度：通过Bms_HalMonitorCellTemperature实现

    - 实现方式：同样通过BQ769X0芯片获取原始数据，温度数据存储在BMS_MonitorData.CellTemp数组中

  - 监控电池电流，通过Bms_HalMonitorBatteryCurrent实现

- MonitorSysMode：负责系统运行模式的监控

  - 功能定位：通过电流监测实现BMS系统状态（sleep/standby/工作模式）的自动转换

  - 实现方式：周期性检查电池电流值，根据阈值判断当前系统应处的模式状态

- 执行特点：属于高频执行任务，需要周期性采集数据

3）电池保护任务分析

- 软件保护实现机制：周期性检测（约1秒），存在延迟

  - 充电保护类型：

    - 过流(OCC)：BatteryCurrent > OCCProtect

    - 过温(OTC)：CellTemp > OTCProtect

    - 低温(LTC)：CellTemp < LTCProtect

    - 处理流程：

      - 调用BMS_HalCtrlCharge(BMS_STATE_DISABLE)停止充电

      - 启动定时器BMS_ProtectStartTimer设置恢复等待时间

      - 更新状态标志ProtectState和BMS_ProtectAlert

  - 放电保护：

    - 主要监控：温度异常（过温/低温）

    - 关键操作：通过BMS_HalCtrlDischarge控制放电MOSFET

- 硬件保护机制

  - 保护类型：立即触发，用于短路等紧急情况

    - OCD：放电过流保护

    - SCD：短路保护

    - OV/UV：过压/欠压保护

  - 处理特点：

    - 硬件自动触发保护动作

    - 软件主要负责状态记录和延时恢复

    - 通过BMS_ProtectStartTimer设置不同的恢复时间：

      - OCDRelieve：放电过流恢复时间

      - SCDRelieve：短路恢复时间

    - 解除操作：

      - 充电相关保护：调用BMS_HalCtrlCharge重新使能充电

      - 放电相关保护：调用BMS_HalCtrlDischarge恢复放电

4) RTOS任务设计好处

  - 简化业务逻辑设计；Monitor与Protect任务独立运行，通过全局变量实现数据共享

  - 任务间解耦，各自专注单一功能；优先级设置与临界区保护

5）电池分析任务-分析内容

  - 最大电压差：通过已排序的电池电压数据计算最高与最低电压差值

  - 平均电压：对所有电池电压求和后取平均

  - 实时功率：通过电流和电压相乘直接计算得到

  - 特点：这些参数都是通过简单计算即可获得，不需要复杂算法

  - 实时校准容量-影响因素

    - 温度变化对电池容量的影响

    - 完整充放电循环次数

    - 电池老化程度

    - 实现方式：通过BMS_AnalysisTempCal()函数进行温度校准

    - 温度校准

      - 温度变化检测：判断当前温度与上次记录温度差值是否超过1度

      - 校准比率计算：根据温度变化范围确定容量调整比率

      - 应用场景：特别在低温环境下，锂电池容量会显著减少，需要进行补偿

  - SOC检查与计算

  - 调用BMS_AnalysisOcvSocCalculate()进行开路电压到SOC的转换

  - 调用BMS_AnalysisAHSocCalculate()进行安时积分计算

6）能量管理任务

  - 信号量作用

    - 同步机制：使用信号量实现任务间同步，当条件成熟时通过信号量唤醒等待任务

    - 均衡控制：信号量专门用于均衡管理，防止多任务同时操作均衡状态

    - 工作流程：任务可能沉睡在信号量上，另一任务发出信号量激活同步

  - 充放电管理

    - 报警处理：仅在BMS_ProtectAlert == FLAG_ALERT_NO时执行充放电操作 

    - SOC控制： 充电停止：当SOC ≥ SocStopChg时调用BMS_HalCtrlCharge

    - 放电停止：当SOC ≤ SocStopDsg时关闭放电 

    - 安全设计：SOC阈值设置需留余量（如物理100%对应逻辑120%） 

    - 模式切换： 

    - 待机模式：检查用户开关状态(BMS_GlobalParam.Charge/Discharge) 

    - 睡眠模式：特殊情况下允许充电底层调用，最终通过BQ769X0_ControlDSGOrCHG控制MOS管 

    - 命令响应：周期性检查全局参数变化实现命令快速响应

  - 均衡管理

    - 触发条件：

      - 需同时满足BalanceFlagfalse且BMS_GlobalParam.BalanceBMS_STATE_ENABLE 

      - 电压回升：设置BalanceVoltRiseTime防止充放电后立即均衡 

      - 硬件操作： 通过BMS_HalCtrlCellsBalance控制具体电芯均衡使用BMS_BalanceStartTimer设置均衡持续时间 

      - 周期控制：BalanceCycleTime配置单次均衡时长（如5秒或5分钟）

      - 芯片封装：调用BQ769X0_CellBalanceControl实现硬件级均衡 

      - 寄存器操作：通过I2C修改CELL_BAL_VALUE寄存器组（3字节位图控制32节电芯）

  7)信息管理业务

    - 功能定位：信息管理业务属于BMS系统的应用层功能模块，主要负责电池状态信息的格式化输出

  8)通信管理业务

    - 功能定位：处理BMS系统与外部设备的通信交互，包括CAN总线、485等通信协议

  

  

      



