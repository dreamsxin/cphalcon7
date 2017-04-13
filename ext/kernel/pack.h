
/*
  +------------------------------------------------------------------------+
  | Phalcon Framework                                                      |
  +------------------------------------------------------------------------+
  | Copyright (c) 2011-2015 Phalcon Team (http://www.phalconphp.com)       |
  +------------------------------------------------------------------------+
  | This source file is subject to the New BSD License that is bundled     |
  | with this package in the file docs/LICENSE.txt.                        |
  |                                                                        |
  | If you did not receive a copy of the license and are unable to         |
  | obtain it through the world-wide-web, please send an email             |
  | to license@phalconphp.com so we can send you a copy immediately.       |
  +------------------------------------------------------------------------+
  | Authors: Andres Gutierrez <andres@phalconphp.com>                      |
  |          Eduar Carvajal <eduar@phalconphp.com>                         |
  |          ZhuZongXin <dreamsxin@qq.com>                                 |
  +------------------------------------------------------------------------+
*/


#ifndef PHALCON_KERNEL_PACK_H
#define PHALCON_KERNEL_PACK_H

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/**
 * 通过指定格式串对数据序列化。
 * 格式串中每一项为 “$（头部写入数据长度）+ size（使用多少字节存储数据长度） + type（类型） + num（个数） + : + size（使用多少字节存储个数）”。
 * 类型为大写，则为数组，且通过传递参数指定个数。有个数，表示是数组。
 * 特殊字符 $ 在第一个字符出现表示在头部写入整个数据长度（不包括此头部）。$后跟数字表示用几个字节存储数据长度。
 * 
 * <code>
 *   int a[3] = {1, 2, 3};
 *   char b[3] = {'a', 'b', 'c'};
 *   pack(buf, 128, "i3sc", a, "haha", 'b');
 *   pack(buf, 128, "Isc", 3, a, "haha", 'b');
 *   pack(buf, 128, "$4Isc", 3, a, "haha", 'b');
 *   pack(buf, 128, "sc3", "haha", b);
 *   pack(buf, 128, "sC", "haha", 3, b);
 *   
 *   char c[16];
 *   int dlen;
 *   char d[3];
 *   unpack(buf, 128, "sC", c, dlen, d);
 * </code>
 * 
 * @param buf
 * @param buflen
 * @param format 格式串
 * @return 正常返回整个数据包长度, 出错返回-1
 */
