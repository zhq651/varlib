
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _NGX_ARRAY_H_INCLUDED_
#define _NGX_ARRAY_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>


struct ngx_array_s {
    void        *elts; 		//Ԫ���׵�ַ
    ngx_uint_t   nelts;		//�Ѿ������ȥ��Ԫ�ظ���
    size_t       size;		//Ԫ�ش�С
    ngx_uint_t   nalloc;	//����Ĵ�С
    ngx_pool_t  *pool;		//�ڴ��
};


ngx_array_t *ngx_array_create(ngx_pool_t *p, ngx_uint_t n, size_t size);
void ngx_array_destroy(ngx_array_t *a);
void *ngx_array_push(ngx_array_t *a);
void *ngx_array_push_n(ngx_array_t *a, ngx_uint_t n);


#endif /* _NGX_ARRAY_H_INCLUDED_ */
