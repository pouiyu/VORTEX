// cmd_exec.h - 统一命令执行头文件
#ifndef CMD_EXEC_H
#define CMD_EXEC_H

/* 命令执行函数
 * cmd: 命令名
 * args: 命令参数
 * show_output: 是否显示执行过程信息（1=显示，0=静默）
 * 返回: 1=成功，0=失败
 */
int cmd_execute(const char *cmd, const char *args, int show_output);

#endif