int pack(char *buf, int buflen, char *format, ...)
{
    int uselen = 0; 
    va_list arg;
    int i = 0;
    int numsize = 1; // array/string len field size, max is 4
    int headlen = 0;

    //DINFO("format:%s\n", format);
    
    if (format[0] == '$') {
        headlen = format[1] - 48;
        if (headlen != 1 && headlen != 2 && headlen != 4) 
            return -1;
        i += 2;
        uselen += headlen;
    }

    va_start(arg, format);
    while (format[i] != 0) {
        //DINFO("format i:%d, %c\n", i, format[i]);
        int nexti = i + 1;
        while (format[nexti] >= '0' && format[nexti] <= '9') {
            nexti++;
        }
        if (format[nexti] == ':') {
            numsize = atoi(&format[nexti+1]);
            if (numsize == 0 || numsize > 4) {  // size error
                DERROR("numsize error:%d\n", numsize);
                return -1;
            }
            nexti++; // skip :
            // skip numbers
            while (format[nexti] >= '0' && format[nexti] <= '9') {
                nexti++;
            }
        }
        uint32_t num = atoi(&format[i+1]);
        //DINFO("num:%d, numsize:%d, uselen:%d\n", num, numsize, uselen);
        switch(format[i]) {
        case 'i': { // int, 32bit
            if (num > 0) {
                memcpy(buf + uselen, &num, numsize);
                uselen += numsize;
                
                int *vals = va_arg(arg, int *);
                memcpy(buf + uselen, vals, sizeof(int)*num);
                uselen += sizeof(int) * num;
            }else{
                int val = va_arg(arg, int);
                memcpy(buf + uselen, &val, sizeof(int));
                uselen += sizeof(int);
            }
            break;
        }
        case 'I': { // int, 32b, array, and array length pass by a param
            num = va_arg(arg, int);
            memcpy(buf + uselen, &num, numsize);
            uselen += numsize;
                
            //DINFO("num:%d\n", num);
            int *vals = va_arg(arg, int *);
            memcpy(buf + uselen, vals, sizeof(int)*num);
            uselen += sizeof(int) * num;
            break;
        }
        case 'f': { // float, 32bit
            if (num > 0) {
                memcpy(buf + uselen, &num, numsize);
                uselen += numsize;
                
                float *vals = va_arg(arg, float *);
                memcpy(buf + uselen, vals, sizeof(float)*num);
                uselen += sizeof(float) * num;
            }else{
                float val = va_arg(arg, double);
                //DINFO("copy float:%f\n", val);
                memcpy(buf + uselen, &val, sizeof(float));
                uselen += sizeof(float);
            }
            break;
        }
        case 'F': { // int, 32b, array, and array length pass by a param
            num = va_arg(arg, int);
            memcpy(buf + uselen, &num, numsize);
            uselen += numsize;
                
            //DINFO("num:%d\n", num);
            float *vals = va_arg(arg, float *);
            memcpy(buf + uselen, vals, sizeof(float)*num);
            uselen += sizeof(float)* num;
            break;
        }

        case 'l': { // long, 64bit
            if (num > 0) {
                memcpy(buf + uselen, &num, numsize);
                uselen += numsize;
                
                int64_t *vals = va_arg(arg, int64_t *);
                memcpy(buf + uselen, vals, sizeof(int64_t)*num);
                uselen += sizeof(int64_t) * num;
            }else{
                int64_t val = va_arg(arg, int64_t);
                memcpy(buf + uselen, &val, sizeof(int64_t));
                uselen += sizeof(int64_t);
            }
            break;
        }
        case 'L': {
            num = va_arg(arg, int);
            memcpy(buf + uselen, &num, numsize);
            uselen += numsize;
                
            int64_t *vals = va_arg(arg, int64_t *);
            memcpy(buf + uselen, vals, sizeof(int64_t)*num);
            uselen += sizeof(int64_t) * num;
            break;
        }
        case 'd': { // double, 64bit
            if (num > 0) {
                memcpy(buf + uselen, &num, numsize);
                uselen += numsize;
                
                double *vals = va_arg(arg, double *);
                memcpy(buf + uselen, vals, sizeof(double)*num);
                uselen += sizeof(double) * num;
            }else{
                double val = va_arg(arg, double);
                memcpy(buf + uselen, &val, sizeof(double));
                uselen += sizeof(double);
            }
            break;
        }
        case 'D': {
            num = va_arg(arg, int);
            memcpy(buf + uselen, &num, numsize);
            uselen += numsize;
                
            double *vals = va_arg(arg, double *);
            memcpy(buf + uselen, vals, sizeof(double)*num);
            uselen += sizeof(double) * num;
            break;
        }

        case 's': { // string
            char *val = va_arg(arg, char *);
            int vlen = strlen(val) + 1;
            memcpy(buf + uselen, val, vlen);
            uselen += vlen;
            break;
        }
        case 'c': { // char, 8bit
            if (num > 0) {
                memcpy(buf + uselen, &num, numsize);
                uselen += numsize;
                
                char *vals = va_arg(arg, char *);
                memcpy(buf + uselen, vals, sizeof(char)*num);
                uselen += sizeof(char) * num;
            }else{
                char val = va_arg(arg, int);
                memcpy(buf + uselen, &val, sizeof(char));
                uselen += sizeof(char);
            }
            break;
        }
        case 'C': {
            num = va_arg(arg, int);
            memcpy(buf + uselen, &num, numsize);
            uselen += numsize;
                
            char *vals = va_arg(arg, char *);
            memcpy(buf + uselen, vals, sizeof(char)*num);
            uselen += sizeof(char) * num;
            break;
        }
        case 'h': { // short, 16bit
            if (num > 0) {
                memcpy(buf + uselen, &num, numsize);
                uselen += numsize;
                
                int16_t *vals = va_arg(arg, int16_t*);
                memcpy(buf + uselen, vals, sizeof(int16_t)*num);
                uselen += sizeof(int16_t) * num;
            }else{
                int16_t val = va_arg(arg, int);
                memcpy(buf + uselen, &val, sizeof(int16_t));
                uselen += sizeof(int16_t);
            }
            break;
        }
        case 'H': {
            num = va_arg(arg, int);
            memcpy(buf + uselen, &num, numsize);
            uselen += numsize;
            
            int16_t *vals = va_arg(arg, int16_t*);
            memcpy(buf + uselen, vals, sizeof(int16_t)*num);
            uselen += sizeof(int16_t) * num;
            break;
        }
        default:
            return -1;
            break;
        }
        if (buflen > 0 && uselen > buflen)
            return -1;
        numsize = 1; // set default
        i = nexti;
    }
    va_end(arg);
    if (headlen > 0) {
        unsigned int blen = uselen - headlen;  
        memcpy(buf, &blen, headlen);
    }
    return uselen; 
}

/**
 * 反序列化
 * @param buf 序列化数据
 * @param buf 序列化数据长度
 * @param format 格式串
 * @return 正常返回反序列化的数据长度，错误返回-1
 */
