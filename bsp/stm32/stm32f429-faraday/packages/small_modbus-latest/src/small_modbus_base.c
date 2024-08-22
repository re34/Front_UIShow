/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-08-21     chenbin      small modbus the first version
 * 2021-11-29     chenbin      fix bug ,add context check
 */
#include "small_modbus_base.h"
#include "small_modbus_utils.h"
#include "ulog.h"

int _modbus_init(small_modbus_t *smb)
{
    if (smb != NULL)
    {
        smb->modbus_magic = MODBUS_MAGIC;
        smb->device_mode = MODBUS_DEVICE_SLAVE;
        smb->transfer_id = 0;
        smb->protocol_id = 0;
        smb->debug_level = 2; // log level info
        smb->timeout_frame = 100;  //默认100ms
        if (smb->timeout_byte == 0)
        {
            smb->timeout_byte = 10;
        }
    }
    return MODBUS_OK;
}

int modbus_context_check(small_modbus_t *smb)
{
    int ret = MODBUS_OK;
    if (NULL == smb)
    {
        ret = MODBUS_ERROR_CONTEXT;
    }
    if (MODBUS_MAGIC != smb->modbus_magic)
    {
        ret = MODBUS_ERROR_CONTEXT;
    }
    if (NULL == smb->port)
    {
        ret = MODBUS_ERROR_CONTEXT;
    }
    return ret;
}

/*
 * *
 */
int modbus_connect(small_modbus_t *smb)
{
    int ret = MODBUS_FAIL;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
    if( NULL != smb->port->open )
    {
        ret = smb->port->open(smb);
    }
    __exit:
    return ret;
}

int modbus_disconnect(small_modbus_t *smb)
{
    int ret = MODBUS_FAIL;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
    if( NULL != smb->port->close )
    {
        ret = smb->port->close(smb);
    }
    __exit:
    return ret;
}

int modbus_write(small_modbus_t *smb, uint8_t *data, uint16_t length)
{
    int ret = MODBUS_FAIL;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
    if( NULL != smb->port->write )
    {
        ret = smb->port->write(smb, data, length);
    }
    __exit:
    return ret;
}

int modbus_read(small_modbus_t *smb, uint8_t *data, uint16_t length)
{
    int ret = MODBUS_FAIL;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
    if( NULL != smb->port->read )
    {
        ret = smb->port->read(smb, data, length);
    }
    __exit:
    return ret;
}

int modbus_flush(small_modbus_t *smb)
{
    int ret = MODBUS_FAIL;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
    if( NULL != smb->port->flush )
    {
        ret = smb->port->flush(smb);
    }
    __exit:
    return ret;
}

int modbus_wait(small_modbus_t *smb, int timeout)
{
    int ret = MODBUS_FAIL;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
    if( NULL != smb->port->wait )
    {
        ret = smb->port->wait(smb, timeout);
    }
    __exit:
    return ret;
}


int modbus_set_wait_time(small_modbus_t *smb, int32_t wait_time)
{
    int ret = MODBUS_FAIL;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
    if(NULL != smb->port->control)
    {
        ret = smb->port->control(smb, RT_DEVICE_CTRL_TIMEOUT, (void *)wait_time);
    }
    __exit:
    return ret;
}

int modbus_want_read(small_modbus_t *smb, uint8_t *buff, uint16_t len, int32_t wait_time)
{
    uint8_t *read_buff = buff;
    uint16_t read_len = 0;
	int ret = -1;
	
	modbus_set_wait_time(smb, wait_time);
    do
    {
        ret = modbus_read(smb, read_buff + read_len, len - read_len);
		//超时或其他错误，直接返回
		if(ret <= 0)
			return ret;
		else	
			read_len += ret;
    }while (read_len < len);
    return read_len; 
}


int modbus_error_recovery(small_modbus_t *smb)
{
    int ret = MODBUS_OK;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
#if 0	
    if( NULL != smb->port->flush )
    {
        ret = smb->port->flush(smb);
    }
#endif
    __exit:
    return ret;
}

int modbus_error_exit(small_modbus_t *smb, int code)
{
    int ret = MODBUS_OK;

    ret = modbus_context_check(smb);
    if( MODBUS_OK == ret )
    {
        smb->error_code = code;
    }
    return ret;
}



