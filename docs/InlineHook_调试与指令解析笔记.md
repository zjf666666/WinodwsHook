# InlineHook 调试与指令解析笔记

本笔记汇总近期排查中涉及的核心知识点，便于后续复用与对照。

## 1. DllMain 与 LastError
- DllMain 处于 Loader Lock 内执行。成功路径（返回 TRUE）不应残留非 0 的 LastError；若中途调用了会改写 LastError 的 API，应在返回前恢复原值。
- 失败路径（返回 FALSE）可以设置一个有意义的错误码，方便 LoadLibrary/GetLastError 诊断。
- Hook 中的惯例：在调用原函数后立刻保存 err=GetLastError()，做完自有逻辑后，返回前 SetLastError(err)。

## 2. 调试断点“只能落在某一行”的原因
- 源行与机器码是多对多映射。小的 if 块可能被编译器合并成一段指令，并统一映射到其中一行，导致其他行“无可执行代码关联”，下不了断点。
- 验证方式：在反汇编窗口对关键指令地址下断；或使用函数断点绕过行级映射。
- 提升可调试性（仅用于分析）：关闭该文件优化、/Zi 或 /ZI、/Oy-、关闭“仅我的代码”。

## 3. Inline Hook 安装验证清单
- 目标入口是否被写入 E9 相对跳转（常见方案）。
- 某些 API（如 CreateFileW）入口可能是转发桩（FF 25 [ptr] → KernelBase 实现），属正常路径。
- 在真实实现地址下断以确认“原函数确实执行”。
- 原函数指针 typedef 需与实际调用约定一致（如 WINAPI/__stdcall）。

## 4. Trampoline 构造要点（易错）
- 先解析并拷贝“被劫持起始处的完整指令”直到累计长度 ≥ 跳转指令自身长度（常用 5 字节近跳或 6 字节绝对跳转替代策略），得到 stolenLen。
- 在 trampoline 末尾生成“无条件跳回 原函数+stolenLen”的跳转。错误示例：把回跳地址写成 原函数+0x0A，而正确应为 原函数+stolenLen（例如首条 FF 25 长度为 6，则应 +0x06）。
- 处理相对分支/调用时需重定位（InstructionRelocator），保证在 trampoline 中仍然跳到正确目标。

## 5. 指令解析流程总览（32 位）
总长度 = prefix_len + opcode_len + (ModRM?1:0) + (SIB?1:0) + disp_len + imm_len。

### 5.1 前缀（prefix）
- 只能出现在最前端，连续出现，直到遇到非前缀字节停止。
- 传统前缀分组（last-wins）：
  - 组1：F0(LOCK), F2(REPNE/NZ), F3(REP/REPE)
  - 组2：段覆盖 2E(CS)/36(SS)/3E(DS)/26(ES)/64(FS)/65(GS)
  - 组3：66 操作数大小覆盖（operand-size override）
  - 组4：67 地址大小覆盖（address-size override）
- 关键规则：只把“实际读到的前缀字节数”累计到 prefix_len；没有前缀时 prefix_len=0，绝不能默认固定偏移（如 8）。
- 注意：在 x86-32 下，0x40–0x4F 是 INC/DEC r32 不是前缀；VEX（C4/C5）若未实现 AVX 解码，不要误判为前缀。

### 5.2 操作码（opcode）
- 一字节主表：直接据此判定是否需要 ModRM/imm/disp。
- 扩展表：0F xx（二字节），以及 0F 38 xx、0F 3A xx（三字节）。
- 前缀对语义的影响：
  - 66/67 影响操作数/地址大小，进而影响立即数与位移的位宽选择。
  - F2/F3/66 在部分 SSE 指令族中选择同一 0F xx 编码下的不同指令变体（如 0F 10：无前缀=MOVUPS，66=MOVUPD，F3=MOVSS，F2=MOVSD）。

### 5.3 ModRM / SIB / 位移
- 是否需要 ModRM 由 opcode（或其扩展）决定。
- 当地址大小为 32 位且 ModRM.r/m=100b 且 mod≠3 时，需要 SIB；SIB 中再决定是否需要位移与位移宽度。
- 位移宽度取决于 mod、r/m（以及地址大小覆盖 67）。

### 5.4 立即数（immediate）
- 宽度由指令和 66 决定（例如很多算术/逻辑的 imm 在 66 下从 32→16）。

## 6. Hook 中的 LastError 规范
- 调用原函数后立即读取 err=GetLastError()；完成日志/判断等后在 return 前 SetLastError(err)。
- 基本用例：不存在路径应返回 INVALID_HANDLE_VALUE 且 LastError=ERROR_PATH_NOT_FOUND；存在路径返回有效句柄且保持原错误码不被污染。

## 7. 调试技巧与建议
- 反汇编窗口对具体指令地址下断，精准控制；函数断点绕过行映射。
- 逐步验证链路：目标入口（E9）→ 转发桩 → 内部实现命中 → 观察返回值与 LastError。
- 对单文件临时关闭优化，/Zi 或 /ZI、/Oy-、关闭“仅我的代码”，便于步进与断点定位。

以上规则直接对应并解释了我们遇到的现象：
- prefix() 绝不能固定推进偏移；
- trampoline 的回跳必须使用 stolenLen；
- DllMain/Hook 中要避免污染 LastError；
- 断点集中在某行是行-指令映射的正常表现。