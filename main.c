//
//  main.c
//  cptp
//
//  Created by 李杰 on 16/5/6.
//  Copyright (c) 2016年 李杰. All rights reserved.
//

#include <stdio.h>
#include "cptp.h"

cptp_single_point t[] = {
    {1, 0xF},
    {2, 0xFF},
    {3, 0xFFF},
    {4, 0xFFFF},
    {5, 0xFFFFF},
    {6, 0xFFFFFF},
    {7, 0xFFFFFFF},
    {8, 0x12345678},
    {9, 0x12345678},
    {10, 0x12345678},
    {11, 0x12345678},
    {12, 0x12345678},
    {13, 0x12345678},
    {14, 0x12345678},
    {15, 0x12345678},
    {16, 0x12345678},
    {17, 0x12345678},
    {18, 0x12345678},
    {19, 0x12345678},
    {20, 0x12345678},
    {21, 0x12345678},
    {22, 0x12345678},
    {23, 0x12345678},
    {24, 0x12345678},
    {25, 0x12345678},
    {26, 0x12345678},
    {27, 0x12345678},
    {28, 0x12345678},
    {29, 0x12345678},
    {30, 0x12345678},
    {31, 0x12345678},
    {32, 0x12345678},
    {33, 0x12345678},
    {18, 0x12345678},
    {19, 0x12345678},
    {20, 0x12345678},
    {21, 0x12345678},
    {22, 0x12345678},
    {23, 0x12345678},
    {24, 0x12345678},
    {0x0000, 0},
};

int on_read(cptp_read_request *request)
{
    int i = 0;
    printf("read request count=%d\n    ID=[BEGIN,\n    ", request->head.count);
    for ( i = 0; i < request->head.count; i ++ ) {
        printf("%d(%04Xh), ", request->id[i], request->id[i]);
    }
    printf("END]\n");
    return 0;
}

int on_write(cptp_write_request *request)
{
    int i = 0;
    printf("write request count=%d\n    ID=[BEGIN,\n    ", request->head.count);
    for ( i = 0; i < request->head.count; i ++ ) {
        printf("%4d(%04Xh)=%8Xh, ", request->points[i].id, request->points[i].id, request->points[i].v);
        if ( i != 0 && !(i % 4) ) printf("\n    ");
    }
    printf("END]\n");
    return 0;
}

int on_refresh(cptp_refresh_request *request)
{
    int i = 0;
    printf("refresh count=%d\n    ID=[BEGIN\n    ", request->head.count);
    for ( i = 0; i < request->head.count; i ++ ) {
        printf("%4d(%04Xh)=0x%-8X, ", request->points[i].id, request->points[i].id, request->points[i].v);
        if ( i != 0 && !(i % 4) ) printf("\n    ");
    }
    printf("END]\n");
    return 0;
}


int on_read_ack(cptp_rw_ack *ack)
{
    int i = 0;
    printf("read ack count=%d\n    ID=[BEGIN\n    ", ack->head.count);
    for ( i = 0; i < ack->head.count; i ++ ) {
        printf("%4d(%04Xh)=0x%-8X, ", ack->points[i].id, ack->points[i].id, ack->points[i].v);
        if ( i != 0 && !(i % 4) ) printf("\n    ");
    }
    printf("END]\n");
    return 0;
}

int on_write_ack(cptp_rw_ack *ack)
{
    int i = 0;
    printf("write ack count=%d\n    ID=[BEGIN,\n    ", ack->head.count);
    for ( i = 0; i < ack->head.count; i ++ ) {
        printf("%4d(%04Xh)=0x%-8X, ", ack->points[i].id, ack->points[i].id, ack->points[i].v);
        if ( i != 0 && !(i % 4) ) printf("\n    ");
    }
    printf("END]\n");
    return 0;
}

int on_refresh_ack(cptp_refresh_ack *ack)
{
    int i = 0;
    printf("refresh ack count=%d\n    ID=[BEGIN\n    ", ack->head.count);
    for ( i = 0; i < ack->head.count; i ++ ) {
        printf("%4d(%04Xh)=0x%-8X, ", ack->points[i].id, ack->points[i].id, ack->points[i].v);
        if ( i != 0 && !(i % 4) ) printf("\n    ");
    }
    printf("END]\n");
    return 0;
}


