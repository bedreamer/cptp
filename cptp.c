//
//  cptp.c
//  cptp
//
//  Created by 李杰 on 16/5/6.
//  Copyright (c) 2016年 李杰. All rights reserved.
//
#include <string.h>
#include <sys/time.h>
#include "cptp.h"

int cptp_init(struct cptp *p, u16 addr, void *rx, size_t s_rx, void *tx, size_t s_tx)
{
    if ( !p ) return -1;
    
    p->addr = addr;
    p->rx.buff = (u8*)rx;
    p->rx.size = s_rx;
    p->rx.count = 0;
    memset(rx, 0, s_rx);

    p->tx.buff = (u8*)tx;
    p->tx.size = s_tx;
    p->tx.count = 0;
    memset(tx, 0, s_tx);

    return 0;
}

u8 cptp_check_sum(u8 *b, size_t s)
{
    u8 sum = 0;

    while ( s -- ) {
        sum = sum + (*b);
        b ++;
    }
    return sum;
}

void cptp_find_header(struct cptp *p, u8 *b, size_t s)
{
}

int cptp_rx_request(struct cptp *p, size_t len);
int cptp_rx_ack(struct cptp *p, size_t len);
int cptp_bytes_push(struct cptp *p, const void *ib, size_t s)
{
    cptp_header_section *head;
    u16 need_len = 0;
    u8 sum;

    if ( !p || ! ib || ! s ) return -1;
    head = (cptp_header_section *)(void *)p->rx.buff;

    if ( p->rx.count == 0 ) {
        memcpy(p->rx.buff, ib, s);
        p->rx.count = p->rx.count + s;
    } else {
        memcpy(&p->rx.buff[p->rx.count], ib, s);
        p->rx.count = p->rx.count + s;
    }
    
    if ( head->bof != 0x68 ) { // 头不符
        memset(p->rx.buff, 0, p->rx.count);
        p->rx.count = 0;
        return -1;
    }
    if ( header_size > p->rx.count ) {
        return 0;
    }
    if ( head->addr.des != p->addr ) { // 目标地址不符
        memset(p->rx.buff, 0, p->rx.count);
        p->rx.count = 0;
        return -1;
    }
    need_len = header_size + head->len + 2;
    if ( need_len < p->rx.count ) { // 总长度不符
        return 0;
    }
    if ( p->rx.buff[need_len-1] != 0x16 ) { // 结束符不符
        memset(p->rx.buff, 0, p->rx.count);
        p->rx.count = 0;
        return -1;
    }
    sum = cptp_check_sum(p->rx.buff, need_len-2);
    if ( sum != p->rx.buff[need_len-2] ) { // 校验不符
        memset(p->rx.buff, 0, p->rx.count);
        p->rx.count = 0;
        return -1;
    }

    // 收到完整的一帧
    if ( head->C.type == FRAME_REQUEST ) {
        return cptp_rx_request(p, need_len);
    } else if ( head->C.type == FRAME_ACKNOWLEDGE ) {
        return cptp_rx_ack(p, need_len);
    } else return -1;
}

int cptp_rx_request(struct cptp *p, size_t len)
{
    cptp_header_section *head = (cptp_header_section*)p->rx.buff;
    
    if ( head->C.func == FUNC_RD_POINTS ) {
        cptp_read_request *r = (cptp_read_request*)head;
        p->on_read(r);
    } else if ( head->C.func == FUNC_WR_POINTS ) {
        cptp_write_request *w = (cptp_write_request*)head;
        p->on_write(w);
    } else if ( head->C.func == FUNC_REFRSH_POINTS ) {
        cptp_refresh_request *r = (cptp_refresh_request*)head;
        p->on_refresh(r);
    } else {
    }

    memmove(p->rx.buff, &p->rx.buff[len], len);
    p->rx.count = p->rx.count - len;
    return 0;
}

int cptp_rx_ack(struct cptp *p, size_t len)
{
    cptp_header_section *head = (cptp_header_section*)p->rx.buff;
    
    if ( head->C.func == FUNC_RD_POINTS ) {
        cptp_rw_ack *read_ack = (cptp_rw_ack*)head;
        p->on_read_ack(read_ack);
    } else if ( head->C.func == FUNC_WR_POINTS ) {
        cptp_rw_ack *write_ack = (cptp_rw_ack*)head;
        p->on_write_ack(write_ack);
    } else if ( head->C.func == FUNC_REFRSH_POINTS ) {
        cptp_rw_ack *refresh_ack = (cptp_rw_ack*)head;
        p->on_refresh_ack(refresh_ack);
    } else {
    }
    memmove(p->rx.buff, &p->rx.buff[len], len);
    p->rx.count = p->rx.count - len;
    return 0;
}