/* set frame timeout (ms) */
int modbus_set_frame_timeout(small_modbus_t *smb, int timeout_ms)
{
    int ret = MODBUS_OK;

    ret = modbus_context_check(smb);
    if( MODBUS_OK == ret )
    {
        smb->timeout_frame = timeout_ms;
    }
    return ret;
}

/* set byte timeout (ms) */
int modbus_set_byte_timeout(small_modbus_t *smb, int timeout_ms)
{
    int ret = MODBUS_OK;

    ret = modbus_context_check(smb);
    if( MODBUS_OK == ret )
    {
        smb->timeout_byte = timeout_ms;
    }
    return ret;
}

/* set slave addr */
int modbus_set_slave(small_modbus_t *smb, int slave)
{
    int ret = MODBUS_OK;

    ret = modbus_context_check(smb);
    if( MODBUS_OK == ret )
    {
        if ((0 < slave) && (247 > slave))
        {
            smb->slave_addr = slave;
        }else
        {
            ret = MODBUS_FAIL;
        }
    }
    return ret;
}

/* set debug level */
int modbus_set_debug(small_modbus_t *smb, int level)
{
    int ret = MODBUS_OK;

    ret = modbus_context_check(smb);
    if( MODBUS_OK == ret )
    {
        smb->debug_level = level;
    }
    return ret;
}

/* 主机发送 */
int modbus_start_request(small_modbus_t *smb, uint8_t *request, int function, int addr, int num, void *write_data)
{
    int ret = MODBUS_OK;
    int len = 0;
    uint16_t count = 0;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }

    //check funcode addr num
    if (modbus_check_addr_num(function, addr, num))
    {
        /*************************************
        *	1. 构建帧头，存入request数组中
        *************************************/
        len = smb->core->build_request_header(smb, request, smb->slave_addr, function, addr, num);
        /*************************************
        *	2. 根据不同的modbus功能码填充指令
        *************************************/
        switch (function)
        {
            case MODBUS_FC_WRITE_SINGLE_COIL:
            {
                request[len - 2] = (*((uint8_t *)write_data)) ? 0xFF : 0x00;
                request[len - 1] = 0x00;
            }
            break;
            case MODBUS_FC_WRITE_SINGLE_REGISTER:
            {
                modbus_reg_h2m(&(request[len - 2]), write_data, 1);
            }
            break;
            case MODBUS_FC_WRITE_MULTIPLE_COILS:
            {
                //localhost data to modbus data
                count = (num / 8) + ((num % 8) ? 1 : 0);
                request[len++] = count;

                modbus_coil_h2m(&(request[len]), write_data, num);

                len += count;
            }
            break;
            case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
            {
                //localhost data to modbus data
                count = (num * 2);
                request[len++] = count;

                modbus_reg_h2m(&(request[len]), write_data, num);

                len += count;
            }
            break;
            default:
            break;
        }
        /*************************************
        *	3. 生成CRC16校验码
        *************************************/
        ret = smb->core->check_send_pre(smb, request, len);
        /*************************************
        *	4. 发送
        *************************************/		
        if (ret > 0)
        {
            len = ret;
            ret = modbus_write(smb, request, len);
        }
    }else
    {
        ret = MODBUS_FAIL_REQUEST;
    }
    __exit:
    return ret;
}
/* master wait for confirmation message */

//主机等待接收
int modbus_wait_confirm(small_modbus_t *smb, int read_want, uint8_t *response)
{
    int ret = MODBUS_OK;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
	
    if(read_want != 0)
    {
    	/**********************************************************
    	*1. 阻塞（有超时时间）读取modbus 应答消息, 直到获取到期望的数据长度
    	*********************************************************/
        ret = modbus_want_read(smb, response, read_want, smb->timeout_frame);
        if (ret != read_want)
        {
             LOG_D("[%d]read(%d) error", ret, read_want);
        }
        else // read ok
        {
			/**********************************************************
			*2. 进行数据校验
			*********************************************************/
    		ret = smb->core->check_wait_response(smb, response, read_want);
        }
    }
    __exit:
    return ret;
}