int main(int argc, const char * argv[]) {
    // insert code here...
    struct cptp p;
    int len;
    cptp_header_section *s;
    u8 rx_buff[1024], tx_buff[1024];
    u8 f[] = {
        0x68,
        0x00, // DES1
        0x00, // DES2
        0x00, // SRC1
        0x00, // SRC2
        0x0D, // LEN_L
        0x01, // C
        0x00, // seq
        0x02, // count
        0x00, // id
        0xFF, // id
        0xFF, // v
        0xFF, // v
        0xFF, // v
        0xFF, // v
        0x00, // id
        0xFF, // id
        0xFF, // v
        0xFF, // v
        0xFF, // v
        0xFF, // v
        0x00,
        0x16
    };
    u8 ack[] = {
        0x68,
        0x00, // DES1
        0x00, // DES2
        0x00, // SRC1
        0x00, // SRC2
        0x0E, // LEN_L
        0x01, // C
        0x00, // seq
        0x00, // errno
        0x02, // count
        0x00, // id
        0xFF, // id
        0xFF, // v
        0xFF, // v
        0xFF, // v
        0xFF, // v
        0x00, // id
        0xFF, // id
        0xFF, // v
        0xFF, // v
        0xFF, // v
        0xFF, // v
        0x00,
        0x16
    };
    u8 tack[1024] = {0xFF};
    memset((void *)tack, 0xFF, 200);

    cptp_init(&p, 0, rx_buff, 1024, tx_buff, 1024);
    p.on_read = on_read;
    p.on_read_ack = on_read_ack;
    p.on_write = on_write;
    p.on_write_ack = on_write_ack;
    p.on_refresh = on_refresh;
    p.on_refresh_ack = on_refresh_ack;

    int i = 0;
    cptp_patch_request_header(tack, 0, 12, FUNC_WR_POINTS, 1, WITHOUT_TSP);
    for ( i = 0; t[i].id; i ++ ) {
        cptp_patch_point(tack, t[i].id, t[i].v);
    }
    len = cptp_patch_tail(tack);
    cptp_bytes_push(&p, tack, len);
    
    u8 pool[1024];
    memset(pool, '*', 1024);
    cptp_patch_request_header(pool, 0, 12, FUNC_RD_POINTS, 1, WITH_TSP);
    cptp_patch_id(pool, 100);
    cptp_patch_id(pool, 110);
    cptp_patch_id(pool, 120);
    cptp_patch_id(pool, 130);
    len = cptp_patch_tail(pool);
    cptp_bytes_push(&p, pool, len);

    cptp_patch_ack_header(pool, 0, 12, FUNC_RD_POINTS, 1, 0, WITH_TSP);
    for ( i = 0; i < 32; i ++ ) {
        cptp_patch_point(pool, i + 100, i * i);
    }
    len = cptp_patch_tail(pool);
    cptp_bytes_push(&p, pool, len);

    cptp_patch_ack_header(pool, 0, 12, FUNC_REFRSH_POINTS, 1, 0, WITHOUT_TSP);
    cptp_patch_point(pool, 100, 0x123);
    cptp_patch_point(pool, 110, 0x456);
    cptp_patch_point(pool, 120, 0x789);
    cptp_patch_point(pool, 130, 0xABC);
    len = cptp_patch_tail(pool);
    cptp_bytes_push(&p, pool, len);

    cptp_patch_request_header(pool, 0, 12, FUNC_REFRSH_POINTS, 1, WITH_TSP);
    cptp_patch_point(pool, 100, 0x123);
    cptp_patch_point(pool, 110, 0x456);
    cptp_patch_point(pool, 120, 0x789);
    cptp_patch_point(pool, 130, 0xABC);
    len = cptp_patch_tail(pool);
    cptp_bytes_push(&p, pool, len);

    printf("\nHello, World!\n");
    return 0;
}