int cptp_bytes_pull(struct cptp *p, void *ob, size_t *s)
{
    cptp_header_section *head = (cptp_header_section *)p->tx.buff;
    int len = 0;

    if ( ! p->tx.count ) return 0;
    if ( header_size > p->tx.count ) return 0;
    len = header_size + head->len + 2;
    if ( len > p->tx.count ) return 0;
    memcpy(ob, head, len);
    if ( s ) *s = len;

    memmove(head, &p->tx.buff[len], len);
    p->rx.count = p->rx.count - len;
    return len;
}

int cptp_send(struct cptp *p, void *s, size_t l)
{
    memcpy(&p->tx.buff[p->tx.count], s, l);
    p->tx.count = p->tx.count + l;
    return 0;
}

int cptp_patch_header(void *f, u16 des, u16 src, u8 func, u8 type, u8 seq, int need_tsp)
{
    cptp_header_section *head = (cptp_header_section*)f;
    head->bof = 0x68;
    head->addr.des = des;
    head->addr.src = src;
    head->C.reserved = 1;
    head->C.func = func;
    head->C.type = type;
    head->seq = seq;
    head->C.with_tsp = need_tsp ? WITH_TSP : WITHOUT_TSP;
    head->count = 0;
    head->len = 0;
    return sizeof(cptp_header_section);
}

int cptp_patch_request_header(void *f, u16 des, u16 src, u8 func, u8 seq, int need_tsp)
{
    return cptp_patch_header(f, des, src, func, FRAME_REQUEST, seq, need_tsp);
}

int cptp_patch_ack_header(void *f, u16 des, u16 src, u8 func, u8 seq, u8 errorno, int need_tsp)
{
    cptp_rw_ack *head = (cptp_rw_ack*)f;

    cptp_patch_header(f, des, src, func, FRAME_ACKNOWLEDGE, seq, need_tsp);
    head->errorno = errorno;
    head->head.len = head->head.len + sizeof(head->errorno);

    return sizeof(cptp_rw_ack);
}

int cptp_patch_id(void *f, u16 id)
{
    cptp_read_request *head = (cptp_read_request*)f;
    head->id[ head->head.count ++ ] = id;
    head->head.len = head->head.len + sizeof(id);
    return head->head.len;
}

int cptp_patch_request_point(void *f, u16 id, u32 v)
{
    cptp_write_request *head = (cptp_write_request*)f;
    if ( head->head.count + 1 > MAX_POINT_NR ) return head->head.count;
    
    head->points[ head->head.count ].id = id;
    head->points[ head->head.count ].v = v;
    head->head.len = head->head.len + sizeof(head->points[ head->head.count ].id) + sizeof(head->points[ head->head.count ].v);
    head->head.count = head->head.count + 1;

    return head->head.count;
}

int cptp_patch_ack_point(void *f, u16 id, u32 v)
{
    cptp_rw_ack *head = (cptp_rw_ack*)f;
    if ( head->head.count + 1 > MAX_POINT_NR ) return head->head.count;
    
    head->points[ head->head.count ].id = id;
    head->points[ head->head.count ].v = v;
    head->head.len = head->head.len + sizeof(head->points[ head->head.count ].id) + sizeof(head->points[ head->head.count ].v);
    head->head.count = head->head.count + 1;

    return head->head.count;
}

int cptp_patch_point(void *f, u16 id, u32 v)
{
    cptp_rw_ack *head = (cptp_rw_ack*)f;
    if ( head->head.count + 1 > MAX_POINT_NR ) return head->head.count;
    if ( head->head.C.type == FRAME_REQUEST ) return cptp_patch_request_point(f, id, v);
    else return cptp_patch_ack_point(f, id, v);
}

int cptp_patch_tail(void *f)
{
    cptp_header_section *head = (cptp_header_section *)f;
    u8 *p = (u8*)f;
    cptp_tsp_section *tsp = NULL;
    int len = header_size + head->len + (head->C.with_tsp ? sizeof(cptp_tsp_section) : 0);
    printf("%d %d %d\n", (int)header_size, head->len, head->C.with_tsp ? (int)sizeof(cptp_tsp_section) : 0);
    if ( head->C.with_tsp ) {
        struct timeval t;
        tsp = (cptp_tsp_section*)(&p[len - sizeof(cptp_tsp_section) ]);
        gettimeofday(&t, NULL);
        tsp->ms = t.tv_usec / 10000;
        head->len = head->len + sizeof(cptp_tsp_section);
    }
    p[ len ] = cptp_check_sum(f, len);
    p[ len + 1 ] = 0x16;

    return len + 2;
}

