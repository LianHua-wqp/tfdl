package settings

import (
	"fmt"

	"github.com/fsnotify/fsnotify"
	"github.com/spf13/viper"
)

var Conf = new(AppConfig)

type AppConfig struct {
	Mode       string `mapstructure:"mode"`
	Port       int    `mapstructure:"port"`
	*Hardware  `mapstructure:"hardware"`
	*LogConfig `mapstructure:"log"`
}

type Hardware struct {
	Uart string `mapstructure:"uart"`
	DMA  string `mapstructure:"dma"`
}

type LogConfig struct {
	Level      string `mapstructure:"level"`
	FileName   string `mapstructure:"filename"`
	MaxSize    int    `mapstructure:"max_size"`
	MaxAgent   int    `mapstructure:"max_agent"`
	MaxBackups int    `mapstructure:"max_backups"`
}

func Init(filename string) error {
	var file string
	if len(filename) > 0 {
		file = filename
	}

	viper.SetConfigFile(file)
	viper.WatchConfig()
	viper.OnConfigChange(func(in fsnotify.Event) {
		fmt.Println("配置文件被更改，可以在这里进行相应的操作")
		viper.Unmarshal(Conf)
	})

	err := viper.ReadInConfig()
	if err != nil {
		return err
	}
	err = viper.Unmarshal(Conf)
	if err != nil {
		return err
	}
	fmt.Println(*Conf.Hardware)
	return err
}
