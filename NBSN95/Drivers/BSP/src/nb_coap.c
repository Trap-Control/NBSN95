#include "nb_coap.h"

static char buff[200]={0};
/**
	* @brief  Configure to show the CoAP option of sender
  * @param  Instruction parameter
  * @retval None
  */
NB_TaskStatus nb_COAP_config_run(const char* param)
{
	NBTask[_AT_COAP_CONFIG].try_num = 2;
	NBTask[_AT_COAP_CONFIG].set(param);
	
	while(NBTask[_AT_COAP_CONFIG].try_num--)
	{
		if(nb_at_send(&NBTask[_AT_COAP_CONFIG]) == NB_CMD_SUCC )
		{
			NBTask[_AT_COAP_CONFIG].nb_cmd_status = NB_CMD_SUCC;
			break;
		}
		else
		{
			NBTask[_AT_COAP_CONFIG].nb_cmd_status = NB_CMD_FAIL;
			HAL_Delay(500);
		}
	}
	return NBTask[_AT_COAP_CONFIG].nb_cmd_status;
}

NB_TaskStatus nb_COAP_config_set(const char* param)
{
	user_main_debug("NBTask[_AT_COAP_CONFIG].ATSendStr:%s",NBTask[_AT_COAP_CONFIG].ATSendStr);
	return NBTask[_AT_COAP_CONFIG].nb_cmd_status;
}

NB_TaskStatus nb_COAP_config_get(const char* param)
{
	return NBTask[_AT_COAP_CONFIG].nb_cmd_status;
}

/**
	* @brief  Create a CoAP context
  * @param  Instruction parameter
  * @retval None
  */
NB_TaskStatus nb_COAP_open_run(const char* param)
{
	NBTask[_AT_COAP_OPEN].try_num = 4;
	NBTask[_AT_COAP_OPEN].set(param);
	
	while(NBTask[_AT_COAP_OPEN].try_num--)
	{
		if(nb_at_send(&NBTask[_AT_COAP_OPEN]) == NB_CMD_SUCC )
		{
			NBTask[_AT_COAP_OPEN].nb_cmd_status = NB_OPEN_SUCC;
			break;
		}
		else
		{
			NBTask[_AT_COAP_OPEN].nb_cmd_status = NB_OPEN_FAIL;
			HAL_Delay(500);
		}
	}
	return NBTask[_AT_COAP_OPEN].nb_cmd_status;
}

NB_TaskStatus nb_COAP_open_set(const char* param)
{
	user_main_debug("NBTask[_AT_COAP_OPEN].ATSendStr:%s",NBTask[_AT_COAP_OPEN].ATSendStr);
	return NBTask[_AT_COAP_OPEN].nb_cmd_status;
}

NB_TaskStatus nb_COAP_open_get(const char* param)
{
	return NBTask[_AT_COAP_OPEN].nb_cmd_status;
}

/**
	* @brief  Configure CoAP Options
  * @param  Instruction parameter
  * @retval None
  */
NB_TaskStatus nb_COAP_option_run(const char* param)
{
	NBTask[_AT_COAP_OPTION].try_num = 4;
	NBTask[_AT_COAP_OPTION].set(param);
	
	while(NBTask[_AT_COAP_OPTION].try_num--)
	{
		if(nb_at_send(&NBTask[_AT_COAP_OPTION]) == NB_CMD_SUCC )
		{
			NBTask[_AT_COAP_OPTION].nb_cmd_status = NB_CMD_SUCC;
			break;
		}
		else
		{
			NBTask[_AT_COAP_OPTION].nb_cmd_status = NB_CMD_FAIL;
			HAL_Delay(500);
		}
	}
	return NBTask[_AT_COAP_OPTION].nb_cmd_status;
}

NB_TaskStatus nb_COAP_option_set(const char* param)
{
	memset(buff,0,sizeof(buff));
	strcat(buff,AT QCOAPOPTION"=");
	strcat(buff,(char*)user.uri);
	strcat(buff,"\n");
	
	NBTask[_AT_COAP_OPTION].ATSendStr  = buff;
	NBTask[_AT_COAP_OPTION].len_string = strlen(NBTask[_AT_COAP_OPTION].ATSendStr);
	user_main_debug("NBTask[_AT_COAP_OPTION].ATSendStr:%s",NBTask[_AT_COAP_OPTION].ATSendStr);
	return NBTask[_AT_COAP_OPTION].nb_cmd_status;
}
	
