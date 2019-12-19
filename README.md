# BCom

An experiment for a (simple) serial reliable protcol
with start and stop byte. 
All data is base64 encoded and crc16 signed. 

future versions will also support syncroneous encryption and
fcount veryfication.

Software is licenced under MIT license see : LICENSE file.

I used software found on multiple sources.

for now this is just a project for educational perposes. 

The layout is simple:

```Microcontroller -> RS232/RS485 -> Tranciever -> USB -> RaspberryPi -> Mqtt Broker 
```^------+------^                                        ^-----------+------------^
```       | 							      |
```	   +- PSoC creator project			              +- BCom2Mqtt C# project
```		  PSoC 5 Devkit C project
		  
		  
```User interface -> Mqtt Broker
```^-----+------^
```      |
````     +-> C# project`
	  
	  
	  
Added a PSoC component. 
Not nearly ready. 
