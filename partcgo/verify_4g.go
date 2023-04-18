package partcgo

const (
	//初始配置部分
	AT_TEST  = iota // 测试AT指令功能是否正常
	AT_CPIN         // 查询SIM卡是否正常，返回ready则表示SIM卡正常
	AT_CREG         // 查询模组是否注册上GSM网络
	AT_CGREG        // 查询模组是否注册上GPRS网络

	AT_QICSGP_CMNET  // 配置移动网络
	AT_QICSGP_CTNET  // 配置电信网络
	AT_QICSGP_UNINET // 配置联通

	//AT_ACTIVATE_NETWORK = "AT+CGATT?\r\n"							//激活网络
	AT_ACTIVATE_SCENE   //场景激活
	AT_DEACTIVATE_SCENE //场景去激活
	AT_OPEN_SOCKET      //创建TCP客户端，0,1表示直吐工作模式
	AT_CLOSE_SOCKET     //断开QISTATE: 0的SOCKET，即断开第一个连接
	AT_SEND_LENTH       //发送的数据长度应该与该命令中的%d一致
)

const (
	REPLY_OK     = "OK"
	REPLY_FAILED = "ERROR"
)

type VerifyStruct struct {
	CMD     string
	Success string
	Failed  []string

	TurnOffError bool //true 即使命令返回的是失败，也不返回任何错误
}

var VerifyMap = make(map[int]*VerifyStruct)

func init() {
	VerifyMap[AT_TEST] = &VerifyStruct{CMD: "AT\r\n", Success: REPLY_OK, Failed: []string{REPLY_FAILED, "AT"}}
	VerifyMap[AT_CPIN] = &VerifyStruct{CMD: "AT+CPIN?\r\n", Success: "+CPIN: READY", Failed: []string{REPLY_FAILED, "+CPIN: "}}
	VerifyMap[AT_CREG] = &VerifyStruct{CMD: "AT+CREG?\r\n", Success: "+CREG: 0,1", Failed: []string{REPLY_FAILED, "+CREG: 0,"}}
	VerifyMap[AT_CGREG] = &VerifyStruct{CMD: "AT+CGREG?\r\n", Success: "+CGREG: 0,1", Failed: []string{REPLY_FAILED, "+CGREG: 0,"}}

	VerifyMap[AT_QICSGP_CTNET] = &VerifyStruct{CMD: "AT+QICSGP=1,1,\"CTNET\",\"\",\"\",1\r\n", Success: REPLY_OK, Failed: []string{REPLY_FAILED}}
	VerifyMap[AT_ACTIVATE_SCENE] = &VerifyStruct{CMD: "AT+QIACT=1\r\n", Success: REPLY_OK, Failed: []string{REPLY_FAILED}, TurnOffError: true}
	VerifyMap[AT_DEACTIVATE_SCENE] = &VerifyStruct{CMD: "AT+QIDEACT=1\r\n", Success: REPLY_OK, Failed: []string{REPLY_FAILED}}
	VerifyMap[AT_OPEN_SOCKET] = &VerifyStruct{CMD: "AT+QIOPEN=1,0,\"TCP\",\"%s\",%d,0,1\r\n", Success: "+QIOPEN: 0,0", Failed: []string{REPLY_FAILED, "+QIOPEN: 0"}}
	VerifyMap[AT_CLOSE_SOCKET] = &VerifyStruct{CMD: "AT+QICLOSE=0\r\n", Success: REPLY_OK, Failed: []string{REPLY_FAILED}}
	VerifyMap[AT_SEND_LENTH] = &VerifyStruct{CMD: "AT+QISEND=0,%d\r\n", Success: ">", Failed: []string{REPLY_FAILED}}
}

func GetVerifyStruct(index int) *VerifyStruct {
	return VerifyMap[index]
}