int unpack(char *buf, int buflen, char *format, ...)
{
    int uselen = 0; 
    va_list arg;
    int i = 0, n;
    int numsize = 1; // array/string len field size, max is 4
    char ch;
    
    va_start(arg, format);
    while (format[i]) {
        //DINFO("unpack %d\n", i);
        int nexti = i + 1;
        while (format[nexti] >= '0' && format[nexti] <= '9') {
            nexti++;
        }
        if (format[nexti] == ':') {
            numsize = atoi(&format[nexti+1]);
            if (numsize == 0 || numsize > 4) {  // size error
                DERROR("numsize error:%d\n", numsize);
                return -1;
            }
            nexti++; // skip :
            // skip numbers
            while (format[nexti] >= '0' && format[nexti] <= '9') {
                nexti++;
            }
        }
        uint32_t num = atoi(&format[i+1]);

        ch = format[i];
        if (ch <= 'Z') {
            ch += 32;
            num = 10; // 数字是多少不重要，只要大于0
        }
        //DINFO("num:%d, numsize:%d, uselen:%d\n", num, numsize, uselen);
        switch(ch) {
        case 'f': {
            if (num > 0) {
                if (numsize == 1) {
                    uint8_t *sz = va_arg(arg, uint8_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint8_t);
                    num = *sz;
                }else{
                    uint32_t *sz = va_arg(arg, uint32_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint32_t);
                    num = *sz;
                }
                float *vals = va_arg(arg, float *);
                memcpy(vals, buf+uselen, sizeof(float)*num);
                uselen += sizeof(float) * num;
            }else{
                float *val = va_arg(arg, float*);
                memcpy(val, buf+uselen, sizeof(float));
                uselen += sizeof(float);
            }
            break;

        }
        case 'i': {
            if (num > 0) {
                if (numsize == 1) {
                    uint8_t *sz = va_arg(arg, uint8_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint8_t);
                    num = *sz;
                }else{
                    uint32_t *sz = va_arg(arg, uint32_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint32_t);
                    num = *sz;
                }
                int *vals = va_arg(arg, int *);
                memcpy(vals, buf+uselen, sizeof(int)*num);
                uselen += sizeof(int) * num;
            }else{
                int *val = va_arg(arg, int*);
                memcpy(val, buf+uselen, sizeof(int));
                uselen += sizeof(int);
            }
            break;
        }
        case 'd':
        case 'l': {
            if (num > 0) {
                if (numsize == 1) {
                    uint8_t *sz = va_arg(arg, uint8_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint8_t);
                    num = *sz;
                }else{
                    uint32_t *sz = va_arg(arg, uint32_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint32_t);
                    num = *sz;
                }
                int64_t *vals = va_arg(arg, int64_t *);
                memcpy(vals, buf+uselen, sizeof(int64_t)*num);
                uselen += sizeof(int64_t) * num;
            }else{
                int64_t *val = va_arg(arg, int64_t*);
                memcpy(val, buf+uselen, sizeof(int64_t));
                uselen += sizeof(int64_t);
            }
            break;
        }
        case 's': {
            char *val = va_arg(arg, char *); 
            n = 0;
            char *data = buf + uselen;
            while (data[n] != 0) {
                val[n] = data[n];
                n++;
            }
            val[n] = 0;
            uselen += n + 1;
            break;
        }
        case 'c': {
            if (num > 0) {
                if (numsize == 1) {
                    uint8_t *sz = va_arg(arg, uint8_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint8_t);
                    num = *sz;
                }else{
                    uint32_t *sz = va_arg(arg, uint32_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint32_t);
                    num = *sz;
                }
                char *vals = va_arg(arg, char *);
                memcpy(vals, buf+uselen, sizeof(char)*num);
                uselen += sizeof(char) * num;
            }else{
                char *val = va_arg(arg, char*);
                memcpy(val, buf+uselen, sizeof(char));
                uselen += sizeof(char);
            }
            break;

        }
        case 'h': {
            if (num > 0) {
                if (numsize == 1) {
                    uint8_t *sz = va_arg(arg, uint8_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint8_t);
                    num = *sz;
                }else{
                    uint32_t *sz = va_arg(arg, uint32_t *);
                    memcpy(sz, buf+uselen, numsize);
                    uselen += sizeof(uint32_t);
                    num = *sz;
                }
                int16_t *vals = va_arg(arg, int16_t *);
                memcpy(vals, buf+uselen, sizeof(int16_t)*num);
                uselen += sizeof(int16_t) * num;
            }else{
                int16_t *val = va_arg(arg, int16_t*);
                memcpy(val, buf+uselen, sizeof(int16_t));
                uselen += sizeof(int16_t);
            }
            break;
        }
        default:
            return -1;
        }
        if (buflen > 0 && uselen > buflen)
            return -1;

        numsize = 1;
        i = nexti;
    }
        
    va_end(arg);

    return uselen;
}

#endif /* PHALCON_KERNEL_PACK_H */
