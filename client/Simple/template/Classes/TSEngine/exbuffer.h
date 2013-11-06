#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef EXBUFFER_H_INCLUDED
#define EXBUFFER_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif
    

#define EXTEND_BYTES 512
    

    enum exbuffer_endian
    {
        EXBUFFER_BIG_ENDIAN,
        EXBUFFER_LITTLE_ENDIAN
    };
    

    typedef struct exbuffer_value
    {
        unsigned char headLen;
        enum exbuffer_endian endian;
        size_t readOffset;
        size_t putOffset;
        size_t dlen;
        unsigned char* buffer;
        size_t bufferlen;
        size_t packetLen;
        unsigned char *packet;
        
        unsigned char *headBytes;
        union HeadBytesS
        {
            unsigned char bytes[2];
            unsigned short val;
        } headS;
        
        union HeadBytesL
        {
            unsigned char bytes[4];
            unsigned long val;
        } headL;
        
        void (*recvHandle)(unsigned char*, size_t);
    } exbuffer_t;
    
    //////////////////////////////////////////////////////////////////////////////////////////
    

    exbuffer_t* exbuffer_new();
    
    void exbuffer_free(exbuffer_t** value);
    
    void exbuffer_printHex(unsigned char* bytes,unsigned short len);
    
    void exbuffer_dump(exbuffer_t* value,unsigned short len);
    
    size_t exbuffer_getLen(exbuffer_t* value);
    
    void exbuffer_put(exbuffer_t* value, unsigned char* buffer,size_t offset,size_t len);
    
    //////////////////////////////////////////////////////////////////////////////////////////
    
    static unsigned char HOST_ENDIAN_LITTLE = 0;
    
    void check_host_endian()
    {
        if((HOST_ENDIAN_LITTLE & 0x01) == 0x01)
        {
            return;
        }
        HOST_ENDIAN_LITTLE = HOST_ENDIAN_LITTLE|0x01;
        unsigned short int i =0x1234;
        unsigned char* p = (unsigned char*)&i;
        if(*p == 0x12)
        {
            //printf(">>big endian\n");
            HOST_ENDIAN_LITTLE = HOST_ENDIAN_LITTLE ^ (~HOST_ENDIAN_LITTLE & (0x00<<1));
        }
        else
        {
            //printf(">>little endian\n");
            HOST_ENDIAN_LITTLE = HOST_ENDIAN_LITTLE ^ (~HOST_ENDIAN_LITTLE & (0x01<<1));
        }
        //free(p);
        p = NULL;
    }
    
    unsigned long _ntohl(unsigned long x,enum exbuffer_endian endian)
    {
        check_host_endian();
        if(
           (endian == EXBUFFER_BIG_ENDIAN  \
            && ((HOST_ENDIAN_LITTLE & (0x01<<1)) == (0x00<<1))//big endian
            )
           ||(endian == EXBUFFER_LITTLE_ENDIAN  \
              && ((HOST_ENDIAN_LITTLE & (0x01<<1)) == (0x01<<1))//little endian
              )
           )
        {
            return x;
        }
        
        return
        ((unsigned long)(	\
                         (((unsigned long)(x) & (unsigned long)0x000000ffUL) << 24) |	 \
                         (((unsigned long)(x) & (unsigned long)0x0000ff00UL) <<  8) |	 \
                         (((unsigned long)(x) & (unsigned long)0x00ff0000UL) >>  8) |	 \
                         (((unsigned long)(x) & (unsigned long)0xff000000UL) >> 24)));
    }
    
    unsigned short _ntohs(unsigned short x,enum exbuffer_endian endian)
    {
        check_host_endian();
        if(
           (endian == EXBUFFER_BIG_ENDIAN  \
            && ((HOST_ENDIAN_LITTLE & (0x01<<1)) == (0x00<<1))//big endian
            )
           ||(endian == EXBUFFER_LITTLE_ENDIAN  \
              && ((HOST_ENDIAN_LITTLE & (0x01<<1)) == (0x01<<1))//little endian
              )
           )
        {
            return x;
        }
        
        return
        ((unsigned short)(	\
                          (((unsigned short)(x) & (unsigned short)0x00ffU) << 8) |	 \
                          (((unsigned short)(x) & (unsigned short)0xff00U) >> 8)));
    }
    
    exbuffer_t* exbuffer_new()
    {
        unsigned char headLen = 2;
        enum exbuffer_endian endian =  EXBUFFER_BIG_ENDIAN;
        size_t bufferlen = 512;
        exbuffer_t* value;
        value =  (exbuffer_t *)malloc(sizeof (exbuffer_t));
        value->bufferlen = bufferlen;
        value->headLen = headLen;
        value->endian = endian;
        value->readOffset = 0;
        value->putOffset = 0;
        value->dlen = 0;
        value->recvHandle = NULL;
        
        value->packetLen = 512;
        value->packet = (unsigned char *)malloc(value->packetLen);
        
        value->headBytes = (unsigned char *)malloc(4);
        
        value->buffer = (unsigned char*)malloc(value->bufferlen);
        //memset(value->buffer,0,value->bufferlen);
        
        return value;
    };
    
    void exbuffer_free(exbuffer_t** value)
    {
        free ((*value)->packet);
        (*value)->packet = NULL;
        
        free ((*value)->buffer);
        (*value)->buffer = NULL;
        
        free ((*value)->headBytes);
        (*value)->headBytes = NULL;
        
        (*value)->recvHandle = NULL;
        
        free (*value);
        (*value) = NULL;
    };
    
    void exbuffer_printHex(unsigned char* bytes,unsigned short len)
    {
        if(len>50)len=50;
        unsigned short iLoop;
        for(iLoop = 0;iLoop < len;iLoop++)
        {
            printf("%02x ",bytes[iLoop]);
        }
        printf("\n");
    };
    
    void exbuffer_dump(exbuffer_t* value,unsigned short len)
    {
        exbuffer_printHex(value->buffer,len);
    };
    
    size_t exbuffer_getLen(exbuffer_t* value)
    {
        if(value->putOffset>= value->readOffset){ // ------******-------
            return value->putOffset - value->readOffset;
        }
        return value->bufferlen - value->readOffset + value->putOffset; //***-------*********
    };
    
    void exbuffer_proc(exbuffer_t* value)
    {
        unsigned short count = 0;
        size_t i;
        unsigned char rlen = 0;
        
        while(1)
        {
            count++;
            if(count>1000)
            {
                fprintf(stderr,"count>1000\n");
                break;
            }
            if(value->dlen == 0)
            {
                //printf("value->dlen == 0\n");
                if(exbuffer_getLen(value) < value->headLen)
                {
                    break;
                }
                if(value->bufferlen - value->readOffset >= value->headLen)//***********[**]
                {
                    //printf("***********[**]\n");
                    for(i=0;i<value->headLen;i++)
                    {
                        value->headBytes[i] = value->buffer[value->readOffset+i];
                    }
                    value->readOffset += value->headLen;
                    //printf("value->readOffset=%d\n",value->readOffset);
                }
                else //*]**----------********[*
                {
                    //printf("*]**----------********[*\n");
                    for(i = 0;i<(value->bufferlen - value->readOffset);i++)
                    {
                        value->headBytes[i] = value->buffer[value->readOffset+i];
                        rlen++;
                    }
                    value->readOffset = 0;
                    for(i = 0;i<(size_t)(value->headLen - rlen);i++)
                    {
                        value->headBytes[rlen+i] = value->buffer[value->readOffset+i];
                    }
                    value->readOffset += (value->headLen - rlen);
                }
                if(value->headLen==2)
                {
                    value->headS.bytes[0] = value->headBytes[0];
                    value->headS.bytes[1] = value->headBytes[1];
                    value->dlen = _ntohs(value->headS.val,value->endian);
                }
                else
                {
                    value->headL.bytes[0] = value->headBytes[0];
                    value->headL.bytes[1] = value->headBytes[1];
                    value->headL.bytes[2] = value->headBytes[2];
                    value->headL.bytes[3] = value->headBytes[3];
                    value->dlen = _ntohl(value->headL.val,value->endian);
                }
                //exbuffer_printHex(value->headBytes,2);
                //printf("value->dlen=%d\n",value->dlen);
                //printf("value->readOffset=%d\n",value->readOffset);
            }
            
            //printf("value->dlen:%d\n",value->dlen);
            if(exbuffer_getLen(value) >= value->dlen)
            {
                if(value->packetLen<value->dlen)
                {
                    size_t rn1 = value->dlen/EXTEND_BYTES;
                    if(value->dlen%EXTEND_BYTES>0)rn1+=1;
                    size_t ex = rn1 * EXTEND_BYTES;
                    
                    value->packetLen = ex;
                    value->packet = (unsigned char *)realloc(value->packet,value->packetLen);
                }
                
                if(value->readOffset + value->dlen > value->bufferlen)//***---*****
                {
                    size_t len1 = value->bufferlen - value->readOffset;
                    if (len1 > 0)
                    {
                        memcpy(value->packet,value->buffer + value->readOffset,len1);
                    }
                    value->readOffset = 0;
                    
                    size_t len2 = value->dlen - len1;
                    memcpy(value->packet + len1,value->buffer + value->readOffset,len2);
                    value->readOffset += len2;
                }
                else
                {
                    memcpy(value->packet,value->buffer + value->readOffset,value->dlen);
                    value->readOffset += value->dlen;
                }
                size_t dlen = value->dlen;
                value->dlen = 0;
                
                if(value->recvHandle == NULL)
                {
                    //printf("receive packet:%d\n",dlen);
                    //exbuffer_printHex(value->packet,dlen);
                }
                else
                {
                    value->recvHandle(value->packet+2,dlen-2);
                }
                
                if (value->readOffset == value->putOffset)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
    }
    
    /*!< Õ∆ÀÕ“ª∂Œ ˝æ›Ω¯»• */
    void exbuffer_put(exbuffer_t* value, unsigned char* buffer,size_t offset,size_t len)
    {
        //exbuffer_dump(value);
        if(len + exbuffer_getLen(value) > value->bufferlen){
            size_t rn1 = (len + exbuffer_getLen(value))/EXTEND_BYTES;
            if((len + exbuffer_getLen(value))%EXTEND_BYTES>0)rn1+=1;
            size_t ex = rn1 * EXTEND_BYTES;
            size_t exlen = ex - value->bufferlen;
            
            value->bufferlen = ex;
            value->buffer =  (unsigned char*)realloc(value->buffer,value->bufferlen);
            
            if (value->putOffset < value->readOffset) //***** ---********-------
            {
                size_t cpylen;
                size_t cpystctstart;
                size_t cpydeststart;
                if (value->putOffset <= exlen) //***** ---********-------
                {
                    cpystctstart = 0;
                    cpydeststart = ex - exlen;
                    cpylen = value->putOffset;
                }
                else //********* ---********-----
                {
                    cpystctstart = 0;
                    cpydeststart = ex - exlen;
                    cpylen = exlen;
                }
                memcpy(value->buffer + cpystctstart,value->buffer + cpydeststart,cpylen);
            }
        }
        
        if(exbuffer_getLen(value) == 0){
            value->putOffset = value->readOffset = 0;
        }
        if((value->putOffset + len) > value->bufferlen){
            size_t len1 = value->bufferlen - value->putOffset;
            memcpy(value->buffer + value->putOffset,buffer + offset,len1);
            offset += len1;
            size_t len2 = len - len1;
            memcpy(value->buffer,buffer + offset,len2);
            value->putOffset = len2;
        }else{
            memcpy(value->buffer + value->putOffset,buffer + offset,len);
            value->putOffset += len;
        }
        //exbuffer_dump(value);
        //printf("value->putOffset:%d\n",value->putOffset);
        //printf("value->readOffset:%d\n",value->readOffset);
        exbuffer_proc(value);
    };
    
    
    
#ifdef __cplusplus
}
#endif


#endif // EXBUFFER_H_INCLUDED