/* master handle confirmation message */
int modbus_handle_confirm(small_modbus_t *smb, uint8_t *request, uint16_t request_len, uint8_t *response, uint16_t response_len, void *read_data)
{
    int ret = MODBUS_OK;
    uint16_t rw_num = 0;
    uint16_t byte_num = 0;
    uint16_t temp = 0;
    uint16_t calc_length = 0;
    uint8_t request_function = 0;
    uint8_t response_function = 0;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
    calc_length        = smb->core->len_header + smb->core->len_checksum; // header + checksum
    request_function   = request[smb->core->len_header];
    response_function  = response[smb->core->len_header];

    if (response_function >= 0x80)
    {
        if ((response_function - 0x80) == request_function)
        {
            modbus_debug_error(smb, "request function code %d\n", request_function);
        }
        modbus_debug_error(smb, "response exception code %d\n", 0 - response_function);
        ret =  0 - response_function;
        goto __exit;
    }
    if (request_function == response_function)
    {
        switch (request_function)
        {
            case MODBUS_FC_READ_HOLDING_COILS:
            case MODBUS_FC_READ_INPUTS_COILS:
            {
                temp = (request[smb->core->len_header + 3] << 8) | (request[smb->core->len_header + 4]); // data length
                calc_length += (2 + (temp / 8) + ((temp % 8) ? 1 : 0));
            }
            break;
            case MODBUS_FC_WRITE_AND_READ_REGISTERS:
            case MODBUS_FC_READ_HOLDING_REGISTERS:
            case MODBUS_FC_READ_INPUT_REGISTERS:
            {
                temp = (request[smb->core->len_header + 3] << 8) | (request[smb->core->len_header + 4]); // data length
                calc_length += (2 + 2 * temp);
            }
            break;
            case MODBUS_FC_READ_EXCEPTION_STATUS:
                calc_length += 3;
                break;
            case MODBUS_FC_MASK_WRITE_REGISTER:
                calc_length += 7;
                break;
            default:
                calc_length += 5;
        }
        if (calc_length == response_len)
        {
            // read data
            switch (response_function)
            {
                case MODBUS_FC_READ_HOLDING_COILS:
                case MODBUS_FC_READ_INPUTS_COILS:
                {
                    rw_num = (request[smb->core->len_header + 3] << 8) | (request[smb->core->len_header + 4]);
                    temp = ((rw_num / 8) + ((rw_num % 8) ? 1 : 0));
                    byte_num = (response[smb->core->len_header + 1]);
                    if ((uint8_t)temp == byte_num)
                    {
                        modbus_coil_m2h(read_data, &(response[smb->core->len_header + 2]), rw_num);
                        ret = MODBUS_OK;
                        goto __exit;
                    }
                }
                break;
                case MODBUS_FC_READ_HOLDING_REGISTERS:
                case MODBUS_FC_READ_INPUT_REGISTERS:
                {
                    rw_num = (request[smb->core->len_header + 3] << 8) | (request[smb->core->len_header + 4]); // data length
                    temp = rw_num * 2;
                    byte_num = (response[smb->core->len_header + 1]);
                    if ((uint8_t)temp == byte_num)
                    {
                        modbus_reg_m2h(read_data, &(response[smb->core->len_header + 2]), rw_num);
                        ret = MODBUS_OK;
                        goto __exit;
                    }
                }
                break;
                case MODBUS_FC_WRITE_MULTIPLE_COILS:
                case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
                {
                    rw_num = (request[smb->core->len_header + 3] << 8) | (request[smb->core->len_header + 4]); // data length
                    temp = (response[smb->core->len_header + 3] << 8) | (response[smb->core->len_header + 4]); // data length
                    if (rw_num == temp)
                    {
                        ret = MODBUS_OK;
                        goto __exit;
                    }
                }
                break;
                case MODBUS_FC_MASK_WRITE_REGISTER:
                case MODBUS_FC_WRITE_AND_READ_REGISTERS:
                case MODBUS_FC_READ_EXCEPTION_STATUS:
                default:
                {
                        ret = MODBUS_OK;
                        goto __exit;
                }
            }
        }
    }else
    {
        ret = MODBUS_FAIL_CONFIRM;
    }
    __exit:
    return ret;
}