NB_TaskStatus nb_COAP_option_get(const char* param)
{
	return NBTask[_AT_COAP_OPTION].nb_cmd_status;
}
/**
	* @brief  COAP Send DATA
  * @param  Instruction parameter
  * @retval None
  */
NB_TaskStatus nb_COAP_send_run(const char* param)
{
	NBTask[_AT_COAP_SEND].try_num = 5;
	NBTask[_AT_COAP_SEND].set(NULL);
	
	if(nb_at_send(&NBTask[_AT_COAP_SEND]) == NB_CMD_SUCC)
	{
		HAL_UART_Transmit_DMA(&hlpuart1,(uint8_t*)sensor.data,sensor.data_len);
		while(NBTask[_AT_COAP_SEND].try_num--)
		{
		if( NBTask[_AT_COAP_SEND].get(param) == NB_SEND_SUCC)
			break;
		else
			user_main_printf("Send ACK not received...");
		}
	}	
	else
	{
		NBTask[_AT_COAP_SEND].nb_cmd_status = NB_SEND_FAIL;
		HAL_Delay(500);
	}
	return NBTask[_AT_COAP_SEND].nb_cmd_status;
}

NB_TaskStatus nb_COAP_send_set(const char* param)
{
	memset(buff,0,sizeof(buff));
	strcat(buff,AT QCOAPSEND"=0,3,");
	strcat(buff,(char*)user.add);
	strcat(buff,"\n");
	
	NBTask[_AT_COAP_SEND].ATSendStr  = buff;
	NBTask[_AT_COAP_SEND].len_string = strlen(NBTask[_AT_COAP_SEND].ATSendStr);
	user_main_debug("NBTask[_AT_COAP_SEND].ATSendStr:%s",NBTask[_AT_COAP_SEND].ATSendStr);
	return NBTask[_AT_COAP_SEND].nb_cmd_status;
}

NB_TaskStatus nb_COAP_send_get(const char* param)
{
	uint32_t time = HAL_GetTick();
	while(HAL_GetTick() - time < 2000 && nb.recieve_flag != NB_RECIEVE )
	{
		user_main_info("...");
	}

	nb.recieve_flag = NB_IDIE;	
	user_main_info("recieve_data_server:%s",nb.recieve_data_server);
	
	if(strstr(nb.recieve_data_server,QCOAPURC": \"rsp\",2,2.04") != NULL || strstr(nb.recieve_data,QCOAPURC": \"rsp\",2,2.04") != NULL )
	{
		NBTask[_AT_COAP_SEND].nb_cmd_status = NB_SEND_SUCC;	
	}
	else
		NBTask[_AT_COAP_SEND].nb_cmd_status = NB_SEND_FAIL;
	
	return NBTask[_AT_COAP_SEND].nb_cmd_status;
}

//////////////////////////////////
NB_TaskStatus nb_COAP_read_run(const char* param)
{
	return NBTask[_AT_COAP_READ].nb_cmd_status;
}
	
NB_TaskStatus nb_COAP_read_set(const char* param)
{
	return NBTask[_AT_COAP_READ].nb_cmd_status;
}
	
NB_TaskStatus nb_COAP_read_get(const char* param)
{
	return NBTask[_AT_COAP_READ].nb_cmd_status;
}

/**
	* @brief  Delete the CoAP context
  * @param  Instruction parameter
  * @retval None
  */
NB_TaskStatus nb_COAP_close_run(const char* param)
{
	NBTask[_AT_COAP_CLOSE].try_num = 2;
	NBTask[_AT_COAP_CLOSE].set(param);
	while(NBTask[_AT_COAP_CLOSE].try_num--)
	{
		if(nb_at_send(&NBTask[_AT_COAP_CLOSE]) == NB_CMD_SUCC )
		{
			NBTask[_AT_COAP_CLOSE].nb_cmd_status = NB_CLOSE_SUCC;
			break;
		}
		else
		{
			NBTask[_AT_COAP_CLOSE].nb_cmd_status = NB_CLOSE_FAIL;
			HAL_Delay(500);
		}
	}
	return NBTask[_AT_COAP_CLOSE].nb_cmd_status;
}
	
NB_TaskStatus nb_COAP_close_set(const char* param)
{
	user_main_debug("NBTask[_AT_COAP_CLOSE].ATSendStr:%s",NBTask[_AT_COAP_CLOSE].ATSendStr);
	return NBTask[_AT_COAP_CLOSE].nb_cmd_status;
}

NB_TaskStatus nb_COAP_close_get(const char* param)
{
	return NBTask[_AT_COAP_CLOSE].nb_cmd_status;
}
