// fs.h - 文件系统命令接口头文件
#ifndef FS_H
#define FS_H

/* 文件系统命令 */
int fs_cmd_mkfile(int argc, char *argv[]);
int fs_cmd_mkdir(int argc, char *argv[]);
int fs_cmd_ls(int argc, char *argv[]);
int fs_cmd_rm(int argc, char *argv[]);
int fs_cmd_cat(int argc, char *argv[]);
int fs_cmd_write(int argc, char *argv[]);
int fs_cmd_df(int argc, char *argv[]);

#endif