/* 读保持线圈状态*/
int modbus_read_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data)
{
    int ret = MODBUS_OK;
    int request_len = 0;
    int response_len = 0;
	int read_want = 0;
	
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
	read_want = smb->core->len_header + smb->core->len_checksum + num + 2;
    // start request
    ret = modbus_start_request(smb, request, MODBUS_FC_READ_HOLDING_COILS, addr, num, NULL);
    if (0 > ret)
    {
        goto __exit;
    }
    request_len = ret;

    // wait slave comfirm
    ret = modbus_wait_confirm(smb, read_want, response);
    if (0 >= ret)
    {
        goto __exit;
    }
    response_len = ret;

    //check comfirm
    ret = modbus_handle_confirm(smb, request, request_len, response, response_len, read_data);

    __exit:
    return ret;
}

/* 读取输入线圈状态 */
int modbus_read_input_bits(small_modbus_t *smb, int addr, int num, uint8_t *read_data)
{
    int ret = MODBUS_OK;
    int request_len = 0;
    int response_len = 0;
	int read_want = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
	read_want = smb->core->len_header + smb->core->len_checksum + num + 2;

    // start request
    ret = modbus_start_request(smb, request, MODBUS_FC_READ_INPUTS_COILS, addr, num, NULL);
    if (0 > ret)
    {
        goto __exit;
    }
    request_len = ret;

    // wait slave comfirm
    ret = modbus_wait_confirm(smb, read_want, response);
    if (0 >= ret)
    {
        goto __exit;
    }
    response_len = ret;

    //check comfirm
    ret = modbus_handle_confirm(smb, request, request_len, response, response_len, read_data);

    __exit:
    return ret;
}

/* 读取多个保持寄存器 */ 
int modbus_read_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data)
{
    int ret = MODBUS_OK;
    int request_len = 0;
    int response_len = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;
	int read_want = 0;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
		LOG_D("modbus_read_registers: modbus_context_check fail");		
        goto __exit;
    }
	read_want = smb->core->len_header + smb->core->len_checksum + num * 2 + 2;
    //发送modbus请求
    ret = modbus_start_request(smb, request, MODBUS_FC_READ_HOLDING_REGISTERS, addr, num, NULL);
    if (0 > ret)
    {
		LOG_D("modbus_read_registers: modbus_start_request fail");	
        goto __exit;
    }
    request_len = ret;
    //等待接收数据
    ret = modbus_wait_confirm(smb, read_want, response);
    if (ret <= 0)
    {
		LOG_D("modbus_read_registers: modbus_wait_confirm fail");	
        goto __exit;
    }
    response_len = ret;
    //check comfirm
    ret = modbus_handle_confirm(smb, request, request_len, response, response_len, read_data);
    __exit:
    return ret;
}


/* 读取多个输入寄存器 */ 
int modbus_read_input_registers(small_modbus_t *smb, int addr, int num, uint16_t *read_data)
{
    int ret = MODBUS_OK;
    int request_len = 0;
    int response_len = 0;
	int read_want = 0;
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
	read_want = smb->core->len_header + smb->core->len_checksum + num * 2 + 2;
    // start request
    ret = modbus_start_request(smb, request, MODBUS_FC_READ_INPUT_REGISTERS, addr, num, NULL);
    if (0 > ret)
    {
        goto __exit;
    }
    request_len = ret;

    // wait slave comfirm
    ret = modbus_wait_confirm(smb, read_want, response);
    if (0 >= ret)
    {
        goto __exit;
    }
    response_len = ret;
    //check comfirm
    ret = modbus_handle_confirm(smb, request, request_len, response, response_len, read_data);
    __exit:
    return ret;
}


/* master write */
//写单个线圈
int modbus_write_bit(small_modbus_t *smb, int addr, int write_status)
{
    int ret = MODBUS_OK;
    int request_len = 0;
    int response_len = 0;
	int read_want = 0;
	
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;
    int status = write_status ? 0xFF00 : 0x0;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
	read_want = smb->core->len_header + smb->core->len_checksum + 5;

    // start request
    ret = modbus_start_request(smb, request, MODBUS_FC_WRITE_SINGLE_COIL, addr, 1, &status);
    if (0 > ret)
    {
        goto __exit;
    }
    request_len = ret;

    // wait slave comfirm
    ret = modbus_wait_confirm(smb, read_want, response);
    if (0 >= ret)
    {
        goto __exit;
    }
    response_len = ret;

    //check comfirm
    ret = modbus_handle_confirm(smb, request, request_len, response, response_len, NULL);

    __exit:
    return ret;
}

