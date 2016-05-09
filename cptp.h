//
//  cptp.h
//  cptp
//
//  Created by 李杰 on 16/5/6.
//  Copyright (c) 2016年 李杰. All rights reserved.
//

#ifndef __cptp__cptp__
#define __cptp__cptp__

#include <stdio.h>

#ifndef u8
    #define u8 unsigned char
#endif
#ifndef u16
    #define u16 unsigned short
#endif
#ifndef u32
    #define u32 unsigned int
#endif

#pragma pack(1)
typedef struct{
    u32 ms:10;   // 0-999
    u32 sec:6;   // 0-59
    u32 min:6;   // 0-59
    u32 hour:5;  // 0-23
    u32 day:5;   // 1-31
}cptp_tsp_section;

typedef struct{
    u8 reserved:1; // set to 1 always
    u8 with_tsp:1; // 0: without timestamp, 1: with a timestamp section.
    u8 type:1;     // 0: request, 1: acknowledge
    u8 func:5;
    // 00000b: 无效, 0h
    // 00001b: 读点, 1h
    // 00010b: 写点, 2h
    // 00011b: 主动刷新, 3h
    // 其他： 保留待扩展
}cptp_control_section;
// 帧类型
#define FRAME_REQUEST                    0
#define FRAME_ACKNOWLEDGE                1
// 是否需要时戳
#define WITHOUT_TSP                      0
#define WITH_TSP                         1
// 帧功能码
#define FUNC_RD_POINTS             1
#define FUNC_WR_POINTS             2
#define FUNC_REFRSH_POINTS         3

typedef struct {
    u16 des;                      // 目标地址
    u16 src;                      // 源地址
}cptp_address_section;

typedef struct {
    u8 bof;                      // 0x86
    cptp_address_section addr;       // 地址域
    u8 len;                      // 数据域长度
    cptp_control_section C;          // 控制域
    u8  seq;                     // 帧序号
    u8 count:5;                  // 0-31 包含数据点的个数
    u8 reserved:3;               // 保留待扩展
}cptp_header_section;
#define MAX_POINT_NR   31
#define BF  0x86
#define EF  0x16
#define header_size sizeof(cptp_header_section)

typedef struct {
    u8 *buff;               // 数据接收缓冲区
    size_t count;              // 缓冲区中实际字节数
    size_t size;               // 数据缓冲区大小
}cptp_buff;

// 单点结构
typedef struct {
    u16 id;     // 点码
    u32 v;     // 点值
}cptp_single_point;

// 多点结构
typedef struct {
    cptp_single_point points[0];  // 点表
}cptp_multi_points;

// 读点数据
typedef struct {
    cptp_header_section head;        // 帧头
    u16 id[0];                    // 数据点ID
}cptp_read_request;

// 写点数据
typedef struct {
    cptp_header_section head;        // 帧头
    cptp_single_point points[0];         // 单点信息组
}cptp_write_request;
typedef cptp_write_request cptp_refresh_request;

// 写点数据应答
typedef struct {
    cptp_header_section head;        // 帧头
    u8 errorno;                      // 错误号
    cptp_single_point points[0];        // 单点信息组
}cptp_rw_ack;
typedef cptp_rw_ack cptp_refresh_ack;

#pragma pack()
struct cptp {
    u16 addr;              // 本地地址
    u8 seq;                // 帧序列

    cptp_buff rx;            // 接收缓冲区
    cptp_buff tx;            // 发送缓冲区

    int (*on_read)(cptp_read_request *request);
    int (*on_write)(cptp_write_request *request);
    int (*on_refresh)(cptp_refresh_request *request);

    int (*on_read_ack)(cptp_rw_ack *ack);
    int (*on_write_ack)(cptp_rw_ack *ack);
    int (*on_refresh_ack)(cptp_refresh_ack *ack);
};

int cptp_init(struct cptp *p, u16 addr, void *rx, size_t s_rx, void *tx, size_t s_tx);
u8 cptp_check_sum(u8 *b, size_t s);

int cptp_bytes_push(struct cptp *, const void *, size_t);
int cptp_bytes_pull(struct cptp *, void *, size_t *);

int cptp_send(struct cptp *p, void *s, size_t l);

int cptp_patch_request_header(void *f, u16 des, u16 src, u8 func, u8 seq, int need_tsp);
int cptp_patch_ack_header(void *f, u16 des, u16 src, u8 func, u8 seq, u8 errorno, int need_tsp);
int cptp_patch_id(void *f, u16 id);
int cptp_patch_point(void *f, u16 id, u32 v);
int cptp_patch_tail(void *f);

#endif /* defined(__cptp__cptp__) */
