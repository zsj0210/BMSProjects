#ifndef __BMS_DEBUG_H__
#define __BMS_DEBUG_H__


#include <rtthread.h>
#include "usart.h"


#define BMS_DEBUG_LEVEL	1


#ifdef BMS_DBG_TAG

#ifndef BMS_DBG_SECTION_NAME
#define BMS_DBG_SECTION_NAME	BMS_DBG_TAG
#endif

#else

#ifndef BMS_DBG_SECTION_NAME
#define BMS_DBG_SECTION_NAME    "DBG"
#endif
#endif 



#if (BMS_DEBUG_LEVEL == 0)

#define BMS_ERROR(...)		do{}while(0)
#define BMS_WARNING(...)	do{}while(0)
#define BMS_INFO(...)		do{}while(0)

#elif (BMS_DEBUG_LEVEL == 1)

#define BMS_ERROR(fmt, arg...)                                		\
		do															\
        {                                                       	\
            print_usart1("[BMS ERROR][%s]", BMS_DBG_SECTION_NAME);    \
            print_usart1(fmt"\r\n", ##arg);                     		\
        } while(0)
#define BMS_WARNING(fmt, arg...)                             		\
		do															\
        {                                                       	\
            print_usart1("[BMS WARNING][%s]", BMS_DBG_SECTION_NAME);  \
            print_usart1(fmt"\r\n", ##arg);                      		\
        } while(0)
#define BMS_INFO(fmt, arg...)                                  		\
		do															\
        {                                                       	\
            print_usart1("[BMS INFO][%s]", BMS_DBG_SECTION_NAME);     \
            print_usart1(fmt"\r\n", ##arg);                      		\
        } while(0)


#elif (BMS_DEBUG_LEVEL == 2)

#define BMS_ERROR(fmt, arg...)                              					\
		do																		\
        {                                                      					\
            print_usart1("[BMS ERROR][%s][%s:%s:%d] ",							\
                    BMS_DBG_SECTION_NAME, __FILE__, __FUNCTION__, __LINE__);    \
            print_usart1(fmt"\r\n", ##arg);                      					\
        } while(0)
#define BMS_WARNING(fmt, arg...)                               					\
		do																		\
        {                                                       				\
            print_usart1("[BMS WARNING][%s][%s:%s:%d] ",							\
                    BMS_DBG_SECTION_NAME, __FILE__, __FUNCTION__, __LINE__);    \
            print_usart1(fmt"\r\n", ##arg);                      					\
        } while(0)
#define BMS_INFO(fmt, arg...)                                  					\
		do																		\
        {                                                       				\
            print_usart1("[BMS INFO][%s][%s:%s:%d] ",								\
                    BMS_DBG_SECTION_NAME, __FILE__, __FUNCTION__, __LINE__);    \
            print_usart1(fmt"\r\n", ##arg);										\
        } while(0)
        
#endif

		
		
		
		
#endif
		
		