//写单个保持寄存器
int modbus_write_register(small_modbus_t *smb, int addr, int write_value)
{
    int ret = MODBUS_OK;
    int request_len = 0;
    int response_len = 0;
	int read_want = 0;
	
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;
    int value = write_value;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
	read_want = smb->core->len_header + smb->core->len_checksum + 5;

    // start request
    ret = modbus_start_request(smb, request, MODBUS_FC_WRITE_SINGLE_REGISTER, addr, 1, &value);
    if (0 > ret)
    {
        goto __exit;
    }
    request_len = ret;

    // wait slave comfirm
    ret = modbus_wait_confirm(smb, read_want, response);
    if (0 >= ret)
    {
        goto __exit;
    }
    response_len = ret;
    //check comfirm
    ret = modbus_handle_confirm(smb, request, request_len, response, response_len, NULL);
    __exit:
    return ret;
}


//写多个线圈
int modbus_write_bits(small_modbus_t *smb, int addr, int num, uint8_t *write_data)
{
    int ret = MODBUS_OK;
    int request_len = 0;
    int response_len = 0;
	int read_want = 0;
	
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
	read_want = smb->core->len_header + smb->core->len_checksum + 5;

    // start request
    ret = modbus_start_request(smb, request, MODBUS_FC_WRITE_MULTIPLE_COILS, addr, num, write_data);
    if (0 > ret)
    {
        goto __exit;
    }
    request_len = ret;
    // wait slave comfirm
    ret = modbus_wait_confirm(smb, read_want, response);
    if (0 >= ret)
    {
        goto __exit;
    }
    response_len = ret;
    //check comfirm
    ret = modbus_handle_confirm(smb, request, request_len, response, response_len, NULL);
    __exit:
    return ret;
}

//写多个保持寄存器
int modbus_write_registers(small_modbus_t *smb, int addr, int num, uint16_t *write_data)
{
    int ret = MODBUS_OK;
    int request_len = 0;
    int response_len = 0;
	int read_want = 0;
	
    uint8_t *request = smb->write_buff;
    uint8_t *response = smb->read_buff;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }
	read_want = smb->core->len_header + smb->core->len_checksum + 5;
    // start request
    ret = modbus_start_request(smb, request, MODBUS_FC_WRITE_MULTIPLE_REGISTERS, addr, num, write_data);
    if (0 > ret)
    {
        goto __exit;
    }
    request_len = ret;

    // wait slave comfirm
    ret = modbus_wait_confirm(smb, read_want, response);
    if (0 >= ret)
    {
        goto __exit;
    }
    response_len = ret;
    //check comfirm
    ret = modbus_handle_confirm(smb, request, request_len, response, response_len, NULL);
    __exit:
    return ret;
}


/* master write and read */
int modbus_mask_write_register(small_modbus_t *smb, int addr, uint16_t and_mask, uint16_t or_mask)
{
    int rc = modbus_context_check(smb);
    if (rc < MODBUS_OK)
    {
        return rc;
    }
    return MODBUS_FAIL;
}
int modbus_write_and_read_registers(small_modbus_t *smb, int write_addr, int write_nb, uint16_t *src, int read_addr, int read_nb, uint16_t *dest)
{
    int rc = modbus_context_check(smb);
    if (rc < MODBUS_OK)
    {
        return rc;
    }
    return MODBUS_FAIL;
}

