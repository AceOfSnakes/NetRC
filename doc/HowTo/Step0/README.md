# How to setup your application

What do You see from the scratch

![About W10](images/Original.png)

Just click on icon with ![](../../../src/images/connect-red.png) and select settings folder 

![About W10](images/Select-Device.png)

Click on icon with plus udner "Device Protocol" and select desired file
## Select file with wanted config and load it into application
### Predefined configurations are stored in ./settings folder 

BDP players

* Oppo 9x
* Oppo 10x
* Pioneer 

UHD BDP players
* Oppo 20x

Reciever
* Pioneer Receiver till 2016"


#### Minimal custom configuration file sample

Just edit it or create for Your purposes

```
{
   "powerOff":"#POW",
   "powerOn":"#POW",

   "pingCommands":[
      "#QPW"
   ],
   "pingResponseErr":"QPW OK OFF",
   "pingResponseOk":"QPW OK ON",
   "prefferedPort":23,
   "crlf":true,
   "family":"Just for connect"
} 
```

##### Select your configuration file
![Alt text](image.png)

Select file with settings and click open

### Apply your settings

Try one of two options 

* click Auto search and select device

![Alt text](images/auto-search.png)

Or type your device IP

* replace question sign with digits

![Alt text](images/ip.png)

#### Result

Predefined | Custom
----------- | ------------
![Predefined](images/predefined.png) | ![Custom](images/custom.png)  