/* slave wait query data */
int modbus_slave_wait(small_modbus_t *smb, uint8_t *request, int32_t wait_time)
{
    int ret = MODBUS_OK;
    int read_step = 0;
    int read_want = 0;
    int read_length = 0;
    int function = 0;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }

    read_want = smb->core->len_header + 1; // header + function code

    while (read_want != 0)
    {
        ret = modbus_want_read(smb, request + read_length, read_want, wait_time);
        if (MODBUS_OK > ret)
        {
            //modbus_debug_error(smb, "[%d]read(%d) error\n", ret, read_want);
            goto __exit;
        }
        if (ret != read_want)
        {
            //modbus_debug_info(smb, "[%d]read(%d) less\n", ret, read_want);
        }

        read_length += ret;     // sum byte length
        read_want -= ret;       // sub byte length

        if (read_want == 0) // read ok
        {
            switch (read_step)
            {
            case 0: /* Function code position */
            {
                function = request[smb->core->len_header];
                if (function <= MODBUS_FC_WRITE_SINGLE_REGISTER) // 0x01 - 0x06
                {
                    read_want = 4;
                }
                else if ((function == MODBUS_FC_WRITE_MULTIPLE_COILS) || (function == MODBUS_FC_WRITE_MULTIPLE_REGISTERS))
                {
                    read_want = 5;
                }
                else if (function == MODBUS_FC_MASK_WRITE_REGISTER)
                {
                    read_want = 6;
                }
                else if (function == MODBUS_FC_WRITE_AND_READ_REGISTERS)
                {
                    read_want = 9;
                }
                else
                {
                    read_want = 0; // not want read
                }
                if (read_want != 0)
                {
                    read_step = 1;
                    break;
                }
            }
            case 1:
            {
                function = request[smb->core->len_header];
                if ((function == MODBUS_FC_WRITE_MULTIPLE_COILS) || (function == MODBUS_FC_WRITE_MULTIPLE_REGISTERS))
                {
                    read_want = request[smb->core->len_header + 5];
                }
                else if (function == MODBUS_FC_WRITE_AND_READ_REGISTERS)
                {
                    read_want = request[smb->core->len_header + 9];
                }
                else
                {
                    read_want = 0;
                }
                read_want += smb->core->len_checksum;
                read_step = 2;
                if ((read_want + read_length) > smb->core->len_adu_max)
                {
                    modbus_debug_error(smb, "More than ADU %d > %d\n", (read_want + read_length), smb->core->len_adu_max);
                    return MODBUS_FAIL;
                }
            }
            break;
            }
        }
        if (read_want)
        {
            wait_time = smb->timeout_byte * read_want; // byte_time * byte_num
        }
    }
    //check data
    ret = smb->core->check_wait_request(smb, request, read_length);

    __exit:
    return ret;
}

/* slave handle query data for callback */
int modbus_slave_handle(small_modbus_t *smb, uint8_t *request, uint16_t request_len, small_modbus_slave_callback_t slave_callback)
{
    int ret = MODBUS_OK;
    // uint8_t *request = smb->read_buff;
    uint8_t *   response = smb->write_buff;
    uint16_t    response_len = 0;
    uint16_t    query_address = 0;
    uint16_t    query_num = 0;
    uint8_t     query_slave = smb->slave_addr;
    uint8_t     query_function = request[smb->core->len_header];
    uint8_t     bytes = 0;
    int         response_data_len = 0;
    int         response_data_exception = MODBUS_OK;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }

    /* Data are flushed on illegal number of values errors. */
    switch (query_function)
    {
        case MODBUS_FC_READ_HOLDING_COILS:
        case MODBUS_FC_READ_INPUTS_COILS:
        {
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];
            if (modbus_check_addr_num(query_function, query_address, query_num))
            {
                response_len = smb->core->build_response_header(smb, response, query_slave, query_function);
                if (slave_callback)
                {
                    response_data_len = slave_callback(smb, query_function, query_address, query_num, (response + response_len + 1));
                }
                if ((0 < response_data_len) && modbus_check_addr_num(query_function, query_address, response_data_len))
                {
                    bytes = (response_data_len / 8) + ((response_data_len % 8) ? 1 : 0);
                    response[response_len] = bytes;

                    modbus_coil_h2m((response + response_len + 1), (response + response_len + 1), response_data_len);

                    response_len += (bytes + 1);
                }
                else
                {
                    modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,num:%d rc:%d\n", query_slave, query_function, query_address, query_num, response_data_len);
                    response_data_exception = MODBUS_EXCEPTION;
                }
            }
            else
            {
                modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,num:%d not at[1-%d]\n", query_slave, query_function, query_address, query_num, MODBUS_MAX_READ_BITS);
                response_data_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
            }
        }
        break;
        case MODBUS_FC_READ_HOLDING_REGISTERS:
        case MODBUS_FC_READ_INPUT_REGISTERS:
        {
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];
            //check funcode addr num
            if (modbus_check_addr_num(query_function, query_address, query_num))
            {
                response_len = smb->core->build_response_header(smb, response, query_slave, query_function);

                if (slave_callback)
                {
                    //callback
                    response_data_len = slave_callback(smb, query_function, query_address, query_num, (response + response_len + 1));
                }
                //check funcode addr num
                if ( (0 < response_data_len) && modbus_check_addr_num(query_function, query_address, response_data_len))
                {
                    bytes = response_data_len * 2;
                    response[response_len] = bytes;
                    //host to modbus
                    modbus_reg_h2m((response + response_len + 1), (response + response_len + 1), response_data_len);

                    response_len += (bytes + 1);
                }
                else
                {
                    modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,num:%d rc:%d\n", query_slave, query_function, query_address, query_num, response_data_len);
                    response_data_exception = MODBUS_EXCEPTION;
                }
            }
            else
            {
                modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,num:%d not at[1-%d]\n", query_slave, query_function, query_address, query_num, MODBUS_MAX_READ_REGISTERS);
                response_data_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
            }
        }
        break;
        case MODBUS_FC_WRITE_SINGLE_COIL:
        {
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];  //value

            response_len = smb->core->build_response_header(smb, response, query_slave, query_function);

            if (slave_callback)
            {
                // modbus to host
                modbus_coil_m2h((request + smb->core->len_header + 3), (request + smb->core->len_header + 3), 1);
                //callback
                response_data_len = slave_callback(smb, query_function, query_address, 1, (request + smb->core->len_header + 3));
            }
            if (1 == response_data_len)
            {
                response[response_len++] = (query_address >> 8);
                response[response_len++] = (query_address & 0x00ff);
                response[response_len++] = (query_num >> 8); //请求 值
                response[response_len++] = (query_num & 0x00ff);
            }
            else
            {
                modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,value:%d rc:%d\n", query_slave, query_function, query_address, query_num, response_data_len);
                response_data_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
            }
        }
        break;
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
        {
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4]; //value

            response_len = smb->core->build_response_header(smb, response, query_slave, query_function);

            if (slave_callback)
            {
                // modbus to host
                modbus_reg_m2h((request + smb->core->len_header + 3), (request + smb->core->len_header + 3), 1);
                //callback
                response_data_len = slave_callback(smb, query_function, query_address, 1, (request + smb->core->len_header + 3));
            }
            if (1 == response_data_len)
            {
                response[response_len++] = (query_address >> 8);
                response[response_len++] = (query_address & 0x00ff);
                response[response_len++] = (query_num >> 8); //value
                response[response_len++] = (query_num & 0x00ff);
            }
            else
            {
                modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,value:%d rc:%d\n", query_slave, query_function, query_address, query_num, response_data_len);
                response_data_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
            }
        }
        break;
        case MODBUS_FC_WRITE_MULTIPLE_COILS:
        {
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];

            if (modbus_check_addr_num(query_function, query_address, query_num))
            {
                response_len = smb->core->build_response_header(smb, response, query_slave, query_function);

                if (slave_callback)
                {
                    // modbus to host
                    modbus_coil_m2h((request + smb->core->len_header + 6), (request + smb->core->len_header + 6), query_num);
                    //callback
                    response_data_len = slave_callback(smb, query_function, query_address, query_num, (request + smb->core->len_header + 6));
                }
                //check funcode addr num
                if ( (0 < response_data_len) && modbus_check_addr_num(query_function, query_address, response_data_len))
                {
                    response[response_len++] = (query_address >> 8);
                    response[response_len++] = (query_address & 0x00ff);
                    response[response_len++] = (query_num >> 8);
                    response[response_len++] = (query_num & 0x00ff);
                }
                else
                {
                    modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,num:%d rc:%d\n", query_slave, query_function, query_address, query_num, response_data_len);
                    response_data_exception = MODBUS_EXCEPTION;
                }
            }
            else
            {
                modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,num:%d not at[1-%d]\n", query_slave, query_function, query_address, query_num, MODBUS_MAX_WRITE_BITS);
                response_data_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
            }
        }
        break;
        case MODBUS_FC_WRITE_MULTIPLE_REGISTERS:
        {
            query_address = (request[smb->core->len_header + 1] << 8) + request[smb->core->len_header + 2];
            query_num = (request[smb->core->len_header + 3] << 8) + request[smb->core->len_header + 4];
            //check funcode addr num
            if (modbus_check_addr_num(query_function, query_address, query_num))
            {
                response_len = smb->core->build_response_header(smb, response, query_slave, query_function);
                if (slave_callback)
                {
                    // modbus to host
                    modbus_reg_m2h((request + smb->core->len_header + 6), (request + smb->core->len_header + 6), query_num);
                    //callback
                    response_data_len = slave_callback(smb, query_function, query_address, query_num, (request + smb->core->len_header + 6));
                }
                //check funcode addr num
                if ( (0 < response_data_len) && modbus_check_addr_num(query_function, query_address, response_data_len))
                {
                    response[response_len++] = (query_address >> 8);
                    response[response_len++] = (query_address & 0x00ff);
                    response[response_len++] = (query_num >> 8);
                    response[response_len++] = (query_num & 0x00ff);
                }
                else
                {
                    modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,num:%d rc:%d\n", query_slave, query_function, query_address, query_num, response_data_len);
                    response_data_exception = MODBUS_EXCEPTION;
                }
            }
            else
            {
                modbus_debug_error(smb, "slave:0x%0X,function:0x%0X,addr:%d,num:%d not at[1-%d]\n", query_slave, query_function, query_address, query_num, MODBUS_MAX_WRITE_REGISTERS);
                response_data_exception = MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE;
            }
        }
        break;
        case MODBUS_FC_REPORT_SLAVE_ID:
        {
            response_len = smb->core->build_response_header(smb, response, query_slave, query_function);
            response[response_len++] = 9;
            response[response_len++] = smb->slave_addr; //_REPORT_SLAVE_ID;
            response[response_len++] = 0xFF;
            response[response_len++] = 'S';
            response[response_len++] = 'M';
            response[response_len++] = 'O';
            response[response_len++] = 'D';
            response[response_len++] = 'B';
            response[response_len++] = 'U';
            response[response_len++] = 'S';
        }
        break;
        case MODBUS_FC_READ_EXCEPTION_STATUS:
        case MODBUS_FC_MASK_WRITE_REGISTER:
        case MODBUS_FC_WRITE_AND_READ_REGISTERS:
        {
            modbus_debug_error(smb, "slave:0x%0X,Unknown Modbus function code: 0x%0X\n", query_slave, query_function);
            response_data_exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
        }
        break;
        default:
        {
            modbus_debug_error(smb, "slave:0x%0X,Unknown Modbus function code: 0x%0X\n", query_slave, query_function);
            response_data_exception = MODBUS_EXCEPTION_ILLEGAL_FUNCTION;
        }
        break;
    }

    if (response_data_exception <= MODBUS_EXCEPTION)
    {
        if(response_data_len < 0)
        {
            //used callback return value
            response_data_exception = response_data_len;
        }
        response_len = smb->core->build_response_header(smb, response, query_slave, query_function + 0x80);
        response[response_len++] = response_data_exception;
    }
    if (response_len > 0)
    {
        ret = smb->core->check_send_pre(smb, response, response_len);
        if (ret > 0)
        {
            ret = modbus_write(smb, response, ret);
        }
    }else
    {
        ret = MODBUS_FAIL_HANDLE;
    }

    __exit:
    return ret;
}

/* slave wait and handle query for callback */
int modbus_slave_wait_handle(small_modbus_t *smb, small_modbus_slave_callback_t slave_callback, int32_t waittime)
{
    int ret = 0;
    // uint8_t *confirm = smb->write_buff;
    uint8_t *request = smb->read_buff;

    ret = modbus_context_check(smb);
    if( MODBUS_OK != ret )
    {
        goto __exit;
    }

    ret = modbus_slave_wait(smb, request, waittime);
    if (0 > ret )
    {
        goto __exit;
    }

    ret = modbus_slave_handle(smb, request, ret, slave_callback);

    __exit:
    return ret;
